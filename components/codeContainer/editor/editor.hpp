#pragma once

#include <wx/stc/stc.h>
#include "../../statusBar/status.hpp"

class Editor : public wxStyledTextCtrl
{
public:
    Editor(wxWindow *parent);
    void Save();
private:
    wxStyledTextCtrl *MiniMap = nullptr;
    wxString currentPath;
    StatusBar *statusBar = ((StatusBar *)FindWindowById(ID_STATUS_BAR));
    bool changedFile = false;
    void InitializePrefs();
    void OnChange(wxStyledTextEvent &event);
    void OnMarginClick(wxStyledTextEvent &event);
    void OnArrowsPress(wxKeyEvent &event);
    void CharAdd(wxStyledTextEvent &event);
    void OnClick(wxMouseEvent &event);
    void OnAutoCompCompleted(wxStyledTextEvent &event);
    void OnClipBoardPaste(wxStyledTextEvent &event);
    void SetMiniMapLine();
    void OnScroll(wxMouseEvent &event);
};