#include "./application.hpp"
#include "platformInfos/platformInfos.hpp"
#include "appConstants/appConstants.hpp"
#include "themesManager/themesManager.hpp"
#include "userSettings/userSettings.hpp"
#include "languagesPreferences/languagesPreferences.hpp"

#if __WXMSW__
#include <wx/msw/private.h>
#include <wx/msw/darkmode.h>
#endif

#include <wx/log.h>
#include <wx/intl.h>
#include <wx/string.h>
#include <wx/translation.h>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

using namespace PlatformInfos;

bool KraftaEditor::OnInit()
{
    try
    {
        if (!wxApp::OnInit())
            return false;

        wxInitAllImageHandlers();
        LoadTranslations();

        VerifyIfSytemIsDarkMode();
        SetupThemeManager();
        SetupUserSettings();

        if (!SetupApplicationDirectories())
            return false;

        CreateMainWindow();
        LoadLanguagesPreferences();

        return true;
    }
    catch (const nlohmann::json::type_error &e)
    {
        wxMessageBox(e.what());
        std::cerr << "JSON Type Error: " << e.what() << std::endl;
    }
    catch (const std::exception &e)
    {
        wxMessageBox(e.what());
        std::cerr << "Standard Exception: " << e.what() << std::endl;
    }

    return false;
}

void KraftaEditor::VerifyIfSytemIsDarkMode()
{
    try
    {
        auto systemInfo = wxSystemSettings::GetAppearance();
        if (systemInfo.IsSystemDark())
        {
            SetAppearance(wxApp::Appearance::Dark);
#if __WXMSW__
            MSWEnableDarkMode(DarkMode_Always);
#endif
        } else {
            SetAppearance(wxApp::Appearance::Light);
        }
    }
    catch (const std::exception &e)
    {
        wxMessageBox(e.what());
        std::cerr << "Standard Exception: " << e.what() << std::endl;
    }
}

void KraftaEditor::SetupThemeManager()
{
    ThemesManager::Get();
}

void KraftaEditor::SetupUserSettings()
{
    UserSettingsManager::Get();
}

bool KraftaEditor::SetupApplicationDirectories()
{
    struct AppPathWithErrorMessage
    {
        wxString path;
        const char *errorMessage;
    };

    std::vector<AppPathWithErrorMessage> appPathsWithErrorMessage = {
        {ApplicationPaths::ApplicationPath(),
         "Failed to locate the application executable path."},
        {ApplicationPaths::AssetsPath(wxEmptyString),
         "Critical error: Unable to locate the application's assets directory."},
        {ApplicationPaths::AssetsPath("icons"),
         "UI Error: The application icons directory was not found."},
        {ApplicationPaths::AssetsPath("themes"),
         "UI Error: Missing application themes directory."},
        {ApplicationPaths::AssetsPath("images"),
         "UI Error: Missing application directory."},
        {ApplicationPaths::AssetsPath("file_ext"),
         "Language Support Error: Missing file extension icons directory."},
        {ApplicationPaths::DevelopmentEnvironmentPath(),
         "Configuration Error: Failed to determine the application root directory."}};

    bool allPathsOk = true;
    for (const auto &AppPath : appPathsWithErrorMessage)
    {
        if (!wxDirExists(AppPath.path))
        {
            allPathsOk = false;
            wxLogError(AppPath.errorMessage);
            break;
        }
    }
    return allPathsOk;
}

bool KraftaEditor::CreateMainWindow()
{
    frame = new MainFrame("Krafta Editor");
    frame->Show();
    wxApp::SetTopWindow(frame);

    return frame != nullptr;
}

void KraftaEditor::OnEventLoopEnter(wxEventLoopBase *WXUNUSED(loop))
{
    static bool s_watcherInitialized = false;
    if (s_watcherInitialized)
    {
        return;
    }

    if (!frame || frame->IsBeingDeleted())
        return;

    if (!frame->CreateWatcherIfNecessary())
        return;

    s_watcherInitialized = true;

    wxConfig config("krafta-editor");
    wxString str;

    if (!m_dirToWatch.empty())
    {
        frame->LoadPath(m_dirToWatch);
    }
    else if (config.Read("workspace", &str))
    {
        frame->LoadPath(str);
    }
}

void KraftaEditor::OnInitCmdLine(wxCmdLineParser &parser)
{
    wxApp::OnInitCmdLine(parser);
    parser.AddParam("directory to watch", wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL);
}

bool KraftaEditor::OnCmdLineParsed(wxCmdLineParser &parser)
{
    if (!wxApp::OnCmdLineParsed(parser))
        return false;
    if (parser.GetParamCount())
        m_dirToWatch = parser.GetParam();
    return true;
}

void KraftaEditor::LoadLanguagesPreferences()
{
    LanguagesPreferences::Get();
}

void KraftaEditor::LoadTranslations()
{
    wxTranslations *trans = new wxTranslations();
    wxTranslations::Set(trans);

    wxFileTranslationsLoader::AddCatalogLookupPathPrefix(ApplicationPaths::GetI18nLanguagePath());

    trans->AddStdCatalog();

    auto sysLang = wxLocale::GetSystemLanguage();

    wxLocale locale;

    if (!locale.Init(sysLang))
    {
        wxLogWarning("The system language (%s) is not fully supported. Using the default language.", wxLocale::GetLanguageName(sysLang));
    }

    if (!trans->AddCatalog("kraftaeditor"))
    {
        wxLogWarning("Failed to load translation catalog '%s'. Check the .mo file.", "kraftaeditor");
    }
}