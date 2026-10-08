#include "lspController.hpp"
#include "gui/editor/controls/textCtrl/textCtrl.hpp"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <nlohmann/json.hpp>
#include <set>
#include <wx/filefn.h>
#include <wx/filename.h>
#include <wx/filesys.h>

using json = nlohmann::json;

static const int SYNC_DELAY_MS = 40;
static const int COMPLETION_DELAY_MS = 40;
static const int MAX_PENDING_CHANGES = 512;
static const int FULL_SYNC_BYTES = 256 * 1024;
static const size_t MAX_ITEMS = 200;
static const char AC_ITEM_SEPARATOR = ' ';

static int Utf16Len(const char *s, size_t n) {
	int units = 0;
	for (size_t i = 0; i < n; ++i) {
		unsigned char b = (unsigned char)s[i];
		if ((b & 0xC0) == 0x80)
			continue;
		units += (b >= 0xF0) ? 2 : 1;
	}
	return units;
}

static std::string ToUtf8(const wxString &s) {
	return std::string(s.utf8_str());
}

static wxString PathToUri(const wxString &path, bool isDir) {
	wxFileName fn = isDir ? wxFileName::DirName(path) : wxFileName(path);
	fn.MakeAbsolute();
	return wxFileSystem::FileNameToURL(fn);
}

static bool IsIdentChar(int c) {
	return c > 0 && c < 128 && (std::isalnum(c) || c == '_');
}

static bool StartsWithNoCase(const std::string &s, const std::string &prefix) {
	if (prefix.size() > s.size())
		return false;
	for (size_t i = 0; i < prefix.size(); ++i)
		if (std::tolower((unsigned char)s[i]) !=
			std::tolower((unsigned char)prefix[i]))
			return false;
	return true;
}

LspController::LspController(TextCtrl *textCtrl,
							 languagePreferencesStruct languagePreferences)
	: m_textCtrl(textCtrl),
	  m_languagePreferences(std::move(languagePreferences)),
	  m_syncTimer(this, ID_SYNC_TIMER),
	  m_completionTimer(this, ID_COMPLETION_TIMER),
	  m_restartTimer(this, ID_RESTART_TIMER) {
	m_stc = dynamic_cast<wxStyledTextCtrl *>(textCtrl);

	Bind(wxEVT_TIMER, &LspController::OnSyncTimer, this, ID_SYNC_TIMER);
	Bind(wxEVT_TIMER, &LspController::OnCompletionTimer, this,
		 ID_COMPLETION_TIMER);
	Bind(wxEVT_TIMER, &LspController::OnRestartTimer, this, ID_RESTART_TIMER);

	if (m_stc) {
		m_stc->SetModEventMask(m_stc->GetModEventMask() | wxSTC_MOD_INSERTTEXT |
							   wxSTC_MOD_DELETETEXT);
		m_stc->Bind(wxEVT_STC_MODIFIED, &LspController::OnTextModified, this);
		m_stc->Bind(wxEVT_STC_CHARADDED, &LspController::OnCharAdd, this);
	}

	CreateLspClientInstance();
}

LspController::~LspController() { Stop(); }

LspController *
LspController::Create(TextCtrl *textCtrl,
					  languagePreferencesStruct languagePreferences) {
	if (textCtrl && !languagePreferences.name.empty())
		return new LspController(textCtrl, std::move(languagePreferences));
	return nullptr;
}

void LspController::SetAutocompleteController(AutoCompleteController *c) {
	if (c)
		m_autocompleteController = c;
}

bool LspController::Alive() const {
	return m_textCtrl && m_stc && !m_textCtrl->m_isDestroyed;
}

