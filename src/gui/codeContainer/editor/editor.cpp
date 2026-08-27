#include "./editor.hpp"

#include "appConstants/appConstants.hpp"
#include "gui/codeContainer/code.hpp"
#include "lsp/lspClient/lspClient.hpp"
#include "lsp/lspManager/lspManager.hpp"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <unordered_set>

static wxCriticalSection g_editorLock;

Editor::Editor(wxWindow *parent)
	: wxStyledTextCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
					   wxBORDER_NONE),
	  m_lspSyncTimer(this, LSP_SYNC_TIMER_ID),
	  m_lspDebounceTimer(this, LSP_DEBOUNCE_ID), m_isDestroyed(false) {

	m_linked_container = dynamic_cast<CodeContainer *>(parent);
	InitializePreferences();
	ConfigureFoldMargin();
	SetupAutoComplete();

	Bind(wxEVT_STC_CHANGE, &Editor::OnChange, this);
	Bind(wxEVT_STC_CHARADDED, &Editor::CharAdd, this);
	Bind(wxEVT_LEFT_DOWN, &Editor::OnClick, this);
	Bind(wxEVT_STC_UPDATEUI, &Editor::OnUpdateUI, this);
	Bind(wxEVT_TIMER, &Editor::OnLspDebounceTimer, this, LSP_DEBOUNCE_ID);
}

Editor::~Editor() {
	m_isDestroyed = true;

	Unbind(wxEVT_STC_CHANGE, &Editor::OnChange, this);
	Unbind(wxEVT_STC_CHARADDED, &Editor::CharAdd, this);
	Unbind(wxEVT_LEFT_DOWN, &Editor::OnClick, this);
	Unbind(wxEVT_STC_UPDATEUI, &Editor::OnUpdateUI, this);

	m_lspSyncTimer.Stop();
	m_lspDebounceTimer.Stop();

	if (m_lsp) {
		m_lsp->Stop();
		m_lsp.reset();
	}
}

void Editor::OnLspSyncTimer(wxTimerEvent &) {
	if (m_isDestroyed)
		return;
	if (m_lsp && m_lsp->IsRunning() && m_lspReady && m_documentOpened &&
		!m_lspDebounceTimer.IsRunning()) {
		wxString uri = "file://" + GetName();
		wxString text = GetText();
		m_lsp->DidChange(uri, text, ++m_docVersion);
	}
}

