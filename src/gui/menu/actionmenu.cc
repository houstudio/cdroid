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
#include <drawables/drawable.h>
#include <menu/actionmenu.h>
#include <menu/actionmenuitem.h>
namespace cdroid{

ActionMenu::ActionMenu(Context* context) {
    mContext = context;
}

Context* ActionMenu::getContext() const{
    return mContext;
}

MenuItem* ActionMenu::add(const std::string& title) {
    return add(0, 0, 0, title);
}

/*MenuItem* add(int titleRes) {
    return add(0, 0, 0, titleRes);
}

MenuItem* ActionMenu::add(int groupId, int itemId, int order, int titleRes) {
    return add(groupId, itemId, order, mContext.getResources().getString(titleRes));
}*/

MenuItem* ActionMenu::add(int groupId, int itemId, int order, const std::string& title) {
    ActionMenuItem* item = new ActionMenuItem(getContext(),groupId, itemId, 0, order, title);
    mItems.insert(mItems.begin()+order, item);
    return item;
}

/*int ActionMenu::addIntentOptions(int groupId, int itemId, int order,
        ComponentName caller, Intent[] specifics, Intent intent, int flags,
        MenuItem[] outSpecificItems) {
    PackageManager pm = mContext.getPackageManager();
    final List<ResolveInfo> lri = pm.queryIntentActivityOptions(caller, specifics, intent, 0);
    final int N = lri != null ? lri.size() : 0;

    if ((flags & FLAG_APPEND_TO_GROUP) == 0) {
        removeGroup(groupId);
    }

    for (int i=0; i<N; i++) {
        final ResolveInfo ri = lri.get(i);
        Intent rintent = new Intent(
            ri.specificIndex < 0 ? intent : specifics[ri.specificIndex]);
        rintent.setComponent(new ComponentName(
                ri.activityInfo.applicationInfo.packageName,
                ri.activityInfo.name));
        final MenuItem item = add(groupId, itemId, order, ri.loadLabel(pm))
                .setIcon(ri.loadIcon(pm))
                .setIntent(rintent);
        if (outSpecificItems != null && ri.specificIndex >= 0) {
            outSpecificItems[ri.specificIndex] = item;
        }
    }

    return N;
}*/

SubMenu* ActionMenu::addSubMenu(const std::string& title) {
    // TODO Implement submenus
    return nullptr;
}

/*SubMenu* ActionMenu::addSubMenu(int titleRes) {
    // TODO Implement submenus
    return nullptr;
}*/

SubMenu* ActionMenu::addSubMenu(int groupId, int itemId, int order,const std::string& title) {
    // TODO Implement submenus
    return nullptr;
}

/*SubMenu* ActionMenu::addSubMenu(int groupId, int itemId, int order, int titleRes) {
    // TODO Implement submenus
    return nullptr;
}*/

void ActionMenu::clear() {
    mItems.clear();
}

void ActionMenu::close() {
}

int ActionMenu::findItemIndex(int id) {
    const int itemCount = mItems.size();
    for (int i = 0; i < itemCount; i++) {
        if (mItems.at(i)->getItemId() == id) {
            return i;
        }
    }
    return -1;
}

MenuItem* ActionMenu::findItem(int id) {
    return mItems.at(findItemIndex(id));
}

MenuItem* ActionMenu::getItem(int index) {
    return mItems.at(index);
}

bool ActionMenu::hasVisibleItems() {
    const int itemCount = mItems.size();
    for (int i = 0; i < itemCount; i++) {
        if (mItems.at(i)->isVisible()) {
            return true;
        }
    }

    return false;
}

ActionMenuItem* ActionMenu::findItemWithShortcut(int keyCode, KeyEvent& event) {
    // TODO Make this smarter.
    const bool qwerty = mIsQwerty;
    const int itemCount = mItems.size();
    const int modifierState = event.getModifiers();
    for (int i = 0; i < itemCount; i++) {
        ActionMenuItem* item = mItems.at(i);
        const int shortcut = qwerty ? item->getAlphabeticShortcut() : item->getNumericShortcut();
        const int shortcutModifiers = qwerty ? item->getAlphabeticModifiers() : item->getNumericModifiers();
        const bool is_modifiers_exact_match = (modifierState & SUPPORTED_MODIFIERS_MASK)
                == (shortcutModifiers & SUPPORTED_MODIFIERS_MASK);
        if ((keyCode == shortcut) && is_modifiers_exact_match) {
            return item;
        }
    }
    return nullptr;
}

bool ActionMenu::isShortcutKey(int keyCode, KeyEvent& event) {
    return findItemWithShortcut(keyCode, event) != nullptr;
}

bool ActionMenu::performIdentifierAction(int id, int flags) {
    const int index = findItemIndex(id);
    if (index < 0) {
        return false;
    }
    return mItems.at(index)->invoke();
}

bool ActionMenu::performShortcut(int keyCode, KeyEvent& event, int flags) {
    ActionMenuItem* item = findItemWithShortcut(keyCode, event);
    if (item == nullptr) {
        return false;
    }
    return item->invoke();
}

void ActionMenu::removeGroup(int groupId) {
    int itemCount = mItems.size();
    int i = 0;
    while (i < itemCount) {
        if (mItems.at(i)->getGroupId() == groupId) {
            mItems.erase(mItems.begin()+i);
            itemCount--;
        } else {
            i++;
        }
    }
}

void ActionMenu::removeItem(int id) {
    mItems.erase(mItems.begin()+findItemIndex(id));
}

void ActionMenu::setGroupCheckable(int group, bool checkable,bool exclusive) {
    const int itemCount = mItems.size();

    for (int i = 0; i < itemCount; i++) {
        ActionMenuItem* item = mItems.at(i);
        if (item->getGroupId() == group) {
            item->setCheckable(checkable);
            item->setExclusiveCheckable(exclusive);
        }
    }
}

void ActionMenu::setGroupEnabled(int group, bool enabled) {
    const int itemCount = mItems.size();

    for (int i = 0; i < itemCount; i++) {
        ActionMenuItem* item = mItems.at(i);
        if (item->getGroupId() == group) {
            item->setEnabled(enabled);
        }
    }
}

void ActionMenu::setGroupVisible(int group, bool visible) {
    const int itemCount = mItems.size();

    for (int i = 0; i < itemCount; i++) {
        ActionMenuItem* item = mItems.at(i);
        if (item->getGroupId() == group) {
            item->setVisible(visible);
        }
    }
}

void ActionMenu::setQwertyMode(bool isQwerty) {
    mIsQwerty = isQwerty;
}

int ActionMenu::size()const {
    return mItems.size();
}

}/*endof namespace*/
