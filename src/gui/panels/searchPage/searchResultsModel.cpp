#include "searchResultsModel.hpp"

SearchResultsModel::SearchResultsModel()
    : wxDataViewVirtualListModel(0) {}

void SearchResultsModel::SetResults(std::vector<SearchResult> results) {
    m_results = std::move(results);
    Reset(static_cast<unsigned int>(m_results.size()));
}

void SearchResultsModel::AppendResult(const SearchResult& result) {
    m_results.push_back(result);
    RowAppended();
}

void SearchResultsModel::Clear() {
    m_results.clear();
    Reset(0);
}

const SearchResult& SearchResultsModel::GetResult(unsigned int row) const {
    return m_results[row];
}

unsigned int SearchResultsModel::GetCount() const {
    return static_cast<unsigned int>(m_results.size());
}

void SearchResultsModel::GetValueByRow(wxVariant& variant, unsigned int row,
                                       unsigned int col) const {
    if (row >= m_results.size())
        return;

    const SearchResult& r = m_results[row];

    switch (col) {
        case Col_File:
            variant = r.filePath;
            break;
        case Col_Line:
            variant = wxString::Format("%d", r.lineNumber);
            break;
        case Col_Preview:
            variant = r.preview;
            break;
        default:
            break;
    }
}

bool SearchResultsModel::SetValueByRow(const wxVariant&, unsigned int,
                                       unsigned int) {
    return false;
}

bool SearchResultsModel::GetAttrByRow(unsigned int, unsigned int,
                                      wxDataViewItemAttr&) const {
    return false;
}