#include "status.hpp"

StatusBar::StatusBar(wxWindow* parent, wxWindowID ID) : wxPanel(parent, ID)
{
	auto background_color = UserTheme["secondary"].template get<std::string>();
	SetBackgroundColour(wxColor(background_color));

	sizer = new wxBoxSizer(wxHORIZONTAL);

	wxPanel* left_ctn = new wxPanel(this);
	wxBoxSizer* left_ctn_sizer = new wxBoxSizer(wxHORIZONTAL);

	wxPanel* error_ctn = new wxPanel(left_ctn);
	wxBoxSizer* error_ctn_sizer = new wxBoxSizer(wxHORIZONTAL);

	wxVector<wxBitmap> bitmaps_e;
	bitmaps_e.push_back(wxBitmap(icons_dir + "cancel.png", wxBITMAP_TYPE_PNG));
	wxStaticBitmap* error_ico = new wxStaticBitmap(error_ctn, wxID_ANY, wxBitmapBundle::FromBitmaps(bitmaps_e));
	error_ctn_sizer->Add(error_ico, 0, wxALIGN_CENTER);

	wxStaticText* error_count = new wxStaticText(error_ctn, wxID_ANY, "0");
	error_count->SetMinSize(wxSize(error_count->GetSize().GetWidth(), 15));
	error_ctn_sizer->Add(error_count, 0, wxALIGN_CENTER | wxLEFT, 3);
	error_ctn->SetSizerAndFit(error_ctn_sizer);

	left_ctn_sizer->Add(error_ctn);
	left_ctn_sizer->SetSizeHints(left_ctn);

	left_ctn->SetSizerAndFit(left_ctn_sizer);
	sizer->Add(left_ctn, 1, wxEXPAND | wxALL, 3);

	wxPanel* right_ctn = new wxPanel(this);
	wxBoxSizer* right_ctn_sizer = new wxBoxSizer(wxHORIZONTAL);

	first_comp = new wxStaticText(right_ctn, ID_STTSBAR_CODELOCALE, "");
	first_comp->SetMinSize(wxSize(110, 15));
	right_ctn_sizer->Add(first_comp, 0, wxALIGN_CENTER | wxRIGHT, 4);

	second_comp = new wxStaticText(right_ctn, ID_STTSBAR_TAB_SIZE, "");
	second_comp->SetMinSize(wxSize(60, 15));
	right_ctn_sizer->Add(second_comp, 0, wxALIGN_CENTER | wxRIGHT, 4);

	third_comp = new wxStaticText(right_ctn, ID_STTSBAR_FILE_EXT, "");
	third_comp->SetMinSize(wxSize(30, 15));
	right_ctn_sizer->Add(third_comp, 0, wxALIGN_CENTER | wxRIGHT, 4);

	right_ctn_sizer->SetSizeHints(right_ctn);
	right_ctn->SetSizerAndFit(right_ctn_sizer);
	sizer->Add(right_ctn, 0, wxEXPAND | wxALL, 3);

	sizer->SetSizeHints(this);
	SetSizerAndFit(sizer);
	Bind(wxEVT_PAINT, &StatusBar::OnPaint, this);
}

void StatusBar::UpdateComps(wxString path, std::string format, const char* language) {
	wxFileName file_props = wxFileName(path);
	if (format == "image") {
		wxBitmap img(path);
		wxImage img_ = img.ConvertToImage();
		first_comp->SetLabel(std::to_string(img_.GetHeight()) + "x" + std::to_string(img_.GetWidth()) + " pixels");
		char s[32] = "";
		second_comp->SetLabel(wxString(FormatBytes(file_props.GetSize().ToULong(), s)));
		if (file_props.HasExt()) {
			third_comp->SetLabel(file_props.GetExt());

		}
		else {
			third_comp->SetLabel("Unk");

		}
	}
	else {
		second_comp->SetLabel("Spaces: 4");
		third_comp->SetLabel(language != " " ? language : "unknown");
	}
}

void StatusBar::ClearLabels() {
	first_comp->SetLabel("");
	second_comp->SetLabel("");
	third_comp->SetLabel("");
}

void StatusBar::UpdateCodeLocale(wxStyledTextCtrl* Code) {
	first_comp->SetLabel(
		"Line " + std::to_string(Code->GetCurrentLine() + 1) + ", Column " + std::to_string(Code->GetColumn(Code->GetCurrentPos()))
	);
}

void StatusBar::OnPaint(wxPaintEvent& event) {
	auto target = ((wxWindow*)event.GetEventObject());
	if (target) {
		wxClientDC dc(target);
		if (dc.IsOk()) {
			dc.SetPen(wxPen(wxColor(UserTheme["borderColor"].template get<std::string>()), 0.20));
			dc.DrawLine(0, 0, target->GetSize().GetWidth(), 0);
			dc.DrawLine(0, target->GetSize().GetHeight(), 0, 0);
		}
	}
}