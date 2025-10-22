#include "./editor.hpp"
#include "appConstants/appConstants.hpp"
#include <unordered_map>
#include <vector>

Editor::Editor(wxWindow *parent) : wxStyledTextCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE)
{
    currentPath = parent->GetLabel();
    InitializePreferences();
    ConfigureFoldMargin();
}

void Editor::InitializePreferences()
{
    auto backgroundColor = Theme["secondary"].template get<std::string>();
    auto textColor = Theme["text"].template get<std::string>();
    auto secondaryTextColor = Theme["secondaryText"].template get<std::string>();

    SetUseTabs(true);
    SetTabIndents(true);
    SetBackSpaceUnIndents(true);
    SetIndentationGuides(true);
    SetScrollWidth(1);

    StyleSetBackground(wxSTC_STYLE_DEFAULT, wxColor(backgroundColor));
    StyleSetForeground(wxSTC_STYLE_DEFAULT, wxColor(textColor));
    StyleClearAll();
    SetCaretForeground(wxColour(textColor));

    SetMarginWidth(0, TextWidth(wxSTC_STYLE_LINENUMBER, wxT("_99999")));
    SetMarginType(0, wxSTC_MARGIN_NUMBER);
    StyleSetForeground(wxSTC_STYLE_LINENUMBER, wxColor(secondaryTextColor));
    StyleSetBackground(wxSTC_STYLE_LINENUMBER, wxColor(backgroundColor));

    StyleSetBackground(wxSTC_STYLE_INDENTGUIDE, wxColor(backgroundColor));
    StyleSetForeground(wxSTC_STYLE_INDENTGUIDE, wxColor(secondaryTextColor));

    SetCaretForeground(ThemesManager::Get().GetColor("editorCaret"));
    SetCaretWidth(3);

    wxAcceleratorEntry entries[5];
    entries[0].Set(wxACCEL_CTRL, WXK_RETURN, static_cast<int>(Event::Edit::MoveCursorDown));
    entries[1].Set(wxACCEL_CTRL | wxACCEL_SHIFT, WXK_RETURN, static_cast<int>(Event::Edit::MoveCursorUp));
    entries[2].Set(wxACCEL_CTRL | wxACCEL_ALT, WXK_DOWN, static_cast<int>(Event::Edit::DuplicateLineDown));
    entries[3].Set(wxACCEL_CTRL | wxACCEL_ALT, WXK_UP, static_cast<int>(Event::Edit::DuplicateLineUp));
    entries[4].Set(wxACCEL_CTRL, (int)'D', static_cast<int>(Event::Edit::SelectNextOccurrence));

    wxAcceleratorTable accel(5, entries);
    SetAcceleratorTable(accel);
}

void Editor::ConfigureFoldMargin()
{
    auto backgroundColor = Theme["secondary"].template get<std::string>();
    SetMarginWidth(2, 20);
    SetMarginType(2, wxSTC_MARGIN_SYMBOL);
    SetMarginMask(2, wxSTC_MASK_FOLDERS);
    SetMarginSensitive(2, true);

    SetFoldMarginColour(true, wxColor(backgroundColor));
    SetFoldMarginHiColour(true, wxColor(backgroundColor));
    SetFoldFlags(wxSTC_FOLDFLAG_LINEAFTER_CONTRACTED | wxSTC_FOLDFLAG_LINEBEFORE_EXPANDED);
}

void Editor::OnUpdateUI(wxStyledTextEvent &event)
{
    HighlightSelectionOccurrences();
    event.Skip();
}

void Editor::OnChange(wxStyledTextEvent &event)
{
    if (event.GetString().IsEmpty() || GetText() == event.GetString())
        return;

    if (GetModify())
    {
        UpdateUnsavedIndicator();
        changedFile = true;
    }

    if (MiniMap)
        MiniMap->SetText(GetText());

    if (statusBar)
        statusBar->UpdateCodeLocale(this);
    event.Skip();
}

void Editor::OnMarginClick(wxStyledTextEvent &event)
{
    int margin = event.GetMargin();
    int line = LineFromPosition(event.GetPosition());
    int foldLevel = GetFoldLevel(line);
    bool isHeader = (foldLevel & wxSTC_FOLDLEVELHEADERFLAG) != 0;

    if (margin == 2 && isHeader)
    {
        ToggleFold(line);
    }
    event.Skip();
}

