#pragma once
#include <wx/stc/stc.h>
#include "../statusBar/status.hpp"
#include "../../src/languagePrefs/prefs.cpp"
#include "../../src/lexerStyle/lexerStyle.cpp"

class CodeContainer : public wxScrolled<wxPanel> {
public:
	wxStyledTextCtrl* codeEditor = nullptr;
	wxStyledTextCtrl* codeMap = nullptr;
	LanguageInfo const* current_lang;
	wxString currentPath;
	bool changed = false;
	CodeContainer(wxWindow* parent, wxString path);
	bool LoadPath(wxString path);
	void OnSave(wxCommandEvent& event);
	void ToggleMiniMapView(wxCommandEvent& event);
	void ToggleCommentLine(wxCommandEvent& event);
private:
	bool changing_values;
	wxFont font;
	StatusBar* status_bar = ((StatusBar*)FindWindowById(ID_STATUS_BAR));
	wxPoint codeMapClickPoint = wxPoint(0, 0);
	bool codeMapMouseOver = false;
	wxString DeterminePrefs(wxString filename);
	bool InitializePrefs(wxString name);
	void CodeEditorInitPrefs();
	void OnCodeEditorScroll(wxStyledTextEvent& event);
	void OnChange(wxStyledTextEvent& event);
	void CharAdd(wxStyledTextEvent& event);
	void OnMarginClick(wxStyledTextEvent& event);
	void OnClick(wxMouseEvent& event);
	void OnArrowsPress(wxKeyEvent& event);
	void CodeMapInitPrefs();
	void OnMapClick(wxMouseEvent& event);
	void OnCodeMapPaint(wxPaintEvent& event);
	void OnCodeMapPainted(wxStyledTextEvent& event);
	void OnCodeMapScroll(wxStyledTextEvent& event);
	void OnCodeMapMouseEnter(wxMouseEvent& event);
	void OnCodeMapMouseLeave(wxMouseEvent& event);
	void OnAutoCompCompleted(wxStyledTextEvent& event);
	void DrawBorder(wxStyledTextEvent& event);
	wxDECLARE_EVENT_TABLE();
};

wxBEGIN_EVENT_TABLE(CodeContainer, wxPanel)
EVT_MENU(wxID_SAVE, CodeContainer::OnSave)
wxEND_EVENT_TABLE()