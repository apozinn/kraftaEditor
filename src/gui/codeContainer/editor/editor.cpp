#include "./editor.hpp"
#include "appConstants/appConstants.hpp"

#define MY_FOLDMARGIN 2

Editor::Editor(wxWindow *parent) : wxStyledTextCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE)
{
    currentPath = parent->GetLabel();
    InitializePrefs();
    SetFoldPreferences();
}

void Editor::InitializePrefs()
{
    auto backgroundColor = Theme["secondary"].template get<std::string>();
    auto textColor = Theme["text"].template get<std::string>();
    auto secondaryTextColor = Theme["secondaryText"].template get<std::string>();

    SetUseTabs(true);
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

    // linking autocomp icons
    RegisterImage(0, wxBitmap(iconsDir + "thunder.png", wxBITMAP_TYPE_PNG));
    RegisterImage(1, wxBitmap(iconsDir + "question.png", wxBITMAP_TYPE_PNG));
}

void Editor::SetFoldPreferences()
{
    auto backgroundColor = Theme["secondary"].template get<std::string>();
    auto secondaryTextColor = Theme["secondaryText"].template get<std::string>();

    MarkerEnableHighlight(true);
    SetMarginSensitive(2, true);

    SetFoldMarginColour(true, wxColor(backgroundColor));
    SetFoldMarginHiColour(true, wxColor(backgroundColor));

    Bind(wxEVT_STC_STYLENEEDED, &Editor::OnStyleNeeded, this);
    Bind(wxEVT_STC_MODIFIED, &Editor::OnChange, this);
    Bind(wxEVT_STC_MARGINCLICK, &Editor::OnMarginClick, this);
    Bind(wxEVT_KEY_UP, &Editor::OnArrowsPress, this);
    Bind(wxEVT_STC_CHARADDED, &Editor::CharAdd, this);
    Bind(wxEVT_LEFT_UP, &Editor::OnClick, this);
    Bind(wxEVT_STC_AUTOCOMP_COMPLETED, &Editor::OnAutoCompCompleted, this);
    Bind(wxEVT_STC_CLIPBOARD_PASTE, &Editor::OnClipBoardPaste, this);
    Bind(wxEVT_MOUSEWHEEL, &Editor::OnScroll, this);
}