void LspController::CreateLspClientInstance() {
	if (m_starting || !Alive())
		return;
	if (m_lsp && m_lsp->IsRunning())
		return;

	std::string serverName;
	if (m_languagePreferences.preferences.contains("lsp") &&
		m_languagePreferences.preferences["lsp"].contains("server") &&
		m_languagePreferences.preferences["lsp"]["server"].contains("name")) {
		auto &f = m_languagePreferences.preferences["lsp"]["server"]["name"];
		if (f.is_string())
			serverName = f.get<std::string>();
	}
	if (serverName.empty())
		return;

	wxString serverPath = LspManager::Get().GetServerPath(serverName);
	if (serverPath.empty())
		return;

	const wxString filePath = m_stc->GetName();
	if (filePath.empty())
		return;

	m_lsp.reset();
	m_lsp = std::make_shared<LspClient>();
	m_starting = true;
	m_ready = false;
	m_pending.clear();
	m_fullSyncPending = false;

	m_lsp->SetOnConnectionLost([this]() {
		if (!Alive())
			return;
		m_ready = false;
		m_starting = false;
		m_pending.clear();
		if (++m_restartCount <= 5)
			m_restartTimer.StartOnce(500 * m_restartCount);
	});

	wxString extraArgs;
	if (serverName == "clangd") {
		wxString projectPath = ProjectSettings::Get().GetProjectPath();
		wxString ccDir = projectPath;
		if (!wxFileName(projectPath, "compile_commands.json").FileExists()) {
			wxFileName build = wxFileName::DirName(projectPath);
			build.AppendDir("build");
			if (wxFileName(build.GetPath(), "compile_commands.json")
					.FileExists())
				ccDir = build.GetPath();
		}
		extraArgs = "--compile-commands-dir=\"" + ccDir + "\"" +
					" --background-index"
					" --header-insertion=never"
					" --completion-style=bundled"
					" --limit-results=300"
					" --pch-storage=memory"
					" --log=error";
		m_languageId = filePath.Lower().EndsWith(".c") ? "c" : "cpp";
	} else if (serverName == "pylsp" || serverName == "python-lsp-server") {
		m_languageId = "python";
	} else {
		m_languageId = "plaintext";
	}

	if (!m_lsp->Start(serverPath, extraArgs)) {
		m_lsp.reset();
		m_starting = false;
		return;
	}

	m_uri = PathToUri(filePath, false);
	const wxString root =
		PathToUri(ProjectSettings::Get().GetProjectPath(), true);

	m_lsp->Initialize(root, [this]() {
		if (!Alive())
			return;
		if (!m_lsp || !m_lsp->IsInitialized()) {
			m_starting = false;
			return;
		}
		m_pending.clear();
		m_fullSyncPending = false;
		m_version = 1;
		m_lsp->DidOpen(m_uri, m_languageId, m_stc->GetText(), m_version);
		m_ready = true;
		m_starting = false;
	});
}

void LspController::OnRestartTimer(wxTimerEvent &) {
	if (Alive())
		CreateLspClientInstance();
}

void LspController::Restart() {
	if (!Alive() || !m_ready)
		return;
	m_fullSyncPending = true;
	FlushSync();
}

void LspController::NotifySaved() {
	if (!Alive() || !m_ready)
		return;
	FlushSync();
	m_lsp->DidSave(m_uri);
}

void LspController::Stop() {
	if (m_syncTimer.IsRunning())
		m_syncTimer.Stop();

	if (m_completionTimer.IsRunning())
		m_completionTimer.Stop();

	if (m_restartTimer.IsRunning())
		m_restartTimer.Stop();

	if (m_stc) {
		m_stc->Unbind(wxEVT_STC_MODIFIED, &LspController::OnTextModified, this);
		m_stc->Unbind(wxEVT_STC_CHARADDED, &LspController::OnCharAdd, this);
		m_stc = nullptr;
	}

	m_ready = false;

	if (m_lsp) {
		m_lsp->Stop();
		m_lsp.reset();
	}
}

void LspController::LspPosition(int pos, int &line, int &utf16Col) const {
	line = m_stc->LineFromPosition(pos);
	const int lineStart = m_stc->PositionFromLine(line);
	utf16Col = 0;
	if (pos > lineStart) {
		wxCharBuffer buf = m_stc->GetTextRangeRaw(lineStart, pos);
		if (buf.data())
			utf16Col = Utf16Len(buf.data(), (size_t)(pos - lineStart));
	}
}

