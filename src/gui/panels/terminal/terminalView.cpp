#include "terminalView.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <wx/clipbrd.h>
#include <wx/dcbuffer.h>
#include <wx/fontenum.h>
#include <wx/menu.h>
#include <wx/utils.h>

#include "themesManager/themesManager.hpp"

wxDEFINE_EVENT(EVT_TERMVIEW_TITLE, wxCommandEvent);
wxDEFINE_EVENT(EVT_TERMVIEW_EXIT, wxCommandEvent);

namespace {
const size_t MAX_QUEUE = 4u << 20;
const size_t FLUSH_CHUNK = 256u << 10;
enum { ID_BLINK_TIMER = wxID_HIGHEST + 801, ID_SCROLL_TIMER };

wxColour Blend(const wxColour &a, const wxColour &b, double t) {
	return wxColour((unsigned char)(a.Red() * (1 - t) + b.Red() * t),
					(unsigned char)(a.Green() * (1 - t) + b.Green() * t),
					(unsigned char)(a.Blue() * (1 - t) + b.Blue() * t));
}

std::string ToUtf8(const wxString &s) { return std::string(s.utf8_str()); }

bool IsWordCell(char32_t c) {
	if (c <= U' ')
		return false;
	return !(c < 128 && std::strchr("\"'()[]{}<>|;,`", (int)c));
}
} // namespace

TerminalView::TerminalView(wxWindow *parent, wxWindowID id, const Config &cfg)
	: wxPanel(parent, id, wxDefaultPosition, wxDefaultSize,
			  wxWANTS_CHARS | wxBORDER_NONE),
	  m_cfg(cfg), m_timer(this, ID_BLINK_TIMER),
	  m_scrollTimer(this, ID_SCROLL_TIMER) {
	SetBackgroundStyle(wxBG_STYLE_PAINT);
	SetCursor(wxCursor(wxCURSOR_IBEAM));
	m_padding = FromDIP(4);
	m_sbWidth = FromDIP(10);

	static const unsigned base[16] = {
		0x000000, 0xcd3131, 0x0dbc79, 0xe5e510, 0x2472c8, 0xbc3fbc,
		0x11a8cd, 0xe5e5e5, 0x666666, 0xf14c4c, 0x23d18b, 0xf5f543,
		0x3b8eea, 0xd670d6, 0x29b8db, 0xe5e5e5};
	for (int i = 0; i < 16; ++i)
		m_palette[i] = wxColour((base[i] >> 16) & 255, (base[i] >> 8) & 255,
								base[i] & 255);
	for (int i = 16; i < 232; ++i) {
		int n = i - 16, r = n / 36, g = (n / 6) % 6, b = n % 6;
		auto v = [](int x) { return x ? 55 + 40 * x : 0; };
		m_palette[i] = wxColour(v(r), v(g), v(b));
	}
	for (int i = 232; i < 256; ++i) {
		int v = 8 + 10 * (i - 232);
		m_palette[i] = wxColour(v, v, v);
	}

	ApplyTheme();
	UpdateMetrics();

	m_buf = std::make_unique<TerminalBuffer>(m_cols, m_rows, m_cfg.scrollback);
	TerminalBuffer::Callbacks cb;
	cb.onResponse = [this](const std::string &s) { WriteToPty(s); };
	cb.onTitle = [this](const std::string &t) {
		m_title = wxString::FromUTF8(t.c_str());
		wxCommandEvent ev(EVT_TERMVIEW_TITLE, GetId());
		ev.SetEventObject(this);
		GetEventHandler()->ProcessEvent(ev);
	};
	cb.onCwd = [this](const std::string &p) {
		m_cwd = wxString::FromUTF8(p.c_str());
		wxCommandEvent ev(EVT_TERMVIEW_TITLE, GetId());
		ev.SetEventObject(this);
		GetEventHandler()->ProcessEvent(ev);
	};
	cb.onBell = [] {};
	m_buf->SetCallbacks(cb);

	Bind(wxEVT_PAINT, &TerminalView::OnPaint, this);
	Bind(wxEVT_SIZE, &TerminalView::OnSize, this);
	Bind(wxEVT_ERASE_BACKGROUND, &TerminalView::OnEraseBackground, this);
	Bind(wxEVT_CHAR_HOOK, &TerminalView::OnCharHook, this);
	Bind(wxEVT_CHAR, &TerminalView::OnChar, this);
	Bind(wxEVT_SET_FOCUS, &TerminalView::OnFocus, this);
	Bind(wxEVT_KILL_FOCUS, &TerminalView::OnFocus, this);
	Bind(wxEVT_TIMER, &TerminalView::OnTimer, this);
	Bind(wxEVT_MOUSE_CAPTURE_LOST, [this](wxMouseCaptureLostEvent &) {
		m_selecting = m_draggingBar = m_reporting = false;
		m_scrollTimer.Stop();
	});

	Bind(wxEVT_LEFT_DOWN,    &TerminalView::OnMouse, this);
	Bind(wxEVT_LEFT_UP,      &TerminalView::OnMouse, this);
	Bind(wxEVT_LEFT_DCLICK,  &TerminalView::OnMouse, this);
	Bind(wxEVT_MOTION,       &TerminalView::OnMouse, this);
	Bind(wxEVT_MOUSEWHEEL,   &TerminalView::OnMouse, this);
	Bind(wxEVT_RIGHT_DOWN,   &TerminalView::OnMouse, this);
	Bind(wxEVT_RIGHT_UP,     &TerminalView::OnMouse, this);
	Bind(wxEVT_MIDDLE_DOWN,  &TerminalView::OnMouse, this);
	Bind(wxEVT_MIDDLE_UP,    &TerminalView::OnMouse, this);

	m_timer.Start(530);
}

