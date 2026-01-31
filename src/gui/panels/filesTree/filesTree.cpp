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
    auto *mainSizer = new wxBoxSizer(wxVERTICAL);
    SetBackgroundColour(ThemesManager::Get().GetColor("main"));

    m_projectInformations = new wxPanel(this, +GUI::ControlID::ProjectToggler);
    m_projectInformations->SetName(ProjectSettings::Get().GetProjectPath());
    auto *projectInfoSizer = new wxBoxSizer(wxHORIZONTAL);

    m_projectInformations->Bind(wxEVT_RIGHT_UP, &FilesTree::OnDirRightClick, this);
    m_projectInformations->Bind(wxEVT_ENTER_WINDOW, &FilesTree::OnProjectInformationsHoverEnter, this);

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
            projectInfoSizer->Add(m_projectInformationsNameArrow, 0, wxEXPAND);
            m_projectInformationsNameArrow->Bind(wxEVT_RIGHT_UP, &FilesTree::OnDirRightClick, this);
        }
    }

    m_projectInformationsName = new wxStaticText(
        m_projectInformations,
        +GUI::ControlID::ProjectToolsName,
        ProjectSettings::Get().GetProjectName());

    m_projectInformationsName->Bind(wxEVT_RIGHT_UP, &FilesTree::OnDirRightClick, this);

    auto font = m_projectInformationsName->GetFont();
    font.SetWeight(wxFONTWEIGHT_MEDIUM);

    m_projectInformationsName->SetFont(font);
    m_projectInformationsName->Refresh();

    projectInfoSizer->Add(m_projectInformationsName, 0, wxLEFT, 4);

    m_projectInfosTools = new ProjectInfosTools(m_projectInformations, +GUI::ControlID::ProjectInfosTools);
    projectInfoSizer->Add(m_projectInfosTools, 1, wxALIGN_CENTER_VERTICAL);

    m_projectInformations->SetSizerAndFit(projectInfoSizer);
    m_projectInformations->SetMinSize(wxSize(GetSize().x, 20));

    mainSizer->Add(m_projectInformations, 0, wxEXPAND | wxTOP | wxBOTTOM, 5);

    if (!ProjectSettings::Get().IsProjectSet())
        m_projectInformations->Hide();

    m_projectFilesContainer = new wxScrolled<wxPanel>(this, +GUI::ControlID::ProjectFilesContainer);
    m_projectFilesContainer->SetScrollbars(20, 20, 50, 50);

    auto *filesContainerSizer = new wxBoxSizer(wxVERTICAL);
    m_projectFilesContainer->SetSizerAndFit(filesContainerSizer);
    mainSizer->Add(m_projectFilesContainer, 1, wxEXPAND);

    LinkClickEventToProjectInformationsComponents();
    SetSizerAndFit(mainSizer);
}

void FilesTree::LinkClickEventToProjectInformationsComponents()
{
    m_projectInformations->Bind(wxEVT_LEFT_DOWN, &FilesTree::ProjectInformationsLeftClick, this);
    m_projectInfosTools->Bind(wxEVT_LEFT_DOWN, &FilesTree::ProjectInformationsLeftClick, this);
    m_projectInformationsName->Bind(wxEVT_LEFT_DOWN, &FilesTree::ProjectInformationsLeftClick, this);
    m_projectInformationsNameArrow->Bind(wxEVT_LEFT_DOWN, &FilesTree::ProjectInformationsLeftClick, this);
}

void FilesTree::ProjectInformationsLeftClick(wxMouseEvent &)
{
    auto arrowBitmap = m_projectInformationsNameArrow->GetBitmap();
    wxVector<wxBitmap> bitmaps;

    if (m_projectFilesContainer->IsShown())
    {
        m_projectFilesContainer->Hide();
        bitmaps.push_back(wxBitmap(arrowBitmap.ConvertToImage().Rotate90(true), -1));
    }
    else
    {
        m_projectFilesContainer->Show();
        bitmaps.push_back(wxBitmap(arrowBitmap.ConvertToImage().Rotate90(false), -1));
    }

    m_projectInformationsNameArrow->SetBitmap(wxBitmapBundle::FromBitmaps(bitmaps));
}

