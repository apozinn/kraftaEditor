#include "./settings.hpp"

SettingsPage::SettingsPage(
	wxWindow* parent, wxWindowID ID
) : wxPanel(parent, ID) {
	this->SetBackgroundColour(wxColor(55, 55, 55));
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	this->SetSizerAndFit(sizer);
}