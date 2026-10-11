#pragma once

#include "ui/ids.hpp"
#include "wx/wx.h"
#include <wx/splitter.h>
#include <wx/string.h>

#include "fileOperations/fileOperations.hpp"

#include "gui/editor/core/editor.hpp"
#include "gui/panels/tabs/tabs.hpp"
#include "gui/widgets/statusBar/statusBar.hpp"

namespace SplitEditorManager {
void CreateSplitedEditor(const wxString &path);
}