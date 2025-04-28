class Find : public wxPanel {
	wxStyledTextCtrl* input;
	std::vector<std::vector<int>> indicators;
	wxStyledTextCtrl* codeEditor = NULL;
public:
	Find(wxWindow* parent, wxString defaultInput) : wxPanel(parent, ID_FIND_CONTAINER, wxPoint(parent->GetSize().GetWidth() - 310, 50), wxSize(300, 51)) {
		SetMinSize(wxSize(300, 51));
		SetSize(wxSize(300, 51));
		SetBackgroundColour(wxColor(UserTheme["main"].template get<std::string>()));

		wxBoxSizer* find_sizer = new wxBoxSizer(wxVERTICAL);

		input = new wxStyledTextCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
		input->SetSize(wxSize(200, 25));
		input->SetPosition(wxPoint(10, 13));
		input->SetText(defaultInput);

		input->SetWrapMode(wxSTC_WRAP_NONE);

		input->StyleSetBackground(wxSTC_STYLE_DEFAULT, wxColor(36, 36, 36));
		input->StyleSetForeground(wxSTC_STYLE_DEFAULT, wxColor(255, 255, 255));
		input->StyleClearAll();

		input->SetMarginWidth(0, 0);
		input->SetMarginWidth(1, 0);
		input->SetUseVerticalScrollBar(false);
		input->SetUseHorizontalScrollBar(false);

		input->SetCaretForeground(wxColour(wxT("WHITE")));

		input->SetFocus();
		input->GotoPos(input->GetLength());

		input->SetSelectionStart(0);
		input->SetSelectionEnd(input->GetLength());
		input->SetHotspotSingleLine(true);

		input->IndicatorFillRange(0, 2);

		SetSizer(find_sizer);

		wxAcceleratorEntry entries[1];
		entries[0].Set(wxACCEL_NORMAL, WXK_ESCAPE, ID_EXIT_FIND_CONTAINER);
		wxAcceleratorTable accel(1, entries);
		SetAcceleratorTable(accel);

		input->Bind(wxEVT_STC_CHARADDED, &Find::Search, this);
	}

	void Close(wxCommandEvent& event) {
		for (int i = 0; i < indicators.size(); i++) {
			std::vector<int> current_indicator = indicators[i];
			if (!current_indicator.empty()) {
				codeEditor->IndicatorClearRange(current_indicator[0], current_indicator[1]);
			}
		}

		Destroy();
	}

	void Search(wxStyledTextEvent& event) {
		char chr = (char)event.GetKey();
		if (chr == '\n') {
			input->Remove(input->GetLength() - 1, input->GetLength());
			auto codeContainer = ((CodeContainer*)FindWindowByName(current_openned_path + "_codeContainer"));
			if (codeContainer) {
				int endTarget = 0;
				// codeEditor = codeContainer->editor;
				int startTarget = codeEditor->FindText(0, codeEditor->GetLength(), input->GetText(), 0, &endTarget);

				indicators.push_back({ startTarget, endTarget });

				if (startTarget > 0) {
					codeEditor->IndicatorSetStyle(0, wxSTC_INDIC_FULLBOX);
					codeEditor->IndicatorFillRange(startTarget, input->GetLength());
					codeEditor->GotoPos(startTarget);
				}

				codeEditor->SearchAnchor();
				int pos = codeEditor->SearchNext(0, input->GetText());
				codeEditor->SearchAnchor();

				int ptb = 0;

				while (!ptb) {
					startTarget = codeEditor->FindText(endTarget, codeEditor->GetLength(), input->GetText(), 0, &endTarget);
					if (startTarget && startTarget != endTarget) {
						if (startTarget > 0) {
							codeEditor->IndicatorSetStyle(0, wxSTC_INDIC_FULLBOX);
							codeEditor->IndicatorFillRange(startTarget, input->GetLength());
							codeEditor->GotoPos(startTarget);
						}
						indicators.push_back({ startTarget, endTarget });
					}
					else {
						ptb++;
					}
				}

				codeEditor->IndicatorSetForeground(0, wxColor(255, 0, 0));
			}
		}
	}

	wxDECLARE_NO_COPY_CLASS(Find);
	wxDECLARE_EVENT_TABLE();
};

wxBEGIN_EVENT_TABLE(Find, wxPanel)
EVT_MENU(ID_EXIT_FIND_CONTAINER, Find::Close)
wxEND_EVENT_TABLE()