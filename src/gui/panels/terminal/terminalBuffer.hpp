#pragma once

#include <cstdint>
#include <deque>
#include <functional>
#include <string>
#include <vector>

struct TermColor {
    enum Kind : uint8_t { Default = 0, Indexed = 1, RGB = 2 };

    Kind    kind = Default;
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;

    bool operator==(const TermColor& o) const {
        return kind == o.kind && r == o.r && g == o.g && b == o.b;
    }
    bool operator!=(const TermColor& o) const { return !(*this == o); }
};

/** @brief Text attribute flags (SGR). */
enum TermAttr : uint16_t {
    ATTR_BOLD      = 1 << 0,
    ATTR_DIM       = 1 << 1,
    ATTR_ITALIC    = 1 << 2,
    ATTR_UNDERLINE = 1 << 3,
    ATTR_BLINK     = 1 << 4,
    ATTR_INVERSE   = 1 << 5,
    ATTR_HIDDEN    = 1 << 6,
    ATTR_STRIKE    = 1 << 7
};

/**
 * @brief A single screen cell.
 *
 * width == 0 means continuation of a wide character started in the previous
 * cell. width == 2 means a wide character occupying this cell and the next.
 */
struct TermCell {
    char32_t  ch     = U' ';
    TermColor fg;
    TermColor bg;
    uint16_t  attrs  = 0;
    uint8_t   width  = 1;
};

/** @brief A single screen row. */
struct TermRow {
    std::vector<TermCell> cells;
    bool                  wrapped = false; /**< true if the line continues on the next. */
};

/**
 * @class TerminalBuffer
 * @brief Terminal emulator: escape sequence parser + screen model.
 *
 * Responsibilities:
 *  - Parse bytes (UTF-8 + CSI/OSC/ESC) coming from the PTY.
 *  - Maintain primary screen, alternate screen and scrollback.
 *  - Expose renderable text and callbacks for events (title, bell, cwd).
 *
 * GUI-independent. Can be used from any thread as long as access is
 * serialized externally.
 */
class TerminalBuffer {
public:
    /** @brief Callbacks for external events fired by the parser. */
    struct Callbacks {
        std::function<void()>                   onBell;
        std::function<void(const std::string&)> onTitle;
        std::function<void(const std::string&)> onCwd;
        std::function<void(const std::string&)> onResponse;
    };

    /** @brief Terminal modes toggled by escape sequences. */
    struct Modes {
        bool appCursorKeys = false;
        bool originMode    = false;
        bool autoWrap      = true;
        bool cursorVisible = true;
        bool altScreen     = false;
        bool focusEvents   = false;
        bool mouseSgr      = false;
        bool bracketedPaste = false;
        bool insertMode    = false;
        bool appKeypad     = false;
        int  mouse         = 0;
    };

    /**
     * @brief Constructs the buffer.
     * @param cols Initial columns (minimum 2).
     * @param rows Initial rows (minimum 1).
     * @param maxScrollback Maximum scrollback lines (0 disables).
     */
    TerminalBuffer(int cols, int rows, int maxScrollback);

    /** @brief Sets the event callbacks. */
    void SetCallbacks(const Callbacks& cb) { m_cb = cb; }

    // -------------------------------------------------------------- input

    /** @brief Feeds the parser with UTF-8 bytes coming from the PTY. */
    void Feed(const char* data, size_t len);

    // ------------------------------------------------------------- state

    /** @brief Resets all state (equivalent to RIS). */
    void Reset();

    /**
     * @brief Resizes the screen.
     * @param cols New columns.
     * @param rows New rows.
     */
    void Resize(int cols, int rows);

    /** @brief Clears scrollback (ED mode 3). */
    void ClearScrollback();

    // ------------------------------------------------------------- output

    /**
     * @brief Extracts text in the range [l1,c1]..[l2,c2] in absolute coords.
     *
     * Lines include scrollback (index 0 is the oldest line).
     */
    std::string GetText(int l1, int c1, int l2, int c2) const;

    // -------------------------------------------------------- introspection

    int Cols() const { return m_cols; }
    int Rows() const { return m_rows; }

    /** @brief Total lines (scrollback + screen). */
    int TotalLines() const { return (int)m_main.size(); }

    /** @brief Number of scrollback lines. */
    int ScrollbackLines() const {
        return (int)m_main.size() - m_rows;
    }

    /** @brief Cursor: current column. */
    int CursorX() const { return m_x; }
    /** @brief Cursor: current row (relative to the screen). */
    int CursorY() const { return m_y; }

    /** @brief Absolute index of the cursor line (scrollback + screen). */
    int CursorLine() const {
        return (int)m_main.size() - m_rows + m_y;
    }

    /** @brief Cursor style (DECSCUSR). */
    int CursorStyle() const { return m_cursorStyle; }
    /** @brief Whether the cursor is visible (DECTCEM). */
    bool CursorVisible() const { return m_modes.cursorVisible; }

