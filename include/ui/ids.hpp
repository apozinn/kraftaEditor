#pragma once
#include <wx/defs.h>

/**
 * @file ids.hpp
 * @brief Central registry of GUI control IDs and application event IDs.
 *
 * This file defines all identifiers used by wxWidgets controls, menus,
 * commands and custom events. IDs are grouped by functional domain and
 * assigned fixed numeric ranges to avoid collisions.
 *
 * Usage examples:
 * @code
 * button->Bind(wxEVT_BUTTON, handler, +GUI::ControlID::ProjectToggler);
 * menu->Bind(wxEVT_MENU, handler, +Event::File::New);
 * @endcode
 */

/* -------------------------------------------------------------------------- */
/* ID BASE RANGES                                                              */
/* -------------------------------------------------------------------------- */

constexpr wxWindowID ID_BASE_FILE        = 10000; ///< File system and file actions
constexpr wxWindowID ID_BASE_FRAME       = 11000; ///< Main window and frame actions
constexpr wxWindowID ID_BASE_PROJECT     = 12000; ///< Project lifecycle actions
constexpr wxWindowID ID_BASE_VIEW        = 13000; ///< UI visibility and layout toggles
constexpr wxWindowID ID_BASE_TERMINAL    = 14000; ///< Integrated terminal controls
constexpr wxWindowID ID_BASE_CONTROL     = 15000; ///< Command/control panel navigation
constexpr wxWindowID ID_BASE_QUICK_OPEN  = 15500; ///< Quick Open navigation and actions
constexpr wxWindowID ID_BASE_SEARCH      = 16000; ///< Search panels and search actions
constexpr wxWindowID ID_BASE_SETTINGS    = 17000; ///< User preferences and settings
constexpr wxWindowID ID_BASE_EDIT        = 18000; ///< Text editing and code manipulation
constexpr wxWindowID ID_TABS_CONTAINER   = 19000; ///< Editor tab container actions

/* -------------------------------------------------------------------------- */
/* GUI CONTROL IDS                                                             */
/* -------------------------------------------------------------------------- */

namespace GUI
{
    /**
     * @enum ControlID
     * @brief Identifiers for visible widgets and layout components.
     *
     * These IDs are assigned to wxWindow-derived objects such as panels,
     * buttons, splitters and custom controls.
     */
    enum class ControlID : wxWindowID
    {
        MainFrame = wxID_ANY,          ///< Main application window
        ProjectToggler,               ///< Button that shows/hides the project tree
        Tabs,                         ///< Main editor tabs control
        TabsContainer,                ///< Container holding all editor tabs
        CodeEditor,                   ///< Primary source code editor widget
        MiniMap,                      ///< Code minimap view
        CodeSearchPanel,              ///< In-editor text search panel
        FileSearchPanel,              ///< Workspace-wide file search panel
        ApplicationLeftMainContainer, ///< Left side of the main layout
        ApplicationRightMainContainer,///< Right side of the main layout
        CenteredContent,              ///< Central editor/content area
        EmptyWindow,                  ///< Placeholder window (no file open)
        MainSplitter,                 ///< Root splitter dividing main layout
        MainContainerSplitter,        ///< Secondary splitter for nested containers
        Terminal,                     ///< Embedded terminal widget
        TerminalCommandInput,         ///< Terminal command input field
        ControlPanel,                 ///< Command palette / control panel
        MainContainer,                ///< Root container for main UI
        FilesTree,                    ///< Project file tree view
        ProjectFiles,                 ///< Flat list of project files
        ProjectFilesContainer,        ///< Container for project files view
        MainCode,                     ///< Main code editing area
        StatusBar,                    ///< Application status bar
        StatusBarFileExt,             ///< Status bar file extension indicator
        StatusBarTabSize,             ///< Status bar tab size indicator
        StatusBarCodeLocale,          ///< Status bar language mode indicator
        MenuBar,                      ///< Application menu bar
        NotifyPanel,                  ///< Notification and alert panel
        ProjectToolsName,             ///< Project name label in tools panel
        ProjectToolsArrow,            ///< Expand/collapse arrow for project tools
        QuickOpenSearchBar,           ///< Input field for Quick Open
        OpenFolderButton,             ///< Button to open a project folder
        QuickOpen,                    ///< Quick Open overlay panel
        ProjectInfosTools,            ///< Project information tools panel
        PageSwitcher,                 ///< Sidebar page switcher (Files/Search)
        OpenFileFromDrop,             ///< Drag-and-drop file open handler
        FilesPage,                    ///< Files page inside page switcher
        PageSwitcherSearchPage,       ///< Search page inside page switcher
        SearchPage                    ///< Full search results page
    };
}

/* -------------------------------------------------------------------------- */
/* EVENT IDS                                                                   */
/* -------------------------------------------------------------------------- */

namespace Event
{
    /**
     * @enum File
     * @brief Events related to file and directory operations.
     */
    enum class File : wxWindowID
    {
        New              = ID_BASE_FILE + 1,
        OpenFile         = ID_BASE_FILE + 2,
        CreateFileEvent  = ID_BASE_FILE + 3,
        RenameFile       = ID_BASE_FILE + 4,
        DeleteFileEvent  = ID_BASE_FILE + 5,
        CloseFile        = ID_BASE_FILE + 6,
        Save             = ID_BASE_FILE + 7,
        SaveAll          = ID_BASE_FILE + 8,
        SaveAs           = ID_BASE_FILE + 9,
        CloseAll         = ID_BASE_FILE + 10,
        CreateDir        = ID_BASE_FILE + 11,
        RenameDir        = ID_BASE_FILE + 12,
        DeleteDir        = ID_BASE_FILE + 13
    };

