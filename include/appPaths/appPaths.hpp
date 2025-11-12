#pragma once

#include <string>
#include <wx/arrstr.h>
#include <wx/string.h>

/**
 * @file applicationPaths.hpp
 * @brief Provides utilities for managing and resolving application-specific paths.
 *
 * This module centralizes the logic for locating important directories such as
 * assets, language definitions, and configuration files, both in development
 * and production environments.
 *
 * It automatically adjusts paths depending on the platform and build context.
 *
 * @namespace ApplicationPaths
 * @version 1.0
 * @date 2025
 * @author
 *   Krafta Editor Developers
 */

namespace ApplicationPaths
{
    // ─────────────────────────────────────────────
    // ─── Constants ───────────────────────────────
    // ─────────────────────────────────────────────

    /**
     * @brief Directory name for static assets (images, icons, etc.).
     */
    inline constexpr auto ASSETS_DIR_NAME = "assets";

    /**
     * @brief Directory name for file extension metadata.
     */
    inline constexpr auto FILE_EXT_DIR_NAME = "file_ext";

    /**
     * @brief Directory name for programming language definitions.
     */
    inline constexpr auto LANGUAGES_DIR_NAME = "languages";

    /**
     * @brief Filenames used to identify a development environment.
     *
     * The presence of any of these files (e.g., CMakeLists.txt) in a parent directory
     * is considered an indicator that the application is running in development mode.
     */
    const wxArrayString DEV_MARKER_FILES = {
        "KraftaEditor.manifest",
        "CMakeLists.txt"};

    // ─────────────────────────────────────────────
    // ─── Core Path Functions ─────────────────────
    // ─────────────────────────────────────────────

    /**
     * @brief Retrieves the absolute path to the directory containing the executable.
     *
     * On Unix-like systems, symbolic links are automatically resolved to their
     * final target to ensure the true binary location is returned.
     *
     * @return A reference to a cached `wxString` containing the absolute executable directory path.
     *
     * @note This path is typically cached after the first call for performance reasons.
     *
     * @example
     * ```cpp
     * wxPuts("Application is running from: " + ApplicationPaths::ApplicationPath());
     * ```
     */
    const wxString &ApplicationPath();

    /**
     * @brief Checks whether the application is running in a development environment.
     *
     * The detection logic searches for known marker files (see @ref DEV_MARKER_FILES)
     * or common directory structures (e.g., `build`, `debug`, `bin/DEBUG`) near the
     * executable location.
     *
     * @return `true` if development environment markers are found; otherwise, `false`.
     *
     * @note This is typically used to decide between resource paths (e.g., local vs. system).
     */
    bool IsRunningInDevelopmentEnvironment();

    /**
     * @brief Recursively searches upward from a given starting path to locate the
     *        development root directory.
     *
     * The function checks each parent directory for the presence of any of the
     * specified marker files, returning the first directory where they are found.
     *
     * @param startPath The directory to start searching from.
     * @param markerFiles A list of filenames used to detect the development root.
     * @param maxLevels The maximum number of parent directories to traverse.
     *
     * @return A `wxString` containing the detected development root path,
     *         or an empty string if no marker is found within the given depth.
     *
     * @example
     * ```cpp
     * wxString root = ApplicationPaths::FindDevelopmentRoot(wxGetCwd(), ApplicationPaths::DEV_MARKER_FILES, 5);
     * if (!root.empty())
     *     wxPuts("Development root found at: " + root);
     * ```
     */
    wxString FindDevelopmentRoot(const wxString &startPath, const wxArrayString &markerFiles, int maxLevels);

    /**
     * @brief Returns the root path of the current environment.
     *
     * In development mode, this is the directory containing known marker files.
     * In production mode, it falls back to the directory of the executable.
     *
     * @return A `wxString` containing the root directory path with a trailing separator.
     *
     * @warning May return an empty string if the development markers cannot be found.
     *
     * @example
     * ```cpp
     * wxString root = ApplicationPaths::DevelopmentEnvironmentPath();
     * wxPuts("Environment root: " + root);
     * ```
     */
    const wxString DevelopmentEnvironmentPath();

    // ─────────────────────────────────────────────
    // ─── Assets and Resource Paths ───────────────
    // ─────────────────────────────────────────────

