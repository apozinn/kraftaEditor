#include "quickOpen.hpp"
#include "ui/ids.hpp"
#include <wx/splitter.h>
#include <projectSettings/projectSettings.hpp>
#include "gui/panels/filesTree/filesTree.hpp"

QuickOpen::QuickOpen(wxFrame *parent)
    : wxPanel(parent, +GUI::ControlID::QuickOpen, wxPoint(parent->GetSize().GetWidth() / 2 - 200, 50), wxSize(450, 200))
{
    SetBackgroundColour(wxColor(Theme.GetColor("colorThree")));
    SetFocus();

    m_topContainer = new wxPanel(this);

    CreateSearchBar();
    m_topContainer->SetSizerAndFit(topContainerSizer);
    m_sizer->Add(m_topContainer, 0, wxEXPAND);

    CreateScrolledContainer();
    scrolledContainer->SetSizerAndFit(scrolledContainerSizer);
    m_sizer->Add(scrolledContainer, 1, wxEXPAND | wxALL, 5);

    SetSizerAndFit(m_sizer);

    SetAccelerators();

    wxSize size = wxSize(450, 200);
    SetMinSize(size);
    SetSize(size);
}

void QuickOpen::SetAccelerators()
{
    wxAcceleratorEntry entries[4];
    entries[0].Set(wxACCEL_NORMAL, WXK_ESCAPE, +Event::QuickOpen::Exit);
    entries[1].Set(wxACCEL_NORMAL, WXK_UP, +Event::QuickOpen::Up);
    entries[2].Set(wxACCEL_NORMAL, WXK_DOWN, +Event::QuickOpen::Down);
    entries[3].Set(wxACCEL_NORMAL, WXK_RETURN, +Event::QuickOpen::Select);
    wxAcceleratorTable accel(4, entries);
    SetAcceleratorTable(accel);
}

void QuickOpen::CreateSearchBar()
{
    m_searchBar = new wxTextCtrl(m_topContainer, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);

    m_searchBar->SetMaxSize(wxSize(450, 20));
    m_searchBar->SetBackgroundColour(wxColor(Theme.GetColor("secondary")));
    m_searchBar->SetFocus();
    m_searchBar->Bind(wxEVT_TEXT, &QuickOpen::OnSearchBarChange, this);

    topContainerSizer->Add(m_searchBar, 1, wxEXPAND | wxALL, 5);
}

void QuickOpen::OnSearchBarChange(wxCommandEvent &WXUNUSED(event))
{
    wxString search = m_searchBar->GetValue();
    if (search.IsEmpty())
        return;
}

void QuickOpen::CreateScrolledContainer()
{
    scrolledContainer = new wxScrolled<wxPanel>(this);
    scrolledContainer->SetName("QuickOpenScrolledContainer");

    int i = 0;
    for (auto const &entry : std::filesystem::directory_iterator{ProjectSettings::Get().GetProjectPath().ToStdString()})
    {
        if (i == 0)
            m_selectedFilePath = entry.path();
        if (entry.is_directory())
            continue;
        CreateFileContainer(QuickOpenFileStruct(entry.path().filename().string(), entry.path()));
        i++;
    }

    scrolledContainer->FitInside();
    scrolledContainer->SetScrollRate(20, 20);
}

