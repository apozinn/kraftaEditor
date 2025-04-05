#include "tabs.hpp"

Tabs::Tabs(wxPanel* parent, wxWindowID ID) : wxPanel(parent, ID)
{
	auto background_color = UserTheme["main"].template get<std::string>();

	SetBackgroundColour(wxColor(background_color));
	sizer = new wxBoxSizer(wxHORIZONTAL);

	wxPanel* tabs_controller = new wxPanel(this);
	wxBoxSizer* tabs_controller_sizer = new wxBoxSizer(wxHORIZONTAL);

	wxVector<wxBitmap> arrow_left_bitmap;
	arrow_left_bitmap.push_back(wxBitmap(icons_dir + "arrow_left.png", wxBITMAP_TYPE_PNG));
	arrow_left = new wxStaticBitmap(tabs_controller, wxID_ANY, wxBitmapBundle::FromBitmaps(arrow_left_bitmap));
	arrow_left->Bind(wxEVT_LEFT_UP, &Tabs::OnPreviousTab, this);

	tabs_controller_sizer->Add(arrow_left, 0, wxLEFT, 5);

	wxVector<wxBitmap> arrow_right_bitmap;
	arrow_right_bitmap.push_back(wxBitmap(icons_dir + "arrow_right.png", wxBITMAP_TYPE_PNG));
	arrow_right = new wxStaticBitmap(tabs_controller, wxID_ANY, wxBitmapBundle::FromBitmaps(arrow_right_bitmap));
	arrow_right->Bind(wxEVT_LEFT_UP, &Tabs::OnNextTab, this);

	tabs_controller_sizer->Add(arrow_right, 0, wxRIGHT, 5);

	tabs_controller->SetSizerAndFit(tabs_controller_sizer);

	sizer->Add(tabs_controller, 0, wxALIGN_CENTER);

	tabs_container = new wxScrolled<wxPanel>(this, ID_TABS_CONTAINER);
	tabs_ctn_sizer = new wxBoxSizer(wxHORIZONTAL);
	tabs_container->SetSizerAndFit(tabs_ctn_sizer);

	wxVector<wxBitmap> bitmaps;
	bitmaps.push_back(wxBitmap(icons_dir + "menu_down.png", wxBITMAP_TYPE_PNG));
	menu = new wxStaticBitmap(this, wxID_ANY, wxBitmapBundle::FromBitmaps(bitmaps));
	menu->Bind(wxEVT_LEFT_UP, &Tabs::OnMenu, this);

	sizer->Add(tabs_container, 1, wxEXPAND | wxTOP | wxBOTTOM, 5);
	sizer->Add(menu, 0, wxALIGN_CENTER | wxRIGHT, 10);

	SetSizerAndFit(sizer);
	tabs_container->SetScrollRate(20, 20);
	tabs_container->EnableScrolling(true, false);
	Bind(wxEVT_PAINT, &Tabs::OnPaint, this);

	SetMinSize(wxSize(parent->GetSize().x, 50));
	Hide();
}

