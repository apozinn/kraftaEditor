#include "gotoline.hpp"

wxBEGIN_EVENT_TABLE(Gotoline, wxPanel)
    EVT_MENU(+Event::Gotoline::Exit, Gotoline::Close)
wxEND_EVENT_TABLE()