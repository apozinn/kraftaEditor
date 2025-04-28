#pragma once

#include "./editor/editor.cpp"
#include "./minimap/minimap.cpp"

#include "../../defs.hpp"
#include "../statusBar/status.hpp"
#include "../../src/languagePrefs/prefs.cpp"
#include "../../src/lexerStyle/lexerStyle.cpp"

#include <wx/stc/stc.h>
#include <wx/scrolwin.h>

class CodeContainer : public wxScrolled<wxPanel>
{
public:
	wxFont font;
	Editor *editor;
	MiniMap *minimap;
	wxString currentPath;
	bool codeMapMouseOver = false;
	LanguageInfo const *currentLanguage;
	wxPoint codeMapClickPoint = wxPoint(0, 0);
	StatusBar *statusBar = ((StatusBar *)FindWindowById(ID_STATUS_BAR));

	CodeContainer(wxWindow *parent, wxString path);
	void LoadPath(wxString path);
	void OnSave(wxCommandEvent &event);
	void OnSaveAs(wxCommandEvent &event);
	void ToggleMiniMapView(wxCommandEvent &event);
	void ToggleCommentLine(wxCommandEvent &event);
private:
	wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
	LanguageInfo const *GetFilelanguage(wxString filename);
	void InitializeLanguagePrefs();
	// void DrawBorder(wxStyledTextEvent &event);
	wxDECLARE_EVENT_TABLE();
};

wxBEGIN_EVENT_TABLE(CodeContainer, wxPanel)
	EVT_MENU(wxID_SAVE, CodeContainer::OnSave)
	wxEND_EVENT_TABLE()