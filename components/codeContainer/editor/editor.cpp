#include "./editor.hpp"
#include "path/path.hpp"

#define MY_FOLDMARGIN 2

Editor::Editor(wxWindow *parent) : wxStyledTextCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE)
{
    currentPath = parent->GetLabel();
    InitializePrefs();
    SetFoldPreferences();
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

    // linking autocomp icons
    RegisterImage(0, wxBitmap(ApplicationPaths::IconsPath() + "thunder.png", wxBITMAP_TYPE_PNG));
    RegisterImage(1, wxBitmap(ApplicationPaths::IconsPath() + "question.png", wxBITMAP_TYPE_PNG));
}

void Editor::SetFoldPreferences()
{
    auto backgroundColor = UserTheme["secondary"].template get<std::string>();
    auto textColor = UserTheme["text"].template get<std::string>();
    auto secondaryTextColor = UserTheme["secondaryText"].template get<std::string>();
    
    SetMarginWidth(MY_FOLDMARGIN, 14);
    SetMarginType(MY_FOLDMARGIN, wxSTC_MARGIN_SYMBOL);
    SetMarginMask(MY_FOLDMARGIN, wxSTC_MASK_FOLDERS);
    SetFoldMarginColour(true, wxColor(backgroundColor));
    SetFoldMarginHiColour(true, wxColor(backgroundColor));

    std::vector<int> marks{wxSTC_MARKNUM_FOLDER,
                           wxSTC_MARKNUM_FOLDEREND,
                           wxSTC_MARKNUM_FOLDERMIDTAIL,
                           wxSTC_MARKNUM_FOLDEROPEN,
                           wxSTC_MARKNUM_FOLDEROPENMID,
                           wxSTC_MARKNUM_FOLDERSUB,
                           wxSTC_MARKNUM_FOLDERTAIL};
    for (int mark : marks)
    {
        MarkerSetBackgroundSelected(mark, wxColor(secondaryTextColor));
    }

    // Set up the markers that will be shown in the fold margin
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

    MarkerDefine(wxSTC_MASK_FOLDERS, wxSTC_MARK_LCORNER);
    MarkerSetForeground(wxSTC_MASK_FOLDERS, wxColor(backgroundColor));
    MarkerSetBackground(wxSTC_MASK_FOLDERS, wxColor(secondaryTextColor));

    // Turn the fold markers red when the caret is a line in the group (optional)
    MarkerEnableHighlight(true);

    // The margin will only respond to clicks if it set sensitive.  Also, connect
    // the event handler that will do the collapsing/restoring
    SetMarginSensitive(MY_FOLDMARGIN, true);

    Bind(wxEVT_STC_STYLENEEDED, &Editor::OnStyleNeeded, this);
    Bind(wxEVT_STC_MODIFIED, &Editor::OnChange, this);
    Bind(wxEVT_STC_MARGINCLICK, &Editor::OnMarginClick, this);
    Bind(wxEVT_KEY_UP, &Editor::OnArrowsPress, this);
    Bind(wxEVT_STC_CHARADDED, &Editor::CharAdd, this);
    Bind(wxEVT_LEFT_UP, &Editor::OnClick, this);
    Bind(wxEVT_STC_AUTOCOMP_COMPLETED, &Editor::OnAutoCompCompleted, this);
    Bind(wxEVT_STC_CLIPBOARD_PASTE, &Editor::OnClipBoardPaste, this);
    Bind(wxEVT_MOUSEWHEEL, &Editor::OnScroll, this);

    // set color for G-Code highlighting
    StyleSetForeground(19, m_GCodecolor);

    // given the text above, folding should produce this output:
    SetFoldLevel(0, 1024);
    SetFoldLevel(1, 1024);
    SetFoldLevel(2, 1024 | wxSTC_FOLDLEVELHEADERFLAG); // header flag: one item before increasing fold level!
    SetFoldLevel(3, 1025);                             // here comes the new fold level in line G0 G54 M0
    SetFoldLevel(4, 1025);                             // the ENDIF
    SetFoldLevel(5, 1024);                             // and this has the lower fold level again
    SetFoldLevel(6, 1024 | wxSTC_FOLDLEVELWHITEFLAG);  // this is an empty line: set fold level white flag
                                                       // setting the fold
    SetProperty(wxT("fold"), wxT("1"));
    SetProperty(wxT("fold.comment"), wxT("1"));
    SetProperty(wxT("fold.compact"), wxT("1"));
    SetProperty(wxT("fold.preprocessor"), wxT("1"));
    SetProperty(wxT("fold.html"), wxT("1"));
    SetProperty(wxT("fold.html.preprocessor"), wxT("1"));
    SetFoldFlags(wxSTC_FOLDFLAG_LINEBEFORE_CONTRACTED | wxSTC_FOLDFLAG_LINEAFTER_CONTRACTED);
}

