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
#include <menu/menuitemimpl.h>
#include <menu/menupopuphelper.h>
#include <menu/menudialoghelper.h>
#include <menu/contextmenubuilder.h>
namespace cdroid{

ContextMenuBuilder::ContextMenuBuilder(Context*context)
    :MenuBuilder(context){
}

ContextMenu& ContextMenuBuilder::setHeaderIcon(Drawable* icon) {
    return (ContextMenu&) MenuBuilder::setHeaderIconInt(icon);
}

/*ContextMenu& ContextMenuBuilder::setHeaderIcon(int iconRes) {
    return (ContextMenu&) MenuBuilder::setHeaderIconInt(iconRes);
}*/

ContextMenu& ContextMenuBuilder::setHeaderTitle(const std::string& title) {
    return (ContextMenu&) MenuBuilder::setHeaderTitleInt(title);
}

/*ContextMenu& ContextMenuBuilder::setHeaderTitle(int titleRes) {
    return (ContextMenu&) MenuBuilder::setHeaderTitleInt(titleRes);
}*/

ContextMenu& ContextMenuBuilder::setHeaderView(View* view) {
    return (ContextMenu&) MenuBuilder::setHeaderViewInt(view);
}

MenuDialogHelper* ContextMenuBuilder::showDialog(View* originalView/*, IBinder token*/) {
    if (originalView != nullptr) {
        // Let relevant views and their populate context listeners populate
        // the context menu
        originalView->createContextMenu(*this);
    }

    if (getVisibleItems().size() > 0) {
        MenuDialogHelper*helper = new MenuDialogHelper(this);
        helper->show(/*token*/);
        return helper;
    }

    return nullptr;
}

MenuPopupHelper* ContextMenuBuilder::showPopup(Context* context, View* originalView, float x, float y) {
    if (originalView != nullptr) {
        // Let relevant views and their populate context listeners populate
        // the context menu
        originalView->createContextMenu(*this);
    }

    if (getVisibleItems().size() > 0) {

        int location[2];
        originalView->getLocationOnScreen(location);

        MenuPopupHelper* helper = new MenuPopupHelper(
                context, this, originalView, false /* overflowOnly */,
                "android:attr/contextPopupMenuStyle");
        helper->show(std::round(x), std::round(y));
        return helper;
    }

    return nullptr;
}
}/*endof namespace*/
