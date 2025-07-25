#pragma once

#include "./application.hpp"
#include "application.hpp"
#include "platformInfos/platformInfos.hpp"
#include "appConstants/appConstants.hpp"
#include "themesManager/themesManager.hpp"
#include "userSettings/userSettings.hpp"

#if __WXMSW__
#include <wx/msw/private.h>
#include <wx/msw/darkmode.h>
#endif

#include "languagesPreferences/languagesPreferences.hpp"

#include <nlohmann/json.hpp>
using json = nlohmann::json;


using namespace PlatformInfos; 

bool KraftaEditor::OnInit()
{
    if (!wxApp::OnInit())
        return false;

    wxInitAllImageHandlers();

    VerifyIfSytemIsDarkMode();
    SetupThemeManager();
    SetupUserSettings();

    if (!SetupApplicationDirectories())
        return false;

    CreateMainWindow();

    json lp = LanguagesPreferences::Get().GetLanguagePreferences("html");
    wxLogMessage(wxString(lp["name"].template get<std::string>()));

    return true;
}

void KraftaEditor::VerifyIfSytemIsDarkMode()
{
    auto systemInfo = wxSystemSettings::GetAppearance();
    if (systemInfo.IsSystemDark())
    {
        SetAppearance(Appearance::Dark);
#if __WXMSW__
        MSWEnableDarkMode(DarkMode_Always);
#endif
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
    return frame ? true : false;
}

void KraftaEditor::OnEventLoopEnter(wxEventLoopBase *WXUNUSED(loop))
{
    if (!frame || !frame->CreateWatcherIfNecessary())
        return;

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