#include "side.hpp"

SideNavigation::SideNavigation(wxPanel* parent, wxWindowID ID) : wxPanel(parent, ID)
{
	this->SetBackgroundColour(wxColor(30, 30, 30));
	sizer = new wxBoxSizer(wxVERTICAL);

	wxPanel* top_container = new wxPanel(this);
	wxBoxSizer* top_ctn_sizer = new wxBoxSizer(wxVERTICAL);

	wxPanel* bottom_container = new wxPanel(this);
	wxBoxSizer* bottom_ctn_sizer = new wxBoxSizer(wxVERTICAL);

	//code page
	wxPanel* code_page = new wxPanel(top_container);
	code_page->Bind(wxEVT_ENTER_WINDOW, &SideNavigation::OnEnterComp, this);
	code_page->Bind(wxEVT_LEAVE_WINDOW, &SideNavigation::OnLeaveComp, this);
	code_page->Bind(wxEVT_PAINT, &SideNavigation::OnPaint, this);
	code_page->SetName("code_page");

	wxBoxSizer* code_pg_sizer = new wxBoxSizer(wxHORIZONTAL);

	wxStaticBitmap* code_page_icon = new wxStaticBitmap(code_page, wxID_ANY, wxBitmapBundle::FromBitmap(wxBitmap(icons_dir + "code_page.png", wxBITMAP_TYPE_PNG)));
	code_page_icon->SetName("code_page");
	code_pg_sizer->Add(code_page_icon, 20, wxEXPAND | wxALL, 6);

	code_page_icon->Bind(wxEVT_LEFT_UP, &SideNavigation::SelectPage, this);
	code_page->Bind(wxEVT_LEFT_UP, &SideNavigation::SelectPage, this);

	code_page->SetSizerAndFit(code_pg_sizer);
	top_ctn_sizer->Add(code_page, 0, wxEXPAND | wxALL, 6);
	//

	//search page
	wxPanel* search_page = new wxPanel(top_container);
	search_page->Bind(wxEVT_ENTER_WINDOW, &SideNavigation::OnEnterComp, this);
	search_page->Bind(wxEVT_LEAVE_WINDOW, &SideNavigation::OnLeaveComp, this);
	search_page->Bind(wxEVT_PAINT, &SideNavigation::OnPaint, this);
	search_page->SetName("search_page");

	wxBoxSizer* search_page_sizer = new wxBoxSizer(wxHORIZONTAL);

	wxStaticBitmap* search_page_icon = new wxStaticBitmap(search_page, wxID_ANY, wxBitmapBundle::FromBitmap(wxBitmap(icons_dir + "search.png", wxBITMAP_TYPE_PNG)));
	search_page_icon->SetName("search_page");
	search_page_sizer->Add(search_page_icon, 20, wxEXPAND | wxALL, 6);

	search_page_icon->Bind(wxEVT_LEFT_UP, &SideNavigation::SelectPage, this);
	search_page->Bind(wxEVT_LEFT_UP, &SideNavigation::SelectPage, this);

	search_page->SetSizerAndFit(search_page_sizer);
	top_ctn_sizer->Add(search_page, 0, wxEXPAND | wxALL, 6);
	//

	//builder page
	wxPanel* builder_page = new wxPanel(top_container);
	builder_page->Bind(wxEVT_ENTER_WINDOW, &SideNavigation::OnEnterComp, this);
	builder_page->Bind(wxEVT_LEAVE_WINDOW, &SideNavigation::OnLeaveComp, this);
	builder_page->Bind(wxEVT_PAINT, &SideNavigation::OnPaint, this);
	builder_page->SetName("builder_page");

	wxBoxSizer* builder_page_sizer = new wxBoxSizer(wxHORIZONTAL);

	wxStaticBitmap* builder_page_icon = new wxStaticBitmap(builder_page, wxID_ANY, wxBitmapBundle::FromBitmap(wxBitmap(icons_dir + "builder.png", wxBITMAP_TYPE_PNG)));
	builder_page_icon->SetName("builder_page");
	builder_page_sizer->Add(builder_page_icon, 20, wxEXPAND | wxALL, 6);

	builder_page_icon->Bind(wxEVT_LEFT_UP, &SideNavigation::SelectPage, this);
	builder_page->Bind(wxEVT_LEFT_UP, &SideNavigation::SelectPage, this);

	builder_page->SetSizerAndFit(builder_page_sizer);
	top_ctn_sizer->Add(builder_page, 0, wxEXPAND | wxALL, 6);
	//

	//extensions page
	wxPanel* extensions_page = new wxPanel(top_container);
	extensions_page->Bind(wxEVT_ENTER_WINDOW, &SideNavigation::OnEnterComp, this);
	extensions_page->Bind(wxEVT_LEAVE_WINDOW, &SideNavigation::OnLeaveComp, this);
	extensions_page->Bind(wxEVT_PAINT, &SideNavigation::OnPaint, this);
	extensions_page->SetName("extensions_page");

	wxBoxSizer* extensions_page_sizer = new wxBoxSizer(wxHORIZONTAL);

	wxStaticBitmap* extensions_page_icon = new wxStaticBitmap(extensions_page, wxID_ANY, wxBitmapBundle::FromBitmap(wxBitmap(icons_dir + "extensions.png", wxBITMAP_TYPE_PNG)));
	extensions_page_icon->SetName("extensions_page");
	extensions_page_sizer->Add(extensions_page_icon, 20, wxEXPAND | wxALL, 6);

	extensions_page_icon->Bind(wxEVT_LEFT_UP, &SideNavigation::SelectPage, this);
	extensions_page->Bind(wxEVT_LEFT_UP, &SideNavigation::SelectPage, this);

	extensions_page->SetSizerAndFit(extensions_page_sizer);
	top_ctn_sizer->Add(extensions_page, 0, wxEXPAND | wxALL, 6);
	//

	//settings page
	wxPanel* settings_page = new wxPanel(bottom_container);
	settings_page->Bind(wxEVT_ENTER_WINDOW, &SideNavigation::OnEnterComp, this);
	settings_page->Bind(wxEVT_LEAVE_WINDOW, &SideNavigation::OnLeaveComp, this);
	settings_page->Bind(wxEVT_PAINT, &SideNavigation::OnPaint, this);
	settings_page->SetName("settings_page");

	wxBoxSizer* settings_page_sizer = new wxBoxSizer(wxHORIZONTAL);

	wxStaticBitmap* settings_page_icon = new wxStaticBitmap(settings_page, wxID_ANY, wxBitmapBundle::FromBitmap(wxBitmap(icons_dir + "settings.png", wxBITMAP_TYPE_PNG)));
	settings_page_icon->SetName("settings_page");
	settings_page_sizer->Add(settings_page_icon, 20, wxEXPAND | wxALL, 6);

	settings_page_icon->Bind(wxEVT_LEFT_UP, &SideNavigation::SelectPage, this);
	settings_page->Bind(wxEVT_LEFT_UP, &SideNavigation::SelectPage, this);

	settings_page->SetSizerAndFit(settings_page_sizer);
	bottom_ctn_sizer->Add(settings_page, 0, wxEXPAND | wxALL, 6);
	//

	//focus page
	wxPanel* focus_page = new wxPanel(bottom_container);
	focus_page->Bind(wxEVT_ENTER_WINDOW, &SideNavigation::OnEnterComp, this);
	focus_page->Bind(wxEVT_LEAVE_WINDOW, &SideNavigation::OnLeaveComp, this);
	focus_page->Bind(wxEVT_PAINT, &SideNavigation::OnPaint, this);
	focus_page->SetName("focus_page");

	wxBoxSizer* focus_page_sizer = new wxBoxSizer(wxHORIZONTAL);

	wxStaticBitmap* focus_page_icon = new wxStaticBitmap(focus_page, wxID_ANY, wxBitmapBundle::FromBitmap(wxBitmap(icons_dir + "focus_mode.png", wxBITMAP_TYPE_PNG)));
	focus_page_icon->SetName("focus_page");
	focus_page_sizer->Add(focus_page_icon, 20, wxEXPAND | wxALL, 6);

	focus_page_icon->Bind(wxEVT_LEFT_UP, &SideNavigation::SelectPage, this);
	focus_page->Bind(wxEVT_LEFT_UP, &SideNavigation::SelectPage, this);

	focus_page->SetSizerAndFit(focus_page_sizer);
	bottom_ctn_sizer->Add(focus_page, 0, wxEXPAND | wxALL, 6);
	//

	top_container->SetSizerAndFit(top_ctn_sizer);
	bottom_container->SetSizerAndFit(bottom_ctn_sizer);

	sizer->Add(top_container, 1, wxEXPAND);
	sizer->Add(bottom_container, 0, wxEXPAND);
	this->SetSizerAndFit(sizer);
}

