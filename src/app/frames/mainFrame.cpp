#include "mainFrame.hpp"

MainFrame::MainFrame(const wxString &title)
    : wxFrame(nullptr, wxID_ANY, title), m_watcher(nullptr), m_followLinks(false)
{
    SetTitle(title);
    WindowResizeFunctions();
    SetThemeEnabled(true);
    SetAppIcon();
    SetupMenuBar();

    SetupMainSplitter();
    SetupApplicationLeftMainContainer();
    SetupPageSwitcher();
    SetupSearchPage();
    SetupFilesTree();
    SetupApplicationRightMainContainer();
    SetupMainContainerSplitter();
    SetupCenteredContent();
    SetupMainContainer();
    SetupTabs();
    SetupEmptyWindow();
    SetupTerminal();
    SetupStatusBar();
    SetupAccelerators();

    SetSizer(sizer);

    SetDropTarget(new FrameFileDropTarget(this));
}

MainFrame::~MainFrame()
{
    delete m_watcher;
}

bool MainFrame::SetAppIcon()
{
    wxIcon aaplicationIcon;
    aaplicationIcon.LoadFile(ApplicationPaths::AssetsPath("images") + "kraftaEditor.png", wxBITMAP_TYPE_PNG);
    if (aaplicationIcon.IsOk())
    {
        SetIcon(aaplicationIcon);
        return true;
    }

    wxMessageBox(_("Failed to set application icon"));
    return false;
}

void MainFrame::SetupMenuBar()
{
    m_menuBar = new MenuBar();
    if (UserSettingsManager::Get().GetSetting<bool>("view/showMenuBar").value)
        SetMenuBar(m_menuBar);

    Bind(wxEVT_MENU, &MainFrame::OnRecentWorkspaceClick, this,
         ID_OPEN_RECENT_WORKSPACE_BASE, ID_OPEN_RECENT_WORKSPACE_MAX);
}

void MainFrame::SetupMainSplitter()
{
    m_mainSplitter = new wxSplitterWindow(this, +GUI::ControlID::MainSplitter);
    m_mainSplitter->SetBackgroundColour(ThemesManager::Get().GetColor("main"));
    wxBoxSizer *mainSplitterSizer = new wxBoxSizer(wxHORIZONTAL);
    m_mainSplitter->SetSizerAndFit(mainSplitterSizer);
    m_mainSplitter->SetMinimumPaneSize(250);

    sizer->Add(m_mainSplitter, 1, wxEXPAND);
    m_mainSplitter->Bind(wxEVT_PAINT, &MainFrame::OnPaintedComponent, this);
}

void MainFrame::SetupApplicationLeftMainContainer()
{
    m_applicationLeftMainContainer = new wxPanel(m_mainSplitter, +GUI::ControlID::ApplicationLeftMainContainer);
    wxBoxSizer *applicationLeftMainContainer_sizer = new wxBoxSizer(wxVERTICAL);
    m_applicationLeftMainContainer->SetSizerAndFit(applicationLeftMainContainer_sizer);
    m_mainSplitter->GetSizer()->Add(m_applicationLeftMainContainer, 0);
}

void MainFrame::SetupPageSwitcher()
{
    m_page_switcher = new PageSwitcher(m_applicationLeftMainContainer);
    m_applicationLeftMainContainer->GetSizer()->Add(m_page_switcher, 0, wxEXPAND);
}

void MainFrame::SetupSearchPage()
{
    m_searchPage = new SearchPage(m_applicationLeftMainContainer);
    m_applicationLeftMainContainer->GetSizer()->Add(m_searchPage, 1, wxEXPAND);

    m_searchPage->Hide();
}

void MainFrame::SetupFilesTree()
{
    m_filesTree = new FilesTree(m_applicationLeftMainContainer, +GUI::ControlID::FilesTree);
    m_applicationLeftMainContainer->GetSizer()->Add(m_filesTree, 1, wxEXPAND);
}

