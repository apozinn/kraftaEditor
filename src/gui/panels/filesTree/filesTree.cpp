#include "filesTree.hpp"

#include <vector>
#include <wx/scrolwin.h>
#include <wx/wfstream.h>
#include <wx/txtstrm.h>
#include <wx/richtooltip.h>
#include <wx/statline.h>
#include <wx/statbmp.h>
#include <wx/graphics.h>
#include <wx/infobar.h>

#include "platformInfos/platformInfos.hpp"
#include "gui/widgets/statusBar/statusBar.hpp"
#include "gui/panels/tabs/tabs.hpp"
#include "gui/codeContainer/code.hpp"
#include "gui/widgets/confirmDialog/confirmDialog.hpp"

wxBEGIN_EVENT_TABLE(FilesTree, wxPanel)
	EVT_MENU(+Event::File::RenameFile, FilesTree::OnFileRename)
		EVT_MENU(+Event::File::DeleteFileEvent, FilesTree::OnDeleteFile)
			EVT_MENU(+Event::File::RenameDir, FilesTree::OnDirRename)
				EVT_MENU(+Event::File::DeleteDir, FilesTree::OnDeleteDir)
					wxEND_EVENT_TABLE()

						FilesTree::FilesTree(wxWindow *parent, wxWindowID ID)
	: wxPanel(parent, ID)
{
	// setting the background color
	auto background_color = Theme["main"].template get<std::string>();
	SetBackgroundColour(wxColor(background_color));

	// projectToggler
	projectToggler = new wxPanel(this, +GUI::ControlID::ProjectToggler);
	wxSizer *projectTogglerSizer = new wxBoxSizer(wxHORIZONTAL);
	projectToggler->SetSizerAndFit(projectTogglerSizer);

	// arrow icon
	wxVector<wxBitmap> bitmaps;
	bitmaps.push_back(wxBitmap(iconsDir + "dir_arrow.png", wxBITMAP_TYPE_PNG));
	pjt_arrow = new wxStaticBitmap(projectToggler, +GUI::ControlID::ProjectToolsArrow, wxBitmapBundle::FromBitmaps(bitmaps));
	projectTogglerSizer->Add(pjt_arrow, 0, wxEXPAND);

	// project namne
	projectToolsName = new wxStaticText(projectToggler, +GUI::ControlID::ProjectToolsName, projectSettings.GetProjectName());
	wxFont projectNameFont = projectToolsName->GetFont();
	projectNameFont.SetWeight(wxFONTWEIGHT_MEDIUM);
	projectToolsName->SetFont(projectNameFont);
	projectTogglerSizer->Add(projectToolsName, 1, wxEXPAND | wxLEFT, 4);

	// adding projectToggler to main sizer
	sizer->Add(projectToggler, 0, wxEXPAND | wxTOP | wxBOTTOM, 5);

	// projectFilesContainer
	projectFilesContainer = new wxScrolled<wxPanel>(this, +GUI::ControlID::ProjectFilesContainer);
	projectFilesContainer->SetScrollbars(20, 20, 50, 50);
	wxBoxSizer *projectFilesContainerSizer = new wxBoxSizer(wxVERTICAL);
	projectFilesContainer->SetSizerAndFit(projectFilesContainerSizer);

	// adding projectFilesContainer to main sizer
	sizer->Add(projectFilesContainer, 1, wxEXPAND);

	SetSizerAndFit(sizer);

	// setting projectToggler min size
	projectToggler->SetMinSize(wxSize(GetSize().x, 15));

	if (!projectSettings.IsProjectSet())
	{
		projectToggler->Hide();
	}

	Bind(wxEVT_PAINT, &FilesTree::OnPaint, this);

	// lincking components of the projectToggler
	projectToggler->CallForEachChild([=](wxWindow *children)
									 {
		//function to toggler view of the projectFilesContainer
		children->Bind(wxEVT_LEFT_DOWN, [=](wxMouseEvent& WXUNUSED(event)) {
			auto arrow_bit = pjt_arrow->GetBitmap();
			wxVector<wxBitmap> bitmaps;

			if(projectFilesContainer->IsShown()) {
				projectFilesContainer->Hide();
				bitmaps.push_back(wxBitmap(arrow_bit.ConvertToImage().Rotate90(true), -1));
			} else {
				projectFilesContainer->Show();
				bitmaps.push_back(wxBitmap(arrow_bit.ConvertToImage().Rotate90(false), -1));
			}

			//updating project arrow icon
			pjt_arrow->SetBitmap(wxBitmapBundle::FromBitmaps(bitmaps)); 
			}); });
}

