#include "gui/editor/controls/textCtrl/textCtrl.hpp"
#include "mainFrame.hpp"

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
	// // File Operations
	EVT_MENU(+Event::File::CreateFileEvent, FilesTree::OnCreateFileRequested)
		EVT_MENU(+Event::File::CreateDir, FilesTree::OnCreateDirRequested)
			EVT_MENU(+Event::File::Save, TextCtrl::OnSave)
				EVT_MENU(+Event::File::SaveAs, TextCtrl::SaveAs)
					EVT_MENU(+Event::File::SaveAll, TextCtrl::SaveAll)
						EVT_MENU(+Event::File::CloseFile, TextCtrl::CloseFile)
							EVT_MENU(+Event::File::CloseAll,
									 MainFrame::CloseAllFiles)
								EVT_MENU(+Event::File::OpenFile,
										 MainFrame::OnOpenFile)

									EVT_MENU(+Event::File::ToggleAutosave,
											 MainFrame::ToggleAutosave)

	// Edit Operations
	EVT_MENU(+Event::Edit::Cut, TextCtrl::DoCut) EVT_MENU(+Event::Edit::Copy,
														  TextCtrl::DoCopy)
		EVT_MENU(+Event::Edit::Paste,
				 TextCtrl::DoPaste) EVT_MENU(+Event::Edit::Redo,
											 TextCtrl::DoRedo)
			EVT_MENU(+Event::Edit::Undo,
					 TextCtrl::DoUndo) EVT_MENU(+Event::Edit::ToggleLineComment,
												TextCtrl::ToggleLineComment)
				EVT_MENU(+Event::Edit::ToggleBlockComment,
						 TextCtrl::ToggleBlockComment)
					EVT_MENU(+Event::Edit::SelectLine, TextCtrl::SelectLine)
						EVT_MENU(+Event::Edit::SelectAll, TextCtrl::DoSelectAll)
							EVT_MENU(+Event::Edit::DuplicateLineDown,
									 TextCtrl::DuplicateLine)
								EVT_MENU(+Event::Edit::MoveLineUp,
										 TextCtrl::MoveLineUp)
									EVT_MENU(+Event::Edit::MoveLineDown,
											 TextCtrl::MoveLineDown)
										EVT_MENU(
											+Event::Edit::RemoveCurrentLine,
											TextCtrl::RemoveCurrentLine)

	// View Operations
	EVT_MENU(+Event::View::ToggleMiniMap, MainFrame::OnToggleMinimapView)
		EVT_MENU(+Event::View::ToggleCodeSearch, MainFrame::OnToggleSearch)
			EVT_MENU(+Event::View::ToggleControlPanel,
					 MainFrame::OnToggleControlPanel)
				EVT_MENU(+Event::View::ToggleQuickOpen,
						 MainFrame::OnToggleQuickOpen)
					EVT_MENU(+Event::View::ToggleFileTree,
							 MainFrame::OnToggleFileTreeView)
						EVT_MENU(+Event::View::ToggleMenuBar,
								 MainFrame::OnToggleMenuBarView)
							EVT_MENU(+Event::View::ToggleStatusBar,
									 MainFrame::OnToggleStatusBarView)
								EVT_MENU(+Event::View::ToggleTabBar,
										 MainFrame::OnToggleTabBarView)

									EVT_MENU(+Event::View::ZoomIn,
											 TextCtrl::DoZoomIn)
										EVT_MENU(+Event::View::ZoomOut,
												 TextCtrl::DoZoomOut)

	// Project Operations
	EVT_MENU(+Event::Project::OpenFolder, MainFrame::OnOpenFolderMenu)
		EVT_MENU(+Event::Project::CloseFolder, MainFrame::OnCloseFolder)

	// Frame Operations
	EVT_MENU(+Event::Frame::NewWindow, MainFrame::OnNewWindow)
		EVT_MENU(+Event::Frame::Exit, MainFrame::OnExit)
			EVT_MENU(+Event::Frame::About, MainFrame::OnAbout)
				EVT_CLOSE(MainFrame::OnClose)

					EVT_MENU(+Event::Edit::GoToLine, MainFrame::OnGotoline)

	// Terminal
	EVT_MENU(+Event::Terminal::Open, MainFrame::OnOpenTerminal)

	// Settings
	EVT_MENU(+Event::UserSettings::Edit, MainFrame::OnEditSettings)

	// Keyboard Shortcuts
	EVT_MENU(+Event::Shortcuts::Edit, MainFrame::OnEditShortcuts)

	// Code Search
	EVT_MENU(+Event::CodeSearch::OpenCodeSearchTab, MainFrame::OpenCodeSearch)

		wxEND_EVENT_TABLE()
