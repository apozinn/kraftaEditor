#pragma once

#include "themesManager/themesManager.hpp"
#include "appPaths/appPaths.hpp"
#include "userSettings/userSettings.hpp"
#include "languagesPreferences/languagesPreferences.hpp"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <wx/wx.h>
#include <wx/stc/stc.h>

class StatusBar : public wxPanel {
	wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
	json Theme = ThemesManager::Get().currentTheme;
	json UserSettings = UserSettingsManager::Get().currentSettings;
    wxString iconsDir = ApplicationPaths::AssetsPath("icons");
public:
	wxStaticText* codeLocale = nullptr;
	wxStaticText* tabSize = nullptr;
	wxStaticText* fileExt = nullptr;
	
	StatusBar(wxWindow* parent, wxWindowID ID);
	void UpdateComponents(wxString path, wxString format, wxString languageName);
	void UpdateCodeLocale(wxStyledTextCtrl* codeEditor);
	void ClearLabels();
	void OnPaint(wxPaintEvent& WXUNUSED(event));

	void UpdateLanguage(const languagePreferencesStruct& language);
};