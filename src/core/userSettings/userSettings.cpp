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
    wxString defaultUserSettingsPath = ApplicationPaths::GetConfigPath("userSettings") + "defaultUserSettings.json";

    if (wxFileExists(defaultUserSettingsPath))
    {
        std::ifstream file(defaultUserSettingsPath.ToStdString());
        if (file.is_open())
        {
            try
            {
                DefaultSettings = json::parse(file);
            }
            catch (const json::parse_error &e)
            {
                wxLogError("Erro de sintaxe no JSON padrão: %s", e.what());
            }
        }
    }
    else
    {
        DefaultSettings = EmergencyDefaultSettings;
    }

    try
    {
        currentSettings = LoadSettingsFromFile();
    }
    catch (const std::exception &e)
    {
        wxMessageBox("Initial settings load failed");
        currentSettings = DefaultSettings;
    }
}

template <typename T>
bool UserSettingsManager::SetSetting(const std::string &token, const T &value)
{
    std::lock_guard<std::mutex> lock(settingsMutex);

    try
    {
        // Atualiza apenas a chave específica no JSON em memória
        currentSettings[token] = value;

        // Salva o estado atualizado no disco
        return SaveInternal(currentSettings);
    }
    catch (const std::exception &e)
    {
        wxLogError("Failed to set setting: %s", e.what());
        return false;
    }
}

// Instanciações para o método SetSetting usando string
template bool UserSettingsManager::SetSetting<bool>(const std::string&, const bool&);
template bool UserSettingsManager::SetSetting<int>(const std::string&, const int&);
template bool UserSettingsManager::SetSetting<std::string>(const std::string&, const std::string&);
template bool UserSettingsManager::SetSetting<double>(const std::string&, const double&);

bool UserSettingsManager::SaveInternal(const nlohmann::json &data)
{
    wxString tempPath = SettingsPath + ".tmp";

    try
    {
        {
            std::ofstream file(tempPath.ToStdString(), std::ios::out | std::ios::trunc);
            if (!file.is_open())
                return false;

            file << std::setw(4) << data << std::endl;
            file.flush(); // Garante que os dados saíram do buffer do C++
        } // O arquivo fecha aqui (RAII)

        // wxRenameFile com 'true' para sobrescrever o original de forma atômica
        if (!wxRenameFile(tempPath, SettingsPath, true))
        {
            wxRemoveFile(tempPath);
            return false;
        }

        return true;
    }
    catch (...)
    {
        return false;
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
        // wxMessageBox("JSON parsing error: %s", e.what());
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
RequestedSetting<T> UserSettingsManager::GetSetting(const std::string &path)
{
    std::lock_guard<std::mutex> lock(settingsMutex);
    RequestedSetting<T> result{};

    try
    {
        auto ptr = nlohmann::json::json_pointer(path.front() == '/' ? path : "/" + path);

        const nlohmann::json *target = nullptr;

        if (currentSettings.contains(ptr))
        {
            target = &currentSettings[ptr];
        }
        else if (DefaultSettings.contains(ptr))
        {
            target = &DefaultSettings[ptr];
        }

        if (target && !target->is_null())
        {
            result.value = target->get<T>();
            result.found = true;
        }
    }
    catch (...)
    {
        result.found = false;
    }

    return result;
}

template RequestedSetting<bool> UserSettingsManager::GetSetting<bool>(const std::string&);
template RequestedSetting<int> UserSettingsManager::GetSetting<int>(const std::string&);
template RequestedSetting<std::string> UserSettingsManager::GetSetting<std::string>(const std::string&);
template RequestedSetting<double> UserSettingsManager::GetSetting<double>(const std::string&);