#include "languagesPreferences/languagesPreferences.hpp"

#include "appPaths/appPaths.hpp"
#include "gui/editor/controls/textCtrl/textCtrl.hpp"
#include "gui/editor/core/editor.hpp"
#include "gui/widgets/statusBar/statusBar.hpp"
#include "platformInfos/platformInfos.hpp"
#include "ui/ids.hpp"

#include <fstream>
#include <functional>
#include <string>
#include <unordered_map>

#include <wx/dir.h>
#include <wx/file.h>
#include <wx/filename.h>
#include <wx/log.h>
#include <wx/msgdlg.h>
#include <wx/settings.h>
#include <wx/string.h>
#include <wx/tokenzr.h>
#include <wx/window.h>

#include <nlohmann/json.hpp>

LanguagesPreferences &LanguagesPreferences::Get() {
	static LanguagesPreferences instance;
	return instance;
}

LanguagesPreferences::LanguagesPreferences() { LoadExtensionsList(); }

void LanguagesPreferences::Reload() {
	m_languages.clear();
	m_extToLang.clear();
	m_preferencesObject = json{};
	m_lexerStylesObject = json{};

	LoadExtensionsList();
}

void LanguagesPreferences::LoadExtensionsList() {
	try {
		wxString extensionsListPath =
			ApplicationPaths::GetLanguagePreferencesPath("") +
			"extensionsList.json";

		if (!wxFileExists(extensionsListPath))
			throw EXTENSIONS_LIS_NOT_FOUND;

		std::ifstream extensionsIo(extensionsListPath.ToStdString());
		json extensionsObject = json::parse(extensionsIo);

		for (auto &[key, value] : extensionsObject.items())
			m_extToLang.insert({key, value.get<std::string>()});
	} catch (const std::exception &e) {
		wxMessageBox(e.what());
		throw std::runtime_error(e.what());
	}
}

bool LanguagesPreferences::VerifyLanguageLsp(
	const languagePreferencesStruct &currentLanguagePreferences,
	std::function<void(bool)> onComplete) {

	if (!currentLanguagePreferences.preferences.contains("lsp")) {
		if (onComplete)
			onComplete(false);
		return false;
	}

	auto &lspConfig = currentLanguagePreferences.preferences["lsp"];
	wxString lspDownloadLink;
	wxString serverName;

	try {
		if (lspConfig.contains("server")) {
			auto &server = lspConfig["server"];

			if (server.contains("name"))
				serverName =
					wxString::FromUTF8(server["name"].get<std::string>());

			if (server.contains("download") &&
				server["download"].contains("urls")) {

				auto &urls = server["download"]["urls"];
				const auto osName = PlatformInfos::OsNameView();

				if (urls.contains(osName) && !urls[osName].is_null())
					lspDownloadLink =
						wxString::FromUTF8(urls[osName].get<std::string>());
			}
		}
	} catch (const std::exception &e) {
		wxLogError("Failed to parse LSP config: %s", e.what());
		if (onComplete)
			onComplete(false);
		return false;
	}

	if (serverName.empty()) {
		wxLogError("No server name in LSP config");
		if (onComplete)
			onComplete(false);
		return false;
	}

	LspManager::Get().VerifyIfLanguageHasLsp(currentLanguagePreferences.name,
											 serverName, lspDownloadLink,
											 onComplete);

	return true;
}

languagePreferencesStruct
LanguagesPreferences::SetupLanguagesPreferences(wxWindow *codeContainer) {
	try {
		wxString path = codeContainer->GetName();

		auto *textCtrl =
			dynamic_cast<wxStyledTextCtrl *>(codeContainer->GetChildren()[0]);

		if (!textCtrl)
			throw std::runtime_error("Editor not found in container");

		languagePreferencesStruct currentLanguagePreferences =
			GetLanguagePreferences(path);

		textCtrl->SetLexer(currentLanguagePreferences.lexer);

		if (currentLanguagePreferences.preferences.contains("lexer_settings")) {
			auto &settings =
				currentLanguagePreferences.preferences["lexer_settings"];

			if (settings.contains("tab_width")) {
				int tabWidth = settings["tab_width"].get<int>();
				textCtrl->SetTabWidth(tabWidth);
				textCtrl->SetIndent(tabWidth);
			}

			if (settings.contains("use_spaces")) {
				bool useSpaces = settings["use_spaces"].get<bool>();
				textCtrl->SetUseTabs(!useSpaces);
			}
		}

		ApplyLexerStyles(currentLanguagePreferences, textCtrl);
		SetupReservedWords(currentLanguagePreferences, textCtrl);
		SetupAutoCompleteWords(currentLanguagePreferences, textCtrl);
		SetupFold(currentLanguagePreferences, textCtrl);
		UpdateStatusBar(currentLanguagePreferences);

		if (currentLanguagePreferences.preferences.contains("lsp")) {
			if (auto *textCtrlPtr = dynamic_cast<TextCtrl *>(textCtrl)) {
				VerifyLanguageLsp(currentLanguagePreferences,
								  [textCtrlPtr](bool success) {
									  if (success) {
										  wxTheApp->CallAfter([textCtrlPtr]() {
											  textCtrlPtr->SetupLsp();
										  });
									  }
								  });
			}
		}

		return currentLanguagePreferences;

	} catch (const std::exception &e) {
		wxMessageBox(wxString::Format(
			_("Failed to get language preferences: %s"), e.what()));

		throw std::runtime_error(
			std::string("Failed to get language preferences: ") + e.what());
	}
}

