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
#ifndef __MENU_ITEM_HOVER_LISTENER_H__
#define __MENU_ITEM_HOVER_LISTENER_H__
#include <functional>
namespace cdroid{
class MenuBuilder;
class MenuItem;
struct MenuItemHoverListener {
    /**
     * Called when hover exits a menu item.
     * <p>
     * If hover is moving to another item, this method will be called before
     * {@link #onItemHoverEnter(MenuBuilder, MenuItem)} for the newly-hovered item.
     *
     * @param menu the item's parent menu
     * @param item the hovered menu item
     */
    std::function<void(MenuBuilder& /*menu*/,MenuItem&/*item*/)> onItemHoverExit;

    /**
     * Called when hover enters a menu item.
     *
     * @param menu the item's parent menu
     * @param item the hovered menu item
     */
    std::function<void(MenuBuilder& /*menu*/,MenuItem&/*item*/)> onItemHoverEnter;;
};
}/*endof namespace*/
#endif/*__MENU_ITEM_HOVER_LISTENER_H__*/
