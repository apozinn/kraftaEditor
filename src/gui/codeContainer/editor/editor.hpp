#pragma once

/**
 * @file Editor.hpp
 * @brief Declaration of the Editor class — a specialized wxStyledTextCtrl used by Krafta Editor.
 *
 * This header defines the Editor class which extends wxStyledTextCtrl and provides:
 * - syntax highlighting hooks,
 * - folding support,
 * - a minimap link,
 * - selection-occurrence highlighting,
 * - auto-pairing and indentation helpers,
 * - editor-specific event handlers.
 *
 * The class references global managers (ThemesManager, ProjectSettings, ApplicationPaths)
 * and a StatusBar control for UI updates.
 */

#include "ui/ids.hpp"
#include "projectSettings/projectSettings.hpp"
#include "themesManager/themesManager.hpp"
#include "appPaths/appPaths.hpp"
#include "gui/widgets/statusBar/statusBar.hpp"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <wx/stc/stc.h>
#include <vector>
#include <unordered_map>

namespace
{
    static const std::unordered_map<wxString, wxString> kPairMap = {
        {"\"", "\""}, {"'", "'"}, {"]", "["}, {"}", "{"}, {")", "("}};
}

/**
 * @class Editor
 * @brief Text editor control with syntax highlighting, folding and helper features.
 *
 * Editor is a custom text control derived from wxStyledTextCtrl that implements
 * project-aware preferences, syntax highlighting and fold-level management,
 * a synchronized minimap view and a set of convenience behaviors (auto-pairing,
 * smart indentation, selection occurrence highlighting, etc.). It binds to
 * various wxStyledTextCtrl events to extend the control behavior required by
 * the application.
 */
class Editor : public wxStyledTextCtrl
{
    /**
     * @brief The JSON object describing the currently loaded theme for styles.
     *
     * Initialized from ThemesManager::Get().currentTheme. This member stores theme
     * properties used when configuring styles for the control.
     */
    json Theme = ThemesManager::Get().currentTheme;

    /**
     * @brief Directory path to editor icons/assets.
     *
     * Uses ApplicationPaths::AssetsPath("icons") to resolve the icons directory.
     */
    wxString iconsDir = ApplicationPaths::AssetsPath("icons");

    /**
     * @brief Reference to global project settings manager.
     *
     * Reference to ProjectSettings::Get() so the editor can read preferences
     * (e.g. tab width, lexer selection) for the currently open project.
     */
    ProjectSettings &projectSettings = ProjectSettings::Get();

    /**
     * @enum
     * @brief Style indices used by the Editor for text styling.
     *
     * These constants map to style numbers used by wxStyledTextCtrl for
     * applying style attributes. Add additional style constants here if needed.
     */
    enum
    {
        STYLE_DEFAULT = 0, /**< Default style index (base). */
        STYLE_KEYWORD = 19 /**< Example: keyword style index. */
    };

public:
    /**
     * @brief Constructs an Editor control.
     * @param parent Pointer to parent wxWindow (owner).
     *
     * The constructor should initialize the styled text control, configure
     * preferences, bind event handlers and optionally connect the minimap control.
     */
    Editor(wxWindow *parent);

    /**
     * @brief Returns whether the editor's document has unsaved modifications.
     * @return true if the document has changes that are not saved; false otherwise.
     *
     * This function should reflect the internal modified/changed flag, or use
     * wxStyledTextCtrl::GetModify() if preferred.
     */
    bool Modified();

    /**
     * @brief Update fold levels for a region of text when text or structure changes.
     * @param fromPos Position in document where updates should start.
     * @param initialFoldLevel The fold level at fromPos before updates.
     * @param text The content of the region used to recompute fold levels.
     *
     * This function analyzes block delimiters (braces, markers, comments, etc.)
     * and computes the folding level changes required from fromPos onward.
     */
    void UpdateFoldLevels(size_t fromPos, int initialFoldLevel, const wxString &text);

    /**
     * @brief Perform syntax highlighting over the provided text range.
     * @param fromPos The start position (byte/char index) to begin highlighting.
     * @param toPos The end position to stop highlighting (exclusive).
     * @param text The text content of the region to analyze.
     *
     * This function tokenizes/lexes the given text and applies styles via the
     * wxStyledTextCtrl style API.
     */
    void HighlightSyntax(size_t fromPos, size_t toPos, const wxString &text);

private:
    /**
     * @brief Pointer to the "minimap" small-scale mirrored view of the document.
     *
     * Mirrored view (another wxStyledTextCtrl) used to display an overview of the
     * document. May be nullptr if no minimap is attached.
     */
    wxStyledTextCtrl *MiniMap = nullptr;