void FilesTree::Load(wxWindow *parent, std::string path)
{
	// destroy all childrens if the current path is the path of the project
	if (path == projectSettings.GetProjectPath())
	{
		projectFilesContainer->DestroyChildren();
	}

	// list all files in directory
	const std::filesystem::path projectDirs{path};
	for (auto const &dir_entry : std::filesystem::directory_iterator{projectDirs})
	{
		wxString name = wxFileNameFromPath(wxString(dir_entry.path()));
		wxString currentPath = wxString(dir_entry.path());

		if (dir_entry.is_directory())
		{
			CreateDir(parent, name, currentPath);
		}
	}

	// list all directorys in directory
	const std::filesystem::path projectFiles{path};
	for (auto const &dir_entry : std::filesystem::directory_iterator{projectFiles})
	{
		wxString name = wxFileNameFromPath(wxString(dir_entry.path()));
		wxString currentPath = wxString(dir_entry.path());

		if (!dir_entry.is_directory())
		{
			CreateFile(parent, name, currentPath);
		}
	}

	// updating the projectName
	projectToolsName->SetLabel(projectSettings.GetProjectName());
	projectToggler->Show();

	projectToolsName->SetToolTip(projectSettings.GetProjectPath());

	// setting menuPath
	projectSettings.SetCurrentlyMenuDir(ProjectSettings::Get().GetProjectPath());
	projectSettings.SetCurrentlyMenuFile(ProjectSettings::Get().GetProjectPath());
}

void FilesTree::CreateFile(
	wxWindow *parent, wxString name, wxString path)
{
	if (!parent || FindWindowByLabel(path + "_file_container"))
		return;

	auto parentSizer = parent->GetSizer();
	if (!parentSizer)
	{
		parentSizer = new wxBoxSizer(wxHORIZONTAL);
		parent->SetSizerAndFit(parentSizer);
	}

	wxPanel *file_container = new wxPanel(parent);
	file_container->SetToolTip(path);

	file_container->Bind(wxEVT_RIGHT_UP, &FilesTree::onFileRightClick, this);
	file_container->SetMinSize(wxSize(file_container->GetSize().GetWidth(), 20));
	file_container->SetSize(file_container->GetSize().GetWidth(), 20);
	file_container->SetName(path);
	file_container->SetLabel(path + "_file_container");
	file_container->Bind(wxEVT_LEFT_UP, &FilesTree::OnFileSelect, this);
	wxBoxSizer *file_ctn_sizer = new wxBoxSizer(wxHORIZONTAL);

	LanguageInfo const *currentLanguageInfo = &languages_prefs[0];
	LanguageInfo const *currentInfo;
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

	wxVector<wxBitmap> bitmaps;
	auto last_dot = path.find_last_of(".");
	if (last_dot != std::string::npos)
	{
		std::string file_ext = path.ToStdString().substr(last_dot + 1);
		if (file_ext.size() && file_ext != path)
		{
			if (file_ext == "png" || file_ext == "jpg" || file_ext == "jpeg")
			{
				bitmaps.push_back(wxBitmap(ApplicationPaths::GetLanguageIcon("image"), wxBITMAP_TYPE_PNG));
			}
			else
			{
				bitmaps.push_back(wxBitmap(ApplicationPaths::GetLanguageIcon(currentLanguageInfo->iconFileName), wxBITMAP_TYPE_PNG));
			}
		}
	}
	else
	{
		bitmaps.push_back(wxBitmap(ApplicationPaths::GetLanguageIcon("unkown_ext"), wxBITMAP_TYPE_PNG));
	}

	wxStaticBitmap *file_icon = new wxStaticBitmap(file_container, wxID_ANY, wxBitmapBundle::FromBitmaps(bitmaps));
	file_ctn_sizer->Add(file_icon, 0, wxALIGN_CENTRE_VERTICAL | wxLEFT, 8);

	wxStaticText *file_name = new wxStaticText(file_container, wxID_ANY, name);

	file_name->SetName(path);
	file_name->Bind(wxEVT_LEFT_UP, &FilesTree::OnFileSelect, this);
	file_name->Bind(wxEVT_RIGHT_UP, &FilesTree::onFileRightClick, this);
	file_ctn_sizer->Add(file_name, 0, wxALIGN_CENTRE_VERTICAL | wxLEFT, 5);
	file_container->SetSizerAndFit(file_ctn_sizer);

	parentSizer->Add(file_container, 0, wxEXPAND | wxLEFT | wxTOP, 2);
	parentSizer->Layout();

	file_container->CallForEachChild([=](wxWindow *win)
									 {
		win->Bind(wxEVT_ENTER_WINDOW, &FilesTree::OnEnterComp, this);
		win->Bind(wxEVT_LEAVE_WINDOW, &FilesTree::OnLeaveComp, this); });
}