void FilesTree::LoadProject(wxWindow *parent, wxString path)
{
    if (!parent)
        return;

    if (!wxDirExists(path))
    {
        wxMessageBox(ErrorMessages::InvalidProjectPath, "Error", wxOK | wxICON_ERROR);
        return;
    }

    if (path.Last() != PlatformInfos::OsPathSeparator())
        path.Append(PlatformInfos::OsPathSeparator());

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

    m_projectInfosTools->Hide();
}

void FilesTree::CloseProject()
{
    if (m_projectFilesContainer)
        m_projectFilesContainer->DestroyChildren();
    if (m_projectInformations)
        m_projectInformations->Hide();

    SetFileHighlight(wxEmptyString);
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

    std::vector<std::filesystem::directory_entry> folders, files;
    for (auto const &entry : std::filesystem::directory_iterator{path.ToStdString()})
    {
        if (entry.is_directory())
            folders.push_back(entry);
        else
            files.push_back(entry);
    }

    for (auto &folder : folders)
        CreateDirContainer(parent, wxString(folder.path()));
    for (auto &file : files)
        CreateFileContainer(parent, wxString(file.path()));
}

wxWindow *FilesTree::CreateFileContainer(wxWindow *parent, const wxString &path)
{
    if (!wxFileExists(path))
    {
        wxMessageBox(ErrorMessages::FileNotFound, "Error", wxOK | wxICON_ERROR);
        return nullptr;
    }
    if (auto exists = FindWindowByLabel(path + "_file_container"))
        return exists;
    if (!parent)
        return nullptr;

    auto *parentSizer = parent->GetSizer();
    auto *fileContainer = new wxPanel(parent);
    auto *fileSizer = new wxBoxSizer(wxHORIZONTAL);

    fileContainer->SetMinSize(wxSize(fileContainer->GetSize().GetWidth(), 20));
    fileContainer->SetSize(fileContainer->GetSize().GetWidth(), 20);
    fileContainer->SetToolTip(path);

    fileContainer->Bind(wxEVT_LEFT_UP, &FilesTree::OnFileLeftClick, this);
    fileContainer->Bind(wxEVT_RIGHT_UP, &FilesTree::OnFileRightClick, this);

    fileContainer->SetName(path);
    fileContainer->SetLabel(path + "_file_container");

    wxString iconPath;
    if (FileOperations::IsImageFile(path))
        iconPath = ApplicationPaths::GetLanguageIcon("image");
    else
        iconPath = LanguagesPreferences::Get().GetLanguageIconPath(path);

    if (!iconPath.IsEmpty())
    {
        wxBitmap iconBitmap(iconPath, wxBITMAP_TYPE_PNG);
        if (iconBitmap.IsOk())
        {
            auto *icon = new wxStaticBitmap(fileContainer, wxID_ANY, iconBitmap);
            fileSizer->Add(icon, 0, wxALIGN_CENTRE_VERTICAL | wxLEFT, 8);
        }
    }

    auto *fileName = new wxStaticText(fileContainer, wxID_ANY, wxFileNameFromPath(path));
    fileName->SetName(path);
    fileName->Bind(wxEVT_LEFT_UP, &FilesTree::OnFileLeftClick, this);
    fileName->Bind(wxEVT_RIGHT_UP, &FilesTree::OnFileRightClick, this);

    fileSizer->Add(fileName, 0, wxALIGN_CENTRE_VERTICAL | wxLEFT, 5);
    fileContainer->SetSizerAndFit(fileSizer);

    parentSizer->Add(fileContainer, 0, wxEXPAND | wxLEFT | wxTOP, 2);
    parentSizer->Layout();

    fileContainer->CallForEachChild([this](wxWindow *win)
                                    {
        win->Bind(wxEVT_ENTER_WINDOW, &FilesTree::OnComponentMouseEnter, this);
        win->Bind(wxEVT_LEAVE_WINDOW, &FilesTree::OnComponentMouseExit, this); });

    return fileContainer;
}