void Editor::InitializePreferences() {
	const wxString backgroundColor(
		Theme["secondary"].template get<std::string>());
	const wxString textColor(Theme["text"].template get<std::string>());
	const wxString secondaryTextColor(
		Theme["secondaryText"].template get<std::string>());

	SetUseTabs(true);
	SetTabIndents(true);
	SetBackSpaceUnIndents(true);
	SetIndentationGuides(true);
	SetEndAtLastLine(true);
	SetFocus();

	StyleSetBackground(wxSTC_STYLE_DEFAULT, wxColor(backgroundColor));
	StyleSetForeground(wxSTC_STYLE_DEFAULT, wxColor(textColor));
	StyleClearAll();

	SetCaretForeground(ThemesManager::Get().GetColor("editorCaret"));
	SetCaretWidth(3);

	SetMultipleSelection(true);
	SetAdditionalSelectionTyping(true);
	SetMultiPaste(wxSTC_MULTIPASTE_EACH);
	SetVirtualSpaceOptions(wxSTC_VS_RECTANGULARSELECTION);

	SetMarginWidth(EditorConstants::LINE_NUMBER_MARGIN,
				   TextWidth(wxSTC_STYLE_LINENUMBER, wxT("_99999")));
	SetMarginType(EditorConstants::LINE_NUMBER_MARGIN, wxSTC_MARGIN_NUMBER);

	StyleSetForeground(wxSTC_STYLE_LINENUMBER, wxColor(secondaryTextColor));
	StyleSetBackground(wxSTC_STYLE_LINENUMBER, wxColor(backgroundColor));

	StyleSetBackground(wxSTC_STYLE_INDENTGUIDE, wxColor(backgroundColor));
	StyleSetForeground(wxSTC_STYLE_INDENTGUIDE, wxColor(secondaryTextColor));

	AutoCompSetSeparator(' ');
	AutoCompSetIgnoreCase(true);
	AutoCompSetAutoHide(true);
	AutoCompSetDropRestOfWord(false);
	AutoCompSetMaxHeight(8);

	wxAcceleratorEntry entries[] = {
		{wxACCEL_CTRL, WXK_RETURN,
		 static_cast<int>(Event::Edit::MoveCursorDown)},
		{wxACCEL_CTRL | wxACCEL_SHIFT, WXK_RETURN,
		 static_cast<int>(Event::Edit::MoveCursorUp)},
		{wxACCEL_CTRL | wxACCEL_SHIFT, (int)'D',
		 static_cast<int>(Event::Edit::DuplicateLineDown)},
		{wxACCEL_CTRL | wxACCEL_ALT | wxACCEL_SHIFT, (int)'D',
		 static_cast<int>(Event::Edit::DuplicateLineUp)},
		{wxACCEL_CTRL, (int)'D',
		 static_cast<int>(Event::Edit::SelectNextOccurrence)},
		{wxACCEL_CTRL, (int)'C', static_cast<int>(Event::Edit::CopyByKeyboard)},
		{wxACCEL_CTRL | wxACCEL_SHIFT, (int)'+',
		 static_cast<int>(Event::View::ZoomIn)},
		{wxACCEL_CTRL, (int)'-', static_cast<int>(Event::View::ZoomOut)},
		{wxACCEL_CTRL, (int)'/',
		 static_cast<int>(Event::Edit::ToggleLineComment)},
		{wxACCEL_CTRL | wxACCEL_SHIFT, (int)'?',
		 static_cast<int>(Event::Edit::ToggleBlockComment)},
	};

	SetAcceleratorTable(wxAcceleratorTable(WXSIZEOF(entries), entries));

	m_AutoCompleteWordsList =
		LanguagesPreferences::Get().GetAutoCompleteWordsList(
			m_LanguagePreferences);

	SetIndent(4);
	SetTabWidth(4);
	SetUseTabs(false);
	SetIndentationGuides(wxSTC_IV_LOOKBOTH);
	SetEndAtLastLine(false);

	Bind(wxEVT_LEFT_DOWN, [this](wxMouseEvent &event) {
		if (ProjectSettings::Get().GetCurrentlyFileOpen() != this->GetName()) {
			ProjectSettings::Get().SetCurrentlyFileOpen(this->GetName());
		}
		event.Skip();
	});
}

void Editor::Lsp() {
	if (m_lspStarting)
		return;
	if (m_lsp && m_lsp->IsRunning())
		return;

	std::string serverName;
	if (m_LanguagePreferences.preferences.contains("lsp") &&
		m_LanguagePreferences.preferences["lsp"].contains("server") &&
		m_LanguagePreferences.preferences["lsp"]["server"].contains("name")) {
		auto &nameField =
			m_LanguagePreferences.preferences["lsp"]["server"]["name"];
		if (nameField.is_string())
			serverName = nameField.get<std::string>();
	}

	if (serverName.empty())
		return;

	wxString serverPath = LspManager::Get().GetServerPath(serverName);
	if (serverPath.empty())
		return;

	m_lspStarting = true;
	m_lsp.reset();
	m_lsp = std::make_unique<LspClient>();
	m_lspReady = false;
	m_documentOpened = false;

	m_lsp->SetOnConnectionLost([this]() {
		if (m_isDestroyed)
			return;
		m_lspReady = false;
		m_documentOpened = false;
		m_lspStarting = false;
		CallAfter([this]() {
			if (!m_isDestroyed)
				Lsp();
		});
	});

	wxString extraArgs;
	wxString languageId;

	if (serverName == "clangd") {
		wxString projectPath = ProjectSettings::Get().GetProjectPath();

		wxString compileCommandsDir = projectPath;
		wxFileName rootCompileCommands(projectPath, "compile_commands.json");

		if (!rootCompileCommands.FileExists()) {
			wxFileName buildCompileCommands(projectPath + "build",
											"compile_commands.json");
			if (buildCompileCommands.FileExists()) {
				compileCommandsDir = projectPath + "build";
			}
		}

		extraArgs = "--compile-commands-dir=" + compileCommandsDir +
					" --background-index";
		languageId = "cpp";
	} else if (serverName == "pylsp" || serverName == "python-lsp-server") {
		extraArgs = "";
		languageId = "python";
	}

	if (!m_lsp->Start(serverPath, extraArgs)) {
		m_lsp.reset();
		m_lspStarting = false;
		return;
	}

	wxString root = "file://" + ProjectSettings::Get().GetProjectPath();
	wxString uri = "file://" + GetName();

	m_lsp->Initialize(root, [this, uri, languageId]() {
		if (m_isDestroyed) {
			return;
		}
		if (!m_lsp || !m_lsp->IsRunning()) {
			m_lspStarting = false;
			return;
		}

		wxString currentText = GetText();
		m_lsp->DidOpen(uri, languageId, currentText);
		m_documentOpened = true;
		m_lspReady = true;
		m_lspStarting = false;
	});
}

