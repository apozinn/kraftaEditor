#include "emptyWindow.hpp"
#include "gui/widgets/imagePanel/imagePanel.hpp"

EmptyWindow::EmptyWindow(wxWindow* parent, wxWindowID ID) : wxPanel(parent, ID)
{
	auto background_color = Theme["secondary"].template get<std::string>();
	SetBackgroundColour(wxColor(background_color));
	sizer = new wxBoxSizer(wxVERTICAL);

	wxImagePanel *logo = new wxImagePanel(this, assetsDir + "emptyWindowLogo.png", wxBITMAP_TYPE_PNG);
	sizer->AddStretchSpacer();
	sizer->Add(logo, 0, wxALL | wxALIGN_CENTER, 10);
	sizer->AddStretchSpacer();
	this->SetSizerAndFit(sizer);
}