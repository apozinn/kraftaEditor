#include "minimapCtrl.hpp"
#include "gui/editor/controls/textCtrl/textCtrl.hpp"

MinimapCtrl::MinimapCtrl(wxWindow *parent, TextCtrl *textCtrl)
	: wxStyledTextCtrlMiniMap(parent, textCtrl) {}