void Editor::CharAdd(wxStyledTextEvent &event) {
	if (m_isDestroyed)
		return;
	const char chr = static_cast<char>(event.GetKey());
	const int pos = GetCurrentPos();
	wxString uri = "file://" + GetName();

	if (std::isalnum(static_cast<unsigned char>(chr)) || chr == '_') {
		const int start = WordStartPosition(pos, true);
		const int len = pos - start;

		if (len >= 2) {
			const wxString word = GetTextRange(start, pos);

			if (m_lsp && m_lsp->IsRunning() && m_lspReady && m_documentOpened) {
				m_lspSyncTimer.Stop();
				m_lspDebounceTimer.Stop();
				m_lspDebounceTimer.StartOnce(300);

				m_lastCompletionLine = LineFromPosition(pos);
				m_lastCompletionCol = GetColumn(pos);
				m_lastCompletionUri = uri;
			} else {
				ShowLocalCompletion(word, len);
			}
		}
	}

	if (chr == '\n')
		OnEnterKey(event);
	HandleAutoPairing(chr);
	event.Skip();
}

void Editor::OnLspDebounceTimer(wxTimerEvent &event) {
	if (m_isDestroyed)
		return;

	if (!m_lsp || !m_lsp->IsRunning() || !m_lspReady || !m_documentOpened)
		return;

	m_lsp->ReleaseStaleCompletion(8000);

	if (m_lsp->HasPendingCompletion())
		return;

	const int curPos = GetCurrentPos();
	const int curStart = WordStartPosition(curPos, true);
	const int curLen = curPos - curStart;

	if (curLen < 2)
		return;

	m_lsp->RequestCompletion(
		m_lastCompletionUri, m_lastCompletionLine, m_lastCompletionCol,
		[this](const std::string &json) {
			if (m_isDestroyed)
				return;
			if (!m_lsp || !m_lsp->IsRunning() || !m_lspReady)
				return;
			if (json.empty() || json == "{}")
				return;

			const int curPos = GetCurrentPos();
			const int curStart = WordStartPosition(curPos, true);
			const int curLen = curPos - curStart;
			if (curLen <= 0)
				return;

			wxString curWord = GetTextRange(curStart, curPos);
			wxString items = ParseCompletionItems(json, curWord);

			if (!items.empty()) {
				AutoCompShow(curLen, items);
			}
		});
}

void Editor::ConfigureFoldMargin() {
	const wxString backgroundColor(
		Theme["secondary"].template get<std::string>());

	SetMarginWidth(EditorConstants::FOLD_MARGIN,
				   EditorConstants::FOLD_MARGIN_WIDTH);
	SetMarginType(EditorConstants::FOLD_MARGIN, wxSTC_MARGIN_SYMBOL);
	SetMarginMask(EditorConstants::FOLD_MARGIN, wxSTC_MASK_FOLDERS);
	SetMarginSensitive(EditorConstants::FOLD_MARGIN, true);

	SetFoldMarginColour(true, wxColor(backgroundColor));
	SetFoldMarginHiColour(true, wxColor(backgroundColor));
	SetFoldFlags(wxSTC_FOLDFLAG_LINEAFTER_CONTRACTED |
				 wxSTC_FOLDFLAG_LINEBEFORE_EXPANDED);
}

