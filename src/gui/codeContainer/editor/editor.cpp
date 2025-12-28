#include "./editor.hpp"
#include "appConstants/appConstants.hpp"
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <cctype>

Editor::Editor(wxWindow *parent)
    : wxStyledTextCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE)
{
    currentPath = parent ? parent->GetLabel() : wxString{};
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
    SetEndAtLastLine(true);

    StyleSetBackground(wxSTC_STYLE_DEFAULT, wxColor(backgroundColor));
    StyleSetForeground(wxSTC_STYLE_DEFAULT, wxColor(textColor));
    StyleClearAll();

    SetCaretForeground(ThemesManager::Get().GetColor("editorCaret"));
    SetCaretWidth(3);

    SetMarginWidth(EditorConstants::LINE_NUMBER_MARGIN,
                   TextWidth(wxSTC_STYLE_LINENUMBER, wxT("_99999")));
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
        {wxACCEL_CTRL, (int)'D', static_cast<int>(Event::Edit::SelectNextOccurrence)}};

    SetAcceleratorTable(wxAcceleratorTable(WXSIZEOF(entries), entries));

    m_AutoCompleteWordsList =
        LanguagesPreferences::Get().GetAutoCompleteWordsList(m_LanguagePreferences);
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
    SetFoldFlags(wxSTC_FOLDFLAG_LINEAFTER_CONTRACTED |
                 wxSTC_FOLDFLAG_LINEBEFORE_EXPANDED);
}

void Editor::OnUpdateUI(wxStyledTextEvent &event)
{
    HighlightSelectionOccurrences();

    if (m_linked_minimap)
        m_linked_minimap->Refresh();

    event.Skip();
}

void Editor::OnChange(wxStyledTextEvent &event)
{
    if (!GetModify())
    {
        event.Skip();
        return;
    }

    changedFile = true;
    UpdateUnsavedIndicator();

    if (statusBar)
        statusBar->UpdateCodeLocale(this);

    event.Skip();
}

void Editor::OnMarginClick(wxStyledTextEvent &event)
{
    if (event.GetMargin() != EditorConstants::FOLD_MARGIN)
    {
        event.Skip();
        return;
    }

    const int line = LineFromPosition(event.GetPosition());
    const int level = GetFoldLevel(line);

    if (level & wxSTC_FOLDLEVELHEADERFLAG)
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

    const long start = GetSelectionStart();
    const long end = GetSelectionEnd();

    if (start != end)
    {
        Remove(start, end);
        return;
    }

    const long pos = GetCurrentPos();

    if (pos <= 0 && key == WXK_BACK)
    {
        event.Skip();
        return;
    }

    const wxString prevChar = pos > 0 ? GetTextRange(pos - 1, pos) : "";
    const wxString nextChar = pos < GetLength() ? GetTextRange(pos, pos + 1) : "";

    if (key == WXK_BACK)
    {
        auto it = kPairMap.find(nextChar);
        if (it != kPairMap.end() && it->second == prevChar)
        {
            Remove(pos - 1, pos + 1);
            return;
        }
    }

    if (key == WXK_DELETE)
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

    if (statusBar)
        statusBar->UpdateCodeLocale(this);

    event.Skip();
}

void Editor::OnClick(wxMouseEvent &event)
{
    ClearIndicators();

    if (statusBar)
        statusBar->UpdateCodeLocale(this);

    event.Skip();
}

void Editor::OnScroll(wxMouseEvent &event)
{
    if (event.ShiftDown())
    {
        OnHorizontalScroll(event);
    }
    else
    {
        event.Skip();
    }
}

bool Editor::Modified() const
{
    return GetModify() && !GetReadOnly();
}

void Editor::HighlightSelectionOccurrences()
{
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

    while (pos < max)
    {
        int found = FindText(pos, max, text,
                             wxSTC_FIND_MATCHCASE | wxSTC_FIND_WHOLEWORD);
        if (found == -1)
            break;

        if (found != start)
            IndicatorFillRange(found, text.Length());

        pos = found + text.Length();
    }
}

void Editor::ClearIndicators()
{
    const int len = GetTextLength();

    for (int i = 0; i <= EditorConstants::MAX_INDICATOR; ++i)
    {
        SetIndicatorCurrent(i);
        IndicatorClearRange(0, len);
    }

    Refresh(false);
}

