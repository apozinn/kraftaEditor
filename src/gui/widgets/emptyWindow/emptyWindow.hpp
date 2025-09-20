#pragma once

#include "themesManager/themesManager.hpp"
#include "appPaths/appPaths.hpp"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <wx/wx.h>

class EmptyWindow : public wxPanel
{
	wxBoxSizer* sizer;
    json Theme = ThemesManager::Get().currentTheme;
    wxString assetsDir = ApplicationPaths::AssetsPath("images");
public:
	EmptyWindow(wxWindow* parent, wxWindowID ID);
};