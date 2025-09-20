#pragma once
#include <wx/string.h>
#include <wx/filename.h>

/**
 * @class ProjectSettings
 * @brief Singleton class that manages the state and metadata of the current project.
 *
 * The ProjectSettings class centralizes all information related to the
 * currently loaded project. This includes:
 *   - The absolute project path
 *   - The project name
 *   - The file currently open in the editor
 *   - The directory and file last accessed through a context menu
 *
 * By consolidating project metadata in one place, this class provides
 * a consistent access point across the application, ensuring data integrity
 * and simplifying project-level state management.
 *
 * This class follows the Singleton design pattern: only one global instance
 * exists during the application’s lifecycle.
 *
 * ### Example:
 * @code
 * // Access the singleton instance
 * auto& settings = ProjectSettings::Get();
 *
 * // Configure the project
 * settings.SetProjectPath("/home/user/MyProject");
 * settings.SetProjectName("MyProject");
 *
 * // Query information
 * wxString id = settings.GetFullProjectIdentifier();
 * if (settings.IsProjectSet()) {
 *     wxLogMessage("Loaded project: %s", id);
 * }
 *
 * // Clear project data
 * settings.ClearProject();
 * @endcode
 */
class ProjectSettings
{
public:
    /**
     * @brief Retrieves the global singleton instance.
     * @return Reference to the ProjectSettings instance.
     *
     * ### Example:
     * @code
     * ProjectSettings& ps = ProjectSettings::Get();
     * @endcode
     */
    static ProjectSettings& Get();

    // ==================== Project Path ====================

    /**
     * @brief Returns the absolute path of the currently loaded project.
     * @param withoutPathSeparator If true, the trailing path separator is removed.
     * @return Project path as a wxString.
     *
     * ### Example:
     * @code
     * wxString path = ProjectSettings::Get().GetProjectPath();
     * @endcode
     */
    wxString GetProjectPath(bool withoutPathSeparator=false) const;

    /**
     * @brief Sets the absolute path of the current project.
     * @param path Absolute directory path of the project.
     *
     * ### Example:
     * @code
     * ProjectSettings::Get().SetProjectPath("/workspace/project");
     * @endcode
     */
    void SetProjectPath(const wxString& path);

    // ==================== Project Name ====================

    /**
     * @brief Returns the name of the currently loaded project.
     * @return Project name as a wxString.
     *
     * ### Example:
     * @code
     * wxString name = ProjectSettings::Get().GetProjectName();
     * @endcode
     */
    wxString GetProjectName() const;

    /**
     * @brief Sets the name of the current project.
     * @param name New project name.
     *
     * ### Example:
     * @code
     * ProjectSettings::Get().SetProjectName("EngineDemo");
     * @endcode
     */
    void SetProjectName(const wxString& name);

    // ==================== Currently Open File ====================

    /**
     * @brief Returns the absolute path of the file currently open in the editor.
     * @return File path as a wxString.
     *
     * ### Example:
     * @code
     * wxString openFile = ProjectSettings::Get().GetCurrentlyFileOpen();
     * @endcode
     */
    wxString GetCurrentlyFileOpen() const;

    /**
     * @brief Sets the absolute path of the file currently open in the editor.
     * @param filePath Absolute path of the open file.
     *
     * ### Example:
     * @code
     * ProjectSettings::Get().SetCurrentlyFileOpen("/workspace/project/main.cpp");
     * @endcode
     */
    void SetCurrentlyFileOpen(const wxString& filePath);

    // ==================== Context Menu Directory ====================

    /**
     * @brief Returns the directory path currently targeted by a context menu.
     * @param withoutPathSeparator If true, the trailing path separator is removed.
     * @return Directory path as a wxString.
     *
     * ### Example:
     * @code
     * wxString menuDir = ProjectSettings::Get().GetCurrentlyMenuDir();
     * @endcode
     */
    wxString GetCurrentlyMenuDir(bool withoutPathSeparator=false) const;

    /**
     * @brief Sets the directory path currently targeted by a context menu.
     * @param dirPath Absolute path of the directory.
     *
     * ### Example:
     * @code
     * ProjectSettings::Get().SetCurrentlyMenuDir("/workspace/project/include");
     * @endcode
     */
    void SetCurrentlyMenuDir(const wxString& dirPath);

    // ==================== Context Menu File ====================

    /**
     * @brief Returns the file path currently targeted by a context menu.
     * @return File path as a wxString.
     *
     * ### Example:
     * @code
     * wxString menuFile = ProjectSettings::Get().GetCurrentlyMenuFile();
     * @endcode
     */
    wxString GetCurrentlyMenuFile() const;

    /**
     * @brief Sets the file path currently targeted by a context menu.
     * @param filePath Absolute path of the file.
     *
     * ### Example:
     * @code
     * ProjectSettings::Get().SetCurrentlyMenuFile("/workspace/project/config.json");
     * @endcode
     */
    void SetCurrentlyMenuFile(const wxString& filePath);

    // ==================== Utility Methods ====================

    /**
     * @brief Returns a unique identifier for the project.
     *        Typically composed of its absolute path and name.
     * @param withoutPathSeparator If true, the trailing path separator is removed.
     * @return Project identifier as a wxString.
     *
     * ### Example:
     * @code
     * wxString id = ProjectSettings::Get().GetFullProjectIdentifier();
     * @endcode
     */
    wxString GetFullProjectIdentifier(bool withoutPathSeparator=false) const;

    /**
     * @brief Checks whether a project is currently set.
     * @return True if both project path and name are defined, false otherwise.
     *
     * ### Example:
     * @code
     * if (ProjectSettings::Get().IsProjectSet()) {
     *     wxLogMessage("Project is active");
     * }
     * @endcode
     */
    bool IsProjectSet() const;

    /**
     * @brief Clears all stored project information,
     *        effectively resetting the project state.
     *
     * ### Example:
     * @code
     * ProjectSettings::Get().ClearProject();
     * @endcode
     */
    void ClearProject();

    // ==================== Rule of Five (restricted) ====================

    ProjectSettings(const ProjectSettings&) = delete; ///< Copy constructor disabled.
    void operator=(const ProjectSettings&) = delete;  ///< Assignment operator disabled.

private:
    /**
     * @brief Private constructor to enforce the Singleton pattern.
     */
    ProjectSettings() = default;

    wxString m_projectPath;       ///< Absolute path of the project directory.
    wxString m_projectName;       ///< Name of the project.
    wxString m_currentlyFileOpen; ///< Path of the currently open file in the editor.
    wxString m_menuDirPath;       ///< Path of the directory selected via context menu.
    wxString m_menuFilePath;      ///< Path of the file selected via context menu.
};
