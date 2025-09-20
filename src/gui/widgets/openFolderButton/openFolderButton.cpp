#include "openFolderButton.hpp"
#include "app/frames/mainFrame.hpp"
#include <wx/window.h>

OpenFolderButton::OpenFolderButton()
{
	auto projectFilesContainer = wxWindow::FindWindowById(+GUI::ControlID::ProjectFilesContainer);
	if (!projectFilesContainer)
	{
		wxLogError("Project files container not found, cannot create OpenFolderButton");
		return;
	}

	auto openFolderButton = new wxPanel(projectFilesContainer);

	auto openFolderButton_sizer = new wxBoxSizer(wxVERTICAL);
	openFolderButton_sizer->AddStretchSpacer(1);

	wxButton *openButton = new wxButton(openFolderButton, wxID_ANY, "Open folder");
	openFolderButton_sizer->Add(openButton, 0, wxALIGN_CENTER);

	openFolderButton_sizer->AddStretchSpacer(1);
	openFolderButton->SetSizerAndFit(openFolderButton_sizer);

	auto mainFrame = (MainFrame *)wxGetTopLevelParent(projectFilesContainer);

	openFolderButton->Bind(wxEVT_LEFT_UP, &MainFrame::OnOpenFolderClick, mainFrame);
	for (auto &&children : openFolderButton->GetChildren())
		children->Bind(wxEVT_LEFT_UP, &MainFrame::OnOpenFolderClick, mainFrame);

	projectFilesContainer->GetSizer()->Add(openFolderButton, 1, wxEXPAND);
	projectFilesContainer->Layout();
}