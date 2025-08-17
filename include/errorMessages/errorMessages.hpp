#include "wx/string.h"

/**
 * @namespace ErrorMessages
 * @brief Centralized collection of error message strings used across the application.
 *
 * Provides consistent user-facing error messages for file, directory, and project operations.
 */
namespace ErrorMessages
{
    inline const wxString FileNotFound = "File not found.";
    inline const wxString CannotOpenDir = "Unable to open the directory.";
    inline const wxString DirNotFound = "Directory not found.";
    inline const wxString CannotOpenDirForReadContent = "Unable to read contents of the directory.";
    inline const wxString InvalidProjectPath = "The specified project path is invalid.";
    inline const wxString ComponentFindError = "Failed to retrieve a GUI component. Please restart the application.";
    inline const wxString CreateMenuContextError = "Failed to create the context menu.";
    inline const wxString RenameDirRequestedError = "Failed to rename the directory. Error: ";
    inline const wxString RenameFileRequestedError = "Failed to rename the file. Error: ";
    inline const wxString ToggleDirectoryVisibilityError = "Failed to toggle directory visibility.";
    inline const wxString CreateDirRequestedError = "Failed to create the directory. Error: ";
    inline const wxString CreateFileRequestedError = "Failed to create the file. Error: ";
    inline const wxString DeleteDirRequestedError = "Failed to delete the directory. Error: ";
    inline const wxString DeleteFileRequestedError = "Failed to delete the file. Error: ";
    inline const wxString DirNameIsNotValid = "The provided directory name is not valid.";
    inline const wxString FileNameIsNotValid = "The provided file name is not valid.";
    inline const wxString CannotFindDirectoryParent = "Unable to locate the parent directory.";
};
