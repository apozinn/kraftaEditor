#pragma once

#include <wx/wx.h>
#include <wx/dnd.h>

/**
 * @class FrameFileDropTarget
 * @brief File drag-and-drop handler attached to the main application frame.
 *
 * This class enables drag-and-drop support for opening files directly into the
 * application. It is designed to be attached to a wxFrame so that file drops
 * are accepted globally, regardless of the focused child window.
 *
 * On Wayland-based systems, where file drop events may not provide filenames,
 * a fallback file dialog is used to allow manual file selection.
 */
class FrameFileDropTarget : public wxFileDropTarget
{
public:
    /**
     * @brief Constructs the file drop target.
     * @param frame Pointer to the main application frame that receives drop events.
     */
    explicit FrameFileDropTarget(wxFrame* frame);

    /**
     * @brief Handles file drop events.
     *
     * Called automatically by wxWidgets when files are dropped onto the frame.
     * If filenames are provided, they are forwarded to the application to be opened.
     * If the file list is empty and the application is running under Wayland,
     * a fallback open-file dialog is triggered.
     *
     * @param x X coordinate of the drop position.
     * @param y Y coordinate of the drop position.
     * @param filenames List of dropped file paths.
     * @return true if the drop was handled; false otherwise.
     */
    bool OnDropFiles(wxCoord x, wxCoord y,
                     const wxArrayString& filenames) override;

private:
    wxFrame* m_frame; /**< Pointer to the main frame receiving the drop events. */

    /**
     * @brief Checks whether the application is running under a Wayland session.
     * @return true if a Wayland environment is detected; false otherwise.
     */
    bool IsWaylandSession() const;

    /**
     * @brief Opens a fallback file dialog to select a file manually.
     *
     * Used when drag-and-drop does not provide filenames, which can occur
     * on some Wayland compositors.
     */
    void FallbackOpenDialog();
};