void MainFrame::SetupApplicationRightMainContainer()
{
    m_applicationRightMainContainer = new wxPanel(m_mainSplitter, +GUI::ControlID::ApplicationRightMainContainer);
    wxBoxSizer *applicationRightMainContainer_sizer = new wxBoxSizer(wxVERTICAL);
    m_applicationRightMainContainer->SetSizerAndFit(applicationRightMainContainer_sizer);
    m_mainSplitter->GetSizer()->Add(m_applicationRightMainContainer, 1, wxEXPAND);
    m_mainSplitter->SplitVertically(m_applicationLeftMainContainer, m_applicationRightMainContainer, 1);
}

void MainFrame::SetupMainContainerSplitter()
{
    m_mainContainerSplitter = new wxSplitterWindow(m_applicationRightMainContainer, +GUI::ControlID::MainContainerSplitter);
    wxBoxSizer *mainContainerSplitterSizer = new wxBoxSizer(wxVERTICAL);
    m_mainContainerSplitter->Bind(wxEVT_PAINT, &MainFrame::OnPaintedComponent, this);
    m_mainContainerSplitter->SetSizerAndFit(mainContainerSplitterSizer);
    m_applicationRightMainContainer->GetSizer()->Add(m_mainContainerSplitter, 1, wxEXPAND);
}

void MainFrame::SetupCenteredContent()
{
    m_centeredContent = new wxPanel(m_mainContainerSplitter, +GUI::ControlID::CenteredContent);
    wxBoxSizer *centeredContentSizer = new wxBoxSizer(wxVERTICAL);
    m_centeredContent->SetSizerAndFit(centeredContentSizer);
    m_mainContainerSplitter->GetSizer()->Add(m_centeredContent, 1, wxEXPAND);
}

void MainFrame::SetupMainContainer()
{
    m_mainContainer = new wxPanel(m_centeredContent, +GUI::ControlID::MainCode);
    wxBoxSizer *mainContainerSizer = new wxBoxSizer(wxVERTICAL);
    m_mainContainer->SetSizerAndFit(mainContainerSizer);
    m_centeredContent->GetSizer()->Add(m_mainContainer, 1, wxEXPAND);
}

void MainFrame::SetupTabs()
{
    m_tabs = new Tabs(m_mainContainer, +GUI::ControlID::Tabs);
    m_mainContainer->GetSizer()->Add(m_tabs, 0, wxEXPAND);
}

void MainFrame::SetupEmptyWindow()
{
    m_emptyWindow = new EmptyWindow(m_mainContainer, +GUI::ControlID::EmptyWindow);
    m_mainContainer->GetSizer()->Add(m_emptyWindow, 1, wxEXPAND);
}

void MainFrame::SetupTerminal()
{
    m_terminal = new Terminal(m_mainContainerSplitter, +GUI::ControlID::Terminal);
    m_mainContainerSplitter->GetSizer()->Add(m_terminal, 0);

    m_mainContainerSplitter->SplitVertically(m_centeredContent, m_terminal, 0);
    m_mainContainerSplitter->Unsplit(m_terminal);
}

void MainFrame::SetupStatusBar()
{
    m_statusBar = new StatusBar(m_applicationRightMainContainer);
    m_applicationRightMainContainer->GetSizer()->Add(m_statusBar, 0, wxEXPAND);
    m_statusBar->Hide();
}

void MainFrame::SetupAccelerators()
{
    wxAcceleratorEntry entries[4];

    entries[0].Set(wxACCEL_ALT, WXK_ALT, +Event::View::ToggleMenuBar);

    entries[1].Set(wxACCEL_CTRL, WXK_SHIFT, +Event::View::ToggleControlPanel);
    entries[1].FromString("Ctrl+Shift+P");

    entries[2].Set(wxACCEL_CTRL, int('P'), +Event::View::ToggleQuickOpen);
    entries[2].FromString("Ctrl+P");

    entries[3].Set(wxACCEL_CTRL, WXK_CONTROL_F, +Event::View::ToggleCodeSearch);
    entries[3].FromString("Ctrl+F");

    wxAcceleratorTable accel(4, entries);
    SetAcceleratorTable(accel);
}

