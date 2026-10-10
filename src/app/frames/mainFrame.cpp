#include "mainFrame.hpp"

MainFrame::MainFrame(const wxString &title)
	: wxFrame(nullptr, wxID_ANY, title), m_watcher(nullptr),
	  m_followLinks(false) {
	SetTitle(title);
	SetupSettingsTimer();
	WindowResizeFunctions();
	SetThemeEnabled(true);

	SetAppIcon();
	SetupMenuBar();
	SetupMainSplitter();
	SetupApplicationLeftMainContainer();
	SetupSearchPage();
	SetupFilesTree();
	SetupApplicationRightMainContainer();
	SetupMainContainerSplitter();
	SetupCenteredContent();
	SetupCodeContainersBlock();
	SetupMainContainer();
	SetupTabs();
	SetupEmptyWindow();
	SetupTerminal();
	SetupStatusBar();
	SetupAccelerators();

	LspManager::Get();

	SetSizer(sizer);
	SetDropTarget(new FrameFileDropTarget(this));
}

MainFrame::~MainFrame() { delete m_watcher; }

void MainFrame::SetupSettingsTimer() {
	m_saveSettingsTimer = new wxTimer(this, ID_SAVE_SETTINGS_TIMER);
	Bind(wxEVT_TIMER, &MainFrame::OnSaveSettingsTimer, this,
		 ID_SAVE_SETTINGS_TIMER);
}

bool MainFrame::SetAppIcon() {
	wxIcon aplicationIcon;
	aplicationIcon.LoadFile(ApplicationPaths::AssetsPath("images") +
								"kraftaEditor.png",
							wxBITMAP_TYPE_PNG);
	if (aplicationIcon.IsOk()) {
		SetIcon(aplicationIcon);
		return true;
	}

	wxMessageBox(_("Failed to set application icon"));
	return false;
}

void MainFrame::SetupMenuBar() {
	m_menuBar = new MenuBar();
	if (UserSettingsManager::Get().GetSetting<bool>("view/showMenuBar").value)
		SetMenuBar(m_menuBar);

	Bind(wxEVT_MENU, &MainFrame::OnRecentWorkspaceClick, this,
		 ID_OPEN_RECENT_WORKSPACE_BASE, ID_OPEN_RECENT_WORKSPACE_MAX);
}

void MainFrame::SetupMainSplitter() {
	m_mainSplitter = new wxSplitterWindow(this, +GUI::ControlID::MainSplitter);
	m_mainSplitter->SetBackgroundColour(ThemesManager::Get().GetColor("main"));
	wxBoxSizer *mainSplitterSizer = new wxBoxSizer(wxHORIZONTAL);
	m_mainSplitter->SetSizerAndFit(mainSplitterSizer);
	m_mainSplitter->SetMinimumPaneSize(250);

	sizer->Add(m_mainSplitter, 1, wxEXPAND);
	m_mainSplitter->Bind(wxEVT_PAINT, &MainFrame::OnPaintedComponent, this);
}

void MainFrame::SetupApplicationLeftMainContainer() {
	m_applicationLeftMainContainer = new wxPanel(
		m_mainSplitter, +GUI::ControlID::ApplicationLeftMainContainer);
	wxBoxSizer *applicationLeftMainContainer_sizer = new wxBoxSizer(wxVERTICAL);
	m_applicationLeftMainContainer->SetSizer(
		applicationLeftMainContainer_sizer);
	m_mainSplitter->GetSizer()->Add(m_applicationLeftMainContainer, 0);
	m_applicationLeftMainContainer->SetBackgroundColour(
		ThemesManager::Get().GetColor("main"));
}

void MainFrame::SetupKraftaTopLogo() {
	m_kraftaTopLogo = new KraftaTopLogo(m_applicationLeftMainContainer);
	m_applicationLeftMainContainer->GetSizer()->Add(m_kraftaTopLogo, 0,
													wxEXPAND);
}

void MainFrame::SetupPageSwitcher() {
	m_page_switcher = new PageSwitcher(m_applicationLeftMainContainer);
	m_applicationLeftMainContainer->GetSizer()->Add(m_page_switcher, 1,
													wxEXPAND);
}