void LspController::OnTextModified(wxStyledTextEvent &event) {
	event.Skip();
	if (!Alive() || !m_ready)
		return;

	const int type = event.GetModificationType();
	const bool ins = (type & wxSTC_MOD_INSERTTEXT) != 0;
	const bool del = (type & wxSTC_MOD_DELETETEXT) != 0;
	if (!ins && !del)
		return;

	if (m_fullSyncPending) {
		ScheduleSync();
		return;
	}

	const int pos = event.GetPosition();
	const int len = event.GetLength();
	const std::string text = ToUtf8(event.GetText());

	if (len > FULL_SYNC_BYTES || (int)m_pending.size() >= MAX_PENDING_CHANGES ||
		(int)text.size() != len) {
		m_fullSyncPending = true;
		m_pending.clear();
		ScheduleSync();
		return;
	}

	LspTextChange ch;
	int line, col;
	LspPosition(pos, line, col);
	ch.startLine = line;
	ch.startChar = col;

	if (ins) {
		ch.endLine = line;
		ch.endChar = col;
		ch.text = text;
	} else {
		const int nl = (int)std::count(text.begin(), text.end(), '\n');
		ch.endLine = line + nl;
		if (nl == 0) {
			ch.endChar = col + Utf16Len(text.data(), text.size());
		} else {
			const size_t last = text.rfind('\n') + 1;
			ch.endChar = Utf16Len(text.data() + last, text.size() - last);
		}
	}

	m_pending.push_back(std::move(ch));
	ScheduleSync();
}

void LspController::ScheduleSync() {
	if (!m_syncTimer.IsRunning())
		m_syncTimer.StartOnce(SYNC_DELAY_MS);
}

void LspController::OnSyncTimer(wxTimerEvent &) { FlushSync(); }

void LspController::FlushSync() {
	m_syncTimer.Stop();
	if (!Alive() || !m_ready || !m_lsp || !m_lsp->IsRunning())
		return;

	if (m_fullSyncPending) {
		m_fullSyncPending = false;
		m_pending.clear();
		m_lsp->DidChange(m_uri, m_stc->GetText(), ++m_version);
	} else if (!m_pending.empty()) {
		m_lsp->DidChange(m_uri, m_pending, ++m_version);
		m_pending.clear();
	}
}

void LspController::OnCharAdd(wxStyledTextEvent &event) {
	event.Skip();
	if (!Alive())
		return;

	const int key = event.GetKey();
	if (key <= 0 || key > 127)
		return;
	const char c = (char)key;
	const int pos = m_stc->GetCurrentPos();

	int kind = 1;
	std::string trig;

	if (IsIdentChar(c)) {
		const int start = m_stc->WordStartPosition(pos, true);
		if (std::isdigit(m_stc->GetCharAt(start)))
			return;
		if (m_stc->AutoCompActive() && !m_lastIncomplete)
			return;
	} else if (c == '.') {
		kind = 2;
		trig = ".";
	} else if (c == '>' && pos >= 2 && m_stc->GetCharAt(pos - 2) == '-') {
		kind = 2;
		trig = ">";
	} else if (c == ':' && pos >= 2 && m_stc->GetCharAt(pos - 2) == ':') {
		kind = 2;
		trig = ":";
	} else if (c == '<' || c == '"' || c == '/') {
		wxString ln = m_stc->GetLine(m_stc->LineFromPosition(pos)).Trim(false);
		ln.Replace(" ", "");
		if (!ln.StartsWith("#include"))
			return;
		kind = 2;
		trig = std::string(1, c);
	} else {
		return;
	}

	m_trigKind = kind;
	m_trigChar = trig;
	m_completionTimer.StartOnce(COMPLETION_DELAY_MS);
}

void LspController::OnCompletionTimer(wxTimerEvent &) {
	RequestCompletionNow(m_trigKind, m_trigChar);
}

void LspController::TriggerCompletion() { RequestCompletionNow(1, ""); }

void LspController::RequestCompletionNow(int triggerKind,
										 const std::string &triggerChar) {
	if (!Alive() || !m_autocompleteController)
		return;

	const int pos = m_stc->GetCurrentPos();
	const int wordStart = m_stc->WordStartPosition(pos, true);

	if (!m_ready || !m_lsp || !m_lsp->IsRunning()) {
		ShowFallback(ToUtf8(m_stc->GetTextRange(wordStart, pos)),
					 pos - wordStart);
		return;
	}

	FlushSync();

	int line, col;
	LspPosition(pos, line, col);

	const uint64_t serial = ++m_completionSerial;
	m_lsp->RequestCompletion(
		m_uri, line, col,
		[this, serial, wordStart, line](const std::string &raw) {
			OnCompletionResponse(serial, wordStart, line, raw);
		},
		triggerKind, triggerChar);
}

struct CompletionEntry {
	std::string name;
	std::string sortKey;
};