    /**
     * @brief Move the cursor one line down.
     *
     * If the cursor is already at the top of the document, do nothing.
     */
    void OnMoveCursorDown(wxCommandEvent& event);

    /**
     * @brief Move the cursor one line up.
     *
     * If the cursor is already at the bottom of the document, do nothing.
     */
    void OnMoveCursorUp(wxCommandEvent& event);

    /**
     * @brief Duplicate the current line one line down.
     */
    void OnDuplicateLineDown(wxCommandEvent& event);

    /**
     * @brief Duplicate the current line one line up.
     */
    void OnDuplicateLineUp(wxCommandEvent& event);

    /**
     * @brief Select the next occurrence of the currently selected text.
     */
    void SelectNextOccurrence(wxCommandEvent& event);

    /**
     * @brief Path of the currently opened file (if any).
     *
     * Empty string when the editor does not currently correspond to a saved file.
     */
    wxString currentPath;

    /**
     * @brief Pointer to the application's status bar control.
     *
     * Resolved via FindWindowById(GUI::ControlID::StatusBar). Used to display
     * cursor position, file status, warnings, etc.
     */
    StatusBar *statusBar = ((StatusBar *)FindWindowById(+GUI::ControlID::StatusBar));

    /**
     * @brief Internal boolean that indicates whether the current file was changed.
     *
     * This is used to show unsaved indicators and to decide whether to prompt on close.
     */
    bool changedFile = false;

    /**
     * @brief Initialize editor preferences from ProjectSettings/theme defaults.
     *
     * This sets up:
     * - lexer and language-specific preferences,
     * - tab/indent behavior,
     * - style colors and fonts derived from `Theme`.
     *
     * Call during construction or when project settings change.
     */
    void InitializePreferences();

    /**
     * @brief Configure fold margin settings and appearance.
     *
     * Sets up the margin (icons, clickable fold area) and initial fold properties.
     */
    void ConfigureFoldMargin();

    /**
     * @brief Bind the editor-specific event handlers to the control events.
     *
     * Binds events such as wxEVT_STC_UPDATEUI, wxEVT_STC_CHARADDED, mouse events,
     * margin clicks, and keyboard handling to the corresponding member callbacks.
     */
    void BindEvents();

    /**
     * @brief Handler called when the UI needs updating (cursor position / selection).
     * @param event The styled text event.
     *
     * This is typically bound to wxEVT_STC_UPDATEUI and used to refresh status
     * bar info and selection-dependent visuals.
     */
    void OnUpdateUI(wxStyledTextEvent &event);

    /**
     * @brief Handler called when the document text changes.
     * @param event The styled text event.
     *
     * Use this to mark the document as modified, update minimap, re-highlight, etc.
     */
    void OnChange(wxStyledTextEvent &event);

    /**
     * @brief Handler for clicks on the fold margin.
     * @param event The styled text event describing the margin click.
     *
     * Toggle folding, show markers or perform other margin-specific actions.
     */
    void OnMarginClick(wxStyledTextEvent &event);

    /**
     * @brief Handler for the Backspace key press.
     * @param event The key event.
     *
     * May implement smart deletion (e.g. deleting matching pairs) and custom
     * indentation adjustments.
     */
    void OnBackspace(wxKeyEvent &event);

    /**
     * @brief Handler for arrow key presses (cursor movement).
     * @param event The key event.
     *
     * Used for custom cursor movement behavior or to update other UI when
     * arrow keys are pressed.
     */
    void OnArrowsPress(wxKeyEvent &event);

    /**
     * @brief Handler called on character insertion events.
     * @param event The styled text event for the added character.
     *
     * Typical uses: auto-pairing, auto-indentation and triggering completion.
     */
    void CharAdd(wxStyledTextEvent &event);

    /**
     * @brief Handler for generic mouse clicks in the editor area.
     * @param event The mouse event.
     *
     * Responsible for selection handling and possibly position-based commands.
     */
    void OnClick(wxMouseEvent &event);

