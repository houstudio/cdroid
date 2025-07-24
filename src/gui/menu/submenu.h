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
#ifndef __SUBMENU_H__
#define __SUBMENU_H__
#include <menu/menu.h>
namespace cdroid{
class SubMenu:virtual public Menu {
public:

    /**
     * Sets the submenu header's title to the title given in <var>title</var>.
     *
     * @param title The character sequence used for the title.
     * @return This SubMenu so additional setters can be called.
     */
    virtual SubMenu& setHeaderTitle(const std::string& title)=0;

    /**
     * Sets the submenu header's icon to the icon given in <var>iconRes</var>
     * resource id.
     *
     * @param iconRes The resource identifier used for the icon.
     * @return This SubMenu so additional setters can be called.
     */
    virtual SubMenu& setHeaderIcon(const std::string&iconRes)=0;

    /**
     * Sets the submenu header's icon to the icon given in <var>icon</var>
     * {@link Drawable}.
     *
     * @param icon The {@link Drawable} used for the icon.
     * @return This SubMenu so additional setters can be called.
     */
    virtual SubMenu& setHeaderIcon(Drawable* icon)=0;

    /**
     * Sets the header of the submenu to the {@link View} given in
     * <var>view</var>. This replaces the header title and icon (and those
     * replace this).
     *
     * @param view The {@link View} used for the header.
     * @return This SubMenu so additional setters can be called.
     */
    virtual SubMenu& setHeaderView(View* view)=0;

    /**
     * Clears the header of the submenu.
     */
    virtual void clearHeader()=0;

    /**
     * Change the icon associated with this submenu's item in its parent menu.
     *
     * @see MenuItem#setIcon(int)
     * @param iconRes The new icon (as a resource ID) to be displayed.
     * @return This SubMenu so additional setters can be called.
     */
    virtual SubMenu& setIcon(const std::string& iconRes)=0;

    /**
     * Change the icon associated with this submenu's item in its parent menu.
     *
     * @see MenuItem#setIcon(Drawable)
     * @param icon The new icon (as a Drawable) to be displayed.
     * @return This SubMenu so additional setters can be called.
     */
    virtual SubMenu& setIcon(Drawable* icon)=0;

    /**
     * Gets the {@link MenuItem} that represents this submenu in the parent
     * menu.  Use this for setting additional item attributes.
     *
     * @return The {@link MenuItem} that launches the submenu when invoked.
     */
    virtual MenuItem* getInvokerItem()=0;
};
}/*endof namespace*/
#endif/*__SUBMENU_H__*/
