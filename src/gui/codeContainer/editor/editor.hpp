#pragma once

/**
 * @file Editor.hpp
 * @brief Declaration of the Editor class: a specialized text control for advanced code editing.
 *
 * This header defines the **Editor** class, which extends **wxStyledTextCtrl** to implement
 * a feature-rich code editor. Key features include:
 * - **Syntax Highlighting** and **Code Folding** (via style-needed events).
 * - Synchronization with an external **MiniMap** control.
 * - **Selection Occurrence Highlighting** (marking all instances of selected text).
 * - **Intelligent Helpers** for auto-pairing, smart indentation, and tag closing.
 * - **Custom Command Handlers** for line manipulation (duplicate, move cursor).
 * - Integration with global **Themes**, **Project Settings**, and **Language Preferences**.
 */

#include "ui/ids.hpp"
#include "projectSettings/projectSettings.hpp"
#include "themesManager/themesManager.hpp"
#include "appPaths/appPaths.hpp"
#include "gui/widgets/statusBar/statusBar.hpp"
#include "languagesPreferences/languagesPreferences.hpp"
#include "gui/codeContainer/minimap/minimap.hpp"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <wx/stc/stc.h>
#include <vector>
#include <unordered_map>

namespace
{
    /**
     * @brief Map of closing-to-opening pairs for smart deletion logic.
     *
     * Used in OnBackspace to automatically delete an entire pair (e.g., "()" or "[]")
     * when the cursor is between them and Backspace/Delete is pressed.
     */
    static const std::unordered_map<wxString, wxString> kPairMap = {
        {"\"", "\""}, {"'", "'"}, {"]", "["}, {"}", "{"}, {")", "("}};
}

namespace EditorConstants
{
    /** @brief Margin index reserved for line numbers. */
    constexpr int LINE_NUMBER_MARGIN = 0;
    /** @brief Margin index reserved for code folding symbols. */
    constexpr int FOLD_MARGIN = 2;
    /** @brief Fixed width for the fold margin in pixels. */
    constexpr int FOLD_MARGIN_WIDTH = 20;
    /** @brief Default indicator index used primarily for selection occurrence highlighting. */
    constexpr int INDICATOR_DEFAULT = 0;
    /** @brief The maximum valid indicator index that can be used. */
    constexpr int MAX_INDICATOR = 7;
    /** @brief Minimum text length required for selection occurrence highlighting to activate. */
    constexpr int MIN_SELECTION_LENGTH = 2;
}

/**
 * @class Editor
 * @brief Custom **wxStyledTextCtrl** for code editing with advanced features.
 *
 * The **Editor** is the central component for displaying and manipulating code.
 * It manages its visual state based on the current theme and project settings,
 * implements custom event handlers for a refined editing experience, and provides
 * core functionality like syntax analysis and code folding.
 *
 * It relies on several singleton managers (ThemesManager, ProjectSettings)
 * and external UI components (StatusBar, MiniMap) for full functionality.
 */
class Editor : public wxStyledTextCtrl
{
    /**
     * @brief The JSON object representing the currently active theme.
     *
     * **Initialized from ThemesManager::Get().currentTheme**. Stores theme properties
     * used to configure the style colours of the control.
     */
    const json Theme = ThemesManager::Get().currentTheme;

    /**
     * @brief Directory path for editor-related icons and assets.
     *
     * **Initialized using ApplicationPaths::AssetsPath("icons")**.
     */
    const wxString iconsDir = ApplicationPaths::AssetsPath("icons");

    /**
     * @brief Reference to the global project settings manager.
     *
     * Used to read project-specific preferences like tab width, indentation style,
     * and language settings.
     */
    ProjectSettings &projectSettings = ProjectSettings::Get();

    /**
     * @enum
     * @brief Style indices used by the Editor for syntax styling.
     *
     * These constants map to style numbers used by wxStyledTextCtrl to apply
     * different visual attributes (foreground, font, etc.) to text tokens.
     */
    enum
    {
        STYLE_DEFAULT = 0, /**< Default style index (base, unstyled text). */
        STYLE_KEYWORD = 19 /**< Example: style index for keywords or reserved words. */
    };

public:
    /**
     * @brief Constructs an Editor control.
     * @param parent Pointer to the parent wxWindow (container).
     *
     * Initializes the wxStyledTextCtrl, configures appearance (preferences, margins),
     * and sets up initial event bindings.
     */
    Editor(wxWindow *parent);

