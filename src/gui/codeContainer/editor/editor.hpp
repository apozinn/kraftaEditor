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

    enum
    {
        STYLE_DEFAULT = 0, 
        STYLE_KEYWORD = 19 
    };

public:
    Editor(wxWindow *parent);
    bool Modified();
    void OnStyleNeeded(wxStyledTextEvent &WXUNUSED(event));
    void HighlightSyntax(size_t fromPos, size_t toPos, const wxString &text);
    bool IsWordBoundary(const wxString &text, size_t pos) const;
    void ApplySyntaxHighlighting(size_t fromPos, size_t toPos, const std::vector<std::pair<size_t, size_t>> &blocks);
    void ApplyFoldLevels(size_t startPos, int initialLevel, size_t textLength, const std::vector<std::pair<size_t, int>> &changes);
    void UpdateFoldLevels(size_t fromPos, int initialFoldLevel, const wxString &text);

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