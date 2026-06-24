#include "splitEditor.hpp"

namespace SplitEditorManager {
void CreateSplitedEditor(const wxString &path) {
	if (path.IsEmpty())
		return;

	ProjectSettings::Get().SetSplitEditorPaths(
		ProjectSettings::Get().GetCurrentlyFileOpen(), path);

	auto CodeContainerBlockLeft =
		wxWindow::FindWindowById(+GUI::ControlID::CodeContainerBlockLeft);
	auto CodeContainerBlockRight =
		wxWindow::FindWindowById(+GUI::ControlID::CodeContainerBlockRight);
	auto tabsContainer =
		((Tabs *)wxWindow::FindWindowById(+GUI::ControlID::Tabs));
	auto statusBar =
		((StatusBar *)wxWindow::FindWindowById(+GUI::ControlID::StatusBar));

	if (!CodeContainerBlockLeft || !CodeContainerBlockRight|| !tabsContainer ||
		!statusBar)
		return;
	if (!wxFileExists(path))
		return;

	auto hideOtherPanelsOfMainCode = [&](wxWindow *except) {
		for (auto &&other : CodeContainerBlockRight->GetChildren()) {
			if (other->GetId() != +GUI::ControlID::Tabs &&
				other->GetId() != +GUI::ControlID::SplitEditorTabs &&
				other != except)
				other->Hide();
		}
		CodeContainerBlockRight->Layout();
	};

	auto splitEditorTabs = new Tabs((wxPanel *)CodeContainerBlockRight,
									+GUI::ControlID::SplitEditorTabs);
	CodeContainerBlockRight->GetSizer()->Add(splitEditorTabs, 0, wxEXPAND);

	splitEditorTabs->Add(wxFileNameFromPath(path), path);

	auto LoadCodeEditor = [&]() {
		auto codeEditor =
			((CodeContainer *)wxFindWindowByLabel(path + "_codeContainer"));

		if (codeEditor)
			tabsContainer->Close(path);

		codeEditor = new CodeContainer(CodeContainerBlockRight, path);

		auto mainCodeParent =
			(wxSplitterWindow *)CodeContainerBlockRight->GetParent();
		if (mainCodeParent)
			mainCodeParent->SplitVertically(CodeContainerBlockLeft,
											CodeContainerBlockRight,
											mainCodeParent->GetSize().x / 2);

		CodeContainerBlockRight->GetSizer()->Add(codeEditor, 1, wxEXPAND);
		CodeContainerBlockRight->GetSizer()->Layout();
		CodeContainerBlockRight->Update();

		hideOtherPanelsOfMainCode(codeEditor);
	};

	wxImage fileImage;
	if (FileOperations::IsImageFile(path)) {
		fileImage.LoadFile(path);
		if (!fileImage.IsOk())
			return;
		if (fileImage.GetWidth() > 1000 || fileImage.GetHeight() > 1000)
			fileImage.Rescale(fileImage.GetWidth() / 2,
							  fileImage.GetHeight() / 2);

		auto imageContainer =
			new wxStaticBitmap(CodeContainerBlockRight, wxID_ANY, fileImage);

		hideOtherPanelsOfMainCode(imageContainer);

		imageContainer->SetLabel(path + "_imageContainer");
		CodeContainerBlockRight->GetSizer()->Add(imageContainer, 1,
												 wxALIGN_CENTER);
		statusBar->UpdateComponents(path);
	} else
		LoadCodeEditor();

	for (auto &children : tabsContainer->GetChildren())
		children->Refresh();

	wxFileName fullPath(path);
	wxString parentPath =
		fullPath.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR);

	ProjectSettings::Get().SetCurrentlyFileOpen(path);
	ProjectSettings::Get().SetCurrentlyMenuDir(parentPath);
	ProjectSettings::Get().SetCurrentlyMenuFile(path);

	if (auto tab = wxFindWindowByLabel(
			ProjectSettings::Get().GetFirstSplittedEditorPath() + "_tab")) {
		auto icon = ((wxStaticBitmap *)tab->GetChildren()[0]->GetChildren()[2]);
		if (icon) {
			icon->SetBitmap(wxBitmapBundle::FromBitmap(
				wxBitmap(ApplicationPaths::AssetsPath("icons") + "close.png",
						 wxBITMAP_TYPE_PNG)));
			icon->SetLabel("saved_icon");
			tab->Layout();
		}
	}

	tabsContainer->GetSizer()->Layout();
	tabsContainer->FitInside();

	return;
}
} // namespace SplitEditorManager