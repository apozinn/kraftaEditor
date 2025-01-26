#include "./files.hpp"

#include <vector>
#include <wx/scrolwin.h>
#include <wx/wfstream.h>
#include <wx/txtstrm.h>
#include <wx/statline.h>
#include <wx/app.h>
#include <wx/statbmp.h>
#include "../../main.hpp"

#include "../../utils/randoms.hpp"
#include "../../members/imagePanel.cpp"
#include "../statusBar/status.hpp"
#include "../codeContainer/code.hpp"
#include "../tabs/tabs.hpp"
#include <wx/graphics.h>
#include "./navigationButtons.cpp"
#include "searchFiles.cpp"

FilesTree::FilesTree(wxWindow* parent, wxWindowID ID) : wxPanel(parent, ID)
{
	auto background_color = UserTheme["main"].template get<std::string>();
	SetBackgroundColour(wxColor(background_color));
	sizer = new wxBoxSizer(wxVERTICAL);

	wxPanel* project_files = new wxPanel(this, ID_PROJECT_FILES);
	wxBoxSizer* pjt_files_sizer = new wxBoxSizer(wxVERTICAL);

	wxPanel* project_tools = new wxPanel(project_files, ID_PROJECT_TOOLS_BAR);
	project_tools->Bind(wxEVT_LEFT_UP, &FilesTree::ToggleDir, this);
	pjt_files_sizer->Add(project_tools, 0, wxEXPAND | wxLEFT, 5);
	wxBoxSizer* pjt_tools_sizer = new wxBoxSizer(wxHORIZONTAL);

	wxVector<wxBitmap> bitmaps;
	bitmaps.push_back(wxBitmap(icons_dir + "dir_arrow.png", wxBITMAP_TYPE_PNG));
	wxStaticBitmap* pjt_arrow = new wxStaticBitmap(project_tools, ID_PJT_TOOLS_ARROW, wxBitmapBundle::FromBitmaps(bitmaps));
	pjt_tools_sizer->Add(pjt_arrow, 0, wxEXPAND | wxBOTTOM, 2);

	wxStaticText* pjt_name = new wxStaticText(project_tools, ID_PJT_TOOLS_PJTNAME, wxString(project_name));
	pjt_name->Bind(wxEVT_LEFT_UP, &FilesTree::ToggleDir, this);
	pjt_tools_sizer->Add(pjt_name, 1, wxEXPAND | wxLEFT, 4);
	project_tools->SetSizerAndFit(pjt_tools_sizer);

	project_files_ctn = new wxScrolled<wxPanel>(project_files, ID_PROJECT_FILES_CTN);
	pjt_files_sizer->Add(project_files_ctn, 1, wxEXPAND);
	auto pjt_files_ctn_sizer = new wxBoxSizer(wxVERTICAL);
	project_files_ctn->SetSizerAndFit(pjt_files_ctn_sizer);
	project_files->SetSizerAndFit(pjt_files_sizer);

	sizer->Add(project_files, 1, wxEXPAND);
	SetSizerAndFit(sizer);
	if (!project_path.size())
		pjt_arrow->Hide();

	Bind(wxEVT_PAINT, &FilesTree::OnPaint, this);
}

void FilesTree::Update()
{
	project_files_ctn->DestroyChildren();
	project_files_ctn->SetName(project_path);
	if (auto project_name_comp = FindWindowById(ID_PJT_TOOLS_PJTNAME))
		project_name_comp->SetLabel(project_name);
	if (auto project_arrow = FindWindowById(ID_PJT_TOOLS_ARROW))
		project_arrow->Show();

	Load(project_files_ctn, project_path.ToStdString());

	project_files_ctn->FitInside();
	project_files_ctn->SetScrollRate(20, 20);
	selectedFile = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(0, 0));
}

