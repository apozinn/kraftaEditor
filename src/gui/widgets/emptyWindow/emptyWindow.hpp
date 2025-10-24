#pragma once

/**
 * @file emptyWindow.hpp
 * @brief Declaration of the EmptyWindow class, displayed when no files are open.
 */

#include <wx/wx.h>
#include <nlohmann/json.hpp>

#include "themesManager/themesManager.hpp"
#include "appPaths/appPaths.hpp"

using json = nlohmann::json;

/**
 * @class EmptyWindow
 * @brief A panel displayed in the main code area when no files are currently open.
 *
 * It usually contains informational content or actions like "Open File" or "Create New File".
 */
class EmptyWindow : public wxPanel
{
public:
    /**
     * @brief Constructs the EmptyWindow panel.
     * @param parent The parent window.
     * @param id The unique ID for the panel.
     */
    EmptyWindow(wxWindow* parent, wxWindowID id);

private:
    /**
     * @brief Configures the graphical layout and content of the empty window.
     */
    void SetupLayout();
    
private:
    wxBoxSizer* sizer { nullptr }; /**< Main sizer for the panel layout. */
    json Theme = ThemesManager::Get().currentTheme; /**< Cached copy of the current theme settings. */
    wxString assetsDir = ApplicationPaths::AssetsPath("images"); /**< Cached path to the application assets directory. */

    wxDECLARE_NO_COPY_CLASS(EmptyWindow);
};