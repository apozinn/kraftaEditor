#pragma once

/**
 * @file terminal.hpp
 * @brief Declaration of the Terminal class, which simulates a basic command-line interface.
 */

#include "themesManager/themesManager.hpp"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <wx/wx.h>
#include <wx/process.h>

/**
 * @class Terminal
 * @brief A panel hosting a basic terminal/command-line interface.
 *
 * It uses a wxTextCtrl for input/output and wxProcess to execute external commands.
 */
class Terminal : public wxPanel
{
public:
    /**
     * @brief Constructs the Terminal panel.
     * @param parent The parent window.
     * @param ID The unique ID for the panel.
     */
    Terminal(wxWindow *parent, wxWindowID ID);

    /**
     * @brief Executes the command entered by the user in the input control.
     *
     * This function is typically bound to the wxEVT_TEXT_ENTER event of the input control.
     * @param WXUNUSED(event) The command event.
     */
    void OnCommand(wxCommandEvent & WXUNUSED(event));

    /**
     * @brief Moves the cursor to the end of the input field on click.
     *
     * This prevents the user from accidentally typing into previous command output.
     * @param WXUNUSED(event) The mouse event.
     */
    void OnCommandInputClick(wxMouseEvent &WXUNUSED(event));

private:
    wxProcess *m_process = new wxProcess(this); /**< Process object to handle asynchronous command execution. */
    wxTextCtrl *m_commandInput; /**< The multi-line text control used for both command input and output display. */
    json Theme = ThemesManager::Get().currentTheme; /**< Cached theme settings. */

    wxDECLARE_NO_COPY_CLASS(Terminal);
};