void Tabs::Add(wxString tab_name, wxString path)
{
	if (!IsShown())
		Show();

	bool exists = false;
	current_openned_path = path;
	for (auto& a_tab : tabs_container->GetChildren())
	{
		if (a_tab->GetName() == path)
		{
			exists = true;
		}
		else
			a_tab->Refresh();
	}
	if (exists)
		return;

	wxPanel* new_tab = new wxPanel(tabs_container);

	new_tab->SetName(path);
	new_tab->SetLabel(path + "_tab");
	new_tab->Bind(wxEVT_LEFT_UP, &Tabs::OnTabClick, this);
	wxBoxSizer* new_tab_sizer = new wxBoxSizer(wxVERTICAL);

	wxPanel* tab_infos = new wxPanel(new_tab);
	wxBoxSizer* tab_infos_sizer = new wxBoxSizer(wxHORIZONTAL);

	LanguageInfo const* currentLanguageInfo;
	LanguageInfo const* currentInfo;
	int languageNr;

	bool found = false;
	for (languageNr = 0; languageNr < languages_prefs_size; languageNr++)
	{
		currentInfo = &languages_prefs[languageNr];
		wxString filepattern = currentInfo->filepattern;
		filepattern.Lower();

		while (!filepattern.empty() && !found)
		{
			wxString cur = filepattern.BeforeFirst(';');
			if ((cur == path) ||
				(cur == (path.BeforeLast('.') + ".*")) ||
				(cur == ("*." + path.AfterLast('.'))))
			{
				found = true;
				currentLanguageInfo = currentInfo;
			}
			filepattern = filepattern.AfterFirst(';');
		}
	}

	if (!found)
		currentLanguageInfo = &languages_prefs[0];

	wxVector<wxBitmap> bitmaps_;
	bitmaps_.push_back(wxBitmap(icons_dir + currentLanguageInfo->icon_path, wxBITMAP_TYPE_PNG));
	wxStaticBitmap* ico = new wxStaticBitmap(tab_infos, wxID_ANY, wxBitmapBundle::FromBitmaps(bitmaps_));
	tab_infos_sizer->Add(ico, 0, wxALIGN_CENTER | wxLEFT, 10);

	wxStaticText* name = new wxStaticText(tab_infos, wxID_ANY, tab_name);

	name->SetName(path);
	name->Bind(wxEVT_LEFT_UP, &Tabs::OnTabClick, this);
	tab_infos_sizer->Add(name, 1, wxEXPAND | wxLEFT | wxRIGHT, 6);

	wxVector<wxBitmap> bitmaps;
	bitmaps.push_back(wxBitmap(wxBitmap(icons_dir + "close.png", wxBITMAP_TYPE_PNG)));
	wxStaticBitmap* close_icon = new wxStaticBitmap(tab_infos, wxID_ANY, wxBitmapBundle::FromBitmaps(bitmaps));
	close_icon->Bind(wxEVT_LEFT_UP, &Tabs::OnCloseTab, this);
	tab_infos_sizer->Add(close_icon, 0, wxALIGN_CENTER | wxRIGHT, 10);

	tab_infos->Bind(wxEVT_ENTER_WINDOW, &Tabs::OnEnterComp, this);
	tab_infos->Bind(wxEVT_LEAVE_WINDOW, &Tabs::OnLeaveComp, this);

	tab_infos->SetSizerAndFit(tab_infos_sizer);
	new_tab_sizer->Add(tab_infos, 1, wxEXPAND | wxTOP | wxBOTTOM, 6);

	wxPanel* active_bar = new wxPanel(new_tab);
	new_tab_sizer->Add(active_bar, 0, wxEXPAND);

	new_tab->SetSizerAndFit(new_tab_sizer);
	tabs_ctn_sizer->Add(new_tab, 0, wxALIGN_CENTER | wxLEFT, 5);
	tabs_container->FitInside();
	tabs_container->Scroll(1000, 0);
	new_tab->Bind(wxEVT_PAINT, &Tabs::OnTabPaint, this);

	SetMinSize(wxSize(GetSize().GetWidth(), new_tab->GetSize().GetHeight() + 10));

	for (auto&& tab : tabs_container->GetChildren())
		tab->Refresh();

	sizer->Layout();
	tabs_ctn_sizer->Layout();
}

