#include "tabs.hpp"
#include "ui/ids.hpp"

// #include "gui/editor/codeEditor/codeEditor.hpp"
#include "gui/codeContainer/code.hpp"
#include "gui/panels/filesTree/filesTree.hpp"
#include "gui/widgets/statusBar/statusBar.hpp"

#include <wx/graphics.h>

Tabs::Tabs(wxPanel *parent, wxWindowID ID) : wxPanel(parent, ID)
{
	auto background_color = Theme["main"].template get<std::string>();
	SetBackgroundColour(wxColor(background_color));
	sizer = new wxBoxSizer(wxHORIZONTAL);

	tabsContainer = new wxScrolled<wxPanel>(this, +GUI::ControlID::TabsContainer);
	tabsContainerSizer = new wxBoxSizer(wxHORIZONTAL);
	tabsContainer->SetSizerAndFit(tabsContainerSizer);

	wxVector<wxBitmap> bitmaps;
	bitmaps.push_back(wxBitmap(iconsDir + "menu_down.png", wxBITMAP_TYPE_PNG));
	menu = new wxStaticBitmap(this, wxID_ANY, wxBitmapBundle::FromBitmaps(bitmaps));
	menu->Bind(wxEVT_LEFT_UP, &Tabs::OnMenu, this);

	sizer->Add(tabsContainer, 1, wxEXPAND | wxTOP | wxBOTTOM, 5);
	sizer->Add(menu, 0, wxALIGN_CENTER | wxRIGHT, 10);

	SetSizerAndFit(sizer);
	tabsContainer->SetScrollRate(20, 20);
	tabsContainer->EnableScrolling(true, false);
	Bind(wxEVT_PAINT, &Tabs::OnPaint, this);

	SetMinSize(wxSize(parent->GetSize().x, 50));
	SetLabel("tabsContainer");
	Hide();
}

void Tabs::Add(wxString tab_name, wxString path)
{
	if (!IsShown())
		Show();

	bool exists = false;
	projectSettings.SetCurrentlyFileOpen(path);
	for (auto &a_tab : tabsContainer->GetChildren())
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

	wxPanel *new_tab = new wxPanel(tabsContainer);

	new_tab->SetName(path);
	new_tab->SetLabel(path + "_tab");
	new_tab->Bind(wxEVT_LEFT_UP, &Tabs::OnTabClick, this);
	wxBoxSizer *new_tab_sizer = new wxBoxSizer(wxVERTICAL);

	wxPanel *tab_infos = new wxPanel(new_tab);
	wxBoxSizer *tab_infos_sizer = new wxBoxSizer(wxHORIZONTAL);

	LanguageInfo const *currentLanguageInfo = &languages_prefs[0];
	LanguageInfo const *currentInfo;
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

	wxVector<wxBitmap> bitmaps_;
	bitmaps_.push_back(wxBitmap(ApplicationPaths::GetLanguageIcon(currentLanguageInfo->iconFileName), wxBITMAP_TYPE_PNG));
	wxStaticBitmap *ico = new wxStaticBitmap(tab_infos, wxID_ANY, wxBitmapBundle::FromBitmaps(bitmaps_));
	tab_infos_sizer->Add(ico, 0, wxALIGN_CENTER | wxLEFT, 10);

	wxStaticText *name = new wxStaticText(tab_infos, wxID_ANY, tab_name);
	name->SetToolTip(path);
	name->SetName(path);
	name->Bind(wxEVT_LEFT_UP, &Tabs::OnTabClick, this);
	tab_infos_sizer->Add(name, 1, wxEXPAND | wxLEFT | wxRIGHT, 6);

	wxVector<wxBitmap> bitmaps;
	bitmaps.push_back(wxBitmap(wxBitmap(iconsDir + "close.png", wxBITMAP_TYPE_PNG)));
	wxStaticBitmap *close_icon = new wxStaticBitmap(tab_infos, wxID_ANY, wxBitmapBundle::FromBitmaps(bitmaps));
	close_icon->Bind(wxEVT_LEFT_UP, &Tabs::OnCloseTab, this);
	tab_infos_sizer->Add(close_icon, 0, wxALIGN_CENTER | wxRIGHT, 10);

	tab_infos->Bind(wxEVT_ENTER_WINDOW, &Tabs::OnEnterComp, this);
	tab_infos->Bind(wxEVT_LEAVE_WINDOW, &Tabs::OnLeaveComp, this);

	tab_infos->SetSizerAndFit(tab_infos_sizer);
	new_tab_sizer->Add(tab_infos, 1, wxEXPAND | wxTOP | wxBOTTOM, 6);

	wxPanel *active_bar = new wxPanel(new_tab);
	new_tab_sizer->Add(active_bar, 0, wxEXPAND);

	new_tab->SetSizerAndFit(new_tab_sizer);
	tabsContainerSizer->Add(new_tab, 0, wxALIGN_CENTER | wxLEFT, 5);
	tabsContainer->FitInside();
	tabsContainer->Scroll(1000, 0);
	new_tab->Bind(wxEVT_PAINT, &Tabs::OnTabPaint, this);

	SetMinSize(wxSize(GetSize().GetWidth(), new_tab->GetSize().GetHeight() + 10));

	for (auto &&tab : tabsContainer->GetChildren())
		tab->Refresh();

	sizer->Layout();
	tabsContainerSizer->Layout();
	;
}

