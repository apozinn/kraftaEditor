#pragma once

#include <wx/stc/stc.h>

class MiniMap : public wxStyledTextCtrl
{
public:
    MiniMap(wxWindow *parent);

private:
    wxStyledTextCtrl *Editor;
    wxString currentPath;
    int clickPoint = 0;
    bool mouseOver = false;
    StatusBar *statusBar = ((StatusBar *)FindWindowById(ID_STATUS_BAR));
    void InitializePrefs();
    void OnClick(wxMouseEvent &event);
	void OnPaint(wxPaintEvent &event);
	void OnPainted(wxStyledTextEvent &event);
    void OnMouseEnter(wxMouseEvent &event);
	void OnMouseLeave(wxMouseEvent &event);
};