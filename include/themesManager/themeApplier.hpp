#pragma once

#include "gui/editor/controls/textCtrl/textCtrl.hpp"
#include "languagesPreferences/languagesPreferences.hpp"
#include <nlohmann/json.hpp>
#include <wx/panel.h>
#include <wx/scrolwin.h>
#include <wx/stattext.h>
#include <wx/stc/stc.h>
#include <wx/textctrl.h>
#include <wx/wx.h>

namespace ThemeApplier {

inline wxColour GetColor(const nlohmann::json &theme, const char *key,
						 const wxColour &fallback) {
	if (!theme.contains(key) || !theme[key].is_string())
		return fallback;
	try {
		return wxColour(wxString::FromUTF8(theme[key].get<std::string>()));
	} catch (...) {
		return fallback;
	}
}

inline void ApplyToAll(wxWindow *window, const wxColour &bg, const wxColour &fg,
					   const wxColour &secondary) {
	if (!window)
		return;

	window->SetBackgroundColour(bg);
	window->SetForegroundColour(fg);

	if (auto *stc = dynamic_cast<TextCtrl *>(window)) {
		stc->StyleSetBackground(wxSTC_STYLE_DEFAULT, bg);
		stc->StyleSetForeground(wxSTC_STYLE_DEFAULT, fg);
		stc->StyleClearAll();

		auto m_languagePreferences =
			LanguagesPreferences::Get().SetupLanguagesPreferences(
				stc->GetParent());

		if (!m_languagePreferences.name.empty()) {
			stc->SetLanguagePreferences(m_languagePreferences);
			stc->RecreateMinimap();
		}
	}

	if (auto *text = dynamic_cast<wxStaticText *>(window)) {
		text->SetForegroundColour(fg);
		text->SetBackgroundColour(bg);
	}

	if (auto *textCtrl = dynamic_cast<wxTextCtrl *>(window)) {
		textCtrl->SetBackgroundColour(bg);
		textCtrl->SetForegroundColour(fg);
	}

	for (auto *child : window->GetChildren()) {
		ApplyToAll(child, bg, fg, secondary);
	}

	window->Refresh();
	window->Update();
}

inline void ApplyTheme(wxWindow *root, const nlohmann::json &theme) {
	const wxColour bg = GetColor(theme, "main", wxColour(30, 30, 30));
	const wxColour fg = GetColor(theme, "text", *wxWHITE);
	const wxColour secondary =
		GetColor(theme, "secondaryText", wxColour(80, 80, 80));

	ApplyToAll(root, bg, fg, secondary);
}

} // namespace ThemeApplier