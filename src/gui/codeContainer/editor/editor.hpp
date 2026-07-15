#pragma once

/**
 * @file Editor.hpp
 * @brief Advanced code editor widget with LSP integration, syntax highlighting, and intelligent editing features.
 *
 * The Editor class extends wxStyledTextCtrl (Scintilla) to provide a professional
 * code editing experience. It integrates with the Language Server Protocol (LSP)
 * for intelligent code completion, diagnostics, and symbol information. Additional
 * features include automatic pair completion, selection occurrence highlighting,
 * code folding, and extensive customization through theme and language preferences.
 *
 * ## Key Features:
 * - **LSP Integration**: Real-time code completion, diagnostics, hover information, and document symbols
 * - **Syntax Highlighting**: Language-specific lexer with customizable keyword lists
 * - **Code Folding**: Configurable fold margin with preprocessor and compact folding support
 * - **Smart Editing**: Auto-pairing of brackets/quotes, smart indentation, tag closing
 * - **Selection Highlighting**: Visual marking of all occurrences of selected text
 * - **Line Manipulation**: Move, duplicate, and remove lines with keyboard shortcuts
 * - **Theme Integration**: Dynamic styling from application theme system
 * - **Auto-completion**: Both local keyword-based and LSP-powered completion suggestions
 * - **Unsaved Changes Indicator**: Visual feedback for modified files in the tab interface
 *
 * ## Dependencies:
 * - wxStyledTextCtrl (Scintilla) for the text editing component
 * - LspClient for Language Server Protocol communication
 * - ThemesManager for visual styling
 * - LanguagesPreferences for language-specific settings
 * - ProjectSettings for project-level configuration
 */

#include "ui/ids.hpp"
#include "projectSettings/projectSettings.hpp"
#include "themesManager/themesManager.hpp"
#include "appPaths/appPaths.hpp"
#include "gui/widgets/statusBar/statusBar.hpp"
#include "languagesPreferences/languagesPreferences.hpp"
#include "userSettings/userSettings.hpp"
#include "lsp/lspClient/lspClient.hpp"

class CodeContainer;

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <wx/stc/stc.h>
#include <wx/timer.h>
#include <memory>
#include <vector>
#include <unordered_map>
#include <unordered_set>

class CodeContainer;
class LspClient;

namespace
{
    /**
     * @brief Mapping of closing characters to their opening counterparts.
     * 
     * Used by the smart backspace/delete handler to detect when the cursor
     * is positioned between a matching pair of brackets, quotes, or other
     * delimiters. When deleting one character of a pair, both are removed
     * simultaneously to maintain balanced pairs.
     * 
     * Example pairs:
     * - `"` ↔ `"`
     * - `]` ↔ `[`
     * - `}` ↔ `{`
     * - `)` ↔ `(`
     */
    static const std::unordered_map<wxString, wxString> kPairMap = {
        {"\"", "\""}, {"'", "'"}, {"]", "["}, {"}", "{"}, {")", "("}};
}

namespace EditorConstants
{
    /** @brief Margin index for line numbers display. */
    constexpr int LINE_NUMBER_MARGIN = 0;
    
    /** @brief Margin index for code folding symbols (expand/collapse). */
    constexpr int FOLD_MARGIN = 2;
    
    /** @brief Width in pixels of the fold margin area. */
    constexpr int FOLD_MARGIN_WIDTH = 20;
    
    /** @brief Indicator style index used for highlighting text occurrences. */
    constexpr int INDICATOR_DEFAULT = 0;
    
    /** @brief Maximum indicator index supported by the editor. */
    constexpr int MAX_INDICATOR = 7;
    
    /** @brief Minimum number of characters required to trigger selection highlighting. */
    constexpr int MIN_SELECTION_LENGTH = 2;
}

/**
 * @class Editor
 * @brief Advanced code editor widget with comprehensive editing and LSP features.
 * 
 * The Editor is the primary text editing component of the application. It extends
 * wxStyledTextCtrl to provide a feature-rich code editing environment with support
 * for multiple programming languages through LSP integration, syntax highlighting,
 * and intelligent editing helpers.
 * 
 * The editor manages its own LSP client lifecycle, including initialization,
 * document synchronization, and completion requests. It communicates with the
 * language server through the LspClient class and processes responses to provide
 * real-time feedback such as code completion suggestions and diagnostic messages.
 * 
 * ## LSP Integration:
 * The editor can connect to any LSP-compliant language server. The connection
 * lifecycle follows these stages:
 * 1. **Initialization**: Start server process and perform LSP handshake
 * 2. **Document Open**: Send file content and language identifier
 * 3. **Ready State**: Wait for initial diagnostics (file indexed)
 * 4. **Active Editing**: Send change notifications and request completions
 * 
 * ## Auto-completion:
 * Two-tier completion system:
 * - **Local**: Fast keyword-based completion from language preferences
 * - **LSP**: Context-aware intelligent completion from the language server
 * 
 * The LSP completion is debounced (400ms) for efficiency, while immediate
 * completion is requested when typing identifiers of 2+ characters.
 */