void Editor::OnUpdateUI(wxStyledTextEvent &event) { event.Skip(); }

void Editor::OnChange(wxStyledTextEvent &event) {
	if (m_isDestroyed)
		return;
	if (!GetModify()) {
		event.Skip();
		return;
	}

	if (m_lsp && m_lsp->IsRunning() && m_lspReady && m_documentOpened) {
		m_lspSyncTimer.Stop();
		m_lspSyncTimer.StartOnce(300);
	}

	if (UserSettingsManager::Get().GetSetting<bool>("editor/autoSave").value &&
		GetName() != UserSettingsManager::Get().SettingsPath) {
		if (m_linked_container) {
			m_linked_container->Save(GetName());
		}
	} else {
		UpdateUnsavedIndicator();
	}

	ClearIndicators();
	event.Skip();
}

void Editor::OnMarginClick(wxStyledTextEvent &event) {
	if (event.GetMargin() != EditorConstants::FOLD_MARGIN) {
		event.Skip();
		return;
	}

	const int line = LineFromPosition(event.GetPosition());
	const int level = GetFoldLevel(line);

	if (level & wxSTC_FOLDLEVELHEADERFLAG)
		ToggleFold(line);

	event.Skip();
}

void Editor::OnBackspace(wxKeyEvent &event) {
	const int key = event.GetKeyCode();

	if (key != WXK_BACK && key != WXK_DELETE) {
		event.Skip();
		return;
	}

	const long start = GetSelectionStart();
	const long end = GetSelectionEnd();

	if (start != end) {
		Remove(start, end);
		return;
	}

	const long pos = GetCurrentPos();

	if (pos <= 0 && key == WXK_BACK) {
		event.Skip();
		return;
	}

	const wxString prevChar = pos > 0 ? GetTextRange(pos - 1, pos) : "";
	const wxString nextChar =
		pos < GetLength() ? GetTextRange(pos, pos + 1) : "";

	if (key == WXK_BACK) {
		auto it = kPairMap.find(nextChar);
		if (it != kPairMap.end() && it->second == prevChar) {
			Remove(pos - 1, pos + 1);
			return;
		}
	}

	if (key == WXK_DELETE) {
		auto it = kPairMap.find(prevChar);
		if (it != kPairMap.end() && it->second == nextChar) {
			Remove(pos - 1, pos + 1);
			return;
		}
	}

	event.Skip();
}

void Editor::OnArrowsPress(wxKeyEvent &event) {
	ClearIndicators();

	if (statusBar)
		statusBar->UpdateCodeLocale(this);

	event.Skip();
}

void Editor::OnClick(wxMouseEvent &event) {
	HighlightSelectionOccurrences();

	if (statusBar)
		statusBar->UpdateCodeLocale(this);

	event.Skip();
}

void Editor::OnScroll(wxMouseEvent &event) {
	if (event.ShiftDown()) {
		OnHorizontalScroll(event);
	} else {
		event.Skip();
	}
}

bool Editor::Modified() const { return GetModify() && !GetReadOnly(); }

void Editor::HighlightSelectionOccurrences() {
	ClearIndicators();

	const int start = GetSelectionStart();
	const int end = GetSelectionEnd();

	if (end <= start)
		return;

	const wxString text = GetTextRange(start, end);

	if (text.Length() < EditorConstants::MIN_SELECTION_LENGTH ||
		!std::isalnum(static_cast<unsigned char>(text[0])))
		return;

	SetIndicatorCurrent(EditorConstants::INDICATOR_DEFAULT);

	int pos = 0;
	const int max = GetTextLength();

	while (pos < max) {
		int found = FindText(pos, max, text,
							 wxSTC_FIND_MATCHCASE | wxSTC_FIND_WHOLEWORD);
		if (found == -1)
			break;

		if (found != start)
			IndicatorFillRange(found, text.Length());

		pos = found + text.Length();
	}
}

