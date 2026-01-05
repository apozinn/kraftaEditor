#include "codeSearch.hpp"
#include "ui/ids.hpp"

Search::Search(wxWindow *parent, const wxString &defaultLabel, wxStyledTextCtrl *editor)
    : wxPanel(parent, +GUI::ControlID::CodeSearch, wxPoint(parent->GetSize().GetWidth() - 360, 50), wxSize(250, 35)),
      m_input(nullptr),
      m_editor(editor)
{
    wxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);

    SetBackgroundColour(parent->GetBackgroundColour());

    m_input = new wxStyledTextCtrl(
        this,
        wxID_ANY,
        wxDefaultPosition,
        wxSize(240, 20),
        wxBORDER_NONE);

    m_input->SetWrapMode(wxSTC_WRAP_NONE);
    m_input->SetUseVerticalScrollBar(false);
    m_input->SetUseHorizontalScrollBar(false);
    m_input->SetMarginWidth(0, 0);
    m_input->SetMarginWidth(1, 0);
    m_input->SetCaretWidth(2);
    m_input->StyleClearAll();

    m_input->SetFocus();

    m_input->Bind(wxEVT_STC_CHANGE, &Search::OnChar, this);

    sizer->Add(m_input, 0, wxALL, 5);

    wxAcceleratorEntry accel[1];
    accel[0].Set(wxACCEL_NORMAL, WXK_ESCAPE, wxID_CLOSE);
    SetAcceleratorTable(wxAcceleratorTable(1, accel));

    Bind(wxEVT_MENU, &Search::Close, this, wxID_CLOSE);
    Bind(wxEVT_PAINT, &Search::OnPaint, this);

    if (m_editor)
    {
        m_editor->IndicatorSetStyle(SEARCH_INDICATOR, wxSTC_INDIC_ROUNDBOX);
        m_editor->IndicatorSetForeground(SEARCH_INDICATOR, wxColour(255, 80, 80));
        m_editor->IndicatorSetAlpha(SEARCH_INDICATOR, 80);
    }

    if (!defaultLabel.IsEmpty())
        m_input->SetText(defaultLabel);

    SetSizerAndFit(sizer);
}

Search::~Search()
{
    if (m_editor)
        m_editor->IndicatorClearRange(0, m_editor->GetLength());
}

void Search::OnChar(wxStyledTextEvent &event)
{
    char key = static_cast<char>(event.GetKey());

    if (key == '\n')
    {
        DoSearch(true);
        return;
    }

    DoSearch(true);
}

void Search::DoSearch(bool forward)
{
    if (!m_editor)
        return;

    wxString query = m_input->GetText();
    int length = m_editor->GetLength();

    m_editor->IndicatorClearRange(0, length);

    if (query.IsEmpty())
        return;

    m_editor->SetSearchFlags(0);
    m_editor->SetTargetStart(0);
    m_editor->SetTargetEnd(length);

    int pos = 0;

    while (true)
    {
        pos = m_editor->SearchInTarget(query);
        if (pos == -1)
            break;

        m_editor->IndicatorFillRange(pos, query.Length());
        m_editor->SetTargetStart(pos + query.Length());
        m_editor->SetTargetEnd(length);
    }

    m_editor->SetTargetStart(0);
    m_editor->SetTargetEnd(length);

    int first = m_editor->SearchInTarget(query);
    if (first != -1)
        m_editor->GotoPos(first);
}

void Search::Close(wxCommandEvent &)
{
    if (m_editor)
        m_editor->IndicatorClearRange(0, m_editor->GetLength());

    Destroy();
}

void Search::OnPaint(wxPaintEvent&)
{
    wxPaintDC dc(this);

    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.SetPen(wxPen(m_borderColor, 1));

    wxSize size = GetClientSize();
    dc.DrawRectangle(0, 0, size.GetWidth(), size.GetHeight());
}
