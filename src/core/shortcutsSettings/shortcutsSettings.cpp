#include "shortcutsSettings/shortcutsSettings.hpp"
#include "appPaths/appPaths.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <wx/file.h>
#include <wx/filename.h>
#include "platformInfos/platformInfos.hpp"
#include <wx/stdpaths.h>
#include <wx/msgdlg.h>

using json = nlohmann::json;

ShortCutSettingsManager &ShortCutSettingsManager::Get()
{
    static ShortCutSettingsManager instance;
    return instance;
}

ShortCutSettingsManager::ShortCutSettingsManager()
{
    if (ShortcutsPath.empty())
    {
        ShortcutsPath =
            wxStandardPaths::Get().GetUserConfigDir() +
            PlatformInfos::OsPathSeparator() +
            ".kraftaEditor" +
            PlatformInfos::OsPathSeparator() +
            "shortcut_settings.json";

        wxFileName fn(ShortcutsPath);
        if (!fn.DirExists() && !fn.Mkdir(wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL))
        {
            wxMessageBox(wxString::Format(_("Failed to create shortcut settings directory: %s"), fn.GetPath()), _("Error"), wxICON_ERROR);
        }
    }

    DefaultShortCutSettings = {
        {"shortcut_new_file", "Ctrl+N"},
        {"shortcut_open_file", "Ctrl+O"},
        {"shortcut_open_folder", "Ctrl+Shift+K"},
        {"shortcut_save_file", "Ctrl+S"},
        {"shortcut_save_file_as", "Ctrl+Shift+S"},
        {"shortcut_save_all", "Ctrl+Alt+S"},
        {"shortcut_close_file", "Ctrl+W"},
        {"shortcut_close_all", "Ctrl+Alt+W"},
        {"shortcut_quit", "Ctrl+Q"},
        {"shortcut_undo", "Ctrl+Z"},
        {"shortcut_redo", "Ctrl+Y"},
        {"shortcut_cut", "Ctrl+X"},
        {"shortcut_copy", "Ctrl+C"},
        {"shortcut_paste", "Ctrl+V"},
        {"shortcut_select_all", "Ctrl+A"},
        {"shortcut_select_line", "Ctrl+L"},
        {"shortcut_duplicate_line", "Ctrl+Shift+D"},
        {"shortcut_delete_line", "Ctrl+Shift+K"},
        {"shortcut_move_line_up", "Alt+Up"},
        {"shortcut_move_line_down", "Alt+Down"},
        {"shortcut_toggle_comment", "Ctrl+;"},
        {"shortcut_toggle_block_comment", "Ctrl+Alt+;"},
        {"shortcut_indent", "Tab"},
        {"shortcut_outdent", "Shift+Tab"},
        {"shortcut_find", "Ctrl+F"},
        {"shortcut_find_replace", "Ctrl+H"},
        {"shortcut_find_next", "F3"},
        {"shortcut_find_previous", "Shift+F3"},
        {"shortcut_go_to_line", "Ctrl+G"},
        {"shortcut_zoom_in", "Ctrl++"},
        {"shortcut_zoom_out", "Ctrl+-"},
        {"shortcut_zoom_reset", "Ctrl+0"},
        {"shortcut_toggle_file_tree", "Ctrl+Shift+E"},
        {"shortcut_toggle_status_bar", "Ctrl+Shift+B"},
        {"shortcut_toggle_tab_bar", "Ctrl+Shift+T"},
        {"shortcut_toggle_menu_bar", "Ctrl+Shift+M"},
        {"shortcut_toggle_full_screen", "F11"},
        {"shortcut_split_editor", "Ctrl+\\"},
        {"shortcut_go_to_definition", "F12"},
        {"shortcut_go_to_symbol", "Ctrl+Shift+O"},
        {"shortcut_go_to_file", "Ctrl+P"},
        {"shortcut_go_back", "Alt+Left"},
        {"shortcut_go_forward", "Alt+Right"},
        {"shortcut_next_tab", "Ctrl+Tab"},
        {"shortcut_previous_tab", "Ctrl+Shift+Tab"},
        {"shortcut_command_palette", "Ctrl+Shift+P"},
        {"shortcut_quick_open", "Ctrl+P"},
        {"shortcut_open_terminal", "Ctrl+J"},
        {"shortcut_open_settings", "Ctrl+,"},
        {"shortcut_open_shortcuts", "Ctrl+alt+,"}};

    try
    {
        currentSettings = LoadSettingsFromFile();
    }
    catch (const std::exception &e)
    {
        wxMessageBox(_("Initial shortcut settings load failed"), _("Error"), wxICON_ERROR);
        currentSettings = DefaultShortCutSettings;
    }
}

