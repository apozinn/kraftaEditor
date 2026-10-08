#include "terminalBuffer.hpp"

#include <algorithm>
#include <cstring>

static const char32_t DEC_GRAPHICS[31] = {
	0x25C6, 0x2592, 0x2409, 0x240C, 0x240D, 0x240A, 0xB0,   0xB1,
	0x2424, 0x240B, 0x2518, 0x2510, 0x250C, 0x2514, 0x253C, 0x23BA,
	0x23BB, 0x2500, 0x23BC, 0x23BD, 0x251C, 0x2524, 0x2534, 0x252C,
	0x2502, 0x2264, 0x2265, 0x3C0,  0x2260, 0xA3,   0xB7};

static const size_t MAX_OSC = 64 * 1024;

int TerminalBuffer::CharWidth(char32_t c) {
	if (c < 0x20 || (c >= 0x7F && c < 0xA0))
		return 0;
	if ((c >= 0x300 && c <= 0x36F) || (c >= 0x200B && c <= 0x200F) ||
		(c >= 0xFE00 && c <= 0xFE0F) || c == 0x20E3)
		return 0;
	if ((c >= 0x1100 && c <= 0x115F) || (c >= 0x2E80 && c <= 0xA4CF) ||
		(c >= 0xAC00 && c <= 0xD7A3) || (c >= 0xF900 && c <= 0xFAFF) ||
		(c >= 0xFE30 && c <= 0xFE6F) || (c >= 0xFF00 && c <= 0xFF60) ||
		(c >= 0xFFE0 && c <= 0xFFE6) || (c >= 0x1F300 && c <= 0x1F64F) ||
		(c >= 0x1F900 && c <= 0x1F9FF) || (c >= 0x20000 && c <= 0x3FFFD))
		return 2;
	return 1;
}

void TerminalBuffer::AppendUtf8(std::string &s, char32_t c) {
	if (c < 0x80) {
		s += (char)c;
	} else if (c < 0x800) {
		s += (char)(0xC0 | (c >> 6));
		s += (char)(0x80 | (c & 0x3F));
	} else if (c < 0x10000) {
		s += (char)(0xE0 | (c >> 12));
		s += (char)(0x80 | ((c >> 6) & 0x3F));
		s += (char)(0x80 | (c & 0x3F));
	} else {
		s += (char)(0xF0 | (c >> 18));
		s += (char)(0x80 | ((c >> 12) & 0x3F));
		s += (char)(0x80 | ((c >> 6) & 0x3F));
		s += (char)(0x80 | (c & 0x3F));
	}
}

TerminalBuffer::TerminalBuffer(int cols, int rows, int maxScrollback)
	: m_cols(std::max(cols, 2)), m_rows(std::max(rows, 1)),
	  m_maxScrollback(std::max(maxScrollback, 0)) {
	Reset();
}

TermCell TerminalBuffer::Blank() const {
	TermCell c;
	c.bg = m_bg;
	return c;
}

TermRow TerminalBuffer::MakeRow() const {
	TermRow r;
	r.cells.assign((size_t)m_cols, Blank());
	return r;
}

void TerminalBuffer::Reset() {
	m_fg = m_bg = TermColor();
	m_attrs = 0;
	m_main.clear();
	m_alt.clear();
	for (int i = 0; i < m_rows; ++i) {
		m_main.push_back(MakeRow());
		m_alt.push_back(MakeRow());
	}
	m_modes = Modes();
	m_x = m_y = 0;
	m_wrapPending = false;
	m_top = 0;
	m_bottom = m_rows - 1;
	m_saved = m_savedAlt = Saved();
	m_charset[0] = m_charset[1] = 0;
	m_gl = 0;
	m_cursorStyle = 1;
	m_state = State::Ground;
	m_utf8Need = 0;
	ResetTabs();
	Touch();
}

void TerminalBuffer::ClearScrollback() {
	if (m_modes.altScreen)
		return;
	while ((int)m_main.size() > m_rows)
		m_main.pop_front();
	Touch();
}

void TerminalBuffer::ResetTabs() {
	m_tabs.assign((size_t)m_cols, false);
	for (int i = 8; i < m_cols; i += 8)
		m_tabs[(size_t)i] = true;
}

int TerminalBuffer::Pn(size_t i, int def) const {
	int v = i < m_params.size() ? m_params[i] : -1;
	return v <= 0 ? def : v;
}