bool FilesTree::Load(wxWindow* parent, std::string path) {
	//destroy all childrens if the current path is the path of the project
	if (path == project_path) {
		project_files_ctn->DestroyChildren();
	}

	//list all files in directory
	const std::filesystem::path projectDirs{ path };
	for (auto const& dir_entry : std::filesystem::directory_iterator{ projectDirs })
	{
		wxString name = wxFileNameFromPath(wxString(dir_entry.path()));
		wxString path = wxString(dir_entry.path());

		if (dir_entry.is_directory()) {
			CreateDir(parent, name, path);
		}
	}

	//list all directorys in directory
	const std::filesystem::path projectFiles{ path };
	for (auto const& dir_entry : std::filesystem::directory_iterator{ projectFiles })
	{
		wxString name = wxFileNameFromPath(wxString(dir_entry.path()));
		wxString path = wxString(dir_entry.path());

		if (!dir_entry.is_directory()) {
			CreateFile(parent, name, path);
		}
	}

	return true;
}

void FilesTree::CreateFile(
	wxWindow* parent, wxString name, wxString path)
{
	if (!parent || FindWindowByLabel(path + "_file_container")) return;
	auto parentSizer = parent->GetSizer();
	if (!parentSizer) {
		parentSizer = new wxBoxSizer(wxHORIZONTAL);
		parent->SetSizerAndFit(parentSizer);
	}

	wxPanel* file_container = new wxPanel(parent);

	file_container->Bind(wxEVT_RIGHT_UP, &FilesTree::onFileRightClick, this);
	file_container->SetMinSize(wxSize(file_container->GetSize().GetWidth(), 20));
	file_container->SetSize(file_container->GetSize().GetWidth(), 20);
	file_container->SetName(path);
	file_container->SetLabel(path + "_file_container");
	file_container->Bind(wxEVT_LEFT_UP, &FilesTree::OnFileSelect, this);
	wxBoxSizer* file_ctn_sizer = new wxBoxSizer(wxHORIZONTAL);

	LanguageInfo const* currentLanguageInfo;
	LanguageInfo const* currentInfo;
	int languageNr;

	bool found = false;
	for (languageNr = 0; languageNr < languages_prefs_size; languageNr++)
	{
		currentInfo = &languages_prefs[languageNr];
		wxString filepattern = currentInfo->filepattern;
		filepattern.Lower();

		while (!filepattern.empty() && !found)
		{
			wxString cur = filepattern.BeforeFirst(';');
			if ((cur == path) ||
				(cur == (path.BeforeLast('.') + ".*")) ||
				(cur == ("*." + path.AfterLast('.'))))
			{
				found = true;
				currentLanguageInfo = currentInfo;
			}
			filepattern = filepattern.AfterFirst(';');
		}
	}

	if (!found)
		currentLanguageInfo = &languages_prefs[0];

	wxVector<wxBitmap> bitmaps;
	auto last_dot = path.find_last_of(".");
	if (last_dot != std::string::npos)
	{
		std::string file_ext = path.ToStdString().substr(last_dot + 1);
		if (file_ext.size() && file_ext != path)
		{
			if (file_ext == "png" || file_ext == "jpg" || file_ext == "jpeg")
			{
				if (wxOS_WINDOWS) {
					bitmaps.push_back(wxBitmap(icons_dir + "file_ext" + "\\image_ext.png", wxBITMAP_TYPE_PNG));
				}
				else {
					bitmaps.push_back(wxBitmap(icons_dir + "file_ext" + "/image_ext.png", wxBITMAP_TYPE_PNG));
				}
			}
			else
			{
				bitmaps.push_back(wxBitmap(icons_dir + currentLanguageInfo->icon_path, wxBITMAP_TYPE_PNG));
			}
		}
	}
	else {
		if (wxOS_WINDOWS) {
			bitmaps.push_back(wxBitmap(icons_dir + "file_ext\\no_ext.png", wxBITMAP_TYPE_PNG));
		}
		else {
			bitmaps.push_back(wxBitmap(icons_dir + "file_ext/no_ext.png", wxBITMAP_TYPE_PNG));
		}
	}

	wxStaticBitmap* file_icon = new wxStaticBitmap(file_container, wxID_ANY, wxBitmapBundle::FromBitmaps(bitmaps));
	file_ctn_sizer->Add(file_icon, 0, wxALIGN_CENTRE_VERTICAL | wxLEFT, 8);

	wxStaticText* file_name = new wxStaticText(file_container, wxID_ANY, name);

	file_name->SetName(path);
	file_name->Bind(wxEVT_LEFT_UP, &FilesTree::OnFileSelect, this);
	file_name->Bind(wxEVT_RIGHT_UP, &FilesTree::onFileRightClick, this);
	file_ctn_sizer->Add(file_name, 0, wxALIGN_CENTRE_VERTICAL | wxLEFT, 5);
	file_container->SetSizerAndFit(file_ctn_sizer);

	parentSizer->Add(file_container, 0, wxEXPAND | wxLEFT | wxTOP, 2);
	parentSizer->Layout();

	file_container->CallForEachChild([=](wxWindow* win) {
		win->Bind(wxEVT_ENTER_WINDOW, &FilesTree::OnEnterComp, this);
		win->Bind(wxEVT_LEAVE_WINDOW, &FilesTree::OnLeaveComp, this);
		});
}

