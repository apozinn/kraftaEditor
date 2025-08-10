#pragma once

#include "fileOperations/fileOperations.hpp"
#include "themesManager/themesManager.hpp"
#include "appPaths/appPaths.hpp"
#include "appConstants/appConstants.hpp"
#include "ui/ids.hpp"
#include "projectSettings/projectSettings.hpp"
#include "userSettings/userSettings.hpp"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <wx/fswatcher.h>
#include <wx/wx.h>

class FilesTree : public wxPanel
{
	wxSizer *sizer = new wxBoxSizer(wxVERTICAL);
	json Theme = ThemesManager::Get().currentTheme;
	const wxString &iconsDir = ApplicationPaths::AssetsPath("icons");
	ProjectSettings &projectSettings = ProjectSettings::Get();
	json UserSettings = UserSettingsManager::Get().currentSettings;

public:
	wxPanel *projectToggler = nullptr;
	wxScrolled<wxPanel> *projectFilesContainer;
	wxStaticText *projectToolsName = nullptr;
	wxStaticBitmap *pjt_arrow = nullptr;
	wxWindow *selectedFile = nullptr;
	FilesTree(wxWindow *parent, wxWindowID ID);
	void Load(wxWindow *parent, std::string path);
	void CreateFile(wxWindow *parent, wxString name, wxString path);
	void CreateDir(wxWindow *parent, wxString name, wxString path, int pos = -1);
	void OnFileSelect(wxMouseEvent &event);
	void ToggleDir(wxMouseEvent &event);
	void OpenFile(wxString path);
	void onTopMenuClick(wxMouseEvent &WXUNUSED(event));
	void onFileRightClick(wxMouseEvent &event);
	void onDirRightClick(wxMouseEvent &event);
	void SetSelectedFile(wxWindow *target);
	void FitContainer(wxWindow *window);
	void OnPaint(wxPaintEvent &event);
	void OnCreateDir(wxCommandEvent &WXUNUSED(event));
	void OnCreateFile(wxCommandEvent &WXUNUSED(event));
	void OnDeleteDir(wxCommandEvent &WXUNUSED(event));
	void OnDeleteFile(wxCommandEvent &WXUNUSED(event));
	void OnFileRename(wxCommandEvent &WXUNUSED(event));
	void OnDirRename(wxCommandEvent &WXUNUSED(event));
	void OnComponentModified(int type, wxString old_path, wxString new_path);

private:
	void OnEnterComp(wxMouseEvent &event);
	void OnLeaveComp(wxMouseEvent &event);
	wxDECLARE_NO_COPY_CLASS(FilesTree);
	wxDECLARE_EVENT_TABLE();
};