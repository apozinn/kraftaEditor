class SearchFiles : public wxPanel {
	wxStyledTextCtrl* search_input;
public:
	SearchFiles(wxWindow* parent, wxWindowID ID, wxColor background_color) : wxPanel(parent, ID) {
		SetBackgroundColour(background_color);
		wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);

		wxStaticBitmap* search_icon = new wxStaticBitmap(
			this, wxID_ANY, wxBitmapBundle::FromBitmap(wxBitmap(icons_dir + "search_small_gray.png", wxBITMAP_TYPE_PNG))
		);
		sizer->Add(search_icon, 0, wxALIGN_CENTER | wxALL, 2);

		search_input = new wxStyledTextCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
		search_input->SetSize(wxSize(GetSize().GetWidth(), GetSize().GetHeight()));
		search_input->SetMinSize(wxSize(GetSize().GetWidth(), GetSize().GetHeight()));
		sizer->Add(search_input, 1, wxALIGN_CENTER);

		search_input->SetWrapMode(wxSTC_WRAP_NONE);
		search_input->StyleSetBackground(wxSTC_STYLE_DEFAULT, background_color);
		search_input->StyleSetForeground(wxSTC_STYLE_DEFAULT, wxColor("#7d7d7e"));
		search_input->StyleClearAll();

		search_input->SetZoom(-1);

		search_input->SetMarginWidth(0, 0);
		search_input->SetMarginWidth(1, 0);
		search_input->SetUseVerticalScrollBar(false);
		search_input->SetUseHorizontalScrollBar(false);

		search_input->SetCaretForeground(wxColour(wxT("WHITE")));
		search_input->SetText("Search a file");
		search_input->Bind(wxEVT_LEFT_UP, &SearchFiles::OnClick, this);
		search_input->Bind(wxEVT_KILL_FOCUS, &SearchFiles::OnLostFocus, this);
		SetSizerAndFit(sizer);
	}
private:
	void OnClick(wxMouseEvent& event) {
		search_input->StyleSetForeground(wxSTC_STYLE_DEFAULT, wxColor(*wxWHITE));
		search_input->StyleClearAll();
		search_input->SetText("");
		event.Skip();
	}
	void OnLostFocus(wxEvent& event) {
		search_input->StyleSetForeground(wxSTC_STYLE_DEFAULT, wxColor("#7d7d7e"));
		search_input->StyleClearAll();
		search_input->SetText("Search a file");
		event.Skip();
	}
	void OnPaint(wxPaintEvent& event) {
		auto target = ((wxSplitterWindow*)event.GetEventObject());
		if (!target || !target->IsEnabled()) return;

		wxClientDC this_dc(target);
		auto border_color = UserTheme["main"].template get<std::string>();

		wxPoint search_files_point = target->GetPosition();

		this_dc.SetBrush(wxColor(border_color));
		this_dc.SetPen(wxColor(border_color));

		this_dc.DrawLine(
			0,
			search_files_point.y,
			target->GetSize().GetWidth() + target->GetSashSize(),
			search_files_point.y);

		this_dc.DrawLine(
			0,
			search_files_point.y + target->GetSize().GetHeight(),
			target->GetSize().GetWidth() + target->GetSashSize(),
			search_files_point.y + target->GetSize().GetHeight());
	}
};