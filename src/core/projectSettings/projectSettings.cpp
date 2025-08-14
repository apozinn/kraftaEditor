#include "projectSettings/projectSettings.hpp"

ProjectSettings& ProjectSettings::Get() {
    static ProjectSettings instance;
    return instance;
}

wxString ProjectSettings::GetProjectPath() const {
    return m_projectPath;
}

wxString ProjectSettings::GetProjectName() const {
    return m_projectName;
}

wxString ProjectSettings::GetCurrentlyFileOpen() const {
    return m_currentlyFileOpen;
}

wxString ProjectSettings::GetCurrentlyMenuDir() const {
    return m_menuDirPath;
}

wxString ProjectSettings::GetCurrentlyMenuFile() const {
    return m_menuFilePath;
}

wxString ProjectSettings::GetFullProjectIdentifier() const {
    if (m_projectPath.empty() || m_projectName.empty()) {
        return wxEmptyString;
    }
    return wxFileName(m_projectPath, m_projectName).GetFullPath();
}

void ProjectSettings::SetProjectPath(const wxString& path) {
    m_projectPath = path;
}

void ProjectSettings::SetProjectName(const wxString& name) {
    m_projectName = name;
}

void ProjectSettings::SetCurrentlyFileOpen(const wxString& filePath) {
    m_currentlyFileOpen = filePath;
}

void ProjectSettings::SetCurrentlyMenuDir(const wxString& dirPath) {
    if(wxDirExists(dirPath)) {
        m_menuDirPath = dirPath;
    }
}

void ProjectSettings::SetCurrentlyMenuFile(const wxString& filePath) {
    m_menuFilePath = filePath;
}

bool ProjectSettings::IsProjectSet() const {
    return !m_projectPath.empty() && !m_projectName.empty();
}

void ProjectSettings::ClearProject() {
    m_projectPath.clear();
    m_projectName.clear();
    m_currentlyFileOpen.clear();
    m_menuDirPath.clear();
    m_menuFilePath.clear();
}