static std::string StrField(const json &o, const char *key) {
	auto it = o.find(key);
	return (it != o.end() && it->is_string()) ? it->get<std::string>()
											  : std::string();
}

static std::string CleanLabel(std::string s) {
	size_t i = 0;
	while (i < s.size() &&
		   (s[i] == ' ' || (unsigned char)s[i] >= 0x80 /* • etc. */))
		++i;
	s.erase(0, i);
	size_t cut = s.find_first_of("( <");
	if (cut != std::string::npos)
		s.erase(cut);
	return s;
}

static std::vector<CompletionEntry> ParseCompletion(const std::string &raw,
													const std::string &prefix,
													bool &incomplete) {
	std::vector<CompletionEntry> out;
	incomplete = false;

	json j = json::parse(raw, nullptr, false);
	if (j.is_discarded() || !j.is_object() || !j.contains("result") ||
		j["result"].is_null())
		return out;

	const json &r = j["result"];
	const json *items = nullptr;
	if (r.is_array())
		items = &r;
	else if (r.is_object() && r.contains("items") && r["items"].is_array()) {
		items = &r["items"];
		incomplete = r.value("isIncomplete", false);
	}
	if (!items)
		return out;

	std::set<std::string> seen;
	for (const auto &it : *items) {
		if (!it.is_object())
			continue;
		std::string name = StrField(it, "filterText");
		if (name.empty())
			name = CleanLabel(StrField(it, "label"));
		if (name.empty() || name.find(' ') != std::string::npos)
			continue;
		if (!prefix.empty() && !StartsWithNoCase(name, prefix))
			continue;
		if (!seen.insert(name).second)
			continue;

		std::string sk = StrField(it, "sortText");
		out.push_back({name, sk.empty() ? name : sk});
	}

	std::stable_sort(out.begin(), out.end(),
					 [](const CompletionEntry &a, const CompletionEntry &b) {
						 return a.sortKey < b.sortKey;
					 });
	if (out.size() > MAX_ITEMS)
		out.resize(MAX_ITEMS);
	return out;
}

void LspController::OnCompletionResponse(uint64_t serial, int wordStart,
										 int line, const std::string &raw) {
	if (!Alive() || serial != m_completionSerial || !m_autocompleteController)
		return;

	const int curPos = m_stc->GetCurrentPos();
	if (m_stc->WordStartPosition(curPos, true) != wordStart ||
		m_stc->LineFromPosition(curPos) != line)
		return;

	const int curLen = curPos - wordStart;
	const std::string prefix = ToUtf8(m_stc->GetTextRange(wordStart, curPos));

	bool incomplete = false;
	auto items = ParseCompletion(raw, prefix, incomplete);
	m_lastIncomplete = incomplete;

	if (!items.empty())
		m_restartCount = 0;

	if (items.empty()) {
		ShowFallback(prefix, curLen);
		return;
	}

	std::string joined;
	for (const auto &e : items) {
		if (!joined.empty())
			joined += AC_ITEM_SEPARATOR;
		joined += e.name;
	}
	m_autocompleteController->ShowPopUp(wxString::FromUTF8(joined.c_str()),
										curLen);
}

void LspController::ShowFallback(const std::string &prefix, int curLen) {
	if (!m_autocompleteController || curLen < 2)
		return;

	std::set<std::string> words;
	const std::string text = ToUtf8(m_stc->GetText());
	size_t i = 0;
	while (i < text.size() && words.size() < MAX_ITEMS) {
		unsigned char ch = (unsigned char)text[i];
		if (std::isalpha(ch) || ch == '_') {
			size_t j = i + 1;
			while (j < text.size() && IsIdentChar((unsigned char)text[j]))
				++j;
			std::string w = text.substr(i, j - i);
			if (w.size() > prefix.size() && StartsWithNoCase(w, prefix))
				words.insert(w);
			i = j;
		} else {
			++i;
		}
	}

	if (!words.empty()) {
		std::string joined;
		for (const auto &w : words) {
			if (!joined.empty())
				joined += AC_ITEM_SEPARATOR;
			joined += w;
		}
		m_autocompleteController->ShowPopUp(wxString::FromUTF8(joined.c_str()),
											curLen);
		return;
	}

	m_autocompleteController->ShowPopUp(
		m_autocompleteController->RequestStandardWordsList(
			wxString::FromUTF8(prefix.c_str())),
		curLen);
}