void SideNavigation::SelectPage(wxMouseEvent& event) {
	auto target = ((wxPanel*)event.GetEventObject());
	if (target) {
		wxString page_name = target->GetName();
		auto main_container = FindWindowById(ID_MAIN_CONTAINER);
		auto side_container = FindWindowById(ID_SIDE_CONTAINER);

		selected_page = page_name;

		if (page_name == "focus_page") {
			ToggleFocusMode();
		}
		else {
			for (auto& children : side_container->GetChildren())
				children->Hide();

			if (page_name == "code_page") {
				FindWindowById(ID_MAIN_SPLITTER)->Show();
				if (FindWindowById(ID_SETTINGS_PAGE)) {
					if (settings_page->IsShown()) settings_page->Hide();
				}
				FindWindowById(ID_FILES_TREE)->Show();
			}

			if (page_name == "search_page") {
				if (FindWindowById(ID_SEARCH_PAGE_INPUT)) {
					auto input = FindWindowById(ID_SEARCH_PAGE_INPUT);
					input->Show();
				}
				else {
					SidePage_Search* search_page = new SidePage_Search(side_container, ID_SEARCH_PAGE_INPUT);
					side_container->GetSizer()->Add(search_page, 1, wxEXPAND | wxLEFT, 5);
					side_container->GetSizer()->Layout();
					side_container->Update();
				}
			}

			if (page_name == "builder_page") {
				if (FindWindowById(ID_BUILDER_PAGE)) {
					auto builder_pg = FindWindowById(ID_BUILDER_PAGE);
					builder_pg->Show();
				}
				else {
					SidePage_Builder* builder_page = new SidePage_Builder(side_container, ID_BUILDER_PAGE);
					side_container->GetSizer()->Add(builder_page, 1, wxEXPAND | wxLEFT, 5);
					side_container->GetSizer()->Layout();
					side_container->Update();
				}
			}

			if (page_name == "extensions_page") {
				if (FindWindowById(ID_EXTENSIONS_PAGE)) {
					auto extensions_pg = FindWindowById(ID_EXTENSIONS_PAGE);
					extensions_pg->Show();
				}
				else {
					SidePage_Extensions* extensions_page = new SidePage_Extensions(side_container, ID_EXTENSIONS_PAGE);
					side_container->GetSizer()->Add(extensions_page, 1, wxEXPAND | wxLEFT, 5);
					side_container->GetSizer()->Layout();
					side_container->Update();
				}
			}

			if (page_name == "settings_page") {
				if (main_container) {
					auto main_ctn_sizer = main_container->GetSizer();
					settings_page = ((SettingsPage*)FindWindowById(ID_SETTINGS_PAGE));
					FindWindowById(ID_MAIN_SPLITTER)->Hide();
					if (settings_page) {
						if (!settings_page->IsShown()) settings_page->Show();
					}
					else {
						settings_page = new SettingsPage(main_container, ID_SETTINGS_PAGE);
						main_ctn_sizer->Add(settings_page, 1, wxEXPAND);
					}

					main_ctn_sizer->Layout();
					main_container->Update();
				}
			}
		}

		for (auto& children : GetChildren())
			for (auto& _children : children->GetChildren())
				_children->Refresh();
	}
}

