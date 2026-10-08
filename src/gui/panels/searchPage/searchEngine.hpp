#pragma once

#include "searchResultsModel.hpp"

#include <atomic>
#include <functional>
#include <set>
#include <wx/string.h>

enum SearchFlag {
    Search_None         = 0,
    Search_CaseSensitive = 1 << 0,
    Search_WholeWord     = 1 << 1,
    Search_Regex         = 1 << 2
};

class SearchEngine {
public:
    using ResultCallback = std::function<void(const SearchResult&)>;
    using ProgressCallback = std::function<void(int filesScanned)>;

    SearchEngine();
    ~SearchEngine();

    void SetWorkspaceRoot(const wxString& root);
    void AddIgnoredDirectory(const wxString& name);

    bool Search(const wxString& needle,
                int flags,
                const ResultCallback& onResult,
                const ProgressCallback& onProgress = nullptr);
    void RequestCancel();
    bool IsCancelled() const;
    bool SearchFile(const wxString& filePath, const wxString& needle,
                    int flags, const ResultCallback& onResult);

private:
friend class SearchDirTraverser;
    bool ShouldIgnoreDirectory(const wxString& name) const;
    
    std::set<wxString> BuildIgnoredSet() const;

    wxString m_workspaceRoot;
    std::set<wxString> m_ignoredDirs;
    std::atomic<bool> m_cancelRequested{false};
    int m_filesScanned = 0;
};