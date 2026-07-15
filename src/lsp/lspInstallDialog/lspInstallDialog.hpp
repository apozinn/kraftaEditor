#pragma once

#include <wx/wx.h>
#include "userSettings/userSettings.hpp"
#include "themesManager/themesManager.hpp"

/**
 * @brief Dialog prompting the user to install an LSP server.
 * 
 * LspInstallDialog presents a modal dialog asking the user whether
 * they want to download and install a language server for a specific
 * programming language. The dialog displays information about which
 * language server is needed and provides options to install it or
 * skip the installation.
 * 
 * The dialog is styled according to the current application theme
 * and provides callback hooks for the install and skip actions.
 * 
 * ## Usage Example:
 * @code
 * LspInstallDialog dialog(parent, "Python", "pylsp");
 * 
 * dialog.SetOnInstall([&]() {
 *     // Start download and installation
 *     LspDownloader downloader;
 *     downloader.Download(url, destPath);
 * });
 * 
 * dialog.SetOnSkip([&]() {
 *     // Continue without LSP support
 *     editor->DisableLsp();
 * });
 * 
 * dialog.ShowModal();
 * @endcode
 */
class LspInstallDialog : public wxDialog {
public:
    /**
     * @brief Constructs the installation prompt dialog.
     * 
     * Creates a modal dialog with information about the LSP server
     * that needs to be installed, along with "Install" and "Skip"
     * buttons. The dialog is centered on the parent window and styled
     * according to the current application theme.
     * 
     * @param parent The parent window for this dialog.
     * @param languageName The display name of the programming language
     *                     (e.g., "C++", "Python", "JavaScript").
     * @param lspName The name of the LSP server to install
     *                (e.g., "clangd", "pylsp", "typescript-language-server").
     */
    LspInstallDialog(wxWindow* parent, const wxString& languageName, 
                     const wxString& lspName);

private:
    /**
     * @brief Handles the "Install" button click event.
     * 
     * Invokes the user-provided installation callback if set,
     * then closes the dialog with the accept result.
     * 
     * @param event The button click event (unused).
     */
    void OnInstall(wxCommandEvent& event);
    
    /**
     * @brief Handles the "Skip" button click event.
     * 
     * Invokes the user-provided skip callback if set,
     * then closes the dialog with the cancel result.
     * 
     * @param event The button click event (unused).
     */
    void OnSkip(wxCommandEvent& event);

    /**
     * @brief Sets the callback for the install action.
     * 
     * This callback is invoked when the user clicks the "Install" button.
     * Use it to trigger the download and installation process.
     * 
     * @param cb Callback function to execute on install.
     */
    void SetOnInstall(std::function<void()> cb) { m_onInstall = cb; }
    
    /**
     * @brief Sets the callback for the skip action.
     * 
     * This callback is invoked when the user clicks the "Skip" button.
     * Use it to disable LSP features or continue without language support.
     * 
     * @param cb Callback function to execute on skip.
     */
    void SetOnSkip(std::function<void()> cb) { m_onSkip = cb; }

    std::function<void()> m_onInstall;  ///< Callback for install action.
    std::function<void()> m_onSkip;     ///< Callback for skip action.

    wxString m_languageName;  ///< Display name of the programming language.
    wxString m_lspName;       ///< Name of the LSP server to install.
};