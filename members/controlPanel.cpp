#include "notifyPanel.cpp"
#include "terminal.cpp"

struct ControlMenu {
	const char* name;
	const char* shortkut;
	int ID;
};

class ControlPanel : public wxPanel {
	wxScrolled<wxPanel>* menus_container;
	wxPanel* selectedMenu;
public:
	ControlPanel(wxFrame* parent, wxWindowID ID) : wxPanel(
		parent, ID, wxPoint(parent->GetSize().GetWidth() / 2 - 225, 50), wxSize(450, 250)
	) {
		wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
		SetMinSize(wxSize(450, 250));
		SetSize(wxSize(450, 250));
		SetBackgroundColour(wxColor(36, 36, 36));
		SetFocus();

		wxPanel* top_container = new wxPanel(this);
		wxBoxSizer* top_ctn_sizer = new wxBoxSizer(wxHORIZONTAL);

		wxVector<wxBitmap> bitmaps;
		bitmaps.push_back(wxBitmap(icons_dir + "search_gray.png", wxBITMAP_TYPE_PNG));
		wxStaticBitmap* search_icon = new wxStaticBitmap(top_container, wxID_ANY, wxBitmapBundle::FromBitmaps(bitmaps));
		top_ctn_sizer->Add(search_icon, 0, wxEXPAND | wxALL, 5);

		wxTextCtrl* search_input = new wxTextCtrl(top_container, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
		search_input->SetBackgroundColour(wxColor(36, 36, 36));
		search_input->SetFocus();
		top_ctn_sizer->Add(search_input, 1, wxEXPAND);

		top_container->SetSizerAndFit(top_ctn_sizer);
		sizer->Add(top_container, 0, wxEXPAND);

		menus_container = new wxScrolled<wxPanel>(this);
		wxBoxSizer* menu_ctn_sizer = new wxBoxSizer(wxVERTICAL);

		for (const auto& ctrl_menu : menus) {
			wxPanel* menu = new wxPanel(menus_container);
			menu->SetLabel(std::to_string(ctrl_menu.ID));
			if (ctrl_menu.ID == 1) {
				menu->SetBackgroundColour(wxColor(56, 56, 56));
				selectedMenu = menu;
			}
			wxBoxSizer* menu_sizer = new wxBoxSizer(wxHORIZONTAL);

			wxStaticText* menu_name = new wxStaticText(menu, wxID_ANY, ctrl_menu.name);
			menu_name->Bind(wxEVT_LEFT_UP, &ControlPanel::OnClickSelect, this);
			menu_name->SetName(std::to_string(ctrl_menu.ID));
			wxFont menu_name_font(10, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
			menu_name_font.SetFaceName(wxT("Monospace"));
			menu_name->SetFont(menu_name_font);
			menu_sizer->Add(menu_name, 1, wxEXPAND | wxALL, 5);

			wxStaticText* menu_shortkut = new wxStaticText(menu, wxID_ANY, ctrl_menu.shortkut);
			wxFont menu_shortkut_font(7, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
			menu_shortkut_font.SetFaceName(wxT("Monospace"));
			menu_shortkut->SetFont(menu_shortkut_font);
			menu_sizer->Add(menu_shortkut, 0, wxEXPAND | wxTOP, 8);

			menu->SetSizerAndFit(menu_sizer);
			menu_ctn_sizer->Add(menu, 0, wxEXPAND | wxALL, 3);
		}

		menus_container->SetSizerAndFit(menu_ctn_sizer);
		menus_container->FitInside();
		menus_container->SetScrollRate(20, 20);
		sizer->Add(menus_container, 1, wxEXPAND);

		wxAcceleratorEntry entries[4];
		entries[0].Set(wxACCEL_NORMAL, WXK_ESCAPE, ID_EXIT_CONTROL_PANEL);
		entries[1].Set(wxACCEL_NORMAL, WXK_UP, ID_CONTROL_PANEL_UP);
		entries[2].Set(wxACCEL_NORMAL, WXK_DOWN, ID_CONTROL_PANEL_DOWN);
		entries[3].Set(wxACCEL_NORMAL, WXK_RETURN, ID_CONTROL_PANEL_SELECT);
		wxAcceleratorTable accel(4, entries);
		SetAcceleratorTable(accel);

		SetSizerAndFit(sizer);
		SetMinSize(wxSize(450, 250));
		SetSize(wxSize(450, 250));
	}
	void Close(wxCommandEvent& event) {
		FindWindowById(ID_STATUS_BAR)->SetFocus();
		Destroy();
	}
	void UpSelection(wxCommandEvent& event) {
		bool finded;
		for (auto&& menu : menus_container->GetChildren()) {
			if (finded) return;
			if (menu == selectedMenu) {
				auto next_menu = menu->GetPrevSibling();
				if (!next_menu) {
					next_menu = menus_container->GetChildren()[0];
				}

				if (next_menu != menu) {
					next_menu->SetBackgroundColour(wxColor(56, 56, 56));
					menu->SetBackgroundColour(wxColor(36, 36, 36));
					selectedMenu = ((wxPanel*)next_menu);
					finded = true;
				}
			}
		}
	}
	void DownSelection(wxCommandEvent& event) {
		bool finded;
		for (auto&& menu : menus_container->GetChildren()) {
			if (finded) return;
			if (menu == selectedMenu) {
				auto next_menu = menu->GetNextSibling();
				if (!next_menu) {
					next_menu = menus_container->GetChildren()[0];
				}

				next_menu->SetBackgroundColour(wxColor(56, 56, 56));
				menu->SetBackgroundColour(wxColor(36, 36, 36));
				selectedMenu = ((wxPanel*)next_menu);
				finded = true;
			}
		}
	}
	void OnKeyBoardSelect(wxCommandEvent& event) {
		wxString id = selectedMenu->GetLabel();
		if (id.size() > 0) Select(id);
	}
	void OnClickSelect(wxMouseEvent& event) {
		wxString id = ((wxWindow*)event.GetEventObject())->GetName();
		if (id.size() > 0) Select(id);
	}
	void Select(wxString id) {
		switch (static_cast<char>(id[0])) {
		case '1': {
		} break;
		case '2': {} break;
		case '3': {} break;
		case '4': {
			std::vector<NotifyInteractions> ntf_interactions{
				{"Cancel", "cancel"},
				{"Ok", "ok"},
			};
			auto notify_panel = new NotifyPanel(
				((wxWindow*)FindWindowById(ID_MAIN_SPLITTER)->GetParent()),
				"Executando teste de painel de Notificações",
				ntf_interactions
			);
		} break;
		case '5': {
			((wxSplitterWindow*)FindWindowById(ID_MAIN_CONTAINER_SPLITTER))->SplitHorizontally(
				FindWindowById(ID_CENTERED_CONTENT), FindWindowById(ID_TERMINAL), 0);
		} break;
		}

		FindWindowById(ID_STATUS_BAR)->SetFocus();
		Destroy();
	}
private:
	std::vector<ControlMenu> menus{
		{"Enter Focus Mode", "Shift+F11", 1},
		{"Hide Side Navigation", "", 2},
		{"Extensions: Install", "", 3},
		{"Notify Panel", "Ctrl+Shift+N", 4},
		{"Open Terminal", "Ctrl+Shift+T", 5},
	};
	wxDECLARE_NO_COPY_CLASS(ControlPanel);
	wxDECLARE_EVENT_TABLE();
};

wxBEGIN_EVENT_TABLE(ControlPanel, wxPanel)
EVT_MENU(ID_EXIT_CONTROL_PANEL, ControlPanel::Close)
EVT_MENU(ID_CONTROL_PANEL_UP, ControlPanel::UpSelection)
EVT_MENU(ID_CONTROL_PANEL_DOWN, ControlPanel::DownSelection)
EVT_MENU(ID_CONTROL_PANEL_SELECT, ControlPanel::OnKeyBoardSelect)
wxEND_EVENT_TABLE()