languagePreferencesStruct
LanguagesPreferences::GetLanguagePreferences(const wxString &path) {
	try {
		wxFileName fileProps(path);
		wxString fileName = wxFileNameFromPath(path);
		wxString fileExt = "." + fileProps.GetExt();

		auto loadLanguageObjects =
			[this](wxString &languageDir) -> languagePreferencesStruct {
			if (!wxDirExists(languageDir))
				languageDir =
					ApplicationPaths::GetLanguagePreferencesPath("default");

			wxDir dir(languageDir);
			if (!dir.IsOpened()) {
				wxLogError("Could not open folder: %s", languageDir);
				throw std::runtime_error(
					ERROR_LANGUAGES_PREFERENCES_FILE_NOT_FOUND);
			}

			wxString languagePreferencesJson = languageDir + "preferences.json";

			wxString languageStyles =
				languageDir + "styles" + PlatformInfos::OsPathSeparator() +
				(ThemesManager::Get().IsDarkTheme() ? "dark.json"
													: "light.json");

			std::ifstream preferencesFile(
				languagePreferencesJson.ToStdString());
			std::ifstream stylesFile(languageStyles.ToStdString());

			if (!preferencesFile || !stylesFile)
				throw std::runtime_error(
					ERROR_LANGUAGES_PREFERENCES_FILE_NOT_FOUND);

			json languagePreferencesObject = json::parse(preferencesFile);
			json languageStylesObject = json::parse(stylesFile);

			if (!languagePreferencesObject.contains("name"))
				throw std::runtime_error(
					ERROR_LANGUAGES_PREFERENCES_FILE_NOT_FOUND);

			languagePreferencesStruct languageObject = {
				languagePreferencesObject["name"],
				languagePreferencesObject["lexer_id"].get<int>(),
				languagePreferencesObject,
				languageStylesObject,
			};

			m_languages.insert(
				{languagePreferencesObject["name"], languageObject});

			return languageObject;
		};

		std::string languageNameLocation = "default";

		auto itExt = m_extToLang.find(fileExt.ToStdString());
		auto itFullName = m_extToLang.find(fileName.ToStdString());

		auto tryResolveCached =
			[this](const std::string &languageName,
				   std::string &locationOut) -> languagePreferencesStruct * {
			auto it = m_languages.find(languageName);
			if (it != m_languages.end())
				return &it->second;
			locationOut = languageName;
			return nullptr;
		};

		if (itFullName != m_extToLang.end()) {
			if (auto *cached =
					tryResolveCached(itFullName->second, languageNameLocation))
				return *cached;
		} else if (itExt != m_extToLang.end()) {
			if (auto *cached =
					tryResolveCached(itExt->second, languageNameLocation))
				return *cached;
		}

		wxString defaultLanguage =
			ApplicationPaths::GetLanguagePreferencesPath(languageNameLocation);

		return loadLanguageObjects(defaultLanguage);

	} catch (const json::exception &e) {
		wxLogError(ERROR_JSON_PARSE_FAILED, e.what());
		throw std::runtime_error(ERROR_JSON_PARSE_FAILED);
	}
}

