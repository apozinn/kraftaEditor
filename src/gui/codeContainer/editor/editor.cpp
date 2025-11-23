#include "./editor.hpp"
#include "appConstants/appConstants.hpp"
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <cctype>

Editor::Editor(wxWindow *parent) : wxStyledTextCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE)
{
    currentPath = parent->GetLabel();
    InitializePreferences();
    ConfigureFoldMargin();
}

void Editor::InitializePreferences()
{
    const wxString backgroundColor(Theme["secondary"].template get<std::string>());
    const wxString textColor(Theme["text"].template get<std::string>());
    const wxString secondaryTextColor(Theme["secondaryText"].template get<std::string>());

    SetUseTabs(true);
    SetTabIndents(true);
    SetBackSpaceUnIndents(true);
    SetIndentationGuides(true);
    SetScrollWidth(1);
	SetEndAtLastLine (true);

    StyleSetBackground(wxSTC_STYLE_DEFAULT, wxColor(backgroundColor));
    StyleSetForeground(wxSTC_STYLE_DEFAULT, wxColor(textColor));
    StyleClearAll();
    
    SetCaretForeground(ThemesManager::Get().GetColor("editorCaret"));
    SetCaretWidth(3);

    SetMarginWidth(EditorConstants::LINE_NUMBER_MARGIN, TextWidth(wxSTC_STYLE_LINENUMBER, wxT("_99999")));
    SetMarginType(EditorConstants::LINE_NUMBER_MARGIN, wxSTC_MARGIN_NUMBER);
    StyleSetForeground(wxSTC_STYLE_LINENUMBER, wxColor(secondaryTextColor));
    StyleSetBackground(wxSTC_STYLE_LINENUMBER, wxColor(backgroundColor));

    StyleSetBackground(wxSTC_STYLE_INDENTGUIDE, wxColor(backgroundColor));
    StyleSetForeground(wxSTC_STYLE_INDENTGUIDE, wxColor(secondaryTextColor));
    
    wxAcceleratorEntry entries[] = {
        {wxACCEL_CTRL, WXK_RETURN, static_cast<int>(Event::Edit::MoveCursorDown)},
        {wxACCEL_CTRL | wxACCEL_SHIFT, WXK_RETURN, static_cast<int>(Event::Edit::MoveCursorUp)},
        {wxACCEL_CTRL | wxACCEL_SHIFT, (int)'D', static_cast<int>(Event::Edit::DuplicateLineDown)},
        {wxACCEL_CTRL | wxACCEL_ALT | wxACCEL_SHIFT, (int)'D', static_cast<int>(Event::Edit::DuplicateLineUp)},
        {wxACCEL_CTRL, (int)'D', static_cast<int>(Event::Edit::SelectNextOccurrence)}
    };

    wxAcceleratorTable accel(sizeof(entries) / sizeof(entries[0]), entries);
    SetAcceleratorTable(accel);

    m_AutoCompleteWordsList = LanguagesPreferences::Get().GetAutoCompleteWordsList(m_LanguagePreferences);
}

