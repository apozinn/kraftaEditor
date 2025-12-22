#pragma once

/**
 * @file menuBar.hpp
 * @brief Declaration of the MenuBar class.
 */

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "ui/ids.hpp"
#include "userSettings/userSettings.hpp"

#include <wx/menu.h>

/**
 * @class MenuBar
 * @brief Application main menu bar.
 *
 * Responsible for creating and managing the top-level application menus
 * (File, Edit, View, etc.), using user settings to configure initial state
 * and behavior when needed.
 */
class MenuBar : public wxMenuBar
{
public:
    /**
     * @brief Constructs the application menu bar.
     *
     * Initializes all menus and menu items, binding them to their respective
     * command identifiers and user actions.
     */
    MenuBar();

private:
    /**
     * @brief Cached user settings.
     *
     * Stores current user preferences such as theme, window behavior,
     * and UI-related options used during menu initialization.
     */
    json UserSettings = UserSettingsManager::Get().currentSettings;

    wxDECLARE_NO_COPY_CLASS(MenuBar);
};
