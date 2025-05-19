#pragma once
#include <vector>

#include "../../utils/randoms.hpp"
#include "../codeContainer/code.hpp"
#include "../filesTree/files.hpp"

struct TabInfo {
	const char* path;
	const char* name;
	const LanguageInfo* language;
	bool inFocus;
};

std::vector<TabInfo> tabList;

class Tabs : public wxPanel {
	wxSizer* sizer;
	wxStaticBitmap* menu;
	wxStaticBitmap* arrow_left;
	wxStaticBitmap* arrow_right;
	wxScrolled<wxPanel>* tabs_container;
	wxBoxSizer* tabs_ctn_sizer;
	public:
	wxString selected_tab;
	Tabs(wxPanel* parent, wxWindowID ID);
	void Add(wxString tab_name, wxString path);
	void Close(wxWindow* tab, wxString tab_path);
	void CloseAll();
	void Select();
	void OnTabClick(wxMouseEvent& event);
	void OnCloseTab(wxMouseEvent& event);
	void OnMenu(wxMouseEvent& event);
private:
	void OnEnterComp(wxMouseEvent& event);
	void OnLeaveComp(wxMouseEvent& event);
	void OnPaint(wxPaintEvent& event);
	void OnTabPaint(wxPaintEvent& event);
	void OnPreviousTab(wxMouseEvent& event);
	void OnNextTab(wxMouseEvent& event);
	void OnTabDestroy(wxWindowDestroyEvent& event);
	wxDECLARE_NO_COPY_CLASS(Tabs);
};