#pragma once

#include <wx/weakref.h>

class TextCtrl;

class SyntaxHighlighting {
    public:
    SyntaxHighlighting(TextCtrl* textCtrl);
    private:
    wxWeakRef<TextCtrl>m_textCtrl;
};