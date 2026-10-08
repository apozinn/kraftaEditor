#pragma once

#include "replaceEngine.hpp"
#include "searchEngine.hpp"
#include "searchResultsModel.hpp"

#include <wx/wx.h>
#include <wx/dataview.h>
#include <wx/srchctrl.h>
#include <wx/checkbox.h>
#include <wx/button.h>
#include <wx/statbmp.h>
#include <wx/stattext.h>
#include <wx/gauge.h>

#include <memory>

class SearchPage : public wxPanel {
public:
    explicit SearchPage(wxWindow* parent);
    ~SearchPage() override;


    void SetWorkspaceRoot(const wxString& path);
    void PerformSearch();
    void CancelSearch();

private:
    void SetupUI();
    void SetupEvents();
    void ApplyTheme();

    int BuildSearchFlags() const;

    void OnSearchButton(wxCommandEvent& event);
    void OnSearchEnter(wxCommandEvent& event);
    void OnReplace(wxCommandEvent& event);
    void OnReplaceAll(wxCommandEvent& event);
    void OnResultActivated(wxDataViewEvent& event);
    void OnResultContextMenu(wxDataViewEvent& event);
    void OnCancelButton(wxCommandEvent& event);

    void ShowStatus(const wxString& message);
    void ClearStatus();

    wxSearchCtrl*   m_searchCtrl{nullptr};
    wxTextCtrl*     m_replaceCtrl{nullptr};
    wxCheckBox*     m_caseCheck{nullptr};
    wxCheckBox*     m_wordCheck{nullptr};
    wxCheckBox*     m_regexCheck{nullptr};
    wxButton*       m_searchButton{nullptr};
    wxButton*       m_replaceButton{nullptr};
    wxButton*       m_replaceAllButton{nullptr};
    wxGauge*        m_progress{nullptr};
    wxStaticText*   m_statusText{nullptr};
    wxDataViewCtrl* m_resultsView{nullptr};
    wxObjectDataPtr<SearchResultsModel> m_resultsModel;
    SearchEngine m_searchEngine;
    wxString m_workspaceRoot;
    bool m_searching{false};
};