#include "lspClient.hpp"

#include <csignal>
#include <cstdlib>
#include <nlohmann/json.hpp>
#include <wx/app.h>
#include <wx/process.h>
#include <wx/utils.h>

using json = nlohmann::json;

static const size_t MAX_MESSAGE_SIZE = 64u * 1024u * 1024u;

class LspProcess : public wxProcess {
  public:
	LspProcess() : wxProcess(wxPROCESS_REDIRECT) {}

	void OnTerminate(int, int) override {
		m_terminated = true;
		if (m_orphan)
			delete this;
	}
	void Orphan() {
		if (m_terminated)
			delete this;
		else
			m_orphan = true;
	}

  private:
	bool m_terminated = false;
	bool m_orphan = false;
};

static std::string Dump(const json &j) {
	return j.dump(-1, ' ', false, json::error_handler_t::replace);
}

static json Pos(int line, int ch) {
	return json{{"line", line}, {"character", ch}};
}

static json TextDoc(const wxString &uri) {
	return json{{"uri", std::string(uri.utf8_str())}};
}

LspClient::LspClient() = default;
LspClient::~LspClient() { Stop(); }

bool LspClient::Start(const wxString &serverPath, const wxString &extraArgs) {
	if (m_running.load() || m_process)
		return false;
	if (!wxFileExists(serverPath))
		return false;

	std::signal(SIGPIPE, SIG_IGN);

	auto *proc = new LspProcess();
	wxString cmd = "\"" + serverPath + "\"";
	if (!extraArgs.IsEmpty())
		cmd += " " + extraArgs;

	long pid = wxExecute(cmd, wxEXEC_ASYNC, proc);
	if (pid <= 0) {
		delete proc;
		return false;
	}

	if (!proc->GetInputStream()) {
		wxProcess::Kill(pid, wxSIGKILL);
		proc->Orphan();
		return false;
	}

	m_self = weak_from_this();
	m_process = proc;
	m_pid = pid;
	{
		std::lock_guard<std::mutex> lk(m_outMutex);
		m_stopWriter = false;
		m_out.clear();
	}
	m_running.store(true);
	m_reader = std::thread([this] { ReaderLoop(); });
	m_writer = std::thread([this] { WriterLoop(); });
	return true;
}

void LspClient::Stop() {
	if (!m_process && !m_reader.joinable() && !m_writer.joinable())
		return;

	m_running.store(false);
	m_initialized.store(false);
	m_onConnectionLost = nullptr;

	{
		std::lock_guard<std::mutex> lk(m_pendingMutex);
		m_pending.clear();
	}
	{
		std::lock_guard<std::mutex> lk(m_handlerMutex);
		m_notificationHandlers.clear();
		m_onDiagnosticsReady.clear();
	}
	{
		std::lock_guard<std::mutex> lk(m_outMutex);
		m_stopWriter = true;
		m_out.clear();
	}
	m_outCv.notify_all();

	if (m_pid > 0 && wxProcess::Exists(m_pid))
		wxProcess::Kill(m_pid, wxSIGKILL);

	if (m_writer.joinable())
		m_writer.join();
	if (m_reader.joinable())
		m_reader.join();

	if (m_process) {
		m_process->Orphan();
		m_process = nullptr;
	}
	m_pid = 0;
}

void LspClient::OnConnectionLost() {
	if (!m_running.exchange(false))
		return;
	m_initialized.store(false);
	{
		std::lock_guard<std::mutex> lk(m_pendingMutex);
		m_pending.clear();
	}
	if (m_onConnectionLost) {
		auto cb = m_onConnectionLost;
		cb();
	}
}

