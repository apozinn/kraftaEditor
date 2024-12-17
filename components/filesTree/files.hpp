#pragma once
#include "../../utils/fileManager.hpp"
#include <wx/fswatcher.h>

class FilesTree : public wxPanel {
	wxPanel* files_tree;
	wxSizer* sizer;
	wxBoxSizer* file_ctn_sizer;
	json Themes = UserConfig().GetThemes();
	wxScrolled<wxPanel>* files_container;
public:
	wxScrolled<wxPanel>* project_files_ctn;
	wxWindow* selectedFile = NULL;
	FileManager* fileManager = new FileManager();
	wxString menufile_path, menudir_path;
	FilesTree(wxWindow* parent, wxWindowID ID);
	void Update();
	bool Load(wxWindow* parent, std::string path);
	void CreateFile(wxWindow* parent, wxString name, wxString path);
	void CreateDir(wxWindow* parent, wxString name, wxString path);
	void OnFileSelect(wxMouseEvent& event);
	void ToggleDir(wxMouseEvent& event);
	void OpenFile(wxString path);
	void onTopMenuClick(wxMouseEvent& event);
	void onFileRightClick(wxMouseEvent& event);
	void onDirRightClick(wxMouseEvent& event);
	void SetSelectedFile(wxWindow* new_window) { selectedFile = new_window; }
	void FitContainer(wxWindow* window);
	void OnPaint(wxPaintEvent& event);
	void OnCreateDir(wxCommandEvent& event);
	void OnCreateFile(wxCommandEvent& event);
	void OnDeleteDir(wxCommandEvent& event);
	void OnDeleteFile(wxCommandEvent& event);
	void OnFileRename(wxCommandEvent& event);
	void OnDirRename(wxCommandEvent& event);
	void OnTreeModifyed(wxString old_path, wxString new_path);
private:
	void OnEnterComp(wxMouseEvent& event);
	void OnLeaveComp(wxMouseEvent& event);
	wxDECLARE_NO_COPY_CLASS(FilesTree);
	wxDECLARE_EVENT_TABLE();
};

wxBEGIN_EVENT_TABLE(FilesTree, wxPanel)
EVT_MENU(ID_CREATE_DIR, FilesTree::OnCreateDir)
EVT_MENU(ID_CREATE_FILE, FilesTree::OnCreateFile)
EVT_MENU(ID_DELETE_DIR, FilesTree::OnDeleteDir)
EVT_MENU(ID_DELETE_FILE, FilesTree::OnDeleteFile)
EVT_MENU(ID_RENAME_FILE, FilesTree::OnFileRename)
EVT_MENU(ID_RENAME_DIR, FilesTree::OnDirRename)
wxEND_EVENT_TABLE()