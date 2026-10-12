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
	std::string userSettingsThemeFilePath =
		UserSettingsManager::Get()
			.GetSetting<std::string>("theme/themeFilePath")
			.value;
	wxString themePath;

	if (wxFileExists(userSettingsThemeFilePath)) {
		themePath = userSettingsThemeFilePath;
	} else {
		if (wxSystemSettings::GetAppearance().IsSystemDark()) {
			themePath =
				ApplicationPaths::AssetsPath("themes") + "standard_dark.json";
		} else {
			themePath =
				ApplicationPaths::AssetsPath("themes") + "standard_light.json";
		}

		if (!wxFileExists(themePath)) {
			wxLogError(_("Failed to load theme"));
			throw std::runtime_error("Theme file not found");
		} else {
			UserSettingsManager::Get().SetSetting("theme/themeFilePath",
												  themePath);
		}
	}

	try {
		std::ifstream file(themePath.ToStdString());
		nlohmann::json themeData = nlohmann::json::parse(file);

		return themeData;
	} catch (const std::exception &e) {
		wxLogError(wxString::Format(_("Failed to load theme: %s"), e.what()));
		throw std::runtime_error("Theme file not found");
	}
}

bool ThemesManager::IsDarkTheme() const {

	if (currentTheme.contains("isDark")) {
		return currentTheme["isDark"].get<bool>();
	}

	return wxSystemSettings::GetAppearance().IsSystemDark();
}