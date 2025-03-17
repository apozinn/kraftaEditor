#include "./editor.hpp"

Editor::Editor(wxWindow *parent) : wxStyledTextCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE)
{
    currentPath = parent->GetLabel();
    InitializePrefs();
}

void Editor::InitializePrefs()
{
    auto backgroundColor = UserTheme["secondary"].template get<std::string>();
    auto textColor = UserTheme["text"].template get<std::string>();
    auto secondaryTextColor = UserTheme["secondaryText"].template get<std::string>();

    SetUseTabs(true);
    SetTabWidth(4);
    SetIndent(4);
    SetTabIndents(true);
    SetBackSpaceUnIndents(true);
    SetIndentationGuides(true);
    SetScrollWidth(1);

    // setting  the default color
    StyleSetBackground(wxSTC_STYLE_DEFAULT, wxColor(backgroundColor));
    StyleSetForeground(wxSTC_STYLE_DEFAULT, wxColor(textColor));

    // clearing all preset styles
    StyleClearAll();

    // setting the caret color
    SetCaretForeground(wxColour(textColor));

    // margin settings
    SetMarginWidth(0, TextWidth(wxSTC_STYLE_LINENUMBER, wxT("_99999")));
    SetMarginType(0, wxSTC_MARGIN_NUMBER);
    StyleSetForeground(wxSTC_STYLE_LINENUMBER, wxColor(secondaryTextColor));
    StyleSetBackground(wxSTC_STYLE_LINENUMBER, wxColor(backgroundColor));

    StyleSetBackground(wxSTC_STYLE_INDENTGUIDE, wxColor(backgroundColor));
    StyleSetForeground(wxSTC_STYLE_INDENTGUIDE, wxColor(secondaryTextColor));

    SetMarginWidth(1, 16);
    SetMarginType(1, wxSTC_MARGIN_SYMBOL);
    SetMarginMask(1, wxSTC_MASK_FOLDERS);
    SetMarginSensitive(1, true);

    // linking events
    Bind(wxEVT_STC_MODIFIED, &Editor::OnChange, this);
    Bind(wxEVT_STC_MARGINCLICK, &Editor::OnMarginClick, this);
    Bind(wxEVT_KEY_UP, &Editor::OnArrowsPress, this);
    Bind(wxEVT_STC_CHARADDED, &Editor::CharAdd, this);
    Bind(wxEVT_LEFT_UP, &Editor::OnClick, this);
    Bind(wxEVT_STC_AUTOCOMP_COMPLETED, &Editor::OnAutoCompCompleted, this);
    Bind(wxEVT_STC_CLIPBOARD_PASTE, &Editor::OnClipBoardPaste, this);
    Bind(wxEVT_MOUSEWHEEL, &Editor::OnScroll, this);

    // linking autocomp icons
    RegisterImage(0, wxBitmap(icons_dir + "thunder.png", wxBITMAP_TYPE_PNG));
    RegisterImage(1, wxBitmap(icons_dir + "question.png", wxBITMAP_TYPE_PNG));

    // setting the fold
    SetProperty(wxT("fold"), wxT("1"));
    SetProperty(wxT("fold.comment"), wxT("1"));
    SetProperty(wxT("fold.compact"), wxT("1"));
    SetProperty(wxT("fold.preprocessor"), wxT("1"));
    SetProperty(wxT("fold.html"), wxT("1"));
    SetProperty(wxT("fold.html.preprocessor"), wxT("1"));
    SetFoldFlags(wxSTC_FOLDFLAG_LINEBEFORE_CONTRACTED | wxSTC_FOLDFLAG_LINEAFTER_CONTRACTED);

    // setting the folder
    SetMarginMask(2, wxSTC_MASK_FOLDERS);
    SetFoldMarginColour(true, wxColor(backgroundColor));
    SetFoldMarginHiColour(true, wxColor(backgroundColor));

    MarkerDefine(wxSTC_MARKNUM_FOLDER, wxSTC_MARK_ARROW);
    MarkerSetForeground(wxSTC_MARKNUM_FOLDER, wxColor(backgroundColor));
    MarkerSetBackground(wxSTC_MARKNUM_FOLDER, wxColor(secondaryTextColor));

    MarkerDefine(wxSTC_MARKNUM_FOLDEREND, wxSTC_MARK_ARROW);
    MarkerSetForeground(wxSTC_MARKNUM_FOLDEREND, wxColor(backgroundColor));
    MarkerSetBackground(wxSTC_MARKNUM_FOLDEREND, wxColor(secondaryTextColor));

    MarkerDefine(wxSTC_MARKNUM_FOLDERMIDTAIL, wxSTC_MARK_TCORNER);
    MarkerSetForeground(wxSTC_MARKNUM_FOLDERMIDTAIL, wxColor(backgroundColor));
    MarkerSetBackground(wxSTC_MARKNUM_FOLDERMIDTAIL, wxColor(secondaryTextColor));

    MarkerDefine(wxSTC_MARKNUM_FOLDEROPEN, wxSTC_MARK_ARROWDOWN);
    MarkerSetForeground(wxSTC_MARKNUM_FOLDEROPEN, wxColor(backgroundColor));
    MarkerSetBackground(wxSTC_MARKNUM_FOLDEROPEN, wxColor(secondaryTextColor));

    MarkerDefine(wxSTC_MARKNUM_FOLDEROPENMID, wxSTC_MARK_ARROWDOWN);
    MarkerSetForeground(wxSTC_MARKNUM_FOLDEROPENMID, wxColor(backgroundColor));
    MarkerSetBackground(wxSTC_MARKNUM_FOLDEROPENMID, wxColor(secondaryTextColor));

    MarkerDefine(wxSTC_MARKNUM_FOLDERSUB, wxSTC_MARK_VLINE);
    MarkerSetForeground(wxSTC_MARKNUM_FOLDERSUB, wxColor(backgroundColor));
    MarkerSetBackground(wxSTC_MARKNUM_FOLDERSUB, wxColor(secondaryTextColor));

    MarkerDefine(wxSTC_MARKNUM_FOLDERTAIL, wxSTC_MARK_LCORNER);
    MarkerSetForeground(wxSTC_MARKNUM_FOLDERTAIL, wxColor(backgroundColor));
    MarkerSetBackground(wxSTC_MARKNUM_FOLDERTAIL, wxColor(secondaryTextColor));

    MarkerEnableHighlight(false);
    SetMarginSensitive(2, true);

    SetFoldLevel(0, 1024);
    SetFoldLevel(1, 1024);
    SetFoldLevel(2, 1024 | wxSTC_FOLDLEVELHEADERFLAG);
    SetFoldLevel(3, 1025);
    SetFoldLevel(4, 1025);
    SetFoldLevel(5, 1024);
    SetFoldLevel(6, 1024 | wxSTC_FOLDLEVELWHITEFLAG);
}

