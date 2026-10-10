#include "quickOpen.hpp"
#include "gui/panels/filesTree/filesTree.hpp"
#include "projectSettings/projectSettings.hpp"

#include <algorithm>
#include <wx/dcbuffer.h>

namespace {

constexpr int kRowHeight = 24;
constexpr int kRowPaddingX = 8;
constexpr int kNameFontSize = 10;
constexpr int kPathFontSize = 8;
constexpr int kSearchFontSize = 11;
constexpr int kMaxFiles = 5000;

const std::unordered_set<std::string> kIgnoredDirs = {
	".git",
	".svn",
	".hg",
	"build",
	"dist",
	"out",
	"node_modules",
	".vscode",
	".idea",
	"__pycache__",
	"cmake-build-debug",
	"cmake-build-release",
};

wxColour GetThemeColor(const ThemesManager &theme, const char *key,
					   const wxColour &fallback) {
	try {
		return wxColour(theme.GetColor(key));
	} catch (...) {
		return fallback;
	}
}

} // namespace

QuickOpen::QuickOpen(wxFrame *parent)
	: wxPanel(parent, +GUI::ControlID::QuickOpen, wxDefaultPosition,
			  wxSize(kPanelWidth, kPanelHeight),
			  wxBORDER_SIMPLE | wxTAB_TRAVERSAL) {
	if (!parent)
		return;

	const auto &theme = ThemesManager::Get();
	m_bgColor = GetThemeColor(theme, "colorThree", wxColour(30, 30, 30));
	m_searchBgColor = GetThemeColor(theme, "secondary", wxColour(40, 40, 40));
	m_textColor = GetThemeColor(theme, "text", *wxWHITE);
	m_secondaryTextColor =
		GetThemeColor(theme, "secondaryText", wxColour(120, 120, 120));
	m_highlightColor = GetThemeColor(theme, "highlight", wxColour(60, 60, 60));

	m_nameFont = MakeMonospaceFont(kNameFontSize);
	m_pathFont = MakeMonospaceFont(kPathFontSize);

	SetBackgroundColour(m_bgColor);
	SetMinSize(wxSize(kPanelWidth, kPanelHeight));

	const wxSize parentSize = parent->GetClientSize();
	SetPosition(wxPoint((parentSize.x - kPanelWidth) / 2, 50));

	m_sizer = new wxBoxSizer(wxVERTICAL);

	BuildLayout();
	IndexProjectFiles();
	ApplyFilter(wxEmptyString);

	SetSizer(m_sizer);
	SetSize(wxSize(kPanelWidth, kPanelHeight));

	if (m_searchBar) {
		m_searchBar->Bind(wxEVT_KEY_DOWN, &QuickOpen::OnSearchKeyDown, this);
		m_searchBar->SetFocus();
	}
}

void QuickOpen::BuildLayout() {
	m_topContainer = new wxPanel(this, wxID_ANY);
	m_topContainer->SetBackgroundColour(m_searchBgColor);

	m_topSizer = new wxBoxSizer(wxHORIZONTAL);

	m_searchBar = new wxTextCtrl(m_topContainer, wxID_ANY, wxEmptyString,
								 wxDefaultPosition, wxDefaultSize,
								 wxBORDER_NONE | wxTE_PROCESS_ENTER);
	m_searchBar->SetBackgroundColour(m_searchBgColor);
	m_searchBar->SetForegroundColour(m_textColor);
	m_searchBar->SetFont(MakeMonospaceFont(kSearchFontSize));
	m_searchBar->Bind(wxEVT_TEXT, &QuickOpen::OnSearchBarChange, this);
	m_topSizer->Add(m_searchBar, 1, wxEXPAND | wxALL, kSearchPadding);

	m_topContainer->SetSizer(m_topSizer);
	m_sizer->Add(m_topContainer, 0, wxEXPAND);

	m_list = new wxScrolledWindow(this, wxID_ANY, wxDefaultPosition,
								  wxDefaultSize, wxVSCROLL | wxBORDER_NONE);
	m_list->SetBackgroundStyle(wxBG_STYLE_PAINT);
	m_list->SetBackgroundColour(m_bgColor);
	m_list->SetScrollRate(0, kRowHeight);

	m_list->Bind(wxEVT_PAINT, &QuickOpen::OnListPaint, this);
	m_list->Bind(wxEVT_MOTION, &QuickOpen::OnListMouse, this);
	m_list->Bind(wxEVT_LEFT_DOWN, &QuickOpen::OnListLeftDown, this);
	m_list->Bind(wxEVT_LEAVE_WINDOW, [this](wxMouseEvent &e) {
		m_hoverIndex = -1;
		m_list->Refresh();
		e.Skip();
	});
	m_list->Bind(wxEVT_SIZE, &QuickOpen::OnListSize, this);

	m_sizer->Add(m_list, 1, wxEXPAND);
}

