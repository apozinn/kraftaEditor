#pragma once

#include "themesManager/themesManager.hpp"
#include "terminalView.hpp"

#include <wx/aui/auibook.h>
#include <wx/aui/tabart.h>
#include <wx/wx.h>

class Terminal : public wxPanel {
  public:
	Terminal(wxWindow *parent, wxWindowID ID);
	~Terminal() override;

	TerminalView *NewSession(const wxString &cwd = wxEmptyString,
							 const wxString &shell = wxEmptyString);
	void CloseSession(int index = -1); 
	TerminalView *Active() const;
	int SessionCount() const;
	
	void RunCommand(const wxString &cmd); 
	void FocusActive();
	void ClearActive();
	void ApplyTheme();
	void SetDefaultConfig(const TerminalView::Config &cfg) { m_defaultCfg = cfg; }

  private:
	void OnPageClose(wxAuiNotebookEvent &e);
	void OnPageChanged(wxAuiNotebookEvent &e);
	void OnViewTitle(wxCommandEvent &e);
	void OnViewExit(wxCommandEvent &e);
	void UpdateTabLabel(TerminalView *view);

	wxAuiNotebook *m_tabs = nullptr;
	wxButton *m_newBtn = nullptr;
	wxPanel *m_bar = nullptr;
	TerminalView::Config m_defaultCfg;
	int m_counter = 0;

	wxDECLARE_NO_COPY_CLASS(Terminal);
};