TerminalView::~TerminalView() {
	m_timer.Stop();
	m_scrollTimer.Stop();
	Stop();
}

void TerminalView::ApplyTheme() {
	m_defBg = ThemesManager::Get().GetColor("main");
	m_defFg = ThemesManager::Get().GetColor("text");
	m_selBg = Blend(m_defBg, m_defFg, 0.30);
	m_cursorColor = m_defFg;
	SetBackgroundColour(m_defBg);
	Refresh();
}

void TerminalView::UpdateMetrics() {
	wxFont base(m_cfg.fontSize, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL,
				wxFONTWEIGHT_NORMAL);
	for (const char *name : {"JetBrains Mono", "Cascadia Mono", "Consolas",
							 "DejaVu Sans Mono", "Menlo"}) {
		if (wxFontEnumerator::IsValidFacename(name)) {
			base.SetFaceName(name);
			break;
		}
	}
	m_fonts[0] = base;
	m_fonts[1] = base;
	m_fonts[1].SetWeight(wxFONTWEIGHT_BOLD);
	m_fonts[2] = base;
	m_fonts[2].SetStyle(wxFONTSTYLE_ITALIC);
	m_fonts[3] = m_fonts[1];
	m_fonts[3].SetStyle(wxFONTSTYLE_ITALIC);

	wxClientDC dc(this);
	dc.SetFont(m_fonts[0]);
	wxCoord w = 0, h = 0;
	dc.GetTextExtent(wxString(wxUniChar('M'), 80), &w, &h);
	double adv = w / 80.0;
	m_cw = std::max(1, (int)std::lround(adv));
	m_ch = std::max(1, (int)h);
	m_perGlyph = std::fabs(adv - m_cw) > 0.05;
}

void TerminalView::SetFontSize(int pts) {
	pts = std::clamp(pts, 6, 40);
	if (pts == m_cfg.fontSize)
		return;
	m_cfg.fontSize = pts;
	UpdateMetrics();
	RecalcGrid();
	Refresh();
}

void TerminalView::RecalcGrid() {
	wxSize s = GetClientSize();
	if (s.x < m_cw * 4 || s.y < m_ch)
		return;
	int cols = std::max(2, (s.x - 2 * m_padding - m_sbWidth) / m_cw);
	int rows = std::max(1, (s.y - 2 * m_padding) / m_ch);
	if (cols != m_cols || rows != m_rows) {
		m_cols = cols;
		m_rows = rows;
		m_buf->Resize(cols, rows);
		m_pty.Resize(cols, rows);
	}
	m_scrollOffset = std::min(m_scrollOffset, m_buf->ScrollbackLines());
}

void TerminalView::OnSize(wxSizeEvent &e) {
	RecalcGrid();
	Refresh();
	e.Skip();
}

bool TerminalView::StartShell() {
	Stop();
	RecalcGrid();
	m_exited = false;
	m_scrollOffset = 0;
	m_hasSel = false;

	PtyProcess::Options o;
	o.program = ToUtf8(m_cfg.shell);
	for (const auto &a : m_cfg.args)
		o.args.push_back(ToUtf8(a));
	o.cwd = ToUtf8(m_cfg.cwd);
	o.cols = m_cols;
	o.rows = m_rows;

	const int gen = ++m_generation;
	bool ok = m_pty.Start(
		o, [this](const char *d, size_t n) { OnPtyData(d, n); },
		[this, gen](int code) {
			CallAfter([this, gen, code] {
				if (gen == m_generation)
					OnProcessExit(code);
			});
		});
	if (!ok) {
		const std::string msg =
			"\x1b[31mCould not start the shell.\x1b[0m\r\n";
		m_buf->Feed(msg.data(), msg.size());
		m_exited = true;
		Refresh();
	}
	return ok;
}

void TerminalView::Stop() {
	m_closing.store(true);
	m_pty.Stop();
	m_closing.store(false);
}

bool TerminalView::IsBusy() const {
	if (!m_pty.IsRunning())
		return false;
	if (m_buf && m_buf->GetModes().altScreen)
		return true;
	return false;
}

void TerminalView::OnProcessExit(int code) {
	m_exited = true;
	m_exitCode = code;
	m_selecting = false;
	std::string msg = "\r\n\x1b[2m[Process exited with code " +
					  std::to_string(code) +
					  " \xe2\x80\x94 press Enter to restart]\x1b[0m\r\n";
	m_buf->Feed(msg.data(), msg.size());
	Refresh();
	wxCommandEvent ev(EVT_TERMVIEW_EXIT, GetId());
	ev.SetInt(code);
	ev.SetEventObject(this);
	GetEventHandler()->ProcessEvent(ev);
}

