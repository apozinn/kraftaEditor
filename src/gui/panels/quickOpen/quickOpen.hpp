#pragma once

#include <wx/wx.h>
#include <wx/scrolwin.h>
#include <unordered_set>

#include "ui/ids.hpp"
#include "themesManager/themesManager.hpp"

/**
 * @struct QuickOpenFileStruct
 * @brief Represents a file entry indexed and displayed by the Quick Open panel.
 *
 * This structure stores metadata required both for rendering the UI entry
 * and for opening the corresponding file when selected.
 */
struct QuickOpenFileStruct
{
    /** File display name (usually filename only) */
    wxString name;

    /** Full filesystem path to the file */
    wxString path;

    /** Panel associated with this file entry in the UI list */
    wxPanel *panel;
};

/**
 * @brief Set of directory names ignored during project file indexing.
 *
 * These directories are skipped entirely during recursive filesystem traversal
 * in order to avoid unnecessary IO, improve performance, and prevent indexing
 * of generated or external dependency files.
 *
 * This list mirrors common editor exclusions (e.g. VS Code).
 */
static const std::unordered_set<std::string> kIgnoredDirs = {
    ".git",
    ".svn",
    ".hg",
    "build",
    "dist",
    "out",
    "node_modules",
    ".vscode",
    ".idea",
    "__pycache__",
    "cmake-build-debug",
    "cmake-build-release"};

/**
 * @class QuickOpen
 * @brief Keyboard-driven file navigation panel (Ctrl+P).
 *
 * QuickOpen provides a fast, searchable interface for locating and opening
 * files within the current project, inspired by Visual Studio Code's
 * "Quick Open" feature.
 *
 * Core responsibilities:
 * - Display a searchable list of project files
 * - Handle keyboard navigation and selection
 * - Integrate with the editor theming system
 * - Manage focus and lifecycle independently
 *
 * Design notes:
 * - Files are indexed once and filtered in-memory
 * - Navigation is optimized for keyboard-first usage
 * - The panel is ephemeral and destroyed on close
 */
class QuickOpen : public wxPanel
{
public:
    /**
     * @brief Constructs the Quick Open panel.
     *
     * Initializes all UI elements and internal state.
     * Focus handling and accelerator setup are performed explicitly
     * after creation.
     *
     * @param parent Pointer to the parent wxFrame (typically the main window).
     */
    explicit QuickOpen(wxFrame *parent);

    /**
     * @brief Handles keyboard navigation upwards.
     *
     * Moves the current selection to the previous file entry.
     *
     * @param event Keyboard command event.
     */
    void OnKeyboardUp(wxCommandEvent &event);

    /**
     * @brief Handles keyboard navigation downwards.
     *
     * Moves the current selection to the next file entry.
     *
     * @param event Keyboard command event.
     */
    void OnKeyboardDown(wxCommandEvent &event);

    /**
     * @brief Handles keyboard confirmation (Enter key).
     *
     * Opens the currently selected file and closes the panel.
     *
     * @param event Keyboard command event.
     */
    void OnKeyboardEnter(wxCommandEvent &event);

    /**
     * @brief Handles mouse left-click on a file entry.
     *
     * Updates the current selection and optionally triggers file opening.
     *
     * @param event Mouse event.
     */
    void OnMouseLeftClick(wxMouseEvent &event);

    /**
     * @brief Handles mouse hover over a file entry.
     *
     * Updates visual highlighting to reflect hover state.
     *
     * @param event Mouse event.
     */
    void OnMouseEnter(wxMouseEvent &event);

    /**
     * @brief Handles mouse leaving a file entry.
     *
     * Restores the normal visual state of the entry.
     *
     * @param event Mouse event.
     */
    void OnMouseLeave(wxMouseEvent &event);

    /**
     * @brief Closes and destroys the Quick Open panel.
     *
     * Typically triggered by ESC key or after a successful file selection.
     *
     * @param event Command event (unused).
     */
    void Close(wxCommandEvent &WXUNUSED(event));

private:
    /** Root vertical layout sizer */
    wxBoxSizer *m_sizer = new wxBoxSizer(wxVERTICAL);

    /** Layout sizer for the scrollable list of file entries */
    wxBoxSizer *scrolledContainerSizer = new wxBoxSizer(wxVERTICAL);

    /** Layout sizer for the top search bar container */
    wxBoxSizer *topContainerSizer = new wxBoxSizer(wxVERTICAL);

    /** Panel containing the search input */
    wxPanel *m_topContainer = nullptr;

    /** Scrollable container holding file entry panels */
    wxScrolled<wxPanel> *scrolledContainer = nullptr;

    /** Text input used to filter indexed files */
    wxTextCtrl *m_searchBar = nullptr;

    /** Centralized theme manager instance */
    ThemesManager Theme = ThemesManager::Get();

    /** Full path of the currently selected file */
    std::string m_selectedFilePath;

    /** Cached list of indexed project files */
    std::vector<QuickOpenFileStruct> m_files;

    /**
     * @brief Configures keyboard accelerators and shortcuts.
     *
     * Handles ESC, Enter, Up, and Down keys regardless of focus.
     */
    void SetAccelerators();

    /**
     * @brief Creates and configures the search bar control.
     *
     * The search bar drives real-time filtering of file entries.
     */
    void CreateSearchBar();

    /**
     * @brief Creates the scrollable container for file entries.
     */
    void CreateScrolledContainer();

    /**
     * @brief Handles text changes in the search bar.
     *
     * Filters the displayed file list based on the current input.
     *
     * @param event Text change event.
     */
    void OnSearchBarChange(wxCommandEvent &event);

    /**
     * @brief Intercepts key events before child controls.
     *
     * Used to capture global keys such as ESC even when the search
     * bar has focus.
     *
     * @param event Key event.
     */
    void OnTextCharHook(wxKeyEvent &event);

    /**
     * @brief Creates a visual UI entry for a file.
     *
     * @param file Metadata describing the file to display.
     */
    void CreateFileContainer(QuickOpenFileStruct file);

    /**
     * @brief Updates the currently selected file entry.
     *
     * @param direction Navigation direction ("up" or "down").
     */
    void ChangeCurrentSelectedFile(const std::string &direction);

    /** Disables copy construction and assignment */
    wxDECLARE_NO_COPY_CLASS(QuickOpen);

    /** wxWidgets event table declaration */
    wxDECLARE_EVENT_TABLE();
};