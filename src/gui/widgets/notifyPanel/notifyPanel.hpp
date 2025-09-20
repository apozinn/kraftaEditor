#pragma once 

#include "appConstants/appConstants.hpp"
#include <wx/wx.h>

class NotifyPanel : public wxPanel {
	wxBoxSizer* sizer;
	wxPanel* content_box;
	wxPanel* interactions_box;
	wxStaticText* notify_label;
public:
	NotifyPanel(
		wxWindow* parent, 
		std::string notify_text,
		std::vector<NotifyInteractions> ntf_interactions
	);
private:
	void Paint(wxPaintEvent& event);
	void InteractionSelected(wxMouseEvent& WXUNUSED(event));
};