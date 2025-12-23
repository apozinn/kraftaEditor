#pragma once

/**
 * @file MainFrame.hpp
 * @brief Declaration of the MainFrame class, the main application window of the Krafta Editor.
 *
 * This file defines the central window structure, manages UI components like the
 * file tree, editor tabs, terminal, and status bar, and handles global events
 * such as file system changes, window resizing, and application commands.
 */

#include <string>
#include <iostream>
#include <filesystem>

#include <wx/wx.h>
#include <wx/splitter.h>
#include <wx/fswatcher.h>
#include <wx/cmdline.h>
#include <wx/config.h>
#include <wx/settings.h>
#include <wx/platinfo.h>
#include <wx/dirdlg.h>
#include <wx/sashwin.h>
#include <wx/image.h>
#include <wx/file.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>

#if wxUSE_CLIPBOARD
#include <wx/dataobj.h>
#include <wx/clipbrd.h>
#endif

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "appPaths/appPaths.hpp"
#include "userSettings/userSettings.hpp"
#include "themesManager/themesManager.hpp"
#include "ui/ids.hpp"
#include "platformInfos/platformInfos.hpp"
#include "projectSettings/projectSettings.hpp"

// UI Component Includes
#include "gui/widgets/menuBar/menuBar.hpp"
#include "gui/panels/filesTree/filesTree.hpp"
#include "gui/codeContainer/code.hpp"
#include "gui/codeContainer/codeSearch/codeSearch.hpp"
#include "gui/panels/tabs/tabs.hpp"
#include "gui/widgets/emptyWindow/emptyWindow.hpp"
#include "gui/panels/terminal/terminal.hpp"
#include "gui/widgets/statusBar/statusBar.hpp"
#include "gui/widgets/openFolderButton/openFolderButton.hpp"
#include "gui/panels/controlPanel/controlPanel.hpp"
#include "gui/panels/quickOpen/quickOpen.hpp"

#include "FrameFileDropTarget/FrameFileDropTarget.hpp"

/**
 * @class MainFrame
 * @brief The main top-level window of the Krafta Editor application.
 *
 * Manages the layout, creation, and destruction of all major UI components
 * (file explorer, tab bar, editor, terminal, status bar). It also handles
 * persistent user settings (window size, maximized state), global commands,
 * and monitors file system changes for the currently open project.
 */
class MainFrame : public wxFrame
{
public:
    /**
     * @brief Constructs the MainFrame.
     * @param title The title displayed in the frame's title bar.
     */
    MainFrame(const wxString &title = "Krafta Editor");

    /**
     * @brief Destructor.
     *
     * Performs necessary cleanup, especially closing the file system watcher.
     */
    virtual ~MainFrame();

    // --- File System and Project Management ---

    /**
     * @brief Adds an entry (file or directory) to the file system watcher queue.
     * @param type The type of file system entry (file or directory).
     * @param filename The path of the file or directory to process.
     */
    void AddEntry(wxFSWPathType type, wxString filename = wxString());

    /**
     * @brief Creates and initializes the wxFileSystemWatcher if a project folder is open.
     * @return **true** if the watcher was successfully created and attached to the event loop; **false** otherwise.
     */
    bool CreateWatcherIfNecessary();

    /**
     * @brief Opens the folder selection dialog and attempts to load the selected path.
     */
    void OpenFolderDialog();

    /**
     * @brief Loads the contents of a specified path (directory) into the file tree and sets up the watcher.
     * @param path The directory path to load as the current project workspace.
     */
    void LoadPath(wxString path);

    // --- Public Command Handlers ---

    /**
     * @brief Handler for the "Open Folder" command from the menu bar.
     * @param WXUNUSED(event) The command event.
     */
    void OnOpenFolderMenu(wxCommandEvent &WXUNUSED(event));

    /**
     * @brief Handler for mouse clicks on the visual "Open Folder" button.
     * @param WXUNUSED(event) The mouse event.
     */
    void OnOpenFolderClick(wxMouseEvent &WXUNUSED(event));

    /**
     * @brief Closes all currently open editor tabs.
     * @param WXUNUSED(event) The command event.
     */
    void CloseAllFiles(wxCommandEvent &WXUNUSED(event));

    /**
     * @brief Handler for the "Open File" command.
     * @param WXUNUSED(event) The command event.
     */
    void OnOpenFile(wxCommandEvent &WXUNUSED(event));

    /**
     * @brief Closes the currently open project folder (clears file tree and watchers).
     * @param WXUNUSED(event) The command event.
     */
    void OnCloseFolder(wxCommandEvent &WXUNUSED(event));

