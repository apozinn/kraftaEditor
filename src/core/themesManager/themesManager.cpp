#include "themesManager/themesManager.hpp"
#include "appPaths/appPaths.hpp"
#include "themesManager/themeApplier.hpp"
#include "userSettings/userSettings.hpp"
#include <fstream>
#include <nlohmann/json.hpp>
#include <wx/colour.h>
#include <wx/file.h>
#include <wx/font.h>
#include <wx/log.h>
#include <wx/msgdlg.h>
#include <wx/settings.h>

using json = nlohmann::json;

json DarkTheme = {{"themeName", "Standard Krafta Dark Theme"},
				  {"isDark", true},
				  {"main", "#292929"},
				  {"secondary", "#1C1C1C"},
				  {"colorThree", "#454545"},
				  {"icon", "#e7e7e7"},
				  {"unsavedIcon", "#e7e7e7"},
				  {"highlight", "#008ff5"},
				  {"border", "#5F5F5F"},
				  {"tint", "#00b5cc"},
				  {"selectedFile", "#3c3c3c"},
				  {"text", "#e7e7e7"},
				  {"secondaryText", "#797979"},
				  {"indicator", {{"foreground", "#89cde8"}}},
				  {"editorCaret", "#dedede"},
				  {"minimapViewport", "#757575"},
				  {"minimapCaretPosition", "#c3dde0"}};

json LightTheme = {{"themeName", "Standard Krafta Light Theme"},
				   {"isDark", false},
				   {"main", "#ffffff"},
				   {"secondary", "#f3f3f3"},
				   {"colorThree", "#e8e8e8"},
				   {"icon", "#1f1f1f"},
				   {"unsavedIcon", "#1f1f1f"},
				   {"highlight", "#0066cc"},
				   {"border", "#e0e0e0"},
				   {"tint", "#0090b8"},
				   {"selectedFile", "#e4e6f1"},
				   {"text", "#1f1f1f"},
				   {"secondaryText", "#6a6a6a"},
				   {"indicator", {{"foreground", "#4fa8d8"}}},
				   {"editorCaret", "#333333"},
				   {"minimapViewport", "#c8c8c8"},
				   {"minimapCaretPosition", "#7fc4d0"}};

ThemesManager &ThemesManager::Get() {
	static ThemesManager instance;
	return instance;
}

wxColor ThemesManager::GetColor(const std::string &key) const {
	try {
		const auto &color = currentTheme.at(key).get<std::string>();
		return wxColor(color);
	} catch (...) {
		wxMessageBox(_("Color '%s' not found - using default"), key);
		return wxColor(0, 0, 0);
	}
}

wxFont ThemesManager::GetFont(const std::string &key) const {
	try {
		const auto &font = currentTheme.at("fonts").at(key);
		return wxFont(font.value("size", 12), wxFONTFAMILY_DEFAULT,
					  font.value("italic", false) ? wxFONTSTYLE_ITALIC
												  : wxFONTSTYLE_NORMAL,
					  font.value("bold", false) ? wxFONTWEIGHT_BOLD
												: wxFONTWEIGHT_NORMAL,
					  false, font.value("family", "Arial"));
	} catch (...) {
		wxMessageBox(_("Font '%s' not found - using default"), key);
		return wxFont(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
					  wxFONTWEIGHT_NORMAL);
	}
}

bool ThemesManager::Reload() {
	try {
		currentTheme = LoadThemeFile();
		return true;
	} catch (...) {
		return false;
	}
}

nlohmann::json ThemesManager::LoadThemeFile() const {
	const bool isSystemDark = wxSystemSettings::GetAppearance().IsSystemDark();
	const wxString defaultThemeFile =
		ApplicationPaths::AssetsPath("themes") +
		(isSystemDark ? "standard_dark.json" : "standard_light.json");

	wxString themePath;

	auto userSetting = UserSettingsManager::Get().GetSetting<std::string>(
		"theme/themeFilePath");

	if (userSetting.found && wxFileExists(userSetting.value)) {
		themePath = userSetting.value;
	} else {
		if (userSetting.found) {
			wxLogError(
				_("Saved theme file not found, falling back to default."));
		}

		themePath = defaultThemeFile;

		if (!wxFileExists(themePath)) {
			std::ofstream fileStream(themePath.ToStdString());
			if (!fileStream) {
				wxLogError(wxString::Format(
					_("Failed to create theme file at: %s"), themePath));
				return isSystemDark ? DarkTheme : LightTheme;
			}

			fileStream << std::setw(4)
					   << (isSystemDark ? DarkTheme : LightTheme) << std::endl;
		}

		UserSettingsManager::Get().SetSetting<std::string>(
			"theme/themeFilePath", themePath.ToStdString());
	}

	try {
		std::ifstream file(themePath.ToStdString());
		return nlohmann::json::parse(file);
	} catch (const std::exception &e) {
		wxLogError(wxString::Format(_("Failed to load theme: %s"), e.what()));
		return isSystemDark ? DarkTheme : LightTheme;
	}
}

bool ThemesManager::IsDarkTheme() const {

	if (currentTheme.contains("isDark")) {
		return currentTheme["isDark"].get<bool>();
	}

	return wxSystemSettings::GetAppearance().IsSystemDark();
}