void Editor::ClearIndicators() {
	const int len = GetTextLength();

	for (int i = 0; i <= EditorConstants::MAX_INDICATOR; ++i) {
		IndicatorClearRange(0, len);
	}
}

void Editor::OnCopy(wxCommandEvent &event) { CopyAllowLine(); }

void Editor::UpdateUnsavedIndicator() {
	wxWindow *tab = FindWindowByLabel(
		ProjectSettings::Get().GetCurrentlyFileOpen() + "_tab");

	if (!tab || !Modified())
		return;

	wxStaticBitmap *icon = nullptr;

	for (auto children : tab->GetChildren()[0]->GetChildren()) {
		if (children->GetName() == "tab_icon_close_or_unsaved") {
			icon = wxDynamicCast(children, wxStaticBitmap);
			break;
		}
	}

	icon->SetBitmap(wxBitmapBundle::FromBitmap(wxBitmap(
		iconsDir + "unsaved" +
			(ThemesManager::Get().IsDarkTheme() ? "_light" : "_dark") + ".png",
		wxBITMAP_TYPE_PNG)));

	tab->Layout();
}

void Editor::HandleAutoPairing(char chr) {
	if (GetSelectionStart() != GetSelectionEnd())
		return;

	wxString pair;

	if (chr == '(')
		pair = ")";
	else if (chr == '{')
		pair = "}";
	else if (chr == '[')
		pair = "]";
	else if (chr == '"')
		pair = "\"";
	else if (chr == '\'')
		pair = "'";
	else if (chr == '`')
		pair = "`";

	if (!pair.empty())
		InsertText(GetCurrentPos(), pair);
}

static int FindLastCharBeforePos(wxStyledTextCtrl *ctrl, int ch, int pos) {
	for (int i = pos; i >= 0; --i)
		if (ctrl->GetCharAt(i) == ch)
			return i;

	return -1;
}

static wxString ExtractTagName(wxStyledTextCtrl *ctrl, int openPos,
							   int closePos) {
	wxString tag;

	for (int i = openPos + 1; i < closePos; ++i) {
		const char c = static_cast<char>(ctrl->GetCharAt(i));
		if (c == '/' || std::isspace(static_cast<unsigned char>(c)) || c == '>')
			break;
		tag += c;
	}

	return tag;
}

void Editor::ShowLocalCompletion(const wxString &word, int len) {
	wxString list;
	std::vector<wxString> matches;

	for (const auto &kw : m_autoCompleteWords) {
		if (kw.StartsWith(word)) {
			matches.push_back(kw);
		}
	}

	std::sort(matches.begin(), matches.end());

	for (size_t i = 0; i < matches.size() && i < 30; ++i) {
		if (i > 0)
			list += " ";
		list += matches[i];
	}

	if (!list.empty()) {
		AutoCompShow(len, list);
	} else {
		AutoCompCancel();
	}
}

void Editor::SetupAutoComplete() {
	AutoCompSetSeparator(' ');
	AutoCompSetIgnoreCase(false);
	AutoCompSetAutoHide(true);
	AutoCompSetDropRestOfWord(false);
	AutoCompSetMaxHeight(8);
	AutoCompSetTypeSeparator('?');

	m_autoCompleteWords = LanguagesPreferences::Get().GetAutoCompleteWordsList(
		m_LanguagePreferences);
}

void Editor::OnEnterKey(wxStyledTextEvent &event) {
	wxStyledTextCtrl *stc = this;
	const int curPos = stc->GetCurrentPos();
	const int curLine = stc->GetCurrentLine();
	const int indentSize = stc->GetIndent();

	int baseIndent = 0;
	if (curLine > 0) {
		const int prevLine = curLine - 1;
		baseIndent = stc->GetLineIndentation(prevLine);

		wxString prevText = stc->GetLine(prevLine);
		prevText.Trim(true).Trim(false);

		if (prevText.EndsWith("{")) {
			baseIndent += indentSize;
		}
	}

	if (curPos > 0 && curPos < stc->GetTextLength()) {
		const char prevChar = stc->GetCharAt(curPos - 1);
		const char nextChar = stc->GetCharAt(curPos);

		if (prevChar == '{' || nextChar == '}') {
			stc->BeginUndoAction();

			stc->AddText("\n");
			stc->SetLineIndentation(curLine, baseIndent);
			stc->SetLineIndentation(curLine + 1, baseIndent - indentSize);
			stc->GotoPos(stc->GetLineIndentPosition(curLine));

			stc->EndUndoAction();
			return;
		}
	}

	event.Skip();

	wxString curText = stc->GetLine(curLine);
	curText.Trim(true).Trim(false);

	int finalIndent = baseIndent;
	if (curText.StartsWith("}")) {
		finalIndent = std::max(0, baseIndent - indentSize);
	}

	stc->SetLineIndentation(curLine, finalIndent);
	stc->GotoPos(stc->GetLineIndentPosition(curLine));
}

