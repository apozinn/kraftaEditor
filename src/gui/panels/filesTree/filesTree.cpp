#include "filesTree.hpp"

#include "fileOperations/fileOperations.hpp"
#include "themesManager/themesManager.hpp"
#include "appPaths/appPaths.hpp"
#include "appConstants/appConstants.hpp"
#include "ui/ids.hpp"
#include "projectSettings/projectSettings.hpp"
#include "userSettings/userSettings.hpp"
#include "platformInfos/platformInfos.hpp"
#include "languagesPreferences/languagesPreferences.hpp"
#include "errorMessages/errorMessages.hpp"
#include "menus/dirContextMenu.hpp"
#include "menus/fileContextMenu.hpp"

#include "gui/widgets/statusBar/statusBar.hpp"
#include "gui/panels/tabs/tabs.hpp"
#include "gui/codeContainer/code.hpp"
#include "gui/widgets/confirmDialog/confirmDialog.hpp"

#include <vector>
#include <wx/fswatcher.h>
#include <wx/scrolwin.h>
#include <wx/wfstream.h>
#include <wx/richtooltip.h>
#include <wx/statbmp.h>
#include <wx/graphics.h>
#include <wx/timer.h>

FilesTree::FilesTree(wxWindow *parent, wxWindowID ID)
	: wxPanel(parent, ID)
{
	wxSizer *m_sizer = new wxBoxSizer(wxVERTICAL);
	SetBackgroundColour(ThemesManager::Get().GetColor("main"));

	m_projectInformations = new wxPanel(this, +GUI::ControlID::ProjectToggler);
	wxSizer *m_projectInformations_sizer = new wxBoxSizer(wxHORIZONTAL);

	wxString arrowPath = ApplicationPaths::GetIconPath("dir_arrow.png");
	if (!arrowPath.IsEmpty())
	{
		wxBitmap arrowBitmap(arrowPath, wxBITMAP_TYPE_PNG);
		if (arrowBitmap.IsOk())
		{
			m_projectInformationsNameArrow = new wxStaticBitmap(
				m_projectInformations,
				+GUI::ControlID::ProjectToolsArrow,
				arrowBitmap);
			m_projectInformations_sizer->Add(m_projectInformationsNameArrow, 0, wxEXPAND);
		}
	}

	m_projectInformationsName = new wxStaticText(m_projectInformations, +GUI::ControlID::ProjectToolsName, ProjectSettings::Get().GetProjectName());
	wxFont newFont = m_projectInformationsName->GetFont();
	newFont.SetWeight(wxFONTWEIGHT_MEDIUM);
	m_projectInformationsName->SetFont(newFont);
	m_projectInformationsName->Refresh();
	m_projectInformations_sizer->Add(m_projectInformationsName, 1, wxEXPAND | wxLEFT, 4);

	m_projectInformations->SetSizerAndFit(m_projectInformations_sizer);
	m_projectInformations->SetMinSize(wxSize(GetSize().x, 15));
	m_sizer->Add(m_projectInformations, 0, wxEXPAND | wxTOP | wxBOTTOM, 5);

	if (!ProjectSettings::Get().IsProjectSet())
	{
		m_projectInformations->Hide();
	}

	m_projectFilesContainer = new wxScrolled<wxPanel>(this, +GUI::ControlID::ProjectFilesContainer);
	m_projectFilesContainer->SetScrollbars(20, 20, 50, 50);
	wxBoxSizer *m_projectFilesContainer_sizer = new wxBoxSizer(wxVERTICAL);
	m_projectFilesContainer->SetSizerAndFit(m_projectFilesContainer_sizer);
	m_sizer->Add(m_projectFilesContainer, 1, wxEXPAND);

	LinkClickEventToProjectInformationsComponents();
	SetSizerAndFit(m_sizer);
}

void FilesTree::LinkClickEventToProjectInformationsComponents()
{
	m_projectInformations->CallForEachChild(
		[this](wxWindow *children)
		{
			children->Bind(wxEVT_LEFT_DOWN, &FilesTree::ProjectInformationsLeftClick, this);
		});
}

void FilesTree::ProjectInformationsLeftClick(wxMouseEvent &WXUNUSED(event))
{
	auto arrow_bit = m_projectInformationsNameArrow->GetBitmap();
	wxVector<wxBitmap> bitmaps;

	if (m_projectFilesContainer->IsShown())
	{
		m_projectFilesContainer->Hide();
		bitmaps.push_back(wxBitmap(arrow_bit.ConvertToImage().Rotate90(true), -1));
	}
	else
	{
		m_projectFilesContainer->Show();
		bitmaps.push_back(wxBitmap(arrow_bit.ConvertToImage().Rotate90(false), -1));
	}

	m_projectInformationsNameArrow->SetBitmap(wxBitmapBundle::FromBitmaps(bitmaps));
}

