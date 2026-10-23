#include "controlPanel.hpp"
#include "app/frames/mainFrame.hpp"
#include <wx/dcbuffer.h>
#include <wx/splitter.h>
#include "shortcutsSettings/shortcutsSettings.hpp"

#include "appConstants/widgetConstants.hpp"
using namespace widgetConstants;

#ifndef _
#define _(s) wxGetTranslation(s)
#endif

ControlPanel::ControlPanel(wxFrame *parent, wxWindowID id)
	: wxPanel(parent, id, wxDefaultPosition, wxSize(widgetConstants::kPanelWidth, widgetConstants::kPanelHeight),
			  wxBORDER_SIMPLE | wxTAB_TRAVERSAL), m_frameParent(parent) {
	if (!parent) {
		wxLogError("ControlPanel: parent frame is null");
		return;
	}

	BuildMenusList();

	const auto &theme = ThemesManager::Get().currentTheme;
	m_bgColor = GetThemeColor(theme, "main", wxColour(30, 30, 30));
	m_selectedColor =
		GetThemeColor(theme, "selectedFile", wxColour(60, 60, 60));
	m_textColor = GetThemeColor(theme, "text", *wxWHITE);
	m_borderColor = GetThemeColor(theme, "secondaryText", wxColour(80, 80, 80));
	m_iconsDir = ApplicationPaths::AssetsPath("icons");

	SetBackgroundColour(m_bgColor);
	SetMinSize(wxSize(kPanelWidth, kPanelHeight));

	const wxSize parentSize = parent->GetClientSize();
	SetPosition(wxPoint((parentSize.x - widgetConstants::kPanelWidth) / 2, 60));

	m_sizer = new wxBoxSizer(wxVERTICAL);

	BuildLayout();
	PopulateMenus();
	ApplyTheme();

	const std::array<wxAcceleratorEntry, 1> entries = {{
		{wxACCEL_NORMAL, WXK_ESCAPE, +Event::ControlPanel::Exit},
	}};
	SetAcceleratorTable(wxAcceleratorTable(entries.size(), entries.data()));

	SetSizer(m_sizer);
	SetSize(wxSize(widgetConstants::kPanelWidth, widgetConstants::kPanelHeight));

	if (m_searchInput) {
		m_searchInput->Bind(wxEVT_KEY_DOWN, &ControlPanel::OnSearchKeyDown,
							this);
		m_searchInput->SetFocus();
	}
}

void ControlPanel::BuildMenusList() {
	auto &sc = ShortCutSettingsManager::Get();

	auto GetSC = [&](const std::string &key) -> wxString {
		auto s = sc.GetSetting<std::string>(key);
		return s.found ? wxString("\t") + s.value : wxString("");
	};

	m_menus = {
		{"Toggle Terminal", "Ctrl+Shift+T", +Event::Terminal::ToggleView},
		{"Toggle Menu Bar", "Ctrl+Shift+M", +Event::View::ToggleMenuBar},
		{"Select Application Theme", "", +Event::ControlPanel::SelectTheme},
		{"Create New File", GetSC("shortcut_new_file"),
		 +Event::File::CreateFileEvent},
		{"Create New Dir", GetSC("shortcut_new_dir"), +Event::File::CreateDir},

		{"Toggle Search Page", "", +Event::View::ToggleSearchPage},
		{"Toggle Minimap", "", +Event::View::ToggleMiniMap},
		{"Toggle Code Search", "", +Event::View::ToggleCodeSearch},
		{"Toggle Quick Open", "", +Event::View::ToggleQuickOpen},
		{"Toggle File Tree", GetSC("shortcut_toggle_file_tree"),
		 +Event::View::ToggleFileTree},
		{"Toggle Status Bar", "", +Event::View::ToggleStatusBar},
		{"Toggle Tab Bar", "", +Event::View::ToggleTabBar}};
}

