#include "./code.hpp"

#include "appConstants/appConstants.hpp"
#include "frameFileDropTarget/frameFileDropTarget.hpp"
#include "languagesPreferences/languagesPreferences.hpp"
#include "platformInfos/platformInfos.hpp"

#include <wx/filename.h>
#include <wx/stc/stc.h>

CodeContainer::CodeContainer(wxWindow *parent, wxString path)
	: wxPanel(parent, wxID_ANY, wxDefaultPosition) {
	sizer = new wxBoxSizer(wxHORIZONTAL);

	editor = new Editor(this);
	sizer->Add(editor, 1, wxEXPAND);

	SetSizerAndFit(sizer);
	LoadPath(path);

	sizer->Layout();
	Layout();

	if (PlatformInfos::IsWindows())
		font = wxFont(wxFontInfo(10).FaceName("Cascadia Code"));
	else
		font = wxFont(wxFontInfo(10).FaceName("Monospace"));

	wxAcceleratorEntry entries[2];
	entries[0].Set(wxACCEL_CTRL, WXK_CONTROL_S, +Event::File::Save);
	entries[0].FromString("Ctrl+S");
	entries[1].Set(wxACCEL_CTRL, WXK_SHIFT, +Event::File::SaveAs);
	entries[1].FromString("Ctrl+Shift+S");
	wxAcceleratorTable accel(2, entries);
	SetAcceleratorTable(accel);
}

void CodeContainer::LoadPath(wxString path) {
	wxFileName file_props(path);
	if (file_props.IsOk() && file_props.FileExists() && editor) {
		SetName(path);
		SetLabel(path + "_codeContainer");
		currentPath = path;

		editor->SetLabel(path + "_codeEditor");
		editor->SetName(path);
		editor->LoadFile(path);

		statusBar->UpdateComponents(path);

		languagePreferences =
			LanguagesPreferences::Get().SetupLanguagesPreferences(this);

		editor->SetAutoCompleteWordsList(
			LanguagesPreferences::Get().GetAutoCompleteWordsList(
				languagePreferences));
		editor->SetLanguagesPreferences(languagePreferences);

		Save(path);
		editor->SendMsg(4003, 0, -1);

		wxStyledTextCtrlMiniMap *minimap =
			new wxStyledTextCtrlMiniMap(this, editor);
		minimap->SetSize(wxSize(100, minimap->GetSize().y));
		minimap->SetMinSize(wxSize(100, minimap->GetSize().y));

		sizer->Add(minimap, 0, wxEXPAND);

		if (!UserSettingsManager::Get()
				 .GetSetting<bool>("editor/showMinimap")
				 .value)
			minimap->Hide();

		minimap->SetLabel(path + "_codeMap");
		minimap->SetName(path);
	} else {
		wxMessageBox(_("There was an error opening the file"), _("Error"),
					 wxICON_ERROR);
	}

	GetParent()->Layout();
	Layout();

	LanguagesPreferences::Get().VerifyLanguageLsp(
		languagePreferences, [this](bool success) {
			if (success) {
				wxTheApp->CallAfter([this]() {
					auto *editor = dynamic_cast<Editor *>(GetChildren()[0]);
					if (editor) {
						editor->Lsp();
					}
				});
			}
		});
}

void CodeContainer::OnSave(wxCommandEvent &WXUNUSED(event)) {
	Save(ProjectSettings::Get().GetCurrentlyFileOpen());
}