void MainFrame::SetupSearchPage() {
	m_searchPage = new SearchPage(m_applicationLeftMainContainer);
	m_applicationLeftMainContainer->GetSizer()->Add(m_searchPage, 1, wxEXPAND);
	m_searchPage->Hide();
}

void MainFrame::SetupFilesTree() {
	m_filesTree = new FilesTree(m_applicationLeftMainContainer,
								+GUI::ControlID::FilesTree);
	m_applicationLeftMainContainer->GetSizer()->Add(m_filesTree, 1,
													wxEXPAND | wxLEFT, 3);
}

void MainFrame::SetupApplicationRightMainContainer() {
	m_applicationRightMainContainer = new wxPanel(
		m_mainSplitter, +GUI::ControlID::ApplicationRightMainContainer);
	wxBoxSizer *applicationRightMainContainer_sizer =
		new wxBoxSizer(wxVERTICAL);
	m_applicationRightMainContainer->SetSizer(
		applicationRightMainContainer_sizer);
	m_mainSplitter->GetSizer()->Add(m_applicationRightMainContainer, 1,
									wxEXPAND);
	m_mainSplitter->SplitVertically(m_applicationLeftMainContainer,
									m_applicationRightMainContainer, 1);
}

void MainFrame::SetupMainContainerSplitter() {
	m_mainContainerSplitter =
		new wxSplitterWindow(m_applicationRightMainContainer,
							 +GUI::ControlID::MainContainerSplitter);
	wxBoxSizer *mainContainerSplitterSizer = new wxBoxSizer(wxVERTICAL);
	m_mainContainerSplitter->SetSizer(mainContainerSplitterSizer);
	m_applicationRightMainContainer->GetSizer()->Add(m_mainContainerSplitter, 1,
													 wxEXPAND);
}

void MainFrame::SetupCenteredContent() {
	m_centeredContent =
		new wxPanel(m_mainContainerSplitter, +GUI::ControlID::CenteredContent);
	wxBoxSizer *centeredContentSizer = new wxBoxSizer(wxVERTICAL);
	m_centeredContent->SetSizer(centeredContentSizer);
	m_mainContainerSplitter->GetSizer()->Add(m_centeredContent, 1, wxEXPAND);
}

void MainFrame::SetupCodeContainersBlock() {
	m_codeContainersBlock = new wxSplitterWindow(
		m_centeredContent, +GUI::ControlID::CodeContainersBlock);
	m_codeContainersBlock->SetSashGravity(0.5);

	wxBoxSizer *m_codeContainersBlock_sizer = new wxBoxSizer(wxHORIZONTAL);

	m_codeContainerBlockLeft = new wxPanel(
		m_codeContainersBlock, +GUI::ControlID::CodeContainerBlockLeft);
	wxBoxSizer *m_codeContainerBlockLeft_sizer = new wxBoxSizer(wxVERTICAL);
	m_codeContainerBlockLeft->SetSizer(m_codeContainerBlockLeft_sizer);

	m_codeContainersBlock_sizer->Add(m_codeContainerBlockLeft, 1, wxEXPAND);

	m_codeContainerBlockRight = new wxPanel(
		m_codeContainersBlock, +GUI::ControlID::CodeContainerBlockRight);
	wxBoxSizer *m_codeContainerBlockRight_sizer = new wxBoxSizer(wxVERTICAL);
	m_codeContainerBlockRight->SetSizer(m_codeContainerBlockRight_sizer);

	m_codeContainersBlock_sizer->Add(m_codeContainerBlockRight, 1, wxEXPAND);

	m_codeContainersBlock->SetMinimumPaneSize(100);

	m_codeContainersBlock->SplitVertically(m_codeContainerBlockLeft,
										   m_codeContainerBlockRight);
	m_codeContainersBlock->SetSizer(m_codeContainersBlock_sizer);
	m_centeredContent->GetSizer()->Add(m_codeContainersBlock, 1, wxEXPAND);

	m_codeContainersBlock->Unsplit(m_codeContainerBlockRight);
}

void MainFrame::SetupMainContainer() {
	m_mainContainer =
		new wxPanel(m_codeContainerBlockLeft, +GUI::ControlID::MainCode);
	wxBoxSizer *mainContainerSizer = new wxBoxSizer(wxVERTICAL);
	m_mainContainer->SetSizer(mainContainerSizer);
	m_codeContainerBlockLeft->GetSizer()->Add(m_mainContainer, 1, wxEXPAND);
}

