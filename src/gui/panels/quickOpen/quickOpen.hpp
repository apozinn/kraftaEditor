#pragma once

#include "themesManager/themesManager.hpp"
#include "ui/ids.hpp"

#include <wx/scrolwin.h>
#include <wx/wx.h>

#include <filesystem>
#include <unordered_set>
#include <vector>

struct QuickOpenFileStruct {
	wxString name;
	wxString path;
};

class QuickOpen : public wxPanel {
  public:
	explicit QuickOpen(wxFrame *parent);
	~QuickOpen() override = default;

	void Close(wxCommandEvent &event);
	void OnKeyboardUp(wxCommandEvent &event);
	void OnKeyboardDown(wxCommandEvent &event);
	void OnKeyboardEnter(wxCommandEvent &event);

  private:
	void BuildLayout();
	void IndexProjectFiles();
	void ApplyFilter(const wxString &query);
	void MoveSelection(int direction);
	void ScrollSelectionIntoView();

	void OnSearchBarChange(wxCommandEvent &event);
	void OnSearchKeyDown(wxKeyEvent &event);
	void OnListPaint(wxPaintEvent &event);
	void OnListMouse(wxMouseEvent &event);
	void OnListLeftDown(wxMouseEvent &event);
	void OnListSize(wxSizeEvent &event);

	int  HitTestRow(int y) const;
	void OpenFileAt(int index);

	wxBoxSizer       *m_sizer{nullptr};
	wxBoxSizer       *m_topSizer{nullptr};
	wxPanel          *m_topContainer{nullptr};
	wxScrolledWindow *m_list{nullptr};
	wxTextCtrl       *m_searchBar{nullptr};

	wxColour m_bgColor;
	wxColour m_searchBgColor;
	wxColour m_textColor;
	wxColour m_secondaryTextColor;
	wxColour m_highlightColor;

	wxFont m_nameFont;
	wxFont m_pathFont;

	int m_rowHeight{24};

	std::vector<QuickOpenFileStruct> m_files;
	std::vector<size_t>              m_visibleIndices;
	int                              m_selectedIndex{-1};
	int                              m_hoverIndex{-1};

	wxDECLARE_NO_COPY_CLASS(QuickOpen);
};