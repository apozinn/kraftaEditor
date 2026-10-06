#include "./textCtrl.hpp"

TextCtrl::TextCtrl(wxWindow *parent)
	: wxStyledTextCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
					   wxBORDER_NONE),
	  m_isDestroyed(false) {

	BindEvents();
	InitializePreferences();
	SetupStyles();
	ConfigureFoldMargin();
	SetupAcceleratorTable();
}

void TextCtrl::BindEvents() {
	Bind(wxEVT_STC_CHANGE, &TextCtrl::OnChange, this);
	Bind(wxEVT_STC_CHARADDED, &TextCtrl::CharAdd, this);
	Bind(wxEVT_LEFT_DOWN, &TextCtrl::OnClick, this);
	Bind(wxEVT_MENU, &TextCtrl::OnSave, this, +Event::File::Save);
	Bind(wxEVT_MENU, &TextCtrl::SaveAs, this, +Event::File::SaveAs);
	Bind(wxEVT_MENU, &TextCtrl::SaveAll, this, +Event::File::SaveAll);
	Bind(wxEVT_MENU, &TextCtrl::CloseFile, this, +Event::File::CloseFile);
	Bind(wxEVT_MENU, &TextCtrl::ToggleLineComment, this,
		 +Event::Edit::ToggleLineComment);
	Bind(wxEVT_MENU, &TextCtrl::ToggleBlockComment, this,
		 +Event::Edit::ToggleBlockComment);
	Bind(wxEVT_MENU, &TextCtrl::SelectLine, this, +Event::Edit::SelectLine);
	Bind(wxEVT_MENU, &TextCtrl::DuplicateLine, this,
		 +Event::Edit::DuplicateLineUp);
	Bind(wxEVT_MENU, &TextCtrl::MoveCursorUp, this, +Event::Edit::MoveCursorUp);
	Bind(wxEVT_MENU, &TextCtrl::MoveCursorDown, this,
		 +Event::Edit::MoveCursorDown);
	Bind(wxEVT_MENU, &TextCtrl::MoveLineUp, this, +Event::Edit::MoveLineUp);
	Bind(wxEVT_MENU, &TextCtrl::MoveLineDown, this, +Event::Edit::MoveLineDown);
	Bind(wxEVT_MENU, &TextCtrl::RemoveCurrentLine, this,
		 +Event::Edit::RemoveCurrentLine);

	Bind(wxEVT_LEFT_DOWN, [this](wxMouseEvent &event) {
		if (ProjectSettings::Get().GetCurrentlyFileOpen() != this->GetName()) {
			ProjectSettings::Get().SetCurrentlyFileOpen(this->GetName());
		}
		event.Skip();
	});
}

void TextCtrl::SetLanguagePreferences(
	languagePreferencesStruct languagePreferences) {
	if (!languagePreferences.name.empty()) {
		m_languagePreferences = languagePreferences;

		SetupAutocomplete();
		SetupLsp();
	}
}

void TextCtrl::SetupAutocomplete() {
	if (m_autocompleteController)
		return;
	m_autocompleteController =
		AutoCompleteController::Create(this, m_languagePreferences);
}

void TextCtrl::SetupLsp() {
	if (m_lspController)
		return;
	m_lspController = LspController::Create(this, m_languagePreferences);
	if (m_lspController && m_autocompleteController) {
		m_lspController->SetAutocompleteController(m_autocompleteController);
	} else {
	}
}

TextCtrl::~TextCtrl() {
    m_isDestroyed = true;
    
    if (m_lspController) {
        m_lspController->Stop();
    }

    Unbind(wxEVT_STC_MODIFIED, &TextCtrl::OnChange, this);
    Unbind(wxEVT_STC_MARGINCLICK, &TextCtrl::OnMarginClick, this);
    Unbind(wxEVT_STC_CHARADDED, &TextCtrl::CharAdd, this);
    Unbind(wxEVT_KEY_UP, &TextCtrl::OnArrowsPress, this);
    Unbind(wxEVT_KEY_DOWN, &TextCtrl::OnBackspace, this);
    Unbind(wxEVT_LEFT_UP, &TextCtrl::OnClick, this);
    Unbind(wxEVT_MOUSEWHEEL, &TextCtrl::OnScroll, this);
    
    Unbind(wxEVT_MENU, &TextCtrl::ToggleLineComment, this, 
           +Event::Edit::ToggleLineComment);
    Unbind(wxEVT_MENU, &TextCtrl::ToggleBlockComment, this, 
           +Event::Edit::ToggleBlockComment);
    Unbind(wxEVT_MENU, &TextCtrl::DoCopy, this, 
           +Event::Edit::CopyByKeyboard);
    Unbind(wxEVT_MENU, &TextCtrl::DoZoomIn, this, 
           +Event::View::ZoomIn);
    Unbind(wxEVT_MENU, &TextCtrl::DoZoomOut, this, 
           +Event::View::ZoomOut);
    Unbind(wxEVT_MENU, &TextCtrl::MoveCursorDown, this, 
           +Event::Edit::MoveCursorDown);
    Unbind(wxEVT_MENU, &TextCtrl::MoveCursorUp, this, 
           +Event::Edit::MoveCursorUp);
    Unbind(wxEVT_MENU, &TextCtrl::DuplicateLine, this, 
           +Event::Edit::DuplicateLineDown);
    Unbind(wxEVT_MENU, &TextCtrl::SelectNextOccurrence, this, 
           +Event::Edit::SelectNextOccurrence);
}