void Editor::OnMoveCursorDown(wxCommandEvent &WXUNUSED(event)) {
	const int line = GetCurrentLine();
	const int lineEnd = GetLineEndPosition(line);

	BeginUndoAction();

	GotoPos(lineEnd);
	InsertText(lineEnd, "\n");

	EndUndoAction();

	GotoLine(line + 1);
	SetEmptySelection(GetCurrentPos());
	EnsureCaretVisible();
}

void Editor::OnMoveCursorUp(wxCommandEvent &WXUNUSED(event)) {
	const int line = GetCurrentLine();
	const int lineStart = PositionFromLine(line);

	BeginUndoAction();

	InsertText(lineStart, "\n");

	EndUndoAction();

	if (line > 0)
		GotoLine(line);
	else
		GotoPos(0);

	SetEmptySelection(GetCurrentPos());
	EnsureCaretVisible();
}

void Editor::OnDuplicateLineDown(wxCommandEvent &event) {
	const int selStart = GetSelectionStart();
	const int selEnd = GetSelectionEnd();

	if (selStart == selEnd) {
		const int line = GetCurrentLine();
		const int lineStart = PositionFromLine(line);
		const int lineEnd = GetLineEndPosition(line);
		const wxString text = GetTextRange(lineStart, lineEnd);
		BeginUndoAction();
		InsertText(lineEnd, "\n" + text);
		EndUndoAction();
		GotoLine(line + 1);
		SetEmptySelection(GetCurrentPos());
	} else {
		const wxString text = GetTextRange(selStart, selEnd);
		BeginUndoAction();
		InsertText(selEnd, "\n" + text);
		EndUndoAction();
		SetSelection(selEnd, selEnd + text.Length());
	}

	EnsureCaretVisible();
}

void Editor::OnDuplicateLineUp(wxCommandEvent &event) {
	const int line = GetCurrentLine();
	const int lineStart = PositionFromLine(line);
	const int lineEnd = GetLineEndPosition(line);
	const wxString text = GetTextRange(lineStart, lineEnd);

	BeginUndoAction();

	InsertText(lineStart, text + "\n");

	EndUndoAction();

	GotoLine(line);
	SetEmptySelection(GetCurrentPos());
	EnsureCaretVisible();
}

void Editor::SelectNextOccurrence(wxCommandEvent &WXUNUSED(event)) {
	if (GetSelections() == 0)
		return;

	const int mainSel = GetMainSelection();
	const int selStart = GetSelectionNStart(mainSel);
	const int selEnd = GetSelectionNEnd(mainSel);

	if (selStart == selEnd)
		return;

	const wxString text = GetTextRange(selStart, selEnd);

	SetTargetStart(selEnd);
	SetTargetEnd(GetLength());
	SetSearchFlags(wxSTC_FIND_MATCHCASE | wxSTC_FIND_WHOLEWORD);

	if (SearchInTarget(text) == -1)
		return;

	const int foundStart = GetTargetStart();
	const int foundEnd = GetTargetEnd();

	AddSelection(foundStart, foundEnd);
	SetMainSelection(GetSelections() - 1);
	EnsureCaretVisible();
}

