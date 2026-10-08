#pragma once

#include "gui/editor/features/autocomplete/autocomplete.hpp"
#include "gui/editor/features/lsp/lspController.hpp"
#include "gui/editor/core/editor.hpp"

#include "ui/ids.hpp"
#include "projectSettings/projectSettings.hpp"
#include "themesManager/themesManager.hpp"
#include "appPaths/appPaths.hpp"
#include "gui/widgets/statusBar/statusBar.hpp"
#include "languagesPreferences/languagesPreferences.hpp"
#include "shortcutsSettings/shortcutsSettings.hpp"
#include "userSettings/userSettings.hpp"
#include "lsp/lspClient/lspClient.hpp"
#include "appConstants/appConstants.hpp"
#include "lsp/lspClient/lspClient.hpp"
#include "lsp/lspManager/lspManager.hpp"

#include <wx/stc/minimap.h>
#include <wx/stc/stc.h>
#include <wx/timer.h>
#include <wx/tokenzr.h>
#include <nlohmann/json.hpp>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <atomic>

#include <algorithm>
#include <cctype>
#include <sstream>
#include <unordered_set>

class Editor;
class LspClient;
static wxCriticalSection g_editorLock;
using json = nlohmann::json;

namespace
{
    static const std::unordered_map<wxString, wxString> kPairMap = {
        {"\"", "\""}, {"'", "'"}, {"]", "["}, {"}", "{"}, {")", "("}};
}

namespace EditorConstants
{
    constexpr int LINE_NUMBER_MARGIN = 0;
    constexpr int FOLD_MARGIN = 2;
    constexpr int FOLD_MARGIN_WIDTH = 20;
    constexpr int INDICATOR_DEFAULT = 0;
    constexpr int MAX_INDICATOR = 7;
    constexpr int MIN_SELECTION_LENGTH = 2;
}

class TextCtrl : public wxStyledTextCtrl
{
    enum
    {
        STYLE_DEFAULT = 0,
        STYLE_KEYWORD = 19
    };

public:
static TextCtrl* GetFocusedTextCtrl();
    TextCtrl(wxWindow *parent);
    ~TextCtrl();

    bool Modified() const;
    void BindEvents();
    void SetLanguagePreferences(languagePreferencesStruct languagePreferences);
    void SetupAutocomplete();
    void SetupLsp();
    void RecreateMinimap();
    void OnSave(wxCommandEvent& e);
    void DoSave(const wxString &path);
    void SaveAs(wxCommandEvent& e);
    void SaveAll(wxCommandEvent& e);
    void CloseFile(wxCommandEvent& e);    
    void DoCopy(wxCommandEvent& e);
    void DoCut(wxCommandEvent& e);
    void DoPaste(wxCommandEvent& e);
    void DoRedo(wxCommandEvent& e);
    void DoUndo(wxCommandEvent& e);
    void SelectLine(wxCommandEvent& e);
    void DoSelectAll(wxCommandEvent& e);
    void DoZoomIn(wxCommandEvent& e);
    void DoZoomOut(wxCommandEvent& e);
    void ToggleLineComment(wxCommandEvent& e);
    void ToggleBlockComment(wxCommandEvent& event);
    void DuplicateLine(wxCommandEvent& e);
    void MoveLineUp(wxCommandEvent& e);
    void MoveLineDown(wxCommandEvent& e);
    void MoveCursorUp(wxCommandEvent& e);
    void MoveCursorDown(wxCommandEvent& e);
    void RemoveCurrentLine(wxCommandEvent& e);
    void OnZoomIn(wxCommandEvent &event);
    void OnZoomOut(wxCommandEvent &event);
    void SelectNextOccurrence(wxCommandEvent &event);
    
    bool m_isDestroyed = false;
    bool changedFile = false;

private:
    
    void SetupAcceleratorTable();
    void SetupStyles();
    void InitializePreferences();
    void ConfigureFoldMargin();
    
    void OnChange(wxStyledTextEvent &event);
    void OnMarginClick(wxStyledTextEvent &event);
    void OnBackspace(wxKeyEvent &event);
    void OnArrowsPress(wxKeyEvent &event);
    void CharAdd(wxStyledTextEvent &event);
    void OnEnterKey(wxStyledTextEvent &event);
    void OnClick(wxMouseEvent &event);
    void OnScroll(wxMouseEvent &event);

    void HighlightSelectionOccurrences();
    void ClearIndicators();
    void UpdateUnsavedIndicator();
    void HandleAutoPairing(char chr);
    void OnHorizontalScroll(wxMouseEvent &event);

    ProjectSettings &projectSettings = ProjectSettings::Get();
    wxStyledTextCtrlMiniMap *minimap = nullptr;
    wxString currentPath;
    languagePreferencesStruct m_languagePreferences;
    AutoCompleteController* m_autocompleteController = nullptr; 
    LspController* m_lspController = nullptr;
    StatusBar *statusBar = ((StatusBar *)FindWindowById(+GUI::ControlID::StatusBar));
    wxDECLARE_NO_COPY_CLASS(TextCtrl);
};