void LspClient::ReaderLoop() {
	std::vector<char> buf(64 * 1024);
	std::string acc;
	wxInputStream *out = m_process->GetInputStream();
	wxInputStream *err = m_process->GetErrorStream();

	auto notifyLost = [this] {
		if (!m_running.load())
			return;
		auto weak = m_self;
		wxTheApp->CallAfter([weak] {
			if (auto c = weak.lock())
				c->OnConnectionLost();
		});
	};

	while (m_running.load()) {
		bool any = false;

		if (err && err->CanRead()) {
			err->Read(buf.data(), buf.size());
			if (err->LastRead() > 0)
				any = true;
		}

		if (out->CanRead()) {
			out->Read(buf.data(), buf.size());
			size_t n = out->LastRead();
			if (n > 0) {
				any = true;
				acc.append(buf.data(), n);

				std::vector<std::string> frames;
				size_t pos = 0;
				while (true) {
					size_t he = acc.find("\r\n\r\n", pos);
					if (he == std::string::npos)
						break;
					std::string head = acc.substr(pos, he - pos);
					for (auto &ch : head)
						ch = (char)std::tolower((unsigned char)ch);
					long len = -1;
					size_t c = head.find("content-length:");
					if (c != std::string::npos)
						len = std::strtol(head.c_str() + c + 15, nullptr, 10);
					if (len <= 0 || (size_t)len > MAX_MESSAGE_SIZE) {
						pos = he + 4;
						continue;
					}
					size_t bodyStart = he + 4;
					if (acc.size() < bodyStart + (size_t)len)
						break;
					frames.emplace_back(acc, bodyStart, (size_t)len);
					pos = bodyStart + (size_t)len;
				}
				acc.erase(0, pos);
				if (acc.size() > MAX_MESSAGE_SIZE)
					acc.clear();

				if (!frames.empty()) {
					auto weak = m_self;
					wxTheApp->CallAfter([weak, frames = std::move(frames)] {
						if (auto c = weak.lock())
							for (const auto &f : frames)
								c->OnDataReceived(f);
					});
				}
			} else if (out->Eof()) {
				notifyLost();
				return;
			}
		}

		if (!any)
			wxMilliSleep(5);
	}
}

void LspClient::WriterLoop() {
	while (true) {
		std::string msg;
		{
			std::unique_lock<std::mutex> lk(m_outMutex);
			m_outCv.wait(lk, [this] { return m_stopWriter || !m_out.empty(); });
			if (m_stopWriter)
				return;
			msg = std::move(m_out.front());
			m_out.pop_front();
		}

		wxOutputStream *os = m_process ? m_process->GetOutputStream() : nullptr;
		if (!os)
			continue;

		size_t written = 0;
		while (written < msg.size() && m_running.load()) {
			os->Write(msg.data() + written, msg.size() - written);
			size_t n = os->LastWrite();
			if (n == 0) {
				if (!os->IsOk())
					break;
				wxMilliSleep(1);
				continue;
			}
			written += n;
		}
	}
}

void LspClient::Send(const std::string &body) {
	if (!m_running.load())
		return;
	{
		std::lock_guard<std::mutex> lk(m_outMutex);
		if (m_stopWriter)
			return;
		m_out.push_back(Frame(body));
	}
	m_outCv.notify_one();
}

int LspClient::SendRequest(const char *method, const json &params,
						   LspResponseCallback cb) {
	int id = NextId();
	if (cb) {
		std::lock_guard<std::mutex> lk(m_pendingMutex);
		m_pending[id] = std::move(cb);
	}
	json msg = {
		{"jsonrpc", "2.0"}, {"id", id}, {"method", method}, {"params", params}};
	Send(Dump(msg));
	return id;
}

void LspClient::SendNotification(const char *method, const json &params) {
	json msg = {{"jsonrpc", "2.0"}, {"method", method}, {"params", params}};
	Send(Dump(msg));
}

void LspClient::CancelRequest(int id) {
	{
		std::lock_guard<std::mutex> lk(m_pendingMutex);
		m_pending.erase(id);
	}
	SendNotification("$/cancelRequest", json{{"id", id}});
}

bool LspClient::NotReady(const LspResponseCallback &cb) {
	if (m_running.load() && m_initialized.load())
		return false;
	if (cb)
		cb("{}");
	return true;
}

void LspClient::Initialize(const wxString &rootUri,
						   std::function<void()> onReady) {
	if (!m_running.load()) {
		if (onReady)
			onReady();
		return;
	}

	std::string root(rootUri.utf8_str());

	json caps = {
		{"general", {{"positionEncodings", json::array({"utf-16"})}}},
		{"window", {{"workDoneProgress", true}}},
		{"workspace", {{"workspaceFolders", true}, {"configuration", false}}},
		{"textDocument",
		 {{"synchronization",
		   {{"didSave", true}, {"dynamicRegistration", false}}},
		  {"completion",
		   {{"contextSupport", true},
			{"completionItem",
			 {{"snippetSupport", false},
			  {"documentationFormat", json::array({"plaintext"})},
			  {"labelDetailsSupport", true}}}}},
		  {"hover", {{"contentFormat", json::array({"plaintext"})}}},
		  {"publishDiagnostics", json::object()}}}};

	json params = {{"processId", (int)wxGetProcessId()},
				   {"clientInfo", {{"name", "KraftaEditor"}}},
				   {"rootUri", root},
				   {"workspaceFolders",
					json::array({{{"uri", root}, {"name", "workspace"}}})},
				   {"capabilities", caps}};

	SendRequest("initialize", params,
				[this, onReady](const std::string &response) {
					json j = json::parse(response, nullptr, false);
					bool ok = !j.is_discarded() && j.is_object() &&
							  j.contains("result") && !j.contains("error");
					if (ok) {
						SendInitialized();
						m_initialized.store(true);
					}
					if (onReady)
						onReady();
				});
}

