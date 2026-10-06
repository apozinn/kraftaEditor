#pragma once
#include <wx/stc/minimap.h>

class TextCtrl;

class MinimapCtrl : public wxStyledTextCtrlMiniMap {
    public:
    MinimapCtrl(wxWindow* parent, TextCtrl* textCtrl);
    void RecreateMinimap();
    private:
    wxWeakRef<wxWindow> m_parent;
    wxWeakRef<TextCtrl> m_textCtrl;
};