    /**
     * @brief Called when an auto-completion item is selected/completed.
     * @param event The styled text event.
     *
     * Use this to insert completion text, reapply formatting, and refresh state.
     */
    void OnAutoCompCompleted(wxStyledTextEvent &event);

    /**
     * @brief Called when text is pasted from the clipboard.
     * @param event The styled text event.
     *
     * Responsible for sanitizing pasted text, adjusting indentation, and updating UI.
     */
    void OnClipBoardPaste(wxStyledTextEvent &event);

    /**
     * @brief Handler for scroll events (mouse wheel or scrollbar interactions).
     * @param event The mouse event.
     *
     * Used to synchronize scrolling with the minimap and to lazy-update visible regions.
     */
    void OnScroll(wxMouseEvent &event);

    /**
     * @brief Highlight all occurrences of the current selection within the document.
     *
     * Typically invoked after a selection change; uses indicators to mark repeated
     * occurrences of the selected text.
     */
    void HighlightSelectionOccurrences();

    /**
     * @brief Clear all currently active indicators (occurrence highlights, errors, etc.).
     *
     * Removes visual markers applied with indicators so the editor returns to an unmarked state.
     */
    void ClearIndicators();

    /**
     * @brief Handler invoked when styling is needed by the control.
     * @param event The styled text event (wxEVT_STC_STYLENEEDED).
     *
     * This should determine the region that needs re-styling and call syntax
     * highlighting routines for the affected range.
     */
    void OnStyleNeeded(wxStyledTextEvent &event);

    /**
     * @brief Determines whether the character at pos in text is a word boundary.
     * @param text The text to examine.
     * @param pos The position within text to check (index).
     * @return true if the character at pos is considered a boundary between words.
     *
     * Helpful when detecting whole-word matches for highlighting or completion.
     */
    bool IsWordBoundary(const wxString &text, size_t pos) const;

    /**
     * @brief Apply computed syntax highlighting after tokens/blocks are identified.
     * @param fromPos Start position for applying styles.
     * @param toPos End position for applying styles.
     * @param blocks Vector of pairs representing token ranges to style:
     *               each pair is (start, length) for a token/block.
     */
    void ApplySyntaxHighlighting(size_t fromPos, size_t toPos, const std::vector<std::pair<size_t, size_t>> &blocks);

    /**
     * @brief Apply previously computed fold-level changes to the document.
     * @param startPos The document start position for applying levels.
     * @param initialLevel The initial fold level applied at startPos.
     * @param textLength Length (in characters) of the region to update.
     * @param changes Vector of pairs (position, level) representing fold level updates.
     *
     * This will call the appropriate API on wxStyledTextCtrl to set fold levels
     * and update fold margin icons as necessary.
     */
    void ApplyFoldLevels(size_t startPos, int initialLevel, size_t textLength, const std::vector<std::pair<size_t, int>> &changes);

    /**
     * @brief Updates any UI indicator that shows whether the buffer is unsaved.
     *
     * Usually toggles the window title, status bar mark, or a document tab indicator.
     */
    void UpdateUnsavedIndicator();

    /**
     * @brief Update the text and rendering of the minimap to reflect the editor buffer.
     *
     * Copies or summarizes the primary buffer contents to the MiniMap control and
     * refreshes the minimap viewport to match the main editor scroll position.
     */
    void UpdateMiniMapText();

    /**
     * @brief Handle indentation logic when a new line is inserted.
     * @param prevChar The character immediately before the insertion location.
     * @param nextChar The character immediately after the insertion location.
     *
     * Implements language-aware indentation behavior on newline (e.g. increase indent after '{').
     */
    void HandleNewLineIndentation(char prevChar, char nextChar);

    /**
     * @brief Handle automatic pairing of characters (brackets, quotes, etc).
     * @param chr The just-typed character that may trigger pairing.
     *
     * This will insert the matching closing character and position the caret appropriately.
     */
    void HandleAutoPairing(char chr);

    /**
     * @brief Set the minimap's currently highlighted line based on the main editor caret.
     *
     * Called when the editor scrolls or the caret moves to keep the minimap indicator in sync.
     */
    void SetMiniMapLine();

    wxDECLARE_NO_COPY_CLASS(Editor);
    wxDECLARE_EVENT_TABLE();
};
