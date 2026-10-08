#pragma once

#include <wx/dataview.h>
#include <wx/string.h>
#include <vector>

struct SearchResult {
    wxString filePath;   
    int      lineNumber;
    int      column;   
    int      length;   
    wxString preview;   
};

class SearchResultsModel : public wxDataViewVirtualListModel {
public:
    enum Column {
        Col_File = 0,
        Col_Line,
        Col_Preview,
        Col_Count
    };

    SearchResultsModel();
    void SetResults(std::vector<SearchResult> results);
    void AppendResult(const SearchResult& result);
    void Clear();
    const SearchResult& GetResult(unsigned int row) const;
    unsigned int GetCount() const override;

    void GetValueByRow(wxVariant& variant, unsigned int row,
                       unsigned int col) const override;
    bool SetValueByRow(const wxVariant& variant, unsigned int row,
                       unsigned int col) override;
    bool GetAttrByRow(unsigned int row, unsigned int col,
                      wxDataViewItemAttr& attr) const override;

private:
    std::vector<SearchResult> m_results;
};