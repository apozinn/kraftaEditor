#pragma once

#include <wx/wx.h>
#include <wx/stc/stc.h>
#include <themesManager/themesManager.hpp>

/**
 * @class Search
 * @brief Lightweight in-editor search panel similar to VS Code Ctrl+F.
 *
 * This panel provides real-time text search functionality for a
 * wxStyledTextCtrl instance. All occurrences are highlighted using
 * a dedicated indicator, and the first match is automatically focused.
 *
 * Features:
 * - Real-time search while typing
 * - ESC to close
 * - ENTER to jump to next occurrence
 * - Non-intrusive highlight using indicators
 *
 * This class does not own the editor instance.
 */
class Search : public wxPanel
{
public:
    /**
     * @brief Constructs the search panel.
     *
     * @param parent Parent window.
     * @param editor Target wxStyledTextCtrl where search is performed.
     */
    explicit Search(wxWindow *parent, const wxString &defaultLabel, wxStyledTextCtrl *editor);

    /**
     * @brief Destructor. Clears search indicators.
     */
    ~Search() override;

private:
    /**
     * @brief Handles character input events.
     *
     * Triggers real-time search and processes ENTER key
     * to move to the next match.
     *
     * @param event Styled text control event.
     */
    void OnChar(wxStyledTextEvent &event);

    /**
     * @brief Executes the search and highlights all matches.
     *
     * @param forward Direction of navigation when pressing ENTER.
     */
    void DoSearch(bool forward);

    /**
     * @brief Closes the search panel and clears highlights.
     *
     * @param event Command event.
     */
    void Close(wxCommandEvent &event);

    /**
     * @brief Draws the search panel.
     *
     * @param event Paint event.
     */
    void OnPaint(wxPaintEvent &event);

private:
    wxStyledTextCtrl *m_input;
    wxStyledTextCtrl *m_editor;

    wxColour m_borderColor = ThemesManager::Get().GetColor("border");

    static constexpr int SEARCH_INDICATOR = 8;
};