void TerminalBuffer::Resize(int cols, int rows) {
	cols = std::max(cols, 2);
	rows = std::max(rows, 1);
	if (cols == m_cols && rows == m_rows)
		return;

	for (Screen *s : {&m_main, &m_alt})
		for (auto &r : *s)
			r.cells.resize((size_t)cols, TermCell());
	m_cols = cols;

	const bool alt = m_modes.altScreen;
	int cy = alt ? m_saved.y : m_y;
	if (rows < m_rows) {
		int d = m_rows - rows;
		while (d > 0 && cy < m_rows - 1) {
			const TermRow &last = m_main.back();
			bool blank = std::all_of(last.cells.begin(), last.cells.end(),
									 [](const TermCell &c) { return c.ch == U' '; });
			if (!blank)
				break;
			m_main.pop_back();
			--d;
			--m_rows;
		}
		cy = std::max(0, cy - d);
	} else if (rows > m_rows) {
		int d = rows - m_rows;
		int take = std::min(d, (int)m_main.size() - m_rows);
		cy += take;
		for (int i = take; i < d; ++i)
			m_main.push_back(MakeRow());
	}
	if (alt)
		m_saved.y = std::clamp(cy, 0, rows - 1);
	else
		m_y = cy;
	while ((int)m_alt.size() > rows)
		m_alt.pop_back();
	while ((int)m_alt.size() < rows)
		m_alt.push_back(MakeRow());

	m_rows = rows;
	m_top = 0;
	m_bottom = rows - 1;
	m_savedAlt.y = std::min(m_savedAlt.y, rows - 1);
	ResetTabs();
	ClampCursor();
	Touch();
}

void TerminalBuffer::ClampCursor() {
	m_x = std::clamp(m_x, 0, m_cols - 1);
	m_y = std::clamp(m_y, 0, m_rows - 1);
}

void TerminalBuffer::ScrollUp(int n) {
	n = std::min(n, m_bottom - m_top + 1);
	Screen &s = Active();
	bool full = (m_top == 0 && m_bottom == m_rows - 1);
	for (int i = 0; i < n; ++i) {
		if (full && !m_modes.altScreen) {
			s.push_back(MakeRow());
			while ((int)s.size() > m_rows + m_maxScrollback)
				s.pop_front();
		} else {
			size_t base = s.size() - (size_t)m_rows;
			s.insert(s.begin() + (long)(base + (size_t)m_bottom + 1), MakeRow());
			s.erase(s.begin() + (long)(base + (size_t)m_top));
		}
	}
}

void TerminalBuffer::ScrollDown(int n) {
	n = std::min(n, m_bottom - m_top + 1);
	Screen &s = Active();
	for (int i = 0; i < n; ++i) {
		size_t base = s.size() - (size_t)m_rows;
		s.insert(s.begin() + (long)(base + (size_t)m_top), MakeRow());
		s.erase(s.begin() + (long)(base + (size_t)m_bottom + 1));
	}
}

void TerminalBuffer::LineFeed() {
	m_wrapPending = false;
	if (m_y == m_bottom)
		ScrollUp(1);
	else if (m_y < m_rows - 1)
		++m_y;
}

void TerminalBuffer::ReverseIndex() {
	m_wrapPending = false;
	if (m_y == m_top)
		ScrollDown(1);
	else if (m_y > 0)
		--m_y;
}

void TerminalBuffer::MoveTo(int row, int col) {
	if (m_modes.originMode) {
		row += m_top;
		m_y = std::clamp(row, m_top, m_bottom);
	} else {
		m_y = std::clamp(row, 0, m_rows - 1);
	}
	m_x = std::clamp(col, 0, m_cols - 1);
	m_wrapPending = false;
}

void TerminalBuffer::EraseCells(int row, int c1, int c2) {
	TermRow &r = ScreenRow(row);
	c1 = std::max(c1, 0);
	c2 = std::min(c2, m_cols);
	for (int c = c1; c < c2; ++c)
		r.cells[(size_t)c] = Blank();
	if (c2 >= m_cols)
		r.wrapped = false;
}

void TerminalBuffer::EraseLine(int mode) {
	if (mode == 0)
		EraseCells(m_y, m_x, m_cols);
	else if (mode == 1)
		EraseCells(m_y, 0, m_x + 1);
	else
		EraseCells(m_y, 0, m_cols);
}

