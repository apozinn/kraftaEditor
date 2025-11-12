#pragma once

/**
 * @file KraftaEditor.hpp
 * @brief Declaration of the KraftaEditor application class, derived from wxApp.
 *
 * Defines the application's entry point, initialization routines, theme and
 * settings management, and global event handling.
 */

#include "../frames/mainFrame.hpp"
#include "appPaths/appPaths.hpp"

/**
 * @class KraftaEditor
 * @brief The main application class for the Krafta Editor.
 *
 * Handles the application lifecycle, initialization, command-line parsing,
 * theme setup, and persistent settings management. It also manages the main
 * window (`MainFrame`).
 */
class KraftaEditor : public wxApp
{
public:
    /**
     * @brief The main initialization function, called when the application starts.
     * @return **true** if initialization was successful and the application should proceed; **false** otherwise.
     */
    virtual bool OnInit() override;

    /**
     * @brief Verifies the current system theme/mode (light/dark) and updates settings accordingly.
     */
    void VerifyIfSytemIsDarkMode();

    /**
     * @brief Initializes and loads the application's theme manager based on user settings.
     */
    void SetupThemeManager();

    /**
     * @brief Initializes and loads persistent user settings.
     */
    void SetupUserSettings();

    /**
     * @brief Creates necessary application directories (e.g., configuration, data).
     * @return **true** if all directories were successfully created or already exist; **false** otherwise.
     */
    bool SetupApplicationDirectories();

    /**
     * @brief Creates and displays the main application window (`MainFrame`).
     * @return **true** if the main window was successfully created; **false** otherwise.
     */
    bool CreateMainWindow();

    /**
     * @brief Loads language preferences for the application's localization (i18n).
     */
    void LoadLanguagesPreferences();

    /**
     * @brief Called when a new event loop is entered.
     *
     * Used here primarily for late initialization tasks that require an active event loop,
     * such as initializing the file system watcher.
     * @param WXUNUSED(loop) The event loop being entered.
     */
    virtual void OnEventLoopEnter(wxEventLoopBase *WXUNUSED(loop)) override;

    /**
     * @brief Initializes the command-line parser with application-specific arguments.
     * @param parser The command-line parser object.
     */
    virtual void OnInitCmdLine(wxCmdLineParser &parser) override;

    /**
     * @brief Called after the command line has been successfully parsed.
     *
     * Used to process command-line arguments (e.g., path to open).
     * @param parser The command-line parser object.
     * @return **true** if parsing was successful; **false** if an error occurred.
     */
    bool OnCmdLineParsed(wxCmdLineParser &parser);

    void LoadTranslations();

private:
    MainFrame *frame; /**< Pointer to the main application window. */
    wxString m_dirToWatch; /**< Directory path received from command line to be loaded. */
    const wxString appPath = ApplicationPaths::ApplicationPath(); /**< The base path for the application's configuration and data. */
};

wxIMPLEMENT_APP(KraftaEditor);
wxDECLARE_APP(KraftaEditor);