void FilesTree::LoadProject(wxWindow *parent, const wxString &path)
{
	if (!parent)
		return;
	if (!wxDirExists(path))
	{
		wxMessageBox(ErrorMessages::InvalidProjectPath, "Error", wxOK | wxICON_ERROR);
		return;
	}

	m_projectFilesContainer->DestroyChildren();

	m_projectInformationsName->SetName(ProjectSettings::Get().GetProjectName());
	m_projectInformationsName->SetLabel(ProjectSettings::Get().GetProjectName());
	m_projectInformationsName->SetToolTip(ProjectSettings::Get().GetProjectPath());

	m_projectFilesContainer->SetName(ProjectSettings::Get().GetProjectPath());
	m_projectFilesContainer->SetLabel(ProjectSettings::Get().GetProjectPath() + "_project_files_container");

	ProjectSettings::Get().SetCurrentlyMenuDir(ProjectSettings::Get().GetProjectPath());
	ProjectSettings::Get().SetCurrentlyMenuFile(ProjectSettings::Get().GetProjectPath());

	m_projectInformations->Show();

	CreateDirectoryComponents(parent, path);

	m_projectFilesContainer->Layout();
	Layout();
}

void FilesTree::CloseProject()
{
	if (m_projectFilesContainer)
		m_projectFilesContainer->DestroyChildren();
	if (m_projectInformations)
		m_projectInformations->Hide();
}

void FilesTree::CreateDirectoryComponents(wxWindow *parent, const wxString &path)
{
	if (!parent)
		return;
	if (!wxDirExists(path))
	{
		wxMessageBox(ErrorMessages::CannotOpenDirForReadContent, "Error", wxOK | wxICON_ERROR);
		return;
	}

	std::vector<std::filesystem::directory_entry> directoryFiles;
	std::vector<std::filesystem::directory_entry> directoryFolders;

	for (auto const &dir_entry : std::filesystem::directory_iterator{path.ToStdString()})
	{
		if (dir_entry.is_directory())
			directoryFolders.push_back(dir_entry);
		else
			directoryFiles.push_back(dir_entry);
	}

	for (auto &folder : directoryFolders)
		CreateDirContainer(parent, wxString(folder.path()));

	for (auto &file : directoryFiles)
		CreateFileContainer(parent, wxString(file.path()));
}

wxWindow *FilesTree::CreateFileContainer(wxWindow *parent, const wxString &path)
{
	if (!wxFileExists(path))
	{
		wxMessageBox(ErrorMessages::FileNotFound, "Error", wxOK | wxICON_ERROR);
		return nullptr;
	}
	if (auto fileAlredyExists = FindWindowByLabel(path + "_file_container"))
		return fileAlredyExists;
	if (!parent)
		return nullptr;

	wxSizer *parentSizer = parent->GetSizer();
	wxPanel *fileContainer = new wxPanel(parent);
	wxBoxSizer *fileContainer_sizer = new wxBoxSizer(wxHORIZONTAL);

	fileContainer->SetToolTip(path);

	fileContainer->Bind(wxEVT_LEFT_UP, &FilesTree::OnFileLeftClick, this);
	fileContainer->Bind(wxEVT_RIGHT_UP, &FilesTree::OnFileRightClick, this);

	fileContainer->SetName(path);
	fileContainer->SetLabel(path + "_file_container");

	wxVector<wxBitmap> bitmaps;
	wxFileName pathProps(path);
	wxString fileExt = pathProps.GetExt().Lower();

	wxString fileIconPath = wxEmptyString;
	wxImage fileImage = wxImage();
	if (fileImage.CanRead(path))
		fileIconPath = ApplicationPaths::GetLanguageIcon("image");
	else
		fileIconPath = LanguagesPreferences::Get().GetLanguageIconPath(path);

	if (!fileIconPath.IsEmpty())
	{
		wxBitmap fileIconBitmap(fileIconPath, wxBITMAP_TYPE_PNG);
		if (fileIconBitmap.IsOk())
		{
			auto fileIcon = new wxStaticBitmap(
				fileContainer,
				wxID_ANY,
				fileIconBitmap);
			fileContainer_sizer->Add(fileIcon, 0, wxALIGN_CENTRE_VERTICAL | wxLEFT, 8);
		}
	}

	wxStaticText *fileName = new wxStaticText(fileContainer, wxID_ANY, wxFileNameFromPath(path));

	fileName->SetName(path);
	fileName->Bind(wxEVT_LEFT_UP, &FilesTree::OnFileLeftClick, this);
	fileName->Bind(wxEVT_RIGHT_UP, &FilesTree::OnFileRightClick, this);

	fileContainer_sizer->Add(fileName, 0, wxALIGN_CENTRE_VERTICAL | wxLEFT, 5);
	fileContainer->SetSizerAndFit(fileContainer_sizer);

	parentSizer->Add(fileContainer, 0, wxEXPAND | wxLEFT | wxTOP, 2);
	parentSizer->Layout();

	fileContainer->CallForEachChild(
		[this](wxWindow *win)
		{
			win->Bind(wxEVT_ENTER_WINDOW, &FilesTree::OnComponentMouseEnter, this);
			win->Bind(wxEVT_LEAVE_WINDOW, &FilesTree::OnComponentMouseExit, this);
		});

	return fileContainer;
}

