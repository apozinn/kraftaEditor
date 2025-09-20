#include "codeSearch.hpp"

wxBEGIN_EVENT_TABLE(Search, wxPanel)
EVT_MENU( +Event::CodeSearch::Close, Search::Close)
wxEND_EVENT_TABLE()

Search::Search(
    wxWindow *parent,
    wxString defaultLabel,
    wxStyledTextCtrl *codeEditor) : wxPanel(parent, +GUI::ControlID::CodeSearchPanel,
                                            wxPoint(parent->GetSize().GetWidth() - 350, 50),
                                            wxSize(250, 50))
{
    SetMinSize(wxSize(250, 50));
    SetSize(wxSize(250, 50));
    SetBackgroundColour(ThemesManager::Get().GetColor("main"));
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
    entries[0].Set(wxACCEL_NORMAL, WXK_ESCAPE, +Event::CodeSearch::Close);
    wxAcceleratorTable accel(1, entries);
    SetAcceleratorTable(accel);

    input->Bind(wxEVT_STC_CHARADDED, &Search::EnterEvent, this);
    Bind(wxEVT_PAINT, &Search::OnPaint, this);
}

void Search::EnterEvent(wxStyledTextEvent &event)
{
    char chr = (char)event.GetKey();
    if (chr == '\n')
    {
        if (input->GetEOLMode() == wxSTC_EOL_CRLF)
        {
            input->Remove(input->GetLength() - 2, input->GetLength());
        }
        else
        {
            input->Remove(input->GetLength() - 1, input->GetLength());
        }

        if (currentEditor)
        {
            wxString textTarget = input->GetText();
            int currentEditorLength = currentEditor->GetLength();

            currentEditor->IndicatorSetStyle(0, wxSTC_INDIC_COMPOSITIONTHICK);
            currentEditor->IndicatorClearRange(0, currentEditorLength);

            int start = currentEditor->SearchNext(0, textTarget);
            currentEditor->GotoPos(start + textTarget.Length());
            currentEditor->SearchAnchor();
            currentEditor->SetTargetRange(start + textTarget.Length(), currentEditorLength);

            if (start != -1)
            {
                currentEditor->IndicatorSetForeground(0, wxColor(255, 0, 0));
                currentEditor->IndicatorFillRange(start, textTarget.Length());
            }

            currentEditor->SetIndicatorValue(10);

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
            currentEditor->IndicatorSetAlpha(0, 63);
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

void Search::OnPaint(wxPaintEvent &WXUNUSED(event))
{
    Refresh();
}