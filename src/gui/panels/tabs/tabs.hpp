#pragma once

/**
 * @file tabs.hpp
 * @brief Declaration of the Tabs panel class, which manages open file tabs.
 */

#include "themesManager/themesManager.hpp"
#include "appPaths/appPaths.hpp"
#include "appConstants/appConstants.hpp"
#include "projectSettings/projectSettings.hpp"
#include "gui/widgets/statusBar/statusBar.hpp"
#include "ui/ids.hpp"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <wx/wx.h>
#include <wx/scrolwin.h>

/**
 * @class Tabs
 * @brief Manages the display and functionality of file tabs (open documents) in the application.
 *
 * It allows adding, closing, and switching between different code editor panels.
 */
class Tabs : public wxPanel
{
public:
    /**
     * @brief Constructs the Tabs panel.
     * @param parent The parent window.
     * @param ID The unique ID for the panel.
     */
    Tabs(wxPanel *parent, wxWindowID ID);

    /**
     * @brief Adds a new tab for a given file.
     * @param tab_name The display name for the tab (usually the file name).
     * @param path The full path of the file.
     */
    void Add(wxString tab_name, wxString path);

    /**
     * @brief Closes a specific tab and removes its corresponding editor.
     * @param tab The wxWindow representing the tab panel to close.
     * @param tab_path The path of the file associated with the tab.
     */
    void Close(wxWindow *tab, wxString tab_path);

    /**
     * @brief Closes all currently open files and their respective tabs.
     */
    void CloseAllFiles();

    /**
     * @brief Switches the view to the currently active/selected tab.
     */
    void Select();

    /**
     * @brief Handles mouse click event on a tab to select it.
     * @param event The mouse event.
     */
    void OnTabClick(wxMouseEvent &event);

    /**
     * @brief Handles mouse click event on the close button (X icon) of a tab.
     * @param event The mouse event.
     */
    void OnCloseTab(wxMouseEvent &event);

    /**
     * @brief Handles mouse click event on the menu button (e.g., to open a context menu for tabs).
     * @param WXUNUSED(event) The mouse event.
     */
    void OnMenu(wxMouseEvent &WXUNUSED(event));

    wxString selected_tab;              /**< The file path of the currently selected tab. */
    wxScrolled<wxPanel> *tabsContainer; /**< The container for all individual tabs, enabling horizontal scrolling. */

private:
    /**
     * @brief Handles the mouse entering a tab component to change its appearance.
     * @param event The mouse event.
     */
    void OnEnterComp(wxMouseEvent &event);

    /**
     * @brief Handles the mouse leaving a tab component to revert its appearance.
     * @param event The mouse event.
     */
    void OnLeaveComp(wxMouseEvent &event);

    /**
     * @brief Handles paint event for the main Tabs panel.
     * @param WXUNUSED(event) The paint event.
     */
    void OnPaint(wxPaintEvent &WXUNUSED(event));

    /**
     * @brief Handles paint event for individual tab panels (used for drawing separators, etc.).
     * @param event The paint event.
     */
    void OnTabPaint(wxPaintEvent &event);

    /**
     * @brief Switches the selection to the previous tab.
     */
    void OnPreviousTab();

    /**
     * @brief Switches the selection to the next tab.
     */
    void OnNextTab();

    wxSizer *sizer;                                                                   /**< Main sizer for the Tabs panel. */
    wxStaticBitmap *menu;                                                             /**< Icon/button for the tabs menu. */
    wxBoxSizer *tabsContainerSizer;                                                   /**< Sizer for the tabs inside tabsContainer. */
    json Theme = ThemesManager::Get().currentTheme;                                   /**< Cached theme settings. */
    wxString iconsDir = ApplicationPaths::AssetsPath("icons");                        /**< Path to the icons directory. */
    ProjectSettings &projectSettings = ProjectSettings::Get();                        /**< Reference to global project settings. */
    StatusBar *statusBar = ((StatusBar *)FindWindowById(+GUI::ControlID::StatusBar)); /**< Pointer to the global status bar. */

    wxDECLARE_NO_COPY_CLASS(Tabs);
};