#include "mainFrame.hpp"

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
    // File Operations
    EVT_MENU(+Event::File::CreateFileEvent, FilesTree::OnCreateFileRequested)
        EVT_MENU(+Event::File::CreateDir, FilesTree::OnCreateDirRequested)
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