void FilesTree::CreateDir(
	wxWindow* parent, wxString name, wxString path)
{
	if (!parent || FindWindowByLabel(path + "_dir_container")) return;
	auto parentSizer = parent->GetSizer();
	if (!parentSizer) {
		parentSizer = new wxBoxSizer(wxHORIZONTAL);
		parent->SetSizerAndFit(parentSizer);
	}

	wxPanel* dir_container = new wxPanel(parent);

	dir_container->SetMinSize(wxSize(dir_container->GetSize().GetWidth(), 20));
	dir_container->SetSize(dir_container->GetSize().GetWidth(), 20);
	dir_container->SetName(path);
	dir_container->SetLabel(path + "_dir_container");
	wxBoxSizer* dir_ctn_sizer = new wxBoxSizer(wxVERTICAL);

	wxPanel* dir_props = new wxPanel(dir_container);
	dir_props->SetLabel("dir_props");

	dir_props->Bind(wxEVT_RIGHT_UP, &FilesTree::onDirRightClick, this);

	wxBoxSizer* props_sizer = new wxBoxSizer(wxHORIZONTAL);
	dir_ctn_sizer->Add(dir_props, 0, wxEXPAND | wxLEFT, 8);

	wxVector<wxBitmap> bitmaps;
	bitmaps.push_back(wxBitmap(icons_dir + "dir_arrow.png", wxBITMAP_TYPE_PNG));
	wxStaticBitmap* dir_arrow = new wxStaticBitmap(dir_props, wxID_ANY, wxBitmapBundle::FromBitmaps(bitmaps));
	props_sizer->Add(dir_arrow, 0, wxEXPAND | wxTOP, 2);

	wxStaticText* dir_name = new wxStaticText(dir_props, wxID_ANY, name);
	dir_name->SetName("dir_name");
	dir_name->Bind(wxEVT_LEFT_UP, &FilesTree::ToggleDir, this);
	dir_name->Bind(wxEVT_RIGHT_UP, &FilesTree::onDirRightClick, this);
	props_sizer->Add(dir_name, 0, wxEXPAND | wxLEFT, 4);
	dir_props->SetSizerAndFit(props_sizer);

	wxPanel* dir_childrens = new wxPanel(dir_container);
	dir_childrens->SetName(path + "_dir_childrens");

	//event to draw a dotted line next side to the dir childrens
	dir_childrens->Bind(wxEVT_PAINT, [=](wxPaintEvent& event) {
		auto target = ((wxPanel*)event.GetEventObject());
		if (!target) return;

		auto borderColor = UserTheme["border"].template get<std::string>();
		wxColour color = wxColour(borderColor);
		wxPaintDC dc(target);

		dc.SetPen(wxPen(wxColor(color), 1.25, wxPENSTYLE_DOT));
		dc.SetBrush(color);
		dc.DrawLine(wxPoint(0, 0), wxPoint(0, target->GetSize().y));
		});

	wxBoxSizer* dir_childrens_sizer = new wxBoxSizer(wxVERTICAL);
	dir_childrens->SetSizerAndFit(dir_childrens_sizer);
	dir_ctn_sizer->Add(dir_childrens, 0, wxEXPAND | wxLEFT, 10);

	dir_container->SetSizerAndFit(dir_ctn_sizer);
	dir_childrens->Hide();

	parentSizer->Add(dir_container, 0, wxEXPAND | wxLEFT, 2);
	parentSizer->Layout();
}