void MainFrame::SetupTabs() {
	m_tabs = new Tabs(m_mainContainer, +GUI::ControlID::Tabs);
	m_mainContainer->GetSizer()->Add(m_tabs, 0, wxEXPAND);
}

void MainFrame::SetupEmptyWindow() {
	m_emptyWindow =
		new EmptyWindow(m_mainContainer, +GUI::ControlID::EmptyWindow);
	m_mainContainer->GetSizer()->Add(m_emptyWindow, 0, wxEXPAND);

	m_emptyWindow->Hide();
}

void MainFrame::SetupTerminal() {
	m_terminal =
		new Terminal(m_mainContainerSplitter, +GUI::ControlID::Terminal);
	m_mainContainerSplitter->GetSizer()->Add(m_terminal, 0);

	m_mainContainerSplitter->SplitHorizontally(m_centeredContent, m_terminal,
											   -200);
	m_mainContainerSplitter->Unsplit(m_terminal);
}

void MainFrame::SetupStatusBar() {
	m_statusBar = new StatusBar(m_applicationRightMainContainer);
	m_applicationRightMainContainer->GetSizer()->Add(m_statusBar, 0, wxEXPAND);
	m_statusBar->Hide();
}

void MainFrame::SetupAccelerators() {
	wxAcceleratorEntry entries[4];

	entries[0].Set(wxACCEL_CTRL, WXK_SHIFT, +Event::View::ToggleMenuBar);
	entries[0].FromString("Ctrl+Shift+M");

	entries[1].Set(wxACCEL_CTRL, WXK_SHIFT, +Event::View::ToggleControlPanel);
	entries[1].FromString("Ctrl+Shift+P");

	entries[2].Set(wxACCEL_CTRL, int('P'), +Event::View::ToggleQuickOpen);
	entries[2].FromString("Ctrl+P");

	entries[3].Set(wxACCEL_CTRL, WXK_CONTROL_F, +Event::View::ToggleCodeSearch);
	entries[3].FromString("Ctrl+F");

	wxAcceleratorTable accel(4, entries);
	SetAcceleratorTable(accel);
}

void MainFrame::OnFrameResized(wxSizeEvent &event) {
	m_saveSettingsTimer->Start(500, wxTIMER_ONE_SHOT);
	event.Skip();
}

void MainFrame::OnFrameMaximized(wxMaximizeEvent &event) {
	m_saveSettingsTimer->Start(500, wxTIMER_ONE_SHOT);
	event.Skip();
}

void MainFrame::OnSaveSettingsTimer(wxTimerEvent &event) {
	UserSettingsManager &settings = UserSettingsManager::Get();

	bool isMaximized = IsMaximized();
	settings.SetSetting("window/maximized", isMaximized);

	wxSize size = GetSize();
	settings.SetSetting("window/sizeX", size.x);
	settings.SetSetting("window/sizeY", size.y);
}

void MainFrame::WindowResizeFunctions() {
	SetMinSize(wxSize(800, 600));
	UserSettingsManager &settings = UserSettingsManager::Get();

	if (settings.GetSetting<bool>("window/maximized").value) {
		Maximize();
	} else {
		auto userPredefinedSize =
			wxSize(settings.GetSetting<int>("window/sizeX").value,
				   settings.GetSetting<int>("window/sizeY").value);

		SetSize(userPredefinedSize);
		Centre();
	}

	Bind(wxEVT_SIZE, &MainFrame::OnFrameResized, this);
	Bind(wxEVT_MAXIMIZE, &MainFrame::OnFrameMaximized, this);
}

void MainFrame::OnNewWindow(wxCommandEvent &WXUNUSED(event)) {
	auto newWindow = new MainFrame();
	if (!newWindow) {
		wxMessageBox(_("Failed to create a new window"));
		return;
	}
	newWindow->Show();
}

void MainFrame::OnAbout(wxCommandEvent &WXUNUSED(event)) {
	wxMessageBox(_("A fast, lightweight, and cross-platform code editor built "
				   "with C++ and wxWidgets"),
				 _("About Krafta Editor"), wxOK | wxICON_INFORMATION, this);
}

