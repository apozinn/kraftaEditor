#include "minimap.hpp"

#include <array>
#include <wx/dcbuffer.h>

void MiniMap::ExtractStyledText()
{
    if (!m_editor)
        return;

    long len = m_editor->GetTextLength();
    if (len <= 0)
        return;

    size_t bytesNeeded = (len * 2) + 2;
    m_buf.SetBufSize(bytesNeeded);

    char *raw = static_cast<char *>(m_buf.GetData());

    Sci_TextRange tr;
    tr.chrg.cpMin = 0;
    tr.chrg.cpMax = len;
    tr.lpstr = raw;

    m_editor->SendMsg(2015, 0, reinterpret_cast<sptr_t>(&tr));

    const auto &lexerStyles = languagePreferences.styles["styles"];
    if (lexerStyles.empty())
        return;

    for (auto it = lexerStyles.begin(); it != lexerStyles.end(); ++it)
    {
        int styleIndex = std::stoi(it.key());
        auto fg = it.value()["foreground"].get<std::string>();

        if (styleIndex >= 0 && styleIndex < (int)m_styleColor.size())
            m_styleColor[styleIndex] = wxColour(fg);
    }

    int totalLines = m_editor->GetLineCount();
    m_lineStart.resize(totalLines);

    for (int i = 0; i < totalLines; i++)
        m_lineStart[i] = m_editor->PositionFromLine(i);

    m_runs.clear();
    m_runs.reserve(len / 3);

    long byteCount = len * 2;
    long posDoc = 0;

    for (long i = 0; i < byteCount;)
    {
        unsigned char style = raw[i + 1];
        int line = m_editor->LineFromPosition(posDoc);
        int column = posDoc - m_lineStart[line];

        MiniRun run;
        run.line = line;
        run.column = column;
        run.length = 1;

        if (
            raw[i] == '\t' ||
            raw[i] == ' ' ||
            raw[i] == '\n' ||
            raw[i] == '\r' ||
            raw[i] == '\v' ||
            raw[i] == '\f' ||
            raw[i] == '\b')
        {
            run.color = wxColour(0, 0, 0, 0);
        }
        else
        {
            run.color = m_styleColor[style];
        }

        i += 2;
        posDoc++;

        while (i < byteCount)
        {
            unsigned char nextStyle = raw[i + 1];
            int nextLine = m_editor->LineFromPosition(posDoc);
            if (nextStyle != style || nextLine != line)
                break;
            run.length++;
            i += 2;
            posDoc++;
        }

        m_runs.push_back(run);
    }

    Refresh();
}

MiniMap::MiniMap(wxWindow *parent, wxStyledTextCtrl *editor)
    : wxPanel(parent), m_editor(editor)
{
    int ph = parent->GetSize().y;
    SetMinSize(wxSize(100, ph));
    SetSize(wxSize(100, ph));
    
    SetBackgroundStyle(wxBG_STYLE_PAINT);

    Bind(wxEVT_PAINT, &MiniMap::OnPaint, this);
    Bind(wxEVT_LEFT_DOWN, &MiniMap::OnMouseDown, this);
    Bind(wxEVT_LEFT_UP, &MiniMap::OnMouseUp, this);
    Bind(wxEVT_MOTION, &MiniMap::OnMouseMove, this);
}

void MiniMap::OnPaint(wxPaintEvent &)
{
    wxAutoBufferedPaintDC dc(this);
    dc.SetBackground(wxBrush(GetBackgroundColour()));
    dc.Clear();

    if (!m_editor)
        return;

    int clientH = GetClientSize().GetHeight();
    int editorFirst = m_editor->GetFirstVisibleLine();

    int firstVisible = editorFirst - minimapPaddingLines;
    if (firstVisible < 0)
        firstVisible = 0;

    int visibleLines = m_editor->LinesOnScreen();
    int caretLine = m_editor->GetCurrentLine();
    int viewportOffset = editorFirst - firstVisible;

    for (const MiniRun &r : m_runs)
    {
        if (r.line < firstVisible)
            continue;

        int y = (r.line - firstVisible) * scaleY;
        if (y > clientH)
            break;

        int x = r.column * scaleX;
        int w = r.length * scaleX;

        dc.SetPen(*wxTRANSPARENT_PEN);
        dc.SetBrush(wxBrush(r.color));
        dc.DrawRectangle(x, y, w, scaleY);
    }

    wxColour viewportColor = m_themesManager.GetColor("minimapViewport");
    viewportColor.Set(viewportColor.Red(), viewportColor.Green(), viewportColor.Blue(), 60);

    wxColour caretColor = m_themesManager.GetColor("minimapCaretPosition");
    caretColor.Set(caretColor.Red(), caretColor.Green(), caretColor.Blue(), 255);

    int vpY = viewportOffset * scaleY;
    int vpH = visibleLines * scaleY;

    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.SetBrush(wxBrush(viewportColor));
    dc.DrawRectangle(0, vpY, GetClientSize().GetWidth(), vpH);

    int caretY = (caretLine - firstVisible) * scaleY;
    if (caretY >= 0 && caretY <= clientH)
    {
        dc.SetBrush(wxBrush(caretColor));
        dc.DrawRectangle(0, caretY, GetClientSize().GetWidth(), scaleY);
    }
}

void MiniMap::ScrollToY(int y)
{
    if (!m_editor)
        return;

    int base = m_editor->GetFirstVisibleLine();
    int target = base + (y / scaleY);

    if (target < 0)
        target = 0;

    int count = m_editor->GetLineCount();
    if (target >= count)
        target = count - 1;

    m_editor->GotoLine(target);
}

void MiniMap::OnMouseDown(wxMouseEvent &e)
{
    CaptureMouse();
    m_dragging = true;
    m_dragOffset = e.GetY();
    ScrollToY(m_dragOffset);
}

void MiniMap::OnMouseUp(wxMouseEvent &)
{
    if (HasCapture())
        ReleaseMouse();
    m_dragging = false;
}

void MiniMap::OnMouseMove(wxMouseEvent &e)
{
    if (!m_dragging)
        return;

    ScrollToY(e.GetY());
}