void Editor::UpdateUnsavedIndicator()
{
    wxWindow *tab = FindWindowByLabel(ProjectSettings::Get().GetCurrentlyFileOpen() + "_tab");

    if (!tab || !Modified())
        return;

    wxStaticBitmap *icon = nullptr;

    for (auto children : tab->GetChildren()[0]->GetChildren())
    {
        if (children->GetName() == "tab_icon_close_or_unsaved")
        {
            icon = wxDynamicCast(children, wxStaticBitmap);
            break;
        }
    }

    icon->SetBitmap(
        wxBitmapBundle::FromBitmap(
            wxBitmap(
                iconsDir + "unsaved" +
                    (ThemesManager::Get().IsDarkTheme() ? "_light" : "_dark") +
                    ".png",
                wxBITMAP_TYPE_PNG)));

    tab->Layout();
}

void Editor::HandleAutoPairing(char chr)
{
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

static int FindLastCharBeforePos(wxStyledTextCtrl *ctrl, int ch, int pos)
{
    for (int i = pos; i >= 0; --i)
        if (ctrl->GetCharAt(i) == ch)
            return i;

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

    const char chr = static_cast<char>(event.GetKey());
    const int pos = GetCurrentPos();

    if (std::isalnum(static_cast<unsigned char>(chr)) || chr == '_')
    {
        const int start = WordStartPosition(pos, true);
        const int len = pos - start;

        if (len > 0)
        {
            const wxString word = GetTextRange(start, pos);
            wxString list;
            list.reserve(256);

            for (const auto &kw : m_AutoCompleteWordsList)
                if (kw.StartsWith(word))
                    list << kw << ' ';

            if (!list.empty())
                AutoCompShow(len, list);
            else
                AutoCompCancel();
        }
    }

    if (chr == '>' && pos > 1)
    {
        const auto &prefs = m_LanguagePreferences.preferences;

        if (prefs.contains("syntax") &&
            prefs["syntax"].value("auto_close_tags", false))
        {
            const int openPos = FindLastCharBeforePos(this, '<', pos - 1);

            if (openPos != -1 && GetCharAt(pos - 2) != '/')
            {
                const wxString tag = ExtractTagName(this, openPos, pos - 1);

                if (!tag.empty())
                {
                    InsertText(pos, "\n\n</" + tag + ">");
                    GotoPos(pos + 1);
                }
            }
        }
    }

    HandleAutoPairing(chr);
    event.Skip();
}

void Editor::OnMoveCursorDown(wxCommandEvent &WXUNUSED(event))
{
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

void Editor::OnMoveCursorUp(wxCommandEvent &WXUNUSED(event))
{
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

void Editor::OnDuplicateLineDown(wxCommandEvent &WXUNUSED(event))
{
    const int line = GetCurrentLine();
    const int lineStart = PositionFromLine(line);
    const int lineEnd = GetLineEndPosition(line);
    const wxString text = GetTextRange(lineStart, lineEnd);

    BeginUndoAction();

    InsertText(lineEnd, "\n" + text);

    EndUndoAction();

    GotoLine(line + 1);
    SetEmptySelection(GetCurrentPos());
    EnsureCaretVisible();
}

void Editor::OnDuplicateLineUp(wxCommandEvent &WXUNUSED(event))
{
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
    SetSearchFlags(wxSTC_FIND_MATCHCASE | wxSTC_FIND_WHOLEWORD);

    if (SearchInTarget(text) == -1)
        return;

    const int foundStart = GetTargetStart();
    const int foundEnd = GetTargetEnd();

    AddSelection(foundStart, foundEnd);
    SetMainSelection(GetSelections() - 1);
    EnsureCaretVisible();
}

void Editor::MoveSelectedLinesUp()
{
    int selStart = GetSelectionStart();
    int selEnd = GetSelectionEnd();

    bool hasSelection = (selStart != selEnd);

    int startLine;
    int endLine;

    if (!hasSelection)
    {
        startLine = endLine = LineFromPosition(GetCurrentPos());
    }
    else
    {
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

void Editor::MoveSelectedLinesDown()
{
    int selStart = GetSelectionStart();
    int selEnd = GetSelectionEnd();

    bool hasSelection = (selStart != selEnd);

    int startLine;
    int endLine;

    if (!hasSelection)
    {
        startLine = endLine = LineFromPosition(GetCurrentPos());
    }
    else
    {
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

void Editor::RemoveCurrentLine()
{
    if (GetSelectionStart() != GetSelectionEnd())
    {
        ReplaceSelection("");
        return;
    }

    const int currentLine = GetCurrentLine();
    const int lineStart = PositionFromLine(currentLine);
    int lineEnd = GetLineEndPosition(currentLine);

    const int lastLine = GetLineCount() - 1;
    if (currentLine < lastLine)
    {
        lineEnd++;
    }

    BeginUndoAction();
    SetTargetStart(lineStart);
    SetTargetEnd(lineEnd);
    ReplaceTarget("");
    EndUndoAction();
}

void Editor::OnHorizontalScroll(wxMouseEvent &event)
{
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