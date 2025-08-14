#include "menuBar.hpp"

MenuBar::MenuBar() {
	wxMenu* menuFile = new wxMenu;
		menuFile->Append(+Event::File::CreateFileEvent, _("&New File\tCtrl-N"));
		menuFile->Append(+Event::File::CreateDir, _("&New Dir"));
		menuFile->Append(+Event::Frame::NewWindow, _("&New Window\tCtrl+Shift+N"));
		menuFile->AppendSeparator();
		menuFile->Append(+Event::Project::OpenFolder, _("&Open Folder...\tCtrl+Shift+K"));
		menuFile->Append(+Event::File::OpenFile, _("&Open File...\tCtrl+O"));
		menuFile->AppendSeparator();
		menuFile->Append(+Event::File::Save, _("&Save\tCtrl+S"));
		menuFile->Append(+Event::File::SaveAs, _("&Save As...\tCtrl+Shift+S"));
		menuFile->Append(+Event::File::SaveAll, _("&Save All\tCtrl+Alt+S"));
		menuFile->AppendSeparator();
		menuFile->Append(+Event::File::CloseFile, _("&Close File"));
		menuFile->Append(+Event::File::CloseAll, _("&Close All Files\tCtrl+Alt+W"));
		menuFile->Append(+Event::Project::CloseFolder, _("&Close Folder"));
		menuFile->Append(+Event::Frame::Exit, _("&Quit\tCtrl+Q"));

		wxMenu* menuEdit = new wxMenu;
		menuEdit->Append (wxID_UNDO, _("&Undo\tCtrl+Z"));
		menuEdit->Append (wxID_REDO, _("&Redo\tCtrl+Shift+Z"));
		menuEdit->AppendSeparator();
		menuEdit->Append (wxID_CUT, _("&Cut\tCtrl+X"));
		menuEdit->Append (wxID_COPY, _("&Copy\tCtrl+C"));
		menuEdit->Append (wxID_PASTE, _("&Paste\tCtrl+V"));
		menuEdit->AppendSeparator();
		menuEdit->Append(+Event::Edit::ToggleComment, _("&Toggle Comment\tCtrl+;"));
		menuEdit->Append(+Event::Edit::ToggleBlockComment, _("&Toggle Block Comment\tShift+alt+A"));
  
		wxMenu* menuSelection = new wxMenu;
		menuSelection->Append(+Event::Edit::SelectAll, _("&Select All") );
		menuSelection->Append(+Event::Edit::SelectLine, _("&Select line"));

		wxMenu* menuView = new wxMenu;
		
		wxMenuItem* toggleFileTreeView = new wxMenuItem(menuView, +Event::View::ToggleFileTree, "File Tree", wxEmptyString, wxITEM_CHECK );
		menuView->Append(toggleFileTreeView);
		toggleFileTreeView->Check(true);

		wxMenuItem* toggleMenuBarView = new wxMenuItem(menuView, +Event::View::ToggleMenuBar, "Menu Bar", wxEmptyString, wxITEM_CHECK );
		menuView->Append(toggleMenuBarView);
		toggleMenuBarView->Check(true);

		wxMenuItem* toggleStatusBarView = new wxMenuItem(menuView, +Event::View::ToggleStatusBar, "Status Bar", wxEmptyString, wxITEM_CHECK );
		menuView->Append(toggleStatusBarView);
		toggleStatusBarView->Check(true);

		wxMenuItem* toggleTabBarView = new wxMenuItem(menuView, +Event::View::ToggleTabBar, "Tab bar", wxEmptyString, wxITEM_CHECK );
		menuView->Append(toggleTabBarView);
		toggleTabBarView->Check(true);

		wxMenuItem* toggleMinimapView = new wxMenuItem(menuView, +Event::View::ToggleMiniMap, "Minimap", wxEmptyString, wxITEM_CHECK );
		menuView->Append(toggleMinimapView);
		toggleMinimapView->Check(true);

		wxMenu* menuTools = new wxMenu;
		menuTools->Append(+Event::View::ToggleControlPanel, _("&Command Palette\tCtrl+Shift+P"));
		menuTools->Append(+Event::Terminal::Open, _("&Open Terminal\tCtrl+J"));

		wxMenu* menuPreference = new wxMenu;
		menuPreference->Append(+Event::UserSettings::Edit, _("&Settings"));

		wxMenu* menuHelp = new wxMenu;
		menuHelp->Append(+Event::Frame::About, _("&About Krafta Editor"));

		this->Append(menuFile, _("&File"));
		this->Append(menuEdit, _("&Edit"));
		this->Append(menuSelection, _("&Selection"));
		this->Append(menuView, _("&View"));
		this->Append(menuTools, _("&Tools"));
		this->Append(menuPreference, _("&Preferences"));
		this->Append(menuHelp, _("&Help"));
}