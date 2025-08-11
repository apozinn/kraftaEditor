#include "./mainFrame.hpp"

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

    // Check for and load last workspace (if any)
    SetupLastWordspace();

    // Configure keyboard shortcuts
    SetupAccelerators();

    // Set up window resize handlers
    WindowResizeFunctions();

    // Apply the main sizer and fit the window to its contents
    SetSizerAndFit(sizer);
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

void MainFrame::SetupLastWordspace()
{
    wxConfig *config = new wxConfig("krafta-editor");
    wxString str;
    if (config->Read("workspace", &str))
    {
        wxString last_workspace = config->Read("workspace", str);
        LoadPath(last_workspace);
    }
    else
    {
        m_tabs->CloseAll();
        // creating a button to open a folder
        if (auto projectContainer = FindWindowById(+GUI::ControlID::ProjectFilesContainer))
        {
            m_openFolderButton = new OpenFolderButton(projectContainer, +GUI::ControlID::OpenFolderButton);
            m_openFolderButton->Bind(wxEVT_LEFT_UP, &MainFrame::OnOpenFolderClick, this);
            for (auto &&children : m_openFolderButton->GetChildren())
                children->Bind(wxEVT_LEFT_UP, &MainFrame::OnOpenFolderClick, this);
        }
    }
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

MainFrame::~MainFrame() { delete m_watcher; }

void MainFrame::OnFrameResized(wxSizeEvent &event)
{
    if (!IsMaximized())
    {
        UserSettings["windowMaximized"] = false;
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
    // auto newApp = new KraftaEditor();
    // newApp->CreateMainWindow();
}

bool MainFrame::CreateWatcherIfNecessary()
{
    if (m_watcher) {
        return false; // Watcher already exists
    }
    
    CreateWatcher();
    Bind(wxEVT_FSWATCHER, &MainFrame::OnFileSystemEvent, this);
    return true;
}

void MainFrame::CreateWatcher()
{
    // Check if application is in valid state for watcher creation
    if (!wxTheApp || !wxTheApp->IsActive() || IsBeingDeleted()) {
        wxLogWarning("Cannot create watcher - application not active or being destroyed");
        return;
    }

    // Prevent duplicate initialization
    if (m_watcher) {
        wxLogError("File watcher already initialized");
        return;
    }

    // Create watcher instance
    m_watcher = new wxFileSystemWatcher();
    if (!m_watcher) {
        wxLogFatalError("Failed to create file system watcher - memory allocation error");
        return;
    }

    // Configure watcher
    m_watcher->SetOwner(this);
    wxLogDebug("File system watcher created successfully");
}
void MainFrame::OnAbout(wxCommandEvent &WXUNUSED(event))
{
    wxMessageBox("A simple code editor for multiple languages",
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
    m_filesTree->OnComponentModified(
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
        path.Append(wxString(PlatformInfos::OsPathSepareator()));
        projectSettings.SetProjectPath(path);

        if (m_tabs)
            m_tabs->CloseAll();

        m_filesTree->Load(m_filesTree->projectFilesContainer, path.ToStdString());

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
    m_tabs->CloseAll();
}

void MainFrame::LoadPath(wxString path)
{
    wxDir dir(path);
    // verify if dir exists
    if (!dir.Exists(path))
    {
        m_tabs->CloseAll();

        // dir dont finded
        wxConfig *config = new wxConfig("krafta-editor");
        config->Write("workspace", "");
        delete config;

        // creating a button to open a folder
        if (auto projectContainer = FindWindowById(+GUI::ControlID::ProjectFilesContainer))
            m_openFolderButton = new OpenFolderButton(projectContainer, +GUI::ControlID::OpenFolderButton);

        m_openFolderButton->Bind(wxEVT_LEFT_UP, &MainFrame::OnOpenFolderClick, this);
        for (auto &&children : m_openFolderButton->GetChildren())
            children->Bind(wxEVT_LEFT_UP, &MainFrame::OnOpenFolderClick, this);
        return;
    }

    projectSettings.SetProjectName(wxFileNameFromPath(path.substr(0, path.size() - 1)));
    projectSettings.SetProjectPath(path);

    wxConfig *config = new wxConfig("krafta-editor");
    config->Write("workspace", path);
    delete config;

    m_tabs->CloseAll();
    m_filesTree->Load(m_filesTree->projectFilesContainer, path.ToStdString());
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
    m_filesTree->projectFilesContainer->DestroyChildren();
    m_filesTree->projectToggler->Hide();

    for (auto &&mainCodeChildren : m_mainCode->GetChildren())
    {
        std::string childrenName = mainCodeChildren->GetLabel().ToStdString();
        if (childrenName.find("_codeContainer") != std::string::npos)
        {
            mainCodeChildren->Destroy();
        }
        else
            mainCodeChildren->Hide();
    }

    m_emptyWindow->Show();
    m_tabs->CloseAll();

    wxConfig *config = new wxConfig("krafta-editor");
    config->Write("workspace", "");
    delete config;

    projectSettings.ClearProject();

    // creating a button to open a folder
    if (auto projectContainer = FindWindowById(+GUI::ControlID::ProjectFilesContainer))
    {
        m_openFolderButton = new OpenFolderButton(projectContainer, +GUI::ControlID::OpenFolderButton);

        m_openFolderButton->Bind(wxEVT_LEFT_UP, &MainFrame::OnOpenFolderClick, this);
        for (auto &&children : m_openFolderButton->GetChildren())
            children->Bind(wxEVT_LEFT_UP, &MainFrame::OnOpenFolderClick, this);
        projectContainer->GetSizer()->Layout();
    }
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
        {
            defaultLabel = currentEditor->GetSelectedText();
        }

        new Search(this, defaultLabel, currentEditor);
    }
}

void MainFrame::OnToggleControlPanel(wxCommandEvent &WXUNUSED(event))
{
    if (FindWindowById(+GUI::ControlID::ControlPanel))
    {
        m_controlPanel->Destroy();
    }
    else
    {
        m_controlPanel = new ControlPanel(this, +GUI::ControlID::ControlPanel);
    }
}

void MainFrame::OnToggleFileTreeView(wxCommandEvent &WXUNUSED(event))
{
    if (m_mainSplitter->IsSplit())
    {
        m_mainSplitter->Unsplit(m_applicationLeftMainContainer);
    }
    else
    {
        m_mainSplitter->SplitVertically(m_applicationLeftMainContainer, m_applicationContent, 1);
    }
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
    if (m_statusBar)
    {
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
        m_applicationContent->GetSizer()->Layout();
    }
    UserSettingsManager::Get().Update(UserSettings);
}

void MainFrame::OnToggleTabBarView(wxCommandEvent &WXUNUSED(event))
{
    if (m_tabs)
    {
        if (m_tabs->IsShown())
        {
            m_tabs->Hide();
        }
        else
            m_tabs->Show();
        m_mainContainer->GetSizer()->Layout();
    }
}

void MainFrame::OnEditSettings(wxCommandEvent &WXUNUSED(event))
{
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

void MainFrame::OnClose(wxCloseEvent& event)
{
    static bool isClosing = false;
    
    // Prevent reentrant calls
    if (isClosing) {
        event.Skip();
        return;
    }
    isClosing = true;

    // Close all tabs
    if (m_tabs) {
        m_tabs->CloseAll();
    }

    // Clean up file watcher
    if (m_watcher) {
        wxLogDebug("Cleaning up file system watcher");
        m_watcher->RemoveAll();  // Stop all watches
        Unbind(wxEVT_FSWATCHER, &MainFrame::OnFileSystemEvent, this);  // Remove event handler
        wxDELETE(m_watcher);  // Safe deletion (sets pointer to nullptr)
    }

    // Initiate controlled shutdown
    Destroy();  // This will automatically exit main loop when done
    event.Skip(false);  // We've handled the event completely
}

void MainFrame::OnOpenFolderMenu(wxCommandEvent &WXUNUSED(event)) { OpenFolderDialog(); }
void MainFrame::OnOpenFolderClick(wxMouseEvent &WXUNUSED(event)) { OpenFolderDialog(); }

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
    // File Operations
    EVT_MENU(+Event::File::CreateFileEvent, FilesTree::OnCreateFile)
    EVT_MENU(+Event::File::CreateDir, FilesTree::OnCreateDir)
    EVT_MENU(+Event::File::Save, CodeContainer::OnSave)
    EVT_MENU(+Event::File::SaveAs, CodeContainer::OnSaveAs)
    EVT_MENU(+Event::File::SaveAll, CodeContainer::OnSaveAll)
    EVT_MENU(+Event::File::CloseFile, CodeContainer::OnCloseFile)
    EVT_MENU(+Event::File::CloseAll, MainFrame::CloseAllFiles)
    EVT_MENU(+Event::File::OpenFile, MainFrame::OnOpenFile)

    // Edit Operations
    EVT_MENU(+Event::Edit::Cut, CodeContainer::OnCut)
    EVT_MENU(+Event::Edit::Copy, CodeContainer::OnCopy)
    EVT_MENU(+Event::Edit::Paste, CodeContainer::OnPaste)
    EVT_MENU(+Event::Edit::Redo, CodeContainer::OnRedo)
    EVT_MENU(+Event::Edit::Undo, CodeContainer::OnUndo)
    EVT_MENU(+Event::Edit::ToggleComment, CodeContainer::ToggleCommentLine)
    EVT_MENU(+Event::Edit::ToggleBlockComment, CodeContainer::ToggleCommentBlock)
    EVT_MENU(+Event::Edit::SelectLine, CodeContainer::OnSelectLine)
    EVT_MENU(+Event::Edit::SelectAll, CodeContainer::OnSelectAll)

    // View Operations
    EVT_MENU(+Event::View::ToggleMiniMap, CodeContainer::OnToggleMinimapView)
    EVT_MENU(+Event::View::ToggleCodeSearch, MainFrame::OnToggleSearch)
    EVT_MENU(+Event::View::ToggleControlPanel, MainFrame::OnToggleControlPanel)
    EVT_MENU(+Event::View::ToggleFileTree, MainFrame::OnToggleFileTreeView)
    EVT_MENU(+Event::View::ToggleMenuBar, MainFrame::OnToggleMenuBarView)
    EVT_MENU(+Event::View::ToggleStatusBar, MainFrame::OnToggleStatusBarView)
    EVT_MENU(+Event::View::ToggleTabBar, MainFrame::OnToggleTabBarView)

    // Project Operations
    EVT_MENU(+Event::Project::OpenFolder, MainFrame::OnOpenFolderMenu)
    EVT_MENU(+Event::Project::CloseFolder, MainFrame::OnCloseFolder)

    // Frame Operations
    EVT_MENU(+Event::Frame::NewWindow, MainFrame::OnNewWindow)
    EVT_MENU(+Event::Frame::Exit, MainFrame::OnExit)
    EVT_MENU(+Event::Frame::About, MainFrame::OnAbout)
    EVT_CLOSE(MainFrame::OnClose)

    // Terminal
    EVT_MENU(+Event::Terminal::Open, MainFrame::OnOpenTerminal)

    // Settings
    EVT_MENU(+Event::UserSettings::Edit, MainFrame::OnEditSettings)
wxEND_EVENT_TABLE()