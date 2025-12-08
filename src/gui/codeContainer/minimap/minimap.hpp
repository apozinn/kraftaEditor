#pragma once

/**
 * @file minimap.hpp
 * @brief Declaration of the MiniMap class and related data structures for rendering
 *        a code overview minimap synchronized with a wxStyledTextCtrl editor.
 */

#include "ui/ids.hpp"
#include "gui/widgets/statusBar/statusBar.hpp"
#include "themesManager/themesManager.hpp"
#include "appPaths/appPaths.hpp"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <wx/stc/stc.h>

/**
 * @typedef sptr_t
 * @brief Signed pointer-sized integer type used by Scintilla for message parameters.
 */
typedef intptr_t sptr_t;

/**
 * @typedef uptr_t
 * @brief Unsigned pointer-sized integer type used by Scintilla for message parameters.
 */
typedef uintptr_t uptr_t;

/**
 * @struct Sci_TextRange
 * @brief Structure used by Scintilla to request both text and style information
 *        over a given character range.
 *
 * This mirrors the `TextRange` structure from Scintilla's C API.
 * When used with `SCI_GETSTYLEDTEXT`, Scintilla writes pairs of bytes:
 *   - byte 0: character
 *   - byte 1: style index (0–255)
 */
struct Sci_TextRange
{
    /**
     * @struct Sci_CharacterRange
     * @brief Defines a half-open interval [cpMin, cpMax) in the document.
     */
    struct Sci_CharacterRange
    {
        long cpMin; ///< Start position of the range (inclusive).
        long cpMax; ///< End position of the range (exclusive).
    } chrg;

    char *lpstr; ///< Pointer to user-allocated buffer where Scintilla writes character/style pairs.
};

/**
 * @struct Color
 * @brief Simple RGB container used to cache style colors.
 */
struct Color
{
    unsigned char r; ///< Red component (0–255).
    unsigned char g; ///< Green component (0–255).
    unsigned char b; ///< Blue component (0–255).
};

/**
 * @struct MiniRun
 * @brief Represents a run of consecutive characters that share the same style.
 *
 * The minimap does not draw a rectangle per character (too slow); instead
 * it draws runs of same-color characters in a compressed horizontal scale.
 */
struct MiniRun
{
    int line;       ///< Document line in which this run appears.
    int column;     ///< Column (character index) where the run begins.
    int length;     ///< Number of characters in the run.
    wxColour color; ///< Resolved foreground color for this style run.
};

/**
 * @class MiniMap
 * @brief A lightweight, scaled-down visualization of the code in a wxStyledTextCtrl.
 *
 * The minimap renders a compact "overview" of the entire file, similar to the
 * minimap found in editors like VS Code.
 *
 * The minimap performs the following tasks:
 *
 *   - Requests the full styled text from Scintilla using `SCI_GETSTYLEDTEXT`,
 *     retrieving pairs of (character, style index).
 *   - Converts style indices into actual RGB colors.
 *   - Builds horizontal runs (`MiniRun`) of consecutive characters that share
 *     the same style, to minimize drawing overhead.
 *   - Scales text visually (scaleX, scaleY) to compress the representation.
 *   - Paints the collected runs inside its drawing area.
 *
 * The minimap does *not* attempt to emulate text glyphs; it only displays
 * colored rectangles that approximate the distribution of syntax-highlighted
 * content across the file.
 */
class MiniMap : public wxPanel
{
public:
    /**
     * @brief Constructs a new MiniMap instance.
     *
     * @param parent  Parent wxWindow that owns this minimap panel.
     * @param editor  Pointer to the main wxStyledTextCtrl whose content is visualized.
     */
    MiniMap(wxWindow *parent, wxStyledTextCtrl *editor);

    /**
     * @brief Extracts styled text from the editor and rebuilds the internal run list.
     *
     * This function:
     *   1. Retrieves the entire styled document using `SCI_GETSTYLEDTEXT`.
     *   2. Converts style indices into cached colors.
     *   3. Computes line start offsets.
     *   4. Generates `MiniRun` structures representing compressed,
        style-based segments of the document.
     *
     * After extraction, the minimap automatically schedules a repaint.
     */
    void ExtractStyledText();

    /**
     * @brief Paint event handler responsible for drawing the minimap’s content.
     *
     * The painting algorithm:
     *   1. Clears the background.
     *   2. Iterates through `m_runs`.
     *   3. Draws scaled rectangles representing syntax-highlighted code areas.
     *
     * @param event The paint event (unused).
     */
    void OnPaint(wxPaintEvent &WXUNUSED(event));

    void OnMouseDown(wxMouseEvent &event);
    void OnMouseUp(wxMouseEvent &event);
    void OnMouseMove(wxMouseEvent &event);
    void ScrollToY(int y);

    /**
     * @brief Stores the language preference configuration (fonts, styles, etc.).
     *
     * This is loaded externally from JSON and used to map Scintilla style IDs
     * to actual colors and presentation rules.
     */
    languagePreferencesStruct languagePreferences;

private:
    wxStyledTextCtrl *m_editor;             ///< Pointer to the associated text editor.
    wxMemoryBuffer m_buf;                   ///< Internal buffer used for `SCI_GETSTYLEDTEXT`.
    std::array<wxColour, 256> m_styleColor; ///< Cached colors for each of the 256 Scintilla style indices.
    std::vector<MiniRun> m_runs;            ///< List of compressed style runs for painting.
    std::vector<int> m_lineStart;           ///< Cached starting positions for each editor line.
    ThemesManager &m_themesManager = ThemesManager::Get();

    float scaleX = 1.0f; ///< Horizontal scaling factor for minimap rendering.
    float scaleY = 2.0f; ///< Vertical scaling factor for minimap rendering.
    bool m_dragging = false;
    int m_dragOffset = 0;
    int minimapPaddingLines = 30;
};
