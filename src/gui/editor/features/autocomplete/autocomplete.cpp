#include "autocomplete.hpp"
#include "gui/editor/controls/textCtrl/textCtrl.hpp"

AutoCompleteController::AutoCompleteController(
	TextCtrl *textCtrl, languagePreferencesStruct languagePreferences)
	: m_textCtrl(textCtrl), m_languagePreferences(languagePreferences) {
	Initialize();
	BindEvents();
}

AutoCompleteController *
AutoCompleteController::Create(TextCtrl *textCtrl,
							   languagePreferencesStruct languagePreferences) {
	if (textCtrl && !languagePreferences.name.empty())
		return new AutoCompleteController(textCtrl, languagePreferences);
	return nullptr;
}

void AutoCompleteController::BindEvents() {
	if (auto *stc = dynamic_cast<wxStyledTextCtrl *>(m_textCtrl.get())) {
		stc->Bind(wxEVT_STC_AUTOCOMP_CANCELLED,
				  &AutoCompleteController::OnAutoCompCancelled, this);
		stc->Bind(wxEVT_STC_AUTOCOMP_SELECTION,
				  &AutoCompleteController::OnAutoCompSelection, this);
	}
}

void AutoCompleteController::Initialize() {
	if (!m_textCtrl) {
		wxMessageBox(_("There is erro while configurate autocomplete."));
		return;
	}

	if (!m_languagePreferences.name.empty()) {
		SetupConfigs();

		m_standardWordsList =
			LanguagesPreferences::Get().GetAutoCompleteWordsList(
				m_languagePreferences);
	}
}

void AutoCompleteController::SetupConfigs() {
	m_textCtrl->AutoCompSetSeparator(' ');
	m_textCtrl->AutoCompSetIgnoreCase(false);
	m_textCtrl->AutoCompSetAutoHide(true);
	m_textCtrl->AutoCompSetDropRestOfWord(false);
	m_textCtrl->AutoCompSetMaxHeight(8);
	m_textCtrl->AutoCompSetTypeSeparator('?');
	m_textCtrl->AutoCompSetChooseSingle(false);
	m_textCtrl->AutoCompSetOrder(wxSTC_ORDER_PRESORTED);
	m_textCtrl->AutoCompSetCancelAtStart(true);
	m_textCtrl->AutoCompSetFillUps(" (.;->");
}

wxString AutoCompleteController::RequestStandardWordsList(wxString word) {
	wxString list;
	std::vector<wxString> matches;

	for (wxString kw : m_standardWordsList) {
		if (kw.StartsWith(word)) {
			matches.push_back(kw);
		}
	}

	std::sort(matches.begin(), matches.end());

	for (size_t i = 0; i < matches.size() && i < 30; ++i) {
		if (i > 0)
			list += " ";
		list += matches[i];
	}

	return list;
}

void AutoCompleteController::ShowPopUp(wxString list, int len) {
	if (!list.empty()) {
		m_textCtrl->AutoCompShow(len, list);
	} else {
		m_textCtrl->AutoCompCancel();
	}
}

wxString AutoCompleteController::ParseCompletionItems(const std::string &json,
													  const wxString &prefix) {
	if (json.empty() || json == "{}")
		return wxEmptyString;

	try {
		auto j = nlohmann::json::parse(json);
		if (!j.contains("result"))
			return wxEmptyString;

		const auto &result = j["result"];
		nlohmann::json items;

		if (result.is_array()) {
			items = result;
		} else if (result.is_object() && result.contains("items")) {
			items = result["items"];
		} else {
			return wxEmptyString;
		}

		if (!items.is_array() || items.empty())
			return wxEmptyString;

		wxString list;
		std::unordered_set<std::string> seen;
		int count = 0;

		for (const auto &item : items) {
			if (count >= 50)
				break;

			wxString label;

			if (item.contains("label") && item["label"].is_string()) {
				label = wxString::FromUTF8(item["label"].get<std::string>());
			} else if (item.contains("insertText") &&
					   item["insertText"].is_string()) {
				label =
					wxString::FromUTF8(item["insertText"].get<std::string>());
			} else if (item.contains("filterText") &&
					   item["filterText"].is_string()) {
				label =
					wxString::FromUTF8(item["filterText"].get<std::string>());
			} else {
				continue;
			}

			if (label.empty() || label.Length() > 100)
				continue;

			label.Trim(true);
			label.Trim(false);

			wxString clean = label;
			clean.Replace(" ", "_");
			clean.Replace("\t", "_");
			clean.Replace("\n", "_");

			if (clean.empty())
				continue;

			if (!prefix.empty()) {
				wxString prefixLower = prefix.Lower();
				wxString cleanLower = clean.Lower();
				if (!cleanLower.StartsWith(prefixLower)) {
					wxString noUnderscore = cleanLower;
					while (!noUnderscore.empty() && noUnderscore[0] == '_') {
						noUnderscore = noUnderscore.Mid(1);
					}
					if (!noUnderscore.StartsWith(prefixLower)) {
						continue;
					}
				}
			}

			std::string key = clean.ToStdString();
			if (seen.find(key) != seen.end())
				continue;
			seen.insert(key);

			if (!list.empty())
				list += " ";
			list += clean;
			count++;
		}

		return list;

	} catch (const std::exception &e) {
		return wxEmptyString;
	}
}

void AutoCompleteController::OnAutoCompCancelled(wxStyledTextEvent &event) {
	m_textCtrl->RecreateMinimap();
	event.Skip();
}

void AutoCompleteController::OnAutoCompSelection(wxStyledTextEvent &event) {
	m_textCtrl->RecreateMinimap();
	event.Skip();
}