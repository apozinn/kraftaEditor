#pragma once

class StatusBar : public wxPanel {
	wxBoxSizer* sizer;
	wxStaticText* first_comp;
	wxStaticText* second_comp;
	wxStaticText* third_comp;
public:
	StatusBar(wxWindow* parent, wxWindowID ID);
	void UpdateComps(wxString path, std::string format, const char* language);
	void ClearLabels();
	void UpdateCodeLocale(wxStyledTextCtrl* Code);
private:
	void OnPaint(wxPaintEvent& event);
	void ResizeComps();
};