void QuickOpen::IndexProjectFiles() {
	namespace fs = std::filesystem;

	const wxString projectPath = ProjectSettings::Get().GetProjectPath();
	if (projectPath.IsEmpty())
		return;

	std::error_code ec;
	const fs::path root(projectPath.ToStdWstring());

	if (!fs::exists(root, ec) || !fs::is_directory(root, ec))
		return;

	fs::recursive_directory_iterator it(
		root, fs::directory_options::skip_permission_denied, ec);
	fs::recursive_directory_iterator end;

	if (ec)
		return;

	m_files.reserve(1024);

	for (; it != end && m_files.size() < kMaxFiles; it.increment(ec)) {
		if (ec) {
			ec.clear();
			continue;
		}

		const fs::directory_entry &entry = *it;

		if (entry.is_directory(ec)) {
			const auto dirName = entry.path().filename().string();
			if (kIgnoredDirs.contains(dirName))
				it.disable_recursion_pending();
			continue;
		}

		if (!entry.is_regular_file(ec))
			continue;

		QuickOpenFileStruct file;
		file.name = wxString::FromUTF8(entry.path().filename().string());
		file.path = wxString::FromUTF8(entry.path().string());

		if (file.path.IsEmpty())
			continue;

		m_files.push_back(std::move(file));
	}
}

void QuickOpen::ApplyFilter(const wxString &query) {
	m_visibleIndices.clear();
	m_visibleIndices.reserve(m_files.size());

	const wxString lowerQuery = query.Lower();

	for (size_t i = 0; i < m_files.size(); ++i) {
		const auto &file = m_files[i];

		const bool matches = lowerQuery.IsEmpty() ||
							 file.path.Lower().Contains(lowerQuery) ||
							 file.name.Lower().Contains(lowerQuery);

		if (matches)
			m_visibleIndices.push_back(i);
	}

	m_selectedIndex = m_visibleIndices.empty() ? -1 : 0;
	m_hoverIndex = -1;

	const int totalHeight =
		static_cast<int>(m_visibleIndices.size()) * m_rowHeight;
	const int virtualW = m_list->GetClientSize().x;
	m_list->SetVirtualSize(virtualW, totalHeight);
	m_list->Scroll(0, 0);

	m_list->Refresh();
	ScrollSelectionIntoView();
}

void QuickOpen::MoveSelection(int direction) {
	if (m_visibleIndices.empty())
		return;

	const int total = static_cast<int>(m_visibleIndices.size());
	int next = m_selectedIndex + direction;

	if (next < 0)
		next = total - 1;
	else if (next >= total)
		next = 0;

	m_selectedIndex = next;

	m_list->Refresh();
	ScrollSelectionIntoView();
}

void QuickOpen::ScrollSelectionIntoView() {
	if (m_selectedIndex < 0 || !m_list)
		return;

	const int rowTop = m_selectedIndex * m_rowHeight;
	const int rowBottom = rowTop + m_rowHeight;

	int scrollX = 0, scrollY = 0;
	m_list->GetViewStart(&scrollX, &scrollY);

	int ppuX = 0, ppuY = 0;
	m_list->GetScrollPixelsPerUnit(&ppuX, &ppuY);

	if (ppuY <= 0)
		return;

	const int topPx = scrollY * ppuY;
	const int bottomPx = topPx + m_list->GetClientSize().y;

	if (rowTop < topPx)
		m_list->Scroll(-1, rowTop / ppuY);
	else if (rowBottom > bottomPx)
		m_list->Scroll(-1, (rowBottom - m_list->GetClientSize().y) / ppuY);
}

