#include "menus/tabsContainerMenu.hpp"
#include "ui/ids.hpp"

namespace TabsContainerMenu
{
    wxMenu *Get()
    {
        wxMenu *menu = new wxMenu;
        menu->Append(+Event::File::CloseAll, _("&Close All"));
        return menu;
    }
}
