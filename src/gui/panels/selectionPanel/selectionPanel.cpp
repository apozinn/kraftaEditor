#include "selectionPanel.hpp"

#include "appConstants/appConstants.hpp"
#include "appPaths/appPaths.hpp"
#include "themesManager/themesManager.hpp"

#include <nlohmann/json.hpp>
#include <vector>
#include <wx/app.h>
#include <wx/dcbuffer.h>
#include <wx/filename.h>
#include <wx/weakref.h>

namespace {

constexpr int kPanelWidth = 480;
constexpr int kPanelHeight = 280;
constexpr int kRowPadding = 8;
constexpr int kRowSpacing = 2;
constexpr int kSearchPadding = 8;

// Posição do mouse na última navegação por teclado / reconstrução da lista.
// No Windows, rolar ou refazer o layout com o cursor parado gera
// ENTER_WINDOW nas linhas que passam por baixo dele; sem este filtro o hover
// "rouba" a seleção de volta e as setas parecem não funcionar.
wxPoint g_lastNavMouse(-1, -1);

void NoteKeyboardNavigation() { g_lastNavMouse = wxGetMousePosition(); }

void RestoreMainFrameFocus() {
	if (auto *top = wxTheApp->GetTopWindow())
		top->SetFocus();
}

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
	wxFont font(size, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, weight);
#if defined(__WXMSW__)
	font.SetFaceName("Consolas");
#elif defined(__WXOSX__)
	font.SetFaceName("Menlo");
#else
	font.SetFaceName("Monospace");
#endif
	return font;
}

// Próxima linha VISÍVEL (circular) a partir de `current`.
// Com current == nullptr devolve a primeira (forward) ou a última (backward).
wxPanel *StepRow(wxWindow *container, wxPanel *current, bool forward) {
	std::vector<wxPanel *> rows;
	for (auto *child : container->GetChildren())
		if (auto *row = dynamic_cast<wxPanel *>(child))
			rows.push_back(row);

	const int count = static_cast<int>(rows.size());
	if (count == 0)
		return nullptr;

	int index = -1;
	for (int i = 0; i < count; ++i) {
		if (rows[i] == current) {
			index = i;
			break;
		}
	}
	if (index < 0)
		index = forward ? -1 : 0;

	for (int step = 1; step <= count; ++step) {
		const int pos = forward ? (index + step) % count
								: ((index - step) % count + count) % count;
		if (rows[pos]->IsShown())
			return rows[pos];
	}
	return nullptr;
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
	SetSize(wxSize(kPanelWidth, kPanelHeight));
	SetMinSize(wxSize(kPanelWidth, kPanelHeight));

	const wxSize parentSize = parent->GetClientSize();
	SetPosition(wxPoint((parentSize.x - kPanelWidth) / 2, 60));

	// Mesma ordem do ControlPanel: sizer ligado antes de montar o layout
	m_sizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(m_sizer);

	BuildLayout();
	ApplyTheme();

	if (m_searchInput) {
		m_searchInput->Bind(wxEVT_KEY_DOWN, &SelectionPanel::OnSearchKeyDown,
							this);
	}
	Bind(wxEVT_KEY_DOWN, &SelectionPanel::OnKeyDown, this);

	// No Windows o SetSize com o mesmo tamanho não dispara relayout
	m_sizer->Layout();
	Layout();

	// Ignora o hover do cursor que já está parado em cima do painel
	NoteKeyboardNavigation();

	// Foco adiado: quem abriu este painel (ex.: ControlPanel) costuma chamar
	// Destroy() + GetTopWindow()->SetFocus() logo depois, o que tirava o foco
	// do campo de busca no Windows. O CallAfter roda depois disso.
	CallAfter([this]() {
		if (m_searchInput)
			m_searchInput->SetFocus();
	});
}

void SelectionPanel::BuildLayout() {
	auto *topContainer = new wxPanel(this);
	topContainer->SetBackgroundColour(m_bgColor);

	auto *topSizer = new wxBoxSizer(wxHORIZONTAL);

	// wxFileName monta o caminho com o separador certo (o `+` simples quebrava
	// no Windows quando m_iconsDir não termina em separador)
	wxBitmap searchBmp;
	const wxString searchIconPath =
		wxFileName(m_iconsDir, "search_gray.png").GetFullPath();
	if (wxFileExists(searchIconPath))
		searchBmp.LoadFile(searchIconPath, wxBITMAP_TYPE_PNG);

	if (searchBmp.IsOk()) {
		auto *searchIcon = new wxStaticBitmap(
			topContainer, wxID_ANY, wxBitmapBundle::FromBitmap(searchBmp));
		searchIcon->SetBackgroundColour(m_bgColor);
		topSizer->Add(searchIcon, 0, wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT,
					  kSearchPadding);
	} else {
		topSizer->AddSpacer(kSearchPadding);
	}

	m_searchInput =
		new wxTextCtrl(topContainer, wxID_ANY, wxEmptyString, wxDefaultPosition,
					   wxDefaultSize, wxBORDER_NONE | wxTE_PROCESS_ENTER);
	m_searchInput->SetBackgroundColour(m_bgColor);
	m_searchInput->SetForegroundColour(m_textColor);
	m_searchInput->SetFont(MakeMonospaceFont(11));
	m_searchInput->SetMinSize(wxSize(-1, 24));
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

	containerSizer->Layout();
	m_optionsContainer->Layout();
	m_optionsContainer->FitInside();
	m_optionsContainer->Scroll(0, 0);
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
		descText->Bind(wxEVT_LEFT_UP, &SelectionPanel::OnRowClick, this);
		rowSizer->Add(descText, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT,
					  kRowPadding);
	}

	row->SetSizer(rowSizer);

	row->Bind(wxEVT_LEFT_UP, &SelectionPanel::OnRowClick, this);
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

	if (m_sizer)
		m_sizer->Layout();
	Layout();

	// As linhas foram recriadas, possivelmente debaixo do cursor
	NoteKeyboardNavigation();

	if (m_searchInput) {
		m_searchInput->ChangeValue(wxEmptyString);
		CallAfter([this]() {
			if (m_searchInput)
				m_searchInput->SetFocus();
		});
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

	auto *nextPanel = StepRow(m_optionsContainer, m_selectedOption, false);
	if (!nextPanel || nextPanel == m_selectedOption)
		return;

	NoteKeyboardNavigation();
	UpdateSelectionVisual(m_selectedOption, nextPanel);
	m_selectedOption = nextPanel;
	ScrollSelectionIntoView(nextPanel);
}

