#include "appPaths/appPaths.hpp"

#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/dir.h>
#include <string>
#include <stdexcept>
#include <wx/arrstr.h>
#include "platformInfos/platformInfos.hpp"
#include <wx/log.h>

namespace ApplicationPaths
{
    const wxString &ApplicationPath()
    {
        static const wxString path = []()
        {
            wxFileName file(wxStandardPaths::Get().GetExecutablePath());
            if (!file.IsOk())
                throw std::runtime_error("Invalid executable path");
            return file.GetPathWithSep(); // Retorna wxString
        }();
        return path;
    }

    bool IsRunningInDevelopmentEnvironment()
    {
        static const bool isDev = []()
        {
            wxFileName exePath(ApplicationPath());
            const wxString fullPath = exePath.GetFullPath().Lower();

            // Common development directory patterns
            const wxArrayString devPatterns = {
                "debug", "release", "cmake-build",
                "build", "bin", "x64", "x86"};

            for (const auto &pattern : devPatterns)
            {
                if (fullPath.Contains(pattern))
                {
                    return true;
                }
            }

// Check standard installation locations
#ifdef __WXMSW__
            return !fullPath.Contains("program files");
#elif defined(__WXGTK__)
            return !(fullPath.StartsWith("/usr/bin/") ||
                     fullPath.StartsWith("/usr/local/bin/"));
#elif defined(__WXOSX__)
            return !fullPath.Contains(".app/contents/macos");
#endif

            return false;
        }();
        return isDev;
    }

    wxString FindDevelopmentRoot(const wxString &startPath, const wxArrayString &markerFiles, int maxLevels = 5)
    {
        wxFileName currentDir(startPath);
        int levelsUp = 0;

        while (levelsUp++ < maxLevels && currentDir.GetDirCount() > 0)
        {
            wxDir dir(currentDir.GetFullPath());
            if (!dir.IsOpened())
            {
                wxLogError("Failed to open directory: %s", currentDir.GetFullPath().c_str());
                return wxEmptyString;
                break;
            }

            wxString filename;
            bool cont = dir.GetFirst(&filename);
            while (cont)
            {
                if (markerFiles.Index(filename) != wxNOT_FOUND)
                {
                    return currentDir.GetPathWithSep();
                }
                cont = dir.GetNext(&filename);
            }

            currentDir.RemoveLastDir();
        }
        return wxEmptyString;
    }

    const wxString DevelopmentEnvironmentPath()
    {
        static const wxString path = []()
        {
            if (!IsRunningInDevelopmentEnvironment())
            {
                return ApplicationPath();
            }

            wxString devRoot = FindDevelopmentRoot(ApplicationPath(), DEV_MARKER_FILES, 5);

            if (devRoot.IsEmpty())
            {
                wxLogWarning("None of the development marker files (%s) found within 5 levels up from executable path.", wxJoin(DEV_MARKER_FILES, ','));
            }

            return devRoot;
        }();
        return path;
    }

    wxString AssetsPath(const wxString &target = wxEmptyString)
    {
        wxString devPath = DevelopmentEnvironmentPath();

        wxString path = devPath;
        path << ASSETS_DIR_NAME << wxFileName::GetPathSeparator();

        if (!target.empty())
        {
            path << target << wxFileName::GetPathSeparator();
        }

        if (!wxFileName::DirExists(path))
        {
            wxLogWarning("Assets directory not found: %s", path);
            return wxEmptyString;
        }
        return path;
    }

    wxString GetLanguageIcon(const wxString &languageName)
    {
        const wxString extDir = AssetsPath(FILE_EXT_DIR_NAME);
        if (extDir.IsEmpty())
        {
            wxLogWarning("Missing assets directory: %s", extDir);
            return wxEmptyString;
        }

        wxFileName iconFile(extDir, languageName + ".png");

        if (!iconFile.Exists())
        {
            iconFile.SetName("unknown_ext");
            if (!iconFile.Exists())
            {
                wxLogWarning("Missing fallback icon: %s", iconFile.GetFullPath());
                return wxEmptyString;
            }
        }
        return iconFile.GetFullPath();
    }
}