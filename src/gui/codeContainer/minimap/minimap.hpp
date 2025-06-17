#pragma once

#include "ui/ids.hpp"
#include "gui/widgets/statusBar/statusBar.hpp"

#include "themesManager/themesManager.hpp"
#include "appPaths/appPaths.hpp"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <wx/stc/stc.h>

class MiniMap : public wxStyledTextCtrl
{
    json Theme = ThemesManager::Get().currentTheme;
public:
    MiniMap(wxWindow *parent);
private:
    wxStyledTextCtrl *Editor;
    wxString currentPath;
    int clickPoint = 0;
    bool mouseOver = false;
    StatusBar *statusBar = ((StatusBar *)FindWindowById(+GUI::ControlID::StatusBar));
    void InitializePrefs();
    void OnClick(wxMouseEvent &WXUNUSED(event));
	void OnPaint(wxPaintEvent &WXUNUSED(event));
	void OnPainted(wxStyledTextEvent &WXUNUSED(event));
    void OnMouseEnter(wxMouseEvent &WXUNUSED(event));
	void OnMouseLeave(wxMouseEvent &WXUNUSED(event));
};