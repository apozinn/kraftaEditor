#include "mainApp.hpp"
#include "mainFrame.cpp"

bool KraftaEditor::OnInit()
{
    if (!wxApp::OnInit())
        return false;

    // getting the OS name
    osName = wxPlatformInfo::Get().GetOperatingSystemFamilyName();

    // verify if system theme is dark
    auto systemInfo = wxSystemSettings::GetAppearance();
    if (systemInfo.IsSystemDark())
    {
        SetAppearance(Appearance::Dark);
#if __WXMSW__
        MSWEnableDarkMode(DarkMode_Always);
#endif
    }
    // define the app dirs path
    applicationPath = wxStandardPaths::Get()
                          .GetUserConfigDir()
                          .ToStdString() +
                      osSlash + ".kraftaEditor" + osSlash;
    if (!wxDirExists(applicationPath))
    {
        try
        {
            bool created = wxFileName::Mkdir(applicationPath);
            if (!created)
            {
                wxMessageBox("an error occurred while creating the configuration dir");
                return false;
            }
        }
        catch (bool e)
        {
            return false;
            wxMessageBox("an error occurred while creating the configuration dir");
        }
    }

    icons_dir = applicationPath + "icons" + osSlash;
    assetsDir = applicationPath + "assets" + osSlash;

    // Get user config and theme
    UserConfigs = UserConfig().Get();
    UserTheme = UserConfig().GetThemes();

    if (!wxDirExists(icons_dir))
    {
        if (!CreateApplicationAssetsDirectories(GetAppDirs("icons").ToStdString(), "icons"))
        {
            return false;
        }
        else
        {
            if (!CreateApplicationAssetsDirectories(GetAppDirs("icons").ToStdString() + "file_ext/", "file_ext"))
            {
                return false;
            }
        }
    }

    if (!wxDirExists(assetsDir))
    {
        if (!CreateApplicationAssetsDirectories(GetAppDirs("assets").ToStdString(), "assets"))
        {
            return false;
        }
    }

    // init images handlers
    wxInitAllImageHandlers();

    // createv the main frame
    CreateNewWindow();
    return true;
}

void KraftaEditor::OnEventLoopEnter(wxEventLoopBase *WXUNUSED(loop))
{
    if (frame->CreateWatcherIfNecessary())
    {
        wxConfig *config = new wxConfig("krafta-editor");
        wxString str;

        if (!m_dirToWatch.empty())
        {
            frame->LoadPath(m_dirToWatch);
        }
        else
        {
            if (config->Read("workspace", &str))
            {
                wxString last_workspace = config->Read("workspace", str);
                frame->LoadPath(last_workspace);
            }
        }
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

void KraftaEditor::CreateNewWindow()
{
    frame = new MainFrame("Krafta Editor");
    frame->Show();
    wxApp::SetTopWindow(frame);
    mainFrame = frame;
}