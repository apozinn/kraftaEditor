#include "mainFrame.hpp"

bool MainFrame::CreateWatcherIfNecessary() {
	try {
		if (m_watcher) {
			return false;
		}

		CreateWatcher();
		if (wxTheApp->IsActive()) {
			Bind(wxEVT_FSWATCHER, &MainFrame::OnFileSystemEvent, this);
		}
	} catch (const std::exception &e) {
		std::cerr << e.what() << std::endl;
	}
	return true;
}

void MainFrame::CreateWatcher() {
	if (!wxTheApp || !wxTheApp->IsActive() || IsBeingDeleted()) {
		wxMessageBox(_("Cannot create watcher - application not active or "
					   "being destroyed"));
		return;
	}

	if (m_watcher) {
		wxMessageBox(_("File watcher already initialized"));
		return;
	}

	m_watcher = new wxFileSystemWatcher();
	if (!m_watcher) {
		wxMessageBox(_(
			"Failed to create file system watcher - memory allocation error"));
		return;
	}

	m_watcher->SetOwner(this);
}

void MainFrame::OnWatch(wxCommandEvent &event) {
	if (event.IsChecked()) {
		wxCHECK_RET(!m_watcher, "Watcher already initialized");
		CreateWatcher();
	} else {
		wxCHECK_RET(m_watcher, "Watcher not initialized");
		wxDELETE(m_watcher);
	}
}

void MainFrame::CloseAllFiles(wxCommandEvent &WXUNUSED(event)) {
	m_tabs->CloseAllFiles();
}

void MainFrame::OnCloseFolder(wxCommandEvent &WXUNUSED(event)) {
	if (!m_filesTree || !m_mainContainer || !m_tabs) {
		wxMessageBox(_("Required UI components not initialized"));
		return;
	}

	m_filesTree->CloseProject();
	m_tabs->CloseAllFiles();

	wxConfig *config = new wxConfig("krafta-editor");
	config->Write("workspace", "");
	delete config;

	ProjectSettings::Get().ClearProject();

	new OpenFolderButton();
}