void Editor::MoveSelectedLinesUp() {
	int selStart = GetSelectionStart();
	int selEnd = GetSelectionEnd();

	bool hasSelection = (selStart != selEnd);

	int startLine;
	int endLine;

	if (!hasSelection) {
		startLine = endLine = LineFromPosition(GetCurrentPos());
	} else {
		startLine = LineFromPosition(selStart);
		endLine = LineFromPosition(selEnd);

		if (selEnd == PositionFromLine(endLine))
			endLine--;
	}

	if (startLine <= 0)
		return;

	BeginUndoAction();

	wxString textAbove = GetLine(startLine - 1);
	wxString blockText;

	for (int i = startLine; i <= endLine; ++i)
		blockText += GetLine(i);

	int removeStart = PositionFromLine(startLine - 1);
	int removeEnd = PositionFromLine(endLine + 1);

	SetTargetStart(removeStart);
	SetTargetEnd(removeEnd);
	ReplaceTarget(blockText + textAbove);

	int newStartPos = PositionFromLine(startLine - 1);
	int newEndPos = newStartPos + blockText.Length();

	if (hasSelection)
		SetSelection(newStartPos, newEndPos);
	else
		GotoPos(newStartPos);

	EndUndoAction();
}

void Editor::MoveSelectedLinesDown() {
	int selStart = GetSelectionStart();
	int selEnd = GetSelectionEnd();

	bool hasSelection = (selStart != selEnd);

	int startLine;
	int endLine;

	if (!hasSelection) {
		startLine = endLine = LineFromPosition(GetCurrentPos());
	} else {
		startLine = LineFromPosition(selStart);
		endLine = LineFromPosition(selEnd);

		if (selEnd == PositionFromLine(endLine))
			endLine--;
	}

	if (endLine >= GetLineCount() - 1)
		return;

	BeginUndoAction();

	wxString textBelow = GetLine(endLine + 1);
	wxString blockText;

	for (int i = startLine; i <= endLine; ++i)
		blockText += GetLine(i);

	int removeStart = PositionFromLine(startLine);
	int removeEnd = PositionFromLine(endLine + 2);

	SetTargetStart(removeStart);
	SetTargetEnd(removeEnd);
	ReplaceTarget(textBelow + blockText);

	int newStartPos = PositionFromLine(startLine + 1);
	int newEndPos = newStartPos + blockText.Length();

	if (hasSelection)
		SetSelection(newStartPos, newEndPos);
	else
		GotoPos(newStartPos);

	EndUndoAction();
}

void Editor::RemoveCurrentLine() {
	if (GetSelectionStart() != GetSelectionEnd()) {
		ReplaceSelection("");
		return;
	}

	const int currentLine = GetCurrentLine();
	const int lineStart = PositionFromLine(currentLine);
	int lineEnd = GetLineEndPosition(currentLine);

	const int lastLine = GetLineCount() - 1;
	if (currentLine < lastLine) {
		lineEnd++;
	}

	BeginUndoAction();
	SetTargetStart(lineStart);
	SetTargetEnd(lineEnd);
	ReplaceTarget("");
	EndUndoAction();
}

void Editor::OnHorizontalScroll(wxMouseEvent &event) {
	int rotation = event.GetWheelRotation();
	int delta = event.GetWheelDelta();

	if (delta == 0)
		return;

	int steps = rotation / delta;

	constexpr int SCROLL_SPEED = 40;

	int currentOffset = GetXOffset();
	int newOffset = currentOffset - (steps * SCROLL_SPEED);

	if (newOffset < 0)
		newOffset = 0;

	Freeze();
	SetXOffset(newOffset);
	Thaw();
}

void Editor::OnZoomIn(wxCommandEvent &event) { ZoomIn(); }

void Editor::OnZoomOut(wxCommandEvent &event) { ZoomOut(); }

void Editor::OnToggleLineComment(wxCommandEvent &event) {
	int lineStart = 0;
	if (GetSelectionEnd() - GetSelectionStart() <= 0) {
		lineStart = PositionFromLine(GetCurrentLine());
	} else {
		lineStart = GetSelectionStart();
	}

	char chr = (char)GetCharAt(lineStart);

	if (chr == ' ') {
		while (chr == ' ') {
			lineStart++;
			chr = (char)GetCharAt(lineStart);
		}
	}

	if (chr == '/' && (char)GetCharAt(lineStart + 1) == '/') {
		DeleteRange(lineStart, 2);
	} else {
		InsertText(lineStart, "//");
	}
}

