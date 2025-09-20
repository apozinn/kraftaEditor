#pragma once

#include <string>
#include <iostream>
#include <filesystem>

#include <wx/wx.h>
#include <wx/splitter.h>
#include <wx/fswatcher.h>
#include <wx/cmdline.h>
#include <wx/config.h>
#include <wx/settings.h>
#include <wx/platinfo.h>
#include <wx/dirdlg.h>
#include <wx/sashwin.h>
#include <wx/image.h>
#include <wx/file.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>

#if wxUSE_CLIPBOARD
#include <wx/dataobj.h>
#include <wx/clipbrd.h>
#endif

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "appPaths/appPaths.hpp"
#include "userSettings/userSettings.hpp"
#include "themesManager/themesManager.hpp"
#include "ui/ids.hpp"
#include "platformInfos/platformInfos.hpp"
#include "projectSettings/projectSettings.hpp"

#include "gui/widgets/menuBar/menuBar.hpp"
#include "gui/panels/filesTree/filesTree.hpp"
#include "gui/codeContainer/code.hpp"
#include "gui/codeContainer/codeSearch/codeSearch.hpp"
#include "gui/panels/tabs/tabs.hpp"
#include "gui/widgets/emptyWindow/emptyWindow.hpp"
#include "gui/panels/terminal/terminal.hpp"
#include "gui/widgets/statusBar/statusBar.hpp"
#include "gui/widgets/openFolderButton/openFolderButton.hpp"
#include "gui/panels/controlPanel/controlPanel.hpp"

class MainFrame : public wxFrame
{
public:
    MainFrame(const wxString &title="Krafta Editor");
    virtual ~MainFrame();

    void AddEntry(wxFSWPathType type, wxString filename = wxString());
    bool CreateWatcherIfNecessary();
    void OnOpenFolderMenu(wxCommandEvent &WXUNUSED(event));
    void OnOpenFolderClick(wxMouseEvent &WXUNUSED(event));
    void OnOpenFile(wxCommandEvent &WXUNUSED(event));
    void PaintSash(wxDC &dc, wxSplitterWindow *target);
    void OnSashPosChange(wxSplitterEvent &event);
    void CloseAllFiles(wxCommandEvent &WXUNUSED(event));
    void OpenFolderDialog();
    void LoadPath(wxString path);
    void OnPaintedComponent(wxPaintEvent &event);
    void OnCloseFolder(wxCommandEvent &WXUNUSED(event));
    void OnNewWindow(wxCommandEvent &WXUNUSED(event));
    void OnToggleSearch(wxCommandEvent &WXUNUSED(event));
    void OnToggleControlPanel(wxCommandEvent &WXUNUSED(event));
    void OnToggleFileTreeView(wxCommandEvent &WXUNUSED(event));
    void OnToggleMenuBarView(wxCommandEvent &WXUNUSED(event));
    void OnToggleStatusBarView(wxCommandEvent &WXUNUSED(event));
    void OnToggleTabBarView(wxCommandEvent &WXUNUSED(event));
    void OnEditSettings(wxCommandEvent &WXUNUSED(event));

private:
    MenuBar *m_menuBar;
    FilesTree *m_filesTree;
    Tabs *m_tabs;
    EmptyWindow *m_emptyWindow;
    Terminal *m_terminal;
    StatusBar *m_statusBar;
    OpenFolderButton *m_openFolderButton;
    ControlPanel *m_controlPanel;
    wxSplitterWindow *m_mainSplitter;
    wxPanel *m_applicationLeftMainContainer;
    wxPanel *m_applicationRightMainContainer;
    wxSplitterWindow *m_mainContainerSplitter;
    wxPanel *m_mainContainer;
    wxPanel *m_centeredContent;
    wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
    wxFileSystemWatcher *m_watcher = nullptr;
    bool m_followLinks;
    
    ProjectSettings& projectSettings = ProjectSettings::Get();
    json UserSettings = UserSettingsManager::Get().currentSettings;
    json Theme = ThemesManager::Get().currentTheme;
    wxString assetsImagePath = ApplicationPaths::AssetsPath("images");
    wxString appPath = ApplicationPaths::ApplicationPath();

    void CreateWatcher();
    void OnClose(wxCloseEvent& (event));
    void OnExit(wxCommandEvent& WXUNUSED(event));
    void OnWatch(wxCommandEvent &event);
    void OnFollowLinks(wxCommandEvent &event);
    void OnAbout(wxCommandEvent &WXUNUSED(event));
    void OnFileSystemEvent(wxFileSystemWatcherEvent &event);
    void OnOpenTerminal(wxCommandEvent &WXUNUSED(event));
    void OnFrameResized(wxSizeEvent &event);
    void OnFrameMaximized(wxMaximizeEvent &WXUNUSED(event));
    bool SetAppIcon();
    void SetupMenuBar();
    void SetupMainSplitter();
    void SetupApplicationLeftMainContainer();
    void SetupFilesTree();
    void SetupApplicationRightMainContainer();
    void SetupMainContainerSplitter();
    void SetupCenteredContent();
    void SetupMainContainer();
    void SetupTabs();
    void SetupEmptyWindow();
    void SetupTerminal();
    void SetupStatusBar();
    void SetupLastWordspace();
    void SetupAccelerators();
    void WindowResizeFunctions();

    wxDECLARE_NO_COPY_CLASS(MainFrame);
    wxDECLARE_EVENT_TABLE();
};