void TerminalView::OnPtyData(const char *d, size_t n) {
	for (;;) {
		{
			std::lock_guard<std::mutex> lk(m_qMutex);
			if (m_queue.size() < MAX_QUEUE) {
				m_queue.append(d, n);
				if (!m_flushScheduled) {
					m_flushScheduled = true;
					CallAfter([this] { FlushPending(); });
				}
				return;
			}
		}
		if (m_closing.load())
			return;
		wxMilliSleep(2);
	}
}

void TerminalView::FlushPending() {
	std::string chunk;
	bool more = false;
	{
		std::lock_guard<std::mutex> lk(m_qMutex);
		if (m_queue.size() <= FLUSH_CHUNK) {
			chunk.swap(m_queue);
			m_flushScheduled = false;
		} else {
			chunk = m_queue.substr(0, FLUSH_CHUNK);
			m_queue.erase(0, FLUSH_CHUNK);
			more = true;
		}
	}

	const int sbBefore = m_buf->ScrollbackLines();
	const bool altBefore = m_buf->GetModes().altScreen;
	m_buf->Feed(chunk.data(), chunk.size());
	const int sbAfter = m_buf->ScrollbackLines();

	if (m_buf->GetModes().altScreen != altBefore) {
		m_scrollOffset = 0;
		m_hasSel = false;
	} else if (m_scrollOffset > 0) {
		m_scrollOffset = std::min(m_scrollOffset + (sbAfter - sbBefore), sbAfter);
	}
	if (m_hasSel && sbBefore >= m_cfg.scrollback)
		m_hasSel = false;

	Refresh();
	if (more)
		CallAfter([this] { FlushPending(); });
}

void TerminalView::WriteToPty(const std::string &s) {
	if (m_pty.IsRunning())
		m_pty.Write(s);
}

void TerminalView::SendText(const wxString &text, bool bracketedPasteAware) {
	std::string s = ToUtf8(text);
	std::string norm;
	norm.reserve(s.size());
	for (size_t i = 0; i < s.size(); ++i) {
		if (s[i] == '\r') {
			norm += '\r';
			if (i + 1 < s.size() && s[i + 1] == '\n')
				++i;
		} else if (s[i] == '\n') {
			norm += '\r';
		} else {
			norm += s[i];
		}
	}
	if (bracketedPasteAware) {
		norm.erase(std::remove(norm.begin(), norm.end(), '\x1b'), norm.end());
		if (m_buf->GetModes().bracketedPaste)
			norm = "\x1b[200~" + norm + "\x1b[201~";
	}
	ScrollToBottom();
	WriteToPty(norm);
}

void TerminalView::RunCommand(const wxString &cmd) {
	if (m_exited)
		StartShell();
	SendText(cmd + "\r");
}

static void SetClipboardText(const wxString &text, bool primary) {
	if (text.IsEmpty() || !wxTheClipboard->Open())
		return;
	wxTheClipboard->UsePrimarySelection(primary);
	wxTheClipboard->SetData(new wxTextDataObject(text));
	wxTheClipboard->UsePrimarySelection(false);
	wxTheClipboard->Close();
}

static wxString GetClipboardText(bool primary) {
	wxString out;
	if (!wxTheClipboard->Open())
		return out;
	wxTheClipboard->UsePrimarySelection(primary);
	if (wxTheClipboard->IsSupported(wxDF_UNICODETEXT)) {
		wxTextDataObject d;
		if (wxTheClipboard->GetData(d))
			out = d.GetText();
	}
	wxTheClipboard->UsePrimarySelection(false);
	wxTheClipboard->Close();
	return out;
}

void TerminalView::Copy() {
	if (!m_hasSel)
		return;
	Pos lo = std::min(m_selAnchor, m_selCaret), hi = std::max(m_selAnchor, m_selCaret);
	SetClipboardText(wxString::FromUTF8(
						 m_buf->GetText(lo.line, lo.col, hi.line, hi.col + 1).c_str()),
					 false);
}

void TerminalView::Paste() { PasteText(GetClipboardText(false)); }

void TerminalView::PasteText(const wxString &t) {
	if (t.IsEmpty() || m_exited)
		return;
	if (t.find_first_of("\r\n") != wxString::npos &&
		!m_buf->GetModes().bracketedPaste) {
		if (wxMessageBox(
				_("The pasted text contains multiple lines and will be "
				  "executed immediately by the shell.\n\nPaste anyway?"),
				_("Paste into terminal"),
				wxYES_NO | wxICON_WARNING, this) != wxYES)
			return;
	}
	SendText(t, true);
}

void TerminalView::SelectAll() {
	m_selAnchor = {0, 0};
	m_selCaret = {m_buf->TotalLines() - 1, m_cols - 1};
	m_hasSel = true;
	Refresh();
}

