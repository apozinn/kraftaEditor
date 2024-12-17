class NavigationButtons : public wxPanel {
	wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
	wxString mouse_over;
	wxStaticBitmap* files_icon;
	wxStaticBitmap* search_icon;
	wxStaticBitmap* builder_icon;
	wxStaticBitmap* extension_icon;
	wxStaticBitmap* focus_icon;
	wxStaticBitmap* settings_icon;
	json Themes = UserConfig().GetThemes();
public:
	wxStaticBitmap* current_page_icon = NULL;
	wxString current_page_name = "files";
	NavigationButtons(wxWindow* parent, wxColor background_color) : wxPanel(parent, ID_NAVIGATION_BUTTONS) {
		SetBackgroundColour(background_color);

		/*wxPanel* files_container = new wxPanel(this);
		files_container->SetName("files");
		files_container->Bind(wxEVT_ENTER_WINDOW, &NavigationButtons::OnMouseEnter, this);
		files_container->Bind(wxEVT_LEAVE_WINDOW, &NavigationButtons::OnMouseLeave, this);
		files_container->Bind(wxEVT_PAINT, &NavigationButtons::OnPaint, this);
		wxBoxSizer* files_container_sizer = new wxBoxSizer(wxHORIZONTAL);

		files_icon = new wxStaticBitmap(
			files_container, wxID_ANY, wxBitmapBundle::FromBitmap(wxBitmap(icons_dir+"files_blue.png", wxBITMAP_TYPE_PNG))
		);
		files_icon->Bind(wxEVT_LEFT_UP, &NavigationButtons::OnSelect, this);
		files_container_sizer->Add(files_icon, 1, wxEXPAND);

		files_container->SetSizerAndFit(files_container_sizer);
		sizer->Add(files_container, 1);

		wxPanel* search_container = new wxPanel(this);
		search_container->SetName("search");
		search_container->Bind(wxEVT_ENTER_WINDOW, &NavigationButtons::OnMouseEnter, this);
		search_container->Bind(wxEVT_LEAVE_WINDOW, &NavigationButtons::OnMouseLeave, this);
		search_container->Bind(wxEVT_PAINT, &NavigationButtons::OnPaint, this);
		wxBoxSizer* search_container_sizer = new wxBoxSizer(wxHORIZONTAL);

		search_icon = new wxStaticBitmap(
			search_container, wxID_ANY, wxBitmapBundle::FromBitmap(wxBitmap(icons_dir+"search_gray.png", wxBITMAP_TYPE_PNG))
		);
		search_icon->Bind(wxEVT_LEFT_UP, &NavigationButtons::OnSelect, this);
		search_container_sizer->Add(search_icon, 1);

		search_container->SetSizerAndFit(search_container_sizer);
		sizer->Add(search_container, 1);

		wxPanel* builder_container = new wxPanel(this);
		builder_container->SetName("builder");
		builder_container->Bind(wxEVT_ENTER_WINDOW, &NavigationButtons::OnMouseEnter, this);
		builder_container->Bind(wxEVT_LEAVE_WINDOW, &NavigationButtons::OnMouseLeave, this);
		builder_container->Bind(wxEVT_PAINT, &NavigationButtons::OnPaint, this);
		wxBoxSizer* builder_container_sizer = new wxBoxSizer(wxHORIZONTAL);

		builder_icon = new wxStaticBitmap(
			builder_container, wxID_ANY, wxBitmapBundle::FromBitmap(wxBitmap(icons_dir+"builder_gray.png", wxBITMAP_TYPE_PNG))
		);
		builder_icon->Bind(wxEVT_LEFT_UP, &NavigationButtons::OnSelect, this);
		builder_container_sizer->Add(builder_icon, 1);

		builder_container->SetSizerAndFit(builder_container_sizer);
		sizer->Add(builder_container, 1);

		wxPanel* extension_container = new wxPanel(this);
		extension_container->SetName("extension");
		extension_container->Bind(wxEVT_ENTER_WINDOW, &NavigationButtons::OnMouseEnter, this);
		extension_container->Bind(wxEVT_LEAVE_WINDOW, &NavigationButtons::OnMouseLeave, this);
		extension_container->Bind(wxEVT_PAINT, &NavigationButtons::OnPaint, this);
		wxBoxSizer* extension_container_sizer = new wxBoxSizer(wxHORIZONTAL);

		extension_icon = new wxStaticBitmap(
			extension_container, wxID_ANY, wxBitmapBundle::FromBitmap(wxBitmap(icons_dir+"extension_gray.png", wxBITMAP_TYPE_PNG))
		);
		extension_icon->Bind(wxEVT_LEFT_UP, &NavigationButtons::OnSelect, this);
		extension_container_sizer->Add(extension_icon, 1);

		extension_container->SetSizerAndFit(extension_container_sizer);
		sizer->Add(extension_container, 1);

		wxPanel* focus_container = new wxPanel(this);
		focus_container->SetName("focus");
		focus_container->Bind(wxEVT_ENTER_WINDOW, &NavigationButtons::OnMouseEnter, this);
		focus_container->Bind(wxEVT_LEAVE_WINDOW, &NavigationButtons::OnMouseLeave, this);
		focus_container->Bind(wxEVT_PAINT, &NavigationButtons::OnPaint, this);
		wxBoxSizer* focus_container_sizer = new wxBoxSizer(wxHORIZONTAL);

		focus_icon = new wxStaticBitmap(
			focus_container, wxID_ANY, wxBitmapBundle::FromBitmap(wxBitmap(icons_dir+"focus_gray.png", wxBITMAP_TYPE_PNG))
		);
		focus_icon->Bind(wxEVT_LEFT_UP, &NavigationButtons::OnSelect, this);
		focus_container_sizer->Add(focus_icon, 1);

		focus_container->SetSizerAndFit(focus_container_sizer);
		sizer->Add(focus_container, 1);

		wxPanel* settings_container = new wxPanel(this);
		settings_container->SetName("settings");
		settings_container->Bind(wxEVT_ENTER_WINDOW, &NavigationButtons::OnMouseEnter, this);
		settings_container->Bind(wxEVT_LEAVE_WINDOW, &NavigationButtons::OnMouseLeave, this);
		settings_container->Bind(wxEVT_PAINT, &NavigationButtons::OnPaint, this);
		wxBoxSizer* settings_container_sizer = new wxBoxSizer(wxHORIZONTAL);

		settings_icon = new wxStaticBitmap(
			settings_container, wxID_ANY, wxBitmapBundle::FromBitmap(wxBitmap(icons_dir+"settings_gray.png", wxBITMAP_TYPE_PNG))
		);
		settings_icon->Bind(wxEVT_LEFT_UP, &NavigationButtons::OnSelect, this);
		settings_container_sizer->Add(settings_icon, 1);

		settings_container->SetSizerAndFit(settings_container_sizer);
		sizer->Add(settings_container, 1);

		SetSizerAndFit(sizer);*/
	}
	void OnSelect(wxMouseEvent& event) {
		/*auto target = ((wxStaticBitmap*)event.GetEventObject());
		if(target->GetName() == current_page_name) return;
		if(current_page_icon) {
			current_page_icon->SetBitmap(wxBitmapBundle::FromBitmap(wxBitmap(icons_dir+current_page_name+"_gray.png", wxBITMAP_TYPE_PNG )));
		}
		current_page_name = target->GetParent()->GetName();
		current_page_icon = target;
		wxString page_name = current_page_name;
		target->SetBitmap(wxBitmapBundle::FromBitmap(wxBitmap(icons_dir+current_page_name+"_blue.png", wxBITMAP_TYPE_PNG )));*/
	}
private:
	void OnPaint(wxPaintEvent& event) {
		/*auto target = ((wxPanel*)event.GetEventObject());
		auto children = ((wxStaticBitmap*)target->GetChildren()[0]);
		if(mouse_over == target->GetName() || current_page_name == target->GetName()) {
			children->SetBitmap(wxBitmapBundle::FromBitmap(wxBitmap(icons_dir+target->GetName()+"_blue.png", wxBITMAP_TYPE_PNG )));
		} else {
			children->SetBitmap(wxBitmapBundle::FromBitmap(wxBitmap(icons_dir+target->GetName()+"_gray.png", wxBITMAP_TYPE_PNG )));
		}*/
	}
	void OnMouseEnter(wxMouseEvent& event) {
		/*auto target = ((wxStaticBitmap*)event.GetEventObject());
		mouse_over = target->GetName();
		target->Refresh();*/
	}
	void OnMouseLeave(wxMouseEvent& event) {
		/*auto target = ((wxStaticBitmap*)event.GetEventObject());
		mouse_over = current_page_name;
		target->Refresh();*/
	}
};