#pragma once
#include "wx/wx.h"
#include <wx/splitter.h>
#include "wx/fswatcher.h"
#include "wx/cmdline.h"
#include <wx/config.h>

#if wxUSE_CLIPBOARD
#include "wx/dataobj.h"
#include "wx/clipbrd.h"
#endif

#ifdef __WXMSW__
#include "wx/msw/private.h"
#endif

#include <string>
#include <iostream>

#include "./app.hpp"
#include "./defs.hpp"
#include "./src/userConfig/userConfig.cpp"

#include "./components/codeContainer/code.cpp"
#include "./components/filesTree/files.cpp"
#include "./components/sideNavigation/side.cpp"
#include "./components/statusBar/status.cpp"
#include "./components/tabs/tabs.cpp"
#include "./members/menuBar.cpp"
#include "./members/emptyWindow.cpp"
#include "./members/openFolderLink.cpp"
#include "./members/controlPanel.cpp"
#include "./members/terminal.cpp"
#include "./members/find.cpp"

class MainFrame : public wxFrame
{
    wxBoxSizer *sizer;
    wxPanel *main_container;
    wxSplitterWindow *main_splitter;
    SideNavigation *side_navigation;
    FilesTree *files_tree;
    wxPanel *main_code;
    Tabs *tabs;
    MenuBar *menu_bar;
    EmptyWindow *empty_window;
    wxPanel *side_container;
    OpenFolderLink *open_folder_link;
    ControlPanel *control_panel;
    Terminal *terminal;
    wxSplitterWindow *servical_container;

public:
    StatusBar *status_bar;
    MainFrame(const wxString &title);
    virtual ~MainFrame();
    void AddEntry(wxFSWPathType type, wxString filename = wxString());
    bool CreateWatcherIfNecessary();
    void OnOpenFolderMenu(wxCommandEvent &event) { OpenFolderDialog(); }
    void OnOpenFolderClick(wxMouseEvent &event) { OpenFolderDialog(); }
    void OnOpenFile(wxCommandEvent &event);
    void OnHiddeFilesTree(wxCommandEvent &event);
    void OnHiddeSideNav(wxCommandEvent &event);
    void OnHiddeMenuBar(wxCommandEvent &event);
    void OnHiddeStatusBar(wxCommandEvent &event);
    void OnHiddeTabs(wxCommandEvent &event);
    void OnSashPaint(wxPaintEvent &event);
    void OnSashPosChange(wxSplitterEvent &event);
    void CloseAllFiles(wxCommandEvent &event);
    void OpenFolderDialog();
    void ToggleControlPanel(wxCommandEvent &event);
    bool LoadPath(wxString path);
    void ToggleFind(wxCommandEvent& event);
    void GotoSearchPage(wxCommandEvent& event);
private:
    void CreateWatcher();
    void OnQuit(wxCommandEvent &WXUNUSED(event)) { Close(true); }
    void OnWatch(wxCommandEvent &event);
    void OnFollowLinks(wxCommandEvent &event);
    void OnAbout(wxCommandEvent &event);
    void OnFileSystemEvent(wxFileSystemWatcherEvent &event);
    void OnOpenTerminal(wxCommandEvent &event);
    wxFileSystemWatcher *m_watcher = nullptr;
    bool m_followLinks;
    wxDECLARE_NO_COPY_CLASS(MainFrame);
    wxDECLARE_EVENT_TABLE();
};

class ThunderCode : public wxApp
{
public:
    MainFrame *frame;
    wxString m_dirToWatch;
    virtual bool OnInit() override
    {
        if (!wxApp::OnInit())
            return false;
        frame = new MainFrame("ThunderCode");
        frame->Show();
        wxApp::SetTopWindow(frame);
        return true;
    }

    virtual void OnEventLoopEnter(wxEventLoopBase *WXUNUSED(loop)) override
    {
        if (frame->CreateWatcherIfNecessary())
        {
            wxConfig *config = new wxConfig("ThunderCode");
            wxString str;

            if (!m_dirToWatch.empty())
            {
                frame->LoadPath(m_dirToWatch);
            }
            else
            {
                if (config->Read("workspace", &str))
                {
                    wxString last_workspace = config->Read("workspace", str);
                    frame->LoadPath(last_workspace);
                }
            }
        }
    }

    virtual void OnInitCmdLine(wxCmdLineParser &parser) override
    {
        wxApp::OnInitCmdLine(parser);
        parser.AddParam("directory to watch", wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL);
    }

    virtual bool OnCmdLineParsed(wxCmdLineParser &parser) override
    {
        if (!wxApp::OnCmdLineParsed(parser))
            return false;
        if (parser.GetParamCount())
            m_dirToWatch = parser.GetParam();
        return true;
    }
};

wxIMPLEMENT_APP(ThunderCode);
wxDECLARE_APP(ThunderCode);

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_MENU(wxID_EXIT, MainFrame::OnQuit)
    EVT_MENU(wxID_ABOUT, MainFrame::OnAbout)
    EVT_MENU(wxID_SAVE, CodeContainer::OnSave)
    EVT_MENU(ID_CREATE_FILE, FilesTree::OnCreateFile)
    EVT_MENU(ID_OPEN_FOLDER, MainFrame::OnOpenFolderMenu)
    EVT_MENU(ID_OPEN_FILE, MainFrame::OnOpenFile)
    EVT_MENU(ID_HIDDE_FILES_TREE, MainFrame::OnHiddeFilesTree)
    EVT_MENU(ID_HIDDE_SIDE_NAV, MainFrame::OnHiddeSideNav)
    EVT_MENU(ID_HIDDE_MENU_BAR, MainFrame::OnHiddeMenuBar)
    EVT_MENU(ID_HIDDE_STATUS_BAR, MainFrame::OnHiddeStatusBar)
    EVT_MENU(ID_HIDDE_TABS, MainFrame::OnHiddeTabs)
    EVT_MENU(ID_FOCUS_MODE, SideNavigation::OnFocusMode)
    EVT_MENU(ID_CLOSE_ALL_FILES, MainFrame::CloseAllFiles)
    EVT_MENU(ID_TOGGLE_CONTROL_PANEL, MainFrame::ToggleControlPanel)
    EVT_MENU(ID_OPEN_TERMINAL, MainFrame::OnOpenTerminal)
    EVT_MENU(ID_TOGGLE_COMMENT_LINE, CodeContainer::ToggleCommentLine)
    EVT_MENU(ID_TOGGLE_MINI_MAP_VIEW, CodeContainer::ToggleMiniMapView)
    EVT_MENU(ID_TOGGLE_FIND, MainFrame::ToggleFind)
wxEND_EVENT_TABLE()