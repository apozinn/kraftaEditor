#include "mainFrame.hpp"

MainFrame::MainFrame(const wxString &title)
    : wxFrame(nullptr, wxID_ANY, title), m_watcher(nullptr), m_followLinks(false)
{
    // Set the window title
    SetTitle(title);

    // Enable theme support for the window
    SetThemeEnabled(true);

    // Set the application icon
    SetAppIcon();

    // Initialize and configure the menu bar
    SetupMenuBar();

    // Initialize main UI components
    // ----------------------------

    // Create the main splitter window (divides main areas)
    SetupMainSplitter();

    // Setup left container panel
    SetupApplicationLeftMainContainer();

    // Initialize the file tree view
    SetupFilesTree();

    // Setup right container panel
    SetupApplicationRightMainContainer();

    // Configure the splitter between main containers
    SetupMainContainerSplitter();

    // Setup the centered content area
    SetupCenteredContent();

    // Configure the main container layout
    SetupMainContainer();

    // Initialize tab interface
    SetupTabs();

    // Setup empty content window
    SetupEmptyWindow();

    // Initialize m_terminal/console component
    SetupTerminal();

    // Create and configure the status bar
    SetupStatusBar();

    // Configure keyboard shortcuts
    SetupAccelerators();

    // Set up window resize handlers
    WindowResizeFunctions();

    // Apply the main sizer and fit the window to its contents
    SetSizerAndFit(sizer);
    SetMinSize(wxSize(800, 600));
}

bool MainFrame::SetAppIcon()
{
    wxIcon aaplicationIcon;
    aaplicationIcon.LoadFile(assetsImagePath + "kraftaEditor.png", wxBITMAP_TYPE_PNG);
    if (aaplicationIcon.IsOk())
    {
        SetIcon(aaplicationIcon);
        return true;
    }

    wxLogError("Failed to set application icon");
    return false;
}

void MainFrame::SetupMenuBar()
{
    m_menuBar = new MenuBar();
    if (UserSettings["show_menu"] == true)
        SetMenuBar(m_menuBar);
}

void MainFrame::SetupMainSplitter()
{
    m_mainSplitter = new wxSplitterWindow(this, +GUI::ControlID::MainSplitter);
    m_mainSplitter->SetBackgroundColour(wxColor(Theme["main"].template get<std::string>()));
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
    m_statusBar = new StatusBar(m_applicationRightMainContainer, +GUI::ControlID::StatusBar);
    m_applicationRightMainContainer->GetSizer()->Add(m_statusBar, 0, wxEXPAND);
    m_statusBar->Hide();
}

void MainFrame::SetupAccelerators()
{
    wxAcceleratorEntry entries[4];
    entries[0].Set(wxACCEL_ALT, WXK_ALT, +Event::View::ToggleMenuBar);
    entries[1].Set(wxACCEL_CTRL, WXK_SHIFT, +Event::View::ToggleControlPanel);
    entries[1].FromString("Ctrl+Shift+P");
    entries[2].Set(wxACCEL_CTRL, WXK_CONTROL_F, +Event::View::ToggleCodeSearch);
    entries[2].FromString("Ctrl+F");
    wxAcceleratorTable accel(4, entries);
    SetAcceleratorTable(accel);
}

void MainFrame::WindowResizeFunctions()
{
    if (UserSettings["windowMaximized"] == true)
    {
        Maximize();
    }
    else
    {
        SetSize(wxSize(
            UserSettings["windowSizeX"].template get<int>(),
            UserSettings["windowSizeY"].template get<int>()));
        Centre();
    }

    // resized events
    Bind(wxEVT_SIZE, &MainFrame::OnFrameResized, this);
    Bind(wxEVT_MAXIMIZE, &MainFrame::OnFrameMaximized, this);
}

MainFrame::~MainFrame()
{
    delete m_watcher;
}

void MainFrame::OnFrameResized(wxSizeEvent &event)
{
    if (!IsMaximized())
    {
        UserSettings["windowMaximized"] = false;
        if( event.GetSize().x < 800 || event.GetSize().y < 600)
        {
            event.SetSize(wxSize(
                std::max(event.GetSize().x, 800),
                std::max(event.GetSize().y, 600)));
            SetSize(event.GetSize());
        }
        UserSettings["windowSizeX"] = event.GetSize().x;
        UserSettings["windowSizeY"] = event.GetSize().y;
        UserSettingsManager::Get().Update(UserSettings);
    }
    event.Skip();
}

void MainFrame::OnFrameMaximized(wxMaximizeEvent &WXUNUSED(event))
{
    UserSettings["windowMaximized"] = true;
    UserSettingsManager::Get().Update(UserSettings);
}

void MainFrame::OnNewWindow(wxCommandEvent &WXUNUSED(event))
{
    auto newWindow = new MainFrame();
    if (!newWindow)
    {
        wxLogError("Failed to create a new window - memory allocation error");
        return;
    }
    newWindow->Show();
}

bool MainFrame::CreateWatcherIfNecessary()
{
    if (m_watcher)
    {
        return false; // Watcher already exists
    }

    CreateWatcher();
    Bind(wxEVT_FSWATCHER, &MainFrame::OnFileSystemEvent, this);
    return true;
}

void MainFrame::CreateWatcher()
{
    // Check if application is in valid state for watcher creation
    if (!wxTheApp || !wxTheApp->IsActive() || IsBeingDeleted())
    {
        wxLogWarning("Cannot create watcher - application not active or being destroyed");
        return;
    }

    // Prevent duplicate initialization
    if (m_watcher)
    {
        wxLogError("File watcher already initialized");
        return;
    }

    // Create watcher instance
    m_watcher = new wxFileSystemWatcher();
    if (!m_watcher)
    {
        wxLogFatalError("Failed to create file system watcher - memory allocation error");
        return;
    }

    // Configure watcher
    m_watcher->SetOwner(this);
}

