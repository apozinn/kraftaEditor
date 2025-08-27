#pragma once

#include "themesManager/themesManager.hpp"
#include "appPaths/appPaths.hpp"
#include "appConstants/appConstants.hpp"
#include "projectSettings/projectSettings.hpp"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <vector>
#include <wx/wx.h>

class Tabs : public wxPanel
{
	wxSizer *sizer;
	wxStaticBitmap *menu;
	wxScrolled<wxPanel> *tabsContainer;
	wxBoxSizer *tabsContainerSizer;
	json Theme = ThemesManager::Get().currentTheme;
    wxString iconsDir = ApplicationPaths::AssetsPath("icons");
    ProjectSettings& projectSettings = ProjectSettings::Get();
public:
	wxString selected_tab;
	Tabs(wxPanel *parent, wxWindowID ID);
	void Add(wxString tab_name, wxString path);
	void Close(wxWindow *tab, wxString tab_path);
	void CloseAllFiles();
	void Select();
	void OnTabClick(wxMouseEvent &event);
	void OnCloseTab(wxMouseEvent &event);
	void OnMenu(wxMouseEvent &WXUNUSED(event));

private:
	void OnEnterComp(wxMouseEvent &event);
	void OnLeaveComp(wxMouseEvent &event);
	void OnPaint(wxPaintEvent &WXUNUSED(event));
	void OnTabPaint(wxPaintEvent &event);
	void OnPreviousTab();
	void OnNextTab();
	wxDECLARE_NO_COPY_CLASS(Tabs);
};