void Editor::OnBackspace(wxKeyEvent &event)
{
    const int key = event.GetKeyCode();
    if (key != WXK_BACK && key != WXK_DELETE)
    {
        event.Skip();
        return;
    }

    long startPos = GetSelectionStart();
    long endPos = GetSelectionEnd();
    bool hasSelection = (startPos != endPos);

    if (hasSelection)
    {
        wxString selectedText = GetTextRange(startPos, endPos);
        if (selectedText.Length() == 2)
        {
            auto it = kPairMap.find(selectedText.Mid(1, 1));
            if (it != kPairMap.end() && it->second == selectedText.Mid(0, 1))
            {
                Remove(startPos, endPos);
                return;
            }
        }
        Remove(startPos, endPos);
        return;
    }

    long pos = GetCurrentPos();
    wxString prevChar, nextChar;

    if (pos > 0)
        prevChar = GetTextRange(pos - 1, pos);

    if (pos < GetLength())
        nextChar = GetTextRange(pos, pos + 1);

    if (key == WXK_BACK)
    {
        auto it = kPairMap.find(nextChar);
        if (it != kPairMap.end() && it->second == prevChar)
        {
            Remove(pos - 1, pos + 1);
            return;
        }
    }
    else if (key == WXK_DELETE)
    {
        auto it = kPairMap.find(prevChar);
        if (it != kPairMap.end() && it->second == nextChar)
        {
            Remove(pos - 1, pos + 1);
            return;
        }
    }

    event.Skip();
}

void Editor::OnArrowsPress(wxKeyEvent &event)
{
    ClearIndicators();
    if (event.GetKeyCode() == '2')
    {
        InsertText(GetCurrentPos(), "2");
        GotoPos(GetCurrentPos() + 1);
    }
    statusBar->UpdateCodeLocale(this);
    event.Skip();
}

void Editor::CharAdd(wxStyledTextEvent &event)
{
    ClearIndicators();
    UpdateMiniMapText();

    const char chr = static_cast<char>(event.GetKey());
    const char prevChar = GetCharAt(GetCurrentPos() - 1);
    const char nextChar = GetCharAt(GetCurrentPos());

    if (chr == '\n')
    {
        HandleNewLineIndentation(prevChar, nextChar);
    }

    HandleAutoPairing(chr);
    SetMiniMapLine();
    event.Skip();
}

void Editor::OnClick(wxMouseEvent &event)
{
    event.Skip();
    ClearIndicators();
    if (statusBar)
        statusBar->UpdateCodeLocale(this);
}

void Editor::OnAutoCompCompleted(wxStyledTextEvent &event)
{
    int pos = event.GetPosition();
    Remove(pos - 1, pos);
    event.Skip();
}

void Editor::OnClipBoardPaste(wxStyledTextEvent &event)
{
    if (MiniMap)
        MiniMap->InsertText(event.GetPosition(), event.GetString());
    event.Skip();
}

void Editor::OnScroll(wxMouseEvent &event)
{
    SetMiniMapLine();
    event.Skip();
}

