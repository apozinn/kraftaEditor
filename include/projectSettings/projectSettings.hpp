#pragma once
#include <wx/string.h>
#include <wx/filename.h>

/**
 * @class ProjectSettings
 * @brief Singleton class that manages global project-related settings
 *
 * This class provides thread-safe access to project path and name
 * throughout the application using the Singleton pattern.
 */
class ProjectSettings
{
public:
    static ProjectSettings &Get();

    // ==================== Project Path Methods ====================
    wxString GetProjectPath() const;
    void SetProjectPath(const wxString &path);

    // ==================== Project Name Methods ====================
    wxString GetProjectName() const;
    void SetProjectName(const wxString &name);

    // ==================== Currently file open Methods ====================
    wxString GetCurrentlyFileOpen() const;
    void SetCurrentlyFileOpen(const wxString &filePath);

    // ==================== Currently menu dir Methods ====================
    wxString GetCurrentlyMenuDir() const;
    void SetCurrentlyMenuDir(const wxString &filePath);

    // ==================== Currently menu file Methods ====================
    wxString GetCurrentlyMenuFile() const;
    void SetCurrentlyMenuFile(const wxString &filePath);

    // ==================== Utility Methods ====================
    wxString GetFullProjectIdentifier() const;
    bool IsProjectSet() const;
    void ClearProject();

    // Delete copy constructor and assignment operator
    ProjectSettings(const ProjectSettings &) = delete;
    void operator=(const ProjectSettings &) = delete;

private:
    ProjectSettings() = default; // Private constructor for Singleton

    wxString m_projectPath;       ///< Stores the project directory path
    wxString m_projectName;       ///< Stores the project name
    wxString m_currentlyFileOpen; ///< Stores the currently file open
    wxString m_menuDirPath;       ///< Stores the currently menu dir
    wxString m_menuFilePath;      ///< Stores the currently menu file
};