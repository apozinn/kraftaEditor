#include "themesManager/themesManager.hpp"
#include "appPaths/appPaths.hpp"
#include <wx/colour.h>
#include <wx/font.h>
#include <nlohmann/json.hpp>
#include <wx/log.h>
#include <fstream>
#include <wx/settings.h>
#include <wx/file.h>

ThemesManager &ThemesManager::Get()
{
    static ThemesManager instance;
    return instance;
}

wxColor ThemesManager::GetColor(const std::string &key) const
{
    try
    {
        const auto &color = currentTheme.at(key).get<std::string>();
        return wxColor(color);
    }
    catch (...)
    {
        wxLogWarning("Color '%s' not found - using default", key);
        return wxColor(0, 0, 0);
    }
}

wxFont ThemesManager::GetFont(const std::string &key) const
{
    try
    {
        const auto &font = currentTheme.at("fonts").at(key);
        return wxFont(
            font.value("size", 12),
            wxFONTFAMILY_DEFAULT,
            font.value("italic", false) ? wxFONTSTYLE_ITALIC : wxFONTSTYLE_NORMAL,
            font.value("bold", false) ? wxFONTWEIGHT_BOLD : wxFONTWEIGHT_NORMAL,
            false,
            font.value("family", "Arial"));
    }
    catch (...)
    {
        wxLogWarning("Font '%s' not found - using default", key);
        return wxFont(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    }
}

bool ThemesManager::Reload()
{
    try
    {
        currentTheme = LoadThemeFile();
        return true;
    }
    catch (...)
    {
        return false;
    }
}

nlohmann::json ThemesManager::LoadThemeFile() const
{
    const wxString themePath = ApplicationPaths::AssetsPath("themes") + "standard_theme.json";

    if (!wxFileExists(themePath))
    {
        wxLogError("Failed to load theme");
        throw std::runtime_error("Theme file not found");
    }

    try
    {
        std::ifstream file(themePath.ToStdString());
        nlohmann::json themeData = nlohmann::json::parse(file);

        return wxSystemSettings::GetAppearance().IsSystemDark()
                   ? themeData["dark"]
                   : themeData["light"];
    }
    catch (const std::exception &e)
    {
        wxLogError("Failed to load theme: %s", e.what());
        throw std::runtime_error("Theme file not found");
    }
}

bool ThemesManager::IsDarkTheme() const
{
    return wxSystemSettings::GetAppearance().IsSystemDark();
}