wxWindow *FilesTree::CreateDirContainer(wxWindow *parent, wxString path, bool withPosition, int pos)
{
	if (wxString(path.Last()) != PlatformInfos::OsPathSepareator())
		path.Append(PlatformInfos::OsPathSepareator());

	if (!parent)
		return nullptr;

	if (!wxDirExists(path))
	{
		wxMessageBox(ErrorMessages::CannotOpenDirForReadContent, "Error", wxOK | wxICON_ERROR);
		return nullptr;
	}

	if (FindWindowByLabel(path + "_dir_container"))
		return nullptr;

	wxSizer *parentSizer = parent->GetSizer();
	wxPanel *dirContainer = new wxPanel(parent);

	dirContainer->SetMinSize(wxSize(dirContainer->GetSize().GetWidth(), 20));
	dirContainer->SetSize(dirContainer->GetSize().GetWidth(), 20);

	wxBoxSizer *dirContainer_sizer = new wxBoxSizer(wxVERTICAL);

	dirContainer->SetToolTip(path);

	dirContainer->Bind(wxEVT_LEFT_UP, &FilesTree::OnDirLeftClick, this);
	dirContainer->Bind(wxEVT_RIGHT_UP, &FilesTree::OnDirRightClick, this);

	dirContainer->SetName(path);
	dirContainer->SetLabel(path + "_dir_container");

	wxPanel *dirContainer_props = new wxPanel(dirContainer);
	wxBoxSizer *dirContainer_props_sizer = new wxBoxSizer(wxHORIZONTAL);

	dirContainer_props->SetToolTip(path);
	dirContainer_props->SetLabel("dir_props");

	dirContainer_props->Bind(wxEVT_LEFT_UP, &FilesTree::OnDirLeftClick, this);

	wxString arrowPath = ApplicationPaths::GetIconPath("dir_arrow.png");
	if (!arrowPath.IsEmpty())
	{
		wxBitmap arrowBitmap(arrowPath, wxBITMAP_TYPE_PNG);
		if (arrowBitmap.IsOk())
		{
			auto dirContainer_arrow = new wxStaticBitmap(
				dirContainer_props,
				wxID_ANY,
				arrowBitmap);
			dirContainer_props_sizer->Add(dirContainer_arrow, 0, wxEXPAND);
		}
	}

	wxString name = path;
	name.RemoveLast();

	wxStaticText *dirContainer_name = new wxStaticText(dirContainer_props, wxID_ANY, wxFileNameFromPath(name));
	dirContainer_name->SetName("dir_name");

	dirContainer_name->Bind(wxEVT_LEFT_UP, &FilesTree::OnDirLeftClick, this);
	dirContainer_name->Bind(wxEVT_RIGHT_UP, &FilesTree::OnDirRightClick, this);

	dirContainer_props_sizer->Add(dirContainer_name, 0, wxEXPAND | wxLEFT, 4);
	dirContainer_props->SetSizerAndFit(dirContainer_props_sizer);

	dirContainer_sizer->Add(dirContainer_props, 0, wxEXPAND | wxLEFT, 8);

	wxPanel *dirChildrens = new wxPanel(dirContainer);
	wxBoxSizer *dirChildrens_sizer = new wxBoxSizer(wxVERTICAL);

	dirChildrens->SetName(path);
	dirChildrens->SetLabel(path + "_dir_childrens");

	dirChildrens->Bind(wxEVT_PAINT, &FilesTree::OnDirChildrensPaint, this);

	dirChildrens->SetSizerAndFit(dirChildrens_sizer);
	dirContainer_sizer->Add(dirChildrens, 0, wxEXPAND | wxLEFT, 10);

	dirContainer->SetSizerAndFit(dirContainer_sizer);
	dirChildrens->Hide();

	if (withPosition)
		parentSizer->Insert(pos, dirContainer, 0, wxEXPAND | wxLEFT, 2);
	else
		parentSizer->Add(dirContainer, 0, wxEXPAND | wxLEFT, 2);

	parent->Layout();
	return dirContainer;
}

