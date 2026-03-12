#pragma once
#include "appPaths/appPaths.hpp"
#include <nlohmann/json.hpp>
#include <mutex>
using json = nlohmann::json;

/**
 * @brief Generic struct for requested shortcut settings
 *
 * Contains the value of a shortcut setting and a flag indicating
 * whether the setting was found in the stored configuration.
 *
 * @tparam T Type of the setting (int, bool, std::string, etc.)
 */
template <typename T>
struct RequestedShortcutSetting
{
    T value;    ///< Value of the shortcut setting
    bool found; ///< True if the shortcut setting exists and is valid
};

/**
 * @class ShortCutSettingsManager
 * @brief Singleton class for managing application shortcut settings with JSON persistence
 *
 * Provides thread-safe access to keyboard shortcut settings and ensures
 * consistency between in-memory and file-stored configuration.
 */
class ShortCutSettingsManager
{
public:
    json currentSettings; ///< In-memory copy of current shortcut settings

    /**
     * @brief Gets the singleton instance of ShortCutSettingsManager
     * @return Reference to the singleton instance
     * @note Thread-safe in C++11 and later
     */
    static ShortCutSettingsManager &Get();

    /**
     * @brief Updates shortcut settings with new data and persists to file
     * @param data New shortcut settings data in JSON format
     * @return true if update succeeded, false otherwise
     * @note This will overwrite both the in-memory settings and the settings file
     * @warning Will not update if file write fails (maintains consistency)
     */
    bool Update(const json &data);

    /**
     * @brief Loads shortcut settings from file or creates defaults if file doesn't exist
     * @return json Loaded shortcut settings data
     * @throws std::runtime_error if settings cannot be loaded or parsed
     */
    json LoadSettingsFromFile();

    /**
     * @brief Gets a typed shortcut setting by name
     *
     * Checks if the shortcut setting exists and is of the correct type.
     *
     * @tparam T Type of the setting (int, bool, std::string)
     * @param settingName Name of the shortcut setting key
     * @return RequestedSetting<T> Struct containing the value and found flag
     */
    template <typename T>
    RequestedShortcutSetting<T> GetSetting(const std::string &settingName);

    wxString ShortcutsPath; ///< Path to the shortcut settings JSON file

private:
    json DefaultShortCutSettings;  ///< Default values for all shortcut settings
    std::mutex settingsMutex;      ///< Mutex for thread-safe access to settings

    /**
     * @brief Private constructor for singleton pattern
     *
     * Initializes shortcut settings by loading from file or
     * creating defaults if no settings file exists
     */
    ShortCutSettingsManager();

    /**
     * @brief Creates a new shortcut settings file with default key bindings
     * @throws std::runtime_error if file creation fails
     */
    void CreateDefaultSettingsFile();

    /**
     * @brief Merges loaded shortcut settings with default values
     *
     * Ensures any missing shortcut keys are filled in with their defaults,
     * preserving existing user-defined bindings.
     *
     * @param data Shortcut settings JSON to merge into (modified in-place)
     * @return json Merged settings with all required shortcut keys present
     */
    json MergeWithDefaults(json &data);
};