void TerminalBuffer::EraseDisplay(int mode) {
	if (mode == 0) {
		EraseCells(m_y, m_x, m_cols);
		for (int r = m_y + 1; r < m_rows; ++r)
			EraseCells(r, 0, m_cols);
	} else if (mode == 1) {
		for (int r = 0; r < m_y; ++r)
			EraseCells(r, 0, m_cols);
		EraseCells(m_y, 0, m_x + 1);
	} else if (mode == 2) {
		for (int r = 0; r < m_rows; ++r)
			EraseCells(r, 0, m_cols);
	} else if (mode == 3) {
		ClearScrollback();
	}
}

void TerminalBuffer::InsertLines(int n) {
	if (m_y < m_top || m_y > m_bottom)
		return;
	int saveTop = m_top;
	m_top = m_y;
	ScrollDown(n);
	m_top = saveTop;
	m_x = 0;
	m_wrapPending = false;
}

void TerminalBuffer::DeleteLines(int n) {
	if (m_y < m_top || m_y > m_bottom)
		return;
	int saveTop = m_top;
	m_top = m_y;
	Screen &s = Active();
	n = std::min(n, m_bottom - m_top + 1);
	for (int i = 0; i < n; ++i) {
		size_t base = s.size() - (size_t)m_rows;
		s.insert(s.begin() + (long)(base + (size_t)m_bottom + 1), MakeRow());
		s.erase(s.begin() + (long)(base + (size_t)m_top));
	}
	m_top = saveTop;
	m_x = 0;
	m_wrapPending = false;
}

void TerminalBuffer::InsertChars(int n) {
	TermRow &r = ScreenRow(m_y);
	n = std::min(n, m_cols - m_x);
	r.cells.insert(r.cells.begin() + m_x, (size_t)n, Blank());
	r.cells.resize((size_t)m_cols);
	m_wrapPending = false;
}

void TerminalBuffer::DeleteChars(int n) {
	TermRow &r = ScreenRow(m_y);
	n = std::min(n, m_cols - m_x);
	r.cells.erase(r.cells.begin() + m_x, r.cells.begin() + m_x + n);
	r.cells.resize((size_t)m_cols, Blank());
	m_wrapPending = false;
}

void TerminalBuffer::SaveCursor() {
	Saved &s = m_modes.altScreen ? m_savedAlt : m_saved;
	s = Saved{m_x, m_y, m_fg, m_bg, m_attrs, m_wrapPending, m_modes.originMode,
			  {m_charset[0], m_charset[1]}, m_gl};
}

void TerminalBuffer::RestoreCursor() {
	const Saved &s = m_modes.altScreen ? m_savedAlt : m_saved;
	m_x = s.x;
	m_y = s.y;
	m_fg = s.fg;
	m_bg = s.bg;
	m_attrs = s.attrs;
	m_wrapPending = s.wrapPending;
	m_modes.originMode = s.origin;
	m_charset[0] = s.charset[0];
	m_charset[1] = s.charset[1];
	m_gl = s.gl;
	ClampCursor();
}

void TerminalBuffer::SwitchAlt(bool on, bool saveCursor) {
	if (m_modes.altScreen == on)
		return;
	if (on) {
		if (saveCursor)
			SaveCursor();
		m_modes.altScreen = true;
		for (int r = 0; r < m_rows; ++r)
			EraseCells(r, 0, m_cols);
		m_top = 0;
		m_bottom = m_rows - 1;
	} else {
		m_modes.altScreen = false;
		m_top = 0;
		m_bottom = m_rows - 1;
		if (saveCursor)
			RestoreCursor();
	}
	m_wrapPending = false;
}