void Tabs::Close(wxWindow *tab, wxString tab_path)
{
	auto codeContainer = ((CodeContainer *)FindWindowByName(tab_path + "_codeContainer"));
	auto imgContainer = ((wxPanel *)FindWindowByLabel(tab_path + "_imgContainer"));
	auto fileContainer = ((FilesTree *)FindWindowById(+GUI::ControlID::FilesTree));
	auto mainCode = FindWindowById(+GUI::ControlID::MainCode);

	// close linked components
	if (codeContainer)
		codeContainer->Destroy();
	if (imgContainer)
		imgContainer->Destroy();

	// verify if exists another tab to active
	wxWindow *descendantTab = NULL;
	if (auto prevTab = tab->GetPrevSibling())
	{
		descendantTab = prevTab;
	}
	else if (auto nextTab = tab->GetNextSibling())
	{
		descendantTab = nextTab;
	}

	// opening the new tab and its components
	if (descendantTab)
	{
		projectSettings.SetCurrentlyFileOpen(descendantTab->GetName());
		descendantTab->Refresh();

		// getting linked file from filesTree and changing highlight file
		auto linkedFile = FindWindowByLabel(ProjectSettings::Get().GetCurrentlyFileOpen() + "_file_container");
		if (linkedFile)
		{
			fileContainer->SetSelectedFile(linkedFile);
		}

		// openning the new component
		auto other_codeContainer = ((CodeContainer *)FindWindowByName(ProjectSettings::Get().GetCurrentlyFileOpen() + "_codeContainer"));
		if (other_codeContainer)
			other_codeContainer->Show();

		auto new_imageContainer = FindWindowByLabel(ProjectSettings::Get().GetCurrentlyFileOpen() + "_imgContainer");
		if (new_imageContainer)
			new_imageContainer->Show();
	}
	else
	{
		// If you don't have another tab, close the tabContainer and show the empty window
		fileContainer->SetSelectedFile(nullptr);

		Hide();
		auto emptyWindow = FindWindowById(+GUI::ControlID::EmptyWindow);
		if (emptyWindow)
		{
			emptyWindow->Show();
		}

		auto statusBar = ((StatusBar *)FindWindowById(+GUI::ControlID::StatusBar));
		if (statusBar)
		{
			statusBar->ClearLabels();
		}
	}

	// destroy tab and update
	tab->Destroy();
	tabsContainer->GetSizer()->Layout();
	tabsContainer->FitInside();
	mainCode->GetSizer()->Layout();
}

void Tabs::CloseAll()
{
	auto fileContainer = ((FilesTree *)FindWindowById(+GUI::ControlID::FilesTree));
	if (fileContainer)
	{
		fileContainer->SetSelectedFile(nullptr);
	}
	
	tabsContainer->DestroyChildren();
	Hide();
	if (auto emptyWindow = FindWindowById(+GUI::ControlID::EmptyWindow))
		emptyWindow->Show();

}

void Tabs::Select()
{
	auto mainCode = FindWindowById(+GUI::ControlID::MainCode);

	for (auto &children : tabsContainer->GetChildren())
		children->Refresh();

	auto codeContainer = ((CodeContainer *)FindWindowByName(ProjectSettings::Get().GetCurrentlyFileOpen() + "_codeContainer"));
	auto imageContainer = ((wxStaticBitmap *)FindWindowByLabel(ProjectSettings::Get().GetCurrentlyFileOpen() + "_imageContainer"));
	auto statusBar = ((StatusBar *)FindWindowById(+GUI::ControlID::StatusBar));

	for (auto &&other_ct : mainCode->GetChildren())
	{
		if (other_ct->GetId() != +GUI::ControlID::Tabs)
			other_ct->Hide();
	}

	if (codeContainer)
	{
		codeContainer->Show();
		statusBar->UpdateComponents(ProjectSettings::Get().GetCurrentlyFileOpen(), "text", codeContainer->currentLanguage->name);
	}

	if (imageContainer)
	{
		imageContainer->Show();
		statusBar->UpdateComponents(ProjectSettings::Get().GetCurrentlyFileOpen(), "image", "img");
	}

	mainCode->GetSizer()->Layout();
	mainCode->Update();
}

void Tabs::OnTabClick(wxMouseEvent &event)
{
	auto this_tab = ((Tabs *)event.GetEventObject());
	wxString tab_path = this_tab->GetName();
	if (tab_path == ProjectSettings::Get().GetCurrentlyFileOpen())
		return;
	projectSettings.SetCurrentlyFileOpen(tab_path);
	Select();
}