void TextCtrl::InitializePreferences() {
	SetUseTabs(true);
	SetTabIndents(true);
	SetBackSpaceUnIndents(true);

	SetIndent(4);
	SetTabWidth(4);
	SetIndentationGuides(wxSTC_IV_LOOKBOTH);
	SetEndAtLastLine(false);

	SetMultipleSelection(true);
	SetAdditionalSelectionTyping(true);
	SetMultiPaste(wxSTC_MULTIPASTE_EACH);
	SetVirtualSpaceOptions(wxSTC_VS_RECTANGULARSELECTION);
	SetCaretWidth(3);

	SetFocus();
}

void TextCtrl::SetupStyles() {
	auto backgroundColor = ThemesManager::Get().GetColor("secondary");
	auto textColor = ThemesManager::Get().GetColor("text");
	auto secondaryTextColor = ThemesManager::Get().GetColor("secondaryText");
	auto caretColor = ThemesManager::Get().GetColor("editorCaret");

	StyleSetBackground(wxSTC_STYLE_DEFAULT, wxColor(backgroundColor));
	StyleSetForeground(wxSTC_STYLE_DEFAULT, wxColor(textColor));
	StyleClearAll();

	SetCaretForeground(caretColor);

	SetMarginWidth(EditorConstants::LINE_NUMBER_MARGIN,
				   TextWidth(wxSTC_STYLE_LINENUMBER, wxT("_99999")));
	SetMarginType(EditorConstants::LINE_NUMBER_MARGIN, wxSTC_MARGIN_NUMBER);

	StyleSetForeground(wxSTC_STYLE_LINENUMBER, wxColor(secondaryTextColor));
	StyleSetBackground(wxSTC_STYLE_LINENUMBER, wxColor(backgroundColor));

	StyleSetBackground(wxSTC_STYLE_INDENTGUIDE, wxColor(backgroundColor));
	StyleSetForeground(wxSTC_STYLE_INDENTGUIDE, wxColor(secondaryTextColor));
}