void MainFrame::OnPaintedComponent(wxPaintEvent &event) {
	auto target = ((wxSplitterWindow *)event.GetEventObject());
	if (!target || !target->IsEnabled())
		return;
	wxPaintDC dc(target);
	PaintSash(dc, target);

	wxColour borderColor = ThemesManager::Get().GetColor("border");
	dc.SetBrush(borderColor);
	dc.SetPen(wxPen(borderColor, 0.20));

	dc.DrawLine(target->GetSashPosition() + 3, 0, target->GetSashPosition() + 3,
				target->GetSize().y);
}

void MainFrame::PaintSash(wxDC &dc, wxSplitterWindow *target) {
	dc.SetPen(target->GetBackgroundColour());
	dc.SetBrush(target->GetBackgroundColour());

	if (target->GetSplitMode() == wxSPLIT_VERTICAL) {
		dc.DrawRectangle(target->GetSashPosition(), 0, target->GetSashSize(),
						 target->GetSize().GetHeight());
	} else {
		dc.DrawRectangle(0, target->GetSashPosition(),
						 target->GetSize().GetWidth(), target->GetSashSize());
	}
}

void MainFrame::OnSashPosChange(wxSplitterEvent &event) {
	auto target = ((wxSplitterWindow *)event.GetEventObject());
	if (!target)
		return;
	target->Refresh();
}

void MainFrame::OnGotoline(wxCommandEvent &event) { new Gotoline(this); }

void MainFrame::OnEditSettings(wxCommandEvent &WXUNUSED(event)) {
	if (!m_filesTree) {
		wxMessageBox(_("Files tree not initialized"));
		return;
	}

	wxString path = UserSettingsManager::Get().SettingsPath;
	if (wxFileExists(path)) {
		m_filesTree->OpenFile(path);
	} else {
		wxMessageBox(_("Settings file not found"));
	}
}

void MainFrame::OnEditShortcuts(wxCommandEvent &WXUNUSED(event)) {
	if (!m_filesTree) {
		wxMessageBox(_("Files tree not initialized"));

		return;
	}

	wxString path = ShortCutSettingsManager::Get().ShortcutsPath;
	if (wxFileExists(path)) {
		m_filesTree->OpenFile(path);
	} else {
		wxMessageBox(_("Settings file not found"));
	}
}

void MainFrame::OnExit(wxCommandEvent &WXUNUSED(event)) { Close(true); }

void MainFrame::OnClose(wxCloseEvent &event) {
	static bool isClosing = false;

	if (isClosing) {
		event.Skip();
		return;
	}
	isClosing = true;

	if (m_tabs) {
		m_tabs->CloseAllFiles();
	}

	if (m_watcher) {
		m_watcher->RemoveAll();
		Unbind(wxEVT_FSWATCHER, &MainFrame::OnFileSystemEvent, this);
		wxDELETE(m_watcher);
	}

	Destroy();
	event.Skip(false);
}

void MainFrame::UpdateRecentWorkspacesMenu(wxMenu *recentsMenu) {
	while (recentsMenu->GetMenuItemCount() > 0) {
		recentsMenu->Delete(recentsMenu->FindItemByPosition(0));
	}

	auto recents = WorkspaceStorageManager::Get().GetRecentWorkspaces();
	if (recents.empty()) {
		recentsMenu->Append(wxID_NONE, _("No Recent Workspaces"))
			->Enable(false);
		return;
	}

	for (size_t i = 0; i < recents.size(); ++i) {
		int currentID = ID_OPEN_RECENT_WORKSPACE_BASE + (int)i;
		recentsMenu->Append(currentID, recents[i].name, recents[i].path);
	}
}

void MainFrame::OnRecentWorkspaceClick(wxCommandEvent &event) {
	int id = event.GetId();
	wxMenuItem *item = m_menuBar->FindItem(id);
	if (item) {
		wxString path = item->GetHelp();
		if (!path.IsEmpty()) {
			this->LoadPath(path);
		}
	}
}

void MainFrame::ToggleAutosave(wxCommandEvent &WXUNUSED(event)) {
	auto &settingsManager = UserSettingsManager::Get();

	bool currentState =
		settingsManager.GetSetting<bool>("editor/autoSave").value;
	bool newState = !currentState;

	settingsManager.SetSetting<bool>("editor/autoSave", newState);
}