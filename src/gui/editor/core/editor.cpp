#include "editor.hpp"

#include "gui/editor/controls/minimapCtrl/minimapCtrl.hpp"
#include "gui/editor/controls/textCtrl/textCtrl.hpp"

Editor::Editor(wxWindow *parent, wxString path)
	: m_path(path), wxPanel(parent) {
	SetName(m_path);
	SetLabel(m_path + "_editor");

	if (Initialize()) {
		SetSizerAndFit(m_sizer);
	} else {
		wxMessageBox(
			_("An error occurred while initializing the editor component."));
	}
}

void Editor::Save(wxString path) {}

void Editor::LoadPath(wxString path) {}

bool Editor::Initialize() {
	if (!VerifyFilePath()) {
		wxMessageBox(_("Invalid path"));
		return false;
	}

	if (!SetupTextCtrl())
		return false;

	SetupLanguagePreferences();
	SetupMinimapCtrl();

	return true;
}

bool Editor::VerifyFilePath() {
	wxFileName file_props(m_path);
	if (file_props.IsOk() && file_props.FileExists())
		return true;
	else
		return false;
}

bool Editor::SetupTextCtrl() {
	m_textCtrl = new TextCtrl(this);
	if (!m_textCtrl)
		return false;

	m_textCtrl->SetName(m_path);
	m_textCtrl->SetLabel(m_path + "_textCtrl");
	m_textCtrl->LoadFile(m_path);

	m_textCtrl->DoSave(m_path);
	m_textCtrl->SendMsg(4003, 0, -1);

	m_sizer->Add(m_textCtrl, 1, wxEXPAND);

	return true;
}

bool Editor::SetupMinimapCtrl() {
	m_minimapCtrl = new MinimapCtrl(this, m_textCtrl);
	if (!m_minimapCtrl) {
		wxMessageBox(
			_("An error occurred while initializing the minimap component."));
		return false;
	}

	m_minimapCtrl->SetName(m_path);
	m_minimapCtrl->SetLabel(m_path + "_minimapCtrl");

	m_minimapCtrl->SetSize(wxSize(100, m_minimapCtrl->GetSize().y));
	m_minimapCtrl->SetMinSize(wxSize(100, m_minimapCtrl->GetSize().y));
	m_sizer->Add(m_minimapCtrl, 0, wxEXPAND);

	//	m_textCtrl->SetMinimapCtrl(m_minimapCtrl);

	if (!UserSettingsManager::Get()
			 .GetSetting<bool>("editor/showMinimap")
			 .value)
		m_minimapCtrl->Hide();

	return true;
}

void Editor::SetupLanguagePreferences() {
	m_languagePreferences =
		LanguagesPreferences::Get().SetupLanguagesPreferences(this);

	if (!m_languagePreferences.name.empty()) {
		m_textCtrl->SetLanguagePreferences(m_languagePreferences);
	} else {
		wxMessageBox(_("An error occurred while retrieving programming "
					   "language preferences."));
	}
}