void SideNavigation::OnFocusMode(wxCommandEvent& WXUNUSED(event)) {
	ToggleFocusMode();
}

void SideNavigation::ToggleFocusMode() {
	auto main_container = FindWindowById(ID_MAIN_CONTAINER);
	auto side_container = FindWindowById(ID_SIDE_CONTAINER);
	auto tabs = FindWindowById(ID_TABS);
	auto status_bar = FindWindowById(ID_STATUS_BAR);
	if (
		!main_container ||
		!side_container ||
		!tabs ||
		!status_bar
		) return;

	if (focusModeIsEnable) {
		tabs->Show();
		status_bar->Show();
		for (auto&& child : main_container->GetChildren()) {
			if (child->GetId() == ID_MAIN_SPLITTER) {
				((wxSplitterWindow*)child)->SplitVertically(side_container, FindWindowById(ID_SERVICAL_CONTAINER), 1);
			}
			else {
				if (child) {
					if (!child->IsShown()) child->Show();
				}
			}
		}
		focusModeIsEnable = false;
	}
	else {
		tabs->Hide();
		status_bar->Hide();
		for (auto&& child : main_container->GetChildren()) {
			if (child->GetId() == ID_MAIN_SPLITTER) {
				((wxSplitterWindow*)child)->Unsplit(side_container);
			}
			else child->Hide();
		}
		focusModeIsEnable = true;
	}

	main_container->GetSizer()->Layout();
	main_container->Update();
	if (status_bar) {
		status_bar->GetParent()->Layout();
		status_bar->GetParent()->Update();
	}
}

void SideNavigation::OnPaint(wxPaintEvent& event) {
	auto window = ((wxPanel*)event.GetEventObject());
	if (selected_page != window->GetName())
		if (mouse_over != window->GetName()) return;
	wxClientDC dc(window);
	wxGraphicsContext* gc = wxGraphicsContext::Create(dc);
	if (!gc) return;

	gc->SetPen(gc->CreatePen(wxGraphicsPenInfo(wxColor(7, 150, 255)).Width(0.20)));
	gc->SetBrush(wxColor(7, 150, 255));
	gc->DrawRoundedRectangle(
		0.0,
		0.0,
		static_cast<double>(window->GetSize().GetWidth()),
		static_cast<double>(window->GetSize().GetHeight()),
		10
	);
	delete gc;
}

void SideNavigation::OnEnterComp(wxMouseEvent& event) {
	auto target = ((wxWindow*)event.GetEventObject());
	if (target) {
		mouse_over = target->GetName();
		target->Refresh();
	}
}

void SideNavigation::OnLeaveComp(wxMouseEvent& event) {
	auto target = ((wxWindow*)event.GetEventObject());
	if (target) {
		mouse_over = selected_page;
		target->Refresh();
	}
}