void FilesTree::OnFileSelect(wxMouseEvent& event)
{
	auto file = ((wxWindow*)event.GetEventObject());
	auto selectedFileColor = UserTheme["selectedFile"].template get<std::string>();
	auto mainColor = UserTheme["main"].template get<std::string>();
	wxString path = file->GetName();

	if (file->GetLabel().find("file_container") == std::string::npos) {
		file = file->GetParent();
	}

	if (path.size())
	{
		if (selectedFile)
		{
			selectedFile->SetBackgroundColour(wxColor(mainColor));
		}
		selectedFile = file;
		selectedFile->SetBackgroundColour(wxColor(selectedFileColor));
		OpenFile(path);
	}
}

void FilesTree::OpenFile(wxString path)
{
	//obtain panels that will be used
	auto mainCode = FindWindowById(ID_MAIN_CODE);
	auto tabsContainer = ((Tabs*)FindWindowById(ID_TABS));
	auto statusBar = ((StatusBar*)FindWindowById(ID_STATUS_BAR));

	//hiding others comps: empty window or code container
	for (auto&& other_ct : mainCode->GetChildren()) {
		if (other_ct->GetId() != ID_TABS)
			other_ct->Hide();
	}

	//add a new tab
	tabsContainer->Add(wxFileNameFromPath(path), path);

	//get the extension of file
	wxString fileExt = wxFileName(path).GetExt();

	//function to load a text file 
	auto LoadCodeContainer = [&]() {
		auto codeContainer = ((CodeContainer*)FindWindowByName(path + "_codeContainer"));
		//creating a new code container if it doesn't exist
		if (!codeContainer) {
			codeContainer = new CodeContainer(mainCode, path);
			mainCode->GetSizer()->Add(codeContainer, 1, wxEXPAND);
		}
		else codeContainer->Show();

		//status bar 
		statusBar->UpdateComps(path, "text", codeContainer->current_lang->name);
		};

	if (!fileExt.empty()) {
		//verify if file is a image
		wxImage fileImage = wxImage();
		if (fileImage.CanRead(path)) {

			fileImage.LoadFile(path, wxBITMAP_DEFAULT_TYPE);
			if (!fileImage.IsOk()) return;

			//resize image if larger than 1000 pixels
			if (fileImage.GetWidth() > 1000 || fileImage.GetHeight() > 1000)
			{
				fileImage.Rescale(fileImage.GetWidth() / 2, fileImage.GetHeight() / 2);
			}
			//creating a image container
			wxVector<wxBitmap> bitmaps;
			bitmaps.push_back(wxBitmap(fileImage));
			wxStaticBitmap* image_container = new wxStaticBitmap(mainCode, wxID_ANY, wxBitmapBundle::FromBitmaps(bitmaps));
			image_container->SetLabel(path + "_imageContainer");

			//updating the main container and status bar
			mainCode->GetSizer()->Add(image_container, 1, wxALIGN_CENTER);
			statusBar->UpdateComps(path, "image", "img");
		}
		else LoadCodeContainer();
	}
	else LoadCodeContainer();

	//updating main container
	project_files_ctn->Update();
	mainCode->GetSizer()->Layout();
}

