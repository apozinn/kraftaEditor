#pragma once

class StatusBar : public wxPanel {
	wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
public:
	wxStaticText* codeLocale = nullptr;
	wxStaticText* tabSize = nullptr;
	wxStaticText* fileExt = nullptr;
	
	StatusBar(wxWindow* parent, wxWindowID ID);
	void UpdateComponents(wxString path, wxString format, const char *language);
	void UpdateCodeLocale(wxStyledTextCtrl* codeEditor);
	void ClearLabels();
	void OnPaint(wxPaintEvent& event);
};