void FilesTree::OnDirChildrensPaint(wxPaintEvent &event)
{
	auto target = ((wxPanel *)event.GetEventObject());
	if (!target)
		return;

	auto color = ThemesManager::Get().GetColor("border");
	wxPaintDC dc(target);

	dc.SetPen(wxPen(wxColor(color), 1.25, wxPENSTYLE_DOT));
	dc.SetBrush(color);
	dc.DrawLine(wxPoint(0, 0), wxPoint(0, target->GetSize().y));
}

void FilesTree::OnFileLeftClick(wxMouseEvent &event)
{
	auto file = ((wxWindow *)event.GetEventObject());
	if (!file)
		return;

	wxString path = file->GetName();
	if (!wxFileExists(path))
	{
		wxMessageBox(ErrorMessages::FileNotFound, "Error", wxOK | wxICON_ERROR);
		return;
	}

	if (OpenFile(path))
		SetFileHighlight(path);
}

bool FilesTree::OpenFile(const wxString &componentIdentifier)
{
	auto mainCode = FindWindowById(+GUI::ControlID::MainCode);
	auto tabsContainer = ((Tabs *)FindWindowById(+GUI::ControlID::Tabs));
	auto statusBar = ((StatusBar *)FindWindowById(+GUI::ControlID::StatusBar));

	if (!mainCode || !tabsContainer || !statusBar)
	{
		wxMessageBox(ErrorMessages::ComponentFindError, "Error", wxOK | wxICON_ERROR);
		return false;
	}

	if (!wxFileExists(componentIdentifier))
	{
		wxMessageBox(ErrorMessages::FileNotFound, "Error", wxOK | wxICON_ERROR);
		return false;
	}

	for (auto &&other_ct : mainCode->GetChildren())
	{
		if (other_ct->GetId() != +GUI::ControlID::Tabs)
			other_ct->Hide();
	}

	tabsContainer->Add(wxFileNameFromPath(componentIdentifier), componentIdentifier);

	auto LoadCodeEditor = [&]()
	{
		auto CodeEditor = ((CodeContainer *)wxFindWindowByLabel(componentIdentifier + "_codeEditor"));
		if (!CodeEditor)
		{
			CodeEditor = new CodeContainer(mainCode, componentIdentifier);
			mainCode->GetSizer()->Add(CodeEditor, 1, wxEXPAND);
		}
		else
			CodeEditor->Show();
	};

	wxImage fileImage = wxImage();
	if (fileImage.CanRead(componentIdentifier))
	{
		fileImage.LoadFile(componentIdentifier);
		if (!fileImage.IsOk())
			return false;

		// resize image if larger than 1000 pixel
		if (fileImage.GetWidth() > 1000 || fileImage.GetHeight() > 1000)
			fileImage.Rescale(fileImage.GetWidth() / 2, fileImage.GetHeight() / 2);

		auto imageContainer = new wxStaticBitmap(
			mainCode,
			wxID_ANY,
			fileImage);

		imageContainer->SetLabel(componentIdentifier + "_imageContainer");
		mainCode->GetSizer()->Add(imageContainer, 1, wxALIGN_CENTER);
		statusBar->UpdateComponents(componentIdentifier, "image", "img");
	}
	else
		LoadCodeEditor();

	mainCode->Layout();

	wxFileName fullPath(componentIdentifier);
	wxString parentPath = fullPath.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR);
	ProjectSettings::Get().SetCurrentlyMenuDir(parentPath);
	ProjectSettings::Get().SetCurrentlyMenuFile(componentIdentifier);

	return true;
}

void FilesTree::OnFileRightClick(wxMouseEvent &event)
{
	auto target = ((wxWindow *)event.GetEventObject());
	if (!target)
		return;

	if (!wxFileExists(target->GetName()))
	{
		wxMessageBox(ErrorMessages::FileNotFound, "Error", wxOK | wxICON_ERROR);
		return;
	}

	ProjectSettings::Get().SetCurrentlyMenuFile(target->GetName());
	auto menuFile = FileContextMenu::Get();
	if (!menuFile)
	{
		wxMessageBox(ErrorMessages::CreateMenuContextError, "Error", wxOK | wxICON_ERROR);
		return;
	}

	PopupMenu(menuFile);
}

void FilesTree::OnDirLeftClick(wxMouseEvent &event)
{
	auto target = ((wxWindow *)event.GetEventObject());
	if (!target)
		return;

	if (target->GetLabel() == "dir_props")
		target = target->GetParent();
	else if (target->GetName() == "dir_name")
		target = target->GetGrandParent();

	if (!wxDirExists(target->GetName()))
	{
		wxMessageBox(ErrorMessages::CannotOpenDir, "Error", wxOK | wxICON_ERROR);
		return;
	}

	ToggleDirVisibility(target->GetName());
}

