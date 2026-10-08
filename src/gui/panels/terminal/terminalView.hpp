#pragma once

/**
 * @file terminalView.hpp
 * @brief A terminal session: shell (PTY) + emulator + rendering/input in wxWidgets.
 */

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <wx/wx.h>

#include "ptyProcess.hpp"
#include "terminalBuffer.hpp"

/** Fired on the parent when the title (OSC 0/2) or cwd (OSC 7) changes. */
wxDECLARE_EVENT(EVT_TERMVIEW_TITLE, wxCommandEvent);
/** Fired when the shell exits; GetInt() = exit code. */
wxDECLARE_EVENT(EVT_TERMVIEW_EXIT, wxCommandEvent);

class TerminalView : public wxPanel {
public:
	struct Config {
		wxString shell;
		wxArrayString args;
		wxString cwd;
		int fontSize = 11;
		int scrollback = 10000;
		bool cursorBlink = true;
		bool copyOnSelect = false;
		bool ctrlCCopiesSelection = true;
	};

	TerminalView(wxWindow *parent, wxWindowID id, const Config &cfg);
	~TerminalView() override;

	bool StartShell();
	void Stop();
	bool IsRunning() const { return m_pty.IsRunning(); }
	bool HasExited() const { return m_exited; }
	bool IsBusy() const;

	void SendText(const wxString &text, bool bracketedPasteAware = false);
	void RunCommand(const wxString &cmd);
	void Paste();
	void PasteText(const wxString &t);
	void Copy();
	bool HasSelection() const { return m_hasSel; }
	void SelectAll();
	void ClearScreen();
	void ScrollToBottom();
	void SetFontSize(int pts);
	int GetFontSize() const { return m_cfg.fontSize; }

	wxString GetTitle() const { return m_title; }
	wxString GetCwd() const { return m_cwd; }
	void ApplyTheme();

private:
	struct Pos {
		int line = 0, col = 0;
		bool operator<(const Pos &o) const {
			return line < o.line || (line == o.line && col < o.col);
		}
		bool operator==(const Pos &o) const {
			return line == o.line && col == o.col;
		}
	};

	void OnPtyData(const char *d, size_t n);
	void FlushPending();
	void OnProcessExit(int code);

	void OnPaint(wxPaintEvent &);
	void OnSize(wxSizeEvent &);
	void OnEraseBackground(wxEraseEvent &) {}
	void DrawRow(wxDC &dc, int screenRow, int absLine);
	void DrawCursor(wxDC &dc);
	void DrawScrollbar(wxDC &dc);
	void UpdateMetrics();
	void RecalcGrid();
	wxColour ResolveColor(const TermColor &c, bool isFg, bool bold) const;
	bool IsSelected(int line, int col) const;

	void OnCharHook(wxKeyEvent &);
	void OnChar(wxKeyEvent &);
	void OnMouse(wxMouseEvent &);
	void OnFocus(wxFocusEvent &e);
	void OnTimer(wxTimerEvent &);
	void OnContextMenu();
	bool HandleSpecialKey(wxKeyEvent &e);
	void SendMouseReport(const wxMouseEvent &e, int button, bool release,
						 bool motion);
	Pos CellAt(const wxPoint &p, bool clampToRows = true) const;
	void SelectWordAt(const Pos &p);
	void SelectLineAt(const Pos &p);
	wxString UrlAt(const Pos &p) const;
	void ScrollByLines(int delta);
	int TopLine() const;
	void WriteToPty(const std::string &s);

	Config m_cfg;
	PtyProcess m_pty;
	std::unique_ptr<TerminalBuffer> m_buf;

	std::mutex m_qMutex;
	std::string m_queue;
	bool m_flushScheduled = false;
	bool m_exited = false;
	int m_exitCode = 0;

	wxFont m_fonts[4];
	int m_cw = 8, m_ch = 16, m_ascentPad = 0;
	bool m_perGlyph = false;
	int m_cols = 80, m_rows = 24;
	int m_padding = 4, m_sbWidth = 10;
	wxColour m_defFg, m_defBg, m_selBg, m_cursorColor;
	wxColour m_palette[256];
	int m_scrollOffset = 0;
	bool m_cursorOn = true;
	wxTimer m_timer;
	wxTimer m_scrollTimer;

	bool m_hasSel = false, m_selecting = false, m_draggingBar = false;
	Pos m_selAnchor, m_selCaret;
	wxPoint m_lastMouse;
	int m_clickCount = 0;
	wxString m_title, m_cwd;
	bool m_bell = false;

	int m_generation = 0;
	bool m_reporting = false;
	int m_reportBtn = -1;
	int m_wheelAcc = 0;
	long long m_lastDclickMs = 0;
	std::atomic<bool> m_closing{false};

	wxDECLARE_NO_COPY_CLASS(TerminalView);
};