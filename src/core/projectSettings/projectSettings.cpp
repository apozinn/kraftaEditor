#include "projectSettings/projectSettings.hpp"

ProjectSettings& ProjectSettings::Get() {
    static ProjectSettings instance;
    return instance;
}

wxString ProjectSettings::GetProjectPath() const {
    return m_projectPath;
}

void ProjectSettings::SetProjectPath(const wxString& path) {
    m_projectPath = path;
}

wxString ProjectSettings::GetProjectName() const {
    return m_projectName;
}

void ProjectSettings::SetProjectName(const wxString& name) {
    m_projectName = name;
}

wxString ProjectSettings::GetCurrentlyFileOpen() const {
    return m_currentlyFileOpen;
}

void ProjectSettings::SetCurrentlyFileOpen(const wxString& filePath) {
    m_currentlyFileOpen = filePath;
}

wxString ProjectSettings::GetFullProjectIdentifier() const {
    if (m_projectPath.empty() || m_projectName.empty()) {
        return wxEmptyString;
    }
    return wxFileName(m_projectPath, m_projectName).GetFullPath();
}

bool ProjectSettings::IsProjectSet() const {
    return !m_projectPath.empty() && !m_projectName.empty();
}

void ProjectSettings::ClearProject() {
    m_projectPath.clear();
    m_projectName.clear();
    m_currentlyFileOpen.clear();
}