    /** @brief Whether the alternate screen is active. */
    bool InAltScreen() const { return m_modes.altScreen; }

    /** @brief Active mouse mode (0 = disabled). */
    int MouseMode() const { return m_modes.mouse; }
    /** @brief Whether the SGR mouse protocol is active. */
    bool MouseSgr() const { return m_modes.mouseSgr; }
    /** @brief Whether bracketed paste is active. */
    bool BracketedPaste() const { return m_modes.bracketedPaste; }
    /** @brief Whether application cursor keys are active. */
    bool AppCursorKeys() const { return m_modes.appCursorKeys; }
    /** @brief Whether application keypad is active. */
    bool AppKeypad() const { return m_modes.appKeypad; }
    /** @brief Whether focus events are active. */
    bool FocusEvents() const { return m_modes.focusEvents; }

    /** @brief Returns the current terminal modes. */
    const Modes& GetModes() const { return m_modes; }

    /** @brief Version counter incremented on every visible change. */
    uint64_t Version() const { return m_version; }

    /** @brief Accesses a row by absolute index (scrollback + screen). */
    const TermRow& Line(int index) const { return m_main[(size_t)index]; }

    /**
     * @brief Accesses a row by absolute index on the active screen.
     *
     * On alternate screen, returns the alt screen row.
     */
    const TermRow& VisibleLine(int index) const {
        return (m_modes.altScreen ? m_alt : m_main)[(size_t)index];
    }

    /** @brief Returns the display width of a Unicode codepoint. */
    static int  CharWidth(char32_t c);
    /** @brief Appends a Unicode codepoint to a string as UTF-8. */
    static void AppendUtf8(std::string& s, char32_t c);

private:
    // --------------------------------------------------------------- types

    using Screen = std::deque<TermRow>;

    struct Saved {
        int x = 0;
        int y = 0;
        TermColor fg;
        TermColor bg;
        uint16_t attrs = 0;
        bool wrapPending = false;
        bool origin = false;
        int charset[2] = {0, 0};
        int gl = 0;
    };

    enum class State {
        Ground,
        Escape,
        EscIntermediate,
        Csi,
        Osc,
        StringIgnore,
        StringEsc
    };

    // ----------------------------------------------------------- utilities

    TermCell Blank() const;
    TermRow  MakeRow() const;

    Screen&       Active()       { return m_modes.altScreen ? m_alt : m_main; }
    const Screen& Active() const { return m_modes.altScreen ? m_alt : m_main; }

    /** @brief Row visible on screen (0 = top of the screen). */
    TermRow&       ScreenRow(int y)       { return Active()[(size_t)(Active().size() - m_rows + y)]; }
    const TermRow& ScreenRow(int y) const { return Active()[(size_t)(Active().size() - m_rows + y)]; }

    void Touch() { ++m_version; }

    void ResetTabs();
    void ClampCursor();

    int  Pn(size_t i, int def = 1) const;

    // ------------------------------------------------------------ commands

    void ScrollUp(int n);
    void ScrollDown(int n);
    void LineFeed();
    void ReverseIndex();
    void CarriageReturn() { m_x = 0; m_wrapPending = false; }
    void MoveTo(int row, int col);

    void EraseCells(int row, int c1, int c2);
    void EraseLine(int mode);
    void EraseDisplay(int mode);

    void InsertLines(int n);
    void DeleteLines(int n);
    void InsertChars(int n);
    void DeleteChars(int n);

    void SaveCursor();
    void RestoreCursor();
    void SwitchAlt(bool on, bool saveCursor);

    void Print(char32_t c);

    // -------------------------------------------------------------- parser

    void Process(uint8_t b);
    void Execute(uint8_t c);
    void EscDispatch(uint8_t c);
    void CsiDispatch(uint8_t f);
    void OscDispatch();
    void SetMode(bool priv, int mode, bool on);
    void Sgr();

    void Respond(const std::string& s);

    // --------------------------------------------------------------- state

    int m_cols;
    int m_rows;
    int m_maxScrollback;

    Screen m_main;
    Screen m_alt;

    Modes m_modes;

    int  m_x = 0;
    int  m_y = 0;
    bool m_wrapPending = false;
    int  m_top = 0;
    int  m_bottom = 0;

    TermColor m_fg;
    TermColor m_bg;
    uint16_t  m_attrs = 0;

    Saved m_saved;
    Saved m_savedAlt;

    int m_charset[2] = {0, 0};
    int m_gl = 0;

    int m_cursorStyle = 1;

    std::vector<bool> m_tabs;

    char32_t m_lastChar = 0;

    State m_state = State::Ground;

    int      m_utf8Cp = 0;
    int      m_utf8Need = 0;

    std::vector<int> m_params;
    bool             m_haveParam = false;
    char             m_priv = 0;
    std::string      m_inter;
    char             m_escInter = 0;

    std::string m_osc;

    Callbacks m_cb;

    uint64_t m_version = 0;
};