void FilesTree::CreateDir(
	wxWindow *parent, wxString name, wxString path, int pos)
{
	path.Append(PlatformInfos::OsPathSepareator());

	if (!parent || FindWindowByLabel(path + "_dir_container"))
		return;
	auto parentSizer = parent->GetSizer();
	if (!parentSizer)
	{
		parentSizer = new wxBoxSizer(wxHORIZONTAL);
		parent->SetSizerAndFit(parentSizer);
	}

	wxPanel *dir_container = new wxPanel(parent);
	dir_container->SetToolTip(path);

	dir_container->SetMinSize(wxSize(dir_container->GetSize().GetWidth(), 20));
	dir_container->SetSize(dir_container->GetSize().GetWidth(), 20);
	dir_container->SetName(path);
	dir_container->SetLabel(path + "_dir_container");
	wxBoxSizer *dir_ctn_sizer = new wxBoxSizer(wxVERTICAL);

	wxPanel *dir_props = new wxPanel(dir_container);
	dir_props->SetToolTip(path);
	dir_props->SetLabel("dir_props");

	dir_props->Bind(wxEVT_RIGHT_UP, &FilesTree::onDirRightClick, this);

	wxBoxSizer *props_sizer = new wxBoxSizer(wxHORIZONTAL);
	dir_ctn_sizer->Add(dir_props, 0, wxEXPAND | wxLEFT, 8);

	wxVector<wxBitmap> bitmaps;
	bitmaps.push_back(wxBitmap(iconsDir + "dir_arrow.png", wxBITMAP_TYPE_PNG));
	wxStaticBitmap *dir_arrow = new wxStaticBitmap(dir_props, wxID_ANY, wxBitmapBundle::FromBitmaps(bitmaps));
	props_sizer->Add(dir_arrow, 0, wxEXPAND | wxTOP, 2);

	wxStaticText *dir_name = new wxStaticText(dir_props, wxID_ANY, name);
	dir_name->SetName("dir_name");
	dir_name->Bind(wxEVT_LEFT_UP, &FilesTree::ToggleDir, this);
	dir_name->Bind(wxEVT_RIGHT_UP, &FilesTree::onDirRightClick, this);
	props_sizer->Add(dir_name, 0, wxEXPAND | wxLEFT, 4);
	dir_props->SetSizerAndFit(props_sizer);

	wxPanel *dir_childrens = new wxPanel(dir_container);
	dir_childrens->SetName(path + "_dir_childrens");
	dir_childrens->SetLabel(path + "_dir_childrens");

	// event to draw a dotted line next side to the dir childrens
	dir_childrens->Bind(wxEVT_PAINT, [=](wxPaintEvent &event)
						{
		auto target = ((wxPanel*)event.GetEventObject());
		if (!target) return;

		auto borderColor = Theme["border"].template get<std::string>();
		wxColour color = wxColour(borderColor);
		wxPaintDC dc(target);

		dc.SetPen(wxPen(wxColor(color), 1.25, wxPENSTYLE_DOT));
		dc.SetBrush(color);
		dc.DrawLine(wxPoint(0, 0), wxPoint(0, target->GetSize().y)); });

	wxBoxSizer *dir_childrens_sizer = new wxBoxSizer(wxVERTICAL);
	dir_childrens->SetSizerAndFit(dir_childrens_sizer);
	dir_ctn_sizer->Add(dir_childrens, 0, wxEXPAND | wxLEFT, 10);

	dir_container->SetSizerAndFit(dir_ctn_sizer);
	dir_childrens->Hide();

	if (pos >= 0)
	{
		parentSizer->Insert(pos, dir_container, 0, wxEXPAND | wxLEFT, 2);
	}
	else
		parentSizer->Add(dir_container, 0, wxEXPAND | wxLEFT, 2);

	parentSizer->Layout();
}