void TerminalView::ClearScreen() {
	const std::string seq = "\x1b[H\x1b[2J\x1b[3J";
	m_buf->Feed(seq.data(), seq.size());
	m_scrollOffset = 0;
	m_hasSel = false;
	if (!m_buf->GetModes().altScreen)
		WriteToPty("\x0c");
	Refresh();
}

void TerminalView::ScrollToBottom() {
	if (m_scrollOffset != 0) {
		m_scrollOffset = 0;
		Refresh();
	}
}

void TerminalView::ScrollByLines(int delta) {
	int old = m_scrollOffset;
	m_scrollOffset = std::clamp(m_scrollOffset + delta, 0, m_buf->ScrollbackLines());
	if (old != m_scrollOffset)
		Refresh();
}

int TerminalView::TopLine() const {
	return m_buf->TotalLines() - m_rows - m_scrollOffset;
}

static std::string ModSeq(const char *prefix, int mod, char fin) {
	return std::string("\x1b[") + prefix + ";" + std::to_string(mod) + fin;
}

bool TerminalView::HandleSpecialKey(wxKeyEvent &e) {
	const int k = e.GetKeyCode();
	const bool shift = e.ShiftDown(), alt = e.AltDown(), ctrl = e.RawControlDown();
	const int mod = 1 + (shift ? 1 : 0) + (alt ? 2 : 0) + (ctrl ? 4 : 0);
	const bool appKeys = m_buf->GetModes().appCursorKeys;
	std::string out;

	auto cursorKey = [&](char fin) {
		if (mod > 1)
			return ModSeq("1", mod, fin);
		return std::string(appKeys ? "\x1bO" : "\x1b[") + fin;
	};
	auto tilde = [&](int n) {
		return mod > 1 ? ModSeq(std::to_string(n).c_str(), mod, '~')
					   : "\x1b[" + std::to_string(n) + "~";
	};

	switch (k) {
	case WXK_RETURN:
	case WXK_NUMPAD_ENTER:
		out = alt ? "\x1b\r" : "\r";
		break;
	case WXK_BACK:
		out = ctrl ? "\x17" : (alt ? "\x1b\x7f" : "\x7f");
		break;
	case WXK_TAB:
		out = shift ? "\x1b[Z" : "\t";
		break;
	case WXK_ESCAPE:
		out = "\x1b";
		break;
	case WXK_UP:	out = cursorKey('A'); break;
	case WXK_DOWN:	out = cursorKey('B'); break;
	case WXK_RIGHT: out = cursorKey('C'); break;
	case WXK_LEFT:	out = cursorKey('D'); break;
	case WXK_HOME:	out = cursorKey('H'); break;
	case WXK_END:	out = cursorKey('F'); break;
	case WXK_INSERT:   out = tilde(2); break;
	case WXK_DELETE:   out = tilde(3); break;
	case WXK_PAGEUP:   out = tilde(5); break;
	case WXK_PAGEDOWN: out = tilde(6); break;
	case WXK_F1: case WXK_F2: case WXK_F3: case WXK_F4: {
		char f = (char)('P' + (k - WXK_F1));
		out = mod > 1 ? ModSeq("1", mod, f) : std::string("\x1bO") + f;
		break;
	}
	case WXK_F5:  out = tilde(15); break;
	case WXK_F6:  out = tilde(17); break;
	case WXK_F7:  out = tilde(18); break;
	case WXK_F8:  out = tilde(19); break;
	case WXK_F9:  out = tilde(20); break;
	case WXK_F10: out = tilde(21); break;
	case WXK_F11: out = tilde(23); break;
	case WXK_F12: out = tilde(24); break;
	default:
		return false;
	}
	ScrollToBottom();
	m_cursorOn = true;
	WriteToPty(out);
	return true;
}

void TerminalView::OnCharHook(wxKeyEvent &e) {
	const int k = e.GetKeyCode();
	const bool shift = e.ShiftDown(), alt = e.AltDown();
	const bool ctrl = e.RawControlDown();
	const bool altScreen = m_buf->GetModes().altScreen;

	if (ctrl && (k == WXK_TAB || k == WXK_PAGEUP || k == WXK_PAGEDOWN || k == '`')) {
		e.Skip();
		return;
	}
#ifdef __WXMAC__
	if (e.CmdDown() && (k == 'C' || k == 'V')) {
		k == 'C' ? Copy() : Paste();
		return;
	}
#endif
	if (ctrl && shift && (k == 'C' || k == 'V')) {
		k == 'C' ? Copy() : Paste();
		return;
	}
	if (ctrl && k == WXK_INSERT) {
		Copy();
		return;
	}
	if (shift && !ctrl && k == WXK_INSERT) {
		Paste();
		return;
	}
	if (ctrl && !shift && !alt && k == 'C' && m_hasSel && m_cfg.ctrlCCopiesSelection) {
		Copy();
		m_hasSel = false;
		Refresh();
		return;
	}
	if (shift && !ctrl && !alt && !altScreen) {
		if (k == WXK_PAGEUP) { ScrollByLines(m_rows - 1); return; }
		if (k == WXK_PAGEDOWN) { ScrollByLines(-(m_rows - 1)); return; }
		if (k == WXK_HOME) { ScrollByLines(m_buf->ScrollbackLines()); return; }
		if (k == WXK_END) { ScrollToBottom(); return; }
	}
	if (m_exited && (k == WXK_RETURN || k == WXK_NUMPAD_ENTER)) {
		StartShell();
		return;
	}
	if (ctrl && shift) {
		e.Skip();
		return;
	}
	if (ctrl && !alt) {
		int code = -1;
		if (k >= 'A' && k <= 'Z') code = k - 'A' + 1;
		else if (k == '[') code = 0x1b;
		else if (k == '\\') code = 0x1c;
		else if (k == ']') code = 0x1d;
		else if (k == '6') code = 0x1e;
		else if (k == '/' || k == '-') code = 0x1f;
		else if (k == ' ' || k == '2' || k == '@') code = 0;
		if (code >= 0) {
			ScrollToBottom();
			m_cursorOn = true;
			WriteToPty(std::string(1, (char)code));
			return;
		}
	}
	if (HandleSpecialKey(e))
		return;
	e.Skip();
}

