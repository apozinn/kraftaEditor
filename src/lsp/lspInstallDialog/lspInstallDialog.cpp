#include "lspInstallDialog.hpp"

LspInstallDialog::LspInstallDialog(wxWindow *parent,
								   const wxString &languageName,
								   const wxString &lspName)
	: wxDialog(parent, wxID_ANY, "Language Server Available", wxDefaultPosition,
			   wxSize(420, 300), wxDEFAULT_DIALOG_STYLE | wxSTAY_ON_TOP),
	  m_languageName(languageName), m_lspName(lspName) {
	SetBackgroundColour(ThemesManager::Get().GetColor("main"));

	auto *root = new wxBoxSizer(wxVERTICAL);

	auto *header = new wxBoxSizer(wxHORIZONTAL);
	auto *title = new wxStaticText(this, wxID_ANY, "Install " + lspName + "?");
	title->SetForegroundColour(ThemesManager::Get().GetColor("text"));
	wxFont titleFont = title->GetFont();
	titleFont.SetPointSize(12);
	titleFont.SetWeight(wxFONTWEIGHT_BOLD);
	title->SetFont(titleFont);
	header->Add(title, 1, wxALIGN_CENTER_VERTICAL);
	root->Add(header, 0, wxEXPAND | wxALL, 20);

	wxString msg = wxString::Format(
		"Krafta detected a %s file. Installing %s (50-200 MB) enables:\n\n"
		"  - Autocomplete based on your code\n"
		"  - Inline errors and warnings\n"
		"  - Go to definition\n"
		"  - Rename symbol",
		languageName, lspName);
	auto *message = new wxStaticText(this, wxID_ANY, msg, wxDefaultPosition,
									 wxDefaultSize, wxST_NO_AUTORESIZE);
	message->SetForegroundColour(ThemesManager::Get().GetColor("text"));
	message->Wrap(380);
	root->Add(message, 1, wxEXPAND | wxLEFT | wxRIGHT, 20);

	auto *buttons = new wxBoxSizer(wxHORIZONTAL);
	buttons->AddStretchSpacer();

	auto *skipBtn = new wxButton(this, wxID_ANY, "Not now");
	skipBtn->SetBackgroundColour(ThemesManager::Get().GetColor("colorThree"));
	skipBtn->SetForegroundColour(ThemesManager::Get().GetColor("text"));
	skipBtn->Bind(wxEVT_BUTTON, &LspInstallDialog::OnSkip, this);
	buttons->Add(skipBtn, 0, wxRIGHT, 10);

	auto *installBtn = new wxButton(this, wxID_ANY, "Install");
	installBtn->SetBackgroundColour(ThemesManager::Get().GetColor("tint"));
	installBtn->SetForegroundColour(ThemesManager::Get().GetColor("text"));
	installBtn->Bind(wxEVT_BUTTON, &LspInstallDialog::OnInstall, this);
	buttons->Add(installBtn, 0);

	root->Add(buttons, 0, wxEXPAND | wxALL, 20);

	SetSizer(root);
	Centre();
}

void LspInstallDialog::OnInstall(wxCommandEvent &) { EndModal(wxID_OK); }

void LspInstallDialog::OnSkip(wxCommandEvent &) {
	auto UserSettingsJson = UserSettingsManager::Get().currentSettings;

	if (UserSettingsJson.contains("prompts") &&
		UserSettingsJson["prompts"].contains("dontAskForInstallLspList")) {

		UserSettingsJson["prompts"]["dontAskForInstallLspList"].push_back(
			m_lspName.ToStdString());
		UserSettingsManager::Get().Update(UserSettingsJson);
	}

	EndModal(wxID_CANCEL);
}