void MainFrame::OnAbout(wxCommandEvent &WXUNUSED(event))
{
    wxMessageBox("A fast, lightweight, and cross-platform code editor built with C++ and wxWidgets",
                 "About Krafta Editor",
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
    {
        projectSettings.SetProjectName(wxFileNameFromPath(path));
        if (path.Last() != PlatformInfos::OsPathSeparator())
            path.Append(PlatformInfos::OsPathSeparator());

        projectSettings.SetProjectPath(path);

        if (m_tabs)
            m_tabs->CloseAllFiles();

        m_filesTree->LoadProject(m_filesTree->GetProjectFilesContainer(), path);

        wxConfig *config = new wxConfig("krafta-editor");
        config->Write("workspace", path);
        delete config;

        SetTitle("Krafta Editor - " + projectSettings.GetProjectName());
        AddEntry(wxFSWPath_Tree, path);
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

    dc.SetBrush(ThemesManager::Get().GetColor("border"));
    dc.SetPen(wxPen(ThemesManager::Get().GetColor("border"), 0.20));

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

void MainFrame::LoadPath(wxString path)
{
    if (!m_tabs || !m_filesTree)
    {
        wxLogError("Tabs or files tree not initialized");
        return;
    }

    wxDir dir(path);
    wxConfig *config = new wxConfig("krafta-editor");

    if (path.IsEmpty() || !dir.Exists(path))
    {
        m_tabs->CloseAllFiles();
        config->Write("workspace", "");
        delete config;

        new OpenFolderButton();
        return;
    }

    if (path == ProjectSettings::Get().GetProjectPath())
        return;

    if (path.Last() != PlatformInfos::OsPathSeparator())
        path.Append(PlatformInfos::OsPathSeparator());

    wxString pathWithoutPathSeparator = path.Clone().RemoveLast();

    projectSettings.SetProjectName(wxFileNameFromPath(pathWithoutPathSeparator));
    projectSettings.SetProjectPath(path);

    config->Write("workspace", path);
    delete config;

    m_tabs->CloseAllFiles();

    m_filesTree->LoadProject(m_filesTree->GetProjectFilesContainer(), path);

    SetTitle("Krafta Editor - " + projectSettings.GetProjectName());

    AddEntry(wxFSWPath_Tree, path);
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
    if (!m_filesTree)
    {
        wxLogError("Files tree not initialized");
        return;
    }

    if (!m_mainContainer)
    {
        wxLogError("Main container not initialized");
        return;
    }

    if (!m_tabs)
    {
        wxLogError("Tabs not initialized");
        return;
    }

    m_filesTree->CloseProject();
    m_tabs->CloseAllFiles();

    wxConfig *config = new wxConfig("krafta-editor");
    config->Write("workspace", "");
    delete config;

    projectSettings.ClearProject();

    new OpenFolderButton();
}

// toggle view events
void MainFrame::OnToggleSearch(wxCommandEvent &WXUNUSED(event))
{
    if (FindWindowById(+GUI::ControlID::CodeSearchPanel))
    {
        ((wxWindow *)FindWindowById(+GUI::ControlID::CodeSearchPanel))->Destroy();
    }
    else
    {
        wxString defaultLabel = "Search a text";
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

void MainFrame::OnToggleFileTreeView(wxCommandEvent &WXUNUSED(event))
{
    if (!m_mainSplitter || !m_applicationLeftMainContainer || !m_applicationRightMainContainer)
    {
        wxLogError("Main splitter or containers not initialized");
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
        UserSettings["show_menu"] = false;
    }
    else
    {
        SetMenuBar(m_menuBar);
        UserSettings["show_menu"] = true;
    }

    UserSettingsManager::Get().Update(UserSettings);
}

void MainFrame::OnToggleStatusBarView(wxCommandEvent &WXUNUSED(event))
{
    if (!m_applicationRightMainContainer || !m_statusBar)
    {
        wxLogError("Status bar or right main container not initialized");
        return;
    }

    if (m_statusBar->IsShown())
    {
        m_statusBar->Hide();
        UserSettings["show_statusBar"] = false;
    }
    else
    {
        m_statusBar->Show();
        UserSettings["show_statusBar"] = true;
    }
    m_applicationRightMainContainer->GetSizer()->Layout();

    UserSettingsManager::Get().Update(UserSettings);
}

void MainFrame::OnToggleTabBarView(wxCommandEvent &WXUNUSED(event))
{
    if (!m_tabs || !m_mainContainer)
    {
        wxLogError("Tabs or main container not initialized");
        return;
    }

    if (m_tabs->IsShown())
        m_tabs->Hide();
    else
        m_tabs->Show();

    m_mainContainer->GetSizer()->Layout();
}

void MainFrame::OnEditSettings(wxCommandEvent &WXUNUSED(event))
{
    if (!m_filesTree)
    {
        wxLogError("Files tree not initialized");
        return;
    }

    wxString path = ApplicationPaths::ApplicationPath() + "user_settings.json";
    if (wxFileExists(path))
    {
        m_filesTree->OpenFile(path);
        AddEntry(wxFSWPath_File, path);
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

    Destroy();
    event.Skip(false);
}

void MainFrame::OnOpenFolderMenu(wxCommandEvent &WXUNUSED(event)) { OpenFolderDialog(); }
void MainFrame::OnOpenFolderClick(wxMouseEvent &WXUNUSED(event)) { OpenFolderDialog(); }