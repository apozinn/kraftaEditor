#pragma once

#include <wx/bitmap.h>
#include <wx/colour.h>
#include <wx/file.h>
#include <wx/filename.h>
#include <wx/string.h>
#include <wx/wx.h>

namespace UiUtils {

wxBitmapBundle CreateIcon(const wxString &iconPath,
						  const wxSize &iconSize = wxDefaultSize,
						  wxColour iconColour = wxNullColour);

} // namespace UiUtils