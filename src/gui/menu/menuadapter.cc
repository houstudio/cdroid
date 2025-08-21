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
#include <menu/menuadapter.h>
#include <menu/menubuilder.h>
#include <menu/menuview.h>
#include <menu/menuitemimpl.h>
#include <menu/listmenuitemview.h>
namespace cdroid{

MenuAdapter::MenuAdapter(MenuBuilder* menu, LayoutInflater* inflater, bool overflowOnly, const std::string& itemLayoutRes) {
    mOverflowOnly = overflowOnly;
    mInflater = inflater;
    mAdapterMenu = menu;
    mItemLayoutRes = itemLayoutRes;
    findExpandedIndex();
}

bool MenuAdapter::getForceShowIcon() const{
    return mForceShowIcon;
}

void MenuAdapter::setForceShowIcon(bool forceShow) {
    mForceShowIcon = forceShow;
}

int MenuAdapter::getCount()const {
    auto items = mOverflowOnly ?
            mAdapterMenu->getNonActionItems() : mAdapterMenu->getVisibleItems();
    if (mExpandedIndex < 0) {
        return items.size();
    }
    return items.size() - 1;
}

MenuBuilder* MenuAdapter::getAdapterMenu() const{
    return mAdapterMenu;
}

void* MenuAdapter::getItem(int position)const{
    auto items = mOverflowOnly ?
            mAdapterMenu->getNonActionItems() : mAdapterMenu->getVisibleItems();
    if (mExpandedIndex >= 0 && position >= mExpandedIndex) {
        position++;
    }
    return items.at(position);
}

long MenuAdapter::getItemId(int position) {
    // Since a menu item's ID is optional, we'll use the position as an
    // ID for the item in the AdapterView
    return position;
}

View* MenuAdapter::getView(int position, View* convertView, ViewGroup* parent) {
    if (convertView == nullptr) {
        convertView = mInflater->inflate(mItemLayoutRes, parent, false);
    }

    const int currGroupId = ((MenuItemImpl*)getItem(position))->getGroupId();
    const MenuItemImpl*menuItem =(MenuItemImpl*)getItem(position - 1);
    const int prevGroupId =  position - 1 >= 0 ? menuItem->getGroupId() : currGroupId;
    // Show a divider if adjacent items are in different groups.
    ((ListMenuItemView*) convertView)->setGroupDividerEnabled(mAdapterMenu->isGroupDividerEnabled()
                    && (currGroupId != prevGroupId));

    MenuView::ItemView* itemView = (MenuView::ItemView*) convertView;
    if (mForceShowIcon) {
        ((ListMenuItemView*) convertView)->setForceShowIcon(true);
    }
    itemView->initialize((MenuItemImpl*)getItem(position), 0);
    return convertView;
}

void MenuAdapter::findExpandedIndex() {
    MenuItemImpl* expandedItem = mAdapterMenu->getExpandedItem();
    if (expandedItem != nullptr) {
        auto items = mAdapterMenu->getNonActionItems();
        const int count = items.size();
        for (int i = 0; i < count; i++) {
            MenuItemImpl* item = items.at(i);
            if (item == expandedItem) {
                mExpandedIndex = i;
                return;
            }
        }
    }
    mExpandedIndex = -1;
}

void MenuAdapter::notifyDataSetChanged() {
    findExpandedIndex();
    BaseAdapter::notifyDataSetChanged();
}

}/*endof namespace*/
