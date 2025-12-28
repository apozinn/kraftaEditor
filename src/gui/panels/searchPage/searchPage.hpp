#pragma once

#include <wx/wx.h>
#include <wx/listctrl.h>
#include <set>

/**
 * @class SearchPage
 * @brief Provides a workspace-wide search and replace interface similar to
 *        the "Search in Files" feature found in modern code editors.
 *
 * SearchPage allows searching text across all files in a workspace directory,
 * displaying results with file path, line number, and line preview.
 * It also supports single and bulk replace operations.
 *
 * The search behavior can be customized using options such as:
 * - Case sensitivity
 * - Whole word matching
 * - Regular expressions
 */
class SearchPage : public wxPanel
{
public:
    /**
     * @brief Constructs the SearchPage panel.
     * @param parent Parent window.
     */
    explicit SearchPage(wxWindow* parent);

    /**
     * @brief Sets the root directory used as the workspace for searching.
     * @param path Absolute path to the workspace root.
     */
    void SetWorkspaceRoot(const wxString& path);

private:
    /**
     * @brief Starts a new search using the current search options.
     *
     * Clears previous results and recursively scans the workspace
     * directory for matches.
     */
    void PerformSearch();

    /**
     * @brief Recursively searches a directory for matching files.
     * @param path Directory path to scan.
     */
    void SearchDirectory(const wxString& path);

    /**
     * @brief Determines whether a directory should be ignored during search.
     *
     * Typically used to skip large or irrelevant folders
     * (e.g. node_modules, .git).
     *
     * @param name Directory name.
     * @return true if the directory should be ignored.
     */
    bool ShouldIgnoreDirectory(const wxString& name) const;

    /**
     * @brief Triggered when the search button is clicked.
     * @param event Command event.
     */
    void OnSearch(wxCommandEvent& event);

    /**
     * @brief Triggered when the Enter key is pressed in the search field.
     * @param event Command event.
     */
    void OnSearchEnter(wxCommandEvent& event);

    /**
     * @brief Replaces the currently selected search result.
     * @param event Command event.
     */
    void OnReplace(wxCommandEvent& event);

    /**
     * @brief Replaces all matches found in the current search.
     * @param event Command event.
     */
    void OnReplaceAll(wxCommandEvent& event);

    /**
     * @brief Triggered when a search result is activated.
     *
     * Usually used to open the file and navigate to the matching line.
    ளம்
     * @param event List control event.
     */
    void OnResultActivated(wxListEvent& event);

    /**
     * @brief Replaces a single match inside a file.
     *
     * @param filePath Path to the file.
     * @param line Line number (zero-based).
     * @param col Column index of the match.
     * @param len Length of the matched text.
     */
    void ReplaceMatch(
        const wxString& filePath,
        int line,
        int col,
        int len
    );

    /**
     * @brief Replaces all matches across all result entries.
     */
    void ReplaceAllMatches();

private:
    wxTextCtrl* m_searchCtrl{nullptr};
    wxTextCtrl* m_replaceCtrl{nullptr};

    wxCheckBox* m_caseCheck{nullptr};
    wxCheckBox* m_wordCheck{nullptr};
    wxCheckBox* m_regexCheck{nullptr};

    wxListCtrl* m_results{nullptr};

    wxString m_workspaceRoot;
};