    /**
     * @brief Checks if the document has unsaved modifications.
     * @return **true** if the document has unsaved changes (**GetModify()** is true) and is not read-only.
     */
    bool Modified() const;

    /**
     * @brief Updates code folding levels based on document structure in a text region.
     * @param fromPos Document position where re-computation of fold levels should start.
     * @param initialFoldLevel The fold level at **fromPos** before processing.
     * @param text The text content of the region to analyze for folding markers.
     *
     * Scans the text for language-specific block delimiters (e.g., **if/endif**, braces)
     * to determine the correct folding level for each line.
     */
    void UpdateFoldLevels(size_t fromPos, int initialFoldLevel, const wxString &text);

    /**
     * @brief Performs syntax highlighting on a specified text range.
     * @param fromPos The starting position (byte/char index) for styling (inclusive).
     * @param toPos The ending position for styling (exclusive).
     * @param text The content of the region to be tokenized and styled.
     *
     * This function calls the internal logic to tokenize the **text** and uses
     * the **wxStyledTextCtrl** API to apply the determined styles within the range.
     */
    void HighlightSyntax(size_t fromPos, size_t toPos, const wxString &text);

    /**
     * @brief Sets the list of words used for auto-completion suggestions.
     * @param words A vector of strings containing all valid auto-completion words.
     */
    void SetAutoCompleteWordsList(const std::vector<wxString> &words) { m_AutoCompleteWordsList = words; }

    /**
     * @brief Sets the language-specific preferences structure for the current file type.
     * @param languagePreferences The structure containing all language-specific settings (lexer, auto-pairing, etc.).
     */
    void SetLanguagesPreferences(languagePreferencesStruct languagePreferences) { this->m_LanguagePreferences = languagePreferences; }

    /**
     * @brief Moves the selected lines one position up.
     *
     * If no full line selection exists, the line where the caret is currently
     * positioned will be moved instead.
     */
    void MoveSelectedLinesUp();

    /**
     * @brief Moves the selected lines one position down.
     *
     * If no full line selection exists, the line where the caret is currently
     * positioned will be moved instead.
     */
    void MoveSelectedLinesDown();

    /**
     * @brief Removes the current line or the active selection.
     *
     * If there is an active text selection, this function removes only the selected
     * text. Otherwise, it removes the entire line where the caret is currently placed,
     * regardless of the caret position within the line.
     *
     * The operation is wrapped in a single undo action, ensuring that Undo/Redo
     * behaves correctly.
     *
     * This function is intended to be bound to the keyboard shortcut:
     * **Ctrl + Delete**.
     */
    void RemoveCurrentLine();

    /**
     * @brief Pointer to an optional synchronized minimap view of the document.
     *
     * Used to display a small-scale, mirrored overview of the code. May be **nullptr**
     * if the minimap feature is disabled or not attached.
     */
    MiniMap *m_linked_minimap = nullptr;

private:
    /**
     * @brief Path of the currently opened file.
     *
     * An empty string if the editor content is unsaved or not tied to a file.
     */
    wxString currentPath;

    /**
     * @brief Pointer to the application's status bar control.
     *
     * **Resolved via FindWindowById(GUI::ControlID::StatusBar)**. Used to update UI
     * elements like cursor position, file encoding, and modification status.
     */
    StatusBar *statusBar = ((StatusBar *)FindWindowById(+GUI::ControlID::StatusBar));

    /**
     * @brief Internal flag indicating if the document has been modified since the last save.
     *
     * This flag supplements **wxStyledTextCtrl::GetModify()** and is used specifically
     * to manage the unsaved indicator in the application's tab bar.
     */
    bool changedFile = false;

    /**
     * @brief List of words for context-aware auto-completion.
     *
     * Populated by **SetAutoCompleteWordsList** based on the current file's language.
     */
    std::vector<wxString> m_AutoCompleteWordsList;

