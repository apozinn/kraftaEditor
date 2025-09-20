#pragma once

#include "ui/ids.hpp"
#include "themesManager/themesManager.hpp"

#include <wx/wx.h>
#include <wx/stc/stc.h>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

class Search : public wxPanel
{
    wxStyledTextCtrl *input;
    wxStyledTextCtrl *currentEditor = nullptr;
    wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
    json Theme = ThemesManager::Get().currentTheme;

public:
    Search(wxWindow *parent, wxString defaultLabel, wxStyledTextCtrl *codeEditor);

private:
    void EnterEvent(wxStyledTextEvent &event);
    void Close(wxCommandEvent &WXUNUSED(event));
    void OnPaint(wxPaintEvent &WXUNUSED(event));
    wxDECLARE_NO_COPY_CLASS(Search);
    wxDECLARE_EVENT_TABLE();
};