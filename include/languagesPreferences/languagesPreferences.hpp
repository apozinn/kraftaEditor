/**
 * @file languagesPreferences.hpp
 * @brief Language preferences management system using JSON configuration
 *
 * Provides centralized control over syntax highlighting, editor behavior,
 * and visual styles for multiple programming languages in a Scintilla-based editor.
 */

#pragma once

#include <nlohmann/json.hpp>
#include <unordered_map>
#include <string>
#include <wx/stc/stc.h>

using json = nlohmann::json; ///< Alias for nlohmann::json namespace

/**
 * @struct languagePreferencesStruct
 * @brief Container for all language-specific configuration
 *
 * Aggregates lexer settings, syntax rules, and visual styles
 * for a single programming language.
 */
struct languagePreferencesStruct
{
    std::string name; ///< Canonical language name (e.g. "Python")
    int lexer;        ///< Scintilla lexer ID (e.g. wxSTC_LEX_PYTHON)
    json preferences; ///< Behavioral preferences (tabs, folding, etc.)
    json styles;      ///< Visual styling rules (colors, fonts, etc.)
};

/**
 * @class LanguagesPreferences
 * @brief Singleton manager for language-specific editor configurations
 *
 * Responsible for:
 * - Loading and caching language definitions from JSON files
 * - Mapping file extensions to language configurations
 * - Applying syntax rules and visual styles to editor components
 *
 * Implements the Singleton pattern to ensure consistent behavior
 * across all editor instances.
 */
class LanguagesPreferences
{
public:
    /**
     * @brief Get the singleton instance
     * @return Reference to the singleton manager
     *
     * @note Thread-safe in C++11 and later (magic static)
     */
    static LanguagesPreferences &Get();

    /**
     * @brief Configures editor components for a specific language
     * @param codeContainer Parent window containing editor/minimap
     * @return Complete language configuration that was applied
     *
     * Performs full setup including:
     * 1. Lexer selection
     * 2. Tab/indentation rules
     * 3. Syntax highlighting
     * 4. Folding markers
     * 5. Status bar updates
     */
    languagePreferencesStruct SetupLanguagesPreferences(wxWindow *codeContainer);

    /**
     * @brief Retrieves language configuration for a file
     * @param path File path used to determine language (via extension)
     * @return Language configuration structure
     *
     * @throws std::runtime_error if configuration files are invalid
     *
     * Fallback order:
     * 1. Extension-matched language
     * 2. Default language
     * 3. Hardcoded failsafe values
     */
    languagePreferencesStruct GetLanguagePreferences(const wxString &path);

    /**
     * @brief Gets the icon path for a language
     * @param path File path used to determine language
     * @return Absolute path to language icon file
     *
     * Returns "unknown" icon if no specific icon configured
     */
    wxString GetLanguageIconPath(const wxString &path);

private:
    // Error message constants
    static constexpr const char *ERROR_LANGUAGES_PREFERENCES_FILE_NOT_FOUND =
        "Language preferences file not found or inaccessible. Path: %s";
    static constexpr const char *ERROR_JSON_PARSE_FAILED =
        "Failed to parse language preferences JSON. Details: %s";
    static constexpr const char *ERROR_LANGUAGE_NOT_FOUND =
        "Language '%s' not found in preferences. Using default.";
    static constexpr const char *ERROR_DEFAULT_NOT_CONFIGURED =
        "Critical: Default language preferences not configured!";
    static constexpr const char *EXTENSIONS_LIS_NOT_FOUND =
        "Critical: Extensions list not found!";

    /**
     * @brief Private constructor (Singleton pattern)
     *
     * Initializes:
     * - Extension-to-language mapping
     * - Default language configurations
     */
    LanguagesPreferences();

    /**
     * @brief Configures code folding for editor components
     * @param currentLanguagePreferences Active language config
     * @param editor Main editor control
     * @param minimap Minimap editor control
     *
     * Applies settings from:
     * 1. Language-specific fold preferences
     * 2. Global fold configuration (foldSettings.json)
     */
    void SetupFold(const languagePreferencesStruct &currentLanguagePreferences,
                   wxStyledTextCtrl *editor,
                   wxStyledTextCtrl *minimap);

    /**
     * @brief Configures syntax highlighting keywords
     * @param currentLanguagePreferences Active language config
     * @param editor Main editor control
     * @param minimap Minimap editor control
     *
     * Processes all keyword lists defined in the language's
     * "syntax.keyword_lists" JSON section
     */
    void SetupReservedWords(const languagePreferencesStruct &currentLanguagePreferences,
                            wxStyledTextCtrl *editor,
                            wxStyledTextCtrl *minimap);

    /**
     * @brief Configures autocomplete word suggestions
     * @param currentLanguagePreferences Active language config
     * @param editor Main editor control
     *
     * Builds a candidate list from "syntax.keyword_lists"
     * and attaches a listener to display suggestions
     * as the user types.
     */
    void SetupAutoCompleteWords(const languagePreferencesStruct &currentLanguagePreferences,
                                wxStyledTextCtrl *editor);

    /**
     * @brief Applies visual styles to editor components
     * @param currentLanguagePreferences Active language config
     * @param editor Main editor control
     * @param minimap Minimap editor control
     *
     * Handles:
     * - Color schemes (foreground/background)
     * - Font styles (bold/italic/underline)
     * - Font family and sizing
     */
    void ApplyLexerStyles(const languagePreferencesStruct &currentLanguagePreferences,
                          wxStyledTextCtrl *editor,
                          wxStyledTextCtrl *minimap);

    /**
     * @brief Updates status bar with language info
     * @param currentLanguagePreferences Active language config
     */
    void UpdateStatusBar(const languagePreferencesStruct &currentLanguagePreferences);

    /**
     * @brief Loads extension-to-language mappings
     *
     * @throws std::runtime_error if extensionsList.json is missing
     *
     * Populates m_extToLang with pairs like:
     * { ".cpp" => "C++", ".py" => "Python" }
     */
    void LoadExtensionsList();

    std::unordered_map<std::string, languagePreferencesStruct> m_languages; ///< Cached language configurations
    std::unordered_map<std::string, std::string> m_extToLang;               ///< File extension to language name mapping
    json m_preferencesObject;                                               ///< Root JSON preferences object (if needed)
    json m_lexerStylesObject;                                               ///< Root JSON styles object (if needed)
};