void Tabs::OnCloseTab(wxMouseEvent &event)
{
	wxObject *obj = event.GetEventObject();
	auto this_tab = ((wxWindow *)obj)->GetGrandParent();
	if (this_tab)
		Tabs::Close(this_tab, this_tab->GetName());
}

void Tabs::OnMenu(wxMouseEvent &WXUNUSED(event))
{
	wxMenu *tabsMenu = new wxMenu;
	tabsMenu->Append(+Event::File::CloseAll, _("&Close All"));
	tabsMenu->Append(wxID_ANY, _("&First Tab"));
	tabsMenu->Append(wxID_ANY, _("&Last Tab"));
	tabsMenu->Append(wxID_ANY, _("&Close Saved"));
	PopupMenu(tabsMenu);
}

void Tabs::OnEnterComp(wxMouseEvent &event)
{
	auto target = ((wxWindow *)event.GetEventObject());
	auto codeContainer = ((wxStyledTextCtrl *)FindWindowByName(target->GetParent()->GetName() + "_codeContainer"));
	if (target && codeContainer)
	{
		auto icon = ((wxStaticBitmap *)target->GetChildren()[2]);
		if (icon)
		{
			icon = ((wxStaticBitmap *)target->GetChildren()[2]);
			if (icon)
			{
				auto codeEditor = ((wxStyledTextCtrl *)codeContainer->GetChildren()[0]);
				if (codeEditor->GetModify())
				{
					icon->Show();
					icon->SetBitmap(wxBitmapBundle::FromBitmap(wxBitmap(iconsDir + "close.png", wxBITMAP_TYPE_PNG)));
				}
			}
		}
	}
}

void Tabs::OnLeaveComp(wxMouseEvent &event)
{
	auto target = ((wxWindow *)event.GetEventObject());
	if (target)
	{
		auto codeContainer = ((wxWindow *)FindWindowByName(target->GetParent()->GetName() + "_codeContainer"));
		wxStaticBitmap *icon = ((wxStaticBitmap *)target->GetChildren()[2]);
		if (codeContainer)
		{
			auto codeEditor = ((wxStyledTextCtrl *)codeContainer->GetChildren()[0]);
			if (codeEditor->GetModify())
			{
				icon->SetBitmap(wxBitmapBundle::FromBitmap(wxBitmap(iconsDir + "white_circle.png", wxBITMAP_TYPE_PNG)));
			}
			else
			{
				icon->SetBitmap(wxBitmapBundle::FromBitmap(wxBitmap(iconsDir + "close.png", wxBITMAP_TYPE_PNG)));
			}
		}
	}
}

void Tabs::OnPaint(wxPaintEvent &WXUNUSED(event))
{
	wxPaintDC dc(this);
	dc.SetBrush(wxColor(Theme["border"].template get<std::string>()));
	dc.SetPen(wxPen(wxColor(Theme["border"].template get<std::string>()), 0.20));
	dc.DrawLine(0, GetSize().GetHeight() - 1, GetSize().GetWidth(), GetSize().GetHeight() - 1);
}

void Tabs::OnTabPaint(wxPaintEvent &event)
{
	auto target = ((wxPanel *)event.GetEventObject());
	wxPaintDC dc(target);
	wxGraphicsContext *gc = wxGraphicsContext::Create(dc);
	if (!gc)
		return;

	if (ProjectSettings::Get().GetCurrentlyFileOpen() == target->GetName())
	{
		auto background = Theme["highlight"].template get<std::string>();
		gc->SetPen(wxColor(background));
		gc->SetBrush(wxColor(background));
		gc->DrawRoundedRectangle(0.0, 0.0, static_cast<double>(target->GetSize().GetWidth()) - 1.0, 1.0, 10);
	}
	else
	{
		auto background = Theme["main"].template get<std::string>();
		gc->SetPen(wxColor(background));
		gc->SetBrush(wxColor(background));
		gc->DrawRoundedRectangle(0.0, 0.0, static_cast<double>(target->GetSize().GetWidth()), static_cast<double>(target->GetSize().GetHeight()), 10);
	}

	delete gc;
}

void Tabs::OnPreviousTab()
{
	for (auto &&tab : tabsContainer->GetChildren())
		if (tab->GetName() == ProjectSettings::Get().GetCurrentlyFileOpen())
		{
			auto prevTab = tab->GetPrevSibling();
			if (prevTab)
			{
				ProjectSettings::Get().GetCurrentlyFileOpen() = prevTab->GetName();
				Select();
			}
		}
}

void Tabs::OnNextTab()
{
	for (auto &&tab : tabsContainer->GetChildren())
		if (tab->GetName() == ProjectSettings::Get().GetCurrentlyFileOpen())
		{
			auto nextTab = tab->GetNextSibling();
			if (nextTab)
			{
				projectSettings.SetCurrentlyFileOpen(nextTab->GetName());
				Select();
			}
		}
}