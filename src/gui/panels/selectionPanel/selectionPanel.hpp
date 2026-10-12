#pragma once

#include <functional>
#include <string>
#include <vector>
#include <wx/scrolwin.h>
#include <wx/wx.h>

struct OptionsListStruct {
	std::string title;
	std::string description;
	int id;
};

class SelectionPanel : public wxPanel {
  public:
	SelectionPanel(wxFrame *parent, wxWindowID id = wxID_ANY);

	void ShowList(const std::vector<OptionsListStruct> &optionsList,
				  std::function<void(const OptionsListStruct &)> opt);

  private:
	void BuildLayout();
	void BuildOptions(const std::vector<OptionsListStruct> &optionsList);
	void ApplyTheme();

	wxPanel *CreateOptionRow(const OptionsListStruct &option);

	void UpSelection();
	void DownSelection();

	void OnKeyDown(wxKeyEvent &event);
	void OnSearchKeyDown(wxKeyEvent &event);
	void OnSearchModified(wxCommandEvent &event);
	void OnRowEnter(wxMouseEvent &event);
	void OnRowLeave(wxMouseEvent &event);
	void OnRowClick(wxMouseEvent &event);

	void SelectCurrent();
	void ScrollSelectionIntoView(wxWindow *row);
	void HighlightRow(wxPanel *row);
	void ClearRowHighlight(wxPanel *row);
	void UpdateSelectionVisual(wxPanel *oldRow, wxPanel *newRow);

	wxBoxSizer *m_sizer{nullptr};
	wxTextCtrl *m_searchInput{nullptr};
	wxScrolledWindow *m_optionsContainer{nullptr};
	wxPanel *m_selectedOption{nullptr};

	wxFrame *m_parent{nullptr};

	wxColour m_bgColor;
	wxColour m_selectedColor;
	wxColour m_textColor;
	wxColour m_borderColor;

	wxString m_iconsDir;

	std::vector<OptionsListStruct> m_optionsList;
	std::function<void(const OptionsListStruct &)> m_onSelect;
};