class OpenFolderButton : public wxPanel {
public:
	wxBoxSizer* sizer;
	OpenFolderButton(wxWindow* parent) : wxPanel(parent, ID_OPEN_FOLDER_BUTTON) {
		sizer = new wxBoxSizer(wxVERTICAL);
		sizer->AddStretchSpacer(1);

		wxButton* openButton = new wxButton(this, wxID_ANY, "Open folder");
		sizer->Add(openButton, 0, wxALIGN_CENTER);

		sizer->AddStretchSpacer(1);
		SetSizerAndFit(sizer);

		parent->GetSizer()->Add(this, 1, wxEXPAND);
	}
};