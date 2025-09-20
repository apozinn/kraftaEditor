#pragma once

#include <wx/wx.h>

/**
 * @namespace FileContextMenu
 * @brief Provides utility functions for creating context menus related to file operations.
 *
 * This namespace encapsulates functionality for building the context menu that appears
 * when a user right-clicks on a file inside the project file tree or editor.
 * It provides options such as renaming or deleting a file.
 */
namespace FileContextMenu
{
    /**
     * @brief Creates and returns the context menu for file-related operations.
     *
     * The context menu includes actions such as renaming and deleting a file.
     * The caller is responsible for managing the menu's lifetime (i.e., deleting it
     * when no longer needed).
     *
     * @return A pointer to a new wxMenu instance containing file-related actions.
     */
    wxMenu* Get();
}
