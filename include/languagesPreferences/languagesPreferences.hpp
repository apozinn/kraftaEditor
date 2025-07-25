/**
 * @file languagesPreferences.hpp
 * @brief Header file for managing language preferences using JSON
 */

#pragma once

#include <nlohmann/json.hpp>
#include <unordered_map>
#include <string>

using json = nlohmann::json; ///< Alias for nlohmann::json

/**
 * @class LanguagesPreferences
 * @brief Singleton class for managing language preferences
 *
 * This class loads language preferences from JSON files and provides
 * access to them through a unified interface. It implements the Singleton
 * pattern to ensure a single instance manages all language preferences.
 */
class LanguagesPreferences
{
public:
    /**
     * @brief Gets the singleton instance
     * @return Reference to the singleton instance
     *
     * Implements the Singleton pattern to ensure only one instance exists.
     */
    static LanguagesPreferences &Get();

    /**
     * @brief Gets preferences for a specific language
     * @param[in] languageName Name of the language to retrieve
     * @return JSON object containing the language preferences
     *
     * If the requested language is not found, returns the default preferences.
     * The returned JSON object contains all configuration for the language.
     */
    json GetLanguagePreferences(const std::string &languageName);

private:
    static constexpr const char *ERROR_LANGUAGES_PREFERENCES_FILE_NOT_FOUND =
        "Language preferences file not found or inaccessible. Path: %s";
    static constexpr const char *ERROR_JSON_PARSE_FAILED =
        "Failed to parse language preferences JSON. Details: %s";
    static constexpr const char *ERROR_LANGUAGE_NOT_FOUND =
        "Language '%s' not found in preferences. Using default.";
    static constexpr const char *ERROR_DEFAULT_NOT_CONFIGURED =
        "Critical: Default language preferences not configured!";
    /**
     * @brief Private constructor (Singleton pattern)
     *
     * Automatically calls LoadLanguages() to initialize preferences
     */
    LanguagesPreferences();

    /**
     * @brief Loads language preferences from JSON files
     * @return JSON object containing all loaded preferences
     *
     * Loads preferences from a JSON file and stores them in m_languages.
     * Initializes with default preferences if loading fails.
     */
    json LoadLanguages();

    std::unordered_map<std::string, json> m_languages; ///< Storage for all language preferences
};