void TerminalView::OnChar(wxKeyEvent &e) {
	wxUniChar u = e.GetUnicodeKey();
	if (u == WXK_NONE) {
		e.Skip();
		return;
	}
	int code = (int)u;
	if (code < 32 || code == 127)
		return;
	std::string s = ToUtf8(wxString(u));
	if (e.AltDown() && !e.RawControlDown())
		s = "\x1b" + s;
	ScrollToBottom();
	m_cursorOn = true;
	if (m_hasSel) {
		m_hasSel = false;
		Refresh();
	}
	WriteToPty(s);
}

void TerminalView::OnFocus(wxFocusEvent &e) {
	m_cursorOn = true;
	if (m_buf->GetModes().focusEvents)
		WriteToPty(e.GetEventType() == wxEVT_SET_FOCUS ? "\x1b[I" : "\x1b[O");
	Refresh();
	e.Skip();
}

void TerminalView::OnTimer(wxTimerEvent &e) {
	if (e.GetId() == ID_BLINK_TIMER) {
		int st = m_buf->CursorStyle();
		bool blinking = m_cfg.cursorBlink && (st == 0 || st % 2 == 1);
		if (blinking && HasFocus()) {
			m_cursorOn = !m_cursorOn;
			Refresh(false);
		}
		return;
	}
	if (!m_selecting)
		return;
	if (m_lastMouse.y < 0) {
		ScrollByLines(1);
		m_selCaret = {TopLine(), m_selCaret.col};
	} else if (m_lastMouse.y > GetClientSize().y) {
		ScrollByLines(-1);
		m_selCaret = {TopLine() + m_rows - 1, m_selCaret.col};
	}
	Refresh(false);
}

TerminalView::Pos TerminalView::CellAt(const wxPoint &p, bool) const {
	int col = std::clamp((p.x - m_padding) / m_cw, 0, m_cols - 1);
	int row = std::clamp((p.y - m_padding) / m_ch, 0, m_rows - 1);
	return {TopLine() + row, col};
}

bool TerminalView::IsSelected(int line, int col) const {
	if (!m_hasSel)
		return false;
	Pos lo = std::min(m_selAnchor, m_selCaret), hi = std::max(m_selAnchor, m_selCaret);
	Pos p{line, col};
	return !(p < lo) && !(hi < p);
}

void TerminalView::SelectWordAt(const Pos &p) {
	const TermRow &row = m_buf->Line(p.line);
	if (!IsWordCell(row.cells[(size_t)p.col].ch) && row.cells[(size_t)p.col].width != 0) {
		m_selAnchor = m_selCaret = p;
	} else {
		int a = p.col, b = p.col;
		while (a > 0 && IsWordCell(row.cells[(size_t)a - 1].ch))
			--a;
		while (b < m_cols - 1 && IsWordCell(row.cells[(size_t)b + 1].ch))
			++b;
		m_selAnchor = {p.line, a};
		m_selCaret = {p.line, b};
	}
	m_hasSel = true;
}

void TerminalView::SelectLineAt(const Pos &p) {
	m_selAnchor = {p.line, 0};
	m_selCaret = {p.line, m_cols - 1};
	m_hasSel = true;
}

wxString TerminalView::UrlAt(const Pos &p) const {
	const TermRow &row = m_buf->Line(p.line);
	std::string s;
	for (const auto &c : row.cells)
		s += (c.ch > 32 && c.ch < 127) ? (char)c.ch : ' ';
	for (size_t i = s.find("http"); i != std::string::npos; i = s.find("http", i + 1)) {
		size_t j = s.find(' ', i);
		if (j == std::string::npos)
			j = s.size();
		std::string cand = s.substr(i, j - i);
		while (!cand.empty() && std::strchr(".,;:!?)]}'\"", cand.back()))
			cand.pop_back();
		bool ok = (cand.rfind("http://", 0) == 0 && cand.size() > 7) ||
				  (cand.rfind("https://", 0) == 0 && cand.size() > 8);
		if (ok && p.col >= (int)i && p.col < (int)(i + cand.size()))
			return wxString::FromUTF8(cand.c_str());
	}
	return {};
}