void FilesTree::OnFileSelect(wxMouseEvent &event)
{
	auto file = ((wxWindow *)event.GetEventObject());
	auto mainColor = Theme["main"].template get<std::string>();
	wxString path = file->GetName();

	if (!wxFileExists(path))
		return;

	if (file->GetLabel().find("file_container") == std::string::npos)
	{
		file = file->GetParent();
	}

	if (path.size())
	{
		// openning file
		OpenFile(path);
	}
}

void FilesTree::OpenFile(wxString path)
{
	// obtain panels that will be used
	auto mainCode = FindWindowById(+GUI::ControlID::MainCode);
	auto tabsContainer = ((Tabs *)FindWindowById(+GUI::ControlID::Tabs));
	auto statusBar = ((StatusBar *)FindWindowById(+GUI::ControlID::StatusBar));

	// highlighting the file component
	auto newSelectedFile = wxFindWindowByLabel(path + "_file_container");
	if (newSelectedFile)
		SetSelectedFile(newSelectedFile);

	// hidding others comps: empty window or code container
	for (auto &&other_ct : mainCode->GetChildren())
	{
		if (other_ct->GetId() != +GUI::ControlID::Tabs)
			other_ct->Hide();
	}

	// add a new tab
	tabsContainer->Add(wxFileNameFromPath(path), path);

	// get the extension of file
	wxString fileExt = wxFileName(path).GetExt();

	// function to load a text file
	auto LoadCodeEditor = [&]()
	{
		auto CodeEditor = ((CodeContainer *)FindWindowByName(path + "_CodeEditor"));
		// creating a new code container if it doesn't exist
		if (!CodeEditor)
		{
			CodeEditor = new CodeContainer(mainCode, path);
			mainCode->GetSizer()->Add(CodeEditor, 1, wxEXPAND);
		}
		else
			CodeEditor->Show();
	};

	if (!fileExt.empty())
	{
		// verify if file is a image
		wxImage fileImage = wxImage();
		if (fileImage.CanRead(path))
		{

			fileImage.LoadFile(path, wxBITMAP_DEFAULT_TYPE);
			if (!fileImage.IsOk())
				return;

			// resize image if larger than 1000 pixels
			if (fileImage.GetWidth() > 1000 || fileImage.GetHeight() > 1000)
			{
				fileImage.Rescale(fileImage.GetWidth() / 2, fileImage.GetHeight() / 2);
			}
			// creating a image container
			wxVector<wxBitmap> bitmaps;
			bitmaps.push_back(wxBitmap(fileImage));
			wxStaticBitmap *image_container = new wxStaticBitmap(mainCode, wxID_ANY, wxBitmapBundle::FromBitmaps(bitmaps));
			image_container->SetLabel(path + "_imageContainer");

			// updating the main container and status bar
			mainCode->GetSizer()->Add(image_container, 1, wxALIGN_CENTER);
			statusBar->UpdateComponents(path, "image", "img");
		}
		else
			LoadCodeEditor();
	}
	else
		LoadCodeEditor();

	// updating main container
	mainCode->GetSizer()->Layout();
}

