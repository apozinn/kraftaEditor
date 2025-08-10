#pragma once

#include <string>
#include <wx/arrstr.h>

namespace ApplicationPaths
{
    const wxString ASSETS_DIR_NAME = "assets";
    const wxString FILE_EXT_DIR_NAME = "file_ext";

    const wxArrayString DEV_MARKER_FILES = {
        "KraftaEditor.manifest",
        "CMakeLists.txt"};

    /**
     * @brief Gets the directory of the current executable
     *
     * Returns the absolute file system path to the directory where the application executable resides.
     * The path uses the path separators native to the current platform.
     *
     * @return wxString directory where the application executable resides
     *
     * @note On Unix systems, this usually resolves symbolic links
     *
     * @example
     * std::cout << "Running from: " << ApplicationPaths::ApplicationPath().ToStdString() << "\n";
     */
    const wxString &ApplicationPath();

    /**
     * @brief Determines if the application is running in a development environment
     *
     * Checks common development directory patterns (like 'build', 'debug', etc.)
     * and compares against standard installation locations.
     *
     * @return bool True if running in development environment
     * @note Thread-safe after first call
     */
    bool IsRunningInDevelopmentEnvironment();

    wxString FindDevelopmentRoot(const wxString &startPath, const wxArrayString &markerFiles, int maxLevels);

    /**
     * @brief Gets the root development environment path
     *
     * Locates the project root directory by searching for marker files
     * (like 'KraftaEditor.manifest' or 'CMakeLists.txt') when in development mode.
     * Falls back to executable directory in production.
     *
     * @return wxString The root path with trailing separator
     * @warning May return empty string if markers not found in development mode
     */
    const wxString DevelopmentEnvironmentPath();

    /**
     * @brief Gets the path to application assets
     *
     * @param target Optional subdirectory within assets (e.g. "icons")
     * @return wxString Full path to assets with trailing separator
     * @pre Directory 'assets/' must exist under development root (if in dev mode)
     *
     * @example
     * auto iconPath = AssetsPath("icons") + "app.png";
     */
    wxString AssetsPath(const wxString &target);

    /**
     * @brief Resolves the path to a language-specific icon
     *
     * @param languageName The programming language name (e.g. "cpp")
     * @return wxString Full path to the icon file, or wxEmptyString if not found
     * @note Falls back to "unknown_ext.png" if language icon not found
     */
    wxString GetLanguageIcon(const wxString &languageName);

    wxString GetLanguagePreferencesPath(const wxString& languageName);
}