bool CodeContainer::Save(wxString path) {
	auto currentEditor = ((Editor *)wxFindWindowByLabel(path + "_codeEditor"));

	if (currentEditor) {
		if (currentEditor->SaveFile(path) && !currentEditor->Modified()) {
			if (auto tab = FindWindowByLabel(path + "_tab")) {
				auto icon =
					((wxStaticBitmap *)tab->GetChildren()[0]->GetChildren()[2]);
				if (icon) {
					icon->SetBitmap(wxBitmapBundle::FromBitmap(wxBitmap(
						ApplicationPaths::AssetsPath("icons") + "close.png",
						wxBITMAP_TYPE_PNG)));
					icon->SetLabel("saved_icon");
					tab->Layout();
				}
			}

			if (path == UserSettingsManager::Get().SettingsPath) {
				UserSettingsManager::Get().LoadSettingsFromFile();
			}

			if (path == ShortCutSettingsManager::Get().ShortcutsPath) {
				ShortCutSettingsManager::Get().LoadSettingsFromFile();
			}

			return true;
		} else {
			wxMessageBox(_("File could not be saved!"), _("Close abort"),
						 wxOK | wxICON_EXCLAMATION);
		}
	} else {
		wxMessageBox(_("File could not be saved!"), _("Close abort"),
					 wxOK | wxICON_EXCLAMATION);
	}
	return false;
}

