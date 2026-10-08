#include "replaceEngine.hpp"

#include <algorithm>
#include <fstream>
#include <map>
#include <sstream>

namespace {
bool ReadFileLines(const wxString& path, std::vector<std::string>& outLines) {
    std::ifstream file(path.ToStdString());
    if (!file.is_open())
        return false;

    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        outLines.push_back(line);
    }
    return true;
}
bool WriteFileLines(const wxString& path,
                    const std::vector<std::string>& lines) {
    std::ofstream file(path.ToStdString(), std::ios::trunc);
    if (!file.is_open())
        return false;

    for (size_t i = 0; i < lines.size(); ++i) {
        file << lines[i];
        if (i + 1 < lines.size())
            file << '\n';
    }
    return true;
}

}  // namespace

bool ReplaceEngine::ReplaceOne(const SearchResult& result,
                               const wxString& replacement) {
    std::vector<std::string> lines;
    if (!ReadFileLines(result.filePath, lines))
        return false;

    if (result.lineNumber < 1 ||
        result.lineNumber > static_cast<int>(lines.size()))
        return false;

    std::string& line = lines[result.lineNumber - 1];

    if (result.column < 0 ||
        result.column + result.length > static_cast<int>(line.size()))
        return false;

    line.replace(result.column, result.length,
                 replacement.ToStdString());

    return WriteFileLines(result.filePath, lines);
}

int ReplaceEngine::ReplaceAll(const std::vector<SearchResult>& results,
                              const wxString& replacement,
                              std::vector<wxString>* outModifiedFiles) {
    std::map<wxString, std::vector<SearchResult>> byFile;
    for (const auto& r : results)
        byFile[r.filePath].push_back(r);

    int totalReplaced = 0;

    for (auto& [filePath, fileResults] : byFile) {
        std::vector<std::string> lines;
        if (!ReadFileLines(filePath, lines))
            continue;
        std::sort(fileResults.begin(), fileResults.end(),
                  [](const SearchResult& a, const SearchResult& b) {
                      if (a.lineNumber != b.lineNumber)
                          return a.lineNumber > b.lineNumber;
                      return a.column > b.column;
                  });

        for (const auto& r : fileResults) {
            if (r.lineNumber < 1 ||
                r.lineNumber > static_cast<int>(lines.size()))
                continue;

            std::string& line = lines[r.lineNumber - 1];

            if (r.column < 0 ||
                r.column + r.length > static_cast<int>(line.size()))
                continue;

            line.replace(r.column, r.length, replacement.ToStdString());
            ++totalReplaced;
        }

        if (WriteFileLines(filePath, lines) && outModifiedFiles)
            outModifiedFiles->push_back(filePath);
    }

    return totalReplaced;
}