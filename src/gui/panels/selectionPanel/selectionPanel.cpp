#include "selectionPanel.hpp"

#include "appConstants/appConstants.hpp"
#include "appPaths/appPaths.hpp"
#include "themesManager/themesManager.hpp"

#include <nlohmann/json.hpp>
#include <wx/dcbuffer.h>

namespace {

constexpr int kPanelWidth = 480;
constexpr int kPanelHeight = 280;
constexpr int kRowPadding = 8;
constexpr int kRowSpacing = 2;
constexpr int kSearchPadding = 8;

wxColour GetThemeColor(const nlohmann::json &theme, const char *key,
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

SelectionPanel::SelectionPanel(wxFrame *parent, wxWindowID id)
	: wxPanel(parent, id, wxDefaultPosition, wxSize(kPanelWidth, kPanelHeight),
			  wxBORDER_SIMPLE | wxTAB_TRAVERSAL),
	  m_parent(parent) {
	if (!parent) {
		wxLogError(_("SelectionPanel: parent frame is null"));
		return;
	}

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
	SetPosition(wxPoint((parentSize.x - kPanelWidth) / 2, 60));

	m_sizer = new wxBoxSizer(wxVERTICAL);

	BuildLayout();
	ApplyTheme();

	SetSizer(m_sizer);
	SetSize(wxSize(kPanelWidth, kPanelHeight));

	if (m_searchInput) {
		m_searchInput->Bind(wxEVT_KEY_DOWN, &SelectionPanel::OnSearchKeyDown,
							this);
		m_searchInput->SetFocus();
	}

	Bind(wxEVT_KEY_DOWN, &SelectionPanel::OnKeyDown, this);
}

void SelectionPanel::BuildLayout() {
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
	topSizer->Add(searchIcon, 0, wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT,
				  kSearchPadding);

	m_searchInput =
		new wxTextCtrl(topContainer, wxID_ANY, wxEmptyString, wxDefaultPosition,
					   wxDefaultSize, wxBORDER_NONE | wxTE_PROCESS_ENTER);
	m_searchInput->SetBackgroundColour(m_bgColor);
	m_searchInput->SetForegroundColour(m_textColor);
	m_searchInput->SetFont(MakeMonospaceFont(11));
	m_searchInput->Bind(wxEVT_TEXT, &SelectionPanel::OnSearchModified, this);
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

	m_optionsContainer =
		new wxScrolledWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
							 wxVSCROLL | wxBORDER_NONE);
	m_optionsContainer->SetBackgroundColour(m_bgColor);
	m_optionsContainer->SetScrollRate(0, 15);
	m_optionsContainer->SetSizer(new wxBoxSizer(wxVERTICAL));

	m_sizer->Add(m_optionsContainer, 1, wxEXPAND);
}

void SelectionPanel::BuildOptions(
	const std::vector<OptionsListStruct> &optionsList) {
	if (!m_optionsContainer)
		return;

	auto *containerSizer = m_optionsContainer->GetSizer();
	if (!containerSizer)
		return;

	containerSizer->Clear(true);

	m_selectedOption = nullptr;

	bool firstSelected = false;

	for (const auto &option : optionsList) {
		auto *row = CreateOptionRow(option);
		if (!row)
			continue;

		containerSizer->Add(row, 0, wxEXPAND | wxTOP | wxBOTTOM, kRowSpacing);

		if (!firstSelected) {
			HighlightRow(row);
			m_selectedOption = row;
			firstSelected = true;
		}
	}

	m_optionsContainer->FitInside();
	m_optionsContainer->Layout();
}

wxPanel *SelectionPanel::CreateOptionRow(const OptionsListStruct &option) {
	if (!m_optionsContainer)
		return nullptr;

	auto *row = new wxPanel(m_optionsContainer, wxID_ANY);
	row->SetBackgroundColour(m_bgColor);
	row->SetLabel(wxString::Format("%d", option.id));
	row->SetName(wxString::Format("%d", option.id));

	auto *rowSizer = new wxBoxSizer(wxHORIZONTAL);

	auto *titleText = new wxStaticText(row, wxID_ANY, option.title);
	titleText->SetName(wxString::Format("%d", option.id));
	titleText->SetFont(MakeMonospaceFont(10));
	titleText->SetForegroundColour(m_textColor);
	titleText->SetBackgroundColour(m_bgColor);
	titleText->Bind(wxEVT_LEFT_UP, &SelectionPanel::OnRowClick, this);
	titleText->Bind(wxEVT_ENTER_WINDOW, &SelectionPanel::OnRowEnter, this);
	titleText->Bind(wxEVT_LEAVE_WINDOW, &SelectionPanel::OnRowLeave, this);
	rowSizer->Add(titleText, 1, wxALIGN_CENTER_VERTICAL | wxALL, kRowPadding);

	if (!option.description.empty()) {
		auto *descText = new wxStaticText(
			row, wxID_ANY, wxString::FromUTF8(option.description));
		descText->SetFont(MakeMonospaceFont(8));
		descText->SetForegroundColour(m_borderColor);
		descText->SetBackgroundColour(m_bgColor);
		rowSizer->Add(descText, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT,
					  kRowPadding);
	}

	row->SetSizer(rowSizer);

	row->Bind(wxEVT_ENTER_WINDOW, &SelectionPanel::OnRowEnter, this);
	row->Bind(wxEVT_LEAVE_WINDOW, &SelectionPanel::OnRowLeave, this);

	return row;
}

void SelectionPanel::ApplyTheme() {
	SetBackgroundColour(m_bgColor);

	if (m_searchInput) {
		m_searchInput->SetBackgroundColour(m_bgColor);
		m_searchInput->SetForegroundColour(m_textColor);
	}

	if (m_optionsContainer)
		m_optionsContainer->SetBackgroundColour(m_bgColor);

	Refresh();
}

void SelectionPanel::ShowList(
	const std::vector<OptionsListStruct> &optionsList,
	std::function<void(const OptionsListStruct &)> opt) {
	m_optionsList = optionsList;
	m_onSelect = std::move(opt);

	BuildOptions(optionsList);
	Layout();

	if (m_searchInput) {
		m_searchInput->ChangeValue(wxEmptyString);
		m_searchInput->SetFocus();
	}
}

void SelectionPanel::SelectCurrent() {
	if (!m_selectedOption)
		return;

	const wxString id = m_selectedOption->GetLabel();
	if (id.IsEmpty())
		return;

	long value = 0;
	if (!id.ToLong(&value))
		return;

	const int selectedId = static_cast<int>(value);

	for (const auto &option : m_optionsList) {
		if (option.id == selectedId) {
			if (m_onSelect)
				m_onSelect(option);
			return;
		}
	}
}

void SelectionPanel::UpSelection() {
	if (!m_optionsContainer || !m_selectedOption)
		return;

	const auto &children = m_optionsContainer->GetChildren();
	if (children.IsEmpty())
		return;

	wxWindow *nextRow = m_selectedOption->GetPrevSibling();
	if (!nextRow)
		nextRow = children[children.GetCount() - 1];

	auto *nextPanel = dynamic_cast<wxPanel *>(nextRow);
	if (!nextPanel || nextPanel == m_selectedOption)
		return;

	UpdateSelectionVisual(m_selectedOption, nextPanel);
	m_selectedOption = nextPanel;
	ScrollSelectionIntoView(nextPanel);
}

void SelectionPanel::DownSelection() {
	if (!m_optionsContainer || !m_selectedOption)
		return;

	const auto &children = m_optionsContainer->GetChildren();
	if (children.IsEmpty())
		return;

	wxWindow *nextRow = m_selectedOption->GetNextSibling();
	if (!nextRow)
		nextRow = children[0];

	auto *nextPanel = dynamic_cast<wxPanel *>(nextRow);
	if (!nextPanel || nextPanel == m_selectedOption)
		return;

	UpdateSelectionVisual(m_selectedOption, nextPanel);
	m_selectedOption = nextPanel;
	ScrollSelectionIntoView(nextPanel);
}

void SelectionPanel::OnKeyDown(wxKeyEvent &event) {
	switch (event.GetKeyCode()) {
	case WXK_UP:
		UpSelection();
		return;

	case WXK_DOWN:
		DownSelection();
		return;

	case WXK_RETURN:
	case WXK_NUMPAD_ENTER:
		SelectCurrent();
		return;

	case WXK_ESCAPE:
		Destroy();
		if (wxTheApp->GetTopWindow())
			wxTheApp->GetTopWindow()->SetFocus();
		return;

	default:
		break;
	}

	event.Skip();
}

void SelectionPanel::OnSearchKeyDown(wxKeyEvent &event) {
	switch (event.GetKeyCode()) {
	case WXK_UP:
		UpSelection();
		return;

	case WXK_DOWN:
		DownSelection();
		return;

	case WXK_RETURN:
	case WXK_NUMPAD_ENTER:
		SelectCurrent();
		Destroy();
		if (wxTheApp->GetTopWindow())
			wxTheApp->GetTopWindow()->SetFocus();
		return;

	case WXK_ESCAPE:
		Destroy();
		if (wxTheApp->GetTopWindow())
			wxTheApp->GetTopWindow()->SetFocus();
		return;

	default:
		break;
	}

	event.Skip();
}

void SelectionPanel::OnSearchModified(wxCommandEvent &WXUNUSED(event)) {
	if (!m_optionsContainer || !m_searchInput)
		return;

	const wxString query = m_searchInput->GetValue().Lower();

	for (auto *child : m_optionsContainer->GetChildren()) {
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

	if (auto *containerSizer = m_optionsContainer->GetSizer()) {
		containerSizer->Layout();
		m_optionsContainer->FitInside();
	}
}

void SelectionPanel::OnRowClick(wxMouseEvent &event) {
	auto *obj = dynamic_cast<wxWindow *>(event.GetEventObject());
	if (!obj)
		return;

	auto *row = obj;
	while (row && !dynamic_cast<wxPanel *>(row))
		row = row->GetParent();

	auto *rowPanel = dynamic_cast<wxPanel *>(row);
	if (!rowPanel)
		return;

	UpdateSelectionVisual(m_selectedOption, rowPanel);
	m_selectedOption = rowPanel;

	SelectCurrent();

	event.Skip();
}

void SelectionPanel::OnRowEnter(wxMouseEvent &event) {
	auto *obj = dynamic_cast<wxWindow *>(event.GetEventObject());
	if (!obj)
		return;

	auto *row = obj;
	while (row && !dynamic_cast<wxPanel *>(row))
		row = row->GetParent();

	auto *rowPanel = dynamic_cast<wxPanel *>(row);
	if (!rowPanel || rowPanel == m_selectedOption) {
		event.Skip();
		return;
	}

	UpdateSelectionVisual(m_selectedOption, rowPanel);
	m_selectedOption = rowPanel;

	event.Skip();
}

void SelectionPanel::OnRowLeave(wxMouseEvent &event) { event.Skip(); }

void SelectionPanel::ScrollSelectionIntoView(wxWindow *row) {
	if (!m_optionsContainer || !row)
		return;

	const wxRect rowRect = row->GetRect();
	const wxSize clientSize = m_optionsContainer->GetClientSize();

	int scrollX = 0;
	int scrollY = 0;
	m_optionsContainer->GetViewStart(&scrollX, &scrollY);

	int ppuX = 0;
	int ppuY = 0;
	m_optionsContainer->GetScrollPixelsPerUnit(&ppuX, &ppuY);

	if (ppuY <= 0)
		return;

	const int topPx = scrollY * ppuY;
	const int bottomPx = topPx + clientSize.y;

	if (rowRect.GetTop() < topPx)
		m_optionsContainer->Scroll(-1, rowRect.GetTop() / ppuY);
	else if (rowRect.GetBottom() > bottomPx)
		m_optionsContainer->Scroll(-1,
								   (rowRect.GetBottom() - clientSize.y) / ppuY);
}

void SelectionPanel::HighlightRow(wxPanel *row) {
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

void SelectionPanel::ClearRowHighlight(wxPanel *row) {
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

void SelectionPanel::UpdateSelectionVisual(wxPanel *oldRow, wxPanel *newRow) {
	if (oldRow == newRow)
		return;

	ClearRowHighlight(oldRow);
	HighlightRow(newRow);
}