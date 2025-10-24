#pragma once

/**
 * @file statusBar.hpp
 * @brief Declaration of the StatusBar class, which displays various file and editor status information.
 */

#include "themesManager/themesManager.hpp"
#include "appPaths/appPaths.hpp"
#include "userSettings/userSettings.hpp"
#include "languagesPreferences/languagesPreferences.hpp"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <wx/wx.h>
#include <wx/stc/stc.h>

/**
 * @class StatusBar
 * @brief A custom panel typically located at the bottom of the main window,
 * used to display file type, line/column number, tab size, and other status info.
 */
class StatusBar : public wxPanel {
public:
    /**
     * @brief Constructs the StatusBar panel.
     * @param parent The parent window.
     * @param ID The unique ID for the panel.
     */
    StatusBar(wxWindow* parent, wxWindowID ID);

    /**
     * @brief Updates all status components (language, tab size, locale) based on the currently open file.
     * @param path The file path of the currently active document.
     */
    void UpdateComponents(const wxString& path);

    /**
     * @brief Updates the cursor position and locale information (Line, Column).
     * @param codeEditor A pointer to the active wxStyledTextCtrl.
     */
    void UpdateCodeLocale(wxStyledTextCtrl* codeEditor);

    /**
     * @brief Clears the text from all status labels.
     */
    void ClearLabels();

    /**
     * @brief Handles the paint event for the status bar (e.g., drawing separators).
     * @param WXUNUSED(event) The paint event.
     */
    void OnPaint(wxPaintEvent& WXUNUSED(event));

    /**
     * @brief Updates the displayed language/file extension based on language preferences.
     * @param language The language preference structure for the current file.
     */
    void UpdateLanguage(const languagePreferencesStruct& language);

public:
    wxStaticText* codeLocale = nullptr; /**< Displays line and column number (e.g., Ln 10, Col 5). */
    wxStaticText* tabSize = nullptr; /**< Displays tab size and/or indentation type. */
    wxStaticText* fileExt = nullptr; /**< Displays the file extension or programming language name. */

private:
    wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL); /**< Main sizer for the status bar layout. */
    json Theme = ThemesManager::Get().currentTheme; /**< Cached theme settings. */
    json UserSettings = UserSettingsManager::Get().currentSettings; /**< Cached user settings. */
    wxString iconsDir = ApplicationPaths::AssetsPath("icons"); /**< Path to the icons directory. */
    
    wxDECLARE_NO_COPY_CLASS(StatusBar);
};