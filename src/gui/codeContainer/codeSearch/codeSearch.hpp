#pragma once

/**
 * @file codeSearch.hpp
 * @brief Declaration of the Search panel class, used for in-editor text searching.
 */

#include "ui/ids.hpp"
#include "themesManager/themesManager.hpp"

#include <wx/wx.h>
#include <wx/stc/stc.h>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

/**
 * @class Search
 * @brief A floating panel used for performing search operations within the active code editor.
 *
 * It provides an input field to enter the search text and handles the logic
 * for highlighting matches in a wxStyledTextCtrl.
 */
class Search : public wxPanel
{
public:
    /**
     * @brief Constructs the Search panel.
     * @param parent The parent window (usually the MainFrame or the editor container).
     * @param defaultLabel The initial text to display in the search input.
     * @param codeEditor A pointer to the wxStyledTextCtrl instance where the search will be performed.
     */
    Search(wxWindow *parent, wxString defaultLabel, wxStyledTextCtrl *codeEditor);

private:
    /**
     * @brief Handles input events, specifically when the Enter key is pressed to initiate a search.
     * @param event The styled text control event.
     */
    void EnterEvent(wxStyledTextEvent &event);

    /**
     * @brief Closes and destroys the search panel.
     *
     * Also clears any active highlighting in the editor.
     * @param WXUNUSED(event) The command event (e.g., triggered by Escape key).
     */
    void Close(wxCommandEvent &WXUNUSED(event));

    /**
     * @brief Handles the paint event for the panel.
     * @param WXUNUSED(event) The paint event.
     */
    void OnPaint(wxPaintEvent &WXUNUSED(event));
    
private:
    wxStyledTextCtrl *input; /**< The input field where the user types the search query. */
    wxStyledTextCtrl *currentEditor = nullptr; /**< The code editor being searched. */
    wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL); /**< The main sizer for the panel. */
    json Theme = ThemesManager::Get().currentTheme; /**< Cache of the current theme settings. */

    wxDECLARE_NO_COPY_CLASS(Search);
    wxDECLARE_EVENT_TABLE();
};