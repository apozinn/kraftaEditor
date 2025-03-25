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
#include "./members/menuBar.cpp"
#include "./members/emptyWindow.cpp"
#include "./members/openFolderLink.cpp"
#include "./members/controlPanel.cpp"
#include "./members/terminal.cpp"
#include "./members/find.cpp"
#include "./utils/createAssetsDir.hpp"

#if __WXMSW__
#include <wx/msw/private.h>
#include <wx/msw/darkmode.h>
#endif

class MainFrame : public wxFrame
{
	wxBoxSizer *sizer;
	wxPanel *main_container;
	wxSplitterWindow *main_splitter;
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
	wxSplitterWindow *mainSplitter;
	wxPanel *navigationContainer;
	wxPanel *mainContainer;
	wxPanel *mainCode;

public:
	StatusBar *statusBar;
	MainFrame(const wxString &title);
	virtual ~MainFrame();
	void AddEntry(wxFSWPathType type, wxString filename = wxString());
	bool CreateWatcherIfNecessary();
	void OnOpenFolderMenu(wxCommandEvent &event) { OpenFolderDialog(); }
	void OnOpenFolderClick(wxMouseEvent &event) { OpenFolderDialog(); }
	void OnOpenFile(wxCommandEvent &event);
	void OnHiddeFilesTree(wxCommandEvent &event);
	void OnHiddeMenuBar(wxCommandEvent &event);
	void OnHiddeStatusBar(wxCommandEvent &event);
	void OnHiddeTabs(wxCommandEvent &event);
	void PaintSash(wxDC &dc, wxSplitterWindow *target);
	void OnSashPosChange(wxSplitterEvent &event);
	void CloseAllFiles(wxCommandEvent &event);
	void OpenFolderDialog();
	void ToggleControlPanel(wxCommandEvent &event);
	void LoadPath(wxString path);
	void ToggleFind(wxCommandEvent &event);
	void OnPaintedComponent(wxPaintEvent &event);

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

class KraftaEditor : public wxApp
{
public:
	MainFrame *frame;
	wxString m_dirToWatch;
	virtual bool OnInit() override
	{
		if (!wxApp::OnInit())
			return false;

		// getting the OS name
		osName = wxPlatformInfo::Get().GetOperatingSystemFamilyName();

		// verify if system theme is dark
		auto systemInfo = wxSystemSettings::GetAppearance();
		if (systemInfo.IsSystemDark())
		{
			SetAppearance(Appearance::Dark);
			if (osName == "windows")
				MSWEnableDarkMode(DarkMode_Always);
		}
		// Get user config and theme
		UserConfigs = UserConfig().Get();
		UserTheme = UserConfig().GetThemes();

		// define the app dirs path
		if (osName == "Windows")
		{
			osSlash="\\";
			applicationPath = wxStandardPaths::Get()
								  .GetUserConfigDir()
								  .ToStdString() +
							  "\\.kraftaEditor\\";

			icons_dir = applicationPath + "icons\\";
			assetsDir = applicationPath + "assets\\";
		}
		else
		{
			osSlash="/";
			applicationPath = wxStandardPaths::Get()
								  .GetUserConfigDir()
								  .ToStdString() +
							  "/.kraftaEditor/";

			icons_dir = applicationPath + "icons/";
			assetsDir = applicationPath + "assets/";
		}

		if (!wxDirExists(icons_dir))
		{
			if (!CreateApplicationAssetsDirectories(GetAppDirs("icons").ToStdString(), "icons"))
			{
				return false;
			}
			else
			{
				if (!CreateApplicationAssetsDirectories(GetAppDirs("icons").ToStdString() + "file_ext/", "file_ext"))
				{
					return false;
				}
			}
		}

		if (!wxDirExists(assetsDir))
		{
			if (!CreateApplicationAssetsDirectories(GetAppDirs("assets").ToStdString(), "assets"))
			{
				return false;
			}
		}

		// init images handlers
		wxInitAllImageHandlers();

		// createv the main frame
		frame = new MainFrame("Krafta Editor");
		frame->Show();
		wxApp::SetTopWindow(frame);
		mainFrame = frame;
		return true;
	}

	virtual void OnEventLoopEnter(wxEventLoopBase *WXUNUSED(loop)) override
	{
		if (frame->CreateWatcherIfNecessary())
		{
			wxConfig *config = new wxConfig("krafta-editor");
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

wxIMPLEMENT_APP(KraftaEditor);
wxDECLARE_APP(KraftaEditor);

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
	EVT_MENU(wxID_EXIT, MainFrame::OnQuit)
		EVT_MENU(wxID_ABOUT, MainFrame::OnAbout)
			EVT_MENU(wxID_SAVE, CodeContainer::OnSave)
				EVT_MENU(ID_CREATE_DIR, FilesTree::OnCreateDir)
					EVT_MENU(ID_CREATE_FILE, FilesTree::OnCreateFile)
						EVT_MENU(ID_OPEN_FOLDER, MainFrame::OnOpenFolderMenu)
							EVT_MENU(ID_OPEN_FILE, MainFrame::OnOpenFile)
								EVT_MENU(ID_HIDDE_FILES_TREE, MainFrame::OnHiddeFilesTree)
									EVT_MENU(ID_HIDDE_MENU_BAR, MainFrame::OnHiddeMenuBar)
										EVT_MENU(ID_HIDDE_STATUS_BAR, MainFrame::OnHiddeStatusBar)
											EVT_MENU(ID_HIDDE_TABS, MainFrame::OnHiddeTabs)
												EVT_MENU(ID_CLOSE_ALL_FILES, MainFrame::CloseAllFiles)
													EVT_MENU(ID_TOGGLE_CONTROL_PANEL, MainFrame::ToggleControlPanel)
														EVT_MENU(ID_OPEN_TERMINAL, MainFrame::OnOpenTerminal)
															EVT_MENU(ID_TOGGLE_COMMENT_LINE, CodeContainer::ToggleCommentLine)
																EVT_MENU(ID_TOGGLE_MINI_MAP_VIEW, CodeContainer::ToggleMiniMapView)
																	EVT_MENU(ID_TOGGLE_FIND, MainFrame::ToggleFind)
																		wxEND_EVENT_TABLE()