#include "statusBar.hpp"
#include "format/format.hpp"
#include "ui/ids.hpp"

#include <wx/file.h>
#include <wx/filename.h>

StatusBar::StatusBar(wxWindow *parent, wxWindowID ID)
	: wxPanel(parent, ID)
{
	// setting the background color
	auto background_color = Theme["secondary"].template get<std::string>();
	SetBackgroundColour(wxColor(background_color));

	sizer->AddStretchSpacer();

	// code locale
	codeLocale = new wxStaticText(this, +GUI::ControlID::StatusBarCodeLocale, "");
	sizer->Add(codeLocale, 0, wxALIGN_CENTER | wxRIGHT, 10);

	// tab size
	tabSize = new wxStaticText(this, +GUI::ControlID::StatusBarTabSize, "");
	sizer->Add(tabSize, 0, wxALIGN_CENTER | wxRIGHT, 10);

	// file extension
	fileExt = new wxStaticText(this, +GUI::ControlID::StatusBarFileExt, "");
	sizer->Add(fileExt, 0, wxALIGN_CENTER | wxRIGHT, 10);

	SetSizerAndFit(sizer);
	Bind(wxEVT_PAINT, &StatusBar::OnPaint, this);
	SetMinSize(wxSize(GetSize().x, 20));
}

void StatusBar::UpdateComponents(const wxString& path)
{
	if(path.empty() || !wxFileExists(path))
	{
		ClearLabels();
		return;
	}

	wxFileName fileProps = wxFileName(path);
	wxImage fileImage = wxImage();

	wxString languageName = LanguagesPreferences::Get().GetLanguagePreferences(path).name;

	if (!languageName.empty()) {
		languageName[0] = wxToupper(languageName[0]);
	}

	if (fileImage.CanRead(path))
	{
		wxImage *image = new wxImage(path);

		codeLocale->SetLabel(std::to_string(image->GetHeight()) + "x" + std::to_string(image->GetWidth()) + " pixels");

		tabSize->SetLabel(wxString(Format::FormatBytes(fileProps.GetSize().ToULong())));

		if (fileProps.HasExt())
			fileExt->SetLabel(fileProps.GetExt());
		else
			fileExt->SetLabel("Unknown");
	} else {
		fileExt->SetLabel(languageName);
	}

	if (!IsShown() && UserSettings["show_status_bar"] == true)
	{
		Show();
		GetParent()->GetSizer()->Layout();
	}

	Refresh();
	sizer->Layout();
}

void StatusBar::UpdateCodeLocale(wxStyledTextCtrl *codeEditor)
{
	if (!IsShown() && UserSettings["show_status_bar"] == true)
	{
		Show();
		GetParent()->GetSizer()->Layout();
	}

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
	Hide();
	GetParent()->GetSizer()->Layout();

	codeLocale->SetLabel("");
	tabSize->SetLabel("");
	fileExt->SetLabel("");

	Refresh();
	sizer->Layout();
}

void StatusBar::OnPaint(wxPaintEvent &WXUNUSED(event))
{
	wxPaintDC dc(this);
	dc.SetBrush(wxColor(Theme["border"].template get<std::string>()));
	dc.SetPen(wxPen(wxColor(Theme["border"].template get<std::string>()), 0.20));
	dc.DrawLine(0, 0, GetSize().GetWidth(), 0);
}

void StatusBar::UpdateLanguage(const languagePreferencesStruct &language)
{
	fileExt->SetLabel(wxString(language.preferences["name"].template get<std::string>()));

	Refresh();
	sizer->Layout();
}