    /**
     * @enum Edit
     * @brief Text editing and code manipulation commands.
     */
    enum class Edit : wxWindowID
    {
        Undo                   = wxID_UNDO,
        Redo                   = wxID_REDO,
        ToggleComment          = ID_BASE_EDIT + 1,
        ToggleBlockComment     = ID_BASE_EDIT + 2,
        SelectAll              = ID_BASE_EDIT + 3,
        SelectLine             = ID_BASE_EDIT + 4,
        Cut                    = wxID_CUT,
        Copy                   = wxID_COPY,
        Paste                  = wxID_PASTE,
        MoveCursorDown         = ID_BASE_EDIT + 5,
        MoveCursorUp           = ID_BASE_EDIT + 6,
        DuplicateLineDown      = ID_BASE_EDIT + 7,
        DuplicateLineUp        = ID_BASE_EDIT + 8,
        SelectNextOccurrence   = ID_BASE_EDIT + 9,
        AutoComplete           = ID_BASE_EDIT + 10,
        MoveLineUp             = ID_BASE_EDIT + 11,
        MoveLineDown           = ID_BASE_EDIT + 12,
        RemoveCurrentLine      = ID_BASE_EDIT + 13
    };

    /**
     * @enum Frame
     * @brief Application-level window actions.
     */
    enum class Frame : wxWindowID
    {
        Exit             = ID_BASE_FRAME + 1,
        NewWindow        = ID_BASE_FRAME + 2,
        About            = ID_BASE_FRAME + 3,
        DontSaveChanges  = ID_BASE_FRAME + 4
    };

    /**
     * @enum View
     * @brief UI layout and visibility toggles.
     */
    enum class View : wxWindowID
    {
        ToggleMenuBar     = ID_BASE_VIEW + 1,
        ToggleStatusBar   = ID_BASE_VIEW + 2,
        ToggleFileTree    = ID_BASE_VIEW + 3,
        ToggleMiniMap     = ID_BASE_VIEW + 4,
        ToggleControlPanel= ID_BASE_VIEW + 5,
        ToggleTabBar      = ID_BASE_VIEW + 6,
        ToggleCodeSearch  = ID_BASE_VIEW + 7,
        FocusMode         = ID_BASE_VIEW + 8,
        ToggleQuickOpen   = ID_BASE_VIEW + 9
    };

    /**
     * @enum Project
     * @brief Project open/close lifecycle events.
     */
    enum class Project : wxWindowID
    {
        OpenFolder  = ID_BASE_PROJECT + 1,
        CloseFolder = ID_BASE_PROJECT + 2
    };

    /**
     * @enum UserSettings
     * @brief User preferences and configuration actions.
     */
    enum class UserSettings : wxWindowID
    {
        Edit = ID_BASE_SETTINGS + 1
    };

    /**
     * @enum Terminal
     * @brief Integrated terminal control and navigation.
     */
    enum class Terminal : wxWindowID
    {
        Open          = ID_BASE_TERMINAL + 1,
        Exit          = ID_BASE_TERMINAL + 2,
        ControlUp     = ID_BASE_TERMINAL + 3,
        ControlDown   = ID_BASE_TERMINAL + 4,
        ControlSelect = ID_BASE_TERMINAL + 5
    };

    /**
     * @enum ControlPanel
     * @brief Command palette navigation events.
     */
    enum class ControlPanel : wxWindowID
    {
        Exit   = ID_BASE_CONTROL + 1,
        Up     = ID_BASE_CONTROL + 2,
        Down   = ID_BASE_CONTROL + 3,
        Select = ID_BASE_CONTROL + 4
    };

    /**
     * @enum QuickOpen
     * @brief Quick Open navigation and selection events.
     */
    enum class QuickOpen : wxWindowID
    {
        Exit   = ID_BASE_QUICK_OPEN + 1,
        Up     = ID_BASE_QUICK_OPEN + 2,
        Down   = ID_BASE_QUICK_OPEN + 3,
        Select = ID_BASE_QUICK_OPEN + 4
    };

    /**
     * @enum CodeSearch
     * @brief Workspace and editor search panel actions.
     */
    enum class CodeSearch : wxWindowID
    {
        Open  = ID_BASE_SEARCH + 1,
        Close = ID_BASE_SEARCH + 2
    };

    /**
     * @enum TabsContainer
     * @brief Editor tab container navigation and management.
     */
    enum class TabsContainer : wxWindowID
    {
        OpenFirstTab       = ID_TABS_CONTAINER + 1,
        OpenLastTab        = ID_TABS_CONTAINER + 2,
        CloseAllSavedTabs  = ID_TABS_CONTAINER + 3
    };
}

/* -------------------------------------------------------------------------- */
/* ID CONVERSION HELPERS                                                       */
/* -------------------------------------------------------------------------- */

constexpr wxWindowID operator+(GUI::ControlID id)
{
    return static_cast<wxWindowID>(id);
}

template <typename T>
constexpr wxWindowID operator+(T id)
{
    return static_cast<wxWindowID>(id);
}

/* -------------------------------------------------------------------------- */
/* SAFETY CHECKS                                                               */
/* -------------------------------------------------------------------------- */

static_assert(+Event::File::New != +Event::Project::OpenFolder, "ID conflict detected");
static_assert(+Event::Frame::Exit != +Event::Project::OpenFolder, "ID conflict detected");