void LspClient::SendInitialized() {
	SendNotification("initialized", json::object());
}

void LspClient::DidOpen(const wxString &fileUri, const wxString &languageId,
						const wxString &content, int version) {
	if (!m_running.load() || !m_initialized.load())
		return;
	SendNotification("textDocument/didOpen",
					 {{"textDocument",
					   {{"uri", std::string(fileUri.utf8_str())},
						{"languageId", std::string(languageId.utf8_str())},
						{"version", version},
						{"text", std::string(content.utf8_str())}}}});
}

void LspClient::DidChange(const wxString &fileUri, const wxString &newContent,
						  int version) {
	if (!m_running.load() || !m_initialized.load())
		return;
	SendNotification(
		"textDocument/didChange",
		{{"textDocument",
		  {{"uri", std::string(fileUri.utf8_str())}, {"version", version}}},
		 {"contentChanges",
		  json::array({{{"text", std::string(newContent.utf8_str())}}})}});
}

void LspClient::DidChange(const wxString &fileUri,
						  const std::vector<LspTextChange> &changes,
						  int version) {
	if (!m_running.load() || !m_initialized.load() || changes.empty())
		return;

	json arr = json::array();
	for (const auto &c : changes) {
		arr.push_back(json{{"range",
							{{"start", Pos(c.startLine, c.startChar)},
							 {"end", Pos(c.endLine, c.endChar)}}},
						   {"text", c.text}});
	}
	SendNotification(
		"textDocument/didChange",
		{{"textDocument",
		  {{"uri", std::string(fileUri.utf8_str())}, {"version", version}}},
		 {"contentChanges", arr}});
}

void LspClient::DidClose(const wxString &fileUri) {
	if (!m_running.load() || !m_initialized.load())
		return;
	SendNotification("textDocument/didClose",
					 {{"textDocument", TextDoc(fileUri)}});
}

void LspClient::DidSave(const wxString &fileUri) {
	if (!m_running.load() || !m_initialized.load())
		return;
	SendNotification("textDocument/didSave",
					 {{"textDocument", TextDoc(fileUri)}});
}

void LspClient::RequestPositional(const char *method, const wxString &uri,
								  int line, int col, LspResponseCallback cb) {
	if (NotReady(cb))
		return;
	SendRequest(method,
				{{"textDocument", TextDoc(uri)}, {"position", Pos(line, col)}},
				std::move(cb));
}

void LspClient::RequestCompletion(const wxString &fileUri, int line, int col,
								  LspResponseCallback cb, int triggerKind,
								  const std::string &triggerChar) {
	if (NotReady(cb))
		return;

	if (m_lastCompletionId != 0)
		CancelRequest(m_lastCompletionId);

	json ctx = {{"triggerKind", triggerKind}};
	if (triggerKind == 2 && !triggerChar.empty())
		ctx["triggerCharacter"] = triggerChar;

	json params = {{"textDocument", TextDoc(fileUri)},
				   {"position", Pos(line, col)},
				   {"context", ctx}};

	m_lastCompletionId =
		SendRequest("textDocument/completion", params, std::move(cb));
}

void LspClient::RequestHover(const wxString &u, int l, int c,
							 LspResponseCallback cb) {
	RequestPositional("textDocument/hover", u, l, c, std::move(cb));
}

void LspClient::RequestDefinition(const wxString &u, int l, int c,
								  LspResponseCallback cb) {
	RequestPositional("textDocument/definition", u, l, c, std::move(cb));
}

void LspClient::RequestReferences(const wxString &u, int l, int c,
								  LspResponseCallback cb) {
	if (NotReady(cb))
		return;
	SendRequest("textDocument/references",
				{{"textDocument", TextDoc(u)},
				 {"position", Pos(l, c)},
				 {"context", {{"includeDeclaration", true}}}},
				std::move(cb));
}

