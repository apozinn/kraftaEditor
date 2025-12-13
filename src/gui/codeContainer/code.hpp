#pragma once

/**
 * @file code.hpp
 * @brief Declaration of the CodeContainer class, which hosts the main editor and minimap.
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
 * @brief A scrollable panel designed to hold a code editor (Editor) and its corresponding minimap (MiniMap).
 *
 * It manages file loading, saving, and keyboard shortcuts related to code editing and file operations.
 */
class CodeContainer : public wxPanel
{
public:
    /**
     * @brief Constructs the CodeContainer.
     * @param parent The parent window.
     * @param path The file path to load into the editor.
     */
    CodeContainer(wxWindow *parent, wxString path);

    /**
     * @brief Loads the content of the specified file path into the editor and minimap.
     * @param path The file path.
     */
    void LoadPath(wxString path);

    // file
    /**
     * @brief Saves the content of the editor to the specified path.
     * @param path The file path to save to.
     * @return **true** if the save operation was successful; **false** otherwise.
     */
    bool Save(wxString path);
    void OnSave(wxCommandEvent &WXUNUSED(event));
    void OnSaveAs(wxCommandEvent &WXUNUSED(event));
    void OnSaveAll(wxCommandEvent &WXUNUSED(event));
    void OnCloseFile(wxCommandEvent &WXUNUSED(event));

    // edit
    void OnRedo(wxCommandEvent &WXUNUSED(event));
    void OnUndo(wxCommandEvent &WXUNUSED(event));
    void OnCut(wxCommandEvent &WXUNUSED(event));
    void OnCopy(wxCommandEvent &WXUNUSED(event));
    void OnPaste(wxCommandEvent &WXUNUSED(event));

    // comment
    void ToggleCommentLine(wxCommandEvent &WXUNUSED(event));
    void ToggleCommentBlock(wxCommandEvent &WXUNUSED(event));

    // select
    void OnSelectAll(wxCommandEvent &WXUNUSED(event));
    void OnSelectLine(wxCommandEvent &WXUNUSED(event));

    // view
    void OnToggleMinimapView(wxCommandEvent &WXUNUSED(event));

public:
    wxString iconsDir = ApplicationPaths::AssetsPath("icons"); /**< Path to the icon directory. */
    wxFont font; /**< Font used for the editor. */
    Editor *editor; /**< The main code editor component. */
    MiniMap *minimap; /**< The code minimap component. */
    wxString currentPath; /**< The file path currently loaded. */
    bool codeMapMouseOver = false; /**< Tracks mouse state over the minimap (unused). */
    languagePreferencesStruct languagePreferences; /**< Structure holding language-specific preferences. */
    wxPoint codeMapClickPoint = wxPoint(0, 0); /**< Last minimap click point (unused). */
    StatusBar *statusBar = ((StatusBar *)FindWindowById(+GUI::ControlID::StatusBar)); /**< Pointer to the status bar. */
    json UserSettings = UserSettingsManager::Get().currentSettings; /**< Cached user settings. */
    ProjectSettings &projectSettings = ProjectSettings::Get(); /**< Reference to global project settings. */

private:
    wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL); /**< Sizer to manage editor and minimap layout. */
};