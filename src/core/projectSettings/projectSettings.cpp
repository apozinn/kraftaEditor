#include "projectSettings/projectSettings.hpp"
#include "gui/panels/filesTree/filesTree.hpp"
#include "fileOperations/fileOperations.hpp"
#include "platformInfos/platformInfos.hpp"
#include "ui/ids.hpp"
#include <wx/window.h>

ProjectSettings& ProjectSettings::Get()
{
    static ProjectSettings instance;
    return instance;
}

wxString ProjectSettings::GetProjectPath(bool withoutPathSeparator) const
{
    if (!m_projectPath.empty() && withoutPathSeparator)
    {
        wxString path = m_projectPath;
        if (path.Last() == PlatformInfos::OsPathSeparator())
        {
            path.RemoveLast();
        }
        return path;
    }
    return m_projectPath;
}

wxString ProjectSettings::GetProjectName() const
{
    return m_projectName;
}

wxString ProjectSettings::GetCurrentlyFileOpen() const
{
    return m_currentlyFileOpen;
}

wxString ProjectSettings::GetCurrentlyMenuDir(bool withoutPathSeparator) const
{
    if (!m_menuDirPath.empty() && withoutPathSeparator)
    {
        wxString path = m_menuDirPath;
        if (path.Last() == PlatformInfos::OsPathSeparator())
        {
            path.RemoveLast();
        }
        return path;
    }
    return m_menuDirPath;
}

wxString ProjectSettings::GetCurrentlyMenuFile() const
{
    return m_menuFilePath;
}

wxString ProjectSettings::GetFullProjectIdentifier(bool withoutPathSeparator) const
{
    if (m_projectPath.empty() || m_projectName.empty())
    {
        return wxEmptyString;
    }

    wxString basePath = m_projectPath;
    if (!basePath.empty() && withoutPathSeparator && basePath.Last() == PlatformInfos::OsPathSeparator())
    {
        basePath.RemoveLast();
    }

    return wxFileName(basePath, m_projectName).GetFullPath();
}

void ProjectSettings::SetProjectPath(const wxString& path)
{
    m_projectPath = path;
}

void ProjectSettings::SetProjectName(const wxString& name)
{
    m_projectName = name;
}

void ProjectSettings::SetCurrentlyFileOpen(const wxString& filePath)
{
    m_currentlyFileOpen = filePath;

    if (auto* fileContainer = dynamic_cast<FilesTree*>(wxWindow::FindWindowById(+GUI::ControlID::FilesTree)))
    {
        fileContainer->SetFileHighlight(filePath);
    }
}

void ProjectSettings::SetCurrentlyMenuDir(const wxString& dirPath)
{
    if (wxDirExists(dirPath))
    {
        m_menuDirPath = dirPath;
    }
}

void ProjectSettings::SetCurrentlyMenuFile(const wxString& filePath)
{
    m_menuFilePath = filePath;
}

bool ProjectSettings::IsProjectSet() const
{
    return !m_projectPath.empty() && !m_projectName.empty();
}

void ProjectSettings::ClearProject()
{
    m_projectPath.clear();
    m_projectName.clear();
    m_currentlyFileOpen.clear();
    m_menuDirPath.clear();
    m_menuFilePath.clear();
}
