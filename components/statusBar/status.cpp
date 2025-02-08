#include "status.hpp"

StatusBar::StatusBar(wxWindow *parent, wxWindowID ID)
	: wxPanel(parent, ID)
{
	// setting the background color
	auto background_color = UserTheme["secondary"].template get<std::string>();
	SetBackgroundColour(wxColor(background_color));

	sizer->AddStretchSpacer();

	// code locale
	codeLocale = new wxStaticText(this, ID_STTSBAR_CODELOCALE, "");
	sizer->Add(codeLocale, 0, wxALIGN_CENTER | wxRIGHT, 10);

	// tab size
	tabSize = new wxStaticText(this, ID_STTSBAR_TAB_SIZE, "");
	sizer->Add(tabSize, 0, wxALIGN_CENTER | wxRIGHT, 10);

	// file extension
	fileExt = new wxStaticText(this, ID_STTSBAR_FILE_EXT, "");
	sizer->Add(fileExt, 0, wxALIGN_CENTER | wxRIGHT, 10);

	SetSizerAndFit(sizer);
	Bind(wxEVT_PAINT, &StatusBar::OnPaint, this);
	SetMinSize(wxSize(GetSize().x, 20));
}

void StatusBar::UpdateComponents(wxString path, wxString format, const char *language)
{
	wxFileName fileProps = wxFileName(path);

	if (format == "image")
	{
		wxImage *image = new wxImage(path);

		codeLocale->SetLabel(std::to_string(image->GetHeight()) + "x" + std::to_string(image->GetWidth()) + " pixels");

		char s[32] = "";
		tabSize->SetLabel(wxString(FormatBytes(fileProps.GetSize().ToULong(), s)));

		if (fileProps.HasExt())
			fileExt->SetLabel(fileProps.GetExt());
		else
			fileExt->SetLabel("Unknown");
	} else {
		fileExt->SetLabel(wxString(language));
	}

	Refresh();
	sizer->Layout();
}

void StatusBar::UpdateCodeLocale(wxStyledTextCtrl *codeEditor)
{
	if (tabSize)
		tabSize->SetLabel("Tab Size: " + std::to_string(codeEditor->GetTabWidth()));

	if (codeLocale)
		codeLocale->SetLabel(
			"Line " + std::to_string(codeEditor->GetCurrentLine() + 1) + ", Column " + std::to_string(codeEditor->GetColumn(codeEditor->GetCurrentPos())));

	Refresh();
	sizer->Layout();
}

void StatusBar::ClearLabels()
{
	codeLocale->SetLabel("");
	tabSize->SetLabel("");
	fileExt->SetLabel("");

	Refresh();
	sizer->Layout();
}

void StatusBar::OnPaint(wxPaintEvent &event)
{
	wxPaintDC dc(this);
	dc.SetBrush(wxColor(UserTheme["border"].template get<std::string>()));
	dc.SetPen(wxPen(wxColor(UserTheme["border"].template get<std::string>()), 0.20));
	dc.DrawLine(0, 0, GetSize().GetWidth(), 0);
}