void ControlPanel::BuildLayout() {
	auto *topContainer = new wxPanel(this);
	topContainer->SetBackgroundColour(m_bgColor);

	auto *topSizer = new wxBoxSizer(wxHORIZONTAL);

	auto *searchIcon = new wxStaticBitmap(
		topContainer, wxID_ANY,
		UiUtils::CreateIcon(ApplicationPaths::GetIconPath("search.svg")));
	searchIcon->SetBackgroundColour(m_bgColor);
	topSizer->Add(searchIcon, 0, wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT,
				  kSearchPadding);

	m_searchInput =
		new wxTextCtrl(topContainer, wxID_ANY, wxEmptyString, wxDefaultPosition,
					   wxDefaultSize, wxBORDER_NONE | wxTE_PROCESS_ENTER);
	m_searchInput->SetBackgroundColour(m_bgColor);
	m_searchInput->SetForegroundColour(m_textColor);
	m_searchInput->SetFont(MakeMonospaceFont(11));
	m_searchInput->Bind(wxEVT_TEXT, &ControlPanel::SearchInputModified, this);
	topSizer->Add(m_searchInput, 1,
				  wxALIGN_CENTER_VERTICAL | wxRIGHT | wxTOP | wxBOTTOM,
				  kSearchPadding);

	topContainer->SetSizer(topSizer);
	m_sizer->Add(topContainer, 0, wxEXPAND);
	m_sizer->AddSpacer(1);

	auto *separator =
		new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(-1, 1));
	separator->SetBackgroundColour(m_borderColor);
	m_sizer->Add(separator, 0, wxEXPAND);

	m_menusContainer =
		new wxScrolledWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
							 wxVSCROLL | wxBORDER_NONE);
	m_menusContainer->SetBackgroundColour(m_bgColor);
	m_menusContainer->SetScrollRate(0, 15);
	m_menusContainer->SetSizer(new wxBoxSizer(wxVERTICAL));

	m_sizer->Add(m_menusContainer, 1, wxEXPAND);
}

void ControlPanel::PopulateMenus() {
	if (!m_menusContainer)
		return;

	auto *containerSizer = m_menusContainer->GetSizer();
	if (!containerSizer)
		return;

	bool firstSelected = false;

	for (const auto &menu : m_menus) {
		auto *row = CreateMenuRow(menu);
		if (!row)
			continue;

		containerSizer->Add(row, 0, wxEXPAND | wxTOP | wxBOTTOM, widgetConstants::kRowSpacing);

		if (!firstSelected) {
			HighlightRow(row);
			m_selectedMenu = row;
			firstSelected = true;
		}
	}

	m_menusContainer->FitInside();
	m_menusContainer->Layout();
}

wxPanel *ControlPanel::CreateMenuRow(const ControlMenu &menu) {
	if (!m_menusContainer)
		return nullptr;

	auto *row = new wxPanel(m_menusContainer, wxID_ANY);
	row->SetBackgroundColour(m_bgColor);
	row->SetLabel(wxString::Format("%d", menu.id));
	row->SetName(wxString::Format("%d", menu.id));

	auto *rowSizer = new wxBoxSizer(wxHORIZONTAL);

	wxString translated = wxGetTranslation(menu.name);

	auto *nameText = new wxStaticText(row, wxID_ANY, translated);
	nameText->SetName(wxString::Format("%d", menu.id));
	nameText->SetFont(MakeMonospaceFont(10));
	nameText->SetForegroundColour(m_textColor);
	nameText->SetBackgroundColour(m_bgColor);
	nameText->Bind(wxEVT_LEFT_UP, &ControlPanel::OnClickSelect, this);
	nameText->Bind(wxEVT_ENTER_WINDOW, &ControlPanel::OnRowEnter, this);
	nameText->Bind(wxEVT_LEAVE_WINDOW, &ControlPanel::OnRowLeave, this);
	rowSizer->Add(nameText, 1, wxALIGN_CENTER_VERTICAL | wxALL, kRowPadding);

	auto *shortcutText = new wxStaticText(row, wxID_ANY, menu.shortcut);
	shortcutText->SetFont(MakeMonospaceFont(8));
	shortcutText->SetForegroundColour(m_borderColor);
	shortcutText->SetBackgroundColour(m_bgColor);
	rowSizer->Add(shortcutText, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT,
				  kRowPadding);

	row->SetSizer(rowSizer);

	row->Bind(wxEVT_ENTER_WINDOW, &ControlPanel::OnRowEnter, this);
	row->Bind(wxEVT_LEAVE_WINDOW, &ControlPanel::OnRowLeave, this);

	return row;
}

void ControlPanel::ApplyTheme() {
	SetBackgroundColour(m_bgColor);
	if (m_searchInput)
		m_searchInput->SetBackgroundColour(m_bgColor);
	Refresh();
}

void ControlPanel::HighlightRow(wxPanel *row) {
	if (!row)
		return;

	row->SetBackgroundColour(m_selectedColor);

	for (auto *child : row->GetChildren()) {
		if (child) {
			child->SetBackgroundColour(m_selectedColor);
			child->Refresh();
		}
	}
	row->Refresh();
}