void TerminalView::SendMouseReport(const wxMouseEvent &e, int button,
								   bool release, bool motion) {
	const auto &m = m_buf->GetModes();
	int col = std::clamp((e.GetX() - m_padding) / m_cw, 0, m_cols - 1) + 1;
	int row = std::clamp((e.GetY() - m_padding) / m_ch, 0, m_rows - 1) + 1;
	int b = button;
	if (e.ShiftDown()) b |= 4;
	if (e.AltDown()) b |= 8;
	if (e.RawControlDown()) b |= 16;
	if (motion) b |= 32;
	std::string out;
	if (m.mouseSgr) {
		out = "\x1b[<" + std::to_string(b) + ";" + std::to_string(col) + ";" +
			  std::to_string(row) + (release ? "m" : "M");
	} else {
		if (col > 223 || row > 223)
			return;
		out = "\x1b[M";
		out += (char)(32 + (release ? 3 : b));
		out += (char)(32 + col);
		out += (char)(32 + row);
	}
	WriteToPty(out);
}

void TerminalView::OnContextMenu() {
	wxMenu menu;
	menu.Append(wxID_COPY, _("Copy"))->Enable(m_hasSel);
	menu.Append(wxID_PASTE, _("Paste"));
	menu.Append(wxID_SELECTALL, _("Select All"));
	menu.AppendSeparator();
	menu.Append(wxID_CLEAR, _("Clear terminal"));
	menu.Bind(wxEVT_MENU, [this](wxCommandEvent &ev) {
		switch (ev.GetId()) {
		case wxID_COPY: Copy(); break;
		case wxID_PASTE: Paste(); break;
		case wxID_SELECTALL: SelectAll(); break;
		case wxID_CLEAR: ClearScreen(); break;
		}
	});
	PopupMenu(&menu);
}

void TerminalView::OnMouse(wxMouseEvent &e) {
	const auto &modes = m_buf->GetModes();
	const bool report = modes.mouse != 0 && !e.ShiftDown();
	const wxEventType t = e.GetEventType();
	m_lastMouse = e.GetPosition();
	const wxSize cs = GetClientSize();
	const bool onBar = !modes.altScreen && m_buf->ScrollbackLines() > 0 &&
					   e.GetX() >= cs.x - m_sbWidth;

	if (t == wxEVT_MOUSEWHEEL) {
		if (e.RawControlDown()) {
			SetFontSize(m_cfg.fontSize + (e.GetWheelRotation() > 0 ? 1 : -1));
			return;
		}
		m_wheelAcc += e.GetWheelRotation();
		int notches = m_wheelAcc / std::max(1, e.GetWheelDelta());
		m_wheelAcc -= notches * std::max(1, e.GetWheelDelta());
		if (notches == 0)
			return;
		int n = std::abs(notches);
		bool up = notches > 0;
		if (report) {
			for (int i = 0; i < n; ++i)
				SendMouseReport(e, up ? 64 : 65, false, false);
		} else if (modes.altScreen) {
			std::string k = std::string(modes.appCursorKeys ? "\x1bO" : "\x1b[") +
							(up ? 'A' : 'B');
			std::string out;
			for (int i = 0; i < n * 3; ++i)
				out += k;
			WriteToPty(out);
		} else {
			ScrollLines(up ? n * 3 : -n * 3);
		}
		return;
	}

	if (t == wxEVT_LEFT_DOWN && onBar) {
		m_draggingBar = true;
		if (!HasCapture())
			CaptureMouse();
	}
	if (m_draggingBar) {
		if (t == wxEVT_LEFT_UP) {
			m_draggingBar = false;
			if (HasCapture())
				ReleaseMouse();
		} else if (e.LeftIsDown()) {
			int sb = m_buf->ScrollbackLines();
			double ratio = std::clamp((double)e.GetY() / std::max(1, cs.y), 0.0, 1.0);
			m_scrollOffset = sb - (int)std::lround(ratio * sb);
			Refresh(false);
		}
		return;
	}

	if (report) {
		if (t == wxEVT_LEFT_DOWN || t == wxEVT_RIGHT_DOWN || t == wxEVT_MIDDLE_DOWN ||
			t == wxEVT_LEFT_DCLICK) {
			SetFocus();
			int b = (t == wxEVT_RIGHT_DOWN) ? 2 : (t == wxEVT_MIDDLE_DOWN) ? 1 : 0;
			m_reportBtn = b;
			m_reporting = true;
			if (!HasCapture())
				CaptureMouse();
			SendMouseReport(e, b, false, false);
		} else if (t == wxEVT_LEFT_UP || t == wxEVT_RIGHT_UP || t == wxEVT_MIDDLE_UP) {
			int b = (t == wxEVT_RIGHT_UP) ? 2 : (t == wxEVT_MIDDLE_UP) ? 1 : 0;
			SendMouseReport(e, b, true, false);
			m_reporting = false;
			if (HasCapture())
				ReleaseMouse();
		} else if (t == wxEVT_MOTION) {
			if (m_reporting && modes.mouse >= 1002)
				SendMouseReport(e, m_reportBtn, false, true);
			else if (!m_reporting && modes.mouse == 1003)
				SendMouseReport(e, 3, false, true);
		}
		SetCursor(wxCursor(wxCURSOR_ARROW));
		return;
	}

	if (t == wxEVT_LEFT_DOWN) {
		SetFocus();
		Pos p = CellAt(e.GetPosition());
		long long now = wxGetLocalTimeMillis().GetValue();
		if (now - m_lastDclickMs < 450) { 
			SelectLineAt(p);
			m_lastDclickMs = 0;
			Refresh(false);
			return;
		}
		m_selAnchor = m_selCaret = p;
		m_hasSel = false;
		m_selecting = true;
		m_scrollTimer.Start(60);
		if (!HasCapture())
			CaptureMouse();
		Refresh(false);
	} else if (t == wxEVT_LEFT_DCLICK) {
		Pos p = CellAt(e.GetPosition());
		SelectWordAt(p);
		m_lastDclickMs = wxGetLocalTimeMillis().GetValue();
		Refresh(false);
	} else if (t == wxEVT_MOTION) {
		if (m_selecting && e.LeftIsDown()) {
			Pos p = CellAt(e.GetPosition());
			m_selCaret = p;
			m_hasSel = !(m_selAnchor == m_selCaret);
			Refresh(false);
		} else if (e.RawControlDown() && !UrlAt(CellAt(e.GetPosition())).IsEmpty()) {
			SetCursor(wxCursor(wxCURSOR_HAND));
		} else {
			SetCursor(wxCursor(wxCURSOR_IBEAM));
		}
	} else if (t == wxEVT_LEFT_UP) {
		bool wasSelecting = m_selecting;
		m_selecting = false;
		m_scrollTimer.Stop();
		if (HasCapture())
			ReleaseMouse();
		if (wasSelecting && m_hasSel) {
			Pos lo = std::min(m_selAnchor, m_selCaret), hi = std::max(m_selAnchor, m_selCaret);
			wxString txt = wxString::FromUTF8(
				m_buf->GetText(lo.line, lo.col, hi.line, hi.col + 1).c_str());
			SetClipboardText(txt, true); 
			if (m_cfg.copyOnSelect)
				SetClipboardText(txt, false);
		} else if (e.RawControlDown()) { 
			wxString url = UrlAt(CellAt(e.GetPosition()));
			if (!url.IsEmpty())
				wxLaunchDefaultBrowser(url);
		}
	} else if (t == wxEVT_MIDDLE_UP) {
		PasteText(GetClipboardText(true));
	} else if (t == wxEVT_RIGHT_UP) {
		OnContextMenu();
	}
}

