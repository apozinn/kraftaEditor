#pragma once

#include <wx/wx.h>

class wxImagePanel : public wxPanel {
	wxBitmap image;
	double i_rotate;
public:
	wxImagePanel(
		wxWindow* parent, wxString file, wxBitmapType format, int size = 0, double rotate = 0.0
	);
	void paintEvent(wxPaintEvent& event);
	void paintNow();
	void render(wxDC& dc);
	void RescaleToBestSize(int min, int max);
	DECLARE_EVENT_TABLE()
};