#pragma once

#include <wx/wx.h>

class FilesTree;

/**
 * @class ProjectInfosTools
 * @brief Auxiliary tools panel for project information and file tree interaction.
 *
 * ProjectInfosTools is a wxWidgets panel responsible for providing
 * interactive UI controls related to the project file tree, such as
 * collapsing and expanding directory containers.
 *
 * This panel is typically attached to a project-related view and interacts
 * with an existing FilesTree instance through window lookup and cached pointers.
 *
 * The class manages its own layout, hover behavior, and event handling.
 */
class ProjectInfosTools : public wxPanel
{
public:
    /**
     * @brief Constructs the ProjectInfosTools panel.
     *
     * Initializes the panel, configures its layout, and sets up
     * event bindings for hover and click interactions.
     *
     * @param parent Pointer to the parent wxPanel.
     * @param ID     Window identifier for this panel.
     */
    ProjectInfosTools(wxPanel *parent, wxWindowID ID);

private:
    /**
     * @brief Creates and configures the collapse button.
     *
     * Loads the collapse icon, instantiates the wxStaticBitmap button,
     * inserts it into the panel sizer, and binds mouse click events.
     */
    void CreateCollapseButton();

    /**
     * @brief Handles mouse click events on the collapse button.
     *
     * Toggles the visibility of directory containers in the associated
     * FilesTree by iterating over child windows of the project files container.
     *
     * @param event Mouse event triggered by a left button click.
     */
    void OnCollapseButtonClick(wxMouseEvent &event);

    /**
     * @brief Handles mouse enter events.
     *
     * Triggered when the mouse cursor enters the panel or its controls.
     * Typically used to show or highlight the panel.
     *
     * @param event Mouse enter event.
     */
    void OnHoverEnter(wxMouseEvent &event);

    /**
     * @brief Handles mouse leave events.
     *
     * Triggered when the mouse cursor leaves the panel area.
     * Typically used to hide or de-emphasize the panel.
     *
     * @param event Mouse leave event.
     */
    void OnHoverLeave(wxMouseEvent &event);

    /**
     * @brief Cached pointer to the project files container window.
     *
     * This window acts as the parent container for directory widgets.
     * It is resolved lazily using FindWindowById().
     */
    wxWindow* m_projectFilesContainer = nullptr;

    /**
     * @brief Pointer to the associated FilesTree instance.
     *
     * Used to trigger file tree operations such as toggling
     * directory visibility.
     */
    FilesTree* m_filesTree = nullptr;

    /**
     * @brief Main horizontal sizer managing the panel layout.
     */
    wxBoxSizer* m_sizer = new wxBoxSizer(wxHORIZONTAL);

    /**
     * @brief Static bitmap acting as the collapse/expand button.
     */
    wxStaticBitmap *m_collapse_button = nullptr;
};
