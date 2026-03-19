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

#ifndef _
#define _(s) wxGetTranslation(s)
#endif

// KRAFTA_ASSET_RELATIVE_PATH is injected by CMake via target_compile_definitions.
// Values per platform:
//   macOS   → "../Resources"          (inside .app bundle)
//   Windows → "assets"                (next to the .exe)
//   Linux   → "/usr/share/kraftaEditor" (absolute, honours CMAKE_INSTALL_PREFIX)
#ifndef KRAFTA_ASSET_RELATIVE_PATH
    #error "KRAFTA_ASSET_RELATIVE_PATH is not defined. Make sure CMake is configured correctly."
#endif

namespace ApplicationPaths
{
    const wxString &ApplicationPath()
    {
        static const wxString path = []
        {
            wxFileName file(wxStandardPaths::Get().GetExecutablePath());
            if (!file.IsOk())
            {
                wxMessageBox(_("ApplicationPath: failed to resolve executable path"), _("Error"), wxICON_ERROR);
                return wxString();
            }
            return file.GetPathWithSep();
        }();
        return path;
    }

    bool IsRunningInDevelopmentEnvironment()
    {
        static const bool isDev = []
        {
            const wxString path = ApplicationPath().Lower();
            const wxArrayString patterns = {"debug", "release", "build", "cmake-build", "x64", "x86"};

            for (const auto &p : patterns)
                if (path.Contains(p))
                    return true;

#ifdef __WXMSW__
            return !(path.Contains("program files") || path.Contains("program files (x86)"));
#elif defined(__WXGTK__)
            return !(path.StartsWith("/usr/") || path.StartsWith("/opt/") ||
                     path.Contains("/snap/")  || path.Contains("/local/"));
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
        static const wxString path = []
        {
            if (!IsRunningInDevelopmentEnvironment())
                return ApplicationPath();
            wxString devRoot = FindDevelopmentRoot(ApplicationPath(), DEV_MARKER_FILES, 6);
            if (devRoot.IsEmpty())
                wxMessageBox(
                    wxString::Format(_("DevelopmentEnvironmentPath: no marker files (%s) found."),
                                     wxJoin(DEV_MARKER_FILES, ',')),
                    _("Warning"), wxICON_WARNING);
            return devRoot.IsEmpty() ? ApplicationPath() : devRoot;
        }();
        return path;
    }

    // Returns the installed base path for a given subdirectory.
    // Uses KRAFTA_ASSET_RELATIVE_PATH (set by CMake) so the binary always
    // respects the actual install prefix — no hardcoded paths.
    wxString InstalledBasePath(const wxString &subdir)
    {
        const wxString assetRoot = wxString::FromUTF8(KRAFTA_ASSET_RELATIVE_PATH);

#ifdef __WXOSX__
        // macOS: path is relative to the binary inside the .app bundle
        wxFileName base(ApplicationPath() + assetRoot);
        base.Normalize();
        return base.GetPathWithSep() + subdir + "/";
#elif defined(__WXMSW__)
        // Windows: path is relative to the directory containing the .exe
        return ApplicationPath() + assetRoot + wxFileName::GetPathSeparator()
               + subdir + wxFileName::GetPathSeparator();
#else
        // Linux/BSD: CMake provides an absolute path (e.g. /usr/share/kraftaEditor)
        return assetRoot + "/" + subdir + "/";
#endif
    }

    wxString AssetsPath(const wxString &target)
    {
        static bool warned = false;
        wxString base = IsRunningInDevelopmentEnvironment()
            ? DevelopmentEnvironmentPath() + "assets" + PlatformInfos::OsPathSeparator()
            : InstalledBasePath("assets");

        if (!target.IsEmpty())
            base += target + wxFileName::GetPathSeparator();

        if (!wxDirExists(base) && !warned)
        {
            wxMessageBox(wxString::Format(_("AssetsPath: directory not found: %s"), base), _("Warning"), wxICON_WARNING);
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
        wxString base = IsRunningInDevelopmentEnvironment()
            ? DevelopmentEnvironmentPath() + "languages" + PlatformInfos::OsPathSeparator()
            : InstalledBasePath("languages");

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
        wxString base = IsRunningInDevelopmentEnvironment()
            ? DevelopmentEnvironmentPath() + "config" + PlatformInfos::OsPathSeparator()
            : InstalledBasePath("config");

        if (name.IsEmpty())
            return base;

        wxString path = base + name + PlatformInfos::OsPathSeparator();
        return wxDirExists(path) ? path : "";
    }

    wxString GetI18nLanguagePath()
    {
        wxString base = IsRunningInDevelopmentEnvironment()
            ? DevelopmentEnvironmentPath() + "i18n" + PlatformInfos::OsPathSeparator()
            : InstalledBasePath("i18n");

        if (wxDirExists(base))
            return base;

        wxMessageBox(wxString::Format(_("GetI18nLanguagePath: directory not found: %s"), base), _("Error"), wxICON_ERROR);
        return wxEmptyString;
    }
}