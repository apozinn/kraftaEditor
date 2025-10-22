#include "editor.hpp"

wxBEGIN_EVENT_TABLE(Editor, wxStyledTextCtrl)
    // Core Editor Events
    EVT_STC_STYLENEEDED(wxID_ANY,        Editor::OnStyleNeeded)
    EVT_STC_MODIFIED(wxID_ANY,           Editor::OnChange)
    EVT_STC_MARGINCLICK(wxID_ANY,        Editor::OnMarginClick)
    EVT_KEY_UP(                         Editor::OnArrowsPress)
    EVT_STC_CHARADDED(wxID_ANY,          Editor::CharAdd)
    EVT_LEFT_UP(                         Editor::OnClick)
    EVT_STC_AUTOCOMP_COMPLETED(wxID_ANY, Editor::OnAutoCompCompleted)
    EVT_STC_CLIPBOARD_PASTE(wxID_ANY,    Editor::OnClipBoardPaste)
    EVT_MOUSEWHEEL(                      Editor::OnScroll)
    EVT_KEY_DOWN(                        Editor::OnBackspace)
    EVT_STC_UPDATEUI(wxID_ANY,           Editor::OnUpdateUI)

    // Custom Keyboard Shortcuts
    EVT_MENU(+Event::Edit::MoveCursorDown, Editor::OnMoveCursorDown)
    EVT_MENU(+Event::Edit::MoveCursorUp,   Editor::OnMoveCursorUp)
    EVT_MENU(+Event::Edit::DuplicateLineDown, Editor::OnDuplicateLineDown)
    EVT_MENU(+Event::Edit::DuplicateLineUp,   Editor::OnDuplicateLineUp)
wxEND_EVENT_TABLE()
