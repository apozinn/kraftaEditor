class AltKeyFilter : public wxEventFilter
{
public:
    AltKeyFilter(wxFrame* frame) : m_frame(frame), m_altPressedAlone(false) {}

    int FilterEvent(wxEvent& event) override
    {
        if (event.GetEventType() == wxEVT_KEY_DOWN)
        {
            wxKeyEvent& key = static_cast<wxKeyEvent&>(event);

            if (key.GetKeyCode() == WXK_ALT)
                m_altPressedAlone = true; 
            else
                m_altPressedAlone = false;
        }
        else if (event.GetEventType() == wxEVT_KEY_UP)
        {
            wxKeyEvent& key = static_cast<wxKeyEvent&>(event);

            if (key.GetKeyCode() == WXK_ALT &&
                m_altPressedAlone            &&
                !key.ControlDown()           &&
                !key.ShiftDown()             &&
                !key.MetaDown())
            {
                m_altPressedAlone = false;
                wxCommandEvent cmd(wxEVT_MENU, +Event::View::ToggleMenuBar);
                wxPostEvent(m_frame, cmd);
                return Event_Processed;
            }

            m_altPressedAlone = false;
        }

        return Event_Skip;
    }

private:
    wxFrame* m_frame;
    bool     m_altPressedAlone;
};