#pragma once

#include "ui/ids.hpp"
#include "gui/widgets/statusBar/statusBar.hpp"

#include <wx/stc/stc.h>
#include <wx/stc/minimap.h>

class TextCtrl;
class MinimapCtrl;

class Editor : public wxPanel {
    public:
    
    Editor(wxWindow *parent, wxString path);
    void LoadPath(wxString path);
    void Save(wxString path);
    private:
    
    bool Initialize();
    void UpdateStatusBarComponents() {if(m_statusBar) m_statusBar->UpdateComponents(m_path);}
    bool VerifyFilePath();
    bool SetupTextCtrl();
    bool SetupMinimapCtrl();
    void SetupLanguagePreferences();

    wxString m_path;
    wxWeakRef<TextCtrl> m_textCtrl;
    wxWeakRef<MinimapCtrl> m_minimapCtrl;
    wxBoxSizer *m_sizer = new wxBoxSizer(wxHORIZONTAL);
    StatusBar* m_statusBar = ((StatusBar *)FindWindowById(+GUI::ControlID::StatusBar));
    json UserSettings = UserSettingsManager::Get().currentSettings;
    languagePreferencesStruct m_languagePreferences;
};