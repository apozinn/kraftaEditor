#pragma once

#include "./editor/editor.cpp"
#include "./minimap/minimap.cpp"

#include "../../defs.hpp"
#include "../../src/languagePrefs/prefs.cpp"
#include "../../src/lexerStyle/lexerStyle.cpp"

#include <wx/stc/stc.h>
#include <wx/scrolwin.h>

#include "../statusBar/status.hpp"
#include "../tabs/tabs.hpp"

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

	//file
	bool Save(wxString path);
	void OnSave(wxCommandEvent &WXUNUSED(event));
	void OnSaveAs(wxCommandEvent &WXUNUSED(event));
	void OnSaveAll(wxCommandEvent &WXUNUSED(event));
	void OnCloseFile(wxCommandEvent &WXUNUSED(event));
	
	//edit
	void OnRedo(wxCommandEvent &WXUNUSED(event));
	void OnUndo(wxCommandEvent &WXUNUSED(event));
	void OnClear(wxCommandEvent &WXUNUSED(event));
	void OnCut(wxCommandEvent &WXUNUSED(event));
	void OnCopy(wxCommandEvent &WXUNUSED(event));
	void OnPaste(wxCommandEvent &WXUNUSED(event));
	
	//comment
	void ToggleCommentLine(wxCommandEvent &WXUNUSED(event));
	void ToggleCommentBlock(wxCommandEvent &WXUNUSED(event));
	
	//select
	void OnSelectAll(wxCommandEvent &WXUNUSED(event));
	void OnSelectLine(wxCommandEvent &WXUNUSED(event));
	
	//view
	void ToggleMiniMapView(wxCommandEvent &WXUNUSED(event));
private:
	wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
	LanguageInfo const *GetFilelanguage(wxString filename);
	void InitializeLanguagePrefs();
	wxDECLARE_EVENT_TABLE();
};

wxBEGIN_EVENT_TABLE(CodeContainer, wxPanel)
	EVT_MENU(wxID_SAVE, CodeContainer::OnSave)
wxEND_EVENT_TABLE()