void TerminalBuffer::Print(char32_t c) {
	if (m_charset[m_gl] == 1 && c >= 0x60 && c <= 0x7E)
		c = DEC_GRAPHICS[c - 0x60];

	int w = CharWidth(c);
	if (w == 0)
		return;

	if (m_wrapPending && m_modes.autoWrap) {
		ScreenRow(m_y).wrapped = true;
		CarriageReturn();
		LineFeed();
	}
	m_wrapPending = false;

	if (w == 2 && m_x == m_cols - 1) {
		if (!m_modes.autoWrap)
			return;
		EraseCells(m_y, m_x, m_x + 1);
		ScreenRow(m_y).wrapped = true;
		CarriageReturn();
		LineFeed();
	}

	TermRow &row = ScreenRow(m_y);
	if (m_modes.insertMode)
		InsertChars(w);

	TermCell &cur = row.cells[(size_t)m_x];
	if (cur.width == 0 && m_x > 0)
		row.cells[(size_t)m_x - 1] = Blank();
	if (cur.width == 2 && m_x + 1 < m_cols)
		row.cells[(size_t)m_x + 1] = Blank();

	TermCell &cell = row.cells[(size_t)m_x];
	cell.ch = c;
	cell.fg = m_fg;
	cell.bg = m_bg;
	cell.attrs = m_attrs;
	cell.width = (uint8_t)w;
	if (w == 2) {
		TermCell &tail = row.cells[(size_t)m_x + 1];
		tail = Blank();
		tail.width = 0;
	}
	m_lastChar = c;

	m_x += w;
	if (m_x >= m_cols) {
		m_x = m_cols - 1;
		m_wrapPending = true;
	}
}

void TerminalBuffer::Feed(const char *data, size_t len) {
	for (size_t i = 0; i < len; ++i)
		Process((uint8_t)data[i]);
	Touch();
}

void TerminalBuffer::Execute(uint8_t c) {
	switch (c) {
	case 0x07:
		if (m_cb.onBell)
			m_cb.onBell();
		break;
	case 0x08:
		if (m_x > 0)
			--m_x;
		m_wrapPending = false;
		break;
	case 0x09: {
		int x = m_x + 1;
		while (x < m_cols - 1 && !m_tabs[(size_t)x])
			++x;
		m_x = std::min(x, m_cols - 1);
		m_wrapPending = false;
		break;
	}
	case 0x0A:
	case 0x0B:
	case 0x0C:
		LineFeed();
		break;
	case 0x0D:
		CarriageReturn();
		break;
	case 0x0E:
		m_gl = 1;
		break;
	case 0x0F:
		m_gl = 0;
		break;
	default:
		break;
	}
}

void TerminalBuffer::Process(uint8_t b) {
	if (m_state == State::Osc || m_state == State::StringIgnore) {
		if (b == 0x07) {
			if (m_state == State::Osc)
				OscDispatch();
			m_state = State::Ground;
		} else if (b == 0x1B) {
			m_state = State::StringEsc;
		} else if (b == 0x18 || b == 0x1A) {
			m_state = State::Ground;
		} else if (m_state == State::Osc && m_osc.size() < MAX_OSC) {
			m_osc += (char)b;
		}
		return;
	}
	if (m_state == State::StringEsc) {
		if (b == '\\') {
			OscDispatch();
			m_state = State::Ground;
		} else {
			m_state = State::Escape;
			Process(b);
		}
		return;
	}

	if (b == 0x1B) {
		m_state = State::Escape;
		m_utf8Need = 0;
		return;
	}
	if (b == 0x18 || b == 0x1A) {
		m_state = State::Ground;
		return;
	}
	if (b < 0x20) {
		Execute(b);
		return;
	}

	switch (m_state) {
	case State::Ground: {
		if (b < 0x80) {
			m_utf8Need = 0;
			if (b == 0x7F)
				return;
			Print(b);
			return;
		}
		if (m_utf8Need == 0) {
			if ((b & 0xE0) == 0xC0) {
				m_utf8Cp = b & 0x1F;
				m_utf8Need = 1;
			} else if ((b & 0xF0) == 0xE0) {
				m_utf8Cp = b & 0x0F;
				m_utf8Need = 2;
			} else if ((b & 0xF8) == 0xF0) {
				m_utf8Cp = b & 0x07;
				m_utf8Need = 3;
			} else {
				Print(0xFFFD);
			}
		} else if ((b & 0xC0) == 0x80) {
			m_utf8Cp = (m_utf8Cp << 6) | (b & 0x3F);
			if (--m_utf8Need == 0)
				Print(m_utf8Cp > 0x10FFFF ? 0xFFFD : m_utf8Cp);
		} else {
			m_utf8Need = 0;
			Print(0xFFFD);
			Process(b);
		}
		break;
	}
	case State::Escape:
		if (b == '[') {
			m_state = State::Csi;
			m_params.clear();
			m_haveParam = false;
			m_priv = 0;
			m_inter.clear();
		} else if (b == ']') {
			m_state = State::Osc;
			m_osc.clear();
		} else if (b == 'P' || b == '^' || b == '_' || b == 'X') {
			m_state = State::StringIgnore;
			m_osc.clear();
		} else if (b == '(' || b == ')' || b == '*' || b == '+' || b == '#' ||
				   b == ' ') {
			m_escInter = b;
			m_state = State::EscIntermediate;
		} else {
			EscDispatch(b);
			m_state = State::Ground;
		}
		break;
	case State::EscIntermediate:
		if (m_escInter == '(' || m_escInter == ')') {
			int idx = (m_escInter == '(') ? 0 : 1;
			m_charset[idx] = (b == '0') ? 1 : 0;
		} else if (m_escInter == '#' && b == '8') {
			for (int r = 0; r < m_rows; ++r)
				for (auto &c : ScreenRow(r).cells)
					c = TermCell{U'E', {}, {}, 0, 1};
		}
		m_state = State::Ground;
		break;
	case State::Csi:
		if (b >= '0' && b <= '9') {
			if (m_params.empty() || !m_haveParam)
				m_params.push_back(0);
			m_haveParam = true;
			int &p = m_params.back();
			p = std::min(p * 10 + (b - '0'), 65535);
		} else if (b == ';' || b == ':') {
			if (!m_haveParam)
				m_params.push_back(-1);
			m_haveParam = false;
			if (m_params.size() > 32)
				m_state = State::Ground;
		} else if (b >= '<' && b <= '?') {
			m_priv = (char)b;
		} else if (b >= 0x20 && b <= 0x2F) {
			m_inter += (char)b;
		} else if (b >= 0x40 && b <= 0x7E) {
			if (!m_haveParam && !m_params.empty())
				m_params.push_back(-1);
			CsiDispatch(b);
			m_state = State::Ground;
		} else {
			m_state = State::Ground;
		}
		break;
	default:
		m_state = State::Ground;
	}
}