void Editor::Save()
{
    auto tab = FindWindowByLabel(current_openned_path + "_tab");
    if (tab)
    {
        // Save the file
        SaveFile(current_openned_path);

        // setting the close icon
        auto icon = ((wxStaticBitmap *)tab->GetChildren()[0]->GetChildren()[2]);
        icon->SetBitmap(wxBitmapBundle::FromBitmap(wxBitmap(icons_dir + "close.png", wxBITMAP_TYPE_PNG)));

        changedFile = false;
    }
}

void Editor::OnChange(wxStyledTextEvent &event)
{
    if (event.GetString() == wxEmptyString || GetText() == event.GetString())
    {

        return;
    }

    if (!changedFile && GetModify())
    {
        auto tab = FindWindowByLabel(current_openned_path + "_tab");
        if (tab)
        {
            auto icon = ((wxStaticBitmap *)tab->GetChildren()[0]->GetChildren()[2]);
            if (icon)
            {
                // setting the unsaved icon
                icon->SetBitmap(wxBitmapBundle::FromBitmap(wxBitmap(icons_dir + "white_circle.png", wxBITMAP_TYPE_PNG)));
            }
        }

        changedFile = true;
    }

    if (MiniMap)
        MiniMap->SetText(GetText());
    statusBar->UpdateCodeLocale(this);
}

