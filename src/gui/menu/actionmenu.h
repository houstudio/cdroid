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
#ifndef __ACTION_MENU_H__
#define __ACTION_MENU_H__
#include <menu/menu.h>
namespace cdroid{
class Context;
class ActionMenuItem;

class ActionMenu:public Menu {
private:
    Context* mContext;
    bool mIsQwerty;
    std::vector<ActionMenuItem*> mItems;
private:
    int findItemIndex(int id);
    ActionMenuItem* findItemWithShortcut(int keyCode, KeyEvent& event);
public:
    ActionMenu(Context* context);

    Context* getContext()const;

    MenuItem* add(const std::string& title)override;
    //MenuItem* add(int titleRes);
    //MenuItem* add(int groupId, int itemId, int order, int titleRes);
    MenuItem* add(int groupId, int itemId, int order, const std::string& title)override;

    /*int addIntentOptions(int groupId, int itemId, int order,ComponentName caller,
     * Intent[] specifics, Intent intent, int flags, MenuItem[] outSpecificItems);*/

    SubMenu* addSubMenu(const std::string& title);
    //SubMenu* addSubMenu(int titleRes)
    SubMenu* addSubMenu(int groupId, int itemId, int order,const std::string& title);
    //SubMenu* addSubMenu(int groupId, int itemId, int order, int titleRes);

    void clear();
    void close();

    MenuItem* findItem(int id);
    MenuItem* getItem(int index);

    bool hasVisibleItems();

    bool isShortcutKey(int keyCode, KeyEvent& event);

    bool performIdentifierAction(int id, int flags);
    bool performShortcut(int keyCode, KeyEvent& event, int flags);

    void removeGroup(int groupId);
    void removeItem(int id);

    void setGroupCheckable(int group, bool checkable,bool exclusive);
    void setGroupEnabled(int group, bool enabled);
    void setGroupVisible(int group, bool visible);

    void setQwertyMode(bool isQwerty);

    int size() const;
};
}/*endof namespace*/
#endif/*__ACTION_MENU_H__*/