bool Editor::Modified()
{
    return GetModify() && !GetReadOnly();
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
            size_t start = pos;
            pos++;

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

void Editor::ApplySyntaxHighlighting(size_t fromPos, size_t toPos, const std::vector<std::pair<size_t, size_t>> &blocks)
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

void Editor::ApplyFoldLevels(size_t startPos, int initialLevel, size_t textLength, const std::vector<std::pair<size_t, int>> &changes)
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

void Editor::HighlightSelectionOccurrences()
{
    ClearIndicators();

    int selStart = GetSelectionStart();
    int selEnd = GetSelectionEnd();

    if (selEnd <= selStart)
        return;

    wxString selectedText = GetTextRange(selStart, selEnd);
    int selectedLength = selectedText.Length();

    if (selectedLength < 2 || !wxIsalnum(selectedText[0]))
        return;

    SetIndicatorCurrent(0);

    int pos = 0;
    while (pos < GetTextLength())
    {
        int foundPos = FindText(pos, GetTextLength(), selectedText, wxSTC_FIND_MATCHCASE | wxSTC_FIND_WHOLEWORD);
        if (foundPos == -1)
            break;

        IndicatorFillRange(foundPos, selectedLength);
        pos = foundPos + selectedLength;
    }
}

void Editor::ClearIndicators()
{
    int textLength = GetTextLength();
    for (int indicator = 0; indicator <= 7; indicator++)
    {
        SetIndicatorCurrent(indicator);
        IndicatorClearRange(0, textLength);
    }
    Refresh();
    Update();
}

void Editor::UpdateUnsavedIndicator()
{
    wxWindow *tab = FindWindowByLabel(ProjectSettings::Get().GetCurrentlyFileOpen() + "_tab");
    if (!tab)
        return;

    auto children = tab->GetChildren();
    wxStaticBitmap *icon = nullptr;

    for (auto *child : children)
    {
        if (auto *bmp = wxDynamicCast(child, wxStaticBitmap))
        {
            if (child->GetName() != "tab_language_icon")
            {
                icon = bmp;
                break;
            }
        }
        for (auto *sub : child->GetChildren())
        {
            if (auto *bmp = wxDynamicCast(sub, wxStaticBitmap))
            {
                if (sub->GetName() != "tab_language_icon")
                {
                    icon = bmp;
                    break;
                }
            }
        }
    }

    if (icon && icon->GetLabel() != "unsaved_icon")
    {
        icon->SetBitmap(wxBitmapBundle::FromBitmap(wxBitmap(iconsDir + "white_circle.png", wxBITMAP_TYPE_PNG)));
        icon->SetLabel("unsaved_icon");
        tab->Layout();
    }
}

void Editor::UpdateMiniMapText()
{
    if (MiniMap)
        MiniMap->SetText(GetText());
}

void Editor::HandleNewLineIndentation(char prevChar, char nextChar)
{
    int currentLine = GetCurrentLine();
    int prevLine = currentLine - 1;
    int prevLineInd = GetLineIndentation(prevLine);
    int currentLineInd = GetLineIndentation(currentLine);
    int nextLineInd = GetLineIndentation(currentLine + 1);

    if ((prevChar == '\n' && (nextChar == '}' || nextChar == ']')))
    {
        SetLineIndentation(currentLine, currentLineInd + GetIndent());
        GotoPos(GetLineEndPosition(currentLine) - 1);
        InsertText(GetCurrentPos(), "\n");
    }
    else if (currentLineInd < nextLineInd)
    {
        SetLineIndentation(currentLine, nextLineInd);
    }
    else
    {
        SetLineIndentation(currentLine, prevLineInd);
    }
}

void Editor::HandleAutoPairing(char chr)
{
    switch (chr)
    {
    case '(':
        InsertText(GetCurrentPos(), ")");
        break;
    case '{':
        InsertText(GetCurrentPos(), "}");
        break;
    case '[':
        InsertText(GetCurrentPos(), "]");
        break;
    case '"':
        InsertText(GetCurrentPos(), "\"");
        break;
    case '\'':
        InsertText(GetCurrentPos(), "'");
        break;
    case '`':
        InsertText(GetCurrentPos(), "`");
        break;
    }
}

void Editor::SetMiniMapLine()
{
    if (MiniMap)
    {
        MiniMap->SetFirstVisibleLine(GetFirstVisibleLine());
        MiniMap->GotoPos(GetFirstVisibleLine());
        MiniMap->Refresh();
    }
}

void Editor::OnMoveCursorDown(wxCommandEvent &event)
{
    int currentLine = GetCurrentLine();
    int lineEndPos = GetLineEndPosition(currentLine);

    GotoPos(lineEndPos);
    AddText("\n");
    EnsureCaretVisible();
    SetEmptySelection(GetCurrentPos());
}

void Editor::OnMoveCursorUp(wxCommandEvent &event)
{
    int currentLine = GetCurrentLine();
    int lineStartPos = PositionFromLine(currentLine);

    GotoPos(lineStartPos);
    InsertText(lineStartPos, "\n");

    if (currentLine > 0)
        GotoPos(PositionFromLine(currentLine));
    else
        GotoPos(0);

    EnsureCaretVisible();
    SetEmptySelection(GetCurrentPos());
}

void Editor::OnDuplicateLineDown(wxCommandEvent &event)
{
    int line = GetCurrentLine();
    wxString text = GetLine(line);
    int insertPos = GetLineEndPosition(line);
    InsertText(insertPos, "\n" + text);
    GotoLine(line + 1);
    EnsureCaretVisible();
}

void Editor::OnDuplicateLineUp(wxCommandEvent &event)
{
    int line = GetCurrentLine();
    wxString text = GetLine(line);
    int insertPos = PositionFromLine(line);
    InsertText(insertPos, text + "\n");
    if (line > 0)
        GotoLine(line);
    else
        GotoLine(0);
    EnsureCaretVisible();
}

void Editor::SelectNextOccurrence(wxCommandEvent &event)
{
    if (GetSelections() == 0)
        return;

    int mainSel = GetMainSelection();
    int selStart = GetSelectionNStart(mainSel);
    int selEnd = GetSelectionNEnd(mainSel);

    if (selStart == selEnd)
        return;

    wxString text = GetTextRange(selStart, selEnd);
    int nextPos = SearchInTarget(text);

    SetTargetStart(selEnd);
    SetTargetEnd(GetLength());

    if (SearchInTarget(text) != -1)
    {
        int foundStart = GetTargetStart();
        int foundEnd = GetTargetEnd();

        AddSelection(foundStart, foundEnd);
        SetMainSelection(GetSelections() - 1);
    }
}