#pragma once

/**
 * @file minimap.hpp
 * @brief Declaration of the MiniMap class, a specialized wxStyledTextCtrl used for code overview.
 */

#include "ui/ids.hpp"
#include "gui/widgets/statusBar/statusBar.hpp"
#include "themesManager/themesManager.hpp"
#include "appPaths/appPaths.hpp"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <wx/stc/stc.h>

/**
 * @class MiniMap
 * @brief Provides a miniaturized view of the code editor content for quick navigation.
 *
 * It is derived from wxStyledTextCtrl but heavily customized to display a zoomed-out,
 * non-editable version of the main editor's content.
 */
class MiniMap : public wxStyledTextCtrl
{
public:
    /**
     * @brief Constructs the MiniMap control.
     * @param parent The parent window.
     */
    MiniMap(wxWindow *parent);

private:
    /**
     * @brief Initializes and applies visual preferences (colors, margins, zoom level)
     * based on the current theme.
     */
    void InitializePrefs();

    /**
     * @brief Handles mouse clicks on the minimap to quickly jump the main editor to that position.
     * @param WXUNUSED(event) The mouse event.
     */
    void OnClick(wxMouseEvent &WXUNUSED(event));

    /**
     * @brief Handles paint events (wxEVT_PAINT).
     * @param WXUNUSED(event) The paint event.
     */
    void OnPaint(wxPaintEvent &WXUNUSED(event));

    /**
     * @brief Handles styled text paint completion events (wxEVT_STC_PAINTED).
     *
     * Used to draw visual indicators like the scroll window highlight.
     * @param WXUNUSED(event) The styled text event.
     */
    void OnPainted(wxStyledTextEvent &WXUNUSED(event));

    /**
     * @brief Handles mouse entering the window area.
     * @param WXUNUSED(event) The mouse event.
     */
    void OnMouseEnter(wxMouseEvent &WXUNUSED(event));

    /**
     * @brief Handles mouse leaving the window area.
     * @param WXUNUSED(event) The mouse event.
     */
    void OnMouseLeave(wxMouseEvent &WXUNUSED(event));

private:
    json Theme = ThemesManager::Get().currentTheme; /**< Cache of the current theme settings. */
    wxStyledTextCtrl *Editor; /**< Pointer to the linked main code editor. */
    wxString currentPath; /**< Path of the file currently displayed (used for linkage). */
    int clickPoint = 0; /**< Y-coordinate of the last click in the minimap area. */
    bool mouseOver = false; /**< Tracks if the mouse cursor is currently over the minimap. */
    StatusBar *statusBar = ((StatusBar *)FindWindowById(+GUI::ControlID::StatusBar)); /**< Pointer to the status bar for possible updates. */
};