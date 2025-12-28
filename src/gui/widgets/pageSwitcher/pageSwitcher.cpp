#include "pageSwitcher.hpp"

#include <appPaths/appPaths.hpp>
#include <themesManager/themesManager.hpp>
#include <ui/ids.hpp>

wxDEFINE_EVENT(EVT_PAGE_SWITCH_FILES, wxCommandEvent);
wxDEFINE_EVENT(EVT_PAGE_SWITCH_SEARCH, wxCommandEvent);

namespace
{
    void ToggleSiblingPages(wxWindow *parent, int showId, int ignoreId)
    {
        auto *sizer = parent->GetSizer();

        for (wxWindow *child : parent->GetChildren())
        {
            const int id = child->GetId();

            if (id == ignoreId)
                continue;

            child->Show(id == showId);
        }

        if (sizer)
            sizer->Layout();
    }
}

PageSwitcher::PageSwitcher(wxWindow *parent)
    : wxPanel(parent, +GUI::ControlID::PageSwitcher),
      m_bg_normal(ThemesManager::Get().GetColor("main")),
      m_bg_hover(ThemesManager::Get().GetColor("selectedFile")),
      m_active_line(ThemesManager::Get().GetColor("tint"))
{
    SetBackgroundColour(m_bg_normal);

    m_sizer = new wxBoxSizer(wxHORIZONTAL);

    CreateFilesPage();
    CreateSearchPage();

    m_files_panel->Bind(wxEVT_PAINT, &PageSwitcher::OnPaintPanel, this);
    m_search_panel->Bind(wxEVT_PAINT, &PageSwitcher::OnPaintPanel, this);

    SetSizerAndFit(m_sizer);

    SetActive(m_files_panel);
}

void PageSwitcher::CreateFilesPage()
{
    m_files_panel = new wxPanel(this, +GUI::ControlID::FilesPage);
    auto *sizer = new wxBoxSizer(wxHORIZONTAL);

    const wxBitmap bmp(
        ApplicationPaths::GetIconPath("pageSwitcherFilesPage.png"),
        wxBITMAP_TYPE_PNG);

    if (bmp.IsOk())
    {
        auto *icon = new wxStaticBitmap(m_files_panel, wxID_ANY, bmp);
        sizer->Add(icon, 0, wxALIGN_CENTER | wxALL, 6);

        icon->Bind(wxEVT_LEFT_DOWN, &PageSwitcher::OnFilesClicked, this);
        icon->Bind(wxEVT_ENTER_WINDOW, &PageSwitcher::OnMouseEnter, this);
        icon->Bind(wxEVT_LEAVE_WINDOW, &PageSwitcher::OnMouseLeave, this);
    }

    m_files_panel->SetSizer(sizer);
    m_files_panel->SetBackgroundColour(m_bg_normal);

    m_files_panel->Bind(wxEVT_LEFT_DOWN, &PageSwitcher::OnFilesClicked, this);
    m_files_panel->Bind(wxEVT_ENTER_WINDOW, &PageSwitcher::OnMouseEnter, this);
    m_files_panel->Bind(wxEVT_LEAVE_WINDOW, &PageSwitcher::OnMouseLeave, this);

    m_sizer->Add(m_files_panel, 0, wxALL, 4);
}

void PageSwitcher::CreateSearchPage()
{
    m_search_panel = new wxPanel(this, +GUI::ControlID::PageSwitcherSearchPage);
    auto *sizer = new wxBoxSizer(wxHORIZONTAL);

    const wxBitmap bmp(
        ApplicationPaths::GetIconPath("pageSwitcherSearchPage.png"),
        wxBITMAP_TYPE_PNG);

    if (bmp.IsOk())
    {
        auto *icon = new wxStaticBitmap(m_search_panel, wxID_ANY, bmp);
        sizer->Add(icon, 0, wxALIGN_CENTER | wxALL, 6);

        icon->Bind(wxEVT_LEFT_DOWN, &PageSwitcher::OnSearchClicked, this);
        icon->Bind(wxEVT_ENTER_WINDOW, &PageSwitcher::OnMouseEnter, this);
        icon->Bind(wxEVT_LEAVE_WINDOW, &PageSwitcher::OnMouseLeave, this);
    }

    m_search_panel->SetSizer(sizer);
    m_search_panel->SetBackgroundColour(m_bg_normal);

    m_search_panel->Bind(wxEVT_LEFT_DOWN, &PageSwitcher::OnSearchClicked, this);
    m_search_panel->Bind(wxEVT_ENTER_WINDOW, &PageSwitcher::OnMouseEnter, this);
    m_search_panel->Bind(wxEVT_LEAVE_WINDOW, &PageSwitcher::OnMouseLeave, this);

    m_sizer->Add(m_search_panel, 0, wxALL, 4);
}

void PageSwitcher::OnFilesClicked(wxMouseEvent &)
{
    SetActive(m_files_panel);

    ToggleSiblingPages(
        GetParent(),
        +GUI::ControlID::FilesTree,
        +GUI::ControlID::PageSwitcher);

    wxCommandEvent evt(EVT_PAGE_SWITCH_FILES);
    wxPostEvent(this, evt);
}

void PageSwitcher::OnSearchClicked(wxMouseEvent &)
{
    SetActive(m_search_panel);

    ToggleSiblingPages(
        GetParent(),
        +GUI::ControlID::SearchPage,
        +GUI::ControlID::PageSwitcher);

    wxCommandEvent evt(EVT_PAGE_SWITCH_SEARCH);
    wxPostEvent(this, evt);
}

void PageSwitcher::OnMouseEnter(wxMouseEvent &event)
{
    wxWindow *obj = static_cast<wxWindow *>(event.GetEventObject());
    m_hover_panel = dynamic_cast<wxPanel *>(obj->GetParent() ? obj->GetParent() : obj);
    UpdateVisuals();
}

void PageSwitcher::OnMouseLeave(wxMouseEvent &)
{
    m_hover_panel = nullptr;
    UpdateVisuals();
}

void PageSwitcher::SetActive(wxPanel *panel)
{
    if (m_active_panel == panel)
        return;

    m_active_panel = panel;
    UpdateVisuals();
}

void PageSwitcher::OnPaintPanel(wxPaintEvent &event)
{
    auto *panel = static_cast<wxPanel *>(event.GetEventObject());
    wxPaintDC dc(panel);

    dc.SetBackground(wxBrush(panel->GetBackgroundColour()));
    dc.Clear();

    if (panel != m_active_panel)
        return;

    const wxSize size = panel->GetClientSize();

    constexpr int lineHeight = 3;
    constexpr int radius = 4;
    constexpr int margin = 6;

    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.SetBrush(wxBrush(m_active_line));

    dc.DrawRoundedRectangle(
        margin,
        size.y - lineHeight - 1,
        size.x - margin * 2,
        lineHeight,
        radius);
}

void PageSwitcher::UpdateVisuals()
{
    for (wxPanel *panel : {m_files_panel, m_search_panel})
    {
        panel->SetBackgroundColour(
            panel == m_hover_panel ? m_bg_hover : m_bg_normal);

        panel->Refresh(false);
    }
}