void FilesTree::OnDirRightClick(wxMouseEvent &event)
{
	auto target = ((wxWindow *)event.GetEventObject());
	if (!target)
		return;

	if (target->GetLabel() == "dir_props")
		target = target->GetParent();
	else if (target->GetName() == "dir_name")
		target = target->GetGrandParent();

	if (!wxDirExists(target->GetName()))
	{
		wxMessageBox(ErrorMessages::CannotOpenDir, "Error", wxOK | wxICON_ERROR);
		return;
	}

	ProjectSettings::Get().SetCurrentlyMenuDir(target->GetName());

	auto menuDir = DirContextMenu::Get();
	if (!menuDir)
	{
		wxMessageBox(ErrorMessages::CreateMenuContextError, "Error", wxOK | wxICON_ERROR);
		return;
	}
	PopupMenu(menuDir);
}

void FilesTree::ToggleDirVisibility(const wxString &componentIdentifier, bool defaultShow)
{
	auto dirContainer = wxFindWindowByLabel(componentIdentifier + "_dir_container");
	if (!dirContainer)
	{
		wxMessageBox(ErrorMessages::CannotOpenDir, "Error", wxOK | wxICON_ERROR);
		return;
	}

	if (!wxDirExists(componentIdentifier))
	{
		auto parent = dirContainer->GetParent();
		if (parent)
		{
			dirContainer->Destroy();
			AdjustContainerSize(parent);
		}

		wxMessageBox(ErrorMessages::CannotOpenDir, "Error", wxOK | wxICON_ERROR);
		return;
	}

	auto dirArrowIcon = ((wxStaticBitmap *)dirContainer->GetChildren()[0]->GetChildren()[0]);
	auto dirChildrens = dirContainer->GetChildren()[1];

	ProjectSettings::Get().SetCurrentlyMenuDir(componentIdentifier);

	if (dirArrowIcon && dirChildrens)
	{
		auto arrow_bit = dirArrowIcon->GetBitmap();
		if (defaultShow)
		{
			dirChildrens->Show();
			return;
		}
		else if (!dirChildrens->IsShown())
		{
			dirArrowIcon->SetBitmap(wxBitmap(arrow_bit.ConvertToImage().Rotate90(true), -1));
			dirChildrens->Show();
			CreateDirectoryComponents(dirChildrens, componentIdentifier);
		}
		else if (dirChildrens->IsShown())
		{
			dirChildrens->Hide();
			dirArrowIcon->SetBitmap(wxBitmap(arrow_bit.ConvertToImage().Rotate90(false), -1));
		}

		AdjustContainerSize(dirChildrens);
	}
	else
	{
		wxMessageBox(ErrorMessages::ToggleDirectoryVisibilityError, "Error", wxOK | wxICON_ERROR);
	}
}

void FilesTree::OnPaint(wxPaintEvent &event)
{
	auto target = ((wxPanel *)event.GetEventObject());
	if (!target)
		return;
	if (target->GetId() == +GUI::ControlID::FilesTree)
		return;
}

void FilesTree::AdjustContainerSize(wxWindow *target)
{
	if (!target)
		return;

	wxWindow *parent = target;
	const int mainContainerId = +GUI::ControlID::ProjectFilesContainer;

	// Case 1: The directory is visible or has no children
	if (!parent->IsShownOnScreen() || parent->GetChildren().empty())
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

			wxSize thisSize = wxSize(parent->GetSize().x, 16);
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

	// Ensure the main project files container recalculates its layout
	if (m_projectFilesContainer)
	{
		m_projectFilesContainer->FitInside();
		m_projectFilesContainer->Layout();
	}
}

void FilesTree::OnCreateDirRequested(wxCommandEvent &WXUNUSED(event))
{
	try
	{
		wxString dirName = wxGetTextFromUser("Enter directory name: ", "Create Directory", "");
		if (dirName.IsEmpty())
		{
			wxMessageBox(ErrorMessages::DirNameIsNotValid, "Error", wxOK | wxICON_ERROR);
			return;
		}

		if (!wxDirExists(ProjectSettings::Get().GetCurrentlyMenuDir()))
		{
			wxMessageBox(ErrorMessages::CannotFindDirectoryParent, "Error", wxOK | wxICON_ERROR);
			return;
		}

		bool created = FileOperations::CreateDir(ProjectSettings::Get().GetCurrentlyMenuDir() + dirName);
		if (created)
		{
			ProjectSettings::Get().SetCurrentlyMenuDir(ProjectSettings::Get().GetCurrentlyMenuDir() + dirName + PlatformInfos::OsPathSepareator());
		}
		else
		{
			wxMessageBox(ErrorMessages::CreateDirRequestedError, "Error", wxOK | wxICON_ERROR);
		}
	}
	catch (const std::exception &err)
	{
		wxMessageBox(ErrorMessages::CreateDirRequestedError + err.what(), "Error", wxOK | wxICON_ERROR);
		return;
	}
}