    /**
     * @brief Builds and returns the absolute path to the `assets` directory or
     *        a specific subdirectory within it.
     *
     * The returned path differs based on the build environment:
     * - **Development**: Relative to the project source root.
     * - **Production**:
     *   - **Linux**: `/usr/share/kraftaEditor/assets`
     *   - **Windows**: `C:\\Program Files\\Krafta Editor\\assets`
     *   - **macOS**: Inside the app bundle (`Resources/assets`)
     *
     * @param target Optional subdirectory within the `assets` folder (default: empty).
     * @return A `wxString` containing the full path with a trailing separator.
     *
     * @pre The `assets` directory must exist in either the development or installation directory.
     *
     * @example
     * ```cpp
     * wxString iconDir = ApplicationPaths::AssetsPath("icons");
     * wxImage logo(iconDir + "logo.png");
     * ```
     */
    wxString AssetsPath(const wxString &target = "");

    /**
     * @brief Resolves the full path to a specific icon file.
     *
     * @param iconName The name of the icon file (e.g., `"save.png"`).
     * @return A `wxString` containing the absolute path to the icon file,
     *         or an empty string if the icon does not exist.
     *
     * @example
     * ```cpp
     * wxBitmap saveIcon(ApplicationPaths::GetIconPath("save.png"), wxBITMAP_TYPE_PNG);
     * ```
     */
    wxString GetIconPath(const wxString &iconName);

    /**
     * @brief Returns the full path to an icon representing a specific programming language.
     *
     * The function attempts to locate the language-specific icon in the `assets/icons`
     * directory. If the icon is not found, it gracefully falls back to `unknown_ext.png`.
     *
     * @param languageName The name or ID of the language (e.g., `"cpp"`, `"python"`).
     * @return A `wxString` containing the icon path, or an empty string if unavailable.
     *
     * @example
     * ```cpp
     * wxString cppIcon = ApplicationPaths::GetLanguageIcon("cpp");
     * ```
     */
    wxString GetLanguageIcon(const wxString &languageName);

    /**
     * @brief Retrieves the path to a JSON file containing preferences for a given language.
     *
     * The preferences files are typically located under:
     * - Development: `<project_root>/languages/<lang>.json`
     * - Production:  `<install_root>/languages/<lang>.json`
     *
     * @param languageName The language identifier (e.g., `"cpp"`, `"python"`).
     * @return A `wxString` containing the full path to the preferences file,
     *         or an empty string if the file does not exist.
     *
     * @example
     * ```cpp
     * wxString prefsPath = ApplicationPaths::GetLanguagePreferencesPath("cpp");
     * ```
     */
    wxString GetLanguagePreferencesPath(const wxString &languageName);

    /**
     * @brief Retrieves the absolute path to a configuration file located in the
     *        application's configuration directory.
     *
     * The method automatically appends the proper extension and resolves
     * platform-specific config directories.
     *
     * @param configName Name of the configuration file (without extension).
     * @return A `wxString` containing the absolute path to the configuration file,
     *         or an empty string if the file cannot be resolved.
     *
     * @example
     * ```cpp
     * wxString uiConfig = ApplicationPaths::GetConfigPath("ui_settings");
     * ```
     */
    wxString GetConfigPath(const wxString &configName);

    /**
     * @brief Returns the absolute path to the directory containing the application's
     *        internationalization (i18n) files.
     *
     * This directory stores localization resources such as `.mo` and `.po` files
     * organized by language and region codes (e.g., `en_US`, `pt_BR`).
     *
     * The returned path automatically adapts to the environment:
     * - **Development mode**: typically `<project_root>/i18n`
     * - **Production mode**: typically `/usr/share/kraftaEditor/i18n` or inside the app bundle
     *
     * @return A `wxString` containing the absolute path to the i18n directory
     *         with a trailing path separator, or an empty string if not found.
     *
     * @note This path is used by translation systems such as `wxTranslations`
     *       to locate language catalogs (e.g., `LC_MESSAGES/kraftaeditor.mo`).
     *
     * @example
     * ```cpp
     * wxString langPath = ApplicationPaths::GetI18nLanguagePath();
     * wxTranslations::Get()->AddCatalogLookupPathPrefix(langPath);
     * ```
     */
    wxString GetI18nLanguagePath();

}
