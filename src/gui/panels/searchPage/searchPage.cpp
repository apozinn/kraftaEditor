#include "searchPage.hpp"

#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/textfile.h>

#include <ui/ids.hpp>
#include <projectSettings/projectSettings.hpp>
#include <gui/panels/filesTree/filesTree.hpp>
#include <themesManager/themesManager.hpp>

SearchPage::SearchPage(wxWindow* parent)
    : wxPanel(parent, +GUI::ControlID::SearchPage)
{
    SetWorkspaceRoot(ProjectSettings::Get().GetProjectPath());
    SetBackgroundColour(ThemesManager::Get().GetColor("main"));

    auto* root = new wxBoxSizer(wxVERTICAL);

    m_searchCtrl = new wxTextCtrl(
        this,
        wxID_ANY,
        wxEmptyString,
        wxDefaultPosition,
        wxDefaultSize,
        wxTE_PROCESS_ENTER
    );

    m_replaceCtrl = new wxTextCtrl(this, wxID_ANY);

    m_caseCheck  = new wxCheckBox(this, wxID_ANY, "Case sensitive");
    m_wordCheck  = new wxCheckBox(this, wxID_ANY, "Whole word");
    m_regexCheck = new wxCheckBox(this, wxID_ANY, "Regex");

    for (auto* cb : { m_caseCheck, m_wordCheck, m_regexCheck })
        cb->SetMinSize(wxSize(-1, 22));

    auto* btnSearch     = new wxButton(this, wxID_ANY, "Search");
    auto* btnReplace    = new wxButton(this, wxID_ANY, "Replace");
    auto* btnReplaceAll = new wxButton(this, wxID_ANY, "Replace All");

    btnSearch->Bind(wxEVT_BUTTON, &SearchPage::OnSearch, this);
    btnReplace->Bind(wxEVT_BUTTON, &SearchPage::OnReplace, this);
    btnReplaceAll->Bind(wxEVT_BUTTON, &SearchPage::OnReplaceAll, this);
    m_searchCtrl->Bind(wxEVT_TEXT_ENTER, &SearchPage::OnSearchEnter, this);

    root->Add(new wxStaticText(this, wxID_ANY, "Search"), 0, wxLEFT | wxTOP, 6);
    root->Add(m_searchCtrl, 0, wxEXPAND | wxALL, 6);

    root->Add(new wxStaticText(this, wxID_ANY, "Replace"), 0, wxLEFT, 6);
    root->Add(m_replaceCtrl, 0, wxEXPAND | wxALL, 6);

    auto* opts = new wxBoxSizer(wxHORIZONTAL);
    opts->Add(m_caseCheck);
    opts->Add(m_wordCheck, 0, wxLEFT, 8);
    opts->Add(m_regexCheck, 0, wxLEFT, 8);

    root->Add(opts, 0, wxLEFT | wxBOTTOM, 6);

    auto* actions = new wxBoxSizer(wxHORIZONTAL);
    actions->Add(btnSearch);
    actions->Add(btnReplace, 0, wxLEFT, 6);
    actions->Add(btnReplaceAll, 0, wxLEFT, 6);

    root->Add(actions, 0, wxLEFT | wxBOTTOM, 6);

    m_results = new wxListCtrl(
        this,
        wxID_ANY,
        wxDefaultPosition,
        wxDefaultSize,
        wxLC_REPORT | wxLC_SINGLE_SEL
    );

    m_results->InsertColumn(0, "File", wxLIST_FORMAT_LEFT, 280);
    m_results->InsertColumn(1, "Line", wxLIST_FORMAT_LEFT, 60);
    m_results->InsertColumn(2, "Preview", wxLIST_FORMAT_LEFT, 600);

    m_results->Bind(
        wxEVT_LIST_ITEM_ACTIVATED,
        &SearchPage::OnResultActivated,
        this
    );

    root->Add(m_results, 1, wxEXPAND | wxALL, 6);

    SetSizer(root);
}

void SearchPage::SetWorkspaceRoot(const wxString& path)
{
    m_workspaceRoot = path;
}