void TextCtrl::SetupAcceleratorTable() {
	wxAcceleratorEntry entries[] = {
		{wxACCEL_CTRL, WXK_RETURN,
		 static_cast<int>(Event::Edit::MoveCursorDown)},

		{wxACCEL_CTRL, (int)'S', static_cast<int>(Event::File::Save)},

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
}

void TextCtrl::ConfigureFoldMargin() {
	auto backgroundColor = ThemesManager::Get().GetColor("secondary");

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

TextCtrl *TextCtrl::GetFocusedTextCtrl() {
	wxWindow *focused = wxWindow::FindFocus();
	TextCtrl *textCtrl = wxDynamicCast(focused, TextCtrl);

	if (!textCtrl) {
		textCtrl = wxDynamicCast(
			FindWindowByLabel(ProjectSettings::Get().GetCurrentlyFileOpen() +
							  "_textCtrl"),
			TextCtrl);
		if (!textCtrl) {
			wxMessageBox(_("could not find the target component"));
		}
	}

	return textCtrl;
}

void TextCtrl::OnSave(wxCommandEvent &e) {
	TextCtrl *focusedTextCtrl = GetFocusedTextCtrl();
	if (!focusedTextCtrl)
		return;

	focusedTextCtrl->DoSave(focusedTextCtrl->GetName());
}

void TextCtrl::DoSave(const wxString &path) {
	try {
		if (wxFileExists(path)) {
			SaveFile(path);

			if (auto tab = FindWindowByLabel(path + "_tab")) {
				auto icon =
					((wxStaticBitmap *)tab->GetChildren()[0]->GetChildren()[2]);
				if (icon) {
					icon->SetBitmap(wxBitmapBundle::FromBitmap(wxBitmap(
						ApplicationPaths::AssetsPath("icons") + "close.png",
						wxBITMAP_TYPE_PNG)));
					icon->SetLabel("saved_icon");
					tab->Layout();
				}
			}

			if (path == UserSettingsManager::Get().SettingsPath) {
				UserSettingsManager::Get().LoadSettingsFromFile();
			}

			if (path == ShortCutSettingsManager::Get().ShortcutsPath) {
				ShortCutSettingsManager::Get().LoadSettingsFromFile();
			}
		} else {
			throw _("This file does not exist");
		}
	} catch (std::exception error) {
		wxMessageBox(error.what());
	}
}

void TextCtrl::SaveAs(wxCommandEvent &e) {
	TextCtrl *focusedTextCtrl = GetFocusedTextCtrl();
	if (!focusedTextCtrl)
		return;

	wxFileName currentFile(ProjectSettings::Get().GetCurrentlyFileOpen());

	wxString filename;
	wxFileDialog dlg(
		this, "Save file", currentFile.GetPath(),
		wxFileNameFromPath(ProjectSettings::Get().GetCurrentlyFileOpen()),
		"Any file (*)|*", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	if (dlg.ShowModal() != wxID_OK)
		return;
	filename = dlg.GetPath();
	focusedTextCtrl->SaveFile(filename);
}

void TextCtrl::SaveAll(wxCommandEvent &e) {
	auto mainCode = FindWindowById(+GUI::ControlID::MainCode);
	if (mainCode) {
		for (auto &&children : mainCode->GetChildren()) {
			if (children->GetLabel().ToStdString().find("_editor") !=
				std::string::npos) {
				for (auto &&w_children : children->GetChildren()) {
					if (w_children->GetLabel().ToStdString().find(
							"_textCtrl") != std::string::npos) {
						TextCtrl *w_textCtrl =
							wxDynamicCast(w_children, TextCtrl);
						if (w_textCtrl)
							w_textCtrl->DoSave(w_textCtrl->GetName());
					}
				}
			}
		}
	}
}

void TextCtrl::CloseFile(wxCommandEvent &e) {
	TextCtrl *focusedTextCtrl = GetFocusedTextCtrl();
	if (!focusedTextCtrl)
		return;

	if (focusedTextCtrl->Modified()) {
		if (wxMessageBox(_("Text is not saved, save before closing?"),
						 _("Close"), wxYES_NO | wxICON_QUESTION) == wxYES) {
			focusedTextCtrl->DoSave(focusedTextCtrl->GetName());
			if (focusedTextCtrl->Modified()) {
				wxMessageBox(_("Text could not be saved!"), _("Close abort"),
							 wxOK | wxICON_EXCLAMATION);
				return;
			}
		}
	}

	auto tabsContainer = ((Tabs *)FindWindowById(+GUI::ControlID::Tabs));
	if (tabsContainer) {
		tabsContainer->Close(ProjectSettings::Get().GetCurrentlyFileOpen());
	}
}

void TextCtrl::OnMarginClick(wxStyledTextEvent &event) {
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

void TextCtrl::OnBackspace(wxKeyEvent &event) {
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

void TextCtrl::OnArrowsPress(wxKeyEvent &event) {
	ClearIndicators();

	if (statusBar)
		statusBar->UpdateCodeLocale(this);

	event.Skip();
}

void TextCtrl::OnClick(wxMouseEvent &event) {
	HighlightSelectionOccurrences();

	if (statusBar)
		statusBar->UpdateCodeLocale(this);

	event.Skip();
}

void TextCtrl::OnScroll(wxMouseEvent &event) {
	if (event.ShiftDown()) {
		OnHorizontalScroll(event);
	} else {
		event.Skip();
	}
}

bool TextCtrl::Modified() const { return GetModify() && !GetReadOnly(); }

void TextCtrl::HighlightSelectionOccurrences() {
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

void TextCtrl::ClearIndicators() {
	const int len = GetTextLength();

	for (int i = 0; i <= EditorConstants::MAX_INDICATOR; ++i) {
		IndicatorClearRange(0, len);
	}
}

void TextCtrl::UpdateUnsavedIndicator() {
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
		ApplicationPaths::AssetsPath("icons") + "unsaved" +
			(ThemesManager::Get().IsDarkTheme() ? "_light" : "_dark") + ".png",
		wxBITMAP_TYPE_PNG)));

	tab->Layout();
}

void TextCtrl::HandleAutoPairing(char chr) {
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

void TextCtrl::SelectNextOccurrence(wxCommandEvent &WXUNUSED(event)) {
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

void TextCtrl::OnHorizontalScroll(wxMouseEvent &event) {
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

void TextCtrl::RecreateMinimap() {
	CallAfter([this]() {
		if (m_isDestroyed)
			return;
		wxWindow *parent = this->GetParent();

		wxMilliSleep(50);
		wxTheApp->Yield(true);

		delete minimap;

		minimap = new wxStyledTextCtrlMiniMap(parent, this);

		minimap->SetSize(wxSize(100, minimap->GetSize().y));
		minimap->SetMinSize(wxSize(100, minimap->GetSize().y));

		parent->GetSizer()->Add(minimap, 0, wxEXPAND);

		parent->Refresh();
		parent->Update();
		parent->GetSizer()->Layout();
	});
}

void TextCtrl::DoZoomIn(wxCommandEvent &e) {
	TextCtrl *focusedTextCtrl = GetFocusedTextCtrl();
	if (!focusedTextCtrl)
		return;

	focusedTextCtrl->ZoomIn();
}

void TextCtrl::DoZoomOut(wxCommandEvent &e) {
	TextCtrl *focusedTextCtrl = GetFocusedTextCtrl();
	if (!focusedTextCtrl)
		return;

	focusedTextCtrl->ZoomOut();
}