class Editor : public wxStyledTextCtrl
{
    /**
     * @enum StyleIndices
     * @brief Style indices for syntax highlighting.
     * 
     * These indices correspond to the style numbers used by the Scintilla
     * lexer to apply different visual attributes to text tokens. Each
     * language may define additional style indices.
     */
    enum
    {
        STYLE_DEFAULT = 0,  /**< Default/base style applied to unstyled text. */
        STYLE_KEYWORD = 19  /**< Style index for language keywords/reserved words. */
    };

public:
    /**
     * @brief Constructs the Editor control.
     * 
     * Initializes the wxStyledTextCtrl with the parent container, configures
     * all visual and behavioral settings from themes and preferences, sets up
     * margins (line numbers and folding), and binds all event handlers.
     * 
     * @param parent Pointer to the parent window, expected to be a CodeContainer.
     */
    Editor(wxWindow *parent);
    ~Editor();
    
    /**
     * @brief Initializes the Language Server Protocol client.
     * 
     * Starts the LSP server process, performs the initialization handshake,
     * opens the current document for analysis, and registers callbacks for
     * file readiness and completion requests.
     * 
     * The method detects the programming language from the file extension
     * and selects the appropriate LSP server and language identifier.
     * 
     * @note Must be called after the editor content is loaded.
     * @note Only one LSP client is active per editor instance.
     */
    void Lsp();

    /**
     * @brief Checks if the document has unsaved modifications.
     * @return true if the document is modified and not read-only.
     */
    bool Modified() const;

    /**
     * @brief Sets the list of local auto-completion words.
     * 
     * These words are used as a fallback when LSP completion is
     * unavailable (server not ready or not configured for the language).
     * 
     * @param words Vector of completion keywords for the current language.
     */
    void SetAutoCompleteWordsList(const std::vector<wxString> &words) { m_AutoCompleteWordsList = words; }

    /**
     * @brief Sets the language preferences for the current file type.
     * 
     * Configures language-specific settings including lexer, auto-pairing
     * rules, comment styles, and other syntax-related options.
     * 
     * @param languagePreferences Structure containing all language-specific configuration.
     */
    void SetLanguagesPreferences(languagePreferencesStruct languagePreferences) { this->m_LanguagePreferences = languagePreferences; }

    /**
     * @brief Moves selected lines up by one position.
     * 
     * If no selection exists, moves the line containing the caret.
     * Maintains proper indentation and relative positioning.
     */
    void MoveSelectedLinesUp();

    /**
     * @brief Moves selected lines down by one position.
     * 
     * If no selection exists, moves the line containing the caret.
     * Maintains proper indentation and relative positioning.
     */
    void MoveSelectedLinesDown();

    /**
     * @brief Removes the current line or selected text.
     * 
     * If text is selected, removes only the selection. Otherwise, removes
     * the entire line where the caret is positioned, regardless of the
     * caret's column position within the line.
     * 
     * The operation is wrapped in a single undo action for proper
     * Undo/Redo behavior.
     * 
     * @note Bound to Ctrl+Delete by default.
     */
    void RemoveCurrentLine();
    
    /**
     * @brief Handles the copy-to-clipboard action.
     * 
     * Copies selected text to the clipboard. If no text is selected,
     * the behavior is determined by the implementation (e.g., copy
     * the current line or do nothing).
     * 
     * @param event Command event from the copy action (Ctrl+C or menu).
     */
    void OnCopy(wxCommandEvent &event);
    
    /**
     * @brief Duplicates the current line below itself.
     * 
     * If text is selected, duplicates the selection instead.
     * Places the caret at the beginning of the duplicated content.
     * 
     * @param event Command event triggering the duplication.
     */
    void OnDuplicateLineDown(wxCommandEvent &event);
    
    /**
     * @brief Toggles line comments for the current line or selection.
     * 
     * Adds or removes single-line comments (e.g., "//" for C++) based
     * on whether the line is currently commented.
     * 
     * @param event Command event triggering the toggle.
     */
    void OnToggleLineComment(wxCommandEvent& event);
    
    /**
     * @brief Toggles block comments for the current selection.
     * 
     * Wraps or unwraps the selection with block comment delimiters
     * (e.g., " / *" and "* /" for C++).
     * 
     * @param event Command event triggering the toggle.
     */
    void OnToggleBlockComment(wxCommandEvent& event);

    /**
     * @brief Pointer to the parent CodeContainer.
     * 
     * The CodeContainer manages tabs, file operations, and coordinates
     * between multiple editor instances. May be nullptr in standalone use.
     */
    CodeContainer *m_linked_container = nullptr;

private:
    wxString currentPath;                          ///< Path of the currently opened file (empty if unsaved).
    StatusBar *statusBar = ((StatusBar *)FindWindowById(+GUI::ControlID::StatusBar));  ///< Application status bar for position info.

    bool changedFile = false;                      ///< Tracks unsaved modifications for the tab indicator.
    std::vector<wxString> m_AutoCompleteWordsList; ///< Local keywords for fallback auto-completion.
    languagePreferencesStruct m_LanguagePreferences; ///< Active language-specific configuration.
    bool m_isDestroyed = false;
    
