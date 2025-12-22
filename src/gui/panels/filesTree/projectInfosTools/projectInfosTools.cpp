#include "projectInfosTools.hpp"
#include <appPaths/appPaths.hpp>
#include <ui/ids.hpp>
#include <gui/panels/filesTree/filesTree.hpp>

ProjectInfosTools::ProjectInfosTools(wxPanel *parent, wxWindowID ID) : wxPanel(parent, ID)
{
    SetBackgroundColour(wxTransparentColor);

    m_sizer->AddStretchSpacer(1);

    CreateCollapseButton();

    SetSizerAndFit(m_sizer);

    Bind(wxEVT_ENTER_WINDOW, &ProjectInfosTools::OnHoverEnter, this);
    Bind(wxEVT_LEAVE_WINDOW, &ProjectInfosTools::OnHoverLeave, this);
    m_collapse_button->Bind(wxEVT_ENTER_WINDOW, &ProjectInfosTools::OnHoverEnter, this);
}

void ProjectInfosTools::CreateCollapseButton()
{
    wxString arrowPath = ApplicationPaths::GetIconPath("collapse.png");
    if (!arrowPath.IsEmpty())
    {
        wxBitmap arrowBitmap(arrowPath, wxBITMAP_TYPE_PNG);
        if (arrowBitmap.IsOk())
        {
            m_collapse_button = new wxStaticBitmap(
                this,
                +GUI::ControlID::ProjectToolsArrow,
                arrowBitmap);
            m_sizer->Add(m_collapse_button, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
            m_collapse_button->Bind(wxEVT_LEFT_UP, &ProjectInfosTools::OnCollapseButtonClick, this);
        }
    }
}

void ProjectInfosTools::OnCollapseButtonClick(wxMouseEvent &event)
{
    if (!m_projectFilesContainer || !m_filesTree)
    {
        m_projectFilesContainer = FindWindowById(+GUI::ControlID::ProjectFilesContainer);
        m_filesTree = ((FilesTree *)FindWindowById(+GUI::ControlID::FilesTree));
    }

    if (m_projectFilesContainer && m_filesTree)
    {
        m_projectFilesContainer->CallForEachChild(
            [&, this](wxWindow *child)
            {
                if (child->GetLabel().Contains("_dir_container"))
                {
                    if (child->GetChildren().empty())
                        return;

                    if (child->GetChildren()[1]->IsShown())
                        m_filesTree->ToggleDirVisibility(child->GetName());
                }
            });
    }
}

void ProjectInfosTools::OnHoverEnter(wxMouseEvent &event)
{
    Show();
}

void ProjectInfosTools::OnHoverLeave(wxMouseEvent &event)
{
    Hide();
}