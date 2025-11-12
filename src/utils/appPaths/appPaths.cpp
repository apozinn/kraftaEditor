#include "appPaths/appPaths.hpp"
#include "platformInfos/platformInfos.hpp"

#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/dir.h>
#include <wx/log.h>
#include <wx/app.h>
#include <wx/arrstr.h>
#include <wx/msgdlg.h>

#include <string>
#include <stdexcept>

namespace ApplicationPaths
{
    const wxString &ApplicationPath()
    {
        static const wxString path = [] {
            wxFileName file(wxStandardPaths::Get().GetExecutablePath());
            if (!file.IsOk())
            {
                wxLogError("ApplicationPath: failed to resolve executable path");
                return wxString();
            }
            return file.GetPathWithSep();
        }();
        return path;
    }

    bool IsRunningInDevelopmentEnvironment()
    {
        static const bool isDev = [] {
            const wxString path = ApplicationPath().Lower();
            const wxArrayString patterns = {"debug", "release", "build", "cmake-build", "x64", "x86"};

            for (const auto &p : patterns)
                if (path.Contains(p))
                    return true;

#ifdef __WXMSW__
            return !(path.Contains("program files") || path.Contains("program files (x86)"));
#elif defined(__WXGTK__)
            return !(path.StartsWith("/usr/") || path.StartsWith("/opt/") || path.Contains("/snap/") || path.Contains("/local/"));
#elif defined(__WXOSX__)
            return !path.Contains(".app/contents/macos");
#else
            return true;
#endif
        }();
        return isDev;
    }

    wxString FindDevelopmentRoot(const wxString &startPath, const wxArrayString &markerFiles, int maxLevels)
    {
        wxFileName dir(startPath);
        for (int i = 0; i < maxLevels && dir.GetDirCount() > 0; ++i)
        {
            wxDir d(dir.GetPath());
            if (d.IsOpened())
            {
                wxString name;
                bool cont = d.GetFirst(&name);
                while (cont)
                {
                    if (markerFiles.Index(name) != wxNOT_FOUND)
                        return dir.GetPathWithSep();
                    cont = d.GetNext(&name);
                }
            }
            dir.RemoveLastDir();
        }
        return wxEmptyString;
    }

    const wxString DevelopmentEnvironmentPath()
    {
        static const wxString path = [] {
            if (!IsRunningInDevelopmentEnvironment())
                return ApplicationPath();
            wxString devRoot = FindDevelopmentRoot(ApplicationPath(), DEV_MARKER_FILES, 6);
            if (devRoot.IsEmpty())
                wxLogWarning("DevelopmentEnvironmentPath: no marker files (%s) found.", wxJoin(DEV_MARKER_FILES, ','));
            return devRoot.IsEmpty() ? ApplicationPath() : devRoot;
        }();
        return path;
    }

    wxString InstalledBasePath(const wxString &subdir)
    {
#ifdef __WXGTK__
        return "/usr/share/kraftaEditor/" + subdir + "/";
#elif defined(__WXMSW__)
        wxString progFiles = wxGetenv("PROGRAMFILES");
        if (progFiles.IsEmpty())
            progFiles = "C:\\Program Files";
        return progFiles + "\\Krafta Editor\\" + subdir + "\\";
#elif defined(__WXOSX__)
        return "/Applications/Krafta Editor.app/Contents/Resources/" + subdir + "/";
#else
        return ApplicationPath();
#endif
    }

    wxString AssetsPath(const wxString &target)
    {
        static bool warned = false;
        wxString base = (IsRunningInDevelopmentEnvironment() ? DevelopmentEnvironmentPath() + "assets" + PlatformInfos::OsPathSeparator() : InstalledBasePath("assets"));
        if (!target.IsEmpty())
            base += target + wxFileName::GetPathSeparator();
        if (!wxDirExists(base) && !warned)
        {
            wxLogWarning("AssetsPath: directory not found: %s", base);
            warned = true;
        }
        return wxDirExists(base) ? base : "";
    }

    wxString GetIconPath(const wxString &name)
    {
        wxString path = AssetsPath("icons") + name;
        return wxFileExists(path) ? path : "";
    }

    wxString GetLanguageIcon(const wxString &lang)
    {
        wxString dir = AssetsPath(FILE_EXT_DIR_NAME);
        if (dir.IsEmpty())
            return wxEmptyString;

        wxFileName icon(dir, lang + ".png");
        if (!icon.Exists())
        {
            icon.SetName("unknown_ext");
            if (!icon.Exists())
                return wxEmptyString;
        }
        return icon.GetFullPath();
    }

    wxString GetLanguagePreferencesPath(const wxString &lang)
    {
        wxString base = (IsRunningInDevelopmentEnvironment() ? DevelopmentEnvironmentPath() + "languages" + PlatformInfos::OsPathSeparator() : InstalledBasePath("languages"));
        if (lang.IsEmpty())
            return base;

        wxString path = base + lang + PlatformInfos::OsPathSeparator();
        if (wxDirExists(path))
            return path;

        wxString fallback = base + "default";
        return wxDirExists(fallback) ? fallback : "";
    }

    wxString GetConfigPath(const wxString &name)
    {
        wxString base = (IsRunningInDevelopmentEnvironment() ? DevelopmentEnvironmentPath() + "config" + PlatformInfos::OsPathSeparator() : InstalledBasePath("config"));
        if (name.IsEmpty())
            return base;
        wxString path = base + name + PlatformInfos::OsPathSeparator(); 
        return wxDirExists(path) ? path : "";
    }

    wxString GetI18nLanguagePath()
    {
        wxString base = (IsRunningInDevelopmentEnvironment() ? DevelopmentEnvironmentPath() + "i18n" + PlatformInfos::OsPathSeparator() : InstalledBasePath("i18n"));
        if (wxDirExists(base))
            return base;
        wxLogError("GetI18nLanguagePath: directory not found: %s", base);
        return wxEmptyString;
    }
}
