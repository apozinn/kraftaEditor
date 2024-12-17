#pragma once
#include "../defs.hpp"
#include <wx/utils.h> 

class Terminal : public wxPanel {
	wxString cmd;
	int startCommand = 29;
	wxStyledTextCtrl* shell;
	wxBoxSizer* sizer;
	wxString shell_title;
public:
	Terminal(wxWindow* parent, wxWindowID ID) : wxPanel(parent, ID) {
		SetBackgroundColour(wxColor(21, 21, 21));
		sizer = new wxBoxSizer(wxVERTICAL);

		auto separator = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxSize(this->GetSize().GetWidth(), 1), wxLI_HORIZONTAL);
		separator->SetBackgroundColour(wxColor(70, 70, 70));
		sizer->Add(separator, 0, wxEXPAND);

		shell = new wxStyledTextCtrl(this);
		shell->SetBackgroundColour(wxColor(21, 21, 21));

		wxFont font(10, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
		//font.SetFaceName(wxT("Monospace"));

		shell->StyleSetBackground(wxSTC_STYLE_DEFAULT, wxColor(21, 21, 21));
		shell->StyleSetForeground(wxSTC_STYLE_DEFAULT, wxColor(221, 221, 221));
		shell->StyleSetFont(wxSTC_STYLE_DEFAULT, font);
		shell->StyleClearAll();

		shell->SetMarginWidth(0, 0);
		shell->SetMarginWidth(1, 0);
		shell->SetMarginWidth(2, 0);
		shell->SetCaretForeground(wxColour(wxT("WHITE")));
		shell->SetUseHorizontalScrollBar(false);
		shell->SetCaretWidth(500);

		wxConfig* config = new wxConfig("krafta-editor");
		wxString str;

		if (config->Read("workspace", &str)) {
			wxString last_workspace = config->Read("workspace", str);
			shell_title = "[" + wxGetUserName() + "@" + wxGetHostName() + " " + wxFileNameFromPath(last_workspace.substr(0, last_workspace.size() - 1)) + "]$ ";
		}
		else {
			shell_title = "[" + wxGetUserName() + "@" + wxGetHostName() + " " + "c" + "]$ ";
		}

		shell->AppendText(shell_title);
		shell->Bind(wxEVT_KEY_UP, &Terminal::OnShellKeyUp, this);
		shell->SetFocus();

		sizer->Add(shell, 1, wxEXPAND | wxTOP, 5);
		SetSizerAndFit(sizer);

		wxAcceleratorEntry entries[1];
		entries[0].Set(wxACCEL_NORMAL, WXK_ESCAPE, ID_EXIT_TERMINAL);
		wxAcceleratorTable accel(1, entries);
		SetAcceleratorTable(accel);
	}
	void Close(wxCommandEvent& event) {
		this->Hide();
		auto parent = ((wxSplitterWindow*)this->GetParent());
		if (parent) {
			parent->Unsplit();
		}
	}
private:
	void Paint(wxPaintEvent& event) {
		wxPaintDC this_dc(this);
		this_dc.SetBrush(wxColour(70, 70, 70));
		this_dc.SetPen(wxColour(70, 70, 70));
	}

	void OnShellKeyUp(wxKeyEvent& event) {
		switch (event.GetKeyCode()) {
		case 13: {
			OnEnterCommand();
		} break;
		case 8: {
			char previous_char = (char)shell->GetCharAt(shell->GetCurrentPos() - 1);
			if (previous_char == '$') {
				shell->AppendText(" ");
				shell->GotoPos(shell->GetCurrentPos() + 1);
			}
		} break;
		}
	}

	void OnEnterCommand() {
		wxArrayString output, errors;
		cmd = shell->GetTextRange(startCommand, shell->GetCurrentPos() - 1);

		if (cmd == "clear") {
			shell->SetText(shell_title);
			shell->GotoPos(shell->GetLength());
			startCommand = shell->GetCurrentPos();
			return;
		}

		int code = wxExecute(cmd, output, errors, wxEXEC_SYNC);
		ShowOutput(output, errors);
	}

	void ShowOutput(
		const wxArrayString& output,
		const wxArrayString& errors
	) {
		size_t ouput_size = output.GetCount();
		size_t erros_size = errors.GetCount();

		if (erros_size) {
			for (size_t n = 0; n < erros_size; n++)
			{
				shell->AppendText(errors[n] + "\n");
			}
		}
		else {
			for (size_t n = 0; n < ouput_size; n++)
			{
				shell->AppendText(output[n] + "\n");
			}
		}

		shell->AppendText(shell_title);
		shell->GotoPos(shell->GetLength());

		startCommand = shell->GetCurrentPos();
		shell->SetFocus();
	}
	wxDECLARE_NO_COPY_CLASS(Terminal);
	wxDECLARE_EVENT_TABLE();
};

wxBEGIN_EVENT_TABLE(Terminal, wxPanel)
EVT_MENU(ID_EXIT_TERMINAL, Terminal::Close)
wxEND_EVENT_TABLE()