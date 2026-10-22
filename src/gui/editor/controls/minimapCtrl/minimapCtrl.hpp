#pragma once
#include <wx/stc/minimap.h>
#include <wx/weakref.h>
#include "gui/editor/controls/textCtrl/textCtrl.hpp"

class MinimapCtrl : public wxStyledTextCtrlMiniMap {
  public:
	MinimapCtrl(wxWindow *parent, TextCtrl *textCtrl);
	void RecreateMinimap();

  private:
};