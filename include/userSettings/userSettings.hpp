#pragma once
#include "appPaths/appPaths.hpp"
#include <nlohmann/json.hpp>
#include <mutex>

using json = nlohmann::json;

/**
 * @brief Generic struct for requested settings
 *
 * Contains the value of the setting and a flag indicating
 * whether the setting was found in the stored configuration.
 *
 * @tparam T Type of the setting (int, bool, std::string, etc.)
 */
template<typename T>
struct RequestedSetting {
    T value;     ///< Value of the setting
    bool found;  ///< True if the setting exists and is valid
};

/**
 * @class UserSettingsManager
 * @brief Singleton class for managing application user settings with JSON persistence
 *
 * Provides thread-safe access to user settings and ensures
 * consistency between in-memory and file-stored configuration.
 */
class UserSettingsManager
{
public:
    json currentSettings; ///< In-memory copy of current user settings

    /**
     * @brief Gets the singleton instance of UserSettingsManager
     * @return Reference to the singleton instance
     * @note Thread-safe in C++11 and later
     */
    static UserSettingsManager& Get();

    /**
     * @brief Updates settings with new data and persists to file
     * @param data New settings data in JSON format
     * @return true if update succeeded, false otherwise
     * @note This will overwrite both the in-memory settings and the settings file
     * @warning Will not update if file write fails (maintains consistency)
     */
    bool Update(const json& data);

    /**
     * @brief Gets a typed setting from user settings
     *
     * Checks if the setting exists and is of the correct type.
     *
     * @tparam T Type of the setting (int, bool, std::string)
     * @param settingName Name of the setting
     * @return RequestedSetting<T> Struct containing value and found flag
     */
    template<typename T>
    RequestedSetting<T> GetSetting(const std::string& settingName);

private:
    json DefaultSettings; 
    wxString SettingsPath;    ///< Path to settings JSON file
    std::mutex settingsMutex; ///< Mutex for thread safety

    /**
     * @brief Private constructor for singleton pattern
     *
     * Initializes settings by loading from file or creating default
     */
    UserSettingsManager();

    /**
     * @brief Loads settings from file or creates default settings if file doesn't exist
     * @return json Loaded settings data
     * @throws std::runtime_error if settings cannot be loaded
     */
    json LoadSettingsFromFile();

    /**
     * @brief Creates a new settings file with default values
     * @throws std::runtime_error if file creation fails
     */
    void CreateDefaultSettingsFile();

    /**
     * @brief Merges loaded settings with default values
     * @param data Settings JSON to merge into (modified in-place)
     */
    json MergeWithDefaults(json &data);
};