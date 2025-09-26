#include "languagesPreferences/languagesPreferences.hpp"
#include "appPaths/appPaths.hpp"
#include "gui/widgets/statusBar/statusBar.hpp"
#include "ui/ids.hpp"
#include "appPaths/appPaths.hpp"
#include "platformInfos/platformInfos.hpp"

#include <unordered_map>
#include <string>
#include <wx/string.h>
#include <wx/file.h>
#include <fstream>
#include <wx/log.h>
#include <functional>
#include <wx/msgdlg.h>
#include <wx/filename.h>
#include <wx/window.h>
#include <wx/settings.h>

#include <nlohmann/json.hpp>
#include <wx/dir.h>
using json = nlohmann::json;

LanguagesPreferences &LanguagesPreferences::Get()
{
    static LanguagesPreferences instance;
    return instance;
}

LanguagesPreferences::LanguagesPreferences()
{
    LoadExtensionsList();
}

void LanguagesPreferences::LoadExtensionsList()
{
    try
    {
        wxString extensionsListPath = ApplicationPaths::DevelopmentEnvironmentPath() + "languages" + PlatformInfos::OsPathSeparator() + "extensionsList.json";
        if (!wxFileExists(extensionsListPath))
            throw EXTENSIONS_LIS_NOT_FOUND;

        std::ifstream extensionsIo(extensionsListPath.ToStdString());
        json extensionsObject = json::parse(extensionsIo);

        for (auto &[key, value] : extensionsObject.items())
            m_extToLang.insert({key, value.template get<std::string>()});
    }
    catch (const std::exception &e)
    {
        wxLogError(e.what());
        throw std::runtime_error(e.what());
    }
}

languagePreferencesStruct LanguagesPreferences::SetupLanguagesPreferences(wxWindow *codeContainer)
{
    try
    {
        wxString path = codeContainer->GetName();
        wxStyledTextCtrl *editor = ((wxStyledTextCtrl *)codeContainer->GetChildren()[0]);
        wxStyledTextCtrl *minimap = ((wxStyledTextCtrl *)codeContainer->GetChildren()[1]);

        languagePreferencesStruct currentLanguagePreferences = GetLanguagePreferences(path);

        editor->SetLexer(currentLanguagePreferences.lexer);
        minimap->SetLexer(currentLanguagePreferences.lexer);

        if (currentLanguagePreferences.preferences.contains("lexer_settings"))
        {
            int tabWidth = currentLanguagePreferences.preferences["lexer_settings"]["tab_width"].template get<int>();
            editor->SetTabWidth(tabWidth);
            editor->SetIndent(tabWidth);
        }

        ApplyLexerStyles(currentLanguagePreferences, editor, minimap);
        SetupReservedWords(currentLanguagePreferences, editor, minimap);
        SetupAutoCompleteWords(currentLanguagePreferences, editor);
        SetupFold(currentLanguagePreferences, editor, minimap);
        UpdateStatusBar(currentLanguagePreferences);

        return currentLanguagePreferences;
    }
    catch (const std::exception &e)
    {
        wxLogError("Unexpected error in GetLanguagePreferences: %s", e.what());
        throw std::runtime_error(std::string("Failed to get language preferences: ") + e.what());
    }
}