bool ShortCutSettingsManager::Update(const json &data)
{
    std::lock_guard<std::mutex> lock(settingsMutex);

    try
    {
        wxString tempPath = ShortcutsPath + ".tmp";

        {
            std::ofstream config_file(tempPath.ToStdString());
            if (!config_file)
            {
                throw std::runtime_error("Failed to open temporary shortcut settings file");
            }
            config_file << std::setw(4) << data << std::endl;
        }

        if (!wxRenameFile(tempPath, ShortcutsPath, true))
        {
            throw std::runtime_error("Failed to replace shortcut settings file");
        }

        currentSettings = data;
        return true;
    }
    catch (const std::exception &e)
    {
        wxMessageBox(_("Shortcut settings update failed"), _("Error"), wxICON_ERROR);
        return false;
    }
}

json ShortCutSettingsManager::LoadSettingsFromFile()
{
    if (!wxFileExists(ShortcutsPath))
    {
        CreateDefaultSettingsFile();
        return DefaultShortCutSettings;
    }

    try
    {
        std::ifstream config_file(ShortcutsPath.ToStdString());
        if (!config_file)
        {
            throw std::runtime_error("Failed to open shortcut settings file for reading");
        }

        json data = json::parse(config_file);
        auto mergedSettings = MergeWithDefaults(data);
        currentSettings = mergedSettings;
        return mergedSettings;
    }
    catch (const json::exception &e)
    {
        wxMessageBox(wxString::Format(_("JSON parsing error: %s"), e.what()), _("Error"), wxICON_ERROR);
        CreateDefaultSettingsFile();
        return DefaultShortCutSettings;
    }
}

void ShortCutSettingsManager::CreateDefaultSettingsFile()
{
    try
    {
        wxString tempPath = ShortcutsPath + ".tmp";

        {
            std::ofstream fileStream(tempPath.ToStdString());
            if (!fileStream)
            {
                throw std::runtime_error("Failed to create temporary shortcut settings file");
            }
            fileStream << std::setw(4) << DefaultShortCutSettings << std::endl;
        }

        if (!wxRenameFile(tempPath, ShortcutsPath))
        {
            throw std::runtime_error("Failed to replace shortcut settings file with defaults");
        }
    }
    catch (const std::exception &e)
    {
        wxMessageBox(_("Failed to create default shortcut settings"), _("Error"), wxICON_ERROR);
        throw;
    }
}

json ShortCutSettingsManager::MergeWithDefaults(json &data)
{
    bool needsUpdate = false;

    for (const auto &[key, value] : DefaultShortCutSettings.items())
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
RequestedShortcutSetting<T> ShortCutSettingsManager::GetSetting(const std::string &settingName)
{
    std::lock_guard<std::mutex> lock(settingsMutex);
    RequestedShortcutSetting<T> result{};
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

template RequestedShortcutSetting<bool> ShortCutSettingsManager::GetSetting<bool>(const std::string &settingName);
template RequestedShortcutSetting<int> ShortCutSettingsManager::GetSetting<int>(const std::string &settingName);
template RequestedShortcutSetting<std::string> ShortCutSettingsManager::GetSetting<std::string>(const std::string &settingName);