#include "wx/string.h"

/**
 * @namespace ErrorMessages
 * @brief Centralized collection of error message strings used across the application.
 *
 * Provides consistent user-facing error messages for file, directory, and project operations.
 */
namespace ErrorMessages
{
    inline const wxString FileNotFound = _("File not found.");
    inline const wxString CannotOpenDir = _("Unable to open the directory.");
    inline const wxString DirNotFound = _("Directory not found.");
    inline const wxString CannotOpenDirForReadContent = _("Unable to read contents of the directory.");
    inline const wxString InvalidProjectPath = _("The specified project path is invalid.");
    inline const wxString ComponentFindError = _("Failed to retrieve a GUI component. Please restart the application.");
    inline const wxString CreateMenuContextError = _("Failed to create the context menu.");
    inline const wxString RenameDirRequestedError = _("Failed to rename the directory. Error: ");
    inline const wxString RenameFileRequestedError = _("Failed to rename the file. Error: ");
    inline const wxString ToggleDirectoryVisibilityError = _("Failed to toggle directory visibility.");
    inline const wxString CreateDirRequestedError = _("Failed to create the directory. Error: ");
    inline const wxString CreateFileRequestedError = _("Failed to create the file. Error: ");
    inline const wxString DeleteDirRequestedError = _("Failed to delete the directory. Error: ");
    inline const wxString DeleteFileRequestedError = _("Failed to delete the file. Error: ");
    inline const wxString DirNameIsNotValid = _("The provided directory name is not valid.");
    inline const wxString FileNameIsNotValid = _("The provided file name is not valid.");
    inline const wxString CannotFindDirectoryParent = _("Unable to locate the parent directory.");
};
