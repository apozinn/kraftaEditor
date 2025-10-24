#pragma once

/**
 * @file wxImagePanel.hpp
 * @brief Declaration of the wxImagePanel class, a custom panel for displaying and manipulating images.
 */

#include <wx/wx.h>

/**
 * @class wxImagePanel
 * @brief A custom wxPanel designed to load, display, and potentially manipulate a single image (Bitmap).
 *
 * It handles the painting logic necessary to render the image on the panel.
 */
class wxImagePanel : public wxPanel {
public:
    /**
     * @brief Constructs the wxImagePanel.
     *
     * @param parent The parent window.
     * @param file The path to the image file to load.
     * @param format The format of the image file (e.g., wxBITMAP_TYPE_PNG).
     * @param size The desired size (used for scaling/rescaling logic, default 0).
     * @param rotate The rotation angle in degrees (default 0.0).
     */
    wxImagePanel(
        wxWindow* parent, 
        wxString file, 
        wxBitmapType format, 
        int size = 0, 
        double rotate = 0.0
    );

    /**
     * @brief Event handler for paint events (wxEVT_PAINT).
     * @param event The paint event.
     */
    void paintEvent(wxPaintEvent& event);

    /**
     * @brief Forces an immediate redraw of the panel.
     *
     * Internally calls Refresh() and Update().
     */
    void paintNow();

    /**
     * @brief The core rendering function that draws the bitmap onto the device context.
     * @param dc The device context to draw on.
     */
    void render(wxDC& dc);

    /**
     * @brief (Placeholder for) A method to rescale the image based on size constraints.
     * @param min The minimum size constraint.
     * @param max The maximum size constraint.
     */
    void RescaleToBestSize(int min, int max);

private:
    wxBitmap image; /**< The bitmap object holding the image data. */
    double i_rotate; /**< Stores the rotation angle for the image. */

    wxDECLARE_NO_COPY_CLASS(wxImagePanel);
    DECLARE_EVENT_TABLE()
};