void FilesTree::ToggleDir(wxMouseEvent &event)
{
	auto dirContainer = ((wxWindow *)event.GetEventObject());

	// getting the main container of the dir component if the event was received by another component
	if (dirContainer->GetLabel() == "dir_props")
	{
		dirContainer = dirContainer->GetParent();
	}
	else if (dirContainer->GetName() == "dir_name")
	{
		dirContainer = dirContainer->GetGrandParent();
	}

	// getting arrow icon and dir container childrens
	auto dir_arrow_ctn = ((wxStaticBitmap *)dirContainer->GetChildren()[0]->GetChildren()[0]);
	auto dir_childrens = dirContainer->GetChildren()[1];

	wxString path = dirContainer->GetName();
	projectSettings.SetCurrentlyMenuDir(path.ToStdString());

	if (!wxDirExists(path))
	{
		wxWindow *parent = dirContainer->GetParent();
		dirContainer->Destroy();
		FitContainer(parent);
		return;
	}

	if (dir_childrens && dir_arrow_ctn)
	{
		// rotate the arrow icon
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

			// load the dir content
			Load(dir_childrens, path.ToStdString());
		}

		// set new arrow icon
		dir_arrow_ctn->SetBitmap(wxBitmapBundle::FromBitmaps(bitmaps));

		// calling a function to resize the container to best size
		FitContainer(dir_childrens);
	}
}

void FilesTree::onTopMenuClick(wxMouseEvent &WXUNUSED(event))
{
	wxMenu *menuFile = new wxMenu;
	menuFile->Append(wxID_ANY, _("&New File"));
	menuFile->Append(+Event::File::CreateDir, _("&New Folder"));
	menuFile->Append(wxID_ANY, _("&Open terminal"));
	PopupMenu(menuFile);
}

void FilesTree::onFileRightClick(wxMouseEvent &event)
{
	auto target = ((wxWindow *)event.GetEventObject());
	if (!target->GetName().size())
		return;

	ProjectSettings::Get().SetCurrentlyMenuFile(target->GetName().ToStdString());

	wxMenu *menuFile = new wxMenu;
	menuFile->Append(+Event::File::RenameFile, _("&Rename"));
	menuFile->Append(+Event::File::DeleteFileEvent, _("&Delete File"));
	PopupMenu(menuFile);
}

void FilesTree::onDirRightClick(wxMouseEvent &event)
{
	auto eventObj = ((wxWindow *)event.GetEventObject());
	wxWindow *target = eventObj;

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
	ProjectSettings::Get().SetCurrentlyMenuDir(target->GetName().ToStdString());

	wxMenu *menuDir = new wxMenu;
	menuDir->Append(+Event::File::RenameDir, _("&Rename"));
	menuDir->Append(+Event::File::CreateFileEvent, _("&New File"));
	menuDir->Append(+Event::File::CreateDir, _("&New Folder"));
	menuDir->Append(+Event::File::DeleteDir, _("&Delete Folder"));
	PopupMenu(menuDir);
}

void FilesTree::OnPaint(wxPaintEvent &event)
{
	auto target = ((wxPanel *)event.GetEventObject());
	if (target->GetId() == +GUI::ControlID::FilesTree)
		return;

	auto border_color = Theme["selectedFile"].template get<std::string>();

	wxClientDC dc(this);
	wxGraphicsContext *gc = wxGraphicsContext::Create(dc);
	if (!gc)
		return;

	if (target->GetId() == +GUI::ControlID::CodeSearchPanel)
	{
		dc.SetPen(wxPen(wxColor(border_color), 0.20));
		dc.DrawLine(0, target->GetPosition().y, target->GetSize().GetWidth(), target->GetPosition().y);
		dc.DrawLine(0, target->GetPosition().y + target->GetSize().GetHeight(), target->GetSize().GetWidth(), target->GetPosition().y + target->GetSize().GetHeight());
	}
	delete gc;
}

