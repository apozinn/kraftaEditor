#include "controlPanel.hpp"
#include "ui/ids.hpp"
#include <wx/splitter.h>

wxBEGIN_EVENT_TABLE(ControlPanel, wxPanel)
    EVT_MENU(+Event::ControlPanel::Exit, ControlPanel::Close)
        EVT_MENU(+Event::ControlPanel::Up, ControlPanel::UpSelection)
            EVT_MENU(+Event::ControlPanel::Down, ControlPanel::DownSelection)
                EVT_MENU(+Event::ControlPanel::Select, ControlPanel::OnKeyBoardSelect)
                    wxEND_EVENT_TABLE()

                        ControlPanel::ControlPanel(wxFrame *parent, wxWindowID ID) : wxPanel(parent, ID, wxPoint(parent->GetSize().GetWidth() / 2 - 225, 50), wxSize(450, 200))
{
    SetBackgroundColour(wxColor(Theme["main"].template get<std::string>()));
    SetFocus();

    wxPanel *topContainer = new wxPanel(this);
    wxBoxSizer *topContainerSizer = new wxBoxSizer(wxHORIZONTAL);

    wxVector<wxBitmap> bitmaps;
    bitmaps.push_back(wxBitmap(iconsDir + "search_gray.png", wxBITMAP_TYPE_PNG));
    wxStaticBitmap *search_icon = new wxStaticBitmap(topContainer, wxID_ANY, wxBitmapBundle::FromBitmaps(bitmaps));
    topContainerSizer->Add(search_icon, 0, wxEXPAND | wxALL, 5);

    searchInput = new wxTextCtrl(topContainer, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
    searchInput->SetMaxSize(wxSize(450, 20));
    searchInput->SetBackgroundColour(wxColor(Theme["main"].template get<std::string>()));
    searchInput->SetFocus();
    searchInput->Bind(wxEVT_TEXT, &ControlPanel::SearchInputModified, this);
    topContainerSizer->Add(searchInput, 1, wxEXPAND | wxTOP, 7);

    topContainer->SetSizerAndFit(topContainerSizer);
    sizer->Add(topContainer, 0, wxEXPAND);

    menusContainer = new wxScrolled<wxPanel>(this);
    wxBoxSizer *menuContainerSizer = new wxBoxSizer(wxVERTICAL);

    for (const auto &ctrl_menu : menus)
    {
        wxPanel *menu = new wxPanel(menusContainer);
        menu->SetLabel(std::to_string(ctrl_menu.ID));
        if (ctrl_menu.ID == 1)
        {
            menu->SetBackgroundColour(wxColor(Theme["selectedFile"].template get<std::string>()));
            selectedMenu = menu;
        }
        wxBoxSizer *menuSizer = new wxBoxSizer(wxHORIZONTAL);

        wxStaticText *menuName = new wxStaticText(menu, wxID_ANY, ctrl_menu.name);
        menuName->Bind(wxEVT_LEFT_UP, &ControlPanel::OnClickSelect, this);
        menuName->SetName(std::to_string(ctrl_menu.ID));
        wxFont menuName_font(10, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
        menuName_font.SetFaceName(wxT("Monospace"));
        menuName->SetFont(menuName_font);
        menuSizer->Add(menuName, 1, wxEXPAND | wxALL, 5);

        wxStaticText *menuShortkut = new wxStaticText(menu, wxID_ANY, ctrl_menu.shortkut);
        wxFont menuShortkut_font(7, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
        menuShortkut_font.SetFaceName(wxT("Monospace"));
        menuShortkut->SetFont(menuShortkut_font);
        menuSizer->Add(menuShortkut, 0, wxEXPAND | wxTOP, 8);

        menu->SetSizerAndFit(menuSizer);
        menuContainerSizer->Add(menu, 0, wxEXPAND | wxALL, 3);
    }

    menusContainer->SetSizerAndFit(menuContainerSizer);
    menusContainer->FitInside();
    menusContainer->SetScrollRate(20, 20);
    sizer->Add(menusContainer, 1, wxEXPAND);

    wxAcceleratorEntry entries[4];
    entries[0].Set(wxACCEL_NORMAL, WXK_ESCAPE, +Event::ControlPanel::Exit);
    entries[1].Set(wxACCEL_NORMAL, WXK_UP, +Event::ControlPanel::Up);
    entries[2].Set(wxACCEL_NORMAL, WXK_DOWN, +Event::ControlPanel::Down);
    entries[3].Set(wxACCEL_NORMAL, WXK_RETURN, +Event::ControlPanel::Select);
    wxAcceleratorTable accel(4, entries);
    SetAcceleratorTable(accel);

    SetSizerAndFit(sizer);
    SetMinSize(wxSize(450, 250));
    SetSize(wxSize(450, 250));
}

void ControlPanel::Close(wxCommandEvent &WXUNUSED(event))
{
    FindWindowById(+GUI::ControlID::StatusBar)->SetFocus();
    Destroy();
}

void ControlPanel::UpSelection(wxCommandEvent &WXUNUSED(event))
{
    bool finded = false;
    for (auto &&menu : menusContainer->GetChildren())
    {
        if (finded)
            return;
        if (menu == selectedMenu)
        {
            auto nextMenu = menu->GetPrevSibling();
            if (!nextMenu)
            {
                nextMenu = menusContainer->GetChildren()[menusContainer->GetChildren().GetCount() - 1];
            }

            if (nextMenu != menu)
            {
                nextMenu->SetBackgroundColour(wxColor(Theme["selectedFile"].template get<std::string>()));
                menu->SetBackgroundColour(wxColor(Theme["main"].template get<std::string>()));
                selectedMenu = ((wxPanel *)nextMenu);
                finded = true;

                nextMenu->Refresh();
            }
        }
        else
        {
            menu->SetBackgroundColour(wxColor(Theme["main"].template get<std::string>()));
        }
        menu->Refresh();
    }
}

void ControlPanel::DownSelection(wxCommandEvent &WXUNUSED(event))
{
    bool finded = false;

    for (auto &&menu : menusContainer->GetChildren())
    {
        if (finded)
            return;
        if (menu == selectedMenu)
        {

            auto nextMenu = menu->GetNextSibling();
            if (!nextMenu)
            {
                nextMenu = menusContainer->GetChildren()[0];
            }

            nextMenu->SetBackgroundColour(wxColor(Theme["selectedFile"].template get<std::string>()));
            menu->SetBackgroundColour(wxColor(Theme["main"].template get<std::string>()));
            selectedMenu = ((wxPanel *)nextMenu);
            finded = true;

            menu->Refresh();
            nextMenu->Refresh();
        }
    }
}

void ControlPanel::OnKeyBoardSelect(wxCommandEvent &WXUNUSED(event))
{
    wxString id = selectedMenu->GetLabel();
    if (id.size() > 0)
        Select(id);
}

void ControlPanel::OnClickSelect(wxMouseEvent &event)
{
    wxString id = ((wxWindow *)event.GetEventObject())->GetName();
    if (id.size() > 0)
        Select(id);
}

void ControlPanel::Select(wxString id)
{
    switch (static_cast<char>(id[0]))
    {
    case '1':
    {
        ((wxSplitterWindow *)
             FindWindowById(
                 +GUI::ControlID::MainContainerSplitter))
            ->SplitHorizontally(
                FindWindowById(+GUI::ControlID::CenteredContent), FindWindowById(+GUI::ControlID::Terminal), 0);
    }
    break;
    }
    FindWindowById(+GUI::ControlID::StatusBar)->SetFocus();
    Destroy();
}

void ControlPanel::SearchInputModified(wxCommandEvent &WXUNUSED(event))
{
    for (auto &&menu : menusContainer->GetChildren())
    {
        if (menu->GetChildren()[0]->GetLabel().Lower().Find(searchInput->GetValue().Lower()) < 0)
        {
            menu->Hide();
        }
        else
            menu->Show();
    }
    menusContainer->GetSizer()->Layout();
}