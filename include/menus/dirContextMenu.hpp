#pragma once

#include <wx/wx.h>

/**
 * @namespace DirContextMenu
 * @brief Provides utility functions for creating context menus related to directory operations.
 *
 * This namespace encapsulates functionality for building the context menu that appears
 * when a user right-clicks on a directory inside the project file tree or explorer.
 * It provides options such as renaming, creating, or deleting folders and files.
 */
namespace DirContextMenu
{
    /**
     * @brief Creates and returns the context menu for directory-related operations.
     *
     * The context menu includes actions such as renaming the directory, creating a new file,
     * creating a new folder, and deleting the selected folder.
     * The caller is responsible for managing the menu's lifetime (i.e., deleting it
     * when no longer needed).
     *
     * @return A pointer to a new wxMenu instance containing directory-related actions.
     */
    wxMenu* Get(bool targetIsProjectInformations);
}