wxWindow *FilesTree::CreateDirContainer(wxWindow *parent, wxString path, bool withPosition, int pos)
{
    if (wxString(path.Last()) != PlatformInfos::OsPathSeparator())
        path.Append(PlatformInfos::OsPathSeparator());

    if (!parent)
        return nullptr;
    if (!wxDirExists(path))
    {
        wxMessageBox(ErrorMessages::CannotOpenDirForReadContent, "Error", wxOK | wxICON_ERROR);
        return nullptr;
    }
    if (FindWindowByLabel(path + "_dir_container"))
        return nullptr;

    auto *parentSizer = parent->GetSizer();
    auto *dirContainer = new wxPanel(parent);

    dirContainer->SetMinSize(wxSize(dirContainer->GetSize().GetWidth(), 20));
    dirContainer->SetSize(dirContainer->GetSize().GetWidth(), 20);

    auto *dirSizer = new wxBoxSizer(wxVERTICAL);
    dirContainer->SetToolTip(path);

    dirContainer->Bind(wxEVT_LEFT_UP, &FilesTree::OnDirLeftClick, this);
    dirContainer->Bind(wxEVT_RIGHT_UP, &FilesTree::OnDirRightClick, this);

    dirContainer->SetName(path);
    dirContainer->SetLabel(path + "_dir_container");

    auto *propsPanel = new wxPanel(dirContainer);
    auto *propsSizer = new wxBoxSizer(wxHORIZONTAL);

    propsPanel->SetToolTip(path);
    propsPanel->SetName(path);
    propsPanel->SetLabel("dir_props");
    propsPanel->Bind(wxEVT_LEFT_UP, &FilesTree::OnDirLeftClick, this);

    wxString arrowPath = ApplicationPaths::GetIconPath("dir_arrow.png");
    if (!arrowPath.IsEmpty())
    {
        wxBitmap arrowBitmap(arrowPath, wxBITMAP_TYPE_PNG);
        if (arrowBitmap.IsOk())
        {
            auto *arrow = new wxStaticBitmap(propsPanel, wxID_ANY, arrowBitmap);
            propsSizer->Add(arrow, 0, wxALIGN_CENTRE_VERTICAL);
        }
    }

    auto *dirName = new wxStaticText(propsPanel, wxID_ANY, wxFileNameFromPath(path.Clone().RemoveLast()));
    dirName->SetName("dir_name");
    dirName->Bind(wxEVT_LEFT_UP, &FilesTree::OnDirLeftClick, this);
    dirName->Bind(wxEVT_RIGHT_UP, &FilesTree::OnDirRightClick, this);

    propsSizer->Add(dirName, 0, wxEXPAND | wxLEFT, 4);
    propsPanel->SetSizerAndFit(propsSizer);

    dirSizer->Add(propsPanel, 0, wxEXPAND | wxLEFT, 8);

    auto *childrenPanel = new wxPanel(dirContainer);
    auto *childrenSizer = new wxBoxSizer(wxVERTICAL);

    childrenPanel->SetName(path);
    childrenPanel->SetLabel(path + "_dir_childrens");
    childrenPanel->Bind(wxEVT_PAINT, &FilesTree::OnDirChildrensPaint, this);

    childrenPanel->SetSizerAndFit(childrenSizer);
    dirSizer->Add(childrenPanel, 0, wxEXPAND | wxLEFT, 10);

    dirContainer->SetSizerAndFit(dirSizer);
    childrenPanel->Hide();

    if (withPosition)
        parentSizer->Insert(pos, dirContainer, 0, wxEXPAND | wxLEFT | wxTOP, 2);
    else
        parentSizer->Add(dirContainer, 0, wxEXPAND | wxLEFT | wxTOP, 2);

    parent->Layout();

    dirContainer->CallForEachChild([this](wxWindow *win)
                                   {
        win->Bind(wxEVT_ENTER_WINDOW, &FilesTree::OnComponentMouseEnter, this);
        win->Bind(wxEVT_LEAVE_WINDOW, &FilesTree::OnComponentMouseExit, this); });

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

    OpenFile(path);
}

