#include "minimapCtrl.hpp"
#include "gui/editor/controls/textCtrl/textCtrl.hpp"

MinimapCtrl::MinimapCtrl(wxWindow *parent, TextCtrl *textCtrl)
	: m_parent(parent), m_textCtrl(textCtrl),
	  wxStyledTextCtrlMiniMap(parent, textCtrl) {}