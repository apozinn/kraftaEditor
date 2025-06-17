#include "userSettings/userSettings.hpp"
#include "appPaths/appPaths.hpp"
#include <wx/colour.h>
#include <wx/font.h>
#include <nlohmann/json.hpp>
#include <wx/log.h>
#include <fstream>
#include <wx/settings.h>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <wx/file.h>
#include <wx/filename.h>

using json = nlohmann::json;

UserSettingsManager& UserSettingsManager::Get()
{
    static UserSettingsManager instance;
    return instance;
}

UserSettingsManager::UserSettingsManager()
{
    if (SettingsPath.empty())
    {
        SettingsPath = ApplicationPaths::ApplicationPath() + "user_settings.json";

        // Ensure the directory exists
        wxFileName fn(SettingsPath);
        if (!fn.DirExists() && !fn.Mkdir(wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL))
        {
            wxLogError("Failed to create settings directory: %s", fn.GetPath());
        }
    }

    DefaultSettings = {
        {"show_minimap", true},
        {"show_menu", true},
        {"show_statusBar", true},
        {"windowMaximized", true},
        {"windowSizeX", 1000},
        {"windowSizeY", 700}};

    // Load settings during initialization
    try
    {
        currentSettings = LoadSettingsFromFile();
    }
    catch (const std::exception& e)
    {
        wxLogError("Initial settings load failed: %s", e.what());
        currentSettings = DefaultSettings;
    }
}

bool UserSettingsManager::Update(const json& data)
{
    std::lock_guard<std::mutex> lock(settingsMutex);

    try
    {
        // 1. Create temporary file
        wxString tempPath = SettingsPath + ".tmp";

        // 2. Write to temporary file
        {
            std::ofstream config_file(tempPath.ToStdString());
            if (!config_file)
            {
                throw std::runtime_error("Failed to open temporary settings file");
            }
            config_file << std::setw(4) << data << std::endl;
        }

        // 3. Replace original file
        if (!wxRenameFile(tempPath, SettingsPath, true))
        {
            throw std::runtime_error("Failed to replace settings file");
        }

        // 4. Update in-memory copy
        currentSettings = data;
        return true;
    }
    catch (const std::exception& e)
    {
        wxLogError("Settings update failed: %s", e.what());
        return false;
    }
}

json UserSettingsManager::LoadSettingsFromFile()
{
    if (!wxFileExists(SettingsPath))
    {
        CreateDefaultSettingsFile();
        return DefaultSettings;
    }

    try
    {
        std::ifstream config_file(SettingsPath.ToStdString());
        if (!config_file)
        {
            throw std::runtime_error("Failed to open settings file for reading");
        }

        json data = json::parse(config_file);
        return MergeWithDefaults(data);
    }
    catch (const json::exception& e)
    {
        wxLogError("JSON parsing error: %s", e.what());
        // Try to create new file if original is corrupted
        CreateDefaultSettingsFile();
        return DefaultSettings;
    }
}

void UserSettingsManager::CreateDefaultSettingsFile()
{
    try
    {
        // Atomic write using temporary file
        wxString tempPath = SettingsPath + ".tmp";

        {
            std::ofstream fileStream(tempPath.ToStdString());
            if (!fileStream)
            {
                throw std::runtime_error("Failed to create temporary settings file");
            }
            fileStream << std::setw(4) << DefaultSettings << std::endl;
        }

        if (!wxRenameFile(tempPath, SettingsPath))
        {
            throw std::runtime_error("Failed to replace settings file with defaults");
        }
    }
    catch (const std::exception& e)
    {
        wxLogError("Failed to create default settings: %s", e.what());
        throw;
    }
}

json UserSettingsManager::MergeWithDefaults(json& data)
{
    bool needsUpdate = false;

    for (const auto& [key, value] : DefaultSettings.items())
    {
        if (!data.contains(key))
        {
            data[key] = value;
            needsUpdate = true;
        }
    }

    if (needsUpdate)
    {
        Update(data); 
    }

    return data;
}