#include "controlPanel.hpp"
#include "app/frames/mainFrame.hpp"

#include <wx/dcbuffer.h>
#include <wx/splitter.h>

namespace {

constexpr int kPanelWidth    = 480;
constexpr int kPanelHeight   = 280;
constexpr int kRowPadding    = 8;
constexpr int kRowSpacing    = 2;
constexpr int kSearchPadding = 8;

wxColour GetThemeColor(const json &theme, const char *key,
                       const wxColour &fallback) {
	if (!theme.contains(key) || !theme[key].is_string())
		return fallback;

	try {
		return wxColour(wxString::FromUTF8(theme[key].get<std::string>()));
	} catch (...) {
		return fallback;
	}
}

wxFont MakeMonospaceFont(int size, wxFontWeight weight = wxFONTWEIGHT_NORMAL) {
	wxFont font(size, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, weight);
	font.SetFaceName("Monospace");
	return font;
}

} // namespace

ControlPanel::ControlPanel(wxFrame *parent, wxWindowID id)
	: wxPanel(parent, id, wxDefaultPosition, wxSize(kPanelWidth, kPanelHeight),
	          wxBORDER_SIMPLE | wxTAB_TRAVERSAL) {
	if (!parent) {
		wxLogError("ControlPanel: parent frame is null");
		return;
	}

	const auto &theme = ThemesManager::Get().currentTheme;
	m_bgColor       = GetThemeColor(theme, "main", wxColour(30, 30, 30));
	m_selectedColor = GetThemeColor(theme, "selectedFile", wxColour(60, 60, 60));
	m_textColor     = GetThemeColor(theme, "text", *wxWHITE);
	m_borderColor   = GetThemeColor(theme, "secondaryText", wxColour(80, 80, 80));
	m_iconsDir      = ApplicationPaths::AssetsPath("icons");

	SetBackgroundColour(m_bgColor);
	SetMinSize(wxSize(kPanelWidth, kPanelHeight));

	const wxSize parentSize = parent->GetClientSize();
	SetPosition(wxPoint((parentSize.x - kPanelWidth) / 2, 60));

	m_sizer = new wxBoxSizer(wxVERTICAL);

	BuildLayout();
	PopulateMenus();
	ApplyTheme();

	const std::array<wxAcceleratorEntry, 1> entries = {{
		{wxACCEL_NORMAL, WXK_ESCAPE, +Event::ControlPanel::Exit},
	}};
	SetAcceleratorTable(wxAcceleratorTable(entries.size(), entries.data()));

	SetSizer(m_sizer);
	SetSize(wxSize(kPanelWidth, kPanelHeight));

	if (m_searchInput) {
		m_searchInput->Bind(wxEVT_KEY_DOWN, &ControlPanel::OnSearchKeyDown,
		                    this);
		m_searchInput->SetFocus();
	}
}

void ControlPanel::BuildLayout() {
	auto *topContainer = new wxPanel(this);
	topContainer->SetBackgroundColour(m_bgColor);

	auto *topSizer = new wxBoxSizer(wxHORIZONTAL);

	wxBitmap searchBmp;
	if (wxFileExists(m_iconsDir + "search_gray.png")) {
		searchBmp.LoadFile(m_iconsDir + "search_gray.png", wxBITMAP_TYPE_PNG);
	}

	auto *searchIcon = new wxStaticBitmap(
		topContainer, wxID_ANY, wxBitmapBundle::FromBitmap(searchBmp));
	searchIcon->SetBackgroundColour(m_bgColor);
	topSizer->Add(searchIcon, 0,
	              wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, kSearchPadding);

	m_searchInput = new wxTextCtrl(topContainer, wxID_ANY, wxEmptyString,
	                               wxDefaultPosition, wxDefaultSize,
	                               wxBORDER_NONE | wxTE_PROCESS_ENTER);
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

	auto *separator = new wxPanel(this, wxID_ANY, wxDefaultPosition,
	                              wxSize(-1, 1));
	separator->SetBackgroundColour(m_borderColor);
	m_sizer->Add(separator, 0, wxEXPAND);

	m_menusContainer = new wxScrolledWindow(this, wxID_ANY, wxDefaultPosition,
	                                        wxDefaultSize,
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

		containerSizer->Add(row, 0, wxEXPAND | wxTOP | wxBOTTOM, kRowSpacing);

		if (!firstSelected) {
			HighlightRow(row);
			m_selectedMenu = row;
			firstSelected  = true;
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

	auto *nameText = new wxStaticText(row, wxID_ANY, menu.name);
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
	rowSizer->Add(shortcutText, 0,
	              wxALIGN_CENTER_VERTICAL | wxRIGHT, kRowPadding);

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

	switch (commandId) {
	case 1: {
		auto *splitter = dynamic_cast<wxSplitterWindow *>(
			FindWindowById(+GUI::ControlID::MainContainerSplitter));
		auto *content = FindWindowById(+GUI::ControlID::CenteredContent);
		auto *terminal = FindWindowById(+GUI::ControlID::Terminal);

		if (splitter && content && terminal)
			splitter->SplitHorizontally(content, terminal, 0);
	} break;

	case 2: {
		if (auto *mainFrame =
		        dynamic_cast<MainFrame *>(wxTheApp->GetTopWindow())) {
			wxCommandEvent evt(wxEVT_MENU, +Event::View::ToggleMenuBar);
			wxPostEvent(mainFrame, evt);
		}
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

	const wxRect rowRect    = row->GetRect();
	const wxSize clientSize = m_menusContainer->GetClientSize();

	int scrollX = 0;
	int scrollY = 0;
	m_menusContainer->GetViewStart(&scrollX, &scrollY);

	int ppuX = 0;
	int ppuY = 0;
	m_menusContainer->GetScrollPixelsPerUnit(&ppuX, &ppuY);

	if (ppuY <= 0)
		return;

	const int topPx    = scrollY * ppuY;
	const int bottomPx = topPx + clientSize.y;

	if (rowRect.GetTop() < topPx)
		m_menusContainer->Scroll(-1, rowRect.GetTop() / ppuY);
	else if (rowRect.GetBottom() > bottomPx)
		m_menusContainer->Scroll(-1, (rowRect.GetBottom() - clientSize.y) / ppuY);
}