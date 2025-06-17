#pragma once
#include <wx/string.h>
#include <wx/colour.h>

struct StyleInfo {
    wxString name;
    wxColour foreground;
    wxString fontname;
    int fontsize;
    int fontstyle;
    int lettercase;
};

extern StyleInfo global_lexer_styles[];