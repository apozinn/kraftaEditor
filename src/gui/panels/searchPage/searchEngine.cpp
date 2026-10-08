#include "searchEngine.hpp"

#include <fstream>
#include <regex>
#include <sstream>
#include <wx/dir.h>
#include <wx/filename.h>

namespace {

class SearchDirTraverser : public wxDirTraverser {
public:
    SearchDirTraverser(SearchEngine* engine,
                       const wxString& needle,
                       int flags,
                       const SearchEngine::ResultCallback& cb,
                       const std::set<wxString>& ignored)
        : m_engine(engine),
          m_needle(needle),
          m_flags(flags),
          m_onResult(cb),
          m_ignored(ignored) {}

    wxDirTraverseResult OnFile(const wxString& filename) override {
        if (m_engine->IsCancelled())
            return wxDIR_STOP;

        m_engine->SearchFile(filename, m_needle, m_flags, m_onResult);
        return wxDIR_CONTINUE;
    }

    wxDirTraverseResult OnDir(const wxString& dirname) override {
        if (m_engine->IsCancelled())
            return wxDIR_STOP;

        wxString name = wxFileName(dirname).GetName();
        if (m_ignored.count(name) > 0 || name.StartsWith("."))
            return wxDIR_IGNORE;

        return wxDIR_CONTINUE;
    }

private:
    SearchEngine* m_engine;
    wxString m_needle;
    int m_flags;
    SearchEngine::ResultCallback m_onResult;
    std::set<wxString> m_ignored;
};

}  // namespace

SearchEngine::SearchEngine() {
    m_ignoredDirs = BuildIgnoredSet();
}

SearchEngine::~SearchEngine() = default;

std::set<wxString> SearchEngine::BuildIgnoredSet() const {
    return {"node_modules", "build", "dist", "out", "target",
            ".git", ".svn", ".hg", ".cache", ".next", "vendor"};
}

void SearchEngine::SetWorkspaceRoot(const wxString& root) {
    m_workspaceRoot = root;
}

void SearchEngine::AddIgnoredDirectory(const wxString& name) {
    m_ignoredDirs.insert(name);
}

void SearchEngine::RequestCancel() {
    m_cancelRequested.store(true);
}

bool SearchEngine::IsCancelled() const {
    return m_cancelRequested.load();
}

bool SearchEngine::ShouldIgnoreDirectory(const wxString& name) const {
    if (name.StartsWith("."))
        return true;
    return m_ignoredDirs.count(name) > 0;
}

bool SearchEngine::Search(const wxString& needle,
                          int flags,
                          const ResultCallback& onResult,
                          const ProgressCallback& onProgress) {
    m_cancelRequested.store(false);
    m_filesScanned = 0;

    if (m_workspaceRoot.IsEmpty() || needle.IsEmpty())
        return true;

    SearchDirTraverser traverser(this, needle, flags, onResult, m_ignoredDirs);

    wxDir dir(m_workspaceRoot);
    if (!dir.IsOpened())
        return true;

    dir.Traverse(traverser, wxEmptyString, wxDIR_FILES | wxDIR_DIRS);

    if (onProgress)
        onProgress(m_filesScanned);

    return !IsCancelled();
}

bool SearchEngine::SearchFile(const wxString& filePath,
                              const wxString& needle,
                              int flags,
                              const ResultCallback& onResult) {
    std::ifstream file(filePath.ToStdString());
    if (!file.is_open())
        return false;

    bool caseSensitive = (flags & Search_CaseSensitive) != 0;
    bool wholeWord     = (flags & Search_WholeWord) != 0;
    bool useRegex      = (flags & Search_Regex) != 0;

    std::string pattern;

    if (useRegex) {
        pattern = needle.ToStdString();
    } else {
        std::string escaped;
        for (char c : needle.ToStdString()) {
            if (std::string(".^$|()[]{}*+?\\").find(c) != std::string::npos)
                escaped += '\\';
            escaped += c;
        }
        pattern = escaped;
    }

    if (wholeWord)
        pattern = "\\b" + pattern + "\\b";

    std::regex_constants::syntax_option_type reFlags =
        std::regex_constants::ECMAScript;
    if (!caseSensitive)
        reFlags |= std::regex_constants::icase;

    std::regex re;
    try {
        re = std::regex(pattern, reFlags);
    } catch (const std::regex_error&) {
        return false;
    }

    std::string line;
    int lineNumber = 1;
    const wxString needleLower = caseSensitive ? needle : needle.Lower();

    while (std::getline(file, line)) {
        if (IsCancelled())
            return false;

        try {
            auto begin = std::sregex_iterator(line.begin(), line.end(), re);
            auto end   = std::sregex_iterator();

            for (auto it = begin; it != end; ++it) {
                const std::smatch& match = *it;

                SearchResult result;
                result.filePath   = filePath;
                result.lineNumber = lineNumber;
                result.column     = static_cast<int>(match.position());
                result.length     = static_cast<int>(match.length());
                result.preview    = wxString::FromUTF8(line);

                if (onResult)
                    onResult(result);
            }
        } catch (const std::regex_error&) {
        }

        ++lineNumber;
    }

    return true;
}