    /**
     * @brief Launches a new instance of the application.
     * @param WXUNUSED(event) The command event.
     */
    void OnNewWindow(wxCommandEvent &WXUNUSED(event));

    /**
     * @brief Handler for the "Edit Settings" command, typically opening a settings file.
     * @param WXUNUSED(event) The command event.
     */
    void OnEditSettings(wxCommandEvent &WXUNUSED(event));

    // --- Layout and View Toggles ---

    /**
     * @brief Toggles the visibility of the global search panel.
     * @param WXUNUSED(event) The command event.
     */
    void OnToggleSearch(wxCommandEvent &WXUNUSED(event));

    /**
     * @brief Toggles the visibility of the Control Panel (e.g., source control).
     * @param WXUNUSED(event) The command event.
     */
    void OnToggleControlPanel(wxCommandEvent &WXUNUSED(event));

    /**
     * @brief Toggles the visibility of the Quick Open panel.
     * @param WXUNUSED(event) The command event.
     */
    void OnToggleQuickOpen(wxCommandEvent &WXUNUSED(event));

    /**
     * @brief Toggles the visibility of the file tree view panel.
     * @param WXUNUSED(event) The command event.
     */
    void OnToggleFileTreeView(wxCommandEvent &WXUNUSED(event));

    /**
     * @brief Toggles the visibility of the main Menu Bar.
     * @param WXUNUSED(event) The command event.
     */
    void OnToggleMenuBarView(wxCommandEvent &WXUNUSED(event));

    /**
     * @brief Toggles the visibility of the Status Bar.
     * @param WXUNUSED(event) The command event.
     */
    void OnToggleStatusBarView(wxCommandEvent &WXUNUSED(event));

    /**
     * @brief Toggles the visibility of the editor tab bar.
     * @param WXUNUSED(event) The command event.
     */
    void OnToggleTabBarView(wxCommandEvent &WXUNUSED(event));

    // --- Splitter Window/Sash Handlers ---

    /**
     * @brief Custom painting function for the sash (divider) of a splitter window.
     * @param dc The device context for drawing.
     * @param target The splitter window being painted.
     */
    void PaintSash(wxDC &dc, wxSplitterWindow *target);

    /**
     * @brief Handler for changes in the splitter sash position.
     *
     * Used to save the current sash position to user settings.
     * @param event The splitter event.
     */
    void OnSashPosChange(wxSplitterEvent &event);

    /**
     * @brief Handler for paint events, used primarily for custom styling of containers.
     * @param event The paint event.
     */
    void OnPaintedComponent(wxPaintEvent &event);