void FilesTree::ToggleDir(wxMouseEvent& event)
{
	auto dirContainer = ((wxWindow*)event.GetEventObject());
	//verify if container is the main container of project
	if (
		dirContainer->GetId() == ID_PROJECT_TOOLS_BAR ||
		dirContainer->GetId() == ID_PJT_TOOLS_PJTNAME
		)
	{
		dirContainer = project_files_ctn;

		//rotate the arrow icon
		auto dir_arrow_ctn = ((wxStaticBitmap*)FindWindowById(ID_PJT_TOOLS_ARROW));
		auto arrow_bit = dir_arrow_ctn->GetBitmap();
		wxVector<wxBitmap> bitmaps;

		if (dirContainer->IsShown()) {
			dirContainer->Hide();
			dirContainer->Disable();
			bitmaps.push_back(wxBitmap(arrow_bit.ConvertToImage().Rotate90(true), -1));
		}
		else
		{
			dirContainer->Show();
			dirContainer->Enable();
			bitmaps.push_back(wxBitmap(arrow_bit.ConvertToImage().Rotate90(false), -1));
		}

		dir_arrow_ctn->SetBitmap(wxBitmapBundle::FromBitmaps(bitmaps));
	}

	//getting the main container of the dir component if the event was received by another component
	if (dirContainer->GetLabel() == "dir_props")
	{
		dirContainer = dirContainer->GetParent();
	}
	else if (dirContainer->GetName() == "dir_name")
	{
		dirContainer = dirContainer->GetGrandParent();
	}

	//getting arrow icon and dir container childrens
	auto dir_arrow_ctn = ((wxStaticBitmap*)dirContainer->GetChildren()[0]->GetChildren()[0]);
	auto dir_childrens = dirContainer->GetChildren()[1];

	wxString path = dirContainer->GetName();

	if (dir_childrens && dir_arrow_ctn)
	{
		//rotate the arrow icon
		auto arrow_bit = dir_arrow_ctn->GetBitmap();
		wxVector<wxBitmap> bitmaps;

		if (dir_childrens->IsShown())
		{
			dir_childrens->Hide();
			bitmaps.push_back(wxBitmap(arrow_bit.ConvertToImage().Rotate90(false), -1));
		}
		else
		{
			bitmaps.push_back(wxBitmap(arrow_bit.ConvertToImage().Rotate90(true), -1));
			dir_childrens->Show();

			for (auto& w : dir_childrens->GetChildren()) {
				w->Show();
			}

			//load the dir content
			Load(dir_childrens, path.ToStdString());
		}

		//set new arrow icon
		dir_arrow_ctn->SetBitmap(wxBitmapBundle::FromBitmaps(bitmaps));

		//calling a function to resize the container to best size
		FitContainer(dir_childrens);
	}
}

void FilesTree::onTopMenuClick(wxMouseEvent& event)
{
	wxMenu* menuFile = new wxMenu;
	menuFile->Append(wxID_ANY, _("&New File"));
	menuFile->Append(ID_CREATE_DIR, _("&New Folder"));
	menuFile->Append(wxID_ANY, _("&Open terminal"));
	PopupMenu(menuFile);
}

void FilesTree::onFileRightClick(wxMouseEvent& event)
{
	auto target = ((wxWindow*)event.GetEventObject());
	if (!target->GetName().size())
		return;
	menufile_path = target->GetName();

	wxMenu* menuFile = new wxMenu;
	menuFile->Append(ID_RENAME_FILE, _("&Rename"));
	menuFile->Append(ID_DELETE_FILE, _("&Delete File"));
	PopupMenu(menuFile);
}

