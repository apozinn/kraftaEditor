#pragma once

#include "ui/ids.hpp"
#include "projectSettings/projectSettings.hpp"
#include "themesManager/themesManager.hpp"
#include "appPaths/appPaths.hpp"

#include "gui/widgets/statusBar/statusBar.hpp"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <wx/stc/stc.h>

class Editor : public wxStyledTextCtrl
{
    json Theme = ThemesManager::Get().currentTheme;
    const wxString &iconsDir = ApplicationPaths::AssetsPath("icons");
    ProjectSettings &projectSettings = ProjectSettings::Get();

public:
    Editor(wxWindow *parent);
    bool Modified();
    void OnStyleNeeded(wxStyledTextEvent &WXUNUSED(event));
    void setfoldlevels(size_t fromPos, int startfoldlevel, wxString &text);
    void highlightSTCsyntax(size_t fromPos, size_t toPos, wxString &text);

private:
    wxStyledTextCtrl *MiniMap = nullptr;
    wxColor m_GCodecolor{0, 0, 0};
    int m_stylemask = 255;
    wxString currentPath;
    StatusBar *statusBar = ((StatusBar *)FindWindowById(+GUI::ControlID::StatusBar));
    bool changedFile = false;
    void InitializePrefs();
    void SetFoldPreferences();
    void OnChange(wxStyledTextEvent &event);
    void OnMarginClick(wxStyledTextEvent &event);
    void OnBackspace(wxKeyEvent &event);
    void OnArrowsPress(wxKeyEvent &event);
    void CharAdd(wxStyledTextEvent &event);
    void OnClick(wxMouseEvent &event);
    void OnAutoCompCompleted(wxStyledTextEvent &event);
    void OnClipBoardPaste(wxStyledTextEvent &event);
    void SetMiniMapLine();
    void OnScroll(wxMouseEvent &event);
};