void LanguagesPreferences::SetupFold(
	const languagePreferencesStruct &currentLanguagePreferences,
	wxStyledTextCtrl *editor) {

	if (!currentLanguagePreferences.preferences.contains("fold"))
		return;

	if (currentLanguagePreferences.preferences["fold"]["enabled"].is_boolean())
		if (!currentLanguagePreferences.preferences["fold"]["enabled"])
			return;

	wxString foldSettingsPath =
		ApplicationPaths::GetConfigPath("foldSettings") + "foldSettings.json";

	if (!wxFileExists(foldSettingsPath)) {
		wxMessageBox(_("Unable to access fold settings:"));
		return;
	}

	std::ifstream foldSettingsIo(foldSettingsPath.ToStdString());
	if (!foldSettingsIo)
		throw std::runtime_error(ERROR_LANGUAGES_PREFERENCES_FILE_NOT_FOUND);

	json foldSettingsObject = json::parse(foldSettingsIo);

	auto applyPreferences = [=](wxStyledTextCtrl *component) {
		auto foldProperties = currentLanguagePreferences.preferences["fold"];

		for (const auto &[key, value] : foldProperties.items()) {
			if (key == "enabled" && value.get<bool>())
				component->SetProperty("fold", "1");
			if (key == "compact" && value.get<bool>())
				component->SetProperty("fold.compact", "1");
			if (key == "html" && value.get<bool>())
				component->SetProperty("fold.html", "1");
			if (key == "preprocessor" && value.get<bool>())
				component->SetProperty("fold.preprocessor", "1");
			if (key == "comment" && value.get<bool>())
				component->SetProperty("fold.comment", "1");
		}

		for (const auto &[key, value] :
			 foldSettingsObject["default_fold_flags"].items()) {
			const std::string flag = value.get<std::string>();

			if (flag == "line_before")
				component->SetFoldFlags(wxSTC_FOLDFLAG_LINEBEFORE_CONTRACTED);
			if (flag == "line_after")
				component->SetFoldFlags(wxSTC_FOLDFLAG_LINEAFTER_CONTRACTED);
			if (flag == "level_numbers")
				component->SetFoldFlags(wxSTC_FOLDFLAG_LEVELNUMBERS);
			if (flag == "line_after_expanded")
				component->SetFoldFlags(wxSTC_FOLDFLAG_LINEAFTER_EXPANDED);
			if (flag == "line_before_expanded")
				component->SetFoldFlags(wxSTC_FOLDFLAG_LINEBEFORE_EXPANDED);
			if (flag == "line_state")
				component->SetFoldFlags(wxSTC_FOLDFLAG_LINESTATE);
		}

		for (const auto &marker : foldSettingsObject["default_markers"]) {
			int markerId = marker["marker_id"];
			int markerSymbol = marker["symbol_id"];

			component->MarkerDefine(markerId, markerSymbol);
			component->MarkerSetBackgroundSelected(
				markerId,
				wxColour(foldSettingsObject["default_selection_background"]
							 .get<std::string>()));

			if (marker.contains("foreground"))
				component->MarkerSetForeground(
					markerId,
					wxColour(marker["foreground"].get<std::string>()));

			if (marker.contains("background"))
				component->MarkerSetBackground(
					markerId,
					wxColour(marker["background"].get<std::string>()));
		}
	};

	editor->SetMarginType(2, wxSTC_MARGIN_SYMBOL);
	editor->SetMarginMask(2, wxSTC_MASK_FOLDERS);
	editor->SetMarginWidth(2, 16);
	editor->SetMarginSensitive(2, true);

	applyPreferences(editor);
}

void LanguagesPreferences::SetupReservedWords(
	const languagePreferencesStruct &currentLanguagePreferences,
	wxStyledTextCtrl *editor) {

	if (!currentLanguagePreferences.preferences.contains("syntax"))
		return;

	auto syntaxPreferences = currentLanguagePreferences.preferences["syntax"];

	if (!syntaxPreferences.contains("keyword_lists"))
		return;

	for (auto &[listId, listWords] : syntaxPreferences["keyword_lists"].items())
		editor->SetKeyWords(std::stoi(listId), listWords.get<std::string>());
}

void LanguagesPreferences::SetupAutoCompleteWords(
	const languagePreferencesStruct &currentLanguagePreferences,
	wxStyledTextCtrl *editor) {

	if (!currentLanguagePreferences.preferences.contains("syntax"))
		return;

	auto syntaxPreferences = currentLanguagePreferences.preferences["syntax"];

	if (!syntaxPreferences.contains("keyword_lists"))
		return;

	editor->Update();
}

std::vector<wxString> LanguagesPreferences::GetAutoCompleteWordsList(
	const languagePreferencesStruct &currentLanguagePreferences) {

	std::vector<wxString> autoCompleteWordsList;

	if (!currentLanguagePreferences.preferences.contains("syntax"))
		return autoCompleteWordsList;

	auto syntaxPreferences = currentLanguagePreferences.preferences["syntax"];

	if (!syntaxPreferences.contains("keyword_lists"))
		return autoCompleteWordsList;

	for (auto &[listId, listWords] :
		 syntaxPreferences["keyword_lists"].items()) {
		for (const auto &word :
			 wxStringTokenize(listWords.get<std::string>(), " ")) {
			autoCompleteWordsList.push_back(wxString(word));
		}
	}

	return autoCompleteWordsList;
}