void TerminalBuffer::EscDispatch(uint8_t c) {
	switch (c) {
	case '7':
		SaveCursor();
		break;
	case '8':
		RestoreCursor();
		break;
	case 'D':
		LineFeed();
		break;
	case 'E':
		CarriageReturn();
		LineFeed();
		break;
	case 'M':
		ReverseIndex();
		break;
	case 'H':
		m_tabs[(size_t)m_x] = true;
		break;
	case 'c':
		Reset();
		break;
	case '=':
		m_modes.appKeypad = true;
		break;
	case '>':
		m_modes.appKeypad = false;
		break;
	default:
		break;
	}
}

void TerminalBuffer::OscDispatch() {
	size_t semi = m_osc.find(';');
	if (semi == std::string::npos)
		return;
	std::string cmd = m_osc.substr(0, semi);
	std::string arg = m_osc.substr(semi + 1);
	if ((cmd == "0" || cmd == "2") && m_cb.onTitle) {
		m_cb.onTitle(arg);
	} else if (cmd == "7" && m_cb.onCwd) {
		if (arg.rfind("file://", 0) == 0) {
			size_t slash = arg.find('/', 7);
			if (slash != std::string::npos) {
				std::string path, enc = arg.substr(slash);
				for (size_t i = 0; i < enc.size(); ++i) {
					if (enc[i] == '%' && i + 2 < enc.size() + 0 &&
						isxdigit((unsigned char)enc[i + 1]) &&
						isxdigit((unsigned char)enc[i + 2])) {
						path += (char)std::stoi(enc.substr(i + 1, 2), nullptr, 16);
						i += 2;
					} else {
						path += enc[i];
					}
				}
				m_cb.onCwd(path);
			}
		}
	}
	m_osc.clear();
}

void TerminalBuffer::SetMode(bool priv, int mode, bool on) {
	if (!priv) {
		if (mode == 4)
			m_modes.insertMode = on;
		return;
	}
	switch (mode) {
	case 1:
		m_modes.appCursorKeys = on;
		break;
	case 6:
		m_modes.originMode = on;
		MoveTo(0, 0);
		break;
	case 7:
		m_modes.autoWrap = on;
		break;
	case 25:
		m_modes.cursorVisible = on;
		break;
	case 47:
	case 1047:
		SwitchAlt(on, false);
		break;
	case 1049:
		SwitchAlt(on, true);
		break;
	case 1000:
	case 1002:
	case 1003:
		m_modes.mouse = on ? mode : 0;
		break;
	case 1004:
		m_modes.focusEvents = on;
		break;
	case 1006:
		m_modes.mouseSgr = on;
		break;
	case 2004:
		m_modes.bracketedPaste = on;
		break;
	default:
		break;
	}
}

