#pragma once
#include <wx/defs.h>

/**
 * @file ids.hpp
 * @brief Centralized definitions for all GUI control IDs and event IDs
 *
 * Usage example:
 * @code
 * button->Bind(wxEVT_BUTTON, handler, +GUI::ControlID::ProjectToggler);
 * menu->Bind(wxEVT_MENU, handler, +Event::File::New);
 * @endcode
 */

// Dedicated ID ranges for each category
constexpr wxWindowID ID_BASE_FILE = 10000;     ///< Base ID for file operations
constexpr wxWindowID ID_BASE_FRAME = 11000;    ///< Base ID for frame operations
constexpr wxWindowID ID_BASE_PROJECT = 12000;  ///< Base ID for project operations
constexpr wxWindowID ID_BASE_VIEW = 13000;     ///< Base ID for view operations
constexpr wxWindowID ID_BASE_TERMINAL = 14000; ///< Base ID for terminal operations
constexpr wxWindowID ID_BASE_CONTROL = 15000;  ///< Base ID for control panel
constexpr wxWindowID ID_BASE_SEARCH = 16000;   ///< Base ID for search operations
constexpr wxWindowID ID_BASE_SETTINGS = 17000; ///< Base ID for user settings
constexpr wxWindowID ID_BASE_EDIT = 18000;     ///< Base ID for code edit

namespace GUI
{

    /**
     * @namespace GUI
     * @brief Contains all GUI control identifiers
     */

    /**
     * @enum ControlID
     * @brief Identifiers for visual controls and windows
     */
    enum class ControlID : wxWindowID
    {
        MainFrame = wxID_ANY,          ///< Main application frame (wxID_ANY = -1)
        ProjectToggler,                ///< Button/control to toggle project visibility
        Tabs,                          ///< Main tab control container
        TabsContainer,                 ///< Container widget holding all tabs
        CodeEditor,                    ///< Primary code editing control (scintilla, etc.)
        MiniMap,                       ///< Code minimap visualization panel
        CodeSearchPanel,               ///< Panel containing code search functionality
        FileSearchPanel,               ///< Panel containing file search functionality
        ApplicationLeftMainContainer,  ///< Left container in main application layout
        ApplicationRightMainContainer, ///< Right container in main application layout
        CenteredContent,               ///< Central content area container
        EmptyWindow,                   ///< Placeholder/empty window (initial state)
        MainSplitter,                  ///< Main window splitter control
        MainContainerSplitter,         ///< Secondary splitter for containers
        Terminal,                      ///< Integrated terminal emulator widget
        ControlPanel,                  ///< Auxiliary control panel for commands
        MainContainer,                 ///< Primary container element for central UI
        FilesTree,                     ///< Tree view showing project files structure
        ProjectFiles,                  ///< List control showing project files
        ProjectFilesContainer,         ///< Container widget for project files list
        MainCode,                      ///< Main code display and editing area
        StatusBar,                     ///< Application status bar at bottom
        StatusBarFileExt,              ///< Status bar element showing file extension
        StatusBarTabSize,              ///< Status bar element showing tab size setting
        StatusBarCodeLocale,           ///< Status bar element showing language mode
        MenuBar,                       ///< Main application menu bar
        NotifyPanel,                   ///< Notification/alert panel
        ProjectToolsName,              ///< Label showing project name in tools panel
        ProjectToolsArrow,             ///< Arrow control in project tools panel
        OpenFolderButton,              ///< Button to open folder dialog
    };

} // namespace GUI

namespace Event
{

    /**
     * @namespace Event
     * @brief Contains all event identifiers organized by functional domain
     */

    /**
     * @enum File
     * @brief File operation related events
     */
    enum class File : wxWindowID
    {
        New = ID_BASE_FILE + 1,             ///< Create new file
        OpenFile = ID_BASE_FILE + 2,        ///< Open existing file dialog
        CreateFileEvent = ID_BASE_FILE + 3, ///< Create new file (K suffix for disambiguation)
        RenameFile = ID_BASE_FILE + 4,      ///< Rename current file
        DeleteFileEvent = ID_BASE_FILE + 5, ///< Delete file (K suffix for disambiguation)
        CloseFile = ID_BASE_FILE + 6,       ///< Close current file
        Save = ID_BASE_FILE + 7,            ///< Save current file
        SaveAll = ID_BASE_FILE + 8,         ///< Save all open files
        SaveAs = ID_BASE_FILE + 9,          ///< Save file with new name dialog
        CloseAll = ID_BASE_FILE + 10,       ///< Close all open files
        CreateDir = ID_BASE_FILE + 11,      ///< Create new directory
        RenameDir = ID_BASE_FILE + 12,      ///< Rename directory
        DeleteDir = ID_BASE_FILE + 13       ///< Delete directory
    };