void FilesTree::FitContainer(wxWindow *window)
{
	wxWindow *parent = window;

	if (!window->IsShownOnScreen())
	{
		while (parent->GetId() != +GUI::ControlID::ProjectFilesContainer)
		{
			if (!parent->IsShownOnScreen())
			{
				parent->SetSize(wxSize(0, 0));
				parent->SetMinSize(wxSize(0, 0));
			}

			parent->GetSizer()->Layout();
			parent->Update();
			parent->Refresh();

			wxSize thisSize = wxSize(parent->GetSize().x, 20);
			for (auto &&children : parent->GetChildren())
			{
				if (children->IsShownOnScreen())
				{
					thisSize.SetHeight(thisSize.y + children->GetSize().y);
				}
			}

			parent->Update();
			parent->Refresh();

			parent = parent->GetParent();

			parent->SetMinSize(thisSize);
			parent->GetSizer()->Layout();
			parent->Update();
			parent->Refresh();

			parent = parent->GetParent();
		}
	}
	else
	{
		while (parent->GetId() != +GUI::ControlID::ProjectFilesContainer)
		{
			parent->SetMinSize(wxSize(parent->GetBestSize()));
			parent->GetSizer()->Layout();
			parent = parent->GetParent();
		}
	}

	projectFilesContainer->GetSizer()->Layout();
	projectFilesContainer->FitInside();
}

void FilesTree::OnCreateDir(wxCommandEvent &WXUNUSED(event))
{
	try
	{
		wxString newDirName = wxGetTextFromUser("Enter dir name: ", "Create Dir", "");
		if (newDirName.IsEmpty())
		{
			wxMessageBox("the file name is dont valid");
			return throw 0;
		}

		if (!wxDirExists(ProjectSettings::Get().GetCurrentlyMenuDir()))
		{
			return throw 0;
		}

		bool created = FileOperations::CreateDir(ProjectSettings::Get().GetCurrentlyMenuDir() + newDirName);

		ProjectSettings::Get().SetCurrentlyMenuDir(ProjectSettings::Get().GetCurrentlyMenuDir() + newDirName);

		if (!created)
			return throw 0;
	}
	catch (...)
	{
		wxMessageBox(_("An error occurred while creating the dir"), _("Create Dir"), wxOK | wxICON_INFORMATION, this);
		return;
	}
}

void FilesTree::OnCreateFile(wxCommandEvent &WXUNUSED(event))
{
	try
	{
		wxString newFileName = wxGetTextFromUser("Enter File Name: ", "Create File", "");
		if (newFileName.IsEmpty())
		{
			wxMessageBox("the file name is dont valid");
			return throw 0;
		}

		if (!wxDirExists(ProjectSettings::Get().GetCurrentlyMenuDir()))
		{
			return throw 0;
		}

		bool created = FileOperations::CreateFileK(ProjectSettings::Get().GetCurrentlyMenuDir() + newFileName);

		if (created)
			OpenFile(wxString(ProjectSettings::Get().GetCurrentlyMenuDir()) + newFileName);
		else
			return throw 0;
	}
	catch (...)
	{
		wxMessageBox(_("An error occurred while creating the file"), _("Create File"), wxOK | wxICON_INFORMATION, this);
		return;
	}
}

void FilesTree::OnDeleteDir(wxCommandEvent &WXUNUSED(event))
{
	auto confirmDialog = wxMessageDialog(NULL, "Are you sure you want to delete this folder?", "Delete Folder", wxOK | wxCANCEL);
	if (confirmDialog.ShowModal() == wxID_OK)
	{
		FileOperations::DeleteDir(ProjectSettings::Get().GetCurrentlyMenuDir());

		if (wxDirExists(ProjectSettings::Get().GetCurrentlyMenuDir()))
		{
			wxLogError("An error occurred while deleting the folder");
			return;
		}

		auto targetComp = wxFindWindowByLabel(ProjectSettings::Get().GetCurrentlyMenuDir() + "_dir_container");
		if (!targetComp)
			return;

		wxWindow *parent = targetComp->GetParent();

		targetComp->Destroy();
		ProjectSettings::Get().SetCurrentlyMenuDir(ProjectSettings::Get().GetProjectPath());

		FitContainer(parent);
	}
}

