#include "mainFrame.hpp"

void MainFrame::OnOpenTerminal(wxCommandEvent &WXUNUSED(event)) {
	if (m_mainContainerSplitter->IsSplit())
		m_mainContainerSplitter->Unsplit(m_terminal);
	else
		m_mainContainerSplitter->SplitHorizontally(m_centeredContent,
												   m_terminal, 0);
}

void MainFrame::OnToggleSearch(wxCommandEvent &WXUNUSED(event)) {
	if (FindWindowById(+GUI::ControlID::CodeSearchPanel)) {
		((wxWindow *)FindWindowById(+GUI::ControlID::CodeSearchPanel))
			->Destroy();
	} else {
		wxString defaultLabel = "";
		auto currentEditor = ((wxStyledTextCtrl *)wxFindWindowByLabel(
			ProjectSettings::Get().GetCurrentlyFileOpen() + "_codeEditor"));
		if (currentEditor)
			defaultLabel = currentEditor->GetSelectedText();

		new Search(this, defaultLabel, currentEditor);
	}
}

void MainFrame::OnToggleControlPanel(wxCommandEvent &WXUNUSED(event)) {
	if (FindWindowById(+GUI::ControlID::ControlPanel)) {
		m_controlPanel->Destroy();
		m_controlPanel = nullptr;
	} else {
		m_controlPanel = new ControlPanel(this, +GUI::ControlID::ControlPanel);
	}
}

void MainFrame::OnToggleQuickOpen(wxCommandEvent &WXUNUSED(event)) {
	if (FindWindowById(+GUI::ControlID::QuickOpen)) {
		m_quickOpen->Destroy();
		m_quickOpen = nullptr;
	} else {
		m_quickOpen = new QuickOpen(this);
	}
}

void MainFrame::OnToggleFileTreeView(wxCommandEvent &WXUNUSED(event)) {
	if (!m_mainSplitter || !m_applicationLeftMainContainer ||
		!m_applicationRightMainContainer) {
		wxMessageBox(_("Main splitter or containers not initialized"));
		return;
	}

	if (m_mainSplitter->IsSplit())
		m_mainSplitter->Unsplit(m_applicationLeftMainContainer);
	else
		m_mainSplitter->SplitVertically(m_applicationLeftMainContainer,
										m_applicationRightMainContainer, 1);
}

void MainFrame::OnToggleMenuBarView(wxCommandEvent &WXUNUSED(event)) {
	if (GetMenuBar()) {
		SetMenuBar(nullptr);
		UserSettingsManager::Get().SetSetting<bool>("view/showMenuBar", false);
	} else {
		SetMenuBar(m_menuBar);
		UserSettingsManager::Get().SetSetting<bool>("view/showMenuBar", true);
	}
}

void MainFrame::OnToggleStatusBarView(wxCommandEvent &WXUNUSED(event)) {
	if (!m_applicationRightMainContainer || !m_statusBar) {
		wxMessageBox(_("Status bar or right main container not initialized"));
		return;
	}

	if (m_statusBar->IsShown()) {
		m_statusBar->Hide();
		UserSettingsManager::Get().SetSetting<bool>("view/showStatusBar",
													false);
	} else {
		m_statusBar->Show();
		UserSettingsManager::Get().SetSetting<bool>("view/showStatusBar", true);
	}
	m_applicationRightMainContainer->GetSizer()->Layout();
}

void MainFrame::OnToggleTabBarView(wxCommandEvent &WXUNUSED(event)) {
	if (!m_tabs || !m_mainContainer) {
		wxLogError(_("Tabs or main container not initialized"));
		return;
	}

	if (m_tabs->IsShown())
		m_tabs->Hide();
	else
		m_tabs->Show();

	m_mainContainer->GetSizer()->Layout();
}

void MainFrame::OnToggleMinimapView(wxCommandEvent &WXUNUSED(event)) {
	auto &settingsManager = UserSettingsManager::Get();

	bool currentState =
		settingsManager.GetSetting<bool>("editor/showMinimap").value;
	bool newState = !currentState;

	settingsManager.SetSetting<bool>("editor/showMinimap", newState);

	if (!m_tabs)
		return;

	for (auto &child : m_tabs->tabsContainer->GetChildren()) {
		auto minimap =
			wxWindow::FindWindowByName(child->GetName() + "_codeMap");
		if (minimap) {
			minimap->Show(newState);
			minimap->GetParent()->Layout();
		}
	}
}

void MainFrame::OpenCodeSearch(wxCommandEvent &event) {
	auto *sizer = m_applicationLeftMainContainer->GetSizer();
	for (wxWindow *child : m_applicationLeftMainContainer->GetChildren()) {
		const int id = child->GetId();

		if (id == +GUI::ControlID::PageSwitcher)
			continue;

		child->Show(id == +GUI::ControlID::SearchPage);
	}

	if (sizer)
		sizer->Layout();

	wxCommandEvent evt(EVT_PAGE_SWITCH_SEARCH);
	wxPostEvent(this, evt);
}