#pragma once

#include <wx/wx.h>

/**
 * @class FilesTree
 * @brief Represents the project file explorer panel, handling directory and file structure visualization.
 *
 * This class manages the visual representation of a project's file and directory structure,
 * including user interactions such as clicking, creating, deleting, renaming, and handling file system events.
 */
class FilesTree : public wxPanel
{
public:
    /**
     * @brief Constructs a new FilesTree object.
     * @param parent The parent window.
     * @param ID The window identifier.
     */
    FilesTree(wxWindow *parent, wxWindowID ID);

    /**
     * @brief Loads a project into the file explorer.
     * @param parent The parent window.
     * @param path The path to the project root directory.
     */
    void LoadProject(wxWindow *parent, const wxString &path);

    /**
     * @brief Closes the currently loaded project and clears the file explorer view.
     *
     * This method removes all project-related UI components, resets the project information
     * panel, and clears the internal references to files and directories. After calling this,
     * the file tree will be empty until a new project is loaded with LoadProject().
     */
    void CloseProject();

    /**
     * @brief Handles a user request to create a new directory.
     * @param event The associated command event.
     */
    void OnCreateDirRequested(wxCommandEvent &WXUNUSED(event));

    /**
     * @brief Handles a user request to create a new file.
     * @param event The associated command event.
     */
    void OnCreateFileRequested(wxCommandEvent &WXUNUSED(event));

    /**
     * @brief Handles a user request to delete a directory.
     * @param event The associated command event.
     */
    void OnDeleteDirRequested(wxCommandEvent &WXUNUSED(event));

    /**
     * @brief Handles a user request to delete a file.
     * @param event The associated command event.
     */
    void OnDeleteFileRequested(wxCommandEvent &WXUNUSED(event));

    /**
     * @brief Handles file system change events.
     * @param type The type of file system event.
     * @param oldPath The previous file or directory path.
     * @param newPath The new file or directory path (if applicable).
     */
    void OnFileSystemEvent(int type, const wxString &oldPath, wxString newPath);

    /**
     * @brief Returns the panel holding project information.
     * @return Pointer to the project information panel.
     */
    wxPanel *GetProjectInformations() const { return m_projectInformations; }

    /**
     * @brief Returns the scrollable container with all project files.
     * @return Pointer to the project files container panel.
     */
    wxScrolled<wxPanel> *GetProjectFilesContainer() const { return m_projectFilesContainer; }

    /**
     * @brief Returns the currently selected file panel.
     * @return Pointer to the selected file window.
     */
    wxWindow *GetCurrentSelectedFile() const { return m_currentSelectedFile; }

    /**
     * @brief Opens a file in the editor panel.
     * @param componentIdentifier The unique identifier (path) of the file component.
     * @return True if the file was successfully opened, false otherwise.
     */
    bool OpenFile(const wxString &componentIdentifier);

    /**
     * @brief Highlights a file or directory component visually.
     * @param componentIdentifier The unique identifier (path) of the component.
     */
    void SetFileHighlight(const wxString &componentIdentifier);

private:
    /**
     * @brief Creates and initializes the UI components required to represent a directory.
     *
     * This method generates the visual elements needed to display a directory in the project tree.
     * It creates the container for the directory, associates the directory path, and attaches
     * the necessary event bindings for user interaction (expand/collapse, selection, context menu).
     *
     * @param parent The parent window in which the directory components will be placed.
     * @param path The filesystem path of the directory to represent.
     */
    void CreateDirectoryComponents(wxWindow *parent, const wxString &path);

    /**
     * @brief Creates a container widget for displaying a file.
     * @param parent The parent window.
     * @param path The path to the file.
     * @return A pointer to the created file container widget.
     */
    wxWindow *CreateFileContainer(wxWindow *parent, const wxString &path);

    /**
     * @brief Creates a container widget for displaying a directory.
     * @param parent The parent window.
     * @param path The path to the directory.
     * @param pos Optional index position within the parent sizer.
     * @return A pointer to the created directory container widget.
     */
    wxWindow *CreateDirContainer(wxWindow *parent, wxString path, bool withPosition=false, int pos = 0);

    /**
     * @brief Links mouse click events to the project information components.
     *
     * Sets up event handlers so that clicking on specific project information UI elements
     * triggers the appropriate click actions.
     */
    void LinkClickEventToProjectInformationsComponents();

    /**
     * @brief Handles left-click events on the project information components.
     *
     * @param event The wxMouseEvent containing information about the mouse click.
     */
    void ProjectInformationsLeftClick(wxMouseEvent &WXUNUSED(event));

    /**
     * @brief Handles the paint event for directory child components.
     * @param event The wxPaintEvent triggered by the window system.
     */
    void OnDirChildrensPaint(wxPaintEvent &event);

    /**
     * @brief Handles a left mouse click on a file.
     * @param event The mouse event.
     */
    void OnFileLeftClick(wxMouseEvent &event);

    /**
     * @brief Handles a right mouse click on a file.
     * @param event The mouse event.
     */
    void OnFileRightClick(wxMouseEvent &event);

    /**
     * @brief Handles a left mouse click on a directory.
     * @param event The mouse event.
     */
    void OnDirLeftClick(wxMouseEvent &event);

    /**
     * @brief Handles a right mouse click on a directory.
     * @param event The mouse event.
     */
    void OnDirRightClick(wxMouseEvent &event);

    /**
     * @brief Toggles the visibility of a directory container.
     * @param componentIdentifier The unique identifier (path) of the directory.
     * @param defaultSho Optional default visibility state.
     */
    void ToggleDirVisibility(const wxString &componentIdentifier, bool defaultSho = false);

    /**
     * @brief Handles the paint event for custom rendering.
     * @param event The paint event.
     */
    void OnPaint(wxPaintEvent &event);

    /**
     * @brief Adjusts the size of a specific container to fit its contents.
     * @param componentIdentifier The unique identifier (path) of the component.
     */
    void AdjustContainerSize(wxWindow* target, bool reduceSize=false);

    /**
     * @brief Handles a user request to rename a directory.
     * @param event The associated command event.
     */
    void OnRenameDirRequested(wxCommandEvent &WXUNUSED(event));

    /**
     * @brief Handles a user request to rename a file.
     * @param event The associated command event.
     */
    void OnRenameFileRequested(wxCommandEvent &WXUNUSED(event));

    /**
     * @brief Changes the background color of a specific file component.
     * @param componentIdentifier The unique identifier (path) of the component.
     * @param color The wxColour to apply.
     */
    void ChangeFileBackground(const wxString &componentIdentifier, wxColour color);

    /**
     * @brief Handles the mouse entering a component area.
     * @param event The mouse event.
     */
    void OnComponentMouseEnter(wxMouseEvent &event);

    /**
     * @brief Handles the mouse leaving a component area.
     * @param event The mouse event.
     */
    void OnComponentMouseExit(wxMouseEvent &event);

    wxPanel *m_projectInformations = nullptr;
    wxScrolled<wxPanel> *m_projectFilesContainer = nullptr;
    wxStaticText *m_projectInformationsName = nullptr;
    wxStaticBitmap *m_projectInformationsNameArrow = nullptr;
    wxWindow *m_currentSelectedFile = nullptr;

    wxDECLARE_NO_COPY_CLASS(FilesTree);
    wxDECLARE_EVENT_TABLE();
};
