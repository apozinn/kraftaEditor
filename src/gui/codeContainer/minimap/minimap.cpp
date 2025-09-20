#include "./minimap.hpp"
#include <wx/graphics.h>

MiniMap::MiniMap(wxWindow *parent) : wxStyledTextCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE)
{
    currentPath = parent->GetLabel();
    if (auto linkedEditor = ((wxStyledTextCtrl *)GetParent()->GetChildren()[0]))
        Editor = linkedEditor;
    InitializePrefs();
}

void MiniMap::InitializePrefs()
{
    auto backgroundColor = Theme["secondary"].template get<std::string>();
    auto textColor = Theme["text"].template get<std::string>();

    SetUseHorizontalScrollBar(false);
    SetUseVerticalScrollBar(false);

    StyleSetBackground(wxSTC_STYLE_DEFAULT, wxColor(backgroundColor));
    StyleSetForeground(wxSTC_STYLE_DEFAULT, wxColor(221, 221, 221));

    StyleClearAll();
    SetCaretForeground(wxColor(backgroundColor));

    SetMarginWidth(0, 0);
    SetMarginType(0, 0);
    StyleSetForeground(wxSTC_STYLE_LINENUMBER, wxColor(textColor));
    StyleSetBackground(wxSTC_STYLE_LINENUMBER, wxColor(backgroundColor));

    SetMarginWidth(1, 0);
    SetMarginType(1, 0);
    SetMarginMask(1, 0);
    SetMarginSensitive(1, false);

    Bind(wxEVT_LEFT_UP, &MiniMap::OnClick, this);
    Bind(wxEVT_ENTER_WINDOW, &MiniMap::OnMouseEnter, this);
    Bind(wxEVT_LEAVE_WINDOW, &MiniMap::OnMouseLeave, this);
    Bind(wxEVT_PAINT, &MiniMap::OnPaint, this);
    Bind(wxEVT_STC_PAINTED, &MiniMap::OnPainted, this);

    SetZoom(-8);
    SetSTCCursor(wxSTC_CURSORARROW);
    SetSize(wxSize(100, GetSize().GetHeight() * 2));
    SetMinSize(wxSize(100, GetSize().GetHeight() * 2));

    SetCaretWidth(0);
}

void MiniMap::OnClick(wxMouseEvent &event)
{
    wxClientDC dc(this);
    clickPoint = event.GetLogicalPosition(dc).y;

    // defining the first visible line of the Editor
    Editor->SetFirstVisibleLine(GetCurrentLine());
    Editor->Refresh();

    mouseOver = true;
    Refresh();

    event.Skip();
}

void MiniMap::OnPaint(wxPaintEvent &event) { event.Skip(); }

void MiniMap::OnPainted(wxStyledTextEvent &WXUNUSED(event))
{
    if (!mouseOver)
        return;
    wxPaintDC dc(this);
    wxGraphicsContext *gc = wxGraphicsContext::Create(dc);

    if (gc)
    {
        // draw a rectangle indicating where the mouse is
        wxColour color(128, 128, 128, 20);
        gc->SetBrush(color);
        gc->SetPen(color);
        gc->DrawRectangle(0, clickPoint - 25, GetSize().GetWidth(), 50);
        delete gc;
    }

    Editor->SetFocus();
}

void MiniMap::OnMouseEnter(wxMouseEvent &WXUNUSED(event))
{
    mouseOver = true;
    Refresh();
}

void MiniMap::OnMouseLeave(wxMouseEvent &WXUNUSED(event))
{
    mouseOver = false;
    Refresh();
}