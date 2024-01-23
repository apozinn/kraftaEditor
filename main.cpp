#include "main.hpp"

#include <wx/sizer.h>
#include <wx/dirdlg.h>
#include <filesystem>
#include <wx/sashwin.h>
#include "wx/image.h"
#include "wx/file.h"
#include "wx/filename.h"

MainFrame::MainFrame(const wxString &title)
    : wxFrame(nullptr, wxID_ANY, title),
      m_watcher(nullptr), m_followLinks(false)
{
    wxInitAllImageHandlers();
    if (wxFile::Exists("./icons/settings.png"))
        icons_dir = "./icons/";
    else if (wxFile::Exists("../icons/settings.png"))
        icons_dir = "../icons/";
    else
        wxLogWarning("Can't find icons dir!");

    json user_config = UserConfig().Get();
    sizer = new wxBoxSizer(wxVERTICAL);

    mainSplitter = new wxSplitterWindow(this, ID_MAIN_SPLITTER);
    mainSplitter->Bind(wxEVT_PAINT, &MainFrame::OnSashPaint, this);
    wxBoxSizer *mainSplitterSizer = new wxBoxSizer(wxHORIZONTAL);

    navigationContainer = new wxPanel(mainSplitter);
    wxBoxSizer *navigationContainerSizer = new wxBoxSizer(wxVERTICAL);
    files_tree = new FilesTree(navigationContainer, ID_FILES_TREE);
    navigationContainerSizer->Add(files_tree, 1, wxEXPAND);
    navigationContainer->SetSizerAndFit(navigationContainerSizer);

    wxPanel *applicationContent = new wxPanel(mainSplitter, ID_APPLICATION_CONTENT);
    wxBoxSizer *applicationContentSizer = new wxBoxSizer(wxVERTICAL);

    wxSplitterWindow *mainContainerSplitter = new wxSplitterWindow(applicationContent, ID_MAIN_CONTAINER_SPLITTER);
    mainContainerSplitter->SetBackgroundColour(wxColor(*wxRED));
    wxBoxSizer *mainContainerSplitterSizer = new wxBoxSizer(wxVERTICAL);

    wxPanel *centeredContent = new wxPanel(mainContainerSplitter, ID_CENTERED_CONTENT);
    wxBoxSizer *centeredContentSizer = new wxBoxSizer(wxVERTICAL);

    mainContainer = new wxPanel(centeredContent, ID_MAIN_CONTAINER);
    mainContainer->SetBackgroundColour(wxColor("#fff"));
    wxBoxSizer *mainContainerSizer = new wxBoxSizer(wxVERTICAL);

    tabs = new Tabs(mainContainer, ID_TABS);
    mainContainerSizer->Add(tabs, 0, wxEXPAND);

    mainCode = new wxPanel(mainContainer, ID_MAIN_CODE);
    mainCode->SetBackgroundColour(wxColor(*wxRED));

    mainContainerSizer->Add(mainCode, 1, wxEXPAND);

    wxBoxSizer *mainCodeSizer = new wxBoxSizer(wxVERTICAL);

    empty_window = new EmptyWindow(mainCode, ID_EMPYT_WINDOW);
    mainCodeSizer->Add(empty_window, 1, wxEXPAND);
    mainCode->SetSizerAndFit(mainCodeSizer);

    mainContainer->SetSizerAndFit(mainContainerSizer);

    centeredContentSizer->Add(mainContainer, 1, wxEXPAND);

    centeredContent->SetSizerAndFit(centeredContentSizer);

    mainContainerSplitterSizer->Add(centeredContent, 1, wxEXPAND);

    terminal = new Terminal(mainContainerSplitter, ID_TERMINAL);
    mainContainerSplitterSizer->Add(terminal, 0);

    mainContainerSplitter->SplitVertically(centeredContent, terminal, 0);
    mainContainerSplitter->Unsplit(terminal);

    mainContainerSplitter->SetSizerAndFit(mainContainerSplitterSizer);

    applicationContentSizer->Add(mainContainerSplitter, 1, wxEXPAND);

    status_bar = new StatusBar(applicationContent, ID_STATUS_BAR);
    applicationContentSizer->Add(status_bar, 0, wxEXPAND);

    applicationContent->SetSizerAndFit(applicationContentSizer);

    mainSplitterSizer->Add(navigationContainer, 0);
    mainSplitterSizer->Add(applicationContent, 1, wxEXPAND);
    mainSplitter->SetSizerAndFit(mainSplitterSizer);

    mainSplitter->SetMinimumPaneSize(250);
    mainSplitter->SplitVertically(navigationContainer, applicationContent, 1);
    sizer->Add(mainSplitter, 1, wxEXPAND);

    SetTitle("ThunderCode");
    SetSizerAndFit(sizer);
    SetOwnForegroundColour(wxColour(*wxWHITE));
    SetThemeEnabled(true);
    Maximize();

    menu_bar = new MenuBar();
    SetMenuBar(menu_bar);
    if (user_config["show_menu"] == false)
        menu_bar->Hide();

    wxConfig *config = new wxConfig("ThunderCode");
    wxString str;
    if (config->Read("workspace", &str))
    {
        wxString last_workspace = config->Read("workspace", str);
        LoadPath(last_workspace);
    }
    else
    {
        tabs->CloseAll();
        if (auto pjt_ctn = FindWindowById(ID_PROJECT_FILES_CTN))
        {
            open_folder_link = new OpenFolderLink(pjt_ctn, ID_OPEN_FOLDER_LINK);
            open_folder_link->Bind(wxEVT_LEFT_UP, &MainFrame::OnOpenFolderClick, this);
            for (auto &&children : open_folder_link->GetChildren())
                children->Bind(wxEVT_LEFT_UP, &MainFrame::OnOpenFolderClick, this);
            pjt_ctn->GetSizer()->Add(open_folder_link, 1, wxEXPAND);
        }
    }

    wxAcceleratorEntry entries[4];
    entries[0].Set(wxACCEL_ALT, WXK_ALT, ID_HIDDE_MENU_BAR);
    entries[1].Set(wxACCEL_CTRL, WXK_SHIFT, ID_TOGGLE_CONTROL_PANEL);
    entries[1].FromString("Ctrl+Shift+P");
    entries[2].Set(wxACCEL_CTRL, WXK_CONTROL_F, ID_TOGGLE_FIND);
    entries[2].FromString("Ctrl+F");
    entries[3].Set(wxACCEL_CTRL, WXK_SHIFT, ID_GOTO_SEARCHPAGE);
    entries[3].FromString("Ctrl+Shift+F");
    wxAcceleratorTable accel(4, entries);
    SetAcceleratorTable(accel);
}

