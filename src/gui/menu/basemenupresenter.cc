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
#include <menu/menubuilder.h>
#include <menu/basemenupresenter.h>
namespace cdroid{ 
BaseMenuPresenter::BaseMenuPresenter(Context* context,const std::string& menuLayoutRes,const std::string& itemLayoutRes){
    mSystemContext = context;
    mMenu = nullptr;
    mInflater = nullptr;
    mMenuView = nullptr;
    mSystemInflater = nullptr;
    mSystemInflater = LayoutInflater::from(context);
    mMenuLayoutRes = menuLayoutRes;
    mItemLayoutRes = itemLayoutRes;
}

void BaseMenuPresenter::initForMenu(Context* context,MenuBuilder* menu) {
    mContext = context;
    mInflater = LayoutInflater::from(mContext);
    mMenu = menu;
}

MenuView* BaseMenuPresenter::getMenuView(ViewGroup* root) {
    if (mMenuView == nullptr) {
        mMenuView = (MenuView*) mSystemInflater->inflate(mMenuLayoutRes, root, false);
        mMenuView->initialize(mMenu);
        updateMenuView(true);
    }

    return mMenuView;
}

void BaseMenuPresenter::updateMenuView(bool cleared) {
    ViewGroup* parent = (ViewGroup*) mMenuView;
    if (parent == nullptr) return;

    int childIndex = 0;
    if (mMenu != nullptr) {
        mMenu->flagActionItems();
        std::vector<MenuItemImpl*> visibleItems = mMenu->getVisibleItems();
        const int itemCount = visibleItems.size();
        for (int i = 0; i < itemCount; i++) {
            MenuItemImpl* item = visibleItems.at(i);
            if (shouldIncludeItem(childIndex, item)) {
                View* convertView = parent->getChildAt(childIndex);
                MenuItemImpl* oldItem = dynamic_cast<MenuView::ItemView*>(convertView) ?
                        ((MenuView::ItemView*) convertView)->getItemData() : nullptr;
                View* itemView = getItemView(item, convertView, parent);
                if (item != oldItem) {
                    // Don't let old states linger with new data.
                    itemView->setPressed(false);
                    itemView->jumpDrawablesToCurrentState();
                }
                if (itemView != convertView) {
                    addItemView(itemView, childIndex);
                }
                childIndex++;
            }
        }
    }

    // Remove leftover views.
    while (childIndex < parent->getChildCount()) {
        if (!filterLeftoverView(parent, childIndex)) {
            childIndex++;
        }
    }
}

void BaseMenuPresenter::addItemView(View* itemView, int childIndex) {
    ViewGroup* currentParent = (ViewGroup*) itemView->getParent();
    if (currentParent != nullptr) {
        currentParent->removeView(itemView);
    }
    ((ViewGroup*) mMenuView)->addView(itemView, childIndex);
}

bool BaseMenuPresenter::filterLeftoverView(ViewGroup* parent, int childIndex) {
    parent->removeViewAt(childIndex);
    return true;
}

void BaseMenuPresenter::setCallback(const Callback& cb) {
    mCallback = cb;
}

MenuPresenter::Callback BaseMenuPresenter::getCallback() {
    return mCallback;
}

MenuView::ItemView* BaseMenuPresenter::createItemView(ViewGroup* parent) {
    return (MenuView::ItemView*) mSystemInflater->inflate(mItemLayoutRes, parent, false);
}

View* BaseMenuPresenter::getItemView(MenuItemImpl* item, View* convertView, ViewGroup* parent) {
    MenuView::ItemView* itemView;
    if (dynamic_cast<MenuView::ItemView*>(convertView)) {
        itemView = (MenuView::ItemView*) convertView;
    } else {
        itemView = createItemView(parent);
    }
    bindItemView(item, itemView);
    return (View*) itemView;
}

bool BaseMenuPresenter::shouldIncludeItem(int childIndex, MenuItemImpl* item) {
    return true;
}

void BaseMenuPresenter::onCloseMenu(MenuBuilder* menu, bool allMenusAreClosing) {
    if (mCallback.onCloseMenu != nullptr) {
        mCallback.onCloseMenu(*menu, allMenusAreClosing);
    }
}

bool BaseMenuPresenter::onSubMenuSelected(SubMenuBuilder* menu) {
    if (mCallback.onOpenSubMenu != nullptr) {
        return mCallback.onOpenSubMenu((MenuBuilder&)menu);
    }
    return false;
}

bool BaseMenuPresenter::flagActionItems() {
    return false;
}

bool BaseMenuPresenter::expandItemActionView(MenuBuilder& menu, MenuItemImpl& item) {
    return false;
}

bool BaseMenuPresenter::collapseItemActionView(MenuBuilder& menu, MenuItemImpl& item) {
    return false;
}

int BaseMenuPresenter::getId() const{
    return mId;
}

void BaseMenuPresenter::setId(int id) {
    mId = id;
}
}/*endof namespace*/
