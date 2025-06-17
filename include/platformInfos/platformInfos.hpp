#pragma once

#include <wx/platinfo.h>
#include <wx/filename.h>
#include <string>

/**
 * @namespace PlatformInfos
 * @brief Provides platform-specific information and utilities
 *
 * This namespace contains functions to retrieve information about the current
 * operating system and platform-specific path handling utilities.
 * All functions are thread-safe and noexcept guaranteed.
 */
namespace PlatformInfos
{

    /**
     * @brief Gets the name of the current operating system family
     * @return std::string containing the OS family name (e.g., "Windows", "Macintosh", "Unix")
     *
     * @note The result is cached after first call for better performance
     * @example
     * std::string os = PlatformInfos::OsName();
     * // On Windows: "Windows"
     * // On macOS: "Macintosh"
     * // On Linux: "Unix"
     */
    inline std::string_view OsNameView() noexcept
    {
        static const auto name = wxPlatformInfo::Get().GetOperatingSystemFamilyName();
        static const std::string cached = name.ToStdString();
        return cached;
    }

    /**
     * @brief Checks if the current platform is Windows
     * @return true if running on Windows, false otherwise
     */
    inline bool IsWindows() noexcept
    {
        return wxPlatformInfo::Get().GetOperatingSystemId() == wxOS_WINDOWS_NT;
    }

    /**
     * @brief Gets the path separator for the current platform
     * @return std::string containing the platform-specific path separator
     *
     * @note The result is cached after first call for better performance
     * @note On Windows returns "\\", on Unix-like systems returns "/"
     * @example
     * std::string sep = PlatformInfos::OsPathSepareator();
     * // On Windows: "\\"
     * // On Unix-like: "/"
     */
    inline std::string OsPathSepareator() noexcept
    {
        static const wxFileName file;
        static const std::string separator(1, file.GetPathSeparator());
        return separator;
    }

}