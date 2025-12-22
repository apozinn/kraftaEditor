#pragma once

/**
 * @file code.hpp
 * @brief Declaration of the CodeContainer class.
 *
 * The CodeContainer is responsible for hosting the main code editor and
 * its auxiliary UI components such as the minimap and status bar integration.
 */

#include "ui/ids.hpp"
#include "userSettings/userSettings.hpp"
#include "appPaths/appPaths.hpp"
#include "projectSettings/projectSettings.hpp"

#include "./editor/editor.hpp"
#include "./minimap/minimap.hpp"

#include "gui/panels/tabs/tabs.hpp"
#include "gui/widgets/statusBar/statusBar.hpp"
#include "languagesPreferences/languagesPreferences.hpp"

#include <wx/stc/stc.h>
#include <wx/scrolwin.h>

/**
 * @class CodeContainer
 * @brief Main container panel for code editing.
 *
 * This class encapsulates the core editing experience by combining:
 * - The main code editor
 * - The minimap view
 * - File I/O operations
 * - Editing commands and shortcuts
 *
 * It acts as the primary bridge between UI actions and editor behavior.
 */
class CodeContainer : public wxPanel
{
public:
    /**
     * @brief Constructs a new CodeContainer.
     *
     * @param parent Parent window.
     * @param path Initial file path to load into the editor.
     */
    CodeContainer(wxWindow *parent, wxString path);

    /**
     * @brief Loads a file into the editor and minimap.
     *
     * This replaces the current editor content with the contents of the file
     * located at the given path.
     *
     * @param path File path to load.
     */
    void LoadPath(wxString path);

    // -------------------------------------------------------------------------
    // File operations
    // -------------------------------------------------------------------------

    /**
     * @brief Saves the current editor content to a file.
     *
     * @param path Destination file path.
     * @return true if the file was saved successfully.
     * @return false if the save operation failed.
     */
    bool Save(wxString path);

    /** @brief Handles the "Save" action. */
    void OnSave(wxCommandEvent &WXUNUSED(event));

    /** @brief Handles the "Save As" action. */
    void OnSaveAs(wxCommandEvent &WXUNUSED(event));

    /** @brief Handles the "Save All" action. */
    void OnSaveAll(wxCommandEvent &WXUNUSED(event));

    /** @brief Handles the "Close File" action. */
    void OnCloseFile(wxCommandEvent &WXUNUSED(event));

    // -------------------------------------------------------------------------
    // Edit operations
    // -------------------------------------------------------------------------

    /** @brief Redoes the last undone action. */
    void OnRedo(wxCommandEvent &WXUNUSED(event));

    /** @brief Undoes the last editor action. */
    void OnUndo(wxCommandEvent &WXUNUSED(event));

    /** @brief Cuts the current selection to the clipboard. */
    void OnCut(wxCommandEvent &WXUNUSED(event));

    /** @brief Copies the current selection to the clipboard. */
    void OnCopy(wxCommandEvent &WXUNUSED(event));

    /** @brief Pastes clipboard content into the editor. */
    void OnPaste(wxCommandEvent &WXUNUSED(event));

    // -------------------------------------------------------------------------
    // Comment operations
    // -------------------------------------------------------------------------

    /** @brief Toggles line comments on the current selection or line. */
    void ToggleCommentLine(wxCommandEvent &WXUNUSED(event));

    /** @brief Toggles block comments on the current selection. */
    void ToggleCommentBlock(wxCommandEvent &WXUNUSED(event));

    // -------------------------------------------------------------------------
    // Selection operations
    // -------------------------------------------------------------------------

    /** @brief Selects all text in the editor. */
    void OnSelectAll(wxCommandEvent &WXUNUSED(event));

    /** @brief Selects the current line. */
    void OnSelectLine(wxCommandEvent &WXUNUSED(event));

    /** @brief Moves the current or selected lines up. */
    void OnMoveLineUp(wxCommandEvent &WXUNUSED(event));

    /** @brief Moves the current or selected lines down. */
    void OnMoveLineDown(wxCommandEvent &WXUNUSED(event));

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
    void OnRemoveCurrentLine(wxCommandEvent &WXUNUSED(event));

    wxString currentPath; /**< Currently opened file path. */
    Editor *editor;       /**< Main code editor instance. */
private:
    wxString iconsDir = ApplicationPaths::AssetsPath("icons");                        /**< Directory containing editor icons. */
    wxFont font;                                                                      /**< Editor font. */
    MiniMap *minimap;                                                                 /**< Minimap instance. */
    bool codeMapMouseOver = false;                                                    /**< Indicates if the mouse is over the minimap. */
    languagePreferencesStruct languagePreferences;                                    /**< Language-specific editor preferences. */
    wxPoint codeMapClickPoint = wxPoint(0, 0);                                        /**< Last minimap click position. */
    StatusBar *statusBar = ((StatusBar *)FindWindowById(+GUI::ControlID::StatusBar)); /**< Pointer to the global status bar. */
    json UserSettings = UserSettingsManager::Get().currentSettings;                   /**< Cached user settings snapshot. */
    ProjectSettings &projectSettings = ProjectSettings::Get();                        /**< Reference to global project settings. */
    wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);                                 /**< Layout sizer for editor and minimap. */
};
