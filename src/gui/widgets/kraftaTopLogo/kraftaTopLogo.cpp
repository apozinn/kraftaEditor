#include "kraftaTopLogo.hpp"

KraftaTopLogo::KraftaTopLogo(wxWindow *parent) : wxPanel(parent) {
	wxSizer *m_sizer = new wxBoxSizer(wxHORIZONTAL);

	const wxBitmap bmp(ApplicationPaths::AssetsPath("images") +
						   "kraftaEditorSmallSize.png",
					   wxBITMAP_TYPE_PNG);

	if (bmp.IsOk()) {
		auto *icon = new wxStaticBitmap(this, wxID_ANY, bmp);
		m_sizer->Add(icon, 0, wxEXPAND | wxALL, 5);
	}

	wxStaticText *kraftaName =
		new wxStaticText(this, wxID_ANY, "KRAFTA EDITOR");
	wxFont styledFont;
	styledFont.Bold();
	styledFont.Larger();

	kraftaName->SetFont(styledFont);

	m_sizer->Add(kraftaName, 0, wxEXPAND | wxALIGN_CENTER);

	SetSizerAndFit(m_sizer);
}