void QuickOpen::CreateFileContainer(QuickOpenFileStruct file)
{
    wxPanel *fileContainer = new wxPanel(scrolledContainer);
    fileContainer->SetName("QuickOpenFileContainer_" + file.path);
    fileContainer->SetLabel(file.path);

    wxBoxSizer *fileContainerSizer = new wxBoxSizer(wxHORIZONTAL);

    wxStaticText *fileName = new wxStaticText(fileContainer, wxID_ANY, file.name, wxDefaultPosition, wxDefaultSize);
    fileContainerSizer->Add(fileName, 0, wxEXPAND | wxALL, 5);

    wxStaticText *filePath = new wxStaticText(fileContainer, wxID_ANY, file.path, wxDefaultPosition, wxDefaultSize);
    wxFont filePath_font(7, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    filePath_font.SetFaceName(wxT("Monospace"));
    filePath->SetFont(filePath_font);
    filePath->SetForegroundColour(wxColor(Theme.GetColor("secondaryText")));

    fileContainerSizer->Add(filePath, 0, wxTOP | wxLEFT, 10);

    fileContainer->SetSizerAndFit(fileContainerSizer);

    if (file.path == m_selectedFilePath)
        fileContainer->SetBackgroundColour(wxColor(Theme.GetColor("highlight")));

    scrolledContainerSizer->Add(fileContainer, 0, wxEXPAND);

    fileContainer->CallForEachChild(
        [&, this](wxWindow *child)
        {
        if(child->GetName() == "QuickOpenFileContainer_" + file.path) return;
        child->SetName(wxString(file.path));
        child->Bind(wxEVT_LEFT_DOWN, &QuickOpen::OnMouseLeftClick, this);
        child->Bind(wxEVT_ENTER_WINDOW, &QuickOpen::OnMouseEnter, this);
        child->Bind(wxEVT_LEAVE_WINDOW, &QuickOpen::OnMouseLeave, this); });
}

void QuickOpen::ChangeCurrentSelectedFile(const std::string &direction)
{
    auto currentSelectedFile = wxWindow::FindWindowByName("QuickOpenFileContainer_" + m_selectedFilePath);
    if (!currentSelectedFile)
        return;

    auto changeFile = [&, this](wxWindow *target)
    {
        target->SetBackgroundColour(Theme.GetColor("highlight"));
        target->Refresh();

        currentSelectedFile->SetBackgroundColour(wxTransparentColour);
        currentSelectedFile->Refresh();

        m_selectedFilePath = target->GetLabel();
    };

    if (direction == "up")
    {
        auto prevFile = currentSelectedFile->GetPrevSibling();
        auto nextFile = scrolledContainer->GetChildren().GetLast().GetData();

        if (prevFile)
            changeFile(prevFile);
        else if (nextFile)
            changeFile(nextFile);
    }
    else
    {
        auto prevFile = scrolledContainer->GetChildren().GetFirst().GetData();
        auto nextFile = currentSelectedFile->GetNextSibling();

        if (nextFile)
            changeFile(nextFile);
        else if (prevFile)
            changeFile(prevFile);
    }
}

void QuickOpen::OnKeyboardUp(wxCommandEvent &event)
{
    ChangeCurrentSelectedFile("up");
}

void QuickOpen::OnKeyboardDown(wxCommandEvent &event)
{
    ChangeCurrentSelectedFile("down");
}

void QuickOpen::OnKeyboardEnter(wxCommandEvent &event)
{
    if (m_selectedFilePath.empty())
        return;

    FilesTree *filesTree = (FilesTree *)wxApp::GetMainTopWindow()->FindWindowById(+GUI::ControlID::FilesTree);
    if (filesTree)
        filesTree->OpenFile(m_selectedFilePath);

    wxCommandEvent evt(wxEVT_MENU, +Event::QuickOpen::Exit);
    wxPostEvent(this, evt);
}

void QuickOpen::OnMouseLeftClick(wxMouseEvent &event)
{
    const wxString path = ((wxStaticText *)event.GetEventObject())->GetName();
    if (path.IsEmpty())
        return;

    FilesTree *filesTree = (FilesTree *)wxApp::GetMainTopWindow()->FindWindowById(+GUI::ControlID::FilesTree);
    if (filesTree)
        filesTree->OpenFile(path);

    wxCommandEvent evt(wxEVT_MENU, +Event::QuickOpen::Exit);
    wxPostEvent(this, evt);
}

void QuickOpen::OnMouseEnter(wxMouseEvent &event)
{
    auto target = ((wxWindow *)event.GetEventObject());
    if (!target)
        return;

    auto parent = target->GetParent();

    if (!parent)
        return;

    if (parent->GetName() == "QuickOpenScrolledContainer")
        return;

    parent->SetBackgroundColour(Theme.GetColor("highlight"));
    parent->Refresh();
}

void QuickOpen::OnMouseLeave(wxMouseEvent &event)
{
    auto target = ((wxWindow *)event.GetEventObject());
    if (!target)
        return;

    auto parent = target->GetParent();
    if (!parent)
        return;

    parent->SetBackgroundColour(wxTransparentColour);
    parent->Refresh();
}

void QuickOpen::Close(wxCommandEvent &WXUNUSED(event))
{
    wxWindow *main = wxApp::GetMainTopWindow();
    Hide();
    Destroy();
    if (main)
        main->CallAfter([main]
                        { main->SetFocus(); });
}