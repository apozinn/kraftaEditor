#include "terminal.hpp"

#include <wx/filename.h>
#include <wx/stdpaths.h>

#include "projectSettings/projectSettings.hpp"

Terminal::Terminal(wxWindow *parent, wxWindowID ID) : wxPanel(parent, ID) {
	auto *root = new wxBoxSizer(wxVERTICAL);

	m_bar = new wxPanel(this);
	auto *barSizer = new wxBoxSizer(wxHORIZONTAL);
	barSizer->AddStretchSpacer(1);
	m_newBtn = new wxButton(m_bar, wxID_ANY, "+", wxDefaultPosition, wxDefaultSize,
							wxBU_EXACTFIT | wxBORDER_NONE);
	m_newBtn->SetToolTip(_("New terminal"));
	barSizer->Add(m_newBtn, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, FromDIP(6));
	m_bar->SetSizer(barSizer);

	m_tabs = new wxAuiNotebook(
		this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
		wxAUI_NB_TOP | wxAUI_NB_TAB_MOVE | wxAUI_NB_SCROLL_BUTTONS |
			wxAUI_NB_CLOSE_ON_ALL_TABS | wxAUI_NB_WINDOWLIST_BUTTON | wxBORDER_NONE);

	root->Add(m_bar, 0, wxEXPAND);
	root->Add(m_tabs, 1, wxEXPAND);
	SetSizer(root);

	m_newBtn->Bind(wxEVT_BUTTON, [this](wxCommandEvent &) { NewSession(); });
	m_tabs->Bind(wxEVT_AUINOTEBOOK_PAGE_CLOSE, &Terminal::OnPageClose, this);
	m_tabs->Bind(wxEVT_AUINOTEBOOK_PAGE_CHANGED, &Terminal::OnPageChanged, this);

	ApplyTheme();

	Bind(wxEVT_SHOW, [this](wxShowEvent &e) {
		if (e.IsShown()) {
			if (SessionCount() == 0)
				NewSession();
			else
				FocusActive();
		}
		e.Skip();
	});
}

Terminal::~Terminal() = default;

void Terminal::ApplyTheme() {
	const wxColour bg = ThemesManager::Get().GetColor("main");
	const wxColour fg = ThemesManager::Get().GetColor("text");
    
	SetBackgroundColour(bg);
	m_bar->SetBackgroundColour(bg);
	m_newBtn->SetBackgroundColour(bg);
	m_newBtn->SetForegroundColour(fg);
	m_tabs->SetBackgroundColour(fg);
	m_tabs->SetForegroundColour(fg);
if (auto *art = m_tabs->GetArtProvider()) {
    art->SetColour(fg);
    art->SetActiveColour(fg); 
}
	for (size_t i = 0; i < m_tabs->GetPageCount(); ++i)
		if (auto *v = dynamic_cast<TerminalView *>(m_tabs->GetPage(i)))
			v->ApplyTheme();
	Refresh();
}

TerminalView *Terminal::NewSession(const wxString &cwd, const wxString &shell) {
	TerminalView::Config cfg = m_defaultCfg;
	cfg.cwd = cwd;
	if (cfg.cwd.IsEmpty())
		cfg.cwd = ProjectSettings::Get().GetProjectPath();
	if (cfg.cwd.IsEmpty() || !wxDirExists(cfg.cwd))
		cfg.cwd = wxGetHomeDir();
	if (!shell.IsEmpty())
		cfg.shell = shell;

	auto *view = new TerminalView(m_tabs, wxID_ANY, cfg);
	view->Bind(EVT_TERMVIEW_TITLE, &Terminal::OnViewTitle, this);
	view->Bind(EVT_TERMVIEW_EXIT, &Terminal::OnViewExit, this);

	wxString label = wxFileName::DirName(cfg.cwd).GetDirs().IsEmpty()
						 ? wxString("terminal")
						 : wxFileName::DirName(cfg.cwd).GetDirs().Last();
	m_tabs->AddPage(view, label, true);
	++m_counter;
	view->StartShell();
	view->SetFocus();
	return view;
}

TerminalView *Terminal::Active() const {
	int sel = m_tabs->GetSelection();
	return sel == wxNOT_FOUND ? nullptr
							  : dynamic_cast<TerminalView *>(m_tabs->GetPage((size_t)sel));
}

int Terminal::SessionCount() const { return (int)m_tabs->GetPageCount(); }

void Terminal::CloseSession(int index) {
	if (index < 0)
		index = m_tabs->GetSelection();
	if (index == wxNOT_FOUND || index >= SessionCount())
		return;
	auto *v = dynamic_cast<TerminalView *>(m_tabs->GetPage((size_t)index));
	if (v && v->IsBusy() &&
		wxMessageBox(_("There is a process running in this terminal. Terminate it?"),
					 _("Close terminal"), wxYES_NO | wxICON_QUESTION, this) != wxYES)
		return;
	m_tabs->DeletePage((size_t)index);
}

void Terminal::RunCommand(const wxString &cmd) {
	TerminalView *v = Active();
	if (!v)
		v = NewSession();
	if (v)
		v->RunCommand(cmd);
}

void Terminal::FocusActive() {
	if (auto *v = Active())
		v->SetFocus();
}

void Terminal::ClearActive() {
	if (auto *v = Active())
		v->ClearScreen();
}

void Terminal::OnPageClose(wxAuiNotebookEvent &e) {
	auto *v = dynamic_cast<TerminalView *>(m_tabs->GetPage((size_t)e.GetSelection()));
	if (v && v->IsBusy() &&
		wxMessageBox(_("There is a process running in this terminal. Terminate it?"),
					 _("Close terminal"), wxYES_NO | wxICON_QUESTION, this) != wxYES)
		e.Veto();
	else
		e.Skip();
}

void Terminal::OnPageChanged(wxAuiNotebookEvent &e) {
	e.Skip();
	CallAfter([this] { FocusActive(); });
}

void Terminal::OnViewTitle(wxCommandEvent &e) {
	if (auto *v = dynamic_cast<TerminalView *>(e.GetEventObject()))
		UpdateTabLabel(v);
}

void Terminal::OnViewExit(wxCommandEvent &e) {
	if (auto *v = dynamic_cast<TerminalView *>(e.GetEventObject()))
		UpdateTabLabel(v);
}

void Terminal::UpdateTabLabel(TerminalView *v) {
	int idx = m_tabs->GetPageIndex(v);
	if (idx == wxNOT_FOUND)
		return;
	wxString label = v->GetTitle();
	if (label.IsEmpty() && !v->GetCwd().IsEmpty())
		label = wxFileName::DirName(v->GetCwd()).GetDirs().IsEmpty()
					? wxString("/")
					: wxFileName::DirName(v->GetCwd()).GetDirs().Last();
	if (label.IsEmpty())
		return;
	if (label.length() > 28)
		label = label.Left(27) + wxString::FromUTF8("…");
	if (v->HasExited())
		label += _(" (closed)");
	m_tabs->SetPageText((size_t)idx, label);
}