bool SearchPage::ShouldIgnoreDirectory(const wxString& name) const
{
    if (name.StartsWith("."))
        return true;

    static const std::set<wxString> ignored = {
        "node_modules",
        "build",
        "dist",
        "out",
        ".cache",
        ".next"
    };

    return ignored.contains(name);
}

void SearchPage::PerformSearch()
{
    m_results->DeleteAllItems();

    if (m_workspaceRoot.IsEmpty())
        m_workspaceRoot = ProjectSettings::Get().GetProjectPath();

    const wxString needle = m_searchCtrl->GetValue();
    if (needle.IsEmpty())
        return;

    SearchDirectory(m_workspaceRoot);
}

void SearchPage::SearchDirectory(const wxString& path)
{
    wxDir dir(path);
    if (!dir.IsOpened())
        return;

    wxString name;
    for (bool cont = dir.GetFirst(&name); cont; cont = dir.GetNext(&name))
    {
        wxFileName fn(path, name);

        if (wxDirExists(fn.GetFullPath()))
        {
            if (!ShouldIgnoreDirectory(name))
                SearchDirectory(fn.GetFullPath());
            continue;
        }

        wxTextFile file(fn.GetFullPath());
        if (!file.Open())
            continue;

        wxString needle = m_searchCtrl->GetValue();
        if (!m_caseCheck->IsChecked())
            needle.MakeLower();

        for (size_t i = 0; i < file.GetLineCount(); ++i)
        {
            const wxString& line = file.GetLine(i);
            wxString hay = m_caseCheck->IsChecked() ? line : line.Lower();

            int pos = hay.Find(needle);
            if (pos == wxNOT_FOUND)
                continue;

            long index = m_results->InsertItem(
                m_results->GetItemCount(),
                fn.GetFullPath()
            );

            m_results->SetItem(index, 1, wxString::Format("%zu", i + 1));
            m_results->SetItem(index, 2, line.Mid(pos, needle.Length()));
        }
    }
}

void SearchPage::OnSearch(wxCommandEvent&)
{
    PerformSearch();
}

void SearchPage::OnSearchEnter(wxCommandEvent&)
{
    PerformSearch();
}

void SearchPage::OnReplace(wxCommandEvent&)
{
    long sel = m_results->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (sel == -1)
        return;

    const wxString file = m_results->GetItemText(sel, 0);
    const int line = wxAtoi(m_results->GetItemText(sel, 1)) - 1;

    ReplaceMatch(
        file,
        line,
        0,
        m_searchCtrl->GetValue().Length()
    );
}

void SearchPage::OnReplaceAll(wxCommandEvent&)
{
    ReplaceAllMatches();
}

void SearchPage::ReplaceMatch(
    const wxString& filePath,
    int line,
    int,
    int
)
{
    wxTextFile file(filePath);
    if (!file.Open())
        return;

    wxString text = file.GetLine(line);
    text.Replace(
        m_searchCtrl->GetValue(),
        m_replaceCtrl->GetValue()
    );

    file.RemoveLine(line);
    file.InsertLine(text, line);
    file.Write();
    file.Close();

    PerformSearch();
}

void SearchPage::ReplaceAllMatches()
{
    for (long i = 0; i < m_results->GetItemCount(); ++i)
    {
        const wxString file = m_results->GetItemText(i, 0);
        const int line = wxAtoi(m_results->GetItemText(i, 1)) - 1;

        ReplaceMatch(
            file,
            line,
            0,
            m_searchCtrl->GetValue().Length()
        );
    }
}

void SearchPage::OnResultActivated(wxListEvent& event)
{
    const wxString file = event.GetText();
    const int line =
        wxAtoi(m_results->GetItemText(event.GetIndex(), 1)) - 1;

    auto* filesTree = static_cast<FilesTree*>(
        wxApp::GetMainTopWindow()->FindWindowById(
            +GUI::ControlID::FilesTree
        )
    );

    if (filesTree)
        filesTree->OpenFile(file, line);
}
