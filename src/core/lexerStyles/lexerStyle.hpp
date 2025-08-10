#pragma once
#include <wx/string.h>
#include <wx/colour.h>

enum {
    FONTSTYLE_BOLD = 1,
    FONTSTYLE_ITALIC = 2,
    FONTSTYLE_UNDERLINE = 4
};

struct StyleInfo {
    wxString name;
    wxColour foreground;
    wxString fontname;
    int fontsize;
    int fontstyle;
    int lettercase;
};

extern StyleInfo global_lexer_styles[];