#include "openFolderButton.hpp"

OpenFolderButton::OpenFolderButton(wxWindow *parent, wxWindowID ID) : wxPanel(parent, ID)
{
	sizer = new wxBoxSizer(wxVERTICAL);
	sizer->AddStretchSpacer(1);

	wxButton *openButton = new wxButton(this, wxID_ANY, "Open folder");
	sizer->Add(openButton, 0, wxALIGN_CENTER);

	sizer->AddStretchSpacer(1);
	SetSizerAndFit(sizer);

	parent->GetSizer()->Add(this, 1, wxEXPAND);
}