bool FilesTree::OpenFile(const wxString &componentIdentifier, int line)
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

    auto hideOtherPanelsOfMainCode = [&](wxWindow *except)
    {
        for (auto &&other : mainCode->GetChildren())
        {
            if (other->GetId() != +GUI::ControlID::Tabs && other != except)
                other->Hide();
        }
        mainCode->Layout();
    };

    tabsContainer->Add(wxFileNameFromPath(componentIdentifier), componentIdentifier);

    auto LoadCodeEditor = [&]()
    {
        auto codeEditor = ((CodeContainer *)wxFindWindowByLabel(componentIdentifier + "_codeContainer"));
        if (!codeEditor)
        {
            codeEditor = new CodeContainer(mainCode, componentIdentifier);
            mainCode->GetSizer()->Add(codeEditor, 1, wxEXPAND);
            codeEditor->Show();
            mainCode->GetSizer()->Layout();
            mainCode->Update();
        }
        else
            codeEditor->Show();

        if (codeEditor && line)
        {
            ((wxStyledTextCtrl *)codeEditor->GetChildren()[0])->GotoLine(line);
        }

        hideOtherPanelsOfMainCode(codeEditor);
    };

    wxImage fileImage;
    if (FileOperations::IsImageFile(componentIdentifier))
    {
        fileImage.LoadFile(componentIdentifier);
        if (!fileImage.IsOk())
            return false;
        if (fileImage.GetWidth() > 1000 || fileImage.GetHeight() > 1000)
            fileImage.Rescale(fileImage.GetWidth() / 2, fileImage.GetHeight() / 2);

        auto imageContainer = new wxStaticBitmap(mainCode, wxID_ANY, fileImage);

        hideOtherPanelsOfMainCode(imageContainer);

        imageContainer->SetLabel(componentIdentifier + "_imageContainer");
        mainCode->GetSizer()->Add(imageContainer, 1, wxALIGN_CENTER);
        statusBar->UpdateComponents(componentIdentifier);
    }
    else
        LoadCodeEditor();

    for (auto &children : tabsContainer->GetChildren())
        children->Refresh();

    mainCode->Layout();

    wxFileName fullPath(componentIdentifier);
    wxString parentPath = fullPath.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR);

    ProjectSettings::Get().SetCurrentlyFileOpen(componentIdentifier);
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

    bool targetIsprojectInformations =
        target->GetId() == +GUI::ControlID::ProjectToggler ||
        target->GetId() == +GUI::ControlID::ProjectToolsName ||
        target->GetId() == +GUI::ControlID::ProjectToolsArrow;

    if (!wxDirExists(target->GetName()) && !targetIsprojectInformations)
    {
        wxMessageBox(ErrorMessages::CannotOpenDir, "Error", wxOK | wxICON_ERROR);
        return;
    }

    if (targetIsprojectInformations)
        ProjectSettings::Get().SetCurrentlyMenuDir(ProjectSettings::Get().GetProjectPath());
    else
        ProjectSettings::Get().SetCurrentlyMenuDir(target->GetName());

    auto menuDir = DirContextMenu::Get(targetIsprojectInformations);
    if (!menuDir)
    {
        wxMessageBox(ErrorMessages::CreateMenuContextError, "Error", wxOK | wxICON_ERROR);
        return;
    }

    PopupMenu(menuDir);
}

