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
#include "platformInfos/platformInfos.hpp"
#include <wx/stdpaths.h>
#include <wx/msgdlg.h>

using json = nlohmann::json;

UserSettingsManager &UserSettingsManager::Get()
{
    static UserSettingsManager instance;
    return instance;
}

UserSettingsManager::UserSettingsManager()
{
    if (SettingsPath.empty())
    {
        SettingsPath =
            wxStandardPaths::Get().GetUserConfigDir() +
            PlatformInfos::OsPathSeparator() +
            ".kraftaEditor" +
            PlatformInfos::OsPathSeparator() +
            "user_settings.json";

        wxFileName fn(SettingsPath);
        if (!fn.DirExists() && !fn.Mkdir(wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL))
        {
            wxLogError("Failed to create settings directory: %s", fn.GetPath());
        }
    }

    DefaultSettings = {
        // viewers settings
        {"show_files_tree", true},
        {"show_menu_bar", true},
        {"show_status_bar", true},
        {"show_tab_bar", true},
        {"show_minimap", true},

        {"windowMaximized", true},
        {"windowSizeX", 1000},
        {"windowSizeY", 700},
        {"dontAskMeAgainFileDelete", false},
        {"dontAskMeAgainDirDelete", false}};
    try
    {
        currentSettings = LoadSettingsFromFile();
    }
    catch (const std::exception &e)
    {
        wxMessageBox("Initial settings load failed: %s", e.what());
        currentSettings = DefaultSettings;
    }
}

bool UserSettingsManager::Update(const json &data)
{
    std::lock_guard<std::mutex> lock(settingsMutex);

    try
    {
        wxString tempPath = SettingsPath + ".tmp";

        {
            std::ofstream config_file(tempPath.ToStdString());
            if (!config_file)
            {
                throw std::runtime_error("Failed to open temporary settings file");
            }
            config_file << std::setw(4) << data << std::endl;
        }

        if (!wxRenameFile(tempPath, SettingsPath, true))
        {
            throw std::runtime_error("Failed to replace settings file");
        }

        currentSettings = data;
        return true;
    }
    catch (const std::exception &e)
    {
        wxMessageBox("Settings update failed: %s", e.what());
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
    catch (const json::exception &e)
    {
        wxMessageBox("JSON parsing error: %s", e.what());
        CreateDefaultSettingsFile();
        return DefaultSettings;
    }
}

void UserSettingsManager::CreateDefaultSettingsFile()
{
    try
    {
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
    catch (const std::exception &e)
    {
        wxMessageBox("Failed to create default settings: %s", e.what());
        throw;
    }
}

json UserSettingsManager::MergeWithDefaults(json &data)
{
    bool needsUpdate = false;

    for (const auto &[key, value] : DefaultSettings.items())
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

template <typename T>
RequestedSetting<T> UserSettingsManager::GetSetting(const std::string &settingName)
{
    std::lock_guard<std::mutex> lock(settingsMutex);
    RequestedSetting<T> result{};
    auto it = currentSettings.find(settingName);
    if (it != currentSettings.end())
    {
        try
        {
            result.value = it->get<T>();
            result.found = true;
        }
        catch (nlohmann::json::type_error &)
        {
            result.found = false;
        }
    }
    else
    {
        result.found = false;
    }
    return result;
}

template RequestedSetting<bool> UserSettingsManager::GetSetting<bool>(const std::string &settingName);
template RequestedSetting<int> UserSettingsManager::GetSetting<int>(const std::string &settingName);
template RequestedSetting<std::string> UserSettingsManager::GetSetting<std::string>(const std::string &settingName);