#pragma once

#include "../frames/mainFrame.hpp"
#include "appPaths/appPaths.hpp"

class KraftaEditor : public wxApp
{
public:
	virtual bool OnInit() override;
	void VerifyIfSytemIsDarkMode();
	void SetupThemeManager();
	void SetupUserSettings();
	void SetupLanguagePreferences();
	bool SetupApplicationDirectories();
	bool CreateMainWindow();
	virtual void OnEventLoopEnter(wxEventLoopBase *WXUNUSED(loop)) override;
	virtual void OnInitCmdLine(wxCmdLineParser &parser) override;
	virtual bool OnCmdLineParsed(wxCmdLineParser &parser) override;
private:
	MainFrame *frame;
	wxString m_dirToWatch;
	const wxString appPath = ApplicationPaths::ApplicationPath();
};

wxIMPLEMENT_APP(KraftaEditor);
wxDECLARE_APP(KraftaEditor);