void FilesTree::ToggleDirVisibility(const wxString &componentIdentifier, bool defaultShow)
{
    auto dirContainer = FindWindowByLabel(componentIdentifier + "_dir_container");

    if (!dirContainer)
        return;

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
        auto arrowBitmap = dirArrowIcon->GetBitmap();
        if (defaultShow)
        {
            dirChildrens->Show();
            return;
        }
        else if (!dirChildrens->IsShown())
        {
            dirArrowIcon->SetBitmap(wxBitmap(arrowBitmap.ConvertToImage().Rotate90(true), -1));
            dirChildrens->Show();
            CreateDirectoryComponents(dirChildrens, componentIdentifier);
        }
        else if (dirChildrens->IsShown())
        {
            dirChildrens->Hide();
            dirArrowIcon->SetBitmap(wxBitmap(arrowBitmap.ConvertToImage().Rotate90(false), -1));
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

void FilesTree::AdjustContainerSize(wxWindow *target, bool reduceSize)
{
    if (!target)
        return;

    wxWindow *parent = target;
    const int mainContainerId = +GUI::ControlID::ProjectFilesContainer;

    if (!parent->IsShownOnScreen() || parent->GetChildren().empty() || reduceSize)
    {
        while (parent->GetId() != mainContainerId)
        {
            if (!parent->IsShownOnScreen())
            {
                parent->SetSize(wxSize(0, 0));
                parent->SetMinSize(wxSize(0, 0));
            }
            parent->Layout();

            wxSize size(parent->GetSize().x, 20);
            for (auto &&child : parent->GetChildren())
            {
                if (child->IsShownOnScreen())
                    size.SetHeight(size.y + child->GetSize().y + 2);
            }
            parent->Layout();

            parent = parent->GetParent();
            parent->SetMinSize(size);
            parent->Layout();
            parent = parent->GetParent();
        }
    }
    else
    {
        while (parent->GetId() != mainContainerId)
        {
            parent->SetMinSize(wxSize(parent->GetBestSize()));
            parent->GetSizer()->Layout();
            parent = parent->GetParent();
        }
    }

    if (m_projectFilesContainer)
    {
        m_projectFilesContainer->FitInside();
        m_projectFilesContainer->Layout();
    }
}

void FilesTree::OnCreateDirRequested(wxCommandEvent &)
{
    try
    {
        wxString dirName = wxGetTextFromUser("Enter directory name: ", "Create Directory", "");
        if (dirName.IsEmpty())
            return;

        if (!wxDirExists(ProjectSettings::Get().GetCurrentlyMenuDir()))
        {
            wxMessageBox(ErrorMessages::CannotFindDirectoryParent, "Error", wxOK | wxICON_ERROR);
            return;
        }
        bool created = FileOperations::CreateDir(ProjectSettings::Get().GetCurrentlyMenuDir() + dirName);
        if (created)
        {
            ProjectSettings::Get().SetCurrentlyMenuDir(ProjectSettings::Get().GetCurrentlyMenuDir() + dirName + PlatformInfos::OsPathSeparator());
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

void FilesTree::OnDeleteDirRequested(wxCommandEvent &)
{
    try
    {
        auto deleteDir = [this]()
        {
            wxString dir = ProjectSettings::Get().GetCurrentlyMenuDir();
            if (!wxDirExists(dir))
            {
                wxMessageBox(ErrorMessages::DirNotFound, "Error", wxOK | wxICON_ERROR);
                return;
            }
            FileOperations::DeleteDir(dir);
            if (wxDirExists(dir))
            {
                wxMessageBox(ErrorMessages::DeleteDirRequestedError, "Error", wxOK | wxICON_ERROR);
                return;
            }
            auto target = wxFindWindowByLabel(dir + "_dir_container");
            if (!target)
                return;
            wxWindow *parent = target->GetParent();
            target->Destroy();
            ProjectSettings::Get().SetCurrentlyMenuDir(ProjectSettings::Get().GetProjectPath());
            AdjustContainerSize(parent, true);
        };

        auto dontAsk = UserSettingsManager::Get().GetSetting<bool>("dontAskMeAgainDirDelete");
        if (dontAsk.found && !dontAsk.value)
        {
            ConfirmDialog dlg(NULL, "Are you sure you want to delete this dir?", "Delete Dir");
            int result = dlg.ShowModal();
            if (result == wxID_OK)
            {
                if (dlg.DontAskAgain())
                {
                    UserSettingsManager::Get().currentSettings["dontAskMeAgainDirDelete"] = true;
                    UserSettingsManager::Get().Update(UserSettingsManager::Get().currentSettings);
                }
                deleteDir();
                return;
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

void FilesTree::OnRenameDirRequested(wxCommandEvent &event)
{
    try
    {
        if (!wxDirExists(ProjectSettings::Get().GetCurrentlyMenuDir()))
        {
            wxMessageBox(ErrorMessages::DirNotFound, "Error", wxOK | wxICON_ERROR);
            return;
        }

        wxString newDirName = wxGetTextFromUser("Enter directory name: ", "Rename directory", wxFileNameFromPath(ProjectSettings::Get().GetCurrentlyMenuDir(true)));
        if (newDirName.IsEmpty())
            return;

        wxFileName dirPath(ProjectSettings::Get().GetCurrentlyMenuDir(true));
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

void FilesTree::OnCreateFileRequested(wxCommandEvent &)
{
    try
    {
        wxString fileName = wxGetTextFromUser("Enter file name: ", "Create file", "");
        if (fileName.IsEmpty())
            return;

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
                ProjectSettings::Get().SetCurrentlyMenuFile(filePath);
                ToggleDirVisibility(parentPath, true);
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

void FilesTree::OnDeleteFileRequested(wxCommandEvent &)
{
    try
    {
        auto deleteFile = []()
        {
            wxString file = ProjectSettings::Get().GetCurrentlyMenuFile();
            if (!wxFileExists(file))
            {
                wxMessageBox(ErrorMessages::FileNotFound, "Error", wxOK | wxICON_ERROR);
                return;
            }
            FileOperations::DeleteFileK(file);
            if (wxFileExists(file))
            {
                wxMessageBox(ErrorMessages::DeleteFileRequestedError, "Error", wxOK | wxICON_ERROR);
                return;
            }
            ProjectSettings::Get().SetCurrentlyMenuFile(ProjectSettings::Get().GetProjectPath());
        };

        auto dontAsk = UserSettingsManager::Get().GetSetting<bool>("dontAskMeAgainFileDelete");
        if (dontAsk.found && !dontAsk.value)
        {
            ConfirmDialog dlg(NULL, "Are you sure you want to delete this file?", "Delete File");
            int result = dlg.ShowModal();
            if (result == wxID_OK)
            {
                if (dlg.DontAskAgain())
                {
                    UserSettingsManager::Get().currentSettings["dontAskMeAgainFileDelete"] = true;
                    UserSettingsManager::Get().Update(UserSettingsManager::Get().currentSettings);
                }
                deleteFile();
                return;
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

void FilesTree::OnRenameFileRequested(wxCommandEvent &)
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
            return;

        wxString parentPath = ProjectSettings::Get().GetCurrentlyMenuFile().substr(0, ProjectSettings::Get().GetCurrentlyMenuFile().find_last_of(PlatformInfos::OsPathSeparator()) + 1);
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
    if (oldPath.IsEmpty() || newPath.IsEmpty())
        return;

    wxFileName fullPath(newPath);
    wxString parentPath;

    if (fullPath.IsDir())
    {
        fullPath.RemoveLastDir();
        parentPath = fullPath.GetPath() + PlatformInfos::OsPathSeparator();
    }
    else
    {
        parentPath = fullPath.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR);
    }

    auto parentComponent = wxFindWindowByLabel(parentPath + "_dir_childrens");
    if (parentPath == ProjectSettings::Get().GetProjectPath())
        parentComponent = m_projectFilesContainer;
    if (!parentComponent)
        return;

    auto linkedEditor = ((CodeContainer *)FindWindowByName(oldPath + "_codeContainer"));
    auto linkedTab = wxFindWindowByLabel(oldPath + "_tab");
    bool isFile = !std::filesystem::is_directory(newPath.ToStdString());

    auto CreateWithPosition = [this, newPath, isFile, parentComponent, parentPath]()
    {
        int position = 0;
        const std::filesystem::path pd{wxFileName(parentPath).GetPath().ToStdString()};
        for (auto const &entry : std::filesystem::directory_iterator{pd})
        {
            if (entry.is_directory())
            {
                std::string dir_path = std::string(entry.path()) + PlatformInfos::OsPathSeparator().ToStdString();
                if (dir_path == newPath.ToStdString())
                    CreateDirContainer(parentComponent, newPath, true, position);
            }
            else
            {
                std::string file_path = std::string(entry.path());
                if (file_path == newPath.ToStdString())
                    CreateFileContainer(parentComponent, newPath);
            }
            position++;
        }
    };

    if (type == wxFSW_EVENT_CREATE)
        CreateWithPosition();

    if (type == wxFSW_EVENT_DELETE)
    {
        auto component = wxFindWindowByLabel(oldPath + "_file_container");
        if (!component)
        {
            if (auto dirComp = wxFindWindowByLabel(oldPath + PlatformInfos::OsPathSeparator() + "_dir_container"))
                component = dirComp;
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
        AdjustContainerSize(parentComponent, true);
        return;
    }

    if (type == wxFSW_EVENT_RENAME || type == wxFSW_EVENT_MODIFY)
    {
        wxWindow *targetComp;
        if (isFile)
            targetComp = wxFindWindowByLabel(oldPath + "_file_container");
        else
            targetComp = wxFindWindowByLabel(oldPath + PlatformInfos::OsPathSeparator() + "_dir_container");
        if (!targetComp)
            return;

        if (targetComp->GetName() != newPath && oldPath != newPath)
        {
            if (isFile)
            {
                targetComp->SetName(newPath);
                targetComp->SetLabel(newPath + "_file_container");
                auto fileName = ((wxStaticText *)targetComp->GetChildren()[1]);
                if (fileName)
                {
                    fileName->SetName(newPath);
                    fileName->SetLabel(wxFileNameFromPath(newPath));
                }
                auto fileIcon = ((wxStaticBitmap *)targetComp->GetChildren()[0]);
                if (fileIcon)
                {
                    wxString iconPath = LanguagesPreferences::Get().GetLanguageIconPath(newPath);
                    if (!iconPath.IsEmpty())
                    {
                        wxBitmap iconBitmap(iconPath, wxBITMAP_TYPE_PNG);
                        if (iconBitmap.IsOk())
                            fileIcon->SetBitmap(iconBitmap);
                    }
                }
                if (linkedEditor)
                    linkedEditor->LoadPath(newPath);
                if (linkedTab)
                {
                    linkedTab->SetName(newPath);
                    linkedTab->SetLabel(newPath + "_tab");
                    linkedTab->SetToolTip(newPath);
                    auto tabText = ((wxStaticText *)linkedTab->GetChildren()[0]->GetChildren()[1]);
                    if (tabText)
                    {
                        tabText->SetName(newPath);
                        tabText->SetLabel(wxFileNameFromPath(newPath));
                    }
                    auto tabIcon = ((wxStaticBitmap *)linkedTab->GetChildren()[0]->GetChildren()[0]);
                    if (tabIcon)
                    {
                        wxString tabIconPath = LanguagesPreferences::Get().GetLanguageIconPath(newPath);
                        if (!tabIconPath.IsEmpty())
                        {
                            wxBitmap tabIconBitmap(tabIconPath, wxBITMAP_TYPE_PNG);
                            if (tabIconBitmap.IsOk())
                                tabIcon->SetBitmap(tabIconBitmap);
                        }
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
    auto target = FindWindowByLabel(componentIdentifier + "_file_container");
    if (!target)
    {
        target = FindWindowByLabel(componentIdentifier + "_dir_container");
        if (target)
            target = target->GetChildren()[0];
    }
    if (!target)
        return;
    if (m_currentSelectedFile && m_currentSelectedFile->GetName() == target->GetName())
        return;
    target->SetBackgroundColour(color);
    target->Refresh();
}

void FilesTree::OnComponentMouseEnter(wxMouseEvent &event)
{
    auto target = ((wxWindow *)event.GetEventObject());
    if (!target)
        return;
    auto color = ThemesManager::Get().GetColor("selectedFile");
    wxString path = target->GetName();
    if (path == "dir_name")
        target = target->GetGrandParent();
    ChangeFileBackground(target->GetName(), color);
}

void FilesTree::OnComponentMouseExit(wxMouseEvent &event)
{
    auto target = ((wxWindow *)event.GetEventObject());
    if (!target)
        return;
    auto color = ThemesManager::Get().GetColor("main");
    wxString path = target->GetName();
    if (path == "dir_name")
        target = target->GetGrandParent();
    ChangeFileBackground(target->GetName(), color);
}

void FilesTree::SetFileHighlight(const wxString &componentIdentifier)
{
    auto target = wxFindWindowByLabel(componentIdentifier + "_file_container");
    auto defaultColor = ThemesManager::Get().GetColor("main");
    auto selectedColor = ThemesManager::Get().GetColor("selectedFile");

    if (m_currentSelectedFile)
    {
        wxWindow *oldWin = m_currentSelectedFile.get();
        if (oldWin)
        {
            oldWin->SetBackgroundColour(defaultColor);
            oldWin->Refresh();
        }
        m_currentSelectedFile = nullptr;
    }

    if (!target || componentIdentifier.IsEmpty())
        return;

    target->SetBackgroundColour(selectedColor);
    target->Refresh();

    m_currentSelectedFile = target;
}

void FilesTree::OnProjectInformationsHoverEnter(wxMouseEvent &event)
{
    if (!m_projectInfosTools)
        return;

    if (m_projectInfosTools->IsShown())
    {
        m_projectInfosTools->Hide();
        return;
    }
    else
    {
        m_projectInfosTools->Show();
        return;
    }
}
