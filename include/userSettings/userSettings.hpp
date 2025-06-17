#pragma once
#include "appPaths/appPaths.hpp"
#include <nlohmann/json.hpp>
#include <mutex>

using json = nlohmann::json;

/**
 * @class UserSettingsManager
 * @brief Singleton class for managing application user settings with JSON persistence
 *
 * This class provides thread-safe access to user settings with automatic file I/O operations.
 * It maintains settings both in memory and in a JSON file, ensuring consistency between them.
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
    static UserSettingsManager &Get();

    /**
     * @brief Updates settings with new data and persists to file
     * @param data New settings data in JSON format
     * @return true if update succeeded, false otherwise
     *
     * @note This will overwrite both the in-memory settings and the settings file
     * @warning Will not update if file write fails (maintains consistency)
     */
    bool Update(const json &data);

private:
    json DefaultSettings; 
    wxString SettingsPath;    ///< Path to settings JSON file
    std::mutex settingsMutex; ///< Mutex for thread safety

    /**
     * @brief Private constructor for singleton pattern
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