    /**
     * @brief Language-specific configuration for the current document.
     *
     * Contains settings like lexer name, comment styles, and auto-pairing rules.
     */
    languagePreferencesStruct m_LanguagePreferences;

    // --- Command Handlers (Bound via Accelerator Table) ---

    /**
     * @brief Handler to move the current line/selection one line down.
     *
     * Typically triggered by an accelerator (e.g., Ctrl+Shift+Down).
     */
    void OnMoveCursorDown(wxCommandEvent &event);

    /**
     * @brief Handler to move the current line/selection one line up.
     *
     * Typically triggered by an accelerator (e.g., Ctrl+Shift+Up).
     */
    void OnMoveCursorUp(wxCommandEvent &event);

    /**
     * @brief Handler to duplicate the current line one line below it.
     */
    void OnDuplicateLineDown(wxCommandEvent &event);

    /**
     * @brief Handler to duplicate the current line one line above it.
     */
    void OnDuplicateLineUp(wxCommandEvent &event);

    /**
     * @brief Searches for and selects the next occurrence of the currently selected text.
     *
     * Useful for multi-cursor editing, allowing the user to select the next match.
     */
    void SelectNextOccurrence(wxCommandEvent &event);

    // --- Core Editor Configuration ---

    /**
     * @brief Initializes all editor preferences from global and project settings.
     *
     * Configures the control's fundamental behavior, including:
     * - Tab/indent settings (**SetUseTabs**, **SetTabIndents**).
     * - Margin settings (line numbers).
     * - Visual styles (colors, fonts) based on the current **Theme**.
     * - Accelerator table setup.
     */
    void InitializePreferences();

    /**
     * @brief Configures the appearance and behavior of the fold margin.
     *
     * Sets the margin width, type (symbol), mask, sensitivity, and fold colours.
     */
    void ConfigureFoldMargin();

    /**
     * @brief Binds all custom member functions to their respective **wxStyledTextCtrl** and standard wxWidgets events.
     *
     * This is where events like **wxEVT_STC_UPDATEUI**, **wxEVT_KEY_DOWN**, and **wxEVT_STC_CHARADDED** are linked.
     */
    void BindEvents(); // NOTE: The implementation of BindEvents is missing, but the doc is improved.

    // --- Event Handlers (wxStyledTextCtrl and wxWidgets Events) ---

    /**
     * @brief Handler called when the editor UI state needs updating.
     * @param event The styled text event (**wxEVT_STC_UPDATEUI**).
     *
     * Used primarily to trigger **HighlightSelectionOccurrences** and update the status bar.
     */
    void OnUpdateUI(wxStyledTextEvent &event);

    /**
     * @brief Handler called after any document content modification.
     * @param event The styled text event (**wxEVT_STC_CHANGE**).
     *
     * Updates the unsaved indicator, the **changedFile** flag, the minimap content, and the status bar.
     */
    void OnChange(wxStyledTextEvent &event);

    /**
     * @brief Handler for clicks on any margin (primarily the fold margin).
     * @param event The styled text event (**wxEVT_STC_MARGINCLICK**).
     *
     * Toggles folding if the fold margin is clicked.
     */
    void OnMarginClick(wxStyledTextEvent &event);

    /**
     * @brief Custom handler for the Backspace and Delete keys.
     * @param event The key event (**wxEVT_KEY_DOWN**).
     *
     * Implements smart deletion logic, such as deleting matching character pairs (e.g., **"|""** to **"|"**).
     */
    void OnBackspace(wxKeyEvent &event);

    /**
     * @brief Handler for cursor movement keys (arrows).
     * @param event The key event (**wxEVT_KEY_DOWN**).
     *
     * Clears indicators and updates the status bar location information.
     */
    void OnArrowsPress(wxKeyEvent &event);

    /**
     * @brief Handler called immediately after a character is inserted.
     * @param event The styled text event (**wxEVT_STC_CHARADDED**).
     *
     * Triggers auto-completion, auto-pairing, smart indentation, and tag closing logic.
     */
    void CharAdd(wxStyledTextEvent &event);

