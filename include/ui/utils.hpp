#pragma once

#include <wx/wx.h>
#include <wx/bitmap.h>
#include <wx/file.h>
#include <wx/filename.h>
#include <wx/colour.h>
#include <wx/string.h>

namespace UiUtils {

wxBitmapBundle CreateIcon(const wxString& iconPath,
                          const wxSize& iconSize = wxDefaultSize,
                          wxColour iconColour = wxNullColour);

} // namespace UiUtils