MainFrame::~MainFrame() { delete m_watcher; }

bool MainFrame::CreateWatcherIfNecessary()
{
    if (m_watcher)
        return false;
    CreateWatcher();
    Bind(wxEVT_FSWATCHER, &MainFrame::OnFileSystemEvent, this);
    return true;
}

void MainFrame::CreateWatcher()
{
    wxCHECK_RET(!m_watcher, _("Watcher already initialized"));
    m_watcher = new wxFileSystemWatcher();
    m_watcher->SetOwner(this);
}

void MainFrame::OnAbout(wxCommandEvent &WXUNUSED(event))
{
    wxMessageBox("A simple Code-editor for multiple usages and languages"
                 "(c) 2023 Okarin Services\n",
                 "About ThunderCode",
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
    bool ok = false;

    wxFileName fn = wxFileName::DirName(filename);
    if (!m_followLinks)
    {
        fn.DontFollowLink();
    }

    switch (type)
    {
    case wxFSWPath_Dir:
        ok = m_watcher->Add(fn);
        prefix = "Dir:  ";
        break;
    case wxFSWPath_Tree:
        ok = m_watcher->AddTree(fn);
        prefix = "Tree: ";
        break;
    case wxFSWPath_File:
    case wxFSWPath_None:
        wxFAIL_MSG("Unexpected path type.");
    }
}

void MainFrame::OnFileSystemEvent(wxFileSystemWatcherEvent &event)
{
    wxString type = GetFSWEventChangeTypeName(event.GetChangeType());
    if (type != "ACCESS")
    {
        files_tree->OnTreeModifyed(
            event.GetPath().GetFullPath(),
            event.GetNewPath().GetFullPath());
    }
}

void MainFrame::OpenFolderDialog()
{
    wxDirDialog *dlg = new wxDirDialog(NULL, "Choose project directory", "", wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
    dlg->ShowModal();
    wxString path = dlg->GetPath();
    if (path.size())
    {
        project_path = path + "/";
        project_name = wxFileNameFromPath(path);
        if (tabs)
        {
            tabs->CloseAll();
        }
        files_tree->Update();

        wxConfig *config = new wxConfig("ThunderCode");
        config->Write("workspace", project_path);
        delete config;

        SetTitle("ThunderCode - " + project_name);
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
        files_tree->OpenFile(path);
    }
}

void MainFrame::OnHiddeFilesTree(wxCommandEvent &WXUNUSED(event))
{
    if (main_splitter->IsSplit())
    {
        main_splitter->Unsplit(side_container);
    }
    else
    {
        main_splitter->SplitVertically(side_container, servical_container, 1);
    }
}

void MainFrame::OnHiddeSideNav(wxCommandEvent &WXUNUSED(event))
{
    if (side_navigation)
    {
        if (side_navigation->IsShown())
        {
            side_navigation->Hide();
        }
        else
            side_navigation->Show();

        main_container->GetSizer()->Layout();
        main_container->Update();
    }
}

void MainFrame::OnHiddeMenuBar(wxCommandEvent &WXUNUSED(event))
{
    if (menu_bar)
    {
        if (menu_bar->IsShown())
        {
            menu_bar->Hide();
        }
        else
            menu_bar->Show();

        json user_config = UserConfig().Get();
        auto is_visible = user_config["show_menu"];

        if (is_visible)
        {
            user_config["show_menu"] = false;
        }
        else
            user_config["show_menu"] = true;
        UserConfig().Update(user_config);
    }
}

void MainFrame::OnHiddeStatusBar(wxCommandEvent &WXUNUSED(event))
{
    if (status_bar)
    {
        if (status_bar->IsShown())
        {
            status_bar->Hide();
        }
        else
            status_bar->Show();

        GetSizer()->Layout();
        Update();
    }
}

void MainFrame::OnHiddeTabs(wxCommandEvent &WXUNUSED(event))
{
    if (tabs)
    {
        if (tabs->IsShown())
        {
            tabs->Hide();
        }
        else
            tabs->Show();
        main_code->GetSizer()->Layout();
        main_code->Update();
    }
}

void MainFrame::OnSashPaint(wxPaintEvent &event)
{
    auto target = ((wxSplitterWindow *)event.GetEventObject());
    if (!target) return;

    auto border_color = Themes["dark"]["borderColor"].template get<std::string>();

    wxClientDC this_dc(target);
    if (target->GetId() == ID_SERVICAL_CONTAINER)
    {
        this_dc.SetBrush(target->GetBackgroundColour());
    }
    else
        this_dc.SetBrush(target->GetBackgroundColour());
    this_dc.SetPen(target->GetBackgroundColour());

    if (target->GetSplitMode() == wxSPLIT_VERTICAL)
    {
        this_dc.DrawRectangle(
            target->GetSashPosition(),
            0,
            target->GetSashSize(),
            target->GetSize().GetHeight());
    }
    else
    {
        this_dc.DrawRectangle(
            0,
            target->GetSashPosition(),
            target->GetSize().GetWidth(),
            target->GetSashSize());
    }

    auto top_content = ((wxPanel*)FindWindowById(ID_FILES_TREE_TOP_CONTENT));
    auto search_files = ((wxPanel*)FindWindowById(ID_SEARCH_FILES));

    if(top_content && search_files) {
        wxPoint top_content_point = top_content->GetPosition();
        wxPoint search_files_point = search_files->GetPosition();

        this_dc.SetBrush(wxColor(border_color));
        this_dc.SetPen(wxColor(border_color));

        this_dc.DrawLine(
            0, 
            top_content_point.x+top_content->GetSize().GetHeight()+5, 
            top_content->GetSize().GetWidth()+target->GetSashSize(), 
            top_content_point.x+top_content->GetSize().GetHeight()+5
        );    

        this_dc.DrawLine(
            0, 
            search_files_point.y+search_files->GetSize().GetHeight(), 
            search_files->GetSize().GetWidth()+target->GetSashSize(), 
            search_files_point.y+search_files->GetSize().GetHeight()
        );     
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
    tabs->CloseAll();
    files_tree->selectedFile->SetBackgroundColour(wxColor(45, 45, 45));
    files_tree->selectedFile = NULL;
}

void MainFrame::ToggleControlPanel(wxCommandEvent &event)
{
    if (FindWindowById(ID_CONTROL_PANEL))
    {
        control_panel->Destroy();
    }
    else
    {
        control_panel = new ControlPanel(this, ID_CONTROL_PANEL);
    }
}

bool MainFrame::LoadPath(wxString path)
{
    project_path = path;
    project_name = wxFileNameFromPath(path.substr(0, path.size() - 1));

    wxConfig *config = new wxConfig("ThunderCode");
    config->Write("workspace", path);
    delete config;

    tabs->CloseAll();
    files_tree->Update();
    SetTitle("ThunderCode - " + project_name);

    AddEntry(wxFSWPath_Tree, path);
    return true;
}

void MainFrame::OnOpenTerminal(wxCommandEvent &event)
{
    if (servical_container->IsSplit())
    {
        servical_container->Unsplit(FindWindowById(ID_TERMINAL));
    }
    else
    {
        servical_container->SplitHorizontally(main_code, FindWindowById(ID_TERMINAL), 0);
    }
}

void MainFrame::ToggleFind(wxCommandEvent &event)
{
    if (FindWindowById(ID_FIND_CONTAINER))
    {
        ((wxWindow *)FindWindowById(ID_FIND_CONTAINER))->Destroy();
    }
    else
    {
        Find *find_container = new Find(this, "You");
    }
}

void MainFrame::MainComponentsDrawnBorder(wxPaintEvent &event)
{
    return;
    auto target = ((wxWindow *)event.GetEventObject());
    if (target)
    {
        wxClientDC dc(target);
        if (!dc.IsOk())
            return;
        dc.SetPen(wxPen(wxColor(65, 65, 65), 0.20));
        dc.DrawLine(target->GetSize().GetWidth() - 1, 0, target->GetSize().GetWidth() - 1, target->GetSize().GetHeight());
    }
}