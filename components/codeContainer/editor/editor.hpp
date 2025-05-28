#pragma once

#include <wx/stc/stc.h>
#include "../../statusBar/status.hpp"

class Editor : public wxStyledTextCtrl
{
public:
    Editor(wxWindow *parent);
    bool Modified();
    void OnStyleNeeded(wxStyledTextEvent &event);
    void setfoldlevels(size_t fromPos, int startfoldlevel, wxString &text);
    void highlightSTCsyntax(size_t fromPos, size_t toPos, wxString &text);

private:
    wxStyledTextCtrl *MiniMap = nullptr;
    wxColor m_GCodecolor{0, 0, 0};
    int m_stylemask = 255;
    wxString currentPath;
    StatusBar *statusBar = ((StatusBar *)FindWindowById(ID_STATUS_BAR));
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