languagePreferencesStruct LanguagesPreferences::GetLanguagePreferences(const wxString &path)
{
    try
    {
        wxFileName fileProps(path);
        wxString fileExt = "." + fileProps.GetExt();

        auto GetLanguagesObjects = [this](wxString &languageDir) -> languagePreferencesStruct
        {
            if (!wxDirExists(languageDir))
                languageDir = ApplicationPaths::DevelopmentEnvironmentPath() + "languages" + PlatformInfos::OsPathSeparator() + "default" + PlatformInfos::OsPathSeparator();

            wxDir dir(languageDir);

            if (!dir.IsOpened())
            {
                wxLogError("Não foi possível abrir a pasta: %s", languageDir);
                throw std::runtime_error(ERROR_LANGUAGES_PREFERENCES_FILE_NOT_FOUND);
            }

            wxString languagePreferencesJson = languageDir + "preferences.json";
            wxString languageStyles = languageDir + "styles" + PlatformInfos::OsPathSeparator() + (wxSystemSettings::GetAppearance().IsSystemDark() ? "dark.json" : "light.json");

            std::ifstream preferencesFile(languagePreferencesJson.ToStdString());
            std::ifstream stylesFile(languageStyles.ToStdString());

            if (!preferencesFile || !stylesFile)
                throw std::runtime_error(ERROR_LANGUAGES_PREFERENCES_FILE_NOT_FOUND);

            json languagePreferencesObject = json::parse(preferencesFile);
            json languageStylesObject = json::parse(stylesFile);

            if (languagePreferencesObject.contains("name"))
            {
                languagePreferencesStruct languageNewObjecy = {
                    languagePreferencesObject["name"],
                    languagePreferencesObject["lexer_id"].template get<int>(),
                    languagePreferencesObject,
                    languageStylesObject,
                };

                m_languages.insert({languagePreferencesObject["name"], languageNewObjecy});
                return languageNewObjecy;
            }
            else
                throw std::runtime_error(ERROR_LANGUAGES_PREFERENCES_FILE_NOT_FOUND);
        };

        std::string languageNameLocation = "default";
        auto it = m_extToLang.find(fileExt.ToStdString());
        if (it != m_extToLang.end())
        {
            std::string languageName = it->second;
            auto languageObject = m_languages.find(languageName);

            if (languageObject != m_languages.end())
                return languageObject->second;
            else
                languageNameLocation = it->second;
        }

        wxString defaultLanguage = ApplicationPaths::DevelopmentEnvironmentPath() + "languages" + PlatformInfos::OsPathSeparator() + languageNameLocation + PlatformInfos::OsPathSeparator();
        languagePreferencesStruct languagePrerencesAndStyles = GetLanguagesObjects(defaultLanguage);
        return languagePrerencesAndStyles;
    }
    catch (const json::exception &e)
    {
        wxLogError(ERROR_JSON_PARSE_FAILED, e.what());
        throw std::runtime_error(ERROR_JSON_PARSE_FAILED);
    }
}

