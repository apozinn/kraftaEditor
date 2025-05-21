#pragma once
#include <vector>

#include "../../utils/randoms.hpp"
#include "../codeContainer/code.hpp"
#include "../filesTree/files.hpp"

class Tabs : public wxPanel
{
	wxSizer *sizer;
	wxStaticBitmap *menu;
	wxScrolled<wxPanel> *tabsContainer;
	wxBoxSizer *tabsContainerSizer;

public:
	wxString selected_tab;
	Tabs(wxPanel *parent, wxWindowID ID);
	void Add(wxString tab_name, wxString path);
	void Close(wxWindow *tab, wxString tab_path);
	void CloseAll();
	void Select();
	void OnTabClick(wxMouseEvent &event);
	void OnCloseTab(wxMouseEvent &event);
	void OnMenu(wxMouseEvent &event);

private:
	void OnEnterComp(wxMouseEvent &event);
	void OnLeaveComp(wxMouseEvent &event);
	void OnPaint(wxPaintEvent &event);
	void OnTabPaint(wxPaintEvent &event);
	void OnPreviousTab();
	void OnNextTab();
	wxDECLARE_NO_COPY_CLASS(Tabs);
};