void Editor::OnToggleBlockComment(wxCommandEvent &event) {
	int selStart = GetSelectionStart();
	int selEnd = GetSelectionEnd();
	if (selStart > selEnd)
		std::swap(selStart, selEnd);

	if (selStart == selEnd) {
		const int line = GetCurrentLine();
		selStart = PositionFromLine(line);
		selEnd = GetLineEndPosition(line);
	}

	auto isSpace = [&](int ch) {
		return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n';
	};

	int s = selStart;
	int e = selEnd;
	while (s < e && isSpace(GetCharAt(s)))
		s++;
	while (e > s && isSpace(GetCharAt(e - 1)))
		e--;

	const bool hasOpen =
		(e - s >= 2) && GetCharAt(s) == '/' && GetCharAt(s + 1) == '*';
	const bool hasClose =
		(e - s >= 4) && GetCharAt(e - 2) == '*' && GetCharAt(e - 1) == '/';

	auto unwrap = [&](wxStyledTextCtrl *ctrl) {
		ctrl->BeginUndoAction();
		ctrl->DeleteRange(e - 2, 2);
		ctrl->DeleteRange(s, 2);
		ctrl->EndUndoAction();
	};

	auto wrap = [&](wxStyledTextCtrl *ctrl) {
		ctrl->BeginUndoAction();
		ctrl->InsertText(e, "*/");
		ctrl->InsertText(s, "/*");
		ctrl->EndUndoAction();
	};

	if (hasOpen && hasClose) {
		unwrap(this);
		SetSelection(selStart, wxMax(selStart, selEnd - 4));
	} else {
		wrap(this);
		SetSelection(selStart, selEnd + 4);
	}
}

wxString Editor::ParseCompletionItems(const std::string &json,
									  const wxString &prefix) {
	if (json.empty() || json == "{}")
		return wxEmptyString;

	try {
		auto j = nlohmann::json::parse(json);
		if (!j.contains("result"))
			return wxEmptyString;

		const auto &result = j["result"];
		nlohmann::json items;

		if (result.is_array()) {
			items = result;
		} else if (result.is_object() && result.contains("items")) {
			items = result["items"];
		} else {
			return wxEmptyString;
		}

		if (!items.is_array() || items.empty())
			return wxEmptyString;

		wxString list;
		std::unordered_set<std::string> seen;
		int count = 0;

		for (const auto &item : items) {
			if (count >= 50)
				break;

			wxString label;

			if (item.contains("label") && item["label"].is_string()) {
				label = wxString::FromUTF8(item["label"].get<std::string>());
			} else if (item.contains("insertText") &&
					   item["insertText"].is_string()) {
				label =
					wxString::FromUTF8(item["insertText"].get<std::string>());
			} else if (item.contains("filterText") &&
					   item["filterText"].is_string()) {
				label =
					wxString::FromUTF8(item["filterText"].get<std::string>());
			} else {
				continue;
			}

			if (label.empty() || label.Length() > 100)
				continue;

			label.Trim(true);
			label.Trim(false);

			wxString clean = label;
			clean.Replace(" ", "_");
			clean.Replace("\t", "_");
			clean.Replace("\n", "_");

			if (clean.empty())
				continue;

			if (!prefix.empty()) {
				wxString prefixLower = prefix.Lower();
				wxString cleanLower = clean.Lower();
				if (!cleanLower.StartsWith(prefixLower)) {
					wxString noUnderscore = cleanLower;
					while (!noUnderscore.empty() && noUnderscore[0] == '_') {
						noUnderscore = noUnderscore.Mid(1);
					}
					if (!noUnderscore.StartsWith(prefixLower)) {
						continue;
					}
				}
			}

			std::string key = clean.ToStdString();
			if (seen.find(key) != seen.end())
				continue;
			seen.insert(key);

			if (!list.empty())
				list += " ";
			list += clean;
			count++;
		}

		return list;

	} catch (const std::exception &e) {
		return wxEmptyString;
	}
}