void LspClient::RequestDocumentSymbols(const wxString &u,
									   LspResponseCallback cb) {
	if (NotReady(cb))
		return;
	SendRequest("textDocument/documentSymbol", {{"textDocument", TextDoc(u)}},
				std::move(cb));
}

void LspClient::RequestFormatting(const wxString &u, LspResponseCallback cb) {
	if (NotReady(cb))
		return;
	SendRequest("textDocument/formatting",
				{{"textDocument", TextDoc(u)},
				 {"options", {{"tabSize", 4}, {"insertSpaces", true}}}},
				std::move(cb));
}

void LspClient::RequestRename(const wxString &u, int l, int c,
							  const wxString &newName, LspResponseCallback cb) {
	if (NotReady(cb))
		return;
	SendRequest("textDocument/rename",
				{{"textDocument", TextDoc(u)},
				 {"position", Pos(l, c)},
				 {"newName", std::string(newName.utf8_str())}},
				std::move(cb));
}

void LspClient::OnNotification(const wxString &method,
							   LspNotificationCallback cb) {
	std::lock_guard<std::mutex> lk(m_handlerMutex);
	m_notificationHandlers[std::string(method.utf8_str())] = std::move(cb);
}

void LspClient::RemoveNotificationHandler(const wxString &method) {
	std::lock_guard<std::mutex> lk(m_handlerMutex);
	m_notificationHandlers.erase(std::string(method.utf8_str()));
}

void LspClient::OnDiagnosticsReady(const wxString &fileUri,
								   std::function<void()> cb) {
	std::lock_guard<std::mutex> lk(m_handlerMutex);
	m_onDiagnosticsReady[std::string(fileUri.utf8_str())].push_back(
		std::move(cb));
}

void LspClient::OnDataReceived(const std::string &raw) {
	if (!m_running.load())
		return;

	json j = json::parse(raw, nullptr, false);
	if (j.is_discarded() || !j.is_object())
		return;

	const bool hasId = j.contains("id") && !j["id"].is_null();
	const bool hasMethod = j.contains("method") && j["method"].is_string();

	if (hasMethod && hasId) {
		json result = nullptr;
		if (j["method"] == "workspace/configuration" && j.contains("params") &&
			j["params"].contains("items"))
			result = json(j["params"]["items"].size(), json(nullptr));
		json reply = {{"jsonrpc", "2.0"}, {"id", j["id"]}, {"result", result}};
		Send(Dump(reply));
		return;
	}

	if (hasMethod) {
		const std::string method = j["method"].get<std::string>();
		const json params = j.contains("params") ? j["params"] : json::object();

		if (method == "textDocument/publishDiagnostics" &&
			params.contains("uri") && params["uri"].is_string()) {
			std::vector<std::function<void()>> cbs;
			{
				std::lock_guard<std::mutex> lk(m_handlerMutex);
				auto it =
					m_onDiagnosticsReady.find(params["uri"].get<std::string>());
				if (it != m_onDiagnosticsReady.end()) {
					cbs = std::move(it->second);
					m_onDiagnosticsReady.erase(it);
				}
			}
			for (auto &cb : cbs)
				if (cb)
					cb();
		}

		LspNotificationCallback handler;
		{
			std::lock_guard<std::mutex> lk(m_handlerMutex);
			auto it = m_notificationHandlers.find(method);
			if (it != m_notificationHandlers.end())
				handler = it->second;
		}
		if (handler)
			handler(method, Dump(params));
		return;
	}

	if (hasId && j["id"].is_number_integer()) {
		int id = j["id"].get<int>();
		LspResponseCallback cb;
		{
			std::lock_guard<std::mutex> lk(m_pendingMutex);
			auto it = m_pending.find(id);
			if (it == m_pending.end())
				return;
			cb = std::move(it->second);
			m_pending.erase(it);
		}
		if (id == m_lastCompletionId)
			m_lastCompletionId = 0;
		if (cb)
			cb(raw);
	}
}

std::string LspClient::Frame(const std::string &body) {
	return "Content-Length: " + std::to_string(body.size()) + "\r\n\r\n" + body;
}

int LspClient::NextId() {
	static std::atomic<int> g_nextId{1};
	return g_nextId.fetch_add(1);
}