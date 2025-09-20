#include "menus/fileContextMenu.hpp"
#include "ui/ids.hpp"

namespace FileContextMenu
{
    wxMenu* Get()
    {
        wxMenu *menuFile = new wxMenu;
        menuFile->Append(+Event::File::RenameFile, _("&Rename"));
        menuFile->Append(+Event::File::DeleteFileEvent, _("&Delete File"));
        return menuFile;
    }
}
