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
#ifndef __CONTEXT_MENU_BUILDER_H__
#define __CONTEXT_MENU_BUILDER_H__
#include <menu/menubuilder.h>
#include <menu/contextmenu.h>
#include <menu/menupopuphelper.h>
#include <menu/menudialoghelper.h>
namespace cdroid{
class MenuPopupHelper;
class MenuDialogHelper;
class ContextMenuBuilder:public MenuBuilder,public ContextMenu {
public:
    ContextMenuBuilder(Context* context);

    ContextMenu& setHeaderIcon(Drawable* icon);
    //ContextMenu& setHeaderIcon(int iconRes);

    ContextMenu& setHeaderTitle(const std::string& title);
    //ContextMenu& setHeaderTitle(int titleRes);

    ContextMenu& setHeaderView(View* view);

    /**
     * Shows this context menu, allowing the optional original view (and its
     * ancestors) to add items.
     *
     * @param originalView Optional, the original view that triggered the context menu.
     * @param token Optional, the window token that should be set on the context menu's window.
     * @return If the context menu was shown, the {@link MenuDialogHelper} fordismissing it. Otherwise, null.
     */
    MenuDialogHelper* showDialog(View* originalView/*,IBinder token*/);

    MenuPopupHelper* showPopup(Context* context, View* originalView, float x, float y);
};
}/*endof namespace*/
#endif/*__CONTEXT_MENU_BUILDER_H__*/
