#include "gotoline.hpp"
#include "ui/ids.hpp"
#include <wx/splitter.h>

Gotoline::Gotoline(wxFrame *parent)
	: wxPanel(parent, wxID_ANY,
			  wxPoint(parent->GetSize().GetWidth() / 2 - 225, 50),
			  wxSize(450, 25)) {
	SetBackgroundColour(wxColor(Theme["main"].template get<std::string>()));
	SetFocus();

	wxPanel *topContainer = new wxPanel(this);
	wxBoxSizer *topContainerSizer = new wxBoxSizer(wxHORIZONTAL);

	searchInput = new wxTextCtrl(topContainer, wxID_ANY, "", wxDefaultPosition,
								 wxDefaultSize, wxBORDER_NONE);
	searchInput->SetMaxSize(wxSize(450, 20));
	searchInput->SetBackgroundColour(
		wxColor(Theme["main"].template get<std::string>()));
	searchInput->SetFocus();
	searchInput->Bind(wxEVT_TEXT, &Gotoline::SearchInputModified, this);
	topContainerSizer->Add(searchInput, 1, wxEXPAND | wxTOP, 2);

	topContainer->SetSizerAndFit(topContainerSizer);
	sizer->Add(topContainer, 0, wxEXPAND);

	wxAcceleratorEntry entries[4];
	entries[0].Set(wxACCEL_NORMAL, WXK_ESCAPE, +Event::Gotoline::Exit);
	wxAcceleratorTable accel(4, entries);
	SetAcceleratorTable(accel);

	SetSizerAndFit(sizer);
	SetMinSize(wxSize(450, 25));
	SetSize(wxSize(450, 25));
}

void Gotoline::Close(wxCommandEvent &WXUNUSED(event)) {
	FindWindowById(+GUI::ControlID::StatusBar)->SetFocus();
	Destroy();
}

void Gotoline::SearchInputModified(wxCommandEvent &WXUNUSED(event)) {
	auto currentEditor = ((Editor *)wxFindWindowByLabel(
		ProjectSettings::Get().GetCurrentlyFileOpen() + "_codeEditor"));
	if (currentEditor) {
		currentEditor->GotoLine(wxAtoi(searchInput->GetValue()));
	}
}