void Editor::OnMarginClick(wxStyledTextEvent &event)
{
    if (event.GetMargin() == 1)
    {
        int lineClick = LineFromPosition(event.GetPosition());
        int levelClick = GetFoldLevel(lineClick);
        if ((levelClick & wxSTC_FOLDLEVELHEADERFLAG) > 0)
            ToggleFold(lineClick);
    }
}

void Editor::OnArrowsPress(wxKeyEvent &event)
{
    statusBar->UpdateCodeLocale(this);
}

void Editor::CharAdd(wxStyledTextEvent &event)
{
    // adding same char to the minimap
    ((wxStyledTextCtrl *)GetParent()->GetChildren()[1])->SetText(GetText());

    char chr = (char)event.GetKey();
    char previous_char = (char)GetCharAt(GetCurrentPos() - 2);
    char next_char = (char)GetCharAt(GetCurrentPos());

    int previousLine = GetCurrentLine() - 1;
    int currentLine = GetCurrentLine();
    int nextLine = GetCurrentLine() + 1;

    int previousLineInd = GetLineIndentation(previousLine);
    int currentLineInd = GetLineIndentation(currentLine);
    int nextLineInd = GetLineIndentation(currentLine + 1);

    if (chr == '\n')
    {
        if (previous_char == '{' && next_char == '}')
        {
            SetLineIndentation(currentLine, currentLineInd + GetIndent());
            GotoPos(GetLineEndPosition(currentLine) - 1);
            SetCurrentPos(GetLineEndPosition(currentLine) - 1);
            InsertText(GetCurrentPos(), "\n");
            return;
        }

        if (currentLineInd < nextLineInd)
        {
            SetLineIndentation(currentLine, nextLineInd);
            LineEnd();
        }
        else
        {
            SetLineIndentation(currentLine, previousLineInd);
            LineEnd();
        }
    }

    if (chr == '#')
    {
        wxString s = "define?0 elif?1 else?0 endif?01 error?0 if?0 ifdef?0 "
                     "ifndef?1 include?0 line?0 pragma?1 undef?0";
        AutoCompShow(0, s);
    }

    if (chr == 'i')
    {
        wxString s = "import?1 if?0";
        AutoCompShow(0, s);
    }

    if (chr == '(')
        InsertText(GetCurrentPos(), ")");
    if (chr == '{')
        InsertText(GetCurrentPos(), "}");
    if (chr == '"')
        InsertText(GetCurrentPos(), "\"");
    if (chr == '`')
        InsertText(GetCurrentPos(), "`");
    if (chr == '[')
        InsertText(GetCurrentPos(), "]");
    if (event.GetKey() == 39)
        InsertText(GetCurrentPos(), "'");

    SetMiniMapLine();
}

void Editor::OnClick(wxMouseEvent &event)
{
    statusBar->UpdateCodeLocale(this);
    event.Skip();
}

void Editor::OnAutoCompCompleted(wxStyledTextEvent &event)
{
    wxString completion = event.GetString();
    int pos = event.GetPosition();
    Remove(pos - 1, pos);
}

void Editor::OnClipBoardPaste(wxStyledTextEvent &event)
{
    if (auto getMinimap = ((wxStyledTextCtrl *)GetNextSibling()))
    {
        MiniMap = getMinimap;
        MiniMap->InsertText(event.GetPosition(), event.GetString());
    }
}

void Editor::SetMiniMapLine()
{
    if (auto getMinimap = ((wxStyledTextCtrl *)GetNextSibling()))
    {
        MiniMap = getMinimap;
        MiniMap->SetFirstVisibleLine(GetFirstVisibleLine());
        MiniMap->GotoPos(GetFirstVisibleLine());
        MiniMap->Refresh();
    }
}

void Editor::OnScroll(wxMouseEvent &event)
{
    SetMiniMapLine();
    event.Skip();
}