void LanguagesPreferences::UpdateStatusBar(
	const languagePreferencesStruct &currentLanguagePreferences) {

	auto *statusBar = static_cast<StatusBar *>(
		wxWindow::FindWindowById(+GUI::ControlID::StatusBar));

	if (statusBar)
		statusBar->UpdateLanguage(currentLanguagePreferences);
}

wxString LanguagesPreferences::GetLanguageIconPath(const wxString &path) {
	auto currentLanguagePreferences = GetLanguagePreferences(path).preferences;

	if (currentLanguagePreferences.contains("icon_file_name"))
		return ApplicationPaths::GetLanguageIcon(wxString(
			currentLanguagePreferences["icon_file_name"].get<std::string>()));

	return ApplicationPaths::GetLanguageIcon("unknown");
}

void LanguagesPreferences::ApplyLexerStyles(
	const languagePreferencesStruct &currentLanguagePreferences,
	wxStyledTextCtrl *editor) {

	try {
		auto backgroundColor = ThemesManager::Get().GetColor("secondary");
		auto textColor = ThemesManager::Get().GetColor("text");
		auto secondaryTextColor =
			ThemesManager::Get().GetColor("secondaryText");
		auto caretColor = ThemesManager::Get().GetColor("editorCaret");
		auto secondaryBackgroundColor =
			ThemesManager::Get().GetColor("secondary");

		editor->StyleSetBackground(wxSTC_STYLE_DEFAULT,
								   wxColor(backgroundColor));
		editor->StyleSetForeground(wxSTC_STYLE_DEFAULT, wxColor(textColor));
		editor->StyleClearAll();

		editor->SetCaretForeground(caretColor);

		editor->SetMarginWidth(
			EditorConstants::LINE_NUMBER_MARGIN,
			editor->TextWidth(wxSTC_STYLE_LINENUMBER, wxT("_99999")));
		editor->SetMarginType(EditorConstants::LINE_NUMBER_MARGIN,
							  wxSTC_MARGIN_NUMBER);

		editor->StyleSetForeground(wxSTC_STYLE_LINENUMBER,
								   wxColor(secondaryTextColor));
		editor->StyleSetBackground(wxSTC_STYLE_LINENUMBER,
								   wxColor(backgroundColor));

		editor->StyleSetBackground(wxSTC_STYLE_INDENTGUIDE,
								   wxColor(backgroundColor));
		editor->StyleSetForeground(wxSTC_STYLE_INDENTGUIDE,
								   wxColor(secondaryTextColor));

		editor->SetFoldMarginColour(true, wxColor(secondaryBackgroundColor));
		editor->SetFoldMarginHiColour(true, wxColor(secondaryBackgroundColor));

		auto stylesList = currentLanguagePreferences.styles["styles"];
		auto fontStyles = currentLanguagePreferences.styles["font_default"];

		wxFont font =
			wxFont(wxFontInfo(fontStyles["size"])
					   .FaceName(fontStyles["family"].get<std::string>()));

		std::map<int, nlohmann::json> sortedStyles;
		for (auto &[key, value] : stylesList.items())
			sortedStyles[std::stoi(key)] = value;

		for (auto &[styleId, styleConfig] : sortedStyles) {
			editor->StyleSetForeground(
				styleId,
				wxColour(styleConfig["foreground"].get<std::string>()));

			if (styleConfig.contains("background"))
				editor->StyleSetBackground(
					styleId,
					wxColour(styleConfig["background"].get<std::string>()));

			editor->StyleSetFont(styleId, font);

			if (styleConfig.contains("bold") && styleConfig["bold"])
				editor->StyleSetBold(styleId, true);

			if (styleConfig.contains("italic") && styleConfig["italic"])
				editor->StyleSetItalic(styleId, true);

			if (styleConfig.contains("underline") && styleConfig["underline"])
				editor->StyleSetUnderline(styleId, true);
		}

		auto theme = ThemesManager::Get().currentTheme;

		if (theme.contains("indicator") &&
			theme["indicator"].contains("foreground")) {
			editor->IndicatorSetStyle(0, wxSTC_INDIC_ROUNDBOX);
			editor->IndicatorSetForeground(
				0,
				wxColour(theme["indicator"]["foreground"].get<std::string>()));
			editor->IndicatorSetAlpha(0, 100);
		}

	} catch (const json::exception &e) {
		wxLogError(ERROR_JSON_PARSE_FAILED, e.what());
		throw std::runtime_error(ERROR_JSON_PARSE_FAILED);
	}
}