/*********************************************************************************
 * Copyright (C) 2019] [houzh@msn.com]
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
#include <widgetEx/navigationview/navigationbarmenubuilder.h>
#include <menu/menubuilder.h>
#include <menu/submenu.h>
#include <menu/menupresenter.h>
#include <menu/menuitem.h>

namespace cdroid{

NavigationBarMenuBuilder::NavigationBarMenuBuilder(MenuBuilder* menuBuilder)
    : mMenuBuilder(menuBuilder)
    , mContentItemCount(0)
    , mVisibleContentItemCount(0)
    , mVisibleMainItemCount(0) {
    refreshItems();
}

int NavigationBarMenuBuilder::size() const {
    return (int)mItems.size();
}

int NavigationBarMenuBuilder::getContentItemCount() const {
    return mContentItemCount;
}

int NavigationBarMenuBuilder::getVisibleContentItemCount() const {
    return mVisibleContentItemCount;
}

int NavigationBarMenuBuilder::getVisibleMainContentItemCount() const {
    return mVisibleMainItemCount;
}

MenuItem* NavigationBarMenuBuilder::getItemAt(int i) {
    return mItems.at(i);
}

bool NavigationBarMenuBuilder::performItemAction(MenuItem* item, MenuPresenter* presenter, int flags) {
    return mMenuBuilder->performItemAction(item, presenter, flags);
}

void NavigationBarMenuBuilder::refreshItems() {
    mItems.clear();
    mContentItemCount = 0;
    mVisibleContentItemCount = 0;
    mVisibleMainItemCount = 0;
    const int count = mMenuBuilder->size();
    for (int i = 0; i < count; i++) {
        MenuItem* item = mMenuBuilder->getItem(i);
        if (item->hasSubMenu()) {
            // AOSP inserts DividerMenuItems around the submenu; CDROID does not
            // port them - flatten the submenu children in place. Children of a
            // hidden subheader are hidden along with it (AOSP parity).
            mItems.push_back(item);
            SubMenu* subMenu = item->getSubMenu();
            const int subCount = subMenu->size();
            for (int j = 0; j < subCount; j++) {
                MenuItem* submenuItem = subMenu->getItem(j);
                if (!item->isVisible()) {
                    submenuItem->setVisible(false);
                }
                mItems.push_back(submenuItem);
                mContentItemCount++;
                if (submenuItem->isVisible()) {
                    mVisibleContentItemCount++;
                }
            }
        } else {
            mItems.push_back(item);
            mContentItemCount++;
            if (item->isVisible()) {
                mVisibleContentItemCount++;
                mVisibleMainItemCount++;
            }
        }
    }
}

} // namespace cdroid
