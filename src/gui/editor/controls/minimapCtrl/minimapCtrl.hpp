#pragma once
#include "gui/editor/controls/textCtrl/textCtrl.hpp"
#include <wx/stc/minimap.h>
#include <wx/weakref.h>

class MinimapCtrl : public wxStyledTextCtrlMiniMap {
  public:
	MinimapCtrl(wxWindow *parent, TextCtrl *textCtrl);
	void RecreateMinimap();

  private:
};