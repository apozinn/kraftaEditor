#pragma once

class MenuBar : public wxMenuBar {
public:
	MenuBar() : wxMenuBar() {
		wxMenu* menuFile = new wxMenu;
		menuFile->Append(ID_CREATE_DIR, _("&Create Dir"));
		menuFile->Append(ID_CREATE_FILE, _("&Create File\tCtrl-N"));
		menuFile->Append(ID_OPEN_FOLDER, _("&Open Folder...\tCtrl+Shift+K"));
		menuFile->Append(ID_OPEN_FILE, _("&Open File...\tCtrl+O"));
		menuFile->AppendSeparator();
		menuFile->Append(wxID_SAVE, _("&Save"));
		menuFile->Append(ID_SAVE_AS, _("&Save As..."));
		menuFile->Append(wxID_ANY, _("&Save All"));
		menuFile->AppendSeparator();
		menuFile->Append(wxID_ANY, _("&New Window"));
		menuFile->Append(wxID_ANY, _("&Close Window"));
		menuFile->AppendSeparator();
		menuFile->Append(wxID_ANY, _("&Close File"));
		menuFile->Append(wxID_ANY, _("&Revert File"));
		menuFile->Append(ID_CLOSE_ALL_FILES, _("&Close All Files\tCtrl+Alt+W"));
		menuFile->Append(ID_CLOSE_FOLDER, _("&Close Folder\tCtrl-K+F"));
		menuFile->Append(wxID_EXIT, _("&Quit\tCtrl+Q"));

		wxMenu* menuEdit = new wxMenu;
		menuEdit->Append(wxID_ANY, _("&Undo"));
		menuEdit->Append(wxID_ANY, _("&Undo Selection"));
		menuEdit->AppendSeparator();
		menuEdit->Append(wxID_ANY, _("&Cut"));
		menuEdit->Append(wxID_ANY, _("&Copy"));
		menuEdit->Append(wxID_ANY, _("&Paste"));
		menuEdit->AppendSeparator();

		wxMenu* commentMenu = new wxMenu();
		commentMenu->Append(ID_TOGGLE_COMMENT_LINE, _("&Toggle Comment"));
		commentMenu->Append(ID_TOGGLE_COMMENT_BLOCK, _("&Toggle Block Comment"));
		menuEdit->AppendSubMenu(commentMenu, _("&Comment"));

		wxMenu* menuSelection = new wxMenu;
		menuSelection->Append(wxID_ANY, _("&Select All"));
		menuSelection->Append(wxID_ANY, _("&Expand Selection"));
		menuSelection->AppendSeparator();
		menuSelection->Append(wxID_ANY, _("&Add Previous Line"));
		menuSelection->Append(wxID_ANY, _("&Add Next Line"));

		wxMenu* menuFind = new wxMenu;
		menuFind->Append(wxID_ANY, _("&Find"));
		menuFind->Append(wxID_ANY, _("&Find Next"));
		menuFind->Append(wxID_ANY, _("&Find Previous"));
		menuFile->AppendSeparator();
		menuFind->Append(wxID_ANY, _("&Replace"));
		menuFind->Append(wxID_ANY, _("&Replace Next"));

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
		this->Append(menuFind, _("&Find"));
		this->Append(menuView, _("&View"));
		this->Append(menuTools, _("&Tools"));
		this->Append(menuPreference, _("&Preferences"));
		this->Append(menuHelp, _("&Help"));
	}
	wxDECLARE_NO_COPY_CLASS(MenuBar);
};