#pragma once
#include <wx/wx.h>
#include <wx/timer.h>

/**
 * @file pageSwitcher.hpp
 * @brief Declaration of the PageSwitcher UI component.
 */

/**
 * @event EVT_PAGE_SWITCH_FILES
 * @brief Emitted when the Files page is selected.
 */
wxDECLARE_EVENT(EVT_PAGE_SWITCH_FILES, wxCommandEvent);

/**
 * @event EVT_PAGE_SWITCH_SEARCH
 * @brief Emitted when the Search page is selected.
 */
wxDECLARE_EVENT(EVT_PAGE_SWITCH_SEARCH, wxCommandEvent);

/**
 * @class PageSwitcher
 * @brief Horizontal page selector used to switch between Files and Search views.
 *
 * This component provides a lightweight page switcher similar to modern editors
 * such as VS Code. It supports hover feedback and an active-page indicator drawn
 * as a rounded bottom line.
 *
 * The PageSwitcher does not directly manage page content; instead, it emits
 * events and updates its visual state when a page becomes active.
 */
class PageSwitcher : public wxPanel
{
public:
    /**
     * @brief Constructs a PageSwitcher.
     * @param parent Parent window.
     */
    explicit PageSwitcher(wxWindow *parent);

    /**
     * @brief Activates the Files page programmatically.
     *
     * Updates the visual state and emits EVT_PAGE_SWITCH_FILES.
     */
    void SetActiveFiles();

    /**
     * @brief Activates the Search page programmatically.
     *
     * Updates the visual state and emits EVT_PAGE_SWITCH_SEARCH.
     */
    void SetActiveSearch();

private:
    /**
     * @brief Creates and initializes the Files page button.
     */
    void CreateFilesPage();

    /**
     * @brief Creates and initializes the Search page button.
     */
    void CreateSearchPage();

    /**
     * @brief Handles mouse click on the Files page.
     * @param event Mouse event data.
     */
    void OnFilesClicked(wxMouseEvent &event);

    /**
     * @brief Handles mouse click on the Search page.
     * @param event Mouse event data.
     */
    void OnSearchClicked(wxMouseEvent &event);

    /**
     * @brief Handles mouse enter events for hover feedback.
     * @param event Mouse event data.
     */
    void OnMouseEnter(wxMouseEvent &event);

    /**
     * @brief Handles mouse leave events and clears hover state.
     * @param event Mouse event data.
     */
    void OnMouseLeave(wxMouseEvent &event);

    /**
     * @brief Paint handler for page panels.
     *
     * Draws the active page indicator as a rounded line at the bottom
     * of the active panel.
     *
     * @param event Paint event data.
     */
    void OnPaintPanel(wxPaintEvent &event);

    /**
     * @brief Sets the currently active panel.
     * @param panel Panel to activate.
     */
    void SetActive(wxPanel *panel);

    /**
     * @brief Updates background colors and triggers redraws
     *        according to hover and active states.
     */
    void UpdateVisuals();

private:
    /** Main horizontal sizer holding page panels. */
    wxBoxSizer *m_sizer{nullptr};

    /** Panel representing the Files page. */
    wxPanel *m_files_panel{nullptr};

    /** Panel representing the Search page. */
    wxPanel *m_search_panel{nullptr};

    /** Currently active page panel. */
    wxPanel *m_active_panel{nullptr};

    /** Panel currently under mouse hover, if any. */
    wxWindow *m_hover_panel{nullptr};

    /** Background color for normal state. */
    wxColour m_bg_normal;

    /** Background color for hover state. */
    wxColour m_bg_hover;

    /** Color used to draw the active page indicator line. */
    wxColour m_active_line;
};