void TerminalBuffer::Sgr() {
	if (m_params.empty()) {
		m_params.push_back(0);
	}
	for (size_t i = 0; i < m_params.size(); ++i) {
		int p = std::max(m_params[i], 0);
		auto setExt = [&](TermColor &target) {
			int kind = i + 1 < m_params.size() ? m_params[i + 1] : -1;
			if (kind == 5 && i + 2 < m_params.size()) {
				target.kind = TermColor::Indexed;
				target.r = (uint8_t)std::clamp(m_params[i + 2], 0, 255);
				i += 2;
			} else if (kind == 2 && i + 4 < m_params.size()) {
				target.kind = TermColor::RGB;
				target.r = (uint8_t)std::clamp(m_params[i + 2], 0, 255);
				target.g = (uint8_t)std::clamp(m_params[i + 3], 0, 255);
				target.b = (uint8_t)std::clamp(m_params[i + 4], 0, 255);
				i += 4;
			} else {
				i = m_params.size();
			}
		};
		auto idx = [](TermColor &c, int n) {
			c.kind = TermColor::Indexed;
			c.r = (uint8_t)n;
		};
		if (p == 0) {
			m_attrs = 0;
			m_fg = m_bg = TermColor();
		} else if (p == 1) m_attrs |= ATTR_BOLD;
		else if (p == 2) m_attrs |= ATTR_DIM;
		else if (p == 3) m_attrs |= ATTR_ITALIC;
		else if (p == 4) m_attrs |= ATTR_UNDERLINE;
		else if (p == 5 || p == 6) m_attrs |= ATTR_BLINK;
		else if (p == 7) m_attrs |= ATTR_INVERSE;
		else if (p == 8) m_attrs |= ATTR_HIDDEN;
		else if (p == 9) m_attrs |= ATTR_STRIKE;
		else if (p == 22) m_attrs &= ~(ATTR_BOLD | ATTR_DIM);
		else if (p == 23) m_attrs &= ~ATTR_ITALIC;
		else if (p == 24) m_attrs &= ~ATTR_UNDERLINE;
		else if (p == 25) m_attrs &= ~ATTR_BLINK;
		else if (p == 27) m_attrs &= ~ATTR_INVERSE;
		else if (p == 28) m_attrs &= ~ATTR_HIDDEN;
		else if (p == 29) m_attrs &= ~ATTR_STRIKE;
		else if (p >= 30 && p <= 37) idx(m_fg, p - 30);
		else if (p == 38) setExt(m_fg);
		else if (p == 39) m_fg = TermColor();
		else if (p >= 40 && p <= 47) idx(m_bg, p - 40);
		else if (p == 48) setExt(m_bg);
		else if (p == 49) m_bg = TermColor();
		else if (p >= 90 && p <= 97) idx(m_fg, p - 90 + 8);
		else if (p >= 100 && p <= 107) idx(m_bg, p - 100 + 8);
	}
}

