#pragma once

#include "themesManager/themesManager.hpp"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <wx/wx.h>

class Terminal : public wxPanel
{
public:
    Terminal(wxWindow *parent, wxWindowID ID);
    void OnCommand(wxCommandEvent &WXUNUSED(event));
    void OnCommandInputClick(wxMouseEvent &WXUNUSED(event));
private:
    wxTextCtrl *m_commandInput;
	json Theme = ThemesManager::Get().currentTheme;
};