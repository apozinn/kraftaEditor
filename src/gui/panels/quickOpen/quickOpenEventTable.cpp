#include "gui/panels/quickOpen/quickOpen.hpp"

wxBEGIN_EVENT_TABLE(QuickOpen, wxPanel)
    EVT_MENU(+Event::QuickOpen::Exit, QuickOpen::Close)
    EVT_MENU(+Event::QuickOpen::Up, QuickOpen::OnKeyboardUp)
    EVT_MENU(+Event::QuickOpen::Down, QuickOpen::OnKeyboardDown)
    EVT_MENU(+Event::QuickOpen::Select, QuickOpen::OnKeyboardEnter)
wxEND_EVENT_TABLE()