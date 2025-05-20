#pragma once

#include "mainFrame.hpp"

class KraftaEditor : public wxApp
{
public:
	MainFrame *frame;
	wxString m_dirToWatch;
	void CreateNewWindow();
	virtual bool OnInit() override;
	virtual void OnEventLoopEnter(wxEventLoopBase *WXUNUSED(loop)) override;
	virtual void OnInitCmdLine(wxCmdLineParser &parser) override;
	virtual bool OnCmdLineParsed(wxCmdLineParser &parser) override;
};

wxIMPLEMENT_APP(KraftaEditor);
wxDECLARE_APP(KraftaEditor);

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
EVT_MENU(ID_CREATE_FILE, FilesTree::OnCreateFile)
EVT_MENU(ID_CREATE_DIR, FilesTree::OnCreateDir)
EVT_MENU(ID_NEW_WINDOW, MainFrame::OnNewWindow)
EVT_MENU(ID_OPEN_FOLDER, MainFrame::OnOpenFolderMenu)
EVT_MENU(ID_OPEN_FILE, MainFrame::OnOpenFile)									
EVT_MENU(wxID_SAVE, CodeContainer::OnSave)
EVT_MENU(ID_SAVE_AS, CodeContainer::OnSaveAs)
EVT_MENU(ID_SAVE_ALL, CodeContainer::OnSaveAll)
EVT_MENU(ID_CLOSE_FILE, CodeContainer::OnCloseFile)
EVT_MENU(ID_CLOSE_ALL_FILES, MainFrame::CloseAllFiles)
EVT_MENU(ID_CLOSE_FOLDER, MainFrame::OnCloseFolder)
EVT_MENU(wxID_EXIT, MainFrame::OnQuit)

EVT_MENU(wxID_ABOUT, MainFrame::OnAbout)

EVT_MENU(ID_TOGGLE_FIND, MainFrame::OnToggleSearch)
EVT_MENU(ID_TOGGLE_CONTROL_PANEL, MainFrame::OnToggleControlPanel)
EVT_MENU(ID_OPEN_TERMINAL, MainFrame::OnOpenTerminal)
EVT_MENU(ID_TOGGLE_FILE_TREE_VIEW, MainFrame::OnToggleFileTreeView)
EVT_MENU(ID_TOGGLE_MENU_BAR_VIEW, MainFrame::OnToggleMenuBarView)
EVT_MENU(ID_TOGGLE_STATUS_BAR_VIEW, MainFrame::OnToggleStatusBarView)
EVT_MENU(ID_TOGGLE_TAB_BAR_VIEW, MainFrame::OnToggleTabBarView)
EVT_MENU(ID_TOGGLE_MINI_MAP_VIEW, CodeContainer::OnToggleMinimapView)

EVT_MENU (wxID_CLEAR, CodeContainer::OnClear)
EVT_MENU (wxID_CUT, CodeContainer::OnCut)
EVT_MENU (wxID_COPY,CodeContainer::OnCopy)
EVT_MENU (wxID_PASTE, CodeContainer::OnPaste)
EVT_MENU (wxID_REDO, CodeContainer::OnRedo)
EVT_MENU (wxID_UNDO, CodeContainer::OnUndo)
EVT_MENU(ID_TOGGLE_COMMENT_LINE, CodeContainer::ToggleCommentLine)
EVT_MENU(ID_TOGGLE_COMMENT_BLOCK, CodeContainer::ToggleCommentBlock)
EVT_MENU (ID_SELECT_ALL, CodeContainer::OnSelectAll)
EVT_MENU (ID_SELECT_LINE, CodeContainer::OnSelectLine)
wxEND_EVENT_TABLE()