void Editor::OnChange(wxStyledTextEvent &event)
{
    if (event.GetString() == wxEmptyString || GetText() == event.GetString())
        return;

    if (!changedFile && GetModify())
    {
        auto tab = FindWindowByLabel(current_openned_path + "_tab");
        if (tab)
        {
            auto icon = ((wxStaticBitmap *)tab->GetChildren()[0]->GetChildren()[2]);
            if (icon)
            {
                // setting the unsaved icon
                icon->SetBitmap(wxBitmapBundle::FromBitmap(wxBitmap(ApplicationPaths::IconsPath() + "white_circle.png", wxBITMAP_TYPE_PNG)));
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

void Editor::OnBackspace(wxKeyEvent &event)
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
    int nextLine = GetCurrentLine() + 1;

    int previousLineInd = GetLineIndentation(previousLine);
    int currentLineInd = GetLineIndentation(currentLine);
    int nextLineInd = GetLineIndentation(currentLine + 1);

    if (chr == '\n')
    {
        if (previous_char == '\n' && next_char == '}' || previous_char == '\n' && next_char == ']')
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

bool Editor::Modified()
{
    return (GetModify() && !GetReadOnly());
}

void Editor::OnStyleNeeded(wxStyledTextEvent &event)
{
    /*this is called every time the styler detects a line that needs style, so we style that range.
    This will save a lot of performance since we only style text when needed instead of parsing the whole file every time.*/
    size_t line_end = LineFromPosition(GetCurrentPos());
    size_t line_start = LineFromPosition(GetEndStyled());
    /*fold level: May need to include the two lines in front because of the fold level these lines have- the line above
    may be affected*/
    if (line_start > 1)
    {
        line_start -= 2;
    }
    else
    {
        line_start = 0;
    }
    // if it is so small that all lines are visible, style the whole document
    if (GetLineCount() == LinesOnScreen())
    {
        line_start = 0;
        line_end = GetLineCount() - 1;
    }
    if (line_end < line_start)
    {
        // that happens when you select parts that are in front of the styled area
        size_t temp = line_end;
        line_end = line_start;
        line_start = temp;
    }
    // style the line following the style area too (if present) in case fold level decreases in that one
    if (line_end < GetLineCount() - 1)
    {
        line_end++;
    }
    // get exact start positions
    size_t startpos = PositionFromLine(line_start);
    size_t endpos = (GetLineEndPosition(line_end));
    int startfoldlevel = GetFoldLevel(line_start);
    startfoldlevel &= wxSTC_FOLDFLAG_LEVELNUMBERS; // mask out the flags and only use the fold level
    wxString text = GetTextRange(startpos, endpos).Upper();
    // call highlighting function
    this->highlightSTCsyntax(startpos, endpos, text);
    // calculate and apply foldings
    this->setfoldlevels(startpos, startfoldlevel, text);
}

void Editor::highlightSTCsyntax(size_t fromPos, size_t toPos, wxString &text)
{
    // this vector will hold the start and end position of each word to highlight
    // if you want to highlight more than one, you should pass a whole class or struct containing the offsets
    std::vector<std::pair<size_t, size_t>> GcodeVector;
    // the following example is a quick and dirty parser for G-Codes.
    // it just iterates through the Text Range and finds "Gxx" where xx is a digit.
    // you could also use regex, but one can build a pretty fast routine based on single char evaluation
    size_t actual_cursorpos = 0;
    size_t startpos = 0;
    size_t end_of_text = text.length();
    bool word_boundary = true; // check for word boundary
    char actualchar;
    while (actual_cursorpos < end_of_text)
    {
        actualchar = text[actual_cursorpos];
        // check if syntax matches "G" followed by a couple of numbers
        if ((actualchar == 'G') && (word_boundary == true))
        {
            // this is a new G-Code, store startposition
            startpos = actual_cursorpos;
            word_boundary = false;
            actual_cursorpos++;
            if (actual_cursorpos < end_of_text)
            {
                // refresh actual character
                actualchar = text[actual_cursorpos];
            }
            // add digits
            while ((std::isdigit(actualchar) && (actual_cursorpos < end_of_text)))
            {
                actual_cursorpos++;
                actualchar = text[actual_cursorpos];
            }
            // check if word boundary occurs at end of digits
            if ((actualchar == ' ') || (actualchar == '\n') || (actualchar == '\r') || (actualchar == '\t') || (actual_cursorpos == end_of_text))
            {
                // success, append this one
                if ((actual_cursorpos - startpos) > 1)
                {
                    // success, append to vector. DO NOT FORGET THE OFFSET HERE! We start from fromPos, so we need to add this
                    GcodeVector.push_back(std::make_pair(startpos + fromPos, actual_cursorpos + fromPos));
                }
                word_boundary = true;
            }
        }
        if ((actualchar == ' ') || (actualchar == '\n') || (actualchar == '\r') || (actualchar == '\t') || (actual_cursorpos == end_of_text))
        {
            word_boundary = true;
        }
        actual_cursorpos++;
    }
    // remove old styling
    StartStyling(fromPos);          // from here
    SetStyling(toPos - fromPos, 0); // with that length and style -> cleared
    // now style the G-Codes
    for (int i = 0; i < GcodeVector.size(); i++)
    {
        size_t startpos = GcodeVector[i].first;
        size_t endpos = GcodeVector[i].second;
        size_t length = (endpos - startpos);
        StartStyling(startpos);
        SetStyling(length, 19); // must match the style set above
    }
}

void Editor::setfoldlevels(size_t fromPos, int startfoldlevel, wxString &text)
{
    /*we'll increase the fold level with "IF" and decrease it with "ENDIF".
    First, find all "IF" included in the text. Then we check if this IF is actually an ENDIF.
    Keep in mind that you still need to check if this is actually commented out and so on.
    This is a pretty simple and not perfect example to demonstrate basic folding*/
    std::vector<size_t> if_positions;
    size_t actual_cursorpos = 0;
    while ((actual_cursorpos < text.size()) && (actual_cursorpos != wxNOT_FOUND))
    {
        actual_cursorpos = text.find("IF", actual_cursorpos + 1);
        if (actual_cursorpos != wxNOT_FOUND)
        {
            if_positions.push_back(actual_cursorpos + fromPos);
        }
    }
    // build a vector to include line and folding level
    // also, check if this IF is actually an ENDIF
    std::vector<std::pair<size_t, int>> foldingvector;
    int actualfoldlevel = startfoldlevel;
    for (int i = 0; i < if_positions.size(); i++)
    {
        size_t this_line = LineFromPosition(if_positions[i]);
        // check if that "IF" is an ENDIF
        wxString endif_string;
        if (if_positions[i] > 3)
        { // if string is longer than 3
            endif_string = text.substr(if_positions[i] - 3 - fromPos, 5);
        }
        // if it's an IF the fold level increases, if it's an ENDIF the foldlevel decreases
        if (endif_string == "ENDIF")
        {
            actualfoldlevel--;
            foldingvector.push_back(std::make_pair(this_line, actualfoldlevel));
        }
        else
        {
            actualfoldlevel++;
            foldingvector.push_back(std::make_pair(this_line, actualfoldlevel));
        }
    }
    // now that we know which lines shall influence folding, we can apply to folding level to the STC line for line
    int foldlevel = startfoldlevel; // this is a temporary marker containing the foldlevel of that position
    size_t vectorcount = 0;
    // get positions from line from start and end
    size_t startline = LineFromPosition(fromPos);
    size_t endline = LineFromPosition(fromPos + text.size());
    // set folding for these lines
    for (size_t i = startline; i <= endline; i++)
    {
        int prevlevel = foldlevel; // previous foldlevel
        int foldflag = foldlevel;  // this flag will be applied to the line
        if ((foldingvector.size() > 0) && (vectorcount < foldingvector.size()))
        {
            if (i == foldingvector[vectorcount].first)
            { // if the fold level changes in that line
                // new foldlevel = foldlevel in that line
                foldlevel = foldingvector[vectorcount].second;
                vectorcount++;
                if (foldlevel > prevlevel)
                {
                    // when incremented, this line keeps the previous fold level (!) but is marked as a folder level header
                    foldflag = foldflag | wxSTC_FOLDLEVELHEADERFLAG; // incremented, set header flag
                }
            }
        }
        foldflag = foldflag | wxSTC_FOLDLEVELBASE; // add 1024 to the fold level
        if (GetLineLength(i) == 0)
        { // if this does not contain any characters, set the white flag
            foldflag = foldflag | wxSTC_FOLDLEVELWHITEFLAG;
        }
        // finally, set fold level to line
        SetFoldLevel(i, foldflag);
    }
}