#pragma once

/**
 * @file controlPanel.hpp
 * @brief Declaration of the ControlPanel class, a global command palette interface.
 */

#include "themesManager/themesManager.hpp"
#include "appPaths/appPaths.hpp"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "ui/ids.hpp"
#include <wx/wx.h>
#include <wx/scrolwin.h>

/**
 * @struct ControlMenu
 * @brief Structure to define an item in the command palette menu.
 */
struct ControlMenu
{
    const char *name; /**< The display name of the command. */
    const char *shortkut; /**< The keyboard shortcut associated with the command. */
    int ID; /**< A unique integer ID for the command. */
};

/**
 * @class ControlPanel
 * @brief Implements a floating command palette (similar to VS Code's Ctrl+P/Ctrl+Shift+P)
 * for quick access to application commands.
 */
class ControlPanel : public wxPanel
{
public:
    /**
     * @brief Constructs the ControlPanel.
     * @param parent The parent frame where the panel will be positioned (usually MainFrame).
     * @param ID The unique ID for the panel.
     */
    ControlPanel(wxFrame *parent, wxWindowID ID);

    /**
     * @brief Closes and destroys the control panel.
     * @param WXUNUSED(event) The command event (e.g., triggered by Escape key).
     */
    void Close(wxCommandEvent &WXUNUSED(event));

    /**
     * @brief Moves the selection highlight up in the list.
     * @param WXUNUSED(event) The command event (e.g., triggered by Up Arrow key).
     */
    void UpSelection(wxCommandEvent &WXUNUSED(event));

    /**
     * @brief Moves the selection highlight down in the list.
     * @param WXUNUSED(event) The command event (e.g., triggered by Down Arrow key).
     */
    void DownSelection(wxCommandEvent &WXUNUSED(event));

    /**
     * @brief Handles mouse clicks on a menu item to select a command.
     * @param event The mouse event.
     */
    void OnClickSelect(wxMouseEvent &event);

    /**
     * @brief Executes the command based on the provided menu ID.
     * @param id The ID of the command (as a wxString) to execute.
     */
    void Select(wxString id);

    /**
     * @brief Handles text changes in the search input to filter the menu list.
     * @param WXUNUSED(event) The text control event.
     */
    void SearchInputModified(wxCommandEvent &WXUNUSED(event));

    /**
     * @brief Selects the currently highlighted menu item using the keyboard.
     * @param WXUNUSED(event) The command event (e.g., triggered by Enter key).
     */
    void OnKeyBoardSelect(wxCommandEvent &WXUNUSED(event));

private:
    wxScrolled<wxPanel> *menusContainer; /**< Container for the scrollable list of menu items. */
    wxPanel *selectedMenu; /**< The currently highlighted menu item. */
    wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL); /**< Main sizer for the panel layout. */
    wxTextCtrl *searchInput; /**< The input field for filtering commands. */
    json Theme = ThemesManager::Get().currentTheme; /**< Cached theme settings. */
    wxString iconsDir = ApplicationPaths::AssetsPath("icons"); /**< Path to the icons directory. */

    std::vector<ControlMenu> menus{
        {"Open Terminal", "Ctrl+Shift+T", 1},
    }; /**< List of available commands. */

    wxDECLARE_NO_COPY_CLASS(ControlPanel);
    wxDECLARE_EVENT_TABLE();
};