#pragma once

#include "ui/ids.hpp"
#include "wx/wx.h"
#include <wx/string.h>
#include <wx/splitter.h>

#include "fileOperations/fileOperations.hpp"

#include "gui/panels/tabs/tabs.hpp"
#include "gui/widgets/statusBar/statusBar.hpp"
#include "gui/codeContainer/code.hpp"

namespace SplitEditorManager {
    void CreateSplitedEditor(const wxString& path);
}