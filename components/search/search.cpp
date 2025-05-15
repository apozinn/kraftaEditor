#include "search.hpp"

Search::Search(
    wxWindow *parent,
    wxString defaultLabel,
    wxStyledTextCtrl *codeEditor) : wxPanel(parent, ID_SEARCH_CONTAINER,
                                            wxPoint(parent->GetSize().GetWidth() - 350, 50),
                                            wxSize(250, 50))
{
    SetMinSize(wxSize(250, 50));
    SetSize(wxSize(250, 50));
    SetBackgroundColour(wxColor(UserTheme["main"].template get<std::string>()));
    currentEditor = codeEditor;

    input = new wxStyledTextCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
    input->SetSize(wxSize(225, 25));
    input->SetPosition(wxPoint(10, 13));
    input->SetText(defaultLabel);

    input->SetWrapMode(wxSTC_WRAP_NONE);

    input->StyleSetBackground(wxSTC_STYLE_DEFAULT, wxColor(36, 36, 36));
    input->StyleSetForeground(wxSTC_STYLE_DEFAULT, wxColor(255, 255, 255));
    input->StyleClearAll();

    input->SetMarginWidth(0, 0);
    input->SetMarginWidth(1, 0);
    input->SetUseVerticalScrollBar(false);
    input->SetUseHorizontalScrollBar(false);

    input->SetCaretForeground(wxColour(wxT("WHITE")));

    input->SetFocus();
    input->GotoPos(input->GetLength());

    input->SetHotspotSingleLine(true);

    SetSizer(sizer);

    wxAcceleratorEntry entries[1];
    entries[0].Set(wxACCEL_NORMAL, WXK_ESCAPE, ID_EXIT_SEARCH_CONTAINER);
    wxAcceleratorTable accel(1, entries);
    SetAcceleratorTable(accel);

    input->Bind(wxEVT_STC_CHARADDED, &Search::EnterEvent, this);
}

void Search::EnterEvent(wxStyledTextEvent &event)
{
    char chr = (char)event.GetKey();
    if (chr == '\n')
    {
        input->Remove(input->GetLength() - 2, input->GetLength());
        if (currentEditor)
        {
            wxString textTarget = input->GetText();
            int currentEditorLength = currentEditor->GetLength();

            currentEditor->IndicatorSetStyle(0, wxSTC_INDIC_BOX);
            currentEditor->IndicatorClearRange(0, currentEditorLength);

            int start = currentEditor->SearchNext(0, textTarget);
            currentEditor->GotoPos(start + textTarget.Length());
            currentEditor->SearchAnchor();
            currentEditor->SetTargetRange(start + textTarget.Length(), currentEditorLength);

            if (start != -1)
                currentEditor->IndicatorFillRange(start, textTarget.Length());

            while (start != -1)
            {
                start = currentEditor->SearchNext(0, textTarget);
                if (start != -1)
                {
                    currentEditor->IndicatorFillRange(start, textTarget.Length());
                    currentEditor->GotoPos(start + textTarget.Length());
                    currentEditor->SearchAnchor();
                }
            }

            currentEditor->IndicatorSetForeground(0, wxColor(255, 0, 0));
        }
    }
}

void Search::Close(wxCommandEvent &WXUNUSED(event))
{
    if (currentEditor)
    {
        currentEditor->IndicatorClearRange(0, currentEditor->GetLength());
    }
    Destroy();
}