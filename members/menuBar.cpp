#pragma once

class MenuBar : public wxMenuBar {
public:
	MenuBar() : wxMenuBar() {
		wxMenu* menuFile = new wxMenu;
		menuFile->Append(ID_CREATE_FILE, _("&New File\tCtrl-N"));
		menuFile->Append(ID_CREATE_DIR, _("&New Dir"));
		menuFile->Append(ID_NEW_WINDOW, _("&New Window\tCtrl+Shift+N"));
		menuFile->AppendSeparator();
		menuFile->Append(ID_OPEN_FOLDER, _("&Open Folder...\tCtrl+Shift+K"));
		menuFile->Append(ID_OPEN_FILE, _("&Open File...\tCtrl+O"));
		menuFile->AppendSeparator();
		menuFile->Append(wxID_SAVE, _("&Save\tCtrl+S"));
		menuFile->Append(ID_SAVE_AS, _("&Save As...\tCtrl+Shift+S"));
		menuFile->Append(ID_SAVE_ALL, _("&Save All\tCtrl+Alt+S"));
		menuFile->AppendSeparator();
		menuFile->Append(ID_CLOSE_FILE, _("&Close File"));
		menuFile->Append(ID_CLOSE_ALL_FILES, _("&Close All Files\tCtrl+Alt+W"));
		menuFile->Append(ID_CLOSE_FOLDER, _("&Close Folder"));
		menuFile->Append(wxID_EXIT, _("&Quit\tCtrl+Q"));

		wxMenu* menuEdit = new wxMenu;
		menuEdit->Append (wxID_UNDO, _("&Undo\tCtrl+Z"));
		menuEdit->Append (wxID_REDO, _("&Redo\tCtrl+Shift+Z"));
		menuEdit->AppendSeparator();
		menuEdit->Append (wxID_CUT, _("Cu&t\tCtrl+X"));
		menuEdit->Append (wxID_COPY, _("&Copy\tCtrl+C"));
		menuEdit->Append (wxID_PASTE, _("&Paste\tCtrl+V"));
		menuEdit->Append (wxID_CLEAR, _("&Delete\tDel"));
		menuEdit->AppendSeparator();
		menuEdit->Append(ID_TOGGLE_COMMENT_LINE, _("&Toggle Comment\tCtrl+;"));
		menuEdit->Append(ID_TOGGLE_COMMENT_BLOCK, _("&Toggle Block Comment\tShift+alt+A"));

		wxMenu* menuSelection = new wxMenu;
		menuSelection->Append(ID_SELECT_ALL, _("&Select All"));
		menuSelection->Append(ID_SELECT_LINE, _("&Select line"));

		wxMenu* menuView = new wxMenu;
		menuView->Append(ID_HIDDE_FILES_TREE, _("&Hidde files-tree"));
		menuView->Append(ID_HIDDE_SIDE_NAV, _("&Hidde Side-Nav"));
		menuView->Append(ID_HIDDE_MENU_BAR, _("&Hidde Menu"));
		menuView->Append(ID_HIDDE_STATUS_BAR, _("&Hidde Status Bar"));
		menuView->Append(ID_HIDDE_TABS, _("&Hidde Tabs"));
		menuView->Append(ID_FOCUS_MODE, _("&Focus Mode\tShift-f11"));
		menuView->Append(ID_TOGGLE_MINI_MAP_VIEW, _("&Hide Minimap"));

		wxMenu* menuTools = new wxMenu;
		menuTools->Append(wxID_ANY, _("&Command Palette"));
		menuTools->Append(wxID_ANY, _("&Snippets"));
		menuTools->AppendSeparator();
		menuTools->Append(wxID_ANY, _("&Build"));
		menuTools->AppendSeparator();
		menuTools->Append(ID_OPEN_TERMINAL, _("&Open Terminal\tCtrl+T"));

		wxMenu* menuPreference = new wxMenu;
		menuPreference->Append(wxID_ANY, _("&Settings"));
		menuPreference->Append(wxID_ANY, _("&Key Bindings"));

		wxMenu* menuHelp = new wxMenu;
		menuHelp->Append(wxID_ANY, _("&Documentation"));
		menuHelp->Append(wxID_ANY, _("&Report a Bug"));
		menuHelp->AppendSeparator();
		menuHelp->Append(wxID_ABOUT, _("&About KraftaEditor"));

		this->Append(menuFile, _("&File"));
		this->Append(menuEdit, _("&Edit"));
		this->Append(menuSelection, _("&Selection"));
		this->Append(menuView, _("&View"));
		this->Append(menuTools, _("&Tools"));
		this->Append(menuPreference, _("&Preferences"));
		this->Append(menuHelp, _("&Help"));
	}
	wxDECLARE_NO_COPY_CLASS(MenuBar);
};