void CodeContainer::OnSaveAs(wxCommandEvent &WXUNUSED(event)) {
	wxString filename;
	wxFileDialog dlg(
		this, "Save file", wxEmptyString,
		wxFileNameFromPath(ProjectSettings::Get().GetCurrentlyFileOpen()),
		"Any file (*)|*", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	if (dlg.ShowModal() != wxID_OK)
		return;
	filename = dlg.GetPath();
	Save(filename);
}

void CodeContainer::OnSaveAll(wxCommandEvent &WXUNUSED(event)) {
	auto mainCode = FindWindowById(+GUI::ControlID::MainCode);
	if (mainCode) {
		for (auto &&children : mainCode->GetChildren()) {
			if (children->GetLabel().ToStdString().find("_codeContainer") !=
				std::string::npos) {
				Save(children->GetName());
			}
		}
	}
}

void CodeContainer::OnCloseFile(wxCommandEvent &WXUNUSED(event)) {
	auto currentEditor = ((Editor *)wxFindWindowByLabel(
		ProjectSettings::Get().GetCurrentlyFileOpen() + "_codeEditor"));
	if (currentEditor) {
		if (currentEditor->Modified()) {
			if (wxMessageBox(_("Text is not saved, save before closing?"),
							 _("Close"), wxYES_NO | wxICON_QUESTION) == wxYES) {
				currentEditor->SaveFile();
				if (currentEditor->Modified()) {
					wxMessageBox(_("Text could not be saved!"),
								 _("Close abort"), wxOK | wxICON_EXCLAMATION);
					return;
				}
			}
		}

		auto tabsContainer = ((Tabs *)FindWindowById(+GUI::ControlID::Tabs));
		if (tabsContainer) {
			tabsContainer->Close(ProjectSettings::Get().GetCurrentlyFileOpen());
		}
	}
}

void CodeContainer::OnRedo(wxCommandEvent &WXUNUSED(event)) {
	auto currentEditor = ((Editor *)wxFindWindowByLabel(
		ProjectSettings::Get().GetCurrentlyFileOpen() + "_codeEditor"));
	if (currentEditor) {
		if (!currentEditor->CanRedo())
			return;
		currentEditor->Redo();
	}
}

void CodeContainer::OnUndo(wxCommandEvent &WXUNUSED(event)) {
	auto currentEditor = ((Editor *)wxFindWindowByLabel(
		ProjectSettings::Get().GetCurrentlyFileOpen() + "_codeEditor"));
	if (currentEditor) {
		if (!currentEditor->CanUndo())
			return;
		currentEditor->Undo();
	}
}

void CodeContainer::OnCut(wxCommandEvent &WXUNUSED(event)) {
	auto currentEditor = ((Editor *)wxFindWindowByLabel(
		ProjectSettings::Get().GetCurrentlyFileOpen() + "_codeEditor"));
	if (currentEditor) {
		if (currentEditor->GetReadOnly() ||
			(currentEditor->GetSelectionEnd() -
				 currentEditor->GetSelectionStart() <=
			 0))
			return;
		currentEditor->Cut();
	}
}

void CodeContainer::OnCopy(wxCommandEvent &WXUNUSED(event)) {
	auto currentEditor = ((Editor *)wxFindWindowByLabel(
		ProjectSettings::Get().GetCurrentlyFileOpen() + "_codeEditor"));
	if (currentEditor) {
		currentEditor->CopyAllowLine();
	}
}

void CodeContainer::OnPaste(wxCommandEvent &WXUNUSED(event)) {
	auto currentEditor = ((Editor *)wxFindWindowByLabel(
		ProjectSettings::Get().GetCurrentlyFileOpen() + "_codeEditor"));
	if (currentEditor) {
		if (!currentEditor->CanPaste())
			return;
		currentEditor->Paste();
	}
}

void CodeContainer::ToggleCommentLine(wxCommandEvent &event) {
	auto currentEditor = ((Editor *)wxFindWindowByLabel(
		ProjectSettings::Get().GetCurrentlyFileOpen() + "_codeEditor"));
	if (currentEditor) {
		currentEditor->OnToggleLineComment(event);
	}
}

void CodeContainer::ToggleCommentBlock(wxCommandEvent &event) {
	auto currentEditor = ((Editor *)wxFindWindowByLabel(
		ProjectSettings::Get().GetCurrentlyFileOpen() + "_codeEditor"));
	if (currentEditor) {
		currentEditor->OnToggleBlockComment(event);
	}
}

void CodeContainer::OnSelectAll(wxCommandEvent &WXUNUSED(event)) {
	auto currentEditor = ((Editor *)wxFindWindowByLabel(
		ProjectSettings::Get().GetCurrentlyFileOpen() + "_codeEditor"));
	if (currentEditor) {
		currentEditor->SetSelection(0, currentEditor->GetTextLength());
	}
}

void CodeContainer::OnSelectLine(wxCommandEvent &WXUNUSED(event)) {
	auto currentEditor = ((Editor *)wxFindWindowByLabel(
		ProjectSettings::Get().GetCurrentlyFileOpen() + "_codeEditor"));
	if (currentEditor) {
		int lineStart =
			currentEditor->PositionFromLine(currentEditor->GetCurrentLine());
		int lineEnd = currentEditor->PositionFromLine(
			currentEditor->GetCurrentLine() + 1);
		currentEditor->SetSelection(lineStart, lineEnd);
	}
}

void CodeContainer::OnMoveLineUp(wxCommandEvent &WXUNUSED(event)) {
	auto currentEditor = ((Editor *)wxFindWindowByLabel(
		ProjectSettings::Get().GetCurrentlyFileOpen() + "_codeEditor"));
	if (currentEditor) {
		currentEditor->MoveSelectedLinesUp();
	}
}

void CodeContainer::OnMoveLineDown(wxCommandEvent &WXUNUSED(event)) {
	auto currentEditor = ((Editor *)wxFindWindowByLabel(
		ProjectSettings::Get().GetCurrentlyFileOpen() + "_codeEditor"));
	if (currentEditor) {
		currentEditor->MoveSelectedLinesDown();
	}
}

void CodeContainer::OnDuplicateLine(wxCommandEvent &event) {
	auto currentEditor = ((Editor *)wxFindWindowByLabel(
		ProjectSettings::Get().GetCurrentlyFileOpen() + "_codeEditor"));
	if (currentEditor) {
		currentEditor->OnDuplicateLineDown(event);
	}
}

void CodeContainer::OnRemoveCurrentLine(wxCommandEvent &WXUNUSED(event)) {
	auto currentEditor = ((Editor *)wxFindWindowByLabel(
		ProjectSettings::Get().GetCurrentlyFileOpen() + "_codeEditor"));
	if (currentEditor) {
		currentEditor->RemoveCurrentLine();
	}
}

void CodeContainer::ZoomIn(wxCommandEvent &event) {
	auto currentEditor = ((Editor *)wxFindWindowByLabel(
		ProjectSettings::Get().GetCurrentlyFileOpen() + "_codeEditor"));
	if (currentEditor) {
		currentEditor->ZoomIn();
	}
}

void CodeContainer::ZoomOut(wxCommandEvent &event) {
	auto currentEditor = ((Editor *)wxFindWindowByLabel(
		ProjectSettings::Get().GetCurrentlyFileOpen() + "_codeEditor"));
	if (currentEditor) {
		currentEditor->ZoomOut();
	}
}