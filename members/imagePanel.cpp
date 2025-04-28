#pragma once

class wxImagePanel : public wxPanel {
	wxBitmap image;
	double i_rotate;
public:
	wxImagePanel(
		wxWindow* parent, wxString file, wxBitmapType format, int size = 0, double rotate = 0.0
	) : wxPanel(parent, wxID_ANY, wxDefaultPosition, size ? wxSize(size, size) : wxDefaultSize) {
		image.LoadFile(file, format);
		i_rotate = rotate;
		SetMinSize(image.ConvertToImage().GetSize());
	}
	void paintEvent(wxPaintEvent& evt);
	void paintNow();
	void render(wxDC& dc);
	void RescaleToBestSize(int min, int max);
	DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(wxImagePanel, wxPanel)
EVT_PAINT(wxImagePanel::paintEvent)
END_EVENT_TABLE()

void wxImagePanel::paintEvent(wxPaintEvent& evt)
{
	wxPaintDC dc(this);
	render(dc);
}

void wxImagePanel::paintNow()
{
	wxClientDC dc(this);
	render(dc);
}

void wxImagePanel::render(wxDC& dc)
{
	if (i_rotate) {
		if (i_rotate == 90.0) {
			dc.DrawBitmap(wxBitmap(image.ConvertToImage().Rotate90(1), -1), 0, 0, false);
		}
		else if (i_rotate == 180.0) {
			dc.DrawBitmap(wxBitmap(image.ConvertToImage().Rotate180(), -1), 0, 0, false);
		}
		else {
			dc.DrawBitmap(wxBitmap(image.ConvertToImage().Rotate(i_rotate, wxPoint(0, 0)), -1), 0, 0, false);
		}
	}
	else {
		dc.DrawBitmap(image, 0, 0, false);
	}
}

void wxImagePanel::RescaleToBestSize(int min, int max) {
	wxImage img = image.ConvertToImage();
	if (img.IsOk()) {
		if (img.GetWidth() < min || img.GetHeight() < min) {
			image = wxBitmap(img.Rescale(img.GetWidth() * 2, img.GetHeight() * 2));
			this->SetMinSize(wxSize(img.GetWidth() / 2, img.GetHeight() / 2));
			this->SetSize(img.GetWidth() / 2, img.GetHeight() / 2);
		}

		if (img.GetWidth() > max || img.GetHeight() > max) {
			image = wxBitmap(img.Rescale(img.GetWidth() / 2, img.GetHeight() / 2));
			this->SetMinSize(wxSize(img.GetWidth() / 2, img.GetHeight() / 2));
			this->SetSize(img.GetWidth() / 2, img.GetHeight() / 2);
		}

		Update();
		Refresh();
	}
}