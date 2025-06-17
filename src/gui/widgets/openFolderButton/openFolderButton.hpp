#pragma once 
#include <wx/wx.h>

class OpenFolderButton : public wxPanel {
public:
	wxBoxSizer* sizer;
	OpenFolderButton(wxWindow* parent, wxWindowID ID);
};