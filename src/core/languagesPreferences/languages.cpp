#include "languagesPreferences/languagesPreferences.hpp"
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <string>
#include <wx/string.h>
#include <wx/file.h>
#include "fstream"
#include <wx/log.h>

#include "appPaths/appPaths.hpp"
#include <wx/msgdlg.h>

using json = nlohmann::json;

json defaultPreferences{
    {"name", "unknown"}};

LanguagesPreferences &LanguagesPreferences::Get()
{
    static LanguagesPreferences instance;
    return instance;
}

json LanguagesPreferences::GetLanguagePreferences(const std::string &languageName)
{
    try
    {
        auto languageFinded = m_languages.find(languageName);
        if (languageFinded != m_languages.end())
        {
            return languageFinded->second;
        }

        wxLogWarning(ERROR_LANGUAGE_NOT_FOUND, languageName);

        auto defaultLanguageFinded = m_languages.find("defaultPreferences");
        if (defaultLanguageFinded != m_languages.end())
        {
            return defaultLanguageFinded->second;
        }

        wxLogError(ERROR_DEFAULT_NOT_CONFIGURED);
        throw std::runtime_error(ERROR_DEFAULT_NOT_CONFIGURED);
    }
    catch (std::exception e)
    {
        wxLogError("Unexpected error in GetLanguagePreferences: %s", e.what());
        throw std::runtime_error(std::string("Failed to get language preferences: ") + e.what());
    }
}

LanguagesPreferences::LanguagesPreferences()
{
    LoadLanguages();
}

json LanguagesPreferences::LoadLanguages()
{
    m_languages.insert({"defaultPreferences", defaultPreferences});

    wxString languagesPreferencesJsonPath = ApplicationPaths::AssetsPath("languagesPreferences") + "languagesPreferences.json";
    if (!wxFileExists(languagesPreferencesJsonPath))
    {
        wxLogError(ERROR_LANGUAGES_PREFERENCES_FILE_NOT_FOUND, languagesPreferencesJsonPath);
        return m_languages;
    }

    try
    {
        std::ifstream preferencesFile(languagesPreferencesJsonPath.ToStdString());
        if (!preferencesFile)
        {
            throw std::runtime_error(ERROR_LANGUAGES_PREFERENCES_FILE_NOT_FOUND);
        }

        json data = json::parse(preferencesFile);

        for (const auto &[key, value] : data.items())
        {
            if (value.contains("name"))
            {
                m_languages.insert({value["name"], key});
            }
        }
    }
    catch (const json::exception &e)
    {
        wxLogError(ERROR_JSON_PARSE_FAILED, e.what());
        throw std::runtime_error(ERROR_JSON_PARSE_FAILED);
    }

    return m_languages;
}