void Editor::OnChange(wxStyledTextEvent &event)
{
    if (event.GetString() == wxEmptyString || GetText() == event.GetString())
        return;

    if (!changedFile && GetModify())
    {
        auto tab = FindWindowByLabel(ProjectSettings::Get().GetCurrentlyFileOpen() + "_tab");
        if (tab)
        {
            auto icon = ((wxStaticBitmap *)tab->GetChildren()[0]->GetChildren()[2]);
            if (icon)
            {
                // setting the unsaved icon
                icon->SetBitmap(wxBitmapBundle::FromBitmap(wxBitmap(iconsDir + "white_circle.png", wxBITMAP_TYPE_PNG)));
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
    int margin = event.GetMargin();
    int position = event.GetPosition();
    int line = LineFromPosition(position);
    int foldLevel = GetFoldLevel(line);
    bool headerFlag = (foldLevel & wxSTC_FOLDLEVELHEADERFLAG) != 0;
    if (margin == MY_FOLDMARGIN && headerFlag)
    {
        ToggleFold(line);
    }
}

void Editor::OnBackspace(wxKeyEvent &WXUNUSED(event))
{
    char chr = (char)GetCharAt(GetCurrentPos());

    if (chr == ']' || chr == '}' || chr == '"' || chr == '\'' || chr == '`' || chr == ')')
    {
        Remove(GetCurrentPos(), GetCurrentPos() + 1);
    }
}

void Editor::OnArrowsPress(wxKeyEvent &event)
{
    if (event.GetKeyCode() == 8)
    {
        OnBackspace(event);
    }

    statusBar->UpdateCodeLocale(this);
}

void Editor::CharAdd(wxStyledTextEvent &event)
{
    // adding same char to the minimap
    ((wxStyledTextCtrl *)GetParent()->GetChildren()[1])->SetText(GetText());

    char chr = (char)event.GetKey();
    char previous_char = (char)GetCharAt(GetCurrentPos() - 1);
    char next_char = (char)GetCharAt(GetCurrentPos());

    int previousLine = GetCurrentLine() - 1;
    int currentLine = GetCurrentLine();

    int previousLineInd = GetLineIndentation(previousLine);
    int currentLineInd = GetLineIndentation(currentLine);
    int nextLineInd = GetLineIndentation(currentLine + 1);

    if (chr == '\n')
    {
        if ((previous_char == '\n' && next_char == '}') || (previous_char == '\n' && next_char == ']'))
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

bool Editor::Modified()
{
    return (GetModify() && !GetReadOnly());
}

void Editor::OnStyleNeeded(wxStyledTextEvent &WXUNUSED(event))
{
    const int currentLine = LineFromPosition(GetCurrentPos());
    const int styledLine = LineFromPosition(GetEndStyled());

    int startLine = std::max(0, styledLine - 2);
    int endLine = currentLine;

    if (GetLineCount() <= LinesOnScreen())
    {
        startLine = 0;
        endLine = GetLineCount() - 1;
    }
    else if (endLine < startLine)
    {
        std::swap(startLine, endLine);
    }

    if (endLine < GetLineCount() - 1)
    {
        endLine++;
    }

    const size_t startPos = PositionFromLine(startLine);
    const size_t endPos = GetLineEndPosition(endLine);
    const int foldLevel = GetFoldLevel(startLine) & wxSTC_FOLDFLAG_LEVELNUMBERS;
    wxString text = GetTextRange(startPos, endPos).Upper();

    HighlightSyntax(startPos, endPos, text);
    UpdateFoldLevels(startPos, foldLevel, text);
}

void Editor::HighlightSyntax(size_t fromPos, size_t toPos, const wxString &text)
{
    std::vector<std::pair<size_t, size_t>> codeBlocks;
    bool isWordBoundary = true;

    for (size_t pos = 0; pos < text.length(); ++pos)
    {
        const char currentChar = text[pos];

        if (currentChar == 'G' && isWordBoundary)
        {
            const size_t start = pos;
            pos++; // Skip 'G'

            while (pos < text.length() && isdigit(text[pos]))
            {
                pos++;
            }

            if (pos - start > 1 && IsWordBoundary(text, pos))
            {
                codeBlocks.emplace_back(start + fromPos, pos + fromPos);
            }

            isWordBoundary = false;
        }

        isWordBoundary = IsWordBoundary(text, pos);
    }

    ApplySyntaxHighlighting(fromPos, toPos, codeBlocks);
}

bool Editor::IsWordBoundary(const wxString &text, size_t pos) const
{
    if (pos >= text.length())
        return true;
    const char c = text[pos];
    return c == ' ' || c == '\n' || c == '\r' || c == '\t';
}

void Editor::ApplySyntaxHighlighting(size_t fromPos, size_t toPos,
                                     const std::vector<std::pair<size_t, size_t>> &blocks)
{
    StartStyling(fromPos);
    SetStyling(toPos - fromPos, STYLE_DEFAULT);

    for (const auto &block : blocks)
    {
        StartStyling(block.first);
        SetStyling(block.second - block.first, STYLE_KEYWORD);
    }
}

void Editor::UpdateFoldLevels(size_t fromPos, int initialFoldLevel, const wxString &text)
{
    std::vector<std::pair<size_t, int>> foldChanges;
    int currentLevel = initialFoldLevel;
    size_t pos = 0;

    while ((pos = text.find("IF", pos)) != wxString::npos)
    {
        const size_t line = LineFromPosition(pos + fromPos);
        const bool isEndIf = (pos >= 3) && (text.substr(pos - 3, 5)) == "ENDIF";

        currentLevel += isEndIf ? -1 : 1;
        foldChanges.emplace_back(line, currentLevel);
        pos += 2;
    }

    ApplyFoldLevels(fromPos, initialFoldLevel, text.length(), foldChanges);
}

void Editor::ApplyFoldLevels(size_t startPos, int initialLevel, size_t textLength,
                             const std::vector<std::pair<size_t, int>> &changes)
{
    const size_t startLine = LineFromPosition(startPos);
    const size_t endLine = LineFromPosition(startPos + textLength);
    int currentLevel = initialLevel;
    size_t changeIndex = 0;

    for (size_t line = startLine; line <= endLine; ++line)
    {
        int levelFlags = currentLevel | wxSTC_FOLDLEVELBASE;

        if (changeIndex < changes.size() && line == changes[changeIndex].first)
        {
            currentLevel = changes[changeIndex].second;
            if (currentLevel > (levelFlags & wxSTC_FOLDLEVELNUMBERMASK))
            {
                levelFlags |= wxSTC_FOLDLEVELHEADERFLAG;
            }
            changeIndex++;
        }

        if (GetLineLength(line) == 0)
        {
            levelFlags |= wxSTC_FOLDLEVELWHITEFLAG;
        }

        SetFoldLevel(line, levelFlags);
    }
}