void FilesTree::OnDeleteFile(wxCommandEvent &WXUNUSED(event))
{
	auto deleteFile = []()
	{
		FileOperations::DeleteFileK(ProjectSettings::Get().GetCurrentlyMenuFile());

		if (wxFileExists(ProjectSettings::Get().GetCurrentlyMenuFile()))
			wxLogError("An error occurred while deleting the file");
	};

	if (UserSettings["dontAskMeAgainFileDelete"] == false)
	{
		ConfirmDialog dlg(NULL, "Are you sure you want to delete this file?", "Delete File");
		int result = dlg.ShowModal();

		if (result == wxID_OK)
		{
			bool dontAsk = dlg.DontAskAgain();
			if (dontAsk)
			{
				UserSettings["dontAskMeAgainFileDelete"] = true;
				UserSettingsManager::Get().Update(UserSettings);
			}
			deleteFile();
		}
	}
	else
	{
		deleteFile();
	}
}

void FilesTree::OnComponentModified(int type, wxString oldPath, wxString newPath)
{
	// getting parent component
	wxString parentCompPath = newPath.ToStdString().substr(0, newPath.ToStdString().find_last_of(PlatformInfos::OsPathSepareator()));
	parentCompPath = parentCompPath + PlatformInfos::OsPathSepareator();
	auto parentComp = wxFindWindowByLabel(parentCompPath + "_dir_childrens");

	if (parentCompPath == projectSettings.GetProjectPath())
		parentComp = projectFilesContainer;

	if (!parentComp)
		return;

	// getting linked code editor
	auto linkedCodeEditor = ((CodeContainer *)FindWindowByName(oldPath + "_CodeEditor"));
	// getting linked tab
	auto linkedTab = wxFindWindowByLabel(oldPath + "_tab");

	bool isFile = true;
	if (std::filesystem::is_directory(newPath.ToStdString()))
		isFile = false;

	auto CreateWithPosition = [=]()
	{
		int position = 0;

		const std::filesystem::path pd{wxFileName(newPath).GetPath().ToStdString()};
		for (auto const &dir_entry : std::filesystem::directory_iterator{pd})
		{
			if (dir_entry.path() == newPath.ToStdString())
			{
				if (isFile)
					CreateFile(parentComp, wxFileNameFromPath(newPath), newPath);
				else
					CreateDir(parentComp, wxFileNameFromPath(newPath), newPath, position);
			}
			position++;
		}
	};

	if (type == wxFSW_EVENT_CREATE)
	{
		CreateWithPosition();
	}

	// getting the target component
	wxWindow *targetComp;
	if (isFile)
		targetComp = wxFindWindowByLabel(oldPath + "_file_container");
	else
		targetComp = wxFindWindowByLabel(oldPath + PlatformInfos::OsPathSepareator() + "_dir_container");
	if (!targetComp)
		return;

	if (type == wxFSW_EVENT_DELETE)
	{
		if (linkedTab)
		{
			auto tabs = ((Tabs *)FindWindowById(+GUI::ControlID::Tabs));
			if (tabs)
			{
				tabs->Close(linkedTab, linkedTab->GetName());
			}
		}
		targetComp->Destroy();
	}

	if (type == wxFSW_EVENT_RENAME || type == wxFSW_EVENT_MODIFY)
	{
		if (targetComp->GetName() != newPath && oldPath != newPath)
		{
			targetComp->Destroy();
			if (isFile)
			{
				CreateFile(parentComp, wxFileNameFromPath(newPath), newPath);

				// updating the linked code editor
				if (linkedCodeEditor)
					linkedCodeEditor->LoadPath(newPath);

				// updating the linked tabs
				if (linkedTab)
				{
					linkedTab->SetName(newPath);
					linkedTab->SetLabel(newPath + "_tab");

					// updating tab display name
					wxStaticText *tabName = ((wxStaticText *)linkedTab->GetChildren()[0]->GetChildren()[1]);
					if (tabName)
					{
						tabName->SetName(newPath);
						tabName->SetLabel(wxFileNameFromPath(newPath));
					}
				}
			}
			else
				CreateWithPosition();
		}
	}
	FitContainer(parentComp);
}

