#include "controlPanel.hpp"

wxBEGIN_EVENT_TABLE(ControlPanel, wxPanel)
    EVT_MENU(+Event::ControlPanel::Exit, ControlPanel::Close)
    EVT_MENU(+Event::ControlPanel::Up, ControlPanel::UpSelection)
    EVT_MENU(+Event::ControlPanel::Down, ControlPanel::DownSelection)
    EVT_MENU(+Event::ControlPanel::Select, ControlPanel::OnKeyBoardSelect)
wxEND_EVENT_TABLE()