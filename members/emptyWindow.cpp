#pragma once

class EmptyWindow : public wxPanel {
	wxBoxSizer* sizer;
	json Themes = UserConfig().GetThemes();
public:
	EmptyWindow(wxWindow* parent, wxWindowID ID) : wxPanel(parent, ID) {
    	auto background_color = Themes["dark"]["secondary"].template get<std::string>();
		SetBackgroundColour(wxColor(background_color));
		sizer = new wxBoxSizer(wxVERTICAL);
		wxString logo_dir;

		if(wxFile::Exists("./assets/logo_emptyWindow.png")) logo_dir = "./assets/logo_emptyWindow.png";
	    else if(wxFile::Exists("../assets/logo_emptyWindow.png")) logo_dir = "../assets/logo_emptyWindow.png";

		wxImagePanel* logo = new wxImagePanel(this, logo_dir, wxBITMAP_TYPE_PNG, 200);
		sizer->AddStretchSpacer();
		sizer->Add(logo, 0, wxALL | wxALIGN_CENTER, 10);
		sizer->AddStretchSpacer();

		this->SetSizerAndFit(sizer);

		Bind(wxEVT_PAINT, &EmptyWindow::OnPaint, this);
	}
private:
	void OnPaint(wxPaintEvent& event) {
		auto target = ((wxWindow*)event.GetEventObject());
    	auto border_color = Themes["dark"]["borderColor"].template get<std::string>();
	    if(target) {
	        wxClientDC dc(target);
	        if(!dc.IsOk()) return;
	        dc.SetPen(wxPen(wxColor(border_color), 0.20));
	        dc.DrawLine(0 , 0, 0, target->GetSize().GetHeight());
	    }
	}
};