void MainFrame::WindowResizeFunctions()
{
    SetMinSize(wxSize(800, 600));
    UserSettingsManager &settings = UserSettingsManager::Get();

    if (settings.GetSetting<bool>("window/maximized").value)
    {
        Maximize();
    }
    else
    {
        auto userPredefinedSize = wxSize(
            settings.GetSetting<int>("window/sizeX").value,
            settings.GetSetting<int>("window/sizeY").value);

        SetSize(userPredefinedSize);
        Centre();
    }

    Bind(wxEVT_SIZE, &MainFrame::OnFrameResized, this);
    Bind(wxEVT_MAXIMIZE, &MainFrame::OnFrameMaximized, this);
}

void MainFrame::OnFrameResized(wxSizeEvent &event)
{
    if (!IsMaximized())
    {
        auto &settings = UserSettingsManager::Get().currentSettings;
        settings["window"]["sizeX"] = event.GetSize().x;
        settings["window"]["sizeY"] = event.GetSize().y;
        settings["window"]["maximized"] = false;
    }

    event.Skip();
}

void MainFrame::OnFrameMaximized(wxMaximizeEvent &(event))
{
    UserSettingsManager::Get().SetSetting<bool>("window/maximized", true);
    event.Skip();
}

void MainFrame::OnNewWindow(wxCommandEvent &WXUNUSED(event))
{
    auto newWindow = new MainFrame();
    if (!newWindow)
    {
        wxMessageBox(_("Failed to create a new window"));
        return;
    }
    newWindow->Show();
}

