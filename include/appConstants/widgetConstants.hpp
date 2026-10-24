#pragma once
#include "themesManager/themeApplier.hpp"
#include <nlohmann/json.hpp>
#include <wx/colour.h>
#include <wx/font.h>
#include <wx/string.h>

using json = nlohmann::json;

namespace widgetConstants {

inline constexpr int kPanelWidth = 480;
inline constexpr int kPanelHeight = 280;
inline constexpr int kRowPadding = 8;
inline constexpr int kRowSpacing = 2;
inline constexpr int kSearchPadding = 8;
inline constexpr int kRowHeight = 24;
inline constexpr int kRowPaddingX = 8;
inline constexpr int kNameFontSize = 10;
inline constexpr int kPathFontSize = 8;
inline constexpr int kSearchFontSize = 11;
inline constexpr int kMaxFiles = 5000;

inline const std::unordered_set<std::string> kIgnoredDirs = {
	".git",
	".svn",
	".hg",
	"build",
	"dist",
	"out",
	"node_modules",
	".vscode",
	".idea",
	"__pycache__",
	"cmake-build-debug",
	"cmake-build-release",
};

inline wxColour GetThemeColor(const nlohmann::json &theme, const char *key,
							  const wxColour &fallback) {
	if (!theme.contains(key) || !theme[key].is_string())
		return fallback;
	try {
		return wxColour(wxString::FromUTF8(theme[key].get<std::string>()));
	} catch (...) {
		return fallback;
	}
}

inline wxFont MakeMonospaceFont(int size,
								wxFontWeight weight = wxFONTWEIGHT_NORMAL) {
	wxFont font(size, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, weight);
	font.SetFaceName("Monospace");
	return font;
}

} // namespace widgetConstants