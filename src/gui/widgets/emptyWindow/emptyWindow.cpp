#include "emptyWindow.hpp"
#include "gui/widgets/imagePanel/imagePanel.hpp"

EmptyWindow::EmptyWindow(wxWindow* parent, wxWindowID ID) : wxPanel(parent, ID)
{
	auto background_color = Theme["secondary"].template get<std::string>();
	SetBackgroundColour(wxColor(background_color));
}