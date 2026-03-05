#pragma once
#include <nlohmann/json.hpp>
#include <mutex>
#include <string>
#include <vector>
#include <wx/string.h>

using json = nlohmann::json;

/**
 * @brief Container for settings retrieved from workspace storage.
 */
template <typename T>
struct RequestedWorkspaceSetting {
    T value;
    bool found;
};

/**
 * @brief Structure representing a recently accessed workspace.
 */
struct RecentWorkspace {
    wxString name;
    wxString path;
};

/**
 * @class WorkspaceStorageManager
 * @brief Singleton for managing project-specific data and global workspace history using JSON.
 */
class WorkspaceStorageManager {
public:
    static WorkspaceStorageManager& Get();

    /**
     * @brief Generates a unique hex hash for a given file system path.
     */
    static std::string ConvertPathToHash(const std::string& path);

    /**
     * @brief Loads the storage file associated with the given workspace ID.
     */
    void Initialize(const std::string& workspaceId);

    /**
     * @brief Updates the workspace-specific data and saves it to disk.
     */
    bool Update(const json& data);

    /**
     * @brief Adds a workspace path to the global recent history list.
     */
    void AddToRecents(const wxString& path);

    /**
     * @brief Retrieves the list of the 8 most recently opened workspaces.
     */
    std::vector<RecentWorkspace> GetRecentWorkspaces();

    /**
     * @brief Safely retrieves a typed value from the current workspace data.
     */
    template <typename T>
    RequestedWorkspaceSetting<T> GetSetting(const std::string& settingName) {
        std::lock_guard<std::mutex> lock(storageMutex);
        if (currentData.contains(settingName) && !currentData[settingName].is_null()) {
            try {
                return {currentData[settingName].get<T>(), true};
            } catch (...) {}
        }
        return {T(), false};
    }

    json currentData;

private:
    WorkspaceStorageManager() = default;
    WorkspaceStorageManager(const WorkspaceStorageManager&) = delete;
    void operator=(const WorkspaceStorageManager&) = delete;

    json LoadStorageFromFile();
    void CreateDefaultStorageFile();
    json MergeWithDefaults(json& data);

    json LoadRecentsIndex();
    void SaveRecentsIndex(const json& data);
    wxString GetRecentsFilePath() const;

    wxString StoragePath;
    json DefaultData;
    std::string currentWorkspaceId;
    mutable std::mutex storageMutex;
};