void TerminalBuffer::CsiDispatch(uint8_t f) {
	const bool priv = (m_priv == '?');

	if (!m_inter.empty()) {
		if (m_inter == " " && f == 'q')
			m_cursorStyle = std::clamp(m_params.empty() ? 1 : std::max(m_params[0], 0), 0, 6);
		return;
	}
	if (m_priv == '>' || m_priv == '=' || m_priv == '<') {
		if (m_priv == '>' && f == 'c')
			Respond("\x1b[>0;10;0c");
		return;
	}

	switch (f) {
	case 'A': m_y = std::max(m_y - Pn(0), m_y >= m_top ? m_top : 0); m_wrapPending = false; break;
	case 'B': m_y = std::min(m_y + Pn(0), m_y <= m_bottom ? m_bottom : m_rows - 1); m_wrapPending = false; break;
	case 'C': m_x = std::min(m_x + Pn(0), m_cols - 1); m_wrapPending = false; break;
	case 'D': m_x = std::max(m_x - Pn(0), 0); m_wrapPending = false; break;
	case 'E': m_x = 0; m_y = std::min(m_y + Pn(0), m_rows - 1); m_wrapPending = false; break;
	case 'F': m_x = 0; m_y = std::max(m_y - Pn(0), 0); m_wrapPending = false; break;
	case 'G':
	case '`': m_x = std::clamp(Pn(0) - 1, 0, m_cols - 1); m_wrapPending = false; break;
	case 'H':
	case 'f': MoveTo(Pn(0) - 1, Pn(1) - 1); break;
	case 'd': MoveTo(Pn(0) - 1, m_x); break;
	case 'a': m_x = std::min(m_x + Pn(0), m_cols - 1); m_wrapPending = false; break;
	case 'e': m_y = std::min(m_y + Pn(0), m_rows - 1); m_wrapPending = false; break;
	case 'I': for (int n = Pn(0); n > 0; --n) Execute(0x09); break;
	case 'Z':
		for (int n = Pn(0); n > 0; --n) {
			int x = m_x - 1;
			while (x > 0 && !m_tabs[(size_t)x]) --x;
			m_x = std::max(x, 0);
		}
		break;
	case 'J': EraseDisplay(m_params.empty() ? 0 : std::max(m_params[0], 0)); break;
	case 'K': EraseLine(m_params.empty() ? 0 : std::max(m_params[0], 0)); break;
	case 'L': InsertLines(Pn(0)); break;
	case 'M': DeleteLines(Pn(0)); break;
	case '@': InsertChars(Pn(0)); break;
	case 'P': DeleteChars(Pn(0)); break;
	case 'X': EraseCells(m_y, m_x, m_x + Pn(0)); break;
	case 'S': ScrollUp(Pn(0)); break;
	case 'T': ScrollDown(Pn(0)); break;
	case 'b':
		for (int n = std::min(Pn(0), 65535); n > 0 && m_lastChar; --n)
			Print(m_lastChar);
		break;
	case 'g':
		if (!m_params.empty() && m_params[0] == 3)
			std::fill(m_tabs.begin(), m_tabs.end(), false);
		else if (m_params.empty() || m_params[0] <= 0)
			m_tabs[(size_t)m_x] = false;
		break;
	case 'm': Sgr(); break;
	case 'h':
	case 'l':
		for (int p : m_params)
			SetMode(priv, p, f == 'h');
		break;
	case 'r': {
		int t = Pn(0) - 1;
		int b = (m_params.size() > 1 && m_params[1] > 0 ? m_params[1] : m_rows) - 1;
		if (t < b && b < m_rows) {
			m_top = std::max(t, 0);
			m_bottom = b;
			MoveTo(0, 0);
		}
		break;
	}
	case 's': SaveCursor(); break;
	case 'u': RestoreCursor(); break;
	case 'n': {
		int p = m_params.empty() ? 0 : m_params[0];
		if (p == 5) Respond("\x1b[0n");
		else if (p == 6) {
			int row = m_y + 1 - (m_modes.originMode ? m_top : 0);
			Respond("\x1b[" + std::to_string(row) + ";" + std::to_string(m_x + 1) + "R");
		}
		break;
	}
	case 'c':
		if (m_params.empty() || m_params[0] <= 0)
			Respond("\x1b[?62;c");
		break;
	default:
		break;
	}
}

void TerminalBuffer::Respond(const std::string& s) {
    if (m_cb.onResponse)
        m_cb.onResponse(s);
}

std::string TerminalBuffer::GetText(int l1, int c1, int l2, int c2) const {
	std::string out;
	l1 = std::max(l1, 0);
	l2 = std::min(l2, TotalLines() - 1);
	for (int l = l1; l <= l2; ++l) {
		const TermRow &row = Line(l);
		int from = (l == l1) ? c1 : 0;
		int to = (l == l2) ? c2 : m_cols;
		from = std::clamp(from, 0, m_cols);
		to = std::clamp(to, 0, m_cols);
		std::string seg;
		for (int c = from; c < to; ++c) {
			const TermCell &cell = row.cells[(size_t)c];
			if (cell.width == 0)
				continue;
			AppendUtf8(seg, cell.ch);
		}
		if (to == m_cols && !row.wrapped) 
			while (!seg.empty() && seg.back() == ' ')
				seg.pop_back();
		out += seg;
		if (l < l2 && !row.wrapped)
			out += '\n';
	}
	return out;
}