wxColour TerminalView::ResolveColor(const TermColor &c, bool isFg, bool bold) const {
	switch (c.kind) {
	case TermColor::Indexed:
		return m_palette[(bold && isFg && c.r < 8) ? c.r + 8 : c.r];
	case TermColor::RGB:
		return wxColour(c.r, c.g, c.b);
	default:
		return isFg ? m_defFg : m_defBg;
	}
}

void TerminalView::OnPaint(wxPaintEvent &) {
	wxAutoBufferedPaintDC dc(this);
	dc.SetBackground(wxBrush(m_defBg));
	dc.Clear();
	dc.SetBackgroundMode(wxPENSTYLE_TRANSPARENT);

	const int top = TopLine();
	for (int r = 0; r < m_rows; ++r) {
		int abs = top + r;
		if (abs >= 0 && abs < m_buf->TotalLines())
			DrawRow(dc, r, abs);
	}
	DrawCursor(dc);
	DrawScrollbar(dc);
}

void TerminalView::DrawRow(wxDC &dc, int sr, int absLine) {
	const TermRow &row = m_buf->Line(absLine);
	const int y = m_padding + sr * m_ch;
	const int ncols = std::min(m_cols, (int)row.cells.size());

	auto style = [&](int col, wxColour &fg, wxColour &bg) {
		const TermCell &c = row.cells[(size_t)col];
		fg = ResolveColor(c.fg, true, (c.attrs & ATTR_BOLD) != 0);
		bg = ResolveColor(c.bg, false, false);
		if (c.attrs & ATTR_INVERSE)
			std::swap(fg, bg);
		if (c.attrs & ATTR_DIM)
			fg = Blend(fg, bg, 0.5);
		if (IsSelected(absLine, col))
			bg = m_selBg;
	};

	dc.SetPen(*wxTRANSPARENT_PEN);
	for (int col = 0; col < ncols;) {
		wxColour fg, bg;
		style(col, fg, bg);
		int end = col + 1;
		while (end < ncols) {
			wxColour f2, b2;
			style(end, f2, b2);
			if (b2 != bg)
				break;
			++end;
		}
		if (bg != m_defBg) {
			dc.SetBrush(wxBrush(bg));
			bool last = end >= ncols;
			dc.DrawRectangle(m_padding + col * m_cw, y,
							 (end - col) * m_cw + (last ? m_sbWidth : 0), m_ch);
		}
		col = end;
	}

	for (int col = 0; col < ncols;) {
		const TermCell &cell = row.cells[(size_t)col];
		if (cell.width == 0 || (cell.attrs & ATTR_HIDDEN) ||
			(cell.ch == U' ' && !(cell.attrs & (ATTR_UNDERLINE | ATTR_STRIKE)))) {
			++col;
			continue;
		}
		wxColour fg, bg;
		style(col, fg, bg);
		const int fi = ((cell.attrs & ATTR_BOLD) ? 1 : 0) | ((cell.attrs & ATTR_ITALIC) ? 2 : 0);
		const uint16_t deco = cell.attrs & (ATTR_UNDERLINE | ATTR_STRIKE);

		std::string utf8;
		int start = col, cells = 0;
		if (cell.width == 2) {
			TerminalBuffer::AppendUtf8(utf8, cell.ch);
			cells = 2;
			col += 2;
		} else {
			while (col < ncols) {
				const TermCell &c = row.cells[(size_t)col];
				if (c.width != 1 || (c.attrs & ATTR_HIDDEN))
					break;
				wxColour f2, b2;
				style(col, f2, b2);
				int fi2 = ((c.attrs & ATTR_BOLD) ? 1 : 0) | ((c.attrs & ATTR_ITALIC) ? 2 : 0);
				if (f2 != fg || fi2 != fi || (c.attrs & (ATTR_UNDERLINE | ATTR_STRIKE)) != deco)
					break;
				TerminalBuffer::AppendUtf8(utf8, c.ch);
				++col;
				++cells;
				if (m_perGlyph)
					break;
			}
		}
		dc.SetFont(m_fonts[fi]);
		dc.SetTextForeground(fg);
		const int x = m_padding + start * m_cw;
		dc.DrawText(wxString::FromUTF8(utf8.c_str()), x, y);
		if (deco) {
			dc.SetPen(wxPen(fg));
			if (deco & ATTR_UNDERLINE)
				dc.DrawLine(x, y + m_ch - 2, x + cells * m_cw, y + m_ch - 2);
			if (deco & ATTR_STRIKE)
				dc.DrawLine(x, y + m_ch / 2, x + cells * m_cw, y + m_ch / 2);
			dc.SetPen(*wxTRANSPARENT_PEN);
		}
	}
}