void SelectionPanel::DownSelection() {
	if (!m_optionsContainer || !m_selectedOption)
		return;

	auto *nextPanel = StepRow(m_optionsContainer, m_selectedOption, true);
	if (!nextPanel || nextPanel == m_selectedOption)
		return;

	NoteKeyboardNavigation();
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
		RestoreMainFrameFocus();
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
	case WXK_NUMPAD_ENTER: {
		if (!m_selectedOption)
			return; // busca sem resultado: não fecha o painel

		// O callback pode destruir este painel; evita destruir duas vezes
		wxWeakRef<wxWindow> self(this);
		SelectCurrent();
		if (self) {
			Destroy();
			RestoreMainFrameFocus();
		}
		return;
	}

	case WXK_ESCAPE:
		Destroy();
		RestoreMainFrameFocus();
		return;

	default:
		break;
	}

	event.Skip();
}

void SelectionPanel::OnSearchModified(wxCommandEvent &WXUNUSED(event)) {
	if (!m_optionsContainer || !m_searchInput)
		return;

	NoteKeyboardNavigation();

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

	if (auto *containerSizer = m_optionsContainer->GetSizer())
		containerSizer->Layout();
	m_optionsContainer->Layout();
	m_optionsContainer->FitInside();
	m_optionsContainer->Scroll(0, 0);

	// Se a linha selecionada sumiu, a seleção vai para a primeira visível;
	// sem isso as setas partiam de uma linha oculta e "não faziam nada".
	if (!m_selectedOption || !m_selectedOption->IsShown()) {
		ClearRowHighlight(m_selectedOption);
		m_selectedOption = StepRow(m_optionsContainer, nullptr, true);
		HighlightRow(m_selectedOption);
	}

	if (m_sizer)
		m_sizer->Layout();
	Layout();
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
	event.Skip();

	// ENTER_WINDOW sem o mouse ter se mexido = a lista rolou/mudou debaixo do
	// cursor parado: não deixa o hover desfazer a navegação por teclado.
	if (wxGetMousePosition() == g_lastNavMouse)
		return;

	auto *obj = dynamic_cast<wxWindow *>(event.GetEventObject());
	if (!obj)
		return;

	auto *row = obj;
	while (row && !dynamic_cast<wxPanel *>(row))
		row = row->GetParent();

	auto *rowPanel = dynamic_cast<wxPanel *>(row);
	if (!rowPanel || rowPanel == m_selectedOption)
		return;

	UpdateSelectionVisual(m_selectedOption, rowPanel);
	m_selectedOption = rowPanel;
}

void SelectionPanel::OnRowLeave(wxMouseEvent &event) { event.Skip(); }

void SelectionPanel::ScrollSelectionIntoView(wxWindow *row) {
	if (!m_optionsContainer || !row)
		return;

	int ppuX = 0;
	int ppuY = 0;
	m_optionsContainer->GetScrollPixelsPerUnit(&ppuX, &ppuY);
	if (ppuY <= 0)
		return;

	// GetPosition() é relativo à área visível; converte para coordenadas
	// absolutas da lista antes de comparar com a janela de visualização.
	const wxPoint absPos =
		m_optionsContainer->CalcUnscrolledPosition(row->GetPosition());
	const int rowTop = absPos.y;
	const int rowBottom = rowTop + row->GetSize().GetHeight();

	int viewX = 0;
	int viewY = 0;
	m_optionsContainer->GetViewStart(&viewX, &viewY);

	const int viewHeight = m_optionsContainer->GetClientSize().GetHeight();
	const int viewTop = viewY * ppuY;
	const int viewBottom = viewTop + viewHeight;

	if (rowTop < viewTop) {
		m_optionsContainer->Scroll(-1, rowTop / ppuY);
	} else if (rowBottom > viewBottom) {
		const int newTop = rowBottom - viewHeight;
		m_optionsContainer->Scroll(-1, (newTop + ppuY - 1) / ppuY);
	}
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