    /**
     * @brief Handler for generic mouse clicks in the text area.
     * @param event The mouse event (**wxEVT_LEFT_DOWN**).
     *
     * Clears active indicators and updates the status bar locale.
     */
    void OnClick(wxMouseEvent &event);

    /**
     * @brief Handler called when an auto-completion item is accepted.
     * @param event The styled text event (**wxEVT_STC_AUTOCCOMPLETED**).
     *
     * Placeholder for any necessary post-completion cleanup or state adjustment.
     */
    void OnAutoCompCompleted(wxStyledTextEvent &event);

    /**
     * @brief Handler for text pasted from the clipboard.
     * @param event The styled text event (**wxEVT_STC_DO_PASTE**).
     *
     * Updates the minimap to reflect the newly pasted content.
     */
    void OnClipBoardPaste(wxStyledTextEvent &event);

    /**
     * @brief Handler for scroll events (mouse wheel or scrollbar interaction).
     * @param event The mouse event (**wxEVT_MOUSEWHEEL** or scrollbar events).
     *
     * Synchronizes the visible line in the minimap with the editor's scroll position.
     */
    void OnScroll(wxMouseEvent &event);

    /**
     * @brief Handler invoked when the editor needs new styling for a region.
     * @param event The styled text event (**wxEVT_STC_STYLENEEDED**).
     *
     * Determines the unstyled range and calls **HighlightSyntax** and **UpdateFoldLevels**.
     */
    void OnStyleNeeded(wxStyledTextEvent &event);

    // --- Utility Methods ---

    /**
     * @brief Highlights all instances of the current selection throughout the document.
     *
     * Uses the **INDICATOR_DEFAULT** to visually mark matching text occurrences.
     */
    void HighlightSelectionOccurrences();

    /**
     * @brief Clears all active visual indicators across the entire document.
     *
     * Removes occurrence highlights, custom error markers, etc.
     */
    void ClearIndicators();

    /**
     * @brief Determines if the character at a given position is a word boundary.
     * @param text The string to check.
     * @param pos The index within the string.
     * @return **true** if the character is whitespace or the end of the string.
     */
    bool IsWordBoundary(const wxString &text, size_t pos) const;

    /**
     * @brief Applies computed style ranges to the document using the wxStyledTextCtrl API.
     * @param fromPos Start position for styling.
     * @param toPos End position for styling.
     * @param blocks Vector of pairs where each pair is **(start position, length)** of a token to be styled.
     */
    void ApplySyntaxHighlighting(size_t fromPos, size_t toPos, const std::vector<std::pair<size_t, size_t>> &blocks);

    /**
     * @brief Applies previously calculated fold level changes to the text control.
     * @param startPos The document start position for applying levels.
     * @param initialLevel The initial fold level at the start of the range.
     * @param textLength Length of the region updated.
     * @param changes Vector of pairs **(line number, new fold level)** representing level updates.
     */
    void ApplyFoldLevels(size_t startPos, int initialLevel, size_t textLength, const std::vector<std::pair<size_t, int>> &changes);

    /**
     * @brief Updates any UI element (e.g., tab icon) that shows the file's unsaved status.
     *
     * Looks up the corresponding tab widget and sets a visual indicator (e.g., a small dot).
     */
    void UpdateUnsavedIndicator();

    /**
     * @brief Updates the content of the minimap to match the editor's buffer.
     */
    void UpdateMiniMapText(int pos, const wxString &text);

    /**
     * @brief Implements smart indentation logic upon inserting a newline.
     * @param prevChar The character before the insertion point.
     * @param nextChar The character after the insertion point.
     *
     * Adjusts the new line's indentation based on characters like '{', '}', or the previous line's indent.
     */
    void HandleNewLineIndentation(char prevChar, char nextChar);

    /**
     * @brief Inserts the corresponding closing character when an opening character is typed.
     * @param chr The just-typed opening character (e.g., '(', '"', '{').
     *
     * Positions the caret between the newly inserted pair.
     */
    void HandleAutoPairing(char chr);

    /**
     * @brief Synchronizes the minimap's visible range with the editor's first visible line.
     */
    void SetMiniMapLine(int line);

    wxDECLARE_NO_COPY_CLASS(Editor);
    wxDECLARE_EVENT_TABLE();
};