void ControlPanel::ClearRowHighlight(wxPanel *row) {
	if (!row)
		return;

	row->SetBackgroundColour(m_bgColor);

	for (auto *child : row->GetChildren()) {
		if (child) {
			child->SetBackgroundColour(m_bgColor);
			child->Refresh();
		}
	}
	row->Refresh();
}

void ControlPanel::UpdateSelectionVisual(wxPanel *oldRow, wxPanel *newRow) {
	if (oldRow == newRow)
		return;

	ClearRowHighlight(oldRow);
	HighlightRow(newRow);
}

void ControlPanel::Close(wxCommandEvent &WXUNUSED(event)) {
	Destroy();
	wxTheApp->GetTopWindow()->SetFocus();
}

void ControlPanel::UpSelection(wxCommandEvent &WXUNUSED(event)) {
	if (!m_menusContainer || !m_selectedMenu)
		return;

	const auto &children = m_menusContainer->GetChildren();
	if (children.IsEmpty())
		return;

	wxWindow *nextRow = m_selectedMenu->GetPrevSibling();
	if (!nextRow)
		nextRow = children[children.GetCount() - 1];

	auto *nextPanel = dynamic_cast<wxPanel *>(nextRow);
	if (!nextPanel || nextPanel == m_selectedMenu)
		return;

	UpdateSelectionVisual(m_selectedMenu, nextPanel);
	m_selectedMenu = nextPanel;
	ScrollSelectionIntoView(nextPanel);
}

void ControlPanel::DownSelection(wxCommandEvent &WXUNUSED(event)) {
	if (!m_menusContainer || !m_selectedMenu)
		return;

	const auto &children = m_menusContainer->GetChildren();
	if (children.IsEmpty())
		return;

	wxWindow *nextRow = m_selectedMenu->GetNextSibling();
	if (!nextRow)
		nextRow = children[0];

	auto *nextPanel = dynamic_cast<wxPanel *>(nextRow);
	if (!nextPanel || nextPanel == m_selectedMenu)
		return;

	UpdateSelectionVisual(m_selectedMenu, nextPanel);
	m_selectedMenu = nextPanel;
	ScrollSelectionIntoView(nextPanel);
}

void ControlPanel::OnKeyBoardSelect(wxCommandEvent &WXUNUSED(event)) {
	if (!m_selectedMenu)
		return;

	const wxString id = m_selectedMenu->GetLabel();
	if (!id.IsEmpty())
		Select(id);
}

void ControlPanel::OnClickSelect(wxMouseEvent &event) {
	auto *obj = dynamic_cast<wxWindow *>(event.GetEventObject());
	if (!obj)
		return;

	const wxString id = obj->GetName();
	if (!id.IsEmpty())
		Select(id);
}

void ControlPanel::Select(const wxString &id) {
	if (id.IsEmpty())
		return;

	const int commandId = wxAtoi(id);

	auto *mainFrame = dynamic_cast<MainFrame *>(wxTheApp->GetTopWindow());
	if (!mainFrame) {
		wxMessageBox(_("Unable to find the application's main window"));
		return;
	}

	auto PostEventInMainFrame = ([this, mainFrame](const int &eventId) {
		wxCommandEvent evt(wxEVT_MENU, eventId);
		wxPostEvent(mainFrame, evt);
	});

	switch (commandId) {

	case +Event::Terminal::ToggleView: {
		PostEventInMainFrame(+Event::Terminal::ToggleView);
	} break;

	case +Event::View::ToggleMenuBar: {
		PostEventInMainFrame(+Event::View::ToggleMenuBar);
	} break;

	case +Event::ControlPanel::SelectTheme: {
		OpenThemeSelectionPanel();
	} break;

	case +Event::File::CreateFileEvent: {
		PostEventInMainFrame(+Event::File::CreateFileEvent);
	} break;

	case +Event::File::CreateDir: {
		PostEventInMainFrame(+Event::File::CreateDir);
	} break;

	case +Event::View::ToggleSearchPage: {
		PostEventInMainFrame(+Event::View::ToggleSearchPage);
	} break;

	case +Event::View::ToggleMiniMap: {
		PostEventInMainFrame(+Event::View::ToggleMiniMap);
	} break;

	case +Event::View::ToggleCodeSearch: {
		PostEventInMainFrame(+Event::View::ToggleCodeSearch);
	} break;

	case +Event::View::ToggleQuickOpen: {
		PostEventInMainFrame(+Event::View::ToggleQuickOpen);
	} break;

	case +Event::View::ToggleFileTree: {
		PostEventInMainFrame(+Event::View::ToggleFileTree);
	} break;

	case +Event::View::ToggleStatusBar: {
		PostEventInMainFrame(+Event::View::ToggleStatusBar);
	} break;

	case +Event::View::ToggleTabBar: {
		PostEventInMainFrame(+Event::View::ToggleTabBar);
	} break;

	default:
		break;
	}

	Destroy();
	wxTheApp->GetTopWindow()->SetFocus();
}