void LanguagesPreferences::SetupFold(const languagePreferencesStruct &currentLanguagePreferences, wxStyledTextCtrl *editor, wxStyledTextCtrl *minimap)
{
    if (!currentLanguagePreferences.preferences.contains("fold"))
        return;

    if (currentLanguagePreferences.preferences["fold"]["enabled"].is_boolean())
        if (!currentLanguagePreferences.preferences["fold"]["enabled"])
            return;

    wxString foldSettingsPath = ApplicationPaths::DevelopmentEnvironmentPath() + "config" + PlatformInfos::OsPathSeparator() + "foldSettings" + PlatformInfos::OsPathSeparator() + "foldSettings.json";
    if (!wxFileExists(foldSettingsPath))
    {
        wxLogError("Não foi possível acessar as configurações do fold:");
        return;
    }

    std::ifstream foldSettingsIo(foldSettingsPath.ToStdString());

    if (!foldSettingsIo)
        throw std::runtime_error(ERROR_LANGUAGES_PREFERENCES_FILE_NOT_FOUND);

    json foldSettingsObject = json::parse(foldSettingsIo);

    auto setPreferences = [=](wxStyledTextCtrl *component)
    {
        auto foldProperties = currentLanguagePreferences.preferences["fold"];

        for (const auto &[key, value] : foldProperties.items())
        {
            if (key == "enabled" && value.template get<bool>())
                component->SetProperty("fold", "1");

            if (key == "compact" && value.template get<bool>())
                component->SetProperty("fold.compact", "1");

            if (key == "html" && value.template get<bool>())
                component->SetProperty("fold.html", "1");

            if (key == "preprocessor" && value.template get<bool>())
                component->SetProperty("fold.preprocessor", "1");

            if (key == "comment" && value.template get<bool>())
                component->SetProperty("fold.comment", "1");
        }

        for (const auto &[key, value] : foldSettingsObject["default_fold_flags"].items())
        {
            if (value.template get<std::string>() == "line_before")
                component->SetFoldFlags(wxSTC_FOLDFLAG_LINEBEFORE_CONTRACTED);
            if (value.template get<std::string>() == "line_after")
                component->SetFoldFlags(wxSTC_FOLDFLAG_LINEAFTER_CONTRACTED);
            if (value.template get<std::string>() == "level_numbers")
                component->SetFoldFlags(wxSTC_FOLDFLAG_LEVELNUMBERS);
            if (value.template get<std::string>() == "line_after_expanded")
                component->SetFoldFlags(wxSTC_FOLDFLAG_LINEAFTER_EXPANDED);
            if (value.template get<std::string>() == "line_before_expanded")
                component->SetFoldFlags(wxSTC_FOLDFLAG_LINEBEFORE_EXPANDED);
            if (value.template get<std::string>() == "line_state")
                component->SetFoldFlags(wxSTC_FOLDFLAG_LINESTATE);
        }

        for (const auto &marker : foldSettingsObject["default_markers"])
        {
            int markerId = marker["marker_id"];
            int markerSymbol = marker["symbol_id"];

            component->MarkerDefine(markerId, markerSymbol);
            component->MarkerSetBackgroundSelected(markerId, wxColour(foldSettingsObject["default_selection_background"].template get<std::string>()));

            if (marker.contains("foreground"))
                component->MarkerSetForeground(markerId, wxColour(marker["foreground"].template get<std::string>()));

            if (marker.contains("background"))
                component->MarkerSetBackground(markerId, wxColour(marker["background"].template get<std::string>()));
        }
    };

    editor->SetMarginType(2, wxSTC_MARGIN_SYMBOL);
    editor->SetMarginMask(2, wxSTC_MASK_FOLDERS);
    editor->SetMarginWidth(2, 16);
    editor->SetMarginSensitive(2, true);

    setPreferences(editor);
    setPreferences(minimap);
}

void LanguagesPreferences::SetupReservedWords(const languagePreferencesStruct &currentLanguagePreferences, wxStyledTextCtrl *editor, wxStyledTextCtrl *minimap)
{
    if (currentLanguagePreferences.preferences.contains("syntax"))
    {
        auto syntaxPreferences = currentLanguagePreferences.preferences["syntax"];
        if (!syntaxPreferences.contains("keyword_lists"))
            return;

        auto setupPreferences = [=](wxStyledTextCtrl *component)
        {
            for (auto &[listId, listWords] : syntaxPreferences["keyword_lists"].items())
                component->SetKeyWords(std::stoi(listId), listWords.template get<std::string>());
        };

        setupPreferences(editor);
        setupPreferences(minimap);
    }
}

void LanguagesPreferences::SetupAutoCompleteWords(
    const languagePreferencesStruct &currentLanguagePreferences,
    wxStyledTextCtrl *editor)
{
    if (!currentLanguagePreferences.preferences.contains("syntax"))
        return;

    auto syntaxPreferences = currentLanguagePreferences.preferences["syntax"];
    if (!syntaxPreferences.contains("keyword_lists"))
        return;

    std::vector<wxString> allKeywords;
    for (auto &[listId, listWords] : syntaxPreferences["keyword_lists"].items())
    {
        wxString words = wxString::FromUTF8(listWords.get<std::string>());
        wxArrayString split = wxSplit(words, ' ');
        for (auto &w : split)
            if (!w.IsEmpty())
                allKeywords.push_back(w);
    }

    editor->AutoCompSetIgnoreCase(true);
    editor->AutoCompSetAutoHide(false);
    editor->AutoCompSetChooseSingle(true);
    editor->AutoCompSetFillUps("()[]{}.,:;+-*/%&|^~=<>!?\\\"'");
    editor->AutoCompSetDropRestOfWord(true);
    editor->AutoCompSetSeparator(' ');
    editor->AutoCompSetMaxHeight(10);
    editor->AutoCompSetMaxWidth(400);

    editor->Bind(wxEVT_STC_CHARADDED,
                 [allKeywords, editor](wxStyledTextEvent &event)
                 {
                     char ch = static_cast<char>(event.GetKey());
                     if (std::isalnum(static_cast<unsigned char>(ch)) || ch == '_')
                     {
                         int currentPos = editor->GetCurrentPos();
                         int startPos = editor->WordStartPosition(currentPos, true);
                         int lenEntered = currentPos - startPos;

                         if (lenEntered > 0)
                         {
                             wxString currentWord = editor->GetTextRange(startPos, currentPos);
                             wxString filtered;
                             for (const auto &kw : allKeywords)
                                 if (kw.StartsWith(currentWord))
                                     filtered += kw + " ";

                             if (!filtered.IsEmpty())
                                 editor->AutoCompShow(lenEntered, filtered);
                             else if (editor->AutoCompActive())
                                 editor->AutoCompCancel();
                         }
                     }
                     event.Skip();
                 });
}

