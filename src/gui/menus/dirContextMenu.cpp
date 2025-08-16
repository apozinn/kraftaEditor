#include "menus/dirContextMenu.hpp"
#include "ui/ids.hpp"

namespace DirContextMenu
{
    wxMenu* Get()
    {
        wxMenu *menuDir = new wxMenu;
        menuDir->Append(+Event::File::RenameDir, _("&Rename"));
        menuDir->Append(+Event::File::CreateFileEvent, _("&New File"));
        menuDir->Append(+Event::File::CreateDir, _("&New Folder"));
        menuDir->Append(+Event::File::DeleteDir, _("&Delete Folder"));
        return menuDir;
    }
}