void QuickOpen::OnListPaint(wxPaintEvent &WXUNUSED(event)) {
	wxAutoBufferedPaintDC dc(m_list);
	dc.SetBackground(wxBrush(m_bgColor));
	dc.Clear();

	if (m_visibleIndices.empty())
		return;

	const wxSize clientSize = m_list->GetClientSize();

	int scrollX = 0, scrollY = 0;
	m_list->GetViewStart(&scrollX, &scrollY);

	int ppuX = 0, ppuY = 0;
	m_list->GetScrollPixelsPerUnit(&ppuX, &ppuY);

	const int topPx = scrollY * ppuY;

	const int firstRow = topPx / m_rowHeight;
	const int lastRow = std::min(static_cast<int>(m_visibleIndices.size()),
								 (topPx + clientSize.y) / m_rowHeight + 2);

	for (int i = firstRow; i < lastRow; ++i) {
		const int y = i * m_rowHeight - topPx;
		const wxRect rowRect(0, y, clientSize.x, m_rowHeight);

		if (i == m_selectedIndex)
			dc.SetBrush(wxBrush(m_highlightColor));
		else if (i == m_hoverIndex)
			dc.SetBrush(wxBrush(m_highlightColor.ChangeLightness(80)));
		else
			dc.SetBrush(wxBrush(m_bgColor));

		dc.SetPen(*wxTRANSPARENT_PEN);
		dc.DrawRectangle(rowRect);

		const auto &file = m_files[m_visibleIndices[i]];
		dc.SetFont(m_nameFont);
		dc.SetTextForeground(m_textColor);

		wxCoord nameW = 0, nameH = 0;
		dc.GetTextExtent(file.name, &nameW, &nameH);

		dc.DrawText(file.name, kRowPaddingX, y + (m_rowHeight - nameH) / 2);
		dc.SetFont(m_pathFont);
		dc.SetTextForeground(m_secondaryTextColor);

		wxCoord pathW = 0, pathH = 0;
		dc.GetTextExtent(file.path, &pathW, &pathH);

		const int pathX = clientSize.x - pathW - kRowPaddingX;
		if (pathX > nameW + kRowPaddingX * 2) {
			dc.DrawText(file.path, pathX, y + (m_rowHeight - pathH) / 2);
		}
	}
}

int QuickOpen::HitTestRow(int y) const {
	if (m_visibleIndices.empty() || !m_list)
		return -1;

	int scrollX = 0, scrollY = 0;
	m_list->GetViewStart(&scrollX, &scrollY);

	int ppuX = 0, ppuY = 0;
	m_list->GetScrollPixelsPerUnit(&ppuX, &ppuY);

	const int topPx = scrollY * ppuY;
	const int absoluteY = y + topPx;
	const int row = absoluteY / m_rowHeight;

	if (row < 0 || row >= static_cast<int>(m_visibleIndices.size()))
		return -1;

	return row;
}

void QuickOpen::OnListMouse(wxMouseEvent &event) {
	const int row = HitTestRow(event.GetY());

	if (row != m_hoverIndex) {
		m_hoverIndex = row;
		m_list->Refresh();
	}

	event.Skip();
}

void QuickOpen::OnListLeftDown(wxMouseEvent &event) {
	const int row = HitTestRow(event.GetY());
	if (row < 0)
		return;

	m_selectedIndex = row;
	OpenFileAt(row);
}

void QuickOpen::OnListSize(wxSizeEvent &event) {
	const int totalHeight =
		static_cast<int>(m_visibleIndices.size()) * m_rowHeight;
	m_list->SetVirtualSize(event.GetSize().x, totalHeight);
	event.Skip();
}

void QuickOpen::OpenFileAt(int index) {
	if (index < 0 || index >= static_cast<int>(m_visibleIndices.size()))
		return;

	const auto &file = m_files[m_visibleIndices[index]];

	if (auto *filesTree =
			dynamic_cast<FilesTree *>(wxTheApp->GetTopWindow()->FindWindowById(
				+GUI::ControlID::FilesTree))) {
		filesTree->OpenFile(file.path);
	}

	Destroy();
	wxTheApp->GetTopWindow()->SetFocus();
}

void QuickOpen::OnSearchBarChange(wxCommandEvent &WXUNUSED(event)) {
	if (!m_searchBar)
		return;

	ApplyFilter(m_searchBar->GetValue());
}

void QuickOpen::OnSearchKeyDown(wxKeyEvent &event) {
	switch (event.GetKeyCode()) {
	case WXK_UP:
		MoveSelection(-1);
		return;
	case WXK_DOWN:
		MoveSelection(1);
		return;
	case WXK_RETURN:
	case WXK_NUMPAD_ENTER:
		OpenFileAt(m_selectedIndex);
		return;
	case WXK_ESCAPE: {
		wxCommandEvent evt(wxEVT_MENU, +Event::QuickOpen::Exit);
		Close(evt);
		return;
	}
	default:
		break;
	}

	event.Skip();
}

void QuickOpen::OnKeyboardUp(wxCommandEvent &WXUNUSED(event)) {
	MoveSelection(-1);
}

void QuickOpen::OnKeyboardDown(wxCommandEvent &WXUNUSED(event)) {
	MoveSelection(1);
}

void QuickOpen::OnKeyboardEnter(wxCommandEvent &WXUNUSED(event)) {
	OpenFileAt(m_selectedIndex);
}

void QuickOpen::Close(wxCommandEvent &WXUNUSED(event)) {
	Destroy();
	wxTheApp->GetTopWindow()->SetFocus();
}