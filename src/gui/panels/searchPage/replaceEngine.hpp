#pragma once

#include "searchResultsModel.hpp"

#include <vector>
#include <wx/string.h>


class ReplaceEngine {
public:
    static bool ReplaceOne(const SearchResult& result,
                           const wxString& replacement);
    static int ReplaceAll(const std::vector<SearchResult>& results,
                          const wxString& replacement,
                          std::vector<wxString>* outModifiedFiles = nullptr);
};