#pragma once
#include <wx/string.h>
#include <wx/filename.h>

/**
 * @class ProjectSettings
 * @brief Singleton class responsible for managing project-related state.
 *
 * The ProjectSettings class centralizes all information related to the
 * currently loaded project, including its path, name, currently open file,
 * and the last accessed menu directory/file. This ensures a consistent
 * and thread-safe access point for project metadata across the application.
 *
 * The class follows the Singleton pattern, meaning only one global instance
 * is ever created and shared throughout the application lifecycle.
 */
class ProjectSettings
{
public:
    /**
     * @brief Retrieves the global singleton instance.
     * @return Reference to the ProjectSettings instance.
     */
    static ProjectSettings& Get();

    // ==================== Project Path ====================

    /**
     * @brief Gets the absolute path to the currently loaded project.
     * @return Project path as a wxString.
     */
    wxString GetProjectPath() const;

    /**
     * @brief Sets the absolute path of the current project.
     * @param path The new project path.
     */
    void SetProjectPath(const wxString& path);

    // ==================== Project Name ====================

    /**
     * @brief Gets the name of the currently loaded project.
     * @return Project name as a wxString.
     */
    wxString GetProjectName() const;

    /**
     * @brief Sets the name of the current project.
     * @param name The new project name.
     */
    void SetProjectName(const wxString& name);

    // ==================== Currently Open File ====================

    /**
     * @brief Gets the path of the file currently open in the editor.
     * @return File path as a wxString.
     */
    wxString GetCurrentlyFileOpen() const;

    /**
     * @brief Sets the path of the file currently open in the editor.
     * @param filePath Absolute path to the file.
     */
    void SetCurrentlyFileOpen(const wxString& filePath);

    // ==================== Context Menu Directory ====================

    /**
     * @brief Gets the directory path currently targeted by a context menu.
     * @return Directory path as a wxString.
     */
    wxString GetCurrentlyMenuDir() const;

    /**
     * @brief Sets the directory path currently targeted by a context menu.
     * @param dirPath Absolute path to the directory.
     */
    void SetCurrentlyMenuDir(const wxString& dirPath);

    // ==================== Context Menu File ====================

    /**
     * @brief Gets the file path currently targeted by a context menu.
     * @return File path as a wxString.
     */
    wxString GetCurrentlyMenuFile() const;

    /**
     * @brief Sets the file path currently targeted by a context menu.
     * @param filePath Absolute path to the file.
     */
    void SetCurrentlyMenuFile(const wxString& filePath);

    // ==================== Utility Methods ====================

    /**
     * @brief Returns a unique identifier for the project,
     *        typically combining its path and name.
     * @return Full project identifier as a wxString.
     */
    wxString GetFullProjectIdentifier() const;

    /**
     * @brief Checks whether a project has been set.
     * @return True if project path and name are defined, false otherwise.
     */
    bool IsProjectSet() const;

    /**
     * @brief Clears all stored project information,
     *        effectively resetting the project state.
     */
    void ClearProject();

    // ==================== Rule of Five (disabled) ====================

    ProjectSettings(const ProjectSettings&) = delete; ///< Deleted copy constructor.
    void operator=(const ProjectSettings&) = delete;  ///< Deleted assignment operator.

private:
    /**
     * @brief Private constructor to enforce the Singleton pattern.
     */
    ProjectSettings() = default;

    wxString m_projectPath;       ///< Absolute path of the project directory.
    wxString m_projectName;       ///< Name of the project.
    wxString m_currentlyFileOpen; ///< Path to the currently open file.
    wxString m_menuDirPath;       ///< Path to the directory selected via context menu.
    wxString m_menuFilePath;      ///< Path to the file selected via context menu.
};
