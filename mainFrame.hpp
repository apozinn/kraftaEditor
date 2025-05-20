#pragma once

#include <wx/wx.h>
#include <wx/splitter.h>
#include <wx/fswatcher.h>
#include <wx/cmdline.h>
#include <wx/config.h>
#include <wx/settings.h>
#include <wx/platinfo.h>

#if wxUSE_CLIPBOARD
#include <wx/dataobj.h>
#include <wx/clipbrd.h>
#endif

#include <string>
#include <iostream>

#include "./src/userConfig/userConfig.cpp"
#include "./defs.hpp"
#include "./app.hpp"

#include "./components/codeContainer/code.cpp"
#include "./components/filesTree/files.cpp"
#include "./components/statusBar/status.cpp"
#include "./components/tabs/tabs.cpp"
#include "./components/search/search.cpp"
#include "./members/menuBar.cpp"
#include "./members/emptyWindow.cpp"
#include "./members/openFolderButton.cpp"
#include "./members/controlPanel.cpp"
#include "./members/terminal.cpp"
#include "./utils/createAssetsDir.hpp"

#if __WXMSW__
#include <wx/msw/private.h>
#include <wx/msw/darkmode.h>
#endif

class MainFrame : public wxFrame
{
    wxBoxSizer *sizer;
    FilesTree *filesTree;
    Tabs *tabs;
    MenuBar *menuBar;
    EmptyWindow *emptyWindow;
    OpenFolderButton *openFolderButton;
    ControlPanel *controlPanel;
    Terminal *terminal;
    wxSplitterWindow *servicalContainer;
    wxSplitterWindow *mainSplitter;
    wxPanel *navigationContainer;
    wxPanel *mainContainer;
    wxPanel *mainCode;
    wxPanel *applicationContent;

public:
    StatusBar *statusBar;
    MainFrame(const wxString &title);
    virtual ~MainFrame();
    void AddEntry(wxFSWPathType type, wxString filename = wxString());
    bool CreateWatcherIfNecessary();
    void OnOpenFolderMenu(wxCommandEvent &event) { OpenFolderDialog(); }
    void OnOpenFolderClick(wxMouseEvent &event) { OpenFolderDialog(); }
    void OnOpenFile(wxCommandEvent &event);
    void PaintSash(wxDC &dc, wxSplitterWindow *target);
    void OnSashPosChange(wxSplitterEvent &event);
    void CloseAllFiles(wxCommandEvent &event);
    void OpenFolderDialog();
    void LoadPath(wxString path);
    void OnPaintedComponent(wxPaintEvent &event);
    void OnCloseFolder(wxCommandEvent &event);
    void OnNewWindow(wxCommandEvent &event);

    void OnToggleSearch(wxCommandEvent &event);
    void OnToggleControlPanel(wxCommandEvent &event);
    void OnToggleFileTreeView(wxCommandEvent &WXUNUSED(event));
    void OnToggleMenuBarView(wxCommandEvent &WXUNUSED(event));
    void OnToggleStatusBarView(wxCommandEvent &WXUNUSED(event));
    void OnToggleTabBarView(wxCommandEvent &WXUNUSED(event));
    void OnToggleMinimapView(wxCommandEvent &WXUNUSED(event));

private:
    void CreateWatcher();
    void OnQuit(wxCommandEvent &WXUNUSED(event)) { Close(true); }
    void OnWatch(wxCommandEvent &event);
    void OnFollowLinks(wxCommandEvent &event);
    void OnAbout(wxCommandEvent &event);
    void OnFileSystemEvent(wxFileSystemWatcherEvent &event);
    void OnOpenTerminal(wxCommandEvent &event);
    bool m_followLinks;
    wxFileSystemWatcher *m_watcher = nullptr;
    wxDECLARE_NO_COPY_CLASS(MainFrame);
    wxDECLARE_EVENT_TABLE();
};
