#include "mainFrame.hpp"

void MainFrame::OnFollowLinks(wxCommandEvent &event) {
	m_followLinks = event.IsChecked();
}

void MainFrame::AddEntry(wxFSWPathType type, wxString filename) {
	if (!m_watcher)
		return;
	if (filename.empty())
		return;

	wxCHECK_RET(m_watcher, "Watcher not initialized");
	wxString prefix;

	wxFileName fn = wxFileName::DirName(filename);
	if (!m_followLinks) {
		fn.DontFollowLink();
	}

	switch (type) {
	case wxFSWPath_Dir:
		m_watcher->Add(fn);
		prefix = "Dir:  ";
		break;
	case wxFSWPath_Tree:
		m_watcher->AddTree(fn);
		prefix = "Tree: ";
		break;
	case wxFSWPath_File:
		m_watcher->Add(fn);
		prefix = "File: ";
		break;
	case wxFSWPath_None:
		wxFAIL_MSG("Unexpected path type.");
	}
}

void MainFrame::OnFileSystemEvent(wxFileSystemWatcherEvent &event) {
	m_filesTree->OnFileSystemEvent(event.GetChangeType(),
								   event.GetPath().GetFullPath(),
								   event.GetNewPath().GetFullPath());
}

void MainFrame::OpenFolderDialog() {
	wxDirDialog *dlg =
		new wxDirDialog(NULL, "Choose project directory", "",
						wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
	dlg->ShowModal();
	wxString path = dlg->GetPath();

	if (path.size())
		LoadPath(path);
}

void MainFrame::LoadPath(wxString path) {
	if (!m_tabs || !m_filesTree) {
		wxMessageBox(
			_("Critical UI components (Tabs/FilesTree) not initialized"));
		return;
	}

	wxFileName fn;
	fn.AssignDir(path);
	wxString normalizedPath = fn.GetPath(wxPATH_GET_SEPARATOR);

	if (normalizedPath == ProjectSettings::Get().GetProjectPath())
		return;

	if (normalizedPath.IsEmpty() || !wxDirExists(normalizedPath)) {
		wxMessageBox(wxString::Format(_("Directory '%s' does not exist"),
									  normalizedPath),
					 "Error", wxICON_ERROR);

		m_tabs->CloseAllFiles();
		m_filesTree->CloseProject();

		wxConfig globalConfig("krafta-editor");
		globalConfig.Write("workspace", "");

		new OpenFolderButton();
		return;
	}

	ProjectSettings::Get().ClearProject();
	ProjectSettings::Get().SetProjectPath(normalizedPath);
	ProjectSettings::Get().SetProjectName(
		wxFileNameFromPath(fn.GetFullPath().RemoveLast()));

	std::string workspaceId = WorkspaceStorageManager::ConvertPathToHash(
		normalizedPath.ToStdString());
	WorkspaceStorageManager::Get().Initialize(workspaceId);
	WorkspaceStorageManager::Get().AddToRecents(normalizedPath);

	wxConfig globalConfig("krafta-editor");
	globalConfig.Write("workspace", normalizedPath);

	m_tabs->CloseAllFiles();
	SetTitle("Krafta Editor - " + ProjectSettings::Get().GetProjectName());

	if (GetMenuBar() && m_menuBar->recentsWorkspacesMenu) {
		UpdateRecentWorkspacesMenu(m_menuBar->recentsWorkspacesMenu);
	}

	m_filesTree->LoadProject(m_filesTree->GetProjectFilesContainer(),
							 normalizedPath);
	AddEntry(wxFSWPath_Tree, normalizedPath);

	auto lastFile = WorkspaceStorageManager::Get().GetSetting<std::string>(
		"last_focused_file");
	if (lastFile.found && wxFileExists(lastFile.value)) {
		m_filesTree->OpenFile(wxString(lastFile.value));
	}
}

void MainFrame::OnOpenFile(wxCommandEvent &WXUNUSED(event)) {
	wxFileDialog *dlg =
		new wxFileDialog(NULL, "Choose a file", "", "", "",
						 wxFD_DEFAULT_STYLE | wxFD_FILE_MUST_EXIST);
	dlg->ShowModal();
	wxString path = dlg->GetPath();
	if (path.size()) {
		m_filesTree->OpenFile(path);
		AddEntry(wxFSWPath_File, path);
	}
}

void MainFrame::OnOpenFolderMenu(wxCommandEvent &WXUNUSED(event)) {
	OpenFolderDialog();
}

void MainFrame::OnOpenFolderClick(wxMouseEvent &WXUNUSED(event)) {
	OpenFolderDialog();
}