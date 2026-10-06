#pragma once

#include <wx/weakref.h>

class TextCtrl;

class AutoCompleteController {
    public:
    static AutoCompleteController* Create(TextCtrl* textCtrl, languagePreferencesStruct languagePreferences);
    void Initialize();
    void SetupConfigs();
    void ShowPopUp(wxString wordsList, int len);
    wxString RequestStandardWordsList(wxString word);
    static wxString ParseCompletionItems(const std::string &json, const wxString &prefix);

    private:
    AutoCompleteController(TextCtrl* textCtrl, languagePreferencesStruct languagePreferences);
    
    void BindEvents();
    void OnAutoCompCancelled(wxStyledTextEvent &event);
    void OnAutoCompSelection(wxStyledTextEvent &event);

    wxWeakRef<TextCtrl> m_textCtrl;
    std::vector<wxString> m_standardWordsList;
    languagePreferencesStruct m_languagePreferences;
};