#pragma once

#include "ui/ids.hpp"
#include "userSettings/userSettings.hpp"
#include "appPaths/appPaths.hpp"
#include "projectSettings/projectSettings.hpp"

#include "./editor/editor.hpp"
#include "./minimap/minimap.hpp"

#include "gui/panels/tabs/tabs.hpp"
#include "gui/widgets/statusBar/statusBar.hpp"

#include <wx/stc/stc.h>
#include <wx/scrolwin.h>

class CodeContainer : public wxScrolled<wxPanel>
{
public:
	wxString iconsDir = ApplicationPaths::AssetsPath("icons");
	wxFont font;
	Editor *editor;
	MiniMap *minimap;
	wxString currentPath;
	bool codeMapMouseOver = false;
	languagePreferencesStruct languagePreferences;
	wxPoint codeMapClickPoint = wxPoint(0, 0);
	StatusBar *statusBar = ((StatusBar *)FindWindowById(+GUI::ControlID::StatusBar));
	json UserSettings = UserSettingsManager::Get().currentSettings;
	ProjectSettings &projectSettings = ProjectSettings::Get();

	CodeContainer(wxWindow *parent, wxString path);
	void LoadPath(wxString path);

	// file
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

private:
	wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
};