void Tabs::Close(wxWindow* tab, wxString tab_path)
{
	auto codeContainer = ((CodeContainer*)FindWindowByName(tab_path + "_codeContainer"));
	auto imgContainer = ((wxPanel*)FindWindowByLabel(tab_path + "_imgContainer"));
	auto fileContainer = ((FilesTree*)FindWindowById(ID_FILES_TREE));
	auto main_code = FindWindowById(ID_MAIN_CODE);

	//close linked components
	if (codeContainer)
		codeContainer->Destroy();
	if (imgContainer)
		imgContainer->Destroy();

	//verify if exists another tab to active
	wxWindow* descendantTab = NULL;
	if (auto prevTab = tab->GetPrevSibling()) {
		descendantTab = prevTab;
	}
	else if (auto nextTab = tab->GetNextSibling()) {
		descendantTab = nextTab;
	}

	//opening the new tab and its components
	if (descendantTab) {
		current_openned_path = descendantTab->GetName();
		descendantTab->Refresh();

		//getting linked file from filesTree and changing highlight file
		auto linkedFile = FindWindowByLabel(current_openned_path + "_file_container");
		if (linkedFile) {
			fileContainer->selectedFile->SetBackgroundColour(wxColor(45, 45, 45));
			fileContainer->SetSelectedFile(linkedFile);
			linkedFile->SetBackgroundColour(wxColor(60, 60, 60));
		}

		//openning the new component 
		auto other_codeContainer = ((CodeContainer*)FindWindowByName(current_openned_path + "_codeContainer"));
		if (other_codeContainer)
			other_codeContainer->Show();

		auto new_imageContainer = FindWindowByLabel(current_openned_path + "_imgContainer");
		if (new_imageContainer)
			new_imageContainer->Show();
	}
	else {
		//If you don't have another tab, close the tabContainer and show the empty window
		fileContainer->selectedFile->SetBackgroundColour(wxColor(45, 45, 45));
		this->Hide();
		FindWindowById(ID_EMPYT_WINDOW)->Show();
		((StatusBar*)FindWindowById(ID_STATUS_BAR))->ClearLabels();
		fileContainer->selectedFile->SetBackgroundColour(wxColor(45, 45, 45));
		fileContainer->selectedFile = NULL;
	}

	//destroy tab and update
	tab->Destroy();
	tabs_container->GetSizer()->Layout();
	tabs_container->FitInside();
	main_code->GetSizer()->Layout();
}

void Tabs::CloseAll()
{
	tabs_container->DestroyChildren();
	Hide();
	if (auto emptyWindow = FindWindowById(ID_EMPYT_WINDOW)) emptyWindow->Show();
}

void Tabs::Select() {
	auto main_code = FindWindowById(ID_MAIN_CODE);

	for (auto& children : tabs_container->GetChildren())
		children->Refresh();

	auto codeContainer = ((CodeContainer*)FindWindowByName(current_openned_path + "_codeContainer"));
	auto imageContainer = ((wxStaticBitmap*)FindWindowByLabel(current_openned_path + "_imageContainer"));
	auto statusBar = ((StatusBar*)FindWindowById(ID_STATUS_BAR));

	for (auto&& other_ct : main_code->GetChildren())
	{
		if (other_ct->GetId() != ID_TABS)
			other_ct->Hide();
	}

	if (codeContainer)
	{
		codeContainer->Show();
		statusBar->UpdateComponents(current_openned_path, "text", codeContainer->currentLanguage->name);
	}

	if (imageContainer)
	{
		imageContainer->Show();
		statusBar->UpdateComponents(current_openned_path, "image", "img");
	}

	main_code->GetSizer()->Layout();
	main_code->Update();
}

void Tabs::OnTabClick(wxMouseEvent& event)
{
	auto this_tab = ((Tabs*)event.GetEventObject());
	wxString tab_path = this_tab->GetName();
	if (tab_path == current_openned_path)
		return;
	current_openned_path = tab_path;
	this->Select();
}

void Tabs::OnCloseTab(wxMouseEvent& event)
{
	wxObject* obj = event.GetEventObject();
	auto this_tab = ((wxWindow*)obj)->GetGrandParent();
	if (this_tab)
		Tabs::Close(this_tab, this_tab->GetName());
}

void Tabs::OnMenu(wxMouseEvent& event)
{
	wxMenu* tabsMenu = new wxMenu;
	tabsMenu->Append(ID_CLOSE_ALL_FILES, _("&Close All"));
	tabsMenu->Append(wxID_ANY, _("&First Tab"));
	tabsMenu->Append(wxID_ANY, _("&Last Tab"));
	tabsMenu->Append(wxID_ANY, _("&Close Saved"));
	PopupMenu(tabsMenu);
}

