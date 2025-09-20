#pragma once

#include <wx/wx.h>

/**
 * @namespace TabsContainerMenu
 * @brief Provides utility functions for creating context menus related to tab container operations.
 *
 * This namespace encapsulates functionality for building the context menu that appears
 * when a user right-clicks on a tab in the tab container. It provides options
 * such as closing the tab, closing other tabs, closing tabs to the right.
 */
namespace TabsContainerMenu
{
    /**
     * @brief Creates and returns the context menu for tab container-related operations.
     *
     * The context menu includes actions such as closing the current tab, closing other tabs,
     * closing tabs to the right, and reloading the selected tab.
     * The caller is responsible for managing the menu's lifetime (i.e., deleting it
     * when no longer needed).
     *
     * @return A pointer to a new wxMenu instance containing tab container-related actions.
     */
    wxMenu* Get();
}