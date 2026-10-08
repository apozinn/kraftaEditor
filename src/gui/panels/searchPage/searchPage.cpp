#include "searchPage.hpp"

#include "gui/panels/filesTree/filesTree.hpp"
#include "projectSettings/projectSettings.hpp"
#include "themesManager/themesManager.hpp"
#include "ui/ids.hpp"
#include "appPaths/appPaths.hpp"

#include <wx/dataview.h>
#include <wx/menu.h>
#include <wx/statbmp.h>

SearchPage::SearchPage(wxWindow* parent)
    : wxPanel(parent, +GUI::ControlID::SearchPage) {
    m_workspaceRoot = ProjectSettings::Get().GetProjectPath();
    m_searchEngine.SetWorkspaceRoot(m_workspaceRoot);

    SetupUI();
    SetupEvents();
    ApplyTheme();
}

SearchPage::~SearchPage() {
    m_searchEngine.RequestCancel();
}

void SearchPage::SetWorkspaceRoot(const wxString& path) {
    m_workspaceRoot = path;
    m_searchEngine.SetWorkspaceRoot(path);
}

void SearchPage::SetupUI() {
    auto* root = new wxBoxSizer(wxVERTICAL);

    auto* backContainer = new wxPanel(this);
    auto* backSizer = new wxBoxSizer(wxHORIZONTAL);

    wxBitmap arrowBitmap(ApplicationPaths::GetIconPath("arrow_left.png"),
                         wxBITMAP_TYPE_PNG);

    auto* arrow = new wxStaticBitmap(backContainer, wxID_ANY, arrowBitmap);
    backSizer->Add(arrow, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    auto* backLabel = new wxStaticText(backContainer, wxID_ANY, _("Back"));
    backSizer->Add(backLabel, 0, wxALIGN_CENTER_VERTICAL | wxTOP, 2);

    auto closeHandler = [this](wxMouseEvent&) {
        auto* parentSizer = GetParent()->GetSizer();
        for (wxWindow* child : GetParent()->GetChildren()) {
            const int id = child->GetId();
            if (id == +GUI::ControlID::PageSwitcher)
                continue;
            child->Show(id == +GUI::ControlID::FilesTree);
        }
        if (parentSizer)
            parentSizer->Layout();
    };

    arrow->Bind(wxEVT_LEFT_UP, closeHandler);
    backLabel->Bind(wxEVT_LEFT_UP, closeHandler);

    backContainer->SetSizerAndFit(backSizer);
    root->Add(backContainer, 0, wxEXPAND);

    root->Add(new wxStaticText(this, wxID_ANY, _("Search")),
              0, wxLEFT | wxTOP, 8);

    m_searchCtrl = new wxSearchCtrl(this, wxID_ANY, wxEmptyString,
                                    wxDefaultPosition, wxDefaultSize,
                                    wxTE_PROCESS_ENTER);
    m_searchCtrl->ShowSearchButton(false);
    m_searchCtrl->ShowCancelButton(true);
    m_searchCtrl->SetDescriptiveText(_("Search in files..."));
    root->Add(m_searchCtrl, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 8);

    root->Add(new wxStaticText(this, wxID_ANY, _("Replace")),
              0, wxLEFT | wxTOP, 8);

    m_replaceCtrl = new wxTextCtrl(this, wxID_ANY);
    root->Add(m_replaceCtrl, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 8);

    auto* opts = new wxBoxSizer(wxHORIZONTAL);
    m_caseCheck  = new wxCheckBox(this, wxID_ANY, _("Case sensitive"));
    m_wordCheck  = new wxCheckBox(this, wxID_ANY, _("Whole word"));
    m_regexCheck = new wxCheckBox(this, wxID_ANY, _("Regex"));

    for (auto* cb : {m_caseCheck, m_wordCheck, m_regexCheck})
        cb->SetMinSize(wxSize(-1, 22));

    opts->Add(m_caseCheck, 0, wxALIGN_CENTER_VERTICAL);
    opts->Add(m_wordCheck, 0, wxLEFT | wxALIGN_CENTER_VERTICAL, 8);
    opts->Add(m_regexCheck, 0, wxLEFT | wxALIGN_CENTER_VERTICAL, 8);
    root->Add(opts, 0, wxLEFT | wxRIGHT | wxBOTTOM, 8);

    auto* actions = new wxBoxSizer(wxHORIZONTAL);
    m_searchButton     = new wxButton(this, wxID_ANY, _("Search"));
    m_replaceButton    = new wxButton(this, wxID_ANY, _("Replace"));
    m_replaceAllButton = new wxButton(this, wxID_ANY, _("Replace All"));

    actions->Add(m_searchButton, 1, wxEXPAND);
    actions->Add(m_replaceButton, 1, wxEXPAND | wxLEFT, 6);
    actions->Add(m_replaceAllButton, 1, wxEXPAND | wxLEFT, 6);
    root->Add(actions, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 8);

    m_progress = new wxGauge(this, wxID_ANY, 100,
                             wxDefaultPosition, wxSize(-1, 4));
    m_progress->Hide();
    root->Add(m_progress, 0, wxEXPAND | wxLEFT | wxRIGHT, 8);

    m_statusText = new wxStaticText(this, wxID_ANY, "");
    root->Add(m_statusText, 0, wxLEFT | wxRIGHT | wxTOP | wxBOTTOM, 8);

    m_resultsView = new wxDataViewCtrl(this, wxID_ANY, wxDefaultPosition,
                                       wxDefaultSize,
                                       wxDV_ROW_LINES | wxDV_VERT_RULES);

    m_resultsModel = new SearchResultsModel();
    m_resultsView->AssociateModel(m_resultsModel.get());

    m_resultsView->AppendTextColumn(_("File"), SearchResultsModel::Col_File,
                                    wxDATAVIEW_CELL_INERT, 280,
                                    wxALIGN_LEFT,
                                    wxDATAVIEW_COL_RESIZABLE |
                                        wxDATAVIEW_COL_SORTABLE);
    m_resultsView->AppendTextColumn(_("Line"), SearchResultsModel::Col_Line,
                                    wxDATAVIEW_CELL_INERT, 60,
                                    wxALIGN_RIGHT,
                                    wxDATAVIEW_COL_RESIZABLE);
    m_resultsView->AppendTextColumn(_("Preview"), SearchResultsModel::Col_Preview,
                                    wxDATAVIEW_CELL_INERT, 600,
                                    wxALIGN_LEFT,
                                    wxDATAVIEW_COL_RESIZABLE);

    root->Add(m_resultsView, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 8);

    SetSizer(root);
}

void SearchPage::SetupEvents() {
    m_searchButton->Bind(wxEVT_BUTTON, &SearchPage::OnSearchButton, this);
    m_replaceButton->Bind(wxEVT_BUTTON, &SearchPage::OnReplace, this);
    m_replaceAllButton->Bind(wxEVT_BUTTON, &SearchPage::OnReplaceAll, this);
    m_searchCtrl->Bind(wxEVT_TEXT_ENTER, &SearchPage::OnSearchEnter, this);
    m_searchCtrl->Bind(wxEVT_SEARCHCTRL_CANCEL_BTN,
                       &SearchPage::OnCancelButton, this);
    m_resultsView->Bind(wxEVT_DATAVIEW_ITEM_ACTIVATED,
                        &SearchPage::OnResultActivated, this);
    m_resultsView->Bind(wxEVT_DATAVIEW_ITEM_CONTEXT_MENU,
                        &SearchPage::OnResultContextMenu, this);
}

void SearchPage::ApplyTheme() {
    SetBackgroundColour(ThemesManager::Get().GetColor("main"));

    const wxColour fg = ThemesManager::Get().GetColor("text");
    const wxColour bg = ThemesManager::Get().GetColor("main");

    m_searchCtrl->SetBackgroundColour(bg);
    m_searchCtrl->SetForegroundColour(fg);
    m_replaceCtrl->SetBackgroundColour(bg);
    m_replaceCtrl->SetForegroundColour(fg);

    m_statusText->SetForegroundColour(
        ThemesManager::Get().GetColor("secondaryText"));

    Refresh();
}

int SearchPage::BuildSearchFlags() const {
    int flags = Search_None;
    if (m_caseCheck->IsChecked())  flags |= Search_CaseSensitive;
    if (m_wordCheck->IsChecked())  flags |= Search_WholeWord;
    if (m_regexCheck->IsChecked()) flags |= Search_Regex;
    return flags;
}

void SearchPage::ShowStatus(const wxString& message) {
    m_statusText->SetLabel(message);
    m_statusText->GetParent()->Layout();
}

void SearchPage::ClearStatus() {
    m_statusText->SetLabel("");
}

void SearchPage::PerformSearch() {
    if (m_searching)
        return;

    const wxString needle = m_searchCtrl->GetValue();
    if (needle.IsEmpty()) {
        m_resultsModel->Clear();
        ClearStatus();
        return;
    }

    if (m_workspaceRoot.IsEmpty())
        m_workspaceRoot = ProjectSettings::Get().GetProjectPath();
    m_searchEngine.SetWorkspaceRoot(m_workspaceRoot);

    m_resultsModel->Clear();
    m_searching = true;
    m_progress->Show();
    m_progress->Pulse();
    m_searchButton->SetLabel(_("Stop"));
    ShowStatus(_("Searching..."));

    const int flags = BuildSearchFlags();

    std::vector<SearchResult> results;
    m_searchEngine.Search(
        needle, flags,
        [&results](const SearchResult& r) { results.push_back(r); },
        nullptr);

    m_resultsModel->SetResults(std::move(results));

    m_searching = false;
    m_progress->Hide();
    m_searchButton->SetLabel(_("Search"));

    const unsigned int count = m_resultsModel->GetCount();
    if (count == 0)
        ShowStatus(_("No results."));
    else
        ShowStatus(wxString::Format(_("%u result(s)."), count));
}

void SearchPage::CancelSearch() {
    if (!m_searching)
        return;
    m_searchEngine.RequestCancel();
}

void SearchPage::OnSearchButton(wxCommandEvent&) {
    if (m_searching)
        CancelSearch();
    else
        PerformSearch();
}

void SearchPage::OnSearchEnter(wxCommandEvent&) {
    PerformSearch();
}

void SearchPage::OnCancelButton(wxCommandEvent&) {
    m_searchCtrl->SetValue("");
    m_resultsModel->Clear();
    ClearStatus();
}

void SearchPage::OnReplace(wxCommandEvent&) {
    const wxDataViewItem item = m_resultsView->GetSelection();
    if (!item.IsOk())
        return;

    const unsigned int row = m_resultsModel->GetRow(item);
    const SearchResult& r = m_resultsModel->GetResult(row);

    if (ReplaceEngine::ReplaceOne(r, m_replaceCtrl->GetValue())) {
        ShowStatus(_("Replaced 1 occurrence."));
        PerformSearch();
    }
}

void SearchPage::OnReplaceAll(wxCommandEvent&) {
    if (m_resultsModel->GetCount() == 0)
        return;

    std::vector<SearchResult> all;
    all.reserve(m_resultsModel->GetCount());
    for (unsigned int i = 0; i < m_resultsModel->GetCount(); ++i)
        all.push_back(m_resultsModel->GetResult(i));

    std::vector<wxString> modified;
    const int count = ReplaceEngine::ReplaceAll(all,
                                                m_replaceCtrl->GetValue(),
                                                &modified);

    ShowStatus(wxString::Format(_("Replaced %d occurrence(s) in %zu file(s)."),
                                count, modified.size()));
    PerformSearch();
}

void SearchPage::OnResultActivated(wxDataViewEvent& event) {
    const unsigned int row = m_resultsModel->GetRow(event.GetItem());
    const SearchResult& r = m_resultsModel->GetResult(row);

    auto* filesTree = dynamic_cast<FilesTree*>(
        wxTheApp->GetMainTopWindow()->FindWindowById(+GUI::ControlID::FilesTree));

    if (filesTree)
        filesTree->OpenFile(r.filePath, r.lineNumber - 1);
}

void SearchPage::OnResultContextMenu(wxDataViewEvent& event) {
    if (!event.GetItem().IsOk())
        return;

    wxMenu menu;
    menu.Append(wxID_ANY, _("Open"));
    menu.Append(wxID_ANY, _("Copy Path"));
    menu.AppendSeparator();
    menu.Append(wxID_ANY, _("Replace this occurrence"));

    m_resultsView->PopupMenu(&menu);
}