void LanguagesPreferences::UpdateStatusBar(const languagePreferencesStruct &currentLanguagePreferences)
{
    StatusBar *statusBar = ((StatusBar *)wxWindow::FindWindowById(+GUI::ControlID::StatusBar));
    if (statusBar)
        statusBar->UpdateLanguage(currentLanguagePreferences);
}

wxString LanguagesPreferences::GetLanguageIconPath(const wxString &path)
{
    auto currentLanguagePreferences = GetLanguagePreferences(path).preferences;

    if (currentLanguagePreferences.contains("icon_file_name"))
        return ApplicationPaths::GetLanguageIcon(wxString(currentLanguagePreferences["icon_file_name"].template get<std::string>()));
    else
        return ApplicationPaths::GetLanguageIcon("unknown");
}

void LanguagesPreferences::ApplyLexerStyles(const languagePreferencesStruct &currentLanguagePreferences, wxStyledTextCtrl *editor, wxStyledTextCtrl *minimap)
{
    try
    {
        auto setStyles = [=](wxStyledTextCtrl *component)
        {
            auto stylesList = currentLanguagePreferences.styles["styles"];
            auto fontStyles = currentLanguagePreferences.styles["font_default"];

            wxFont font = wxFont(wxFontInfo(fontStyles["size"]).FaceName(fontStyles["family"].template get<std::string>()));
            std::map<int, nlohmann::json> sortedStyles;

            for (auto &[key, value] : stylesList.items())
                sortedStyles[std::stoi(key)] = value;

            for (auto &[styleId, styleConfig] : sortedStyles)
            {
                wxColour styleForeground = wxColour(styleConfig["foreground"].get<std::string>());
                component->StyleSetForeground(styleId, styleForeground);
                if (styleConfig.contains("background"))
                {
                    wxColour styleBackground = wxColour(styleConfig["background"].get<std::string>());
                    component->StyleSetBackground(styleId, styleBackground);
                }
                component->StyleSetFont(styleId, font);

                if (styleConfig.contains("bold") && styleConfig["bold"])
                    component->StyleSetBold(styleId, true);
                if (styleConfig.contains("italic") && styleConfig["italic"])
                    component->StyleSetItalic(styleId, true);
                if (styleConfig.contains("underline") && styleConfig["underline"])
                    component->StyleSetUnderline(styleId, true);
            }
        };

        auto Theme = ThemesManager::Get().currentTheme;
        if (Theme.contains("indicator"))
        {
            if (Theme["indicator"].contains("foreground"))
            {
                editor->IndicatorSetStyle(0, wxSTC_INDIC_ROUNDBOX);
                editor->IndicatorSetForeground(0, wxColour(Theme["indicator"]["foreground"].template get<std::string>()));
                editor->IndicatorSetAlpha(0, 100);
            }
        }

        setStyles(editor);
        setStyles(minimap);

        minimap->Update();
    }
    catch (const json::exception &e)
    {
        wxLogError(ERROR_JSON_PARSE_FAILED, e.what());
        throw std::runtime_error(ERROR_JSON_PARSE_FAILED);
    }
}