    /**
     * @brief Toggles the visibility of the minimap panel.
     *
     * Enables or disables the minimap view associated with the editor,
     * allowing the user to quickly show or hide it via a command or shortcut.
     *
     * @param event Command event (unused).
     */
    void OnToggleMinimapView(wxCommandEvent &WXUNUSED(event));

private:
    // --- UI Component Pointers ---
    MenuBar *m_menuBar;                             /**< The main menu bar control. */
    FilesTree *m_filesTree;                         /**< The file explorer/tree view panel. */
    Tabs *m_tabs;                                   /**< The tab control managing open editor buffers. */
    EmptyWindow *m_emptyWindow;                     /**< Placeholder window shown when no project is open. */
    Terminal *m_terminal;                           /**< The integrated terminal panel. */
    StatusBar *m_statusBar;                         /**< The application status bar. */
    OpenFolderButton *m_openFolderButton;           /**< Button shown in the empty workspace view. */
    ControlPanel *m_controlPanel;                   /**< Panel for controls like source control or project settings. */
    QuickOpen *m_quickOpen;                         /**< Panel for quick file opening. */
    wxSplitterWindow *m_mainSplitter;               /**< The primary splitter dividing the file tree and editor area. */
    wxPanel *m_applicationLeftMainContainer;        /**< Container for the file tree and related side content. */
    wxPanel *m_applicationRightMainContainer;       /**< Container for the tabs and editor area. */
    wxSplitterWindow *m_mainContainerSplitter;      /**< Splitter dividing the editor area from the terminal/output panel. */
    wxPanel *m_mainContainer;                       /**< The panel holding the editor/tabs area. */
    wxPanel *m_centeredContent;                     /**< Panel for content centered in the editor area (e.g., when empty). */
    wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL); /**< The main sizer managing the frame layout. */

    // --- File System Watcher ---
    wxFileSystemWatcher *m_watcher = nullptr; /**< The file system watcher instance for monitoring project changes. */
    bool m_followLinks;                       /**< Flag indicating whether the watcher should follow symbolic links. */

    // --- Configuration and State ---
    ProjectSettings &projectSettings = ProjectSettings::Get();         /**< Reference to global project settings manager. */
    json UserSettings = UserSettingsManager::Get().currentSettings;    /**< Current user settings (window state, theme, etc.). */
    json Theme = ThemesManager::Get().currentTheme;                    /**< Current visual theme configuration. */
    wxString assetsImagePath = ApplicationPaths::AssetsPath("images"); /**< Path to the application's image assets. */
    wxString appPath = ApplicationPaths::ApplicationPath();            /**< The base application execution path. */

    // --- Private Setup and Utility Methods ---

    /**
     * @brief Initializes the wxFileSystemWatcher object.
     */
    void CreateWatcher();

    /**
     * @brief Sets the application icon based on available assets.
     * @return **true** if the icon was set successfully; **false** otherwise.
     */
    bool SetAppIcon();

    /**
     * @brief Configures the main menu bar.
     */
    void SetupMenuBar();

    /**
     * @brief Sets up the main splitter window (`m_mainSplitter`).
     */
    void SetupMainSplitter();

    /**
     * @brief Sets up the container for the left-hand side panels (file tree).
     */
    void SetupApplicationLeftMainContainer();

    /**
     * @brief Initializes and configures the file tree component.
     */
    void SetupFilesTree();

    /**
     * @brief Sets up the container for the right-hand side panels (editor/tabs).
     */
    void SetupApplicationRightMainContainer();

    /**
     * @brief Sets up the splitter dividing the editor area from the output/terminal area.
     */
    void SetupMainContainerSplitter();

    /**
     * @brief Configures the main editor container panel.
     */
    void SetupMainContainer();

    /**
     * @brief Sets up the centered content panel (for the empty workspace view).
     */
    void SetupCenteredContent();

    /**
     * @brief Initializes and configures the tab control.
     */
    void SetupTabs();

    /**
     * @brief Initializes and configures the empty window placeholder.
     */
    void SetupEmptyWindow();

    /**
     * @brief Initializes and configures the terminal panel.
     */
    void SetupTerminal();

    /**
     * @brief Initializes and configures the status bar.
     */
    void SetupStatusBar();

    /**
     * @brief Sets up the application's keyboard accelerators.
     */
    void SetupAccelerators();

    /**
     * @brief Helper function containing logic related to saving and restoring window size/position.
     */
    void WindowResizeFunctions();

    // --- Private Event Handlers ---

    /**
     * @brief Handler for the window close event (**wxEVT_CLOSE**).
     *
     * Used for confirmation checks (unsaved files) and final cleanup.
     * @param event The close event.
     */
    void OnClose(wxCloseEvent &event);

    /**
     * @brief Handler for the "Exit" command (**wxID_EXIT**).
     * @param WXUNUSED(event) The command event.
     */
    void OnExit(wxCommandEvent &WXUNUSED(event));

    /**
     * @brief Handler for the "Watch" command, related to file system monitoring settings.
     * @param event The command event.
     */
    void OnWatch(wxCommandEvent &event);

    /**
     * @brief Handler for toggling symbolic link following in the file system watcher.
     * @param event The command event.
     */
    void OnFollowLinks(wxCommandEvent &event);

    /**
     * @brief Handler for the "About" command (**wxID_ABOUT**).
     * @param WXUNUSED(event) The command event.
     */
    void OnAbout(wxCommandEvent &WXUNUSED(event));

    /**
     * @brief Handler for file system change events detected by the watcher.
     *
     * Processes changes like file creation, deletion, or modification.
     * @param event The file system watcher event.
     */
    void OnFileSystemEvent(wxFileSystemWatcherEvent &event);

    /**
     * @brief Handler for the "Open Terminal" command.
     * @param WXUNUSED(event) The command event.
     */
    void OnOpenTerminal(wxCommandEvent &WXUNUSED(event));

    /**
     * @brief Handler for frame resizing events (**wxEVT_SIZE**).
     *
     * Used to save non-maximized window dimensions to user settings.
     * @param event The size event.
     */
    void OnFrameResized(wxSizeEvent &event);

    /**
     * @brief Handler for frame maximization events (**wxEVT_MAXIMIZE**).
     *
     * Used to save the maximized state to user settings.
     * @param WXUNUSED(event) The maximize event.
     */
    void OnFrameMaximized(wxMaximizeEvent &WXUNUSED(event));

    wxDECLARE_NO_COPY_CLASS(MainFrame);
    wxDECLARE_EVENT_TABLE();
};