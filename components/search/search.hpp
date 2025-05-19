#pragma once

class Search : public wxPanel
{
    wxStyledTextCtrl *input;
    wxStyledTextCtrl *currentEditor = nullptr;
    wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);

public:
    Search(wxWindow *parent, wxString defaultLabel, wxStyledTextCtrl *codeEditor);

private:
    void EnterEvent(wxStyledTextEvent &event);
    void Close(wxCommandEvent &WXUNUSED(event));
    void OnPaint(wxPaintEvent& event);
    wxDECLARE_NO_COPY_CLASS(Search);
    wxDECLARE_EVENT_TABLE();
};

wxBEGIN_EVENT_TABLE(Search, wxPanel)
    EVT_MENU(ID_EXIT_SEARCH_CONTAINER, Search::Close)
        wxEND_EVENT_TABLE()