void FilesTree::OnEnterComp(wxMouseEvent &event)
{
	// event to set an accent color for the component on mouse focus
	auto target = ((wxWindow *)event.GetEventObject());
	if (selectedFile)
	{
		if (selectedFile->GetName() == target->GetName())
			return;
	}
	auto color = Theme["selectedFile"].template get<std::string>();

	auto fileContainer = FindWindowByLabel(target->GetName() + "_file_container");
	if (!fileContainer)
		return;

	fileContainer->SetBackgroundColour(wxColor(color));
	fileContainer->Refresh();
}

void FilesTree::OnLeaveComp(wxMouseEvent &event)
{
	// event to set a color for the component outside of mouse focus
	auto target = ((wxWindow *)event.GetEventObject());
	if (selectedFile)
	{
		if (selectedFile->GetName() == target->GetName())
			return;
	}
	auto color = Theme["main"].template get<std::string>();

	auto fileContainer = FindWindowByLabel(target->GetName() + "_file_container");
	if (!fileContainer)
		return;

	fileContainer->SetBackgroundColour(wxColor(color));
	fileContainer->Refresh();
}

void FilesTree::OnFileRename(wxCommandEvent &WXUNUSED(event))
{
	// getting user input
	wxString newFileName = wxGetTextFromUser("Enter file name: ", "Rename File", wxFileNameFromPath(ProjectSettings::Get().GetCurrentlyMenuFile()));
	if (newFileName.IsEmpty() || ProjectSettings::Get().GetCurrentlyMenuFile().IsEmpty())
		return;

	// creating new path
	wxString parentPath = ProjectSettings::Get().GetCurrentlyMenuFile().substr(0, ProjectSettings::Get().GetCurrentlyMenuFile().find_last_of(PlatformInfos::OsPathSepareator()) + 1);
	wxString newPath = parentPath + newFileName;

	// renaming file
	wxRename(wxString(ProjectSettings::Get().GetCurrentlyMenuFile()), newPath);

	ProjectSettings::Get().SetCurrentlyMenuFile(ProjectSettings::Get().GetProjectPath());

	if (!wxFileExists(newPath))
		wxMessageBox("An error occurred while renaming the file", "", wxOK | wxICON_INFORMATION);
}

void FilesTree::OnDirRename(wxCommandEvent &WXUNUSED(event))
{
	// getting user input
	wxString newDirName = wxGetTextFromUser("Enter folder name: ", "Rename Folder", wxFileNameFromPath(ProjectSettings::Get().GetCurrentlyMenuDir()));
	if (newDirName.IsEmpty() || ProjectSettings::Get().GetCurrentlyMenuDir().IsEmpty())
		return;

	// creating new path
	wxFileName dirPath(ProjectSettings::Get().GetCurrentlyMenuDir());
	dirPath.RemoveLastDir();
	wxString newPath = dirPath.GetFullPath() + newDirName;

	// renaming folder
	wxRename(ProjectSettings::Get().GetCurrentlyMenuDir(), newPath);

	if (!wxDirExists(newPath))
		wxMessageBox("An error occurred while renaming the folder", "", wxOK | wxICON_INFORMATION);
}

void FilesTree::SetSelectedFile(wxWindow *target)
{
	auto defaultFileColor = Theme["main"].template get<std::string>();
	auto selectedFileColor = Theme["selectedFile"].template get<std::string>();

	if (target)
	{
		if (selectedFile)
		{
			selectedFile->SetBackgroundColour(wxColor(defaultFileColor));
			selectedFile->Refresh();

			selectedFile = target;
			selectedFile->SetBackgroundColour(wxColor(selectedFileColor));
			selectedFile->Refresh();
		}
		else
		{
			selectedFile = target;
			selectedFile->SetBackgroundColour(wxColor(selectedFileColor));
			selectedFile->Refresh();
		}
	}
	else
	{
		if (selectedFile)
		{
			selectedFile->SetBackgroundColour(wxColor(defaultFileColor));
			selectedFile->Refresh();
		}
		selectedFile = target;
	}
}