#pragma once

#include <wx/platinfo.h>
#include <wx/filename.h>
#include <string>

/**
 * @namespace PlatformInfos
 * @brief Provides platform-specific information and utilities.
 *
 * The PlatformInfos namespace contains helper functions for:
 *   - Querying the current operating system family
 *   - Performing platform checks (e.g., Windows-specific logic)
 *   - Retrieving platform-specific path separators
 *
 * All functions are:
 *   - **Thread-safe**
 *   - **Noexcept guaranteed**
 *   - Optimized with caching for repeated calls
 *
 * ### Example:
 * @code
 * #include "PlatformInfos.h"
 *
 * if (PlatformInfos::IsWindows()) {
 *     wxLogMessage("Running on Windows, path separator: %s", 
 *                  PlatformInfos::OsPathSeparator());
 * } else {
 *     wxLogMessage("Running on %s", PlatformInfos::OsNameView());
 * }
 * @endcode
 */
namespace PlatformInfos
{

    /**
     * @brief Gets the name of the current operating system family.
     * @return std::string_view containing the OS family name
     *         (e.g., `"Windows"`, `"Macintosh"`, `"Unix"`).
     *
     * The value is cached after the first call for better performance.
     *
     * ### Example:
     * @code
     * std::string_view os = PlatformInfos::OsNameView();
     * if (os == "Windows") {
     *     wxLogMessage("Special behavior for Windows");
     * }
     * @endcode
     */
    inline std::string_view OsNameView() noexcept
    {
        static const auto name = wxPlatformInfo::Get().GetOperatingSystemFamilyName();
        static const std::string cached = name.ToStdString();
        return cached;
    }

    /**
     * @brief Checks if the current platform is Windows.
     * @return True if running on Windows, false otherwise.
     *
     * ### Example:
     * @code
     * if (PlatformInfos::IsWindows()) {
     *     wxLogMessage("Windows-specific path handling enabled.");
     * }
     * @endcode
     */
    inline bool IsWindows() noexcept
    {
        return wxPlatformInfo::Get().GetOperatingSystemId() == wxOS_WINDOWS_NT;
    }

    /**
     * @brief Gets the path separator for the current platform.
     * @return wxString containing the platform-specific path separator.
     *
     * The result is cached after the first call.
     * - On Windows returns `"\\\\"`
     * - On Unix-like systems returns `"/"`
     *
     * ### Example:
     * @code
     * wxString sep = PlatformInfos::OsPathSeparator();
     * wxString filePath = "folder" + sep + "file.txt";
     * @endcode
     */
    inline wxString OsPathSeparator() noexcept
    {
        static const wxFileName file;
        static const std::string separator(1, file.GetPathSeparator());
        return separator;
    }

}