void Editor::ConfigureFoldMargin()
{
    const wxString backgroundColor(Theme["secondary"].template get<std::string>());
    
    SetMarginWidth(EditorConstants::FOLD_MARGIN, EditorConstants::FOLD_MARGIN_WIDTH);
    SetMarginType(EditorConstants::FOLD_MARGIN, wxSTC_MARGIN_SYMBOL);
    SetMarginMask(EditorConstants::FOLD_MARGIN, wxSTC_MASK_FOLDERS);
    SetMarginSensitive(EditorConstants::FOLD_MARGIN, true);

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
    if (event.GetString().IsEmpty() || GetModify() == false)
        return;

    if (GetModify()) {
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
    if (event.GetMargin() != EditorConstants::FOLD_MARGIN) {
        event.Skip();
        return;
    }

    const int line = LineFromPosition(event.GetPosition());
    const int foldLevel = GetFoldLevel(line);
    const bool isHeader = (foldLevel & wxSTC_FOLDLEVELHEADERFLAG) != 0;

    if (isHeader)
        ToggleFold(line);

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

    const long startPos = GetSelectionStart();
    const long endPos = GetSelectionEnd();
    const bool hasSelection = (startPos != endPos);

    if (hasSelection)
    {
        wxString selectedText = GetTextRange(startPos, endPos);
        if (selectedText.Length() == EditorConstants::MIN_SELECTION_LENGTH)
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

    const long pos = GetCurrentPos();
    if (pos == 0 && key == WXK_BACK) {
        event.Skip();
        return;
    }
    
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
    if (statusBar)
        statusBar->UpdateCodeLocale(this);
    event.Skip();
}

void Editor::OnClick(wxMouseEvent &event)
{
    event.Skip();
    ClearIndicators();
    if (statusBar)
        statusBar->UpdateCodeLocale(this);
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

bool Editor::Modified() const
{
    return GetModify() && !GetReadOnly() && IsModified();
}

void Editor::OnStyleNeeded(wxStyledTextEvent &WXUNUSED(event))
{
    const int currentLine = GetCurrentLine();
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
    
    const wxString text = GetTextRange(startPos, endPos).Upper();

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

            while (pos < text.length() && std::isdigit(text[pos]))
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
    return std::isspace(static_cast<unsigned char>(c)) != 0;
}

void Editor::ApplySyntaxHighlighting(size_t fromPos, size_t toPos, const std::vector<std::pair<size_t, size_t>> &blocks)
{
    StartStyling(fromPos);
    SetStyling(toPos - fromPos, STYLE_DEFAULT);
    for (const auto &block : blocks)
    {
        if (block.first >= fromPos && block.second <= toPos) {
            StartStyling(block.first);
            SetStyling(block.second - block.first, STYLE_KEYWORD);
        }
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
        
        bool isEndIf = false;
        if (pos >= 3) {
            const wxString segment = text.Mid(pos - 3, 5);
            if (segment.Cmp("ENDIF") == 0) {
                isEndIf = true;
            }
        }

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

    const int selStart = GetSelectionStart();
    const int selEnd = GetSelectionEnd();

    if (selEnd <= selStart)
        return;

    const wxString selectedText = GetTextRange(selStart, selEnd);
    const int selectedLength = selectedText.Length();

    if (selectedLength < EditorConstants::MIN_SELECTION_LENGTH || !std::isalnum(selectedText[0]))
        return;

    SetIndicatorCurrent(EditorConstants::INDICATOR_DEFAULT);

    int pos = 0;
    const int maxPos = GetTextLength();
    const int findFlags = wxSTC_FIND_MATCHCASE | wxSTC_FIND_WHOLEWORD;

    while (pos < maxPos)
    {
        int foundPos = FindText(pos, maxPos, selectedText, findFlags);
        if (foundPos == -1)
            break;

        if (foundPos == selStart && foundPos + selectedLength == selEnd) {
             pos = foundPos + selectedLength;
             continue;
        }
        
        IndicatorFillRange(foundPos, selectedLength);
        pos = foundPos + selectedLength;
    }
}

void Editor::ClearIndicators()
{
    const int textLength = GetTextLength();
    for (int indicator = 0; indicator <= EditorConstants::MAX_INDICATOR; indicator++)
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

    wxStaticBitmap *icon = nullptr;
    const wxString languageIconName = "tab_language_icon";
    const wxString unsavedIconLabel = "unsaved_icon";

    auto findIcon = [&](wxWindow *parent) -> wxStaticBitmap* {
        for (wxWindow *child : parent->GetChildren()) {
            if (auto *bmp = wxDynamicCast(child, wxStaticBitmap)) {
                if (bmp->GetName() != languageIconName) {
                    return bmp;
                }
            }
        }
        return nullptr;
    };

    icon = findIcon(tab);
    if (!icon) {
        for (wxWindow *child : tab->GetChildren()) {
            icon = findIcon(child);
            if (icon) break;
        }
    }

    if (icon && icon->GetLabel() != unsavedIconLabel && Modified())
    {
        icon->SetBitmap(wxBitmapBundle::FromBitmap(wxBitmap(iconsDir + "white_circle.png", wxBITMAP_TYPE_PNG)));
        icon->SetLabel(unsavedIconLabel);
		icon->Refresh();
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
    const int currentLine = GetCurrentLine();
    const int prevLine = currentLine - 1;
    const int prevLineInd = GetLineIndentation(prevLine);
    const int currentLineInd = GetLineIndentation(currentLine);
    const int nextLineInd = GetLineIndentation(currentLine + 1);
    const int indentSize = GetIndent();
    
    if (prevChar == '\n' && (nextChar == '}' || nextChar == ']'))
    {
        SetLineIndentation(currentLine, currentLineInd + indentSize);
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
        case '(': InsertText(GetCurrentPos(), ")"); break;
        case '{': InsertText(GetCurrentPos(), "}"); break;
        case '[': InsertText(GetCurrentPos(), "]"); break;
        case '"': InsertText(GetCurrentPos(), "\""); break;
        case '\'': InsertText(GetCurrentPos(), "'"); break;
        case '`': InsertText(GetCurrentPos(), "`"); break;
    }
}

void Editor::SetMiniMapLine()
{
    if (MiniMap)
    {
        MiniMap->SetFirstVisibleLine(GetFirstVisibleLine());
        MiniMap->Refresh();
    }
}

void Editor::OnMoveCursorDown(wxCommandEvent &WXUNUSED(event))
{
    const int currentLine = GetCurrentLine();
    const int lineEndPos = GetLineEndPosition(currentLine);

    GotoPos(lineEndPos);
    AddText("\n");
    EnsureCaretVisible();
    SetEmptySelection(GetCurrentPos());
}

void Editor::OnMoveCursorUp(wxCommandEvent &WXUNUSED(event))
{
    const int currentLine = GetCurrentLine();
    const int lineStartPos = PositionFromLine(currentLine);

    GotoPos(lineStartPos);
    InsertText(lineStartPos, "\n");

    if (currentLine > 0)
        GotoPos(PositionFromLine(currentLine));
    else
        GotoPos(0);

    EnsureCaretVisible();
    SetEmptySelection(GetCurrentPos());
}

void Editor::OnDuplicateLineDown(wxCommandEvent &WXUNUSED(event))
{
    const int line = GetCurrentLine();
    const wxString text = GetLine(line);
    const int insertPos = GetLineEndPosition(line);
    
    InsertText(insertPos, "\n" + text);
    GotoLine(line + 1);
    EnsureCaretVisible();
}

void Editor::OnDuplicateLineUp(wxCommandEvent &WXUNUSED(event))
{
    const int line = GetCurrentLine();
    const wxString text = GetLine(line);
    const int insertPos = PositionFromLine(line);
    
    InsertText(insertPos, text + "\n");
    if (line > 0)
        GotoLine(line);
    else
        GotoLine(0);
    
    EnsureCaretVisible();
}

void Editor::SelectNextOccurrence(wxCommandEvent &WXUNUSED(event))
{
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
    
    SetSearchFlags(wxSTC_FIND_WHOLEWORD | wxSTC_FIND_MATCHCASE);

    if (SearchInTarget(text) != -1)
    {
        const int foundStart = GetTargetStart();
        const int foundEnd = GetTargetEnd();

        AddSelection(foundStart, foundEnd);
        SetMainSelection(GetSelections() - 1);
        ScrollToLine(LineFromPosition(foundStart));
    }
}

static int FindLastCharBeforePos(wxStyledTextCtrl *ctrl, int ch, int pos)
{
    for (int i = pos; i >= 0; --i)
    {
        if (ctrl->GetCharAt(i) == ch)
            return i;
    }
    return -1;
}

static wxString ExtractTagName(wxStyledTextCtrl *ctrl, int openPos, int closePos)
{
    wxString tag;
    for (int i = openPos + 1; i < closePos; ++i)
    {
        const char c = static_cast<char>(ctrl->GetCharAt(i));
        if (c == '/' || std::isspace(static_cast<unsigned char>(c)) || c == '>')
            break;
        tag += c;
    }
    return tag;
}

void Editor::CharAdd(wxStyledTextEvent &event)
{
    ClearIndicators();
    UpdateMiniMapText();

    const char chr = static_cast<char>(event.GetKey());
    const int currentPos = GetCurrentPos();
    
    const char prevChar = (currentPos > 0) ? GetCharAt(currentPos - 1) : '\0';
    const char nextChar = GetCharAt(currentPos);

    if (std::isalnum(static_cast<unsigned char>(chr)) || chr == '_')
    {
        const int startPos = WordStartPosition(currentPos, true);
        const int lenEntered = currentPos - startPos;

        if (lenEntered > 0)
        {
            const wxString currentWord = GetTextRange(startPos, currentPos);
            wxString filtered;
            
            for (const auto &kw : m_AutoCompleteWordsList)
            {
                if (kw.StartsWith(currentWord)) {
                    filtered += kw + " ";
                }
            }

            if (!filtered.IsEmpty())
                AutoCompShow(lenEntered, filtered);
            else if (AutoCompActive())
                AutoCompCancel();
        }
    }

    if (chr == '\n')
    {
        HandleNewLineIndentation(prevChar, nextChar);
    }
    
    if (chr == '>')
    {
        if (m_LanguagePreferences.preferences.contains("syntax") &&
            m_LanguagePreferences.preferences["syntax"].contains("auto_close_tags") &&
            m_LanguagePreferences.preferences["syntax"]["auto_close_tags"].get<bool>())
        {
            const int searchPos = currentPos - 1;
            const int openPos = FindLastCharBeforePos(this, '<', searchPos);
            
            if (openPos != -1 && GetCharAt(searchPos - 1) != '/')
            {
                const wxString tag = ExtractTagName(this, openPos, searchPos);
                
                if (!tag.IsEmpty() && !tag.StartsWith("/"))
                {
                    const wxString closing = "\n\n</" + tag + ">";
                    InsertText(currentPos, closing);
                    
                    GotoPos(currentPos + 1);
                    EnsureCaretVisible();
                    SetEmptySelection(GetCurrentPos());
                }
            }
        }
    }

    HandleAutoPairing(chr);
    SetMiniMapLine();
    event.Skip();
}

void Editor::OnAutoCompCompleted(wxStyledTextEvent &WXUNUSED(event)) 
{
}