    /**
     * @enum Edit
     * @brief Text editing related events
     */
    enum class Edit : wxWindowID
    {
        Undo = wxID_UNDO,                      ///< Standard undo operation
        Redo = wxID_REDO,                      ///< Standard redo operation
        ToggleComment = ID_BASE_EDIT + 1,      ///< Toggle line comment on current line/selection
        ToggleBlockComment = ID_BASE_EDIT + 2, ///< Toggle block comment on selection
        SelectAll = ID_BASE_EDIT + 3,          ///< Select all text in current editor
        SelectLine = ID_BASE_EDIT + 4,         ///< Select current line
        Cut = wxID_CUT,                        ///< Standard cut operation
        Copy = wxID_COPY,                      ///< Standard copy operation
        Paste = wxID_PASTE                     ///< Standard paste operation
    };

    /**
     * @enum Frame
     * @brief Main frame related events
     */
    enum class Frame : wxWindowID
    {
        Exit = ID_BASE_FRAME + 1,      ///< Exit application
        NewWindow = ID_BASE_FRAME + 2, ///< Create new application window
        About = ID_BASE_FRAME + 3      ///< Show about dialog
    };

    /**
     * @enum View
     * @brief UI visibility and appearance events
     */
    enum class View : wxWindowID
    {
        ToggleMenuBar = ID_BASE_VIEW + 1,      ///< Show/hide menu bar
        ToggleStatusBar = ID_BASE_VIEW + 2,    ///< Show/hide status bar
        ToggleFileTree = ID_BASE_VIEW + 3,     ///< Show/hide file tree panel
        ToggleMiniMap = ID_BASE_VIEW + 4,      ///< Show/hide code minimap
        ToggleControlPanel = ID_BASE_VIEW + 5, ///< Show/hide control panel
        ToggleTabBar = ID_BASE_VIEW + 6,       ///< Show/hide tab bar
        ToggleCodeSearch = ID_BASE_VIEW + 7,   ///< Show/hide code search panel
        FocusMode = ID_BASE_VIEW + 8           ///< Toggle distraction-free focus mode
    };

    /**
     * @enum Project
     * @brief Project management events
     */
    enum class Project : wxWindowID
    {
        OpenFolder = ID_BASE_PROJECT + 1, ///< Open project folder dialog
        CloseFolder = ID_BASE_PROJECT + 2 ///< Close current project
    };

    /**
     * @enum User settings
     * @brief User settings events
     */
    enum class UserSettings : wxWindowID
    {
        Edit = ID_BASE_SETTINGS + 1 ///< Edit user preferences/settings
    };

    /**
     * @enum Terminal
     * @brief Terminal emulator events
     */
    enum class Terminal : wxWindowID
    {
        Open = ID_BASE_TERMINAL + 1,         ///< Open terminal panel
        Exit = ID_BASE_TERMINAL + 2,         ///< Close terminal panel
        ControlUp = ID_BASE_TERMINAL + 3,    ///< Terminal control navigation up
        ControlDown = ID_BASE_TERMINAL + 4,  ///< Terminal control navigation down
        ControlSelect = ID_BASE_TERMINAL + 5 ///< Terminal control selection
    };

    /**
     * @enum Control Panel
     * @brief Control Panel emulator events
     */
    enum class ControlPanel : wxWindowID
    {
        Exit = ID_BASE_CONTROL + 1,  ///< Exit control panel mode
        Up = ID_BASE_CONTROL + 2,    ///< Navigate up in control panel
        Down = ID_BASE_CONTROL + 3,  ///< Navigate down in control panel
        Select = ID_BASE_CONTROL + 4 ///< Select item in control panel
    };

    /**
     * @enum Code Search
     * @brief Code Search emulator events
     */
    enum class CodeSearch : wxWindowID
    {
        Open = ID_BASE_SEARCH + 1, ///< Open code search panel
        Close = ID_BASE_SEARCH + 2 ///< Close code search panel
    };

} // namespace Event

/**
 * @brief Safe conversion operator for ControlIDs
 * @param id The control ID to convert
 * @return wxWindowID value for wxWidgets APIs
 *
 * Enables using enum values directly in wxWidgets APIs with + prefix
 */
constexpr wxWindowID operator+(GUI::ControlID id)
{
    return static_cast<wxWindowID>(id);
}

/**
 * @brief Safe conversion operator for Event IDs
 * @tparam T Event enum type
 * @param id The event ID to convert
 * @return wxWindowID value for wxWidgets APIs
 *
 * Template version works with any Event namespace enum
 */
template <typename T>
constexpr wxWindowID operator+(T id)
{
    return static_cast<wxWindowID>(id);
}

// Static assertions to guarantee no ID conflicts
static_assert(+Event::File::New != +Event::Project::OpenFolder, "ID conflict detected!");
static_assert(+Event::Frame::Exit != +Event::Project::OpenFolder, "ID conflict detected!");
// Add more static assertions as needed