void FilesTree::onDirRightClick(wxMouseEvent& event)
{
	auto eventObj = ((wxWindow*)event.GetEventObject());
	wxWindow* target;

	if (eventObj->GetLabel() == "dir_props")
	{
		target = eventObj->GetParent();
	}
	else if (eventObj->GetName() == "dir_name")
	{
		target = eventObj->GetGrandParent();
	}

	if (!target->GetName().size())
		return;
	menudir_path = target->GetName();

	wxMenu* menuDir = new wxMenu;
	menuDir->Append(ID_RENAME_DIR, _("&Rename"));
	menuDir->Append(ID_CREATE_FILE, _("&New File"));
	menuDir->Append(ID_CREATE_DIR, _("&New Folder"));
	menuDir->Append(wxID_ANY, _("&Open Folder"));
	menuDir->Append(ID_DELETE_DIR, _("&Delete Folder"));
	PopupMenu(menuDir);
}

void FilesTree::OnPaint(wxPaintEvent& event)
{
	auto target = ((wxPanel*)event.GetEventObject());
	if (target->GetId() == ID_FILES_TREE) return;

	json nThemes = UserConfig().GetThemes();
	auto border_color = nThemes["dark"]["selectedFile"].template get<std::string>();

	wxClientDC dc(this);
	wxGraphicsContext* gc = wxGraphicsContext::Create(dc);
	if (!gc) return;

	if (target->GetId() == ID_SEARCH_FILES)
	{
		dc.SetPen(wxPen(wxColor(border_color), 0.20));
		dc.DrawLine(0, target->GetPosition().y, target->GetSize().GetWidth(), target->GetPosition().y);
		dc.DrawLine(0, target->GetPosition().y + target->GetSize().GetHeight(), target->GetSize().GetWidth(), target->GetPosition().y + target->GetSize().GetHeight());
	}
	else if (target->GetId() == ID_FILES_TREE_TOP_CONTENT)
	{
		dc.SetPen(wxPen(wxColor(border_color), 0.20));
		dc.DrawLine(target->GetSize().GetWidth(), target->GetSize().GetHeight() - 10, 0, target->GetSize().GetHeight() - 10);
	}
	delete gc;
}

void FilesTree::FitContainer(wxWindow* window)
{
	wxWindow* parent = window;

	if (!window->IsShownOnScreen()) {
		while (parent->GetId() != ID_PROJECT_FILES_CTN) {
			if (!parent->IsShownOnScreen()) parent->SetMinSize(wxSize(0, 0));
			if (!parent->IsShownOnScreen()) parent->SetSize(wxSize(0, 0));
			parent->GetSizer()->Layout();

			wxSize thisSize = wxSize(parent->GetSize().x, 20);
			for (auto&& children : parent->GetChildren()) {
				if (children->IsShownOnScreen()) {
					thisSize.SetHeight(thisSize.y + children->GetSize().y);
				}
			}

			parent = parent->GetParent();
			parent->SetMinSize(thisSize);
			parent->GetSizer()->Layout();
			parent = parent->GetParent();
		}

	}
	else {
		while (parent->GetId() != ID_PROJECT_FILES_CTN) {
			parent->SetMinSize(wxSize(parent->GetBestSize()));
			parent->GetSizer()->Layout();
			parent = parent->GetParent();
		}
	}

	project_files_ctn->GetSizer()->Layout();
	project_files_ctn->FitInside();
}

void FilesTree::OnCreateDir(wxCommandEvent& event)
{
	wxString folder_name = wxGetTextFromUser("Enter the folder name: ", "Create Folder", "");
	if (folder_name.empty())
		return;
	if (__WXWINDOWS__) {
		fileManager->CreateDir(menudir_path + "\\" + folder_name);
	}
	else {
		fileManager->CreateDir(menudir_path + "/" + folder_name);
	}
}

