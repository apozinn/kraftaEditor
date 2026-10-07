#pragma once

#include "appPaths/appPaths.hpp"
#include "themesManager/themesManager.hpp"
#include "ui/ids.hpp"

#include <nlohmann/json.hpp>
#include <wx/scrolwin.h>
#include <wx/wx.h>

#include <array>
#include <vector>

using json = nlohmann::json;

struct ControlMenu {
	const char *name;
	const char *shortcut;
	int         id;
};

class ControlPanel : public wxPanel {
  public:
	ControlPanel(wxFrame *parent, wxWindowID id);
	~ControlPanel() override = default;

	void Close(wxCommandEvent &event);
	void UpSelection(wxCommandEvent &event);
	void DownSelection(wxCommandEvent &event);
	void OnClickSelect(wxMouseEvent &event);
	void OnKeyBoardSelect(wxCommandEvent &event);
	void SearchInputModified(wxCommandEvent &event);
	void Select(const wxString &id);

  private:
	void BuildLayout();
	void PopulateMenus();
	wxPanel *CreateMenuRow(const ControlMenu &menu);
	void ApplyTheme();
	void HighlightRow(wxPanel *row);
	void ClearRowHighlight(wxPanel *row);
	void UpdateSelectionVisual(wxPanel *oldRow, wxPanel *newRow);
	void ScrollSelectionIntoView(wxWindow *row);

	void OnSearchKeyDown(wxKeyEvent &event);
	void OnRowEnter(wxMouseEvent &event);
	void OnRowLeave(wxMouseEvent &event);

	wxScrolledWindow *m_menusContainer{nullptr};
	wxPanel          *m_selectedMenu{nullptr};
	wxTextCtrl       *m_searchInput{nullptr};
	wxBoxSizer       *m_sizer{nullptr};

	wxColour m_bgColor;
	wxColour m_selectedColor;
	wxColour m_textColor;
	wxColour m_borderColor;

	wxString m_iconsDir;

	const std::vector<ControlMenu> m_menus{
		{"Open Terminal", "Ctrl+Shift+T", 1},
		{"Toggle Menu Bar", "Ctrl+Shift+M", 2},
	};

	wxDECLARE_NO_COPY_CLASS(ControlPanel);
};