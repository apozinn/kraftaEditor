#include "editor.hpp"

wxBEGIN_EVENT_TABLE(Editor, wxStyledTextCtrl)
    EVT_STC_MODIFIED(wxID_ANY, Editor::OnChange)
    EVT_STC_MARGINCLICK(wxID_ANY, Editor::OnMarginClick)
    EVT_KEY_UP(Editor::OnArrowsPress)
    EVT_STC_CHARADDED(wxID_ANY, Editor::CharAdd)
    EVT_LEFT_UP(Editor::OnClick)
    EVT_MOUSEWHEEL(Editor::OnScroll)
    EVT_KEY_DOWN(Editor::OnBackspace)
    EVT_STC_UPDATEUI(wxID_ANY, Editor::OnUpdateUI)

    EVT_MENU(+Event::Edit::MoveCursorDown,Editor::OnMoveCursorDown)
    EVT_MENU(+Event::Edit::MoveCursorUp, Editor::OnMoveCursorUp)
    EVT_MENU(+Event::Edit::DuplicateLineDown, Editor::OnDuplicateLineDown)
    EVT_MENU(+Event::Edit::DuplicateLineUp, Editor::OnDuplicateLineUp)
    EVT_MENU(+Event::Edit::SelectNextOccurrence, Editor::SelectNextOccurrence)

    
wxEND_EVENT_TABLE()