void FilesTree::OnCreateFile(wxCommandEvent& event)
{
	wxString file_name = wxGetTextFromUser("Enter the file name: ", "Create File", "");
	if (file_name.empty())
		return;

	if (menudir_path.Len() > 1000)
	{
		if (project_path.Len() > 1000)
		{
			wxMessageBox(_("You can't create a file in a empty project"), _("Create File"), wxOK | wxICON_INFORMATION, this);
		}
		else
		{
			fileManager->CreateFile(project_path + file_name);
			OpenFile(project_path + file_name);
		}
	}
	else
	{
		fileManager->CreateFile(menudir_path + "/" + file_name);
		OpenFile(menudir_path + "/" + file_name);
	}
}

void FilesTree::OnDeleteDir(wxCommandEvent& event)
{
	bool deleted = fileManager->DeleteDir(menudir_path);
}

void FilesTree::OnDeleteFile(wxCommandEvent& event)
{
	bool deleted = fileManager->DeleteFile(menufile_path);
}

void FilesTree::OnTreeModifyed(wxString old_path, wxString new_path)
{
	if (!old_path || !new_path) return;
	wxFileName path_props(old_path);
	wxString parent_path;

	auto opennedEditor = ((CodeContainer*)FindWindowByName(new_path + "_codeContainer"));
	if (opennedEditor) {
		opennedEditor->LoadPath(new_path);
	}

	if (path_props.IsDir())
	{
		parent_path = path_props.GetPath().substr(0, path_props.GetPath().find_last_of("/"));
	}
	else
	{
		parent_path = path_props.GetPath();
	}

	auto parent = FindWindowByName(parent_path);
	if (parent_path + "/" == project_path)
		parent = project_files_ctn;
	if (parent)
	{
		if (parent->GetId() != ID_PROJECT_FILES_CTN)
			parent = parent->GetChildren()[1];
		parent->DestroyChildren();
		Load(parent, parent_path.ToStdString() + "/");
		FitContainer(parent);
	}
}

void FilesTree::OnEnterComp(wxMouseEvent& event)
{
	//event to set an accent color for the component on mouse focus
	auto target = ((wxWindow*)event.GetEventObject());
	if (selectedFile) {
		if (selectedFile->GetName() == target->GetName()) return;
	}
	auto color = UserTheme["border"].template get<std::string>();

	auto fileContainer = FindWindowByLabel(target->GetName() + "_file_container");
	if (!fileContainer) return;

	fileContainer->SetBackgroundColour(wxColor(color));
	fileContainer->Refresh();
}

void FilesTree::OnLeaveComp(wxMouseEvent& event)
{
	//event to set a color for the component outside of mouse focus
	auto target = ((wxWindow*)event.GetEventObject());
	if (selectedFile) {
		if (selectedFile->GetName() == target->GetName()) return;
	}
	auto color = UserTheme["main"].template get<std::string>();

	auto fileContainer = FindWindowByLabel(target->GetName() + "_file_container");
	if (!fileContainer) return;

	fileContainer->SetBackgroundColour(wxColor(color));
	fileContainer->Refresh();
}

void FilesTree::OnFileRename(wxCommandEvent& event)
{
	wxString new_name = wxGetTextFromUser("Enter the new file name: ", "Rename File", "");
	if (new_name.empty())
		return;
	if (menufile_path.empty())
		return;

	wxString target_parent = menufile_path.substr(0, menufile_path.find_last_of("/") + 1);
	wxString new_path = target_parent + new_name;

	if (!wxRename(menufile_path, new_path))
	{
		wxMessageBox("There was an error renaming the file", "", wxOK | wxICON_INFORMATION);
	}
}

void FilesTree::OnDirRename(wxCommandEvent& event)
{
	wxString new_name = wxGetTextFromUser("Enter the new dir name: ", "Rename Dir", "");
	if (new_name.empty())
		return;
	if (menudir_path.empty())
		return;

	wxString target_parent = menudir_path.substr(0, menudir_path.find_last_of("/") + 1);
	wxString new_path = target_parent + new_name;

	if (!wxRename(menudir_path, new_path))
	{
		wxMessageBox("There was an error renaming the dir", "", wxOK | wxICON_INFORMATION);
	}
}