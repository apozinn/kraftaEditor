/**
 * @file ApplicationPaths.h
 * @brief Provides utilities for accessing application paths
 */

#ifndef APPLICATION_PATHS_H
#define APPLICATION_PATHS_H

#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <string>
#include <string_view>
#include "platform/platform.hpp"

namespace ApplicationPaths
{

   /**
    * @brief Gets the full path to the current executable
    *
    * Returns the absolute filesystem path to the running application executable.
    * The path uses the native path separators for the current platform.
    *
    * @return std::string The executable path in UTF-8 encoding
    *
    * @note The returned path includes the executable filename
    * @note On Unix systems, this typically resolves symlinks
    *
    * @example
    * std::cout << "Running from: " << ApplicationPaths::ApplicationPath() << "\n";
    */
   inline std::string ApplicationPath() noexcept
   {
      // Cache the result since it won't change during runtime
      wxFileName file = (wxStandardPaths::Get().GetExecutablePath());
      static const std::string path = file.GetPath().Append(file.GetPathSeparator()).ToStdString();
      return path;
   }

   inline wxString IconsPath() noexcept
   {
      return ApplicationPath() + "icons" + Platform::OsPathSepareator();
   }

   inline wxString GetLanguageIcon(const std::string languageName) noexcept
   {
      wxFileName languageIconFile = wxFileName(IconsPath()+ "file_ext" + Platform::OsPathSepareator() + languageName + ".png");
      if (!languageIconFile.Exists())
      {
         wxFileName unknownIcon = wxFileName(IconsPath() + "file_ext" + Platform::OsPathSepareator() + "unknown_ext.png");
         if (unknownIcon.Exists())
         {
            return unknownIcon.GetFullPath();
         }
      }
      return languageIconFile.GetFullPath();
   }

   inline wxString AssetsPath() noexcept
   {
      return ApplicationPath() + "assets" + Platform::OsPathSepareator();
   }

} // namespace ApplicationPaths

#endif // APPLICATION_PATHS_H