#pragma once

#include <wx/wx.h>
#include <wx/scrolwin.h>

#include "ui/ids.hpp"
#include "themesManager/themesManager.hpp"

/**
 * @struct QuickOpenFileStruct
 * @brief Represents a file entry displayed in the Quick Open panel.
 *
 * This structure stores basic metadata about a file that can be selected
 * through the Quick Open interface.
 */
struct QuickOpenFileStruct
{
    /** Display name of the file */
    std::string name;

    /** Absolute or relative filesystem path to the file */
    std::string path;
};

/**
 * @class QuickOpen
 * @brief Quick Open panel similar to VS Code's "Quick Open" feature.
 *
 * This panel provides a searchable list of files within the current project,
 * allowing fast keyboard-driven navigation and selection.
 *
 * Features:
 * - Text-based filtering using a search input
 * - Keyboard navigation (Up / Down / Enter)
 * - Mouse interaction support
 * - Fully themed via ThemesManager
 * - Non-copyable wxWidgets component
 *
 * The panel is intended to be created dynamically and destroyed when closed.
 */
class QuickOpen : public wxPanel
{
public:
    /**
     * @brief Constructs the Quick Open panel.
     *
     * Creates all internal UI elements but does not assume focus handling.
     * Final initialization (focus, accelerators) should occur after the panel
     * is shown.
     *
     * @param parent Pointer to the parent wxFrame (usually the main window).
     */
    explicit QuickOpen(wxFrame *parent);

    /**
     * @brief Handles keyboard "Up" navigation.
     *
     * Moves the current selection to the previous file entry.
     *
     * @param event Keyboard command event.
     */
    void OnKeyboardUp(wxCommandEvent &event);

    /**
     * @brief Handles keyboard "Down" navigation.
     *
     * Moves the current selection to the next file entry.
     *
     * @param event Keyboard command event.
     */
    void OnKeyboardDown(wxCommandEvent &event);

    /**
     * @brief Handles keyboard "Enter" action.
     *
     * Opens the currently selected file.
     *
     * @param event Keyboard command event.
     */
    void OnKeyboardEnter(wxCommandEvent &event);

    /**
     * @brief Handles mouse left-click selection.
     *
     * Selects the file entry under the mouse cursor.
     *
     * @param event Mouse event.
     */
    void OnMouseLeftClick(wxMouseEvent &event);

    /**
     * @brief Handles mouse hover over a file entry.
     *
     * Updates visual feedback when the cursor enters a file entry.
     *
     * @param event Mouse event.
     */
    void OnMouseEnter(wxMouseEvent &event);

    /**
     * @brief Handles mouse leaving a file entry.
     *
     * Restores visual state when the cursor leaves a file entry.
     *
     * @param event Mouse event.
     */
    void OnMouseLeave(wxMouseEvent &event);

    /**
     * @brief Closes and destroys the Quick Open panel.
     *
     * Typically triggered by ESC or selection completion.
     *
     * @param event Command event (unused).
     */
    void Close(wxCommandEvent &WXUNUSED(event));

private:
    /** Main vertical layout sizer */
    wxBoxSizer *m_sizer = new wxBoxSizer(wxVERTICAL);

    /** Layout sizer for the scrollable file list */
    wxBoxSizer *scrolledContainerSizer = new wxBoxSizer(wxVERTICAL);

    /** Layout sizer for the top search bar container */
    wxBoxSizer *topContainerSizer = new wxBoxSizer(wxVERTICAL);

    /** Container holding the search bar */
    wxPanel *m_topContainer = nullptr;

    /** Scrollable container for file entries */
    wxScrolled<wxPanel> *scrolledContainer = nullptr;

    /** Text input used to filter files */
    wxTextCtrl *m_searchBar = nullptr;

    /** Theme manager instance */
    ThemesManager Theme = ThemesManager::Get();

    /** Path of the currently selected file */
    std::string m_selectedFilePath;

    /**
     * @brief Configures keyboard accelerators for the panel.
     *
     * Handles ESC, navigation keys and selection actions.
     */
    void SetAccelerators();

    /**
     * @brief Creates and initializes the search bar.
     *
     * The search bar is used to filter visible file entries.
     */
    void CreateSearchBar();

    /**
     * @brief Creates the scrollable container for file entries.
     */
    void CreateScrolledContainer();

    /**
     * @brief Handles changes in the search input.
     *
     * Filters file entries based on the current search text.
     *
     * @param event Text change event.
     */
    void OnSearchBarChange(wxCommandEvent &event);

    /**
     * @brief Intercepts keyboard input before child controls.
     *
     * Used to capture global keys such as ESC regardless of focus.
     *
     * @param event Key event.
     */
    void OnTextCharHook(wxKeyEvent &event);

    /**
     * @brief Creates a visual entry for a file in the list.
     *
     * @param file File metadata structure.
     */
    void CreateFileContainer(QuickOpenFileStruct file);

    /**
     * @brief Changes the currently selected file entry.
     *
     * Direction is typically "up" or "down".
     *
     * @param direction Navigation direction.
     */
    void ChangeCurrentSelectedFile(const std::string &direction);

    /** Disables copy construction and assignment */
    wxDECLARE_NO_COPY_CLASS(QuickOpen);

    /** wxWidgets event table declaration */
    wxDECLARE_EVENT_TABLE();
};
