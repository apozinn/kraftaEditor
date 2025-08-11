#pragma once

#include "themesManager/themesManager.hpp"
#include "appPaths/appPaths.hpp"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "ui/ids.hpp"
#include <wx/wx.h>

struct ControlMenu
{
    const char *name;
    const char *shortkut;
    int ID;
};

class ControlPanel : public wxPanel
{
    wxScrolled<wxPanel> *menusContainer;
    wxPanel *selectedMenu;
    wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
    wxTextCtrl *searchInput;
    json Theme = ThemesManager::Get().currentTheme;
    wxString iconsDir = ApplicationPaths::AssetsPath("icons");

public:
    ControlPanel(wxFrame *parent, wxWindowID ID);
    void Close(wxCommandEvent &WXUNUSED(event));
    void UpSelection(wxCommandEvent &WXUNUSED(event));
    void DownSelection(wxCommandEvent &WXUNUSED(event));
    void OnClickSelect(wxMouseEvent &event);
    void Select(wxString id);
    void SearchInputModified(wxCommandEvent &WXUNUSED(event));
    void OnKeyBoardSelect(wxCommandEvent &WXUNUSED(event));

private:
    std::vector<ControlMenu> menus{
        {"Open Terminal", "Ctrl+Shift+T", 1},
        {"Open sla", "Ctrl+Shift+T", 2},
    };
    wxDECLARE_NO_COPY_CLASS(ControlPanel);
    wxDECLARE_EVENT_TABLE();
};
