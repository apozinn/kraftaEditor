#pragma once

#include "themesManager/themesManager.hpp"
#include "appPaths/appPaths.hpp"
#include "projectSettings/projectSettings.hpp"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "ui/ids.hpp"
#include <wx/wx.h>
#include <wx/scrolwin.h>

class Gotoline : public wxPanel
{
public:
    Gotoline(wxFrame *parent);

    void Close(wxCommandEvent &WXUNUSED(event));
    void SearchInputModified(wxCommandEvent &WXUNUSED(event));

private:
    wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL); /**< Main sizer for the panel layout. */
    wxTextCtrl *searchInput; /**< The input field for filtering commands. */
    json Theme = ThemesManager::Get().currentTheme; /**< Cached theme settings. */
    ProjectSettings &projectSettings = ProjectSettings::Get();                        /**< Reference to global project settings. */
    
    wxDECLARE_NO_COPY_CLASS(Gotoline);
    wxDECLARE_EVENT_TABLE();
};