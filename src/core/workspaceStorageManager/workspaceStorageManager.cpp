#include "workspaceStorageManager/workspaceStorageManager.hpp"
#include <wx/stdpaths.h>
#include <wx/filename.h>
#include <wx/log.h>
#include <fstream>
#include <iomanip>
#include <functional>
#include <ctime>
#include <iostream>

WorkspaceStorageManager& WorkspaceStorageManager::Get() {
    static WorkspaceStorageManager instance;
    return instance;
}

std::string WorkspaceStorageManager::ConvertPathToHash(const std::string& path) {
    size_t hashValue = std::hash<std::string>{}(path);
    std::stringstream ss;
    ss << std::hex << std::setw(16) << std::setfill('0') << hashValue;
    return ss.str();
}

void WorkspaceStorageManager::Initialize(const std::string& workspaceId) {
    std::lock_guard<std::mutex> lock(storageMutex);
    if (workspaceId.empty()) return;

    this->currentWorkspaceId = workspaceId;

    wxString baseDir = wxStandardPaths::Get().GetUserConfigDir() + 
                       wxFileName::GetPathSeparator() + ".kraftaEditor" + 
                       wxFileName::GetPathSeparator() + "workspaces" +
                       wxFileName::GetPathSeparator() + workspaceId;

    wxFileName fn(baseDir, "storage.json");
    if (!fn.DirExists()) {
        fn.Mkdir(wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
    }

    StoragePath = fn.GetFullPath();

    DefaultData = {
        {"last_opened_files", json::array()},
        {"cursor_positions", json::object()},
        {"folded_regions", json::object()}
    };

    try {
        currentData = LoadStorageFromFile();
    } catch (...) {
        currentData = DefaultData;
    }
}

bool WorkspaceStorageManager::Update(const json& data) {
    std::lock_guard<std::mutex> lock(storageMutex);
    if (StoragePath.empty()) return false;

    try {
        wxString tempPath = StoragePath + ".tmp";
        std::ofstream file(tempPath.ToStdString());
        if (file.is_open()) {
            file << std::setw(4) << data << std::endl;
            file.close();
            if (wxRenameFile(tempPath, StoragePath, true)) {
                currentData = data;
                return true;
            }
        }
    } catch (...) {}
    return false;
}

json WorkspaceStorageManager::LoadStorageFromFile() {
    if (!wxFileExists(StoragePath)) {
        CreateDefaultStorageFile();
        return DefaultData;
    }
    try {
        std::ifstream file(StoragePath.ToStdString());
        json data = json::parse(file);
        return MergeWithDefaults(data);
    } catch (...) {
        return DefaultData;
    }
}

void WorkspaceStorageManager::CreateDefaultStorageFile() {
    std::ofstream file(StoragePath.ToStdString());
    if (file.is_open()) {
        file << std::setw(4) << DefaultData << std::endl;
    }
}

json WorkspaceStorageManager::MergeWithDefaults(json& data) {
    // bool changed = false;
    for (const auto& [key, value] : DefaultData.items()) {
        if (!data.contains(key)) {
            data[key] = value;
            // changed = true;
        }
    }
    return data;
}

wxString WorkspaceStorageManager::GetRecentsFilePath() const {
    return wxStandardPaths::Get().GetUserConfigDir() + 
           wxFileName::GetPathSeparator() + ".kraftaEditor" + 
           wxFileName::GetPathSeparator() + "recent_workspaces.json";
}

json WorkspaceStorageManager::LoadRecentsIndex() {
    wxString path = GetRecentsFilePath();
    if (!wxFileExists(path)) return json::array();
    try {
        std::ifstream file(path.ToStdString());
        json j;
        file >> j;
        return j.is_array() ? j : json::array();
    } catch (...) {
        return json::array();
    }
}

void WorkspaceStorageManager::SaveRecentsIndex(const json& data) {
    wxString path = GetRecentsFilePath();
    wxFileName fn(path);
    if (!fn.DirExists()) fn.Mkdir(wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
    try {
        std::ofstream file(path.ToStdString());
        file << std::setw(4) << data << std::endl;
    } catch (...) {}
}

void WorkspaceStorageManager::AddToRecents(const wxString& path) {
    if (path.IsEmpty()) return;
    wxFileName fn(path);
    wxString normalizedPath = fn.GetPath(wxPATH_GET_SEPARATOR);

    json recents = LoadRecentsIndex();
    for (auto it = recents.begin(); it != recents.end();) {
        if ((*it)["path"] == normalizedPath.ToStdString()) {
            it = recents.erase(it);
        } else {
            ++it;
        }
    }

    json entry;
    entry["name"] = fn.GetName().ToStdString();
    if (entry["name"].get<std::string>().empty()) entry["name"] = normalizedPath.ToStdString();
    entry["path"] = normalizedPath.ToStdString();
    
    recents.insert(recents.begin(), entry);
    if (recents.size() > 8) recents.erase(recents.begin() + 8, recents.end());

    SaveRecentsIndex(recents);
}

std::vector<RecentWorkspace> WorkspaceStorageManager::GetRecentWorkspaces() {
    std::vector<RecentWorkspace> result;
    json recents = LoadRecentsIndex();
    for (const auto& item : recents) {
        std::string p = item["path"];
        if (wxDirExists(p)) {
            result.push_back({
                wxString(item["name"].get<std::string>()),
                wxString(p)
            });
        }
    }
    return result;
}

template RequestedWorkspaceSetting<bool> WorkspaceStorageManager::GetSetting<bool>(const std::string&);
template RequestedWorkspaceSetting<int> WorkspaceStorageManager::GetSetting<int>(const std::string&);
template RequestedWorkspaceSetting<std::string> WorkspaceStorageManager::GetSetting<std::string>(const std::string&);
template RequestedWorkspaceSetting<json> WorkspaceStorageManager::GetSetting<json>(const std::string&);