void Tabs::OnEnterComp(wxMouseEvent& event)
{
	auto target = ((wxWindow*)event.GetEventObject());
	auto codeContainer = ((wxStyledTextCtrl*)FindWindowByName(target->GetParent()->GetName() + "_codeContainer"));
	if (target && codeContainer)
	{
		auto icon = ((wxStaticBitmap*)target->GetChildren()[2]);
		if (icon)
		{
			auto icon = ((wxStaticBitmap*)target->GetChildren()[2]);
			if (icon)
			{
				auto codeEditor = ((wxStyledTextCtrl*)codeContainer->GetChildren()[0]);
				if (codeEditor->GetModify())
				{
					icon->Show();
					icon->SetBitmap(wxBitmapBundle::FromBitmap(wxBitmap(icons_dir + "close.png", wxBITMAP_TYPE_PNG)));
				}
			}
		}
	}
}

void Tabs::OnLeaveComp(wxMouseEvent& event)
{
	auto target = ((wxWindow*)event.GetEventObject());
	if (target)
	{
		auto codeContainer = ((wxWindow*)FindWindowByName(target->GetParent()->GetName() + "_codeContainer"));
		wxStaticBitmap* icon = ((wxStaticBitmap*)target->GetChildren()[2]);
		if (codeContainer)
		{
			auto codeEditor = ((wxStyledTextCtrl*)codeContainer->GetChildren()[0]);
			if (codeEditor->GetModify())
			{
				icon->SetBitmap(wxBitmapBundle::FromBitmap(wxBitmap(icons_dir + "white_circle.png", wxBITMAP_TYPE_PNG)));
			}
			else
			{
				icon->SetBitmap(wxBitmapBundle::FromBitmap(wxBitmap(icons_dir + "close.png", wxBITMAP_TYPE_PNG)));
			}
		}
	}
}

void Tabs::OnPaint(wxPaintEvent& event)
{
	wxPaintDC dc(this);
	dc.SetBrush(wxColor(UserTheme["border"].template get<std::string>()));
	dc.SetPen(wxPen(wxColor(UserTheme["border"].template get<std::string>()), 0.20));
	dc.DrawLine(0, GetSize().GetHeight() - 1, GetSize().GetWidth(), GetSize().GetHeight() - 1);
}

void Tabs::OnTabPaint(wxPaintEvent& event)
{
	auto target = ((wxPanel*)event.GetEventObject());
	wxPaintDC dc(target);
	wxGraphicsContext* gc = wxGraphicsContext::Create(dc);
	if (!gc)
		return;

	if (current_openned_path == target->GetName())
	{
		auto background = UserTheme["highlight"].template get<std::string>();
		gc->SetPen(wxColor(background));
		gc->SetBrush(wxColor(background));
		gc->DrawRoundedRectangle(0.0, 0.0, static_cast<double>(target->GetSize().GetWidth()) - 1.0, 1.0, 10);
	}
	else
	{
		auto background = UserTheme["main"].template get<std::string>();
		gc->SetPen(wxColor(background));
		gc->SetBrush(wxColor(background));
		gc->DrawRoundedRectangle(0.0, 0.0, static_cast<double>(target->GetSize().GetWidth()), static_cast<double>(target->GetSize().GetHeight()), 10);
	}

	delete gc;
}

void Tabs::OnPreviousTab(wxMouseEvent& event) {
	for (auto&& tab : tabs_container->GetChildren())
		if (tab->GetName() == current_openned_path) {
			auto prevTab = tab->GetPrevSibling();
			if (prevTab) {
				current_openned_path = prevTab->GetName();
				this->Select();
			}
		}
}

void Tabs::OnNextTab(wxMouseEvent& event) {
	for (auto&& tab : tabs_container->GetChildren())
		if (tab->GetName() == current_openned_path) {
			auto nextTab = tab->GetNextSibling();
			if (nextTab) {
				current_openned_path = nextTab->GetName();
				this->Select();
			}
		}
}