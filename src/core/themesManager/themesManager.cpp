#include "themesManager/themesManager.hpp"
#include "appPaths/appPaths.hpp"
#include "platformInfos/platformInfos.hpp"
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
				  {"minimapCaretPosition", "#c3dde0"},
				  {"editor",
				   {{"font",
					 {{"family",
					   {{"windows", "Consolas"},
						{"linux", "DejaVu Sans Mono"},
						{"macos", "Menlo"}}},
					  {"size", {{"windows", 14}, {"linux", 20}, {"macos", 20}}},
					  {"weight", 400},
					  {"lineHeight", 1.5}}}}}};

json LightTheme = {
	{"themeName", "Standard Krafta Light Theme"},
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
	{"minimapCaretPosition", "#7fc4d0"},
	{"editor",
	 {{"font",
	   {{"family",
		 {{"windows", "Consolas"},
		  {"linux", "DejaVu Sans Mono"},
		  {"macos", "Menlo"}}},
		{"size", {{"windows", 14}, {"linux", 20}, {"macos", 20}}},
		{"weight", 400},
		{"lineHeight", 1.5}}}}}};

ThemesManager &ThemesManager::Get() {
	static ThemesManager instance;
	return instance;
}

wxColor ThemesManager::GetColor(const std::string &key) const {
	try {
		const auto &color = currentTheme.at(key).get<std::string>();
		return wxColor(color);
	} catch (...) {
		wxLogDebug(_("Color '%s' not found - using default"), key);
		return wxColor(0, 0, 0);
	}
}

wxFont ThemesManager::GetFont() const {
	int kDefaultSize = 14;

#if defined(__WXMSW__)
	const std::string defaultFamily = "Consolas";
#elif defined(__WXMAC__)
	const std::string defaultFamily = "Menlo";
	kDefaultSize = 20;
#elif defined(__WXGTK__)
	const std::string defaultFamily = "DejaVu Sans Mono";
	kDefaultSize = 20;
#else
	const std::string defaultFamily = "monospace";
#endif

	constexpr int kDefaultWeight = 400;
	constexpr auto kFamilyType = wxFONTFAMILY_TELETYPE;

	int size = kDefaultSize;
	int weight = kDefaultWeight;
	std::string family = defaultFamily;

	try {
		if (currentTheme.contains("editor") &&
			currentTheme["editor"].contains("font")) {

			const auto &fontNode = currentTheme["editor"]["font"];
			if (fontNode.is_object()) {
				size = fontNode["size"].value(PlatformInfos::OsNameView(),
											  kDefaultSize);
				weight = fontNode.value("weight", kDefaultWeight);

				if (fontNode.contains("family") &&
					fontNode["family"].is_object()) {
					const auto &famNode = fontNode["family"];
#if defined(__WXMSW__)
					family = famNode.value("windows", defaultFamily);
#elif defined(__WXMAC__)
					family = famNode.value("macos", defaultFamily);
#elif defined(__WXGTK__)
					family = famNode.value("linux", defaultFamily);
#endif
				}
			}
		}
	} catch (...) {
	}

	if (size < 6 || size > 72)
		size = kDefaultSize;
	if (weight < 100 || weight > 900)
		weight = kDefaultWeight;

	const auto weightEnum = static_cast<wxFontWeight>(weight);

	auto makeFont = [&](const std::string &fam) -> wxFont {
		return wxFont(wxSize(size, size), kFamilyType, wxFONTSTYLE_NORMAL,
					  weightEnum, false, wxString::FromUTF8(fam));
	};

	wxFont f = makeFont(family);
	if (f.IsOk())
		return f;

	if (family != defaultFamily) {
		f = makeFont(defaultFamily);
		if (f.IsOk())
			return f;
	}

	f = wxFont(size, kFamilyType, wxFONTSTYLE_NORMAL, weightEnum, false);
	if (f.IsOk())
		return f;

	return wxFont(wxFontInfo(size).Family(kFamilyType));
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
		wxMessageBox("887");
		if (userSetting.found) {
			wxLogDebug(
				_("Saved theme file not found, falling back to default."));
		}

		themePath = defaultThemeFile;

		if (!wxFileExists(themePath)) {
			std::ofstream fileStream(themePath.ToStdString());
			if (!fileStream) {
				wxLogDebug(wxString::Format(
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