void FilesTree::OnDeleteDirRequested(wxCommandEvent &WXUNUSED(event))
{
	try
	{
		auto deleteDir = [this]()
		{
			wxString currentlyMenuDir = ProjectSettings::Get().GetCurrentlyMenuDir();
			if (!wxDirExists(currentlyMenuDir))
			{
				wxMessageBox(ErrorMessages::DirNotFound, "Error", wxOK | wxICON_ERROR);
				return;
			}

			FileOperations::DeleteDir(currentlyMenuDir);

			if (wxDirExists(currentlyMenuDir))
			{
				wxMessageBox(ErrorMessages::DeleteDirRequestedError, "Error", wxOK | wxICON_ERROR);
				return;
			}

			auto targetComp = wxFindWindowByLabel(currentlyMenuDir + "_dir_container");
			if (!targetComp)
				return;

			wxWindow *parent = targetComp->GetParent();

			targetComp->Destroy();
			ProjectSettings::Get().SetCurrentlyMenuDir(ProjectSettings::Get().GetProjectPath());

			AdjustContainerSize(parent);
		};

		auto dontAskMeAgainDirDelete = UserSettingsManager::Get().GetSetting<bool>("dontAskMeAgainDirDelete");
		if (dontAskMeAgainDirDelete.found)
		{
			if (dontAskMeAgainDirDelete.value == false)
			{
				ConfirmDialog dlg(NULL, "Are you sure you want to delete this dir?", "Delete Dir");
				int result = dlg.ShowModal();

				if (result == wxID_OK)
				{
					bool dontAsk = dlg.DontAskAgain();
					if (dontAsk)
					{
						UserSettingsManager::Get().currentSettings["dontAskMeAgainDirDelete"] = true;
						UserSettingsManager::Get().Update(UserSettingsManager::Get().currentSettings);
					}
					deleteDir();
					return;
				}
			}
		}
		deleteDir();
	}
	catch (const std::exception &err)
	{
		wxMessageBox(ErrorMessages::DeleteDirRequestedError + err.what(), "Error", wxOK | wxICON_ERROR);
		return;
	}
}

void FilesTree::OnRenameDirRequested(wxCommandEvent &WXUNUSED(event))
{
	try
	{
		if (!wxDirExists(ProjectSettings::Get().GetCurrentlyMenuDir()))
		{
			wxMessageBox(ErrorMessages::DirNotFound, "Error", wxOK | wxICON_ERROR);
			return;
		}

		wxString newDirName = wxGetTextFromUser("Enter directory name: ", "Rename directory", wxFileNameFromPath(ProjectSettings::Get().GetCurrentlyMenuDir()));
		if (newDirName.IsEmpty())
		{
			wxMessageBox(ErrorMessages::DirNameIsNotValid, "Error", wxOK | wxICON_ERROR);
			return;
		}

		wxFileName dirPath(ProjectSettings::Get().GetCurrentlyMenuDir());
		dirPath.RemoveLastDir();
		wxString newPath = dirPath.GetFullPath() + newDirName;

		wxRename(wxString(ProjectSettings::Get().GetCurrentlyMenuDir()), newPath);

		if (!wxDirExists(newPath))
		{
			wxMessageBox(ErrorMessages::RenameDirRequestedError, "Error", wxOK | wxICON_ERROR);
			return;
		}

		ProjectSettings::Get().SetCurrentlyMenuDir(newPath);
	}
	catch (const std::exception &err)
	{
		wxMessageBox(ErrorMessages::RenameDirRequestedError + err.what(), "Error", wxOK | wxICON_ERROR);
		return;
	}
}