    // --- Line Manipulation Handlers ---
    void OnMoveCursorDown(wxCommandEvent &event);  ///< Moves current line/selection down.
    void OnMoveCursorUp(wxCommandEvent &event);    ///< Moves current line/selection up.
    void OnDuplicateLineUp(wxCommandEvent &event); ///< Duplicates current line above itself.
    void OnZoomIn(wxCommandEvent &event);           ///< Increases editor font size.
    void OnZoomOut(wxCommandEvent &event);          ///< Decreases editor font size.
    void SelectNextOccurrence(wxCommandEvent &event); ///< Selects next occurrence of current selection.

    // --- Core Configuration ---
    void InitializePreferences();  ///< Configures editor appearance and behavior from settings.
    void ConfigureFoldMargin();    ///< Sets up the code folding margin appearance.
    void BindEvents();             ///< Binds all event handlers to their respective events.

    // --- wxStyledTextCtrl Event Handlers ---
    void OnUpdateUI(wxStyledTextEvent &event);   ///< Updates UI state (highlights, status bar).
    void OnChange(wxStyledTextEvent &event);      ///< Handles content modifications.
    void OnMarginClick(wxStyledTextEvent &event); ///< Handles fold margin clicks.
    void OnBackspace(wxKeyEvent &event);           ///< Smart backspace/delete handler.
    void OnArrowsPress(wxKeyEvent &event);         ///< Cursor movement handler.
    void CharAdd(wxStyledTextEvent &event);        ///< Character insertion handler (completion triggers).
    void OnEnterKey(wxStyledTextEvent &event);     ///< Smart indentation on Enter.
    void OnClick(wxMouseEvent &event);             ///< Mouse click handler.
    void OnScroll(wxMouseEvent &event);            ///< Scroll synchronization handler.

    // --- Utility Methods ---
    void HighlightSelectionOccurrences();  ///< Marks all instances of selected text.
    void ClearIndicators();                ///< Clears all visual indicators in the document.
    void UpdateUnsavedIndicator();         ///< Updates the tab icon for unsaved changes.
    void HandleAutoPairing(char chr);      ///< Inserts matching closing character.
    void ShowLocalCompletion(const wxString& word, int len); ///< Shows keyword-based completion.
    void OnHorizontalScroll(wxMouseEvent &event);           ///< Horizontal scroll with Shift+Wheel.
    void OnLspSyncTimer(wxTimerEvent& event);               ///< Timer handler for LSP synchronization.
    void SetupAutoComplete();              ///< Configures auto-completion behavior and appearance.

    /**
     * @brief Parses LSP completion response into Scintilla-compatible format.
     * 
     * Extracts completion item labels from the JSON response, deduplicates,
     * limits to 30 items, and formats them as a space-separated string
     * for display in the auto-completion popup.
     * 
     * @param json Raw JSON response from the LSP completion request.
     * @param prefix Current word prefix being completed.
     * @return Space-separated list of completion items.
     */
    wxString ParseCompletionItems(const std::string& json, const wxString& prefix);

    // --- Theme and Paths ---
    const json Theme = ThemesManager::Get().currentTheme;     ///< Active theme JSON object.
    const wxString iconsDir = ApplicationPaths::AssetsPath("icons"); ///< Path to editor icons.
    wxString m_serverPath;                                    ///< Path to the LSP server executable.

    ProjectSettings &projectSettings = ProjectSettings::Get(); ///< Reference to project settings.

    // --- LSP Integration ---
    std::unique_ptr<LspClient> m_lsp;  ///< LSP client instance (nullptr if not active).
    bool m_lspReady = false;           ///< True when server has indexed the file.
    int m_docVersion = 0;              ///< Monotonically increasing document version.
    wxTimer m_lspSyncTimer;            ///< Timer for periodic LSP synchronization.
    wxTimer m_lspDebounceTimer;        ///< Timer for debouncing change notifications.
    
    bool m_documentOpened = false;
    bool m_lspStarting = false;  // ← ADICIONAR

    std::vector<wxString> m_autoCompleteWords; ///< Cached local completion words.

    static constexpr int LSP_SYNC_TIMER_ID = 1001;  ///< Timer ID for LSP sync events.
    static constexpr int LSP_DEBOUNCE_ID = 1002;    ///< Timer ID for LSP debounce events.
    
    int m_lastCompletionLine = 0;
    int m_lastCompletionCol = 0;
    wxString m_lastCompletionUri;
    wxString m_lastSyncedText;
    int m_completionRequestCount = 0;
wxStopWatch m_completionRateLimiter;

int m_completionCount = 0;
wxStopWatch m_completionTimer;
bool m_lspNeedsReset = false;

    bool m_completionPause = false;
    wxLongLong m_lastPauseTime = 0;
    
    void OnLspDebounceTimer(wxTimerEvent &event);

    wxDECLARE_NO_COPY_CLASS(Editor);
    wxDECLARE_EVENT_TABLE();
};