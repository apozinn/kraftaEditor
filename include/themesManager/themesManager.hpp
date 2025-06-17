#pragma once

#include "appPaths/appPaths.hpp"  
#include <wx/colour.h>           
#include <wx/font.h>             
#include <nlohmann/json.hpp>     
#include <wx/log.h>              
#include <fstream>               
#include <wx/settings.h>         

/**
 * @class ThemeManager
 * @brief Singleton class for managing application themes
 * 
 * This class handles loading and providing access to theme configurations
 * stored in JSON format. It supports color and font management with
 * thread-safe access to theme resources.
 */
class ThemesManager
{
public:
    nlohmann::json currentTheme; ///< Current theme data in JSON format

    /**
     * @brief Gets the singleton instance of ThemeManager
     * @return Reference to the singleton instance
     * 
     * @note Thread-safe in C++11 and later due to static local initialization
     */
    static ThemesManager& Get();

    /**
     * @brief Gets a color from the current theme
     * @param key JSON key for the color value (e.g., "background", "text.primary")
     * @return wxColor corresponding to the requested key
     * 
     * @throws std::out_of_range if key doesn't exist
     * @throws std::runtime_error if color format is invalid
     * 
     * @example
     * wxColor bgColor = ThemeManager::Get().GetColor("background");
     */
    wxColor GetColor(const std::string& key) const;

    /**
     * @brief Gets a font from the current theme
     * @param key JSON key for the font configuration (e.g., "main.font", "title.font")
     * @return wxFont configured according to theme specifications
     * 
     * @throws std::out_of_range if key doesn't exist
     * @throws std::runtime_error if font configuration is invalid
     * 
     * @note Expected JSON format:
     * {
     *   "family": "Arial",
     *   "size": 12,
     *   "style": "normal|italic",
     *   "weight": "normal|bold"
     * }
     */
    wxFont GetFont(const std::string& key) const;

    /**
     * @brief Reloads the theme from disk
     * @return true if theme was successfully reloaded, false otherwise
     * 
     * @note Logs errors using wxLog if reload fails
     * @warning Existing theme references may become invalid after reload
     */
    bool Reload();

private:
    /**
     * @brief Private constructor for singleton pattern
     * @param themeData Initial theme data to load
     */
    ThemesManager() : currentTheme(LoadThemeFile()) {}

    /**
     * @brief Loads theme data from JSON file
     * @return nlohmann::json containing theme data
     * 
     * @throws std::runtime_error if theme file cannot be loaded or parsed
     * @note Expected file path: {AppPaths::ThemesDir()}/active_theme.json
     */
    nlohmann::json LoadThemeFile() const;
};