bool MainFrame::CreateWatcherIfNecessary()
{
    try
    {
        if (m_watcher)
        {
            return false;
        }

        CreateWatcher();
        if (wxTheApp->IsActive())
        {
            Bind(wxEVT_FSWATCHER, &MainFrame::OnFileSystemEvent, this);
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }
    return true;
}

void MainFrame::CreateWatcher()
{
    if (!wxTheApp || !wxTheApp->IsActive() || IsBeingDeleted())
    {
        wxMessageBox(_("Cannot create watcher - application not active or being destroyed"));
        return;
    }

    if (m_watcher)
    {
        wxMessageBox(_("File watcher already initialized"));
        return;
    }

    m_watcher = new wxFileSystemWatcher();
    if (!m_watcher)
    {
        wxMessageBox(_("Failed to create file system watcher - memory allocation error"));
        return;
    }

    m_watcher->SetOwner(this);
}

void MainFrame::OnAbout(wxCommandEvent &WXUNUSED(event))
{
    wxMessageBox(_("A fast, lightweight, and cross-platform code editor built with C++ and wxWidgets"),
                 _("About Krafta Editor"),
                 wxOK | wxICON_INFORMATION, this);
}

void MainFrame::OnWatch(wxCommandEvent &event)
{
    if (event.IsChecked())
    {
        wxCHECK_RET(!m_watcher, "Watcher already initialized");
        CreateWatcher();
    }
    else
    {
        wxCHECK_RET(m_watcher, "Watcher not initialized");
        wxDELETE(m_watcher);
    }
}

void MainFrame::OnFollowLinks(wxCommandEvent &event) { m_followLinks = event.IsChecked(); }

void MainFrame::AddEntry(wxFSWPathType type, wxString filename)
{
    if (!m_watcher)
        return;
    if (filename.empty())
        return;

    wxCHECK_RET(m_watcher, "Watcher not initialized");
    wxString prefix;

    wxFileName fn = wxFileName::DirName(filename);
    if (!m_followLinks)
    {
        fn.DontFollowLink();
    }

    switch (type)
    {
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

void MainFrame::OnFileSystemEvent(wxFileSystemWatcherEvent &event)
{
    m_filesTree->OnFileSystemEvent(
        event.GetChangeType(),
        event.GetPath().GetFullPath(),
        event.GetNewPath().GetFullPath());
}

void MainFrame::OpenFolderDialog()
{
    wxDirDialog *dlg = new wxDirDialog(NULL, "Choose project directory", "", wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
    dlg->ShowModal();
    wxString path = dlg->GetPath();

    if (path.size())
        LoadPath(path);
}

void MainFrame::LoadPath(wxString path)
{
    if (!m_tabs || !m_filesTree)
    {
        wxMessageBox(_("Critical UI components (Tabs/FilesTree) not initialized"));
        return;
    }

    wxFileName fn;
    fn.AssignDir(path);
    wxString normalizedPath = fn.GetPath(wxPATH_GET_SEPARATOR);

    if (normalizedPath == ProjectSettings::Get().GetProjectPath())
        return;

    if (normalizedPath.IsEmpty() || !wxDirExists(normalizedPath))
    {
        wxMessageBox(wxString::Format(_("Directory '%s' does not exist"), normalizedPath), "Error", wxICON_ERROR);

        m_tabs->CloseAllFiles();
        m_filesTree->CloseProject();

        wxConfig globalConfig("krafta-editor");
        globalConfig.Write("workspace", "");

        new OpenFolderButton();
        return;
    }

    ProjectSettings::Get().ClearProject();
    ProjectSettings::Get().SetProjectPath(normalizedPath);
    ProjectSettings::Get().SetProjectName(wxFileNameFromPath(fn.GetFullPath().RemoveLast()));

    std::string workspaceId = WorkspaceStorageManager::ConvertPathToHash(normalizedPath.ToStdString());
    WorkspaceStorageManager::Get().Initialize(workspaceId);
    WorkspaceStorageManager::Get().AddToRecents(normalizedPath);

    wxConfig globalConfig("krafta-editor");
    globalConfig.Write("workspace", normalizedPath);

    m_tabs->CloseAllFiles();
    SetTitle("Krafta Editor - " + ProjectSettings::Get().GetProjectName());

    if (GetMenuBar() && m_menuBar->recentsWorkspacesMenu)
    {
        UpdateRecentWorkspacesMenu(m_menuBar->recentsWorkspacesMenu);
    }

    m_filesTree->LoadProject(m_filesTree->GetProjectFilesContainer(), normalizedPath);
    AddEntry(wxFSWPath_Tree, normalizedPath);

    auto lastFile = WorkspaceStorageManager::Get().GetSetting<std::string>("last_focused_file");
    if (lastFile.found && wxFileExists(lastFile.value))
    {
        m_filesTree->OpenFile(wxString(lastFile.value));
    }
}

void MainFrame::OnOpenFile(wxCommandEvent &WXUNUSED(event))
{
    wxFileDialog *dlg = new wxFileDialog(NULL, "Choose a file", "", "", "", wxFD_DEFAULT_STYLE | wxFD_FILE_MUST_EXIST);
    dlg->ShowModal();
    wxString path = dlg->GetPath();
    if (path.size())
    {
        m_filesTree->OpenFile(path);
        AddEntry(wxFSWPath_File, path);
    }
}

void MainFrame::OnPaintedComponent(wxPaintEvent &event)
{
    auto target = ((wxSplitterWindow *)event.GetEventObject());
    if (!target || !target->IsEnabled())
        return;
    wxPaintDC dc(target);
    PaintSash(dc, target);

    wxColour borderColor = ThemesManager::Get().GetColor("border");
    dc.SetBrush(borderColor);
    dc.SetPen(wxPen(borderColor, 0.20));

    dc.DrawLine(target->GetSashPosition() + 3, 0, target->GetSashPosition() + 3, target->GetSize().y);
}

void MainFrame::PaintSash(wxDC &dc, wxSplitterWindow *target)
{
    dc.SetPen(target->GetBackgroundColour());
    dc.SetBrush(target->GetBackgroundColour());

    if (target->GetSplitMode() == wxSPLIT_VERTICAL)
    {
        dc.DrawRectangle(
            target->GetSashPosition(),
            0,
            target->GetSashSize(),
            target->GetSize().GetHeight());
    }
    else
    {
        dc.DrawRectangle(
            0,
            target->GetSashPosition(),
            target->GetSize().GetWidth(),
            target->GetSashSize());
    }
}

void MainFrame::OnSashPosChange(wxSplitterEvent &event)
{
    auto target = ((wxSplitterWindow *)event.GetEventObject());
    if (!target)
        return;
    target->Refresh();
}

void MainFrame::CloseAllFiles(wxCommandEvent &WXUNUSED(event))
{
    m_tabs->CloseAllFiles();
}

void MainFrame::OnOpenTerminal(wxCommandEvent &WXUNUSED(event))
{
    if (m_mainContainerSplitter->IsSplit())
        m_mainContainerSplitter->Unsplit(m_terminal);
    else
        m_mainContainerSplitter->SplitHorizontally(m_centeredContent, m_terminal, 0);
}

void MainFrame::OnCloseFolder(wxCommandEvent &WXUNUSED(event))
{
    if (!m_filesTree || !m_mainContainer || !m_tabs)
    {
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

void MainFrame::OnToggleSearch(wxCommandEvent &WXUNUSED(event))
{
    if (FindWindowById(+GUI::ControlID::CodeSearchPanel))
    {
        ((wxWindow *)FindWindowById(+GUI::ControlID::CodeSearchPanel))->Destroy();
    }
    else
    {
        wxString defaultLabel = "";
        auto currentEditor = ((wxStyledTextCtrl *)wxFindWindowByLabel(ProjectSettings::Get().GetCurrentlyFileOpen() + "_codeEditor"));
        if (currentEditor)
            defaultLabel = currentEditor->GetSelectedText();

        new Search(this, defaultLabel, currentEditor);
    }
}

void MainFrame::OnToggleControlPanel(wxCommandEvent &WXUNUSED(event))
{
    if (FindWindowById(+GUI::ControlID::ControlPanel))
        m_controlPanel->Destroy();
    else
        m_controlPanel = new ControlPanel(this, +GUI::ControlID::ControlPanel);
}

void MainFrame::OnToggleQuickOpen(wxCommandEvent &WXUNUSED(event))
{
    if (FindWindowById(+GUI::ControlID::QuickOpen))
        m_quickOpen->Destroy();
    else
        m_quickOpen = new QuickOpen(this);
}

void MainFrame::OnToggleFileTreeView(wxCommandEvent &WXUNUSED(event))
{
    if (!m_mainSplitter || !m_applicationLeftMainContainer || !m_applicationRightMainContainer)
    {
        wxMessageBox(_("Main splitter or containers not initialized"));
        return;
    }

    if (m_mainSplitter->IsSplit())
        m_mainSplitter->Unsplit(m_applicationLeftMainContainer);
    else
        m_mainSplitter->SplitVertically(m_applicationLeftMainContainer, m_applicationRightMainContainer, 1);
}

void MainFrame::OnToggleMenuBarView(wxCommandEvent &WXUNUSED(event))
{
    if (GetMenuBar())
    {
        SetMenuBar(nullptr);
        UserSettingsManager::Get().SetSetting<bool>("showMenuBar", false);
    }
    else
    {
        SetMenuBar(m_menuBar);
        UserSettingsManager::Get().SetSetting<bool>("showMenuBar", true);
    }
}

void MainFrame::OnToggleStatusBarView(wxCommandEvent &WXUNUSED(event))
{
    if (!m_applicationRightMainContainer || !m_statusBar)
    {
        wxMessageBox(_("Status bar or right main container not initialized"));
        return;
    }

    if (m_statusBar->IsShown())
    {
        m_statusBar->Hide();
        UserSettingsManager::Get().SetSetting<bool>("showStatusBar", false);
    }
    else
    {
        m_statusBar->Show();
        UserSettingsManager::Get().SetSetting<bool>("showStatusBar", true);
    }
    m_applicationRightMainContainer->GetSizer()->Layout();
}

void MainFrame::OnToggleTabBarView(wxCommandEvent &WXUNUSED(event))
{
    if (!m_tabs || !m_mainContainer)
    {
        wxLogError(_("Tabs or main container not initialized"));
        return;
    }

    if (m_tabs->IsShown())
        m_tabs->Hide();
    else
        m_tabs->Show();

    m_mainContainer->GetSizer()->Layout();
}

void MainFrame::OnToggleMinimapView(wxCommandEvent &WXUNUSED(event))
{
    auto &settingsManager = UserSettingsManager::Get();

    bool currentState = settingsManager.GetSetting<bool>("editor/showMinimap").value;
    bool newState = !currentState;

    settingsManager.SetSetting<bool>("editor/showMinimap", newState);

    if (!m_tabs)
        return;

    for (auto &child : m_tabs->tabsContainer->GetChildren())
    {
        auto minimap = wxWindow::FindWindowByName(child->GetName() + "_codeMap");
        if (minimap)
        {
            minimap->Show(newState);
            minimap->GetParent()->Layout();
        }
    }
}

void MainFrame::OnEditSettings(wxCommandEvent &WXUNUSED(event))
{
    if (!m_filesTree)
    {
        wxMessageBox(_("Files tree not initialized"));
        return;
    }

    wxString path = UserSettingsManager::Get().SettingsPath;
    if (wxFileExists(path))
    {
        m_filesTree->OpenFile(path);
    }
    else
    {
        wxMessageBox(_("Settings file not found"));
    }
}
void MainFrame::OnEditShortcuts(wxCommandEvent &WXUNUSED(event))
{
    if (!m_filesTree)
    {
        wxMessageBox(_("Files tree not initialized"));

        return;
    }

    wxString path = ShortCutSettingsManager::Get().ShortcutsPath;
    if (wxFileExists(path))
    {
        m_filesTree->OpenFile(path);
    }
    else
    {
        wxMessageBox(_("Settings file not found"));
    }
}

void MainFrame::OnExit(wxCommandEvent &WXUNUSED(event))
{
    Close(true);
}

void MainFrame::OnClose(wxCloseEvent &event)
{
    static bool isClosing = false;

    if (isClosing)
    {
        event.Skip();
        return;
    }
    isClosing = true;

    if (m_tabs)
    {
        m_tabs->CloseAllFiles();
    }

    if (m_watcher)
    {
        m_watcher->RemoveAll();
        Unbind(wxEVT_FSWATCHER, &MainFrame::OnFileSystemEvent, this);
        wxDELETE(m_watcher);
    }

    UserSettingsManager::Get().Update(UserSettingsManager::Get().currentSettings);

    Destroy();
    event.Skip(false);
}

void MainFrame::OnOpenFolderMenu(wxCommandEvent &WXUNUSED(event)) { OpenFolderDialog(); }
void MainFrame::OnOpenFolderClick(wxMouseEvent &WXUNUSED(event)) { OpenFolderDialog(); }

void MainFrame::UpdateRecentWorkspacesMenu(wxMenu *recentsMenu)
{
    while (recentsMenu->GetMenuItemCount() > 0)
    {
        recentsMenu->Delete(recentsMenu->FindItemByPosition(0));
    }

    auto recents = WorkspaceStorageManager::Get().GetRecentWorkspaces();
    if (recents.empty())
    {
        recentsMenu->Append(wxID_NONE, _("No Recent Workspaces"))->Enable(false);
        return;
    }

    for (size_t i = 0; i < recents.size(); ++i)
    {
        int currentID = ID_OPEN_RECENT_WORKSPACE_BASE + (int)i;
        recentsMenu->Append(currentID, recents[i].name, recents[i].path);
    }
}

void MainFrame::OnRecentWorkspaceClick(wxCommandEvent &event)
{
    int id = event.GetId();
    wxMenuItem *item = m_menuBar->FindItem(id);
    if (item)
    {
        wxString path = item->GetHelp();
        if (!path.IsEmpty())
        {
            this->LoadPath(path);
        }
    }
}