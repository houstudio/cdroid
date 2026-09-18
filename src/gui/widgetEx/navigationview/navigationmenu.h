/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
#ifndef __NAVIGATION_MENU_H__
#define __NAVIGATION_MENU_H__
// Port of com.google.android.material.internal.NavigationMenu and
// NavigationSubMenu: the only MenuBuilder customization is which SubMenu
// implementation addSubMenu() creates.
#include <menu/menubuilder.h>
#include <menu/submenubuilder.h>

namespace cdroid{

class NavigationSubMenu:public SubMenuBuilder{
public:
    NavigationSubMenu(Context* context, MenuBuilder* parentMenu, MenuItemImpl* item)
        :SubMenuBuilder(context, parentMenu, item){}
};

class NavigationMenu:public MenuBuilder{
public:
    NavigationMenu(Context* context):MenuBuilder(context){}

    SubMenu* addSubMenu(int group, int id, int categoryOrder,
                        const std::string& title)override;
};

inline SubMenu* NavigationMenu::addSubMenu(int group, int id, int categoryOrder,
                                            const std::string& title){
    MenuItemImpl* item = (MenuItemImpl*)addInternal(group, id, categoryOrder, title);
    SubMenuBuilder* subMenu = new NavigationSubMenu(getContext(), this, item);
    item->setSubMenu(subMenu);
    return subMenu;
}

}//namespace cdroid
#endif/*__NAVIGATION_MENU_H__*/