void ControlPanel::SearchInputModified(wxCommandEvent &WXUNUSED(event)) {
	if (!m_menusContainer || !m_searchInput)
		return;

	const wxString query = m_searchInput->GetValue().Lower();

	for (auto *child : m_menusContainer->GetChildren()) {
		auto *row = dynamic_cast<wxPanel *>(child);
		if (!row)
			continue;

		wxString rowName;
		for (auto *rowChild : row->GetChildren()) {
			if (auto *text = dynamic_cast<wxStaticText *>(rowChild)) {
				rowName = text->GetLabel().Lower();
				break;
			}
		}

		const bool matches =
			query.IsEmpty() || rowName.Find(query) != wxNOT_FOUND;

		row->Show(matches);
	}

	if (auto *containerSizer = m_menusContainer->GetSizer()) {
		containerSizer->Layout();
		m_menusContainer->FitInside();
	}
}

void ControlPanel::OnSearchKeyDown(wxKeyEvent &event) {
	switch (event.GetKeyCode()) {
	case WXK_UP: {
		wxCommandEvent evt(wxEVT_MENU, +Event::ControlPanel::Up);
		UpSelection(evt);
		return;
	}
	case WXK_DOWN: {
		wxCommandEvent evt(wxEVT_MENU, +Event::ControlPanel::Down);
		DownSelection(evt);
		return;
	}
	case WXK_RETURN:
	case WXK_NUMPAD_ENTER: {
		wxCommandEvent evt(wxEVT_MENU, +Event::ControlPanel::Select);
		OnKeyBoardSelect(evt);
		return;
	}
	case WXK_ESCAPE: {
		wxCommandEvent evt(wxEVT_MENU, +Event::ControlPanel::Exit);
		Close(evt);
		return;
	}
	default:
		break;
	}

	event.Skip();
}

void ControlPanel::OnRowEnter(wxMouseEvent &event) {
	auto *obj = dynamic_cast<wxWindow *>(event.GetEventObject());
	if (!obj)
		return;

	auto *row = obj;
	while (row && !dynamic_cast<wxPanel *>(row))
		row = row->GetParent();

	auto *rowPanel = dynamic_cast<wxPanel *>(row);
	if (!rowPanel || rowPanel == m_selectedMenu) {
		event.Skip();
		return;
	}

	UpdateSelectionVisual(m_selectedMenu, rowPanel);
	m_selectedMenu = rowPanel;

	event.Skip();
}

void ControlPanel::OnRowLeave(wxMouseEvent &event) { event.Skip(); }

void ControlPanel::ScrollSelectionIntoView(wxWindow *row) {
	if (!m_menusContainer || !row)
		return;

	const wxRect rowRect = row->GetRect();
	const wxSize clientSize = m_menusContainer->GetClientSize();

	int scrollX = 0, scrollY = 0;
	m_menusContainer->GetViewStart(&scrollX, &scrollY);

	int ppuX = 0, ppuY = 0;
	m_menusContainer->GetScrollPixelsPerUnit(&ppuX, &ppuY);

	if (ppuY <= 0)
		return;

	const int topPx = scrollY * ppuY;
	const int bottomPx = topPx + clientSize.y;

	const int rowTop = rowRect.GetTop();
	const int rowBottom = rowRect.GetBottom();

	if (rowTop >= topPx && rowBottom <= bottomPx)
		return;

	int newScrollY = scrollY;

	if (rowTop < topPx)
		newScrollY = rowTop / ppuY;
	else if (rowBottom > bottomPx)
		newScrollY = (rowBottom - clientSize.y + ppuY - 1) / ppuY;

	if (newScrollY != scrollY) {
		m_menusContainer->Scroll(0, newScrollY + 10);
		m_menusContainer->Refresh();
		m_menusContainer->Update();
	}
}