void TerminalView::DrawCursor(wxDC &dc) {
	if (!m_buf->GetModes().cursorVisible || m_scrollOffset != 0 || m_exited)
		return;
	const int cx = m_buf->CursorX(), cy = m_buf->CursorY();
	if (cy >= m_rows || cx >= m_cols)
		return;
	const TermRow &row = m_buf->Line(m_buf->CursorLine());
	const TermCell &cell = row.cells[(size_t)cx];
	const int w = (cell.width == 2 ? 2 : 1) * m_cw;
	const int x = m_padding + cx * m_cw, y = m_padding + cy * m_ch;
	const int st = m_buf->CursorStyle();

	if (!HasFocus()) { 
		dc.SetPen(wxPen(m_cursorColor));
		dc.SetBrush(*wxTRANSPARENT_BRUSH);
		dc.DrawRectangle(x, y, w, m_ch);
		return;
	}
	if (!m_cursorOn)
		return;

	dc.SetPen(*wxTRANSPARENT_PEN);
	dc.SetBrush(wxBrush(m_cursorColor));
	if (st >= 5) {
		dc.DrawRectangle(x, y, std::max(2, FromDIP(2)), m_ch);
	} else if (st >= 3) {
		dc.DrawRectangle(x, y + m_ch - std::max(2, FromDIP(2)), w, std::max(2, FromDIP(2)));
	} else {
		dc.DrawRectangle(x, y, w, m_ch); 
		if (cell.ch != U' ' && cell.width != 0) {
			std::string u;
			TerminalBuffer::AppendUtf8(u, cell.ch);
			dc.SetFont(m_fonts[(cell.attrs & ATTR_BOLD) ? 1 : 0]);
			dc.SetTextForeground(m_defBg);
			dc.DrawText(wxString::FromUTF8(u.c_str()), x, y);
		}
	}
}

void TerminalView::DrawScrollbar(wxDC &dc) {
	const int sb = m_buf->ScrollbackLines();
	if (sb <= 0 || m_buf->GetModes().altScreen)
		return;
	const wxSize cs = GetClientSize();
	const int total = m_buf->TotalLines();
	const int thumbH = std::max(24, cs.y * m_rows / total);
	const double frac = sb ? (double)TopLine() / sb : 1.0;
	const int thumbY = (int)((cs.y - thumbH) * frac);
	dc.SetPen(*wxTRANSPARENT_PEN);
	dc.SetBrush(wxBrush(Blend(m_defBg, m_defFg, m_scrollOffset > 0 ? 0.35 : 0.18)));
	dc.DrawRoundedRectangle(cs.x - m_sbWidth + 2, thumbY, m_sbWidth - 4, thumbH, 3);
}