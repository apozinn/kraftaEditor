#include "menuBar.hpp"
#include "shortcutsSettings/shortcutsSettings.hpp"
#include "app/frames/mainFrame.hpp"

MenuBar::MenuBar()
{
	auto &sc = ShortCutSettingsManager::Get();

	auto GetSC = [&](const std::string &key) -> wxString
	{
		auto s = sc.GetSetting<std::string>(key);
		return s.found ? wxString("\t") + s.value : wxString("");
	};

	wxMenu *menuFile = new wxMenu;
	menuFile->Append(+Event::File::CreateFileEvent, _("&New File") + GetSC("shortcut_new_file"));
	menuFile->Append(+Event::File::CreateDir, _("&New Dir"));
	menuFile->AppendSeparator();
	menuFile->Append(+Event::Project::OpenFolder, _("&Open Folder...") + GetSC("shortcut_open_folder"));
	menuFile->Append(+Event::File::OpenFile, _("&Open File...") + GetSC("shortcut_open_file"));

	auto projectFilesContainer = wxWindow::FindWindowById(+GUI::ControlID::ProjectFilesContainer);
	auto mainFrame = (MainFrame *)wxGetTopLevelParent(projectFilesContainer);

	mainFrame->UpdateRecentWorkspacesMenu(recentsWorkspacesMenu);
	menuFile->AppendSubMenu(recentsWorkspacesMenu, "Recent Workspaces");

	menuFile->Bind(wxEVT_MENU_OPEN, [this, mainFrame](wxMenuEvent &std_event)
				   {
    mainFrame->UpdateRecentWorkspacesMenu(recentsWorkspacesMenu);
    std_event.Skip(); });

	menuFile->AppendSeparator();
	menuFile->Append(+Event::File::Save, _("&Save") + GetSC("shortcut_save_file"));
	menuFile->Append(+Event::File::SaveAs, _("&Save As...") + GetSC("shortcut_save_file_as"));
	menuFile->Append(+Event::File::SaveAll, _("&Save All") + GetSC("shortcut_save_all"));
	menuFile->AppendSeparator();
	menuFile->Append(+Event::File::CloseFile, _("&Close File") + GetSC("shortcut_close_file"));
	menuFile->Append(+Event::File::CloseAll, _("&Close All Files") + GetSC("shortcut_close_all"));
	menuFile->Append(+Event::Project::CloseFolder, _("&Close Folder"));
	menuFile->Append(+Event::Frame::Exit, _("&Quit") + GetSC("shortcut_quit"));

	wxMenu *menuEdit = new wxMenu;
	menuEdit->Append(wxID_UNDO, _("&Undo") + GetSC("shortcut_undo"));
	menuEdit->Append(wxID_REDO, _("&Redo") + GetSC("shortcut_redo"));
	menuEdit->AppendSeparator();
	menuEdit->Append(wxID_CUT, _("&Cut") + GetSC("shortcut_cut"));
	menuEdit->Append(wxID_COPY, _("&Copy") + GetSC("shortcut_copy"));
	menuEdit->Append(wxID_PASTE, _("&Paste") + GetSC("shortcut_paste"));
	menuEdit->AppendSeparator();
	menuEdit->Append(+Event::Edit::ToggleComment, _("&Toggle Comment") + GetSC("shortcut_toggle_comment"));
	menuEdit->Append(+Event::Edit::ToggleBlockComment, _("&Toggle Block Comment") + GetSC("shortcut_toggle_block_comment"));
	menuEdit->AppendSeparator();
	menuEdit->Append(+Event::Edit::MoveLineUp, _("&Move Line Up") + GetSC("shortcut_move_line_up"));
	menuEdit->Append(+Event::Edit::MoveLineDown, _("&Move Line Down") + GetSC("shortcut_move_line_down"));
	menuEdit->Append(+Event::Edit::RemoveCurrentLine, _("&Remove Current Line") + GetSC("shortcut_delete_line"));

	wxMenu *menuSelection = new wxMenu;
	menuSelection->Append(+Event::Edit::SelectAll, _("&Select All") + GetSC("shortcut_select_all"));
	menuSelection->Append(+Event::Edit::SelectLine, _("&Select Line") + GetSC("shortcut_select_line"));

	wxMenu *menuView = new wxMenu;
	wxMenuItem *toggleFileTreeView = new wxMenuItem(menuView, +Event::View::ToggleFileTree, "File Tree", wxEmptyString, wxITEM_CHECK);
	menuView->Append(toggleFileTreeView);
	toggleFileTreeView->Check(UserSettings["show_files_tree"]);
	wxMenuItem *toggleMenuBarView = new wxMenuItem(menuView, +Event::View::ToggleMenuBar, "Menu Bar", wxEmptyString, wxITEM_CHECK);
	menuView->Append(toggleMenuBarView);
	toggleMenuBarView->Check(UserSettings["show_menu_bar"]);
	wxMenuItem *toggleStatusBarView = new wxMenuItem(menuView, +Event::View::ToggleStatusBar, "Status Bar", wxEmptyString, wxITEM_CHECK);
	menuView->Append(toggleStatusBarView);
	toggleStatusBarView->Check(UserSettings["show_status_bar"]);
	wxMenuItem *toggleTabBarView = new wxMenuItem(menuView, +Event::View::ToggleTabBar, "Tab bar", wxEmptyString, wxITEM_CHECK);
	menuView->Append(toggleTabBarView);
	toggleTabBarView->Check(UserSettings["show_tab_bar"]);
	wxMenuItem *toggleMinimapView = new wxMenuItem(menuView, +Event::View::ToggleMiniMap, "Minimap", wxEmptyString, wxITEM_CHECK);
	menuView->Append(toggleMinimapView);
	toggleMinimapView->Check(UserSettings["show_minimap"]);
	menuView->AppendSeparator();
	menuView->Append(+Event::View::ToggleFullScreen, _("&Full Screen") + GetSC("shortcut_toggle_full_screen"));
	menuView->Append(+Event::View::SplitEditor, _("&Split Editor") + GetSC("shortcut_split_editor"));
	menuView->Append(+Event::View::ZoomIn, _("&Zoom In") + GetSC("shortcut_zoom_in"));
	menuView->Append(+Event::View::ZoomOut, _("&Zoom Out") + GetSC("shortcut_zoom_out"));
	menuView->Append(+Event::View::ZoomReset, _("&Reset Zoom") + GetSC("shortcut_zoom_reset"));

	wxMenu *menuTools = new wxMenu;
	menuTools->Append(+Event::View::ToggleControlPanel, _("&Command Palette") + GetSC("shortcut_command_palette"));
	menuTools->Append(+Event::View::ToggleQuickOpen, _("&Quick Open") + GetSC("shortcut_quick_open"));
	menuTools->Append(+Event::Terminal::Open, _("&Open Terminal") + GetSC("shortcut_open_terminal"));
	menuTools->AppendSeparator();
	menuTools->Append(+Event::Edit::GoToLine, _("&Go to Line") + GetSC("shortcut_go_to_line"));
	menuTools->Append(+Event::Edit::GoToSymbol, _("&Go to Symbol") + GetSC("shortcut_go_to_symbol"));
	menuTools->Append(+Event::Edit::GoToDefinition, _("&Go to Definition") + GetSC("shortcut_go_to_definition"));

	wxMenu *menuPreference = new wxMenu;
	menuPreference->Append(+Event::UserSettings::Edit, _("&Settings") + GetSC("shortcut_open_settings"));
	menuPreference->Append(+Event::Shortcuts::Edit, _("&Shortcuts") + GetSC("shortcut_open_shortcuts"));

	wxMenu *menuHelp = new wxMenu;
	menuHelp->Append(+Event::Frame::About, _("&About Krafta Editor"));

	this->Append(menuFile, _("&File"));
	this->Append(menuEdit, _("&Edit"));
	this->Append(menuSelection, _("&Selection"));
	this->Append(menuView, _("&View"));
	this->Append(menuTools, _("&Tools"));
	this->Append(menuPreference, _("&Preferences"));
	this->Append(menuHelp, _("&Help"));
}