void FilesTree::OnCreateFileRequested(wxCommandEvent &WXUNUSED(event))
{
	try
	{
		wxString fileName = wxGetTextFromUser("Enter file name: ", "Create file", "");
		if (fileName.IsEmpty())
		{
			wxMessageBox(ErrorMessages::FileNameIsNotValid, "Error", wxOK | wxICON_ERROR);
			return;
		}

		if (!wxDirExists(ProjectSettings::Get().GetCurrentlyMenuDir()))
		{
			wxMessageBox(ErrorMessages::CannotFindDirectoryParent, "Error", wxOK | wxICON_ERROR);
			return;
		}

		wxString filePath = ProjectSettings::Get().GetCurrentlyMenuDir() + fileName;

		bool created = FileOperations::CreateFileK(filePath);
		if (created)
		{
			if (OpenFile(filePath))
			{
				wxFileName fullPath(filePath);
				wxString parentPath = fullPath.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR);

				ToggleDirVisibility(parentPath, true);

				ProjectSettings::Get().SetCurrentlyMenuFile(filePath);
			}
		}
		else
			wxMessageBox(ErrorMessages::CreateFileRequestedError, "Error", wxOK | wxICON_ERROR);
	}
	catch (const std::exception &err)
	{
		wxMessageBox(ErrorMessages::CreateFileRequestedError + err.what(), "Error", wxOK | wxICON_ERROR);
		return;
	}
}

void FilesTree::OnDeleteFileRequested(wxCommandEvent &WXUNUSED(event))
{
	try
	{
		auto deleteFile = []()
		{
			wxString currentlyMenuFile = ProjectSettings::Get().GetCurrentlyMenuFile();
			if (!wxFileExists(currentlyMenuFile))
			{
				wxMessageBox(ErrorMessages::FileNotFound, "Error", wxOK | wxICON_ERROR);
				return;
			}

			FileOperations::DeleteFileK(currentlyMenuFile);

			if (wxFileExists(currentlyMenuFile))
			{
				wxMessageBox(ErrorMessages::DeleteFileRequestedError, "Error", wxOK | wxICON_ERROR);
				return;
			}

			ProjectSettings::Get().SetCurrentlyMenuFile(ProjectSettings::Get().GetProjectPath());
		};

		auto dontAskMeAgainFileDelete = UserSettingsManager::Get().GetSetting<bool>("dontAskMeAgainFileDelete");
		if (dontAskMeAgainFileDelete.found)
		{
			if (dontAskMeAgainFileDelete.value == false)
			{
				ConfirmDialog dlg(NULL, "Are you sure you want to delete this file?", "Delete File");
				int result = dlg.ShowModal();

				if (result == wxID_OK)
				{
					bool dontAsk = dlg.DontAskAgain();
					if (dontAsk)
					{
						UserSettingsManager::Get().currentSettings["dontAskMeAgainFileDelete"] = true;
						UserSettingsManager::Get().Update(UserSettingsManager::Get().currentSettings);
					}
					deleteFile();
					return;
				}
			}
		}
		deleteFile();
	}
	catch (const std::exception &err)
	{
		wxMessageBox(ErrorMessages::DeleteFileRequestedError + err.what(), "Error", wxOK | wxICON_ERROR);
		return;
	}
}

void FilesTree::OnRenameFileRequested(wxCommandEvent &WXUNUSED(event))
{
	try
	{
		if (!wxFileExists(ProjectSettings::Get().GetCurrentlyMenuFile()))
		{
			wxMessageBox(ErrorMessages::FileNotFound, "Error", wxOK | wxICON_ERROR);
			return;
		}

		wxString newFileName = wxGetTextFromUser("Enter file name: ", "Rename File", wxFileNameFromPath(ProjectSettings::Get().GetCurrentlyMenuFile()));
		if (newFileName.IsEmpty())
		{
			wxMessageBox(ErrorMessages::FileNameIsNotValid, "Error", wxOK | wxICON_ERROR);
			return;
		}

		wxString parentPath = ProjectSettings::Get().GetCurrentlyMenuFile().substr(0, ProjectSettings::Get().GetCurrentlyMenuFile().find_last_of(PlatformInfos::OsPathSepareator()) + 1);
		wxString newPath = parentPath + newFileName;

		wxRename(wxString(ProjectSettings::Get().GetCurrentlyMenuFile()), newPath);

		if (!wxFileExists(newPath))
		{
			wxMessageBox(ErrorMessages::RenameFileRequestedError, "Error", wxOK | wxICON_ERROR);
			return;
		}

		ProjectSettings::Get().SetCurrentlyMenuFile(newPath);
	}
	catch (const std::exception &err)
	{
		wxMessageBox(ErrorMessages::RenameFileRequestedError + err.what(), "Error", wxOK | wxICON_ERROR);
		return;
	}
}

void FilesTree::OnFileSystemEvent(int type, const wxString &oldPath, wxString newPath)
{
	wxFileName fullPath(newPath);
	wxString parentComponentPath = fullPath.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR);

	auto parentComponent = wxFindWindowByLabel(parentComponentPath + "_dir_childrens");

	if (parentComponentPath == ProjectSettings::Get().GetProjectPath())
		parentComponent = m_projectFilesContainer;

	if (!parentComponent)
		return;

	auto linkedCodeEditor = ((CodeContainer *)FindWindowByName(oldPath + "_codeEditor"));
	auto linkedTab = wxFindWindowByLabel(oldPath + "_tab");

	bool isFile = true;
	if (std::filesystem::is_directory(newPath.ToStdString()))
		isFile = false;

	auto CreateWithPosition = [this, newPath, isFile, parentComponent]()
	{
		int position = 0;
		const std::filesystem::path pd{wxFileName(newPath).GetPath().ToStdString()};
		for (auto const &dir_entry : std::filesystem::directory_iterator{pd})
		{
			if (dir_entry.path() == newPath.ToStdString())
			{
				if (isFile)
					CreateFileContainer(parentComponent, newPath);
				else
					CreateDirContainer(parentComponent, newPath, true, position);
			}
			position++;
		}
	};

	if (type == wxFSW_EVENT_CREATE)
	{
		CreateWithPosition();
	}

	if (type == wxFSW_EVENT_DELETE)
	{
		auto component = wxFindWindowByLabel(oldPath + "_file_container");
		if (!component)
		{
			if (auto isDirComponent = wxFindWindowByLabel(oldPath + PlatformInfos::OsPathSepareator() + "_dir_container"))
				component = isDirComponent;
			else
				return;
		}

		if (linkedTab)
		{
			auto tabs = ((Tabs *)FindWindowById(+GUI::ControlID::Tabs));
			if (tabs)
				tabs->Close(linkedTab, linkedTab->GetName());
		}
		if (component)
			component->Destroy();
	}

	if (type == wxFSW_EVENT_RENAME || type == wxFSW_EVENT_MODIFY)
	{
		wxWindow *targetComp;
		if (isFile)
			targetComp = wxFindWindowByLabel(oldPath + "_file_container");
		else
			targetComp = wxFindWindowByLabel(oldPath + PlatformInfos::OsPathSepareator() + "_dir_container");

		if (!targetComp)
			return;

		if (targetComp->GetName() != newPath && oldPath != newPath)
		{
			targetComp->Destroy();

			if (isFile)
			{
				CreateFileContainer(parentComponent, newPath);

				if (linkedCodeEditor)
					linkedCodeEditor->LoadPath(newPath);

				if (linkedTab)
				{
					linkedTab->SetName(newPath);
					linkedTab->SetLabel(newPath + "_tab");

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
	AdjustContainerSize(parentComponent);
}

void FilesTree::ChangeFileBackground(const wxString &componentIdentifier, wxColour color)
{
	auto fileContainer = FindWindowByLabel(componentIdentifier + "_file_container");
	if (!fileContainer)
		return;

	if (m_currentSelectedFile)
	{
		if (m_currentSelectedFile->GetName() == fileContainer->GetName())
			return;
	}

	fileContainer->SetBackgroundColour(color);
	fileContainer->Refresh();
}

void FilesTree::OnComponentMouseEnter(wxMouseEvent &event)
{
	auto target = ((wxWindow *)event.GetEventObject());
	if (!target)
		return;

	auto highLigthFileColor = ThemesManager::Get().GetColor("selectedFile");
	ChangeFileBackground(target->GetName(), highLigthFileColor);
}

void FilesTree::OnComponentMouseExit(wxMouseEvent &event)
{
	auto target = ((wxWindow *)event.GetEventObject());
	if (!target)
		return;

	auto defaultFileColor = ThemesManager::Get().GetColor("main");
	ChangeFileBackground(target->GetName(), defaultFileColor);
}

void FilesTree::SetFileHighlight(const wxString &componentIdentifier)
{
	auto target = wxFindWindowByLabel(componentIdentifier + "_file_container");

	auto defaultFileColor = ThemesManager::Get().GetColor("main");
	auto selectedFileColor = ThemesManager::Get().GetColor("selectedFile");

	if (target)
	{
		if (m_currentSelectedFile)
		{
			m_currentSelectedFile->SetBackgroundColour(defaultFileColor);
			m_currentSelectedFile->Refresh();

			m_currentSelectedFile = target;
			m_currentSelectedFile->SetBackgroundColour(selectedFileColor);
			m_currentSelectedFile->Refresh();
		}
		else
		{
			m_currentSelectedFile = target;
			m_currentSelectedFile->SetBackgroundColour(selectedFileColor);
			m_currentSelectedFile->Refresh();
		}
	}
	else
	{
		if (m_currentSelectedFile)
		{
			m_currentSelectedFile->SetBackgroundColour(defaultFileColor);
			m_currentSelectedFile->Refresh();
		}
		m_currentSelectedFile = nullptr;
	}
}