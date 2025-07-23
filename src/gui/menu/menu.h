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
#ifndef __MENU_H__
#define __MENU_H__
#include <string>
#include <vector>
#include <functional>
#include <core/callbackbase.h>
#include <view/keyevent.h>
namespace cdroid{
class View;
class SubMenu;
class MenuItem;
class Drawable;
#ifndef DECLARE_UIEVENT
#define DECLARE_UIEVENT(type, name, ...) using name = std::function<type(__VA_ARGS__)>
#endif
class Menu{
public:
    /**
     * This is the part of an order integer that the user can provide.
     * @hide
     */
    static constexpr int USER_MASK = 0x0000ffff;
    /**
     * Bit shift of the user portion of the order integer.
     * @hide
     */
    static constexpr int USER_SHIFT = 0;

    /**
     * This is the part of an order integer that supplies the category of the
     * item.
     * @hide
     */
    static constexpr int CATEGORY_MASK = 0xffff0000;
    /**
     * Bit shift of the category portion of the order integer.
     * @hide
     */
    static constexpr int CATEGORY_SHIFT = 16;

    /**
     * A mask of all supported modifiers for MenuItem's keyboard shortcuts
     */
    static constexpr int SUPPORTED_MODIFIERS_MASK = KeyEvent::META_META_ON | KeyEvent::META_CTRL_ON
            | KeyEvent::META_ALT_ON | KeyEvent::META_SHIFT_ON | KeyEvent::META_SYM_ON | KeyEvent::META_FUNCTION_ON;

    /**
     * Value to use for group and item identifier integers when you don't care
     * about them.
     */
    static constexpr int NONE = 0;

    /**
     * First value for group and item identifier integers.
     */
    static constexpr int FIRST = 1;

    // Implementation note: Keep these CATEGORY_* in sync with the category enum
    // in attrs.xml

    /**
     * Category code for the order integer for items/groups that are part of a
     * container -- or/add this with your base value.
     */
    static constexpr int CATEGORY_CONTAINER = 0x00010000;

    /**
     * Category code for the order integer for items/groups that are provided by
     * the system -- or/add this with your base value.
     */
    static constexpr int CATEGORY_SYSTEM = 0x00020000;

    /**
     * Category code for the order integer for items/groups that are
     * user-supplied secondary (infrequently used) options -- or/add this with
     * your base value.
     */
    static constexpr int CATEGORY_SECONDARY = 0x00030000;

    /**
     * Category code for the order integer for items/groups that are
     * alternative actions on the data that is currently displayed -- or/add
     * this with your base value.
     */
    static constexpr int CATEGORY_ALTERNATIVE = 0x00040000;

    /**
     * Flag for {@link #addIntentOptions}: if set, do not automatically remove
     * any existing menu items in the same group.
     */
    static constexpr int FLAG_APPEND_TO_GROUP = 0x0001;

    /**
     * Flag for {@link #performShortcut}: if set, do not close the menu after
     * executing the shortcut.
     */
    static constexpr int FLAG_PERFORM_NO_CLOSE = 0x0001;

    /**
     * Flag for {@link #performShortcut(int, KeyEvent, int)}: if set, always
     * close the menu after executing the shortcut. Closing the menu also resets
     * the prepared state.
     */
    static constexpr int FLAG_ALWAYS_PERFORM_CLOSE = 0x0002;
public:
    virtual ~Menu()=default;
    virtual MenuItem* add(const std::string&title)=0;
    virtual MenuItem* add(int groupId, int itemId, int order,const std::string&title)=0;
    virtual SubMenu* addSubMenu(const std::string&title)=0;
    virtual SubMenu* addSubMenu(int groupId,int itemId,int order,const std::string& title)=0;
    virtual void close()=0;
    virtual void removeItem(int id)=0;
    virtual void removeGroup(int groupId)=0;
    virtual void clear()=0;
    virtual void setGroupCheckable(int group, bool checkable, bool exclusive)=0;
    virtual void setGroupVisible(int group, bool visible)=0;
    virtual void setOptionalIconsVisible(bool visible){};
    virtual void setGroupEnabled(int group, bool enabled)=0;
    virtual bool hasVisibleItems()const=0;
    virtual MenuItem*findItem(int id)const=0;
    virtual int size()const=0;
    virtual MenuItem* getItem(int index)=0;
    virtual bool performShortcut(int keyCode, KeyEvent& event, int flags)=0;
    virtual bool isShortcutKey(int keyCode,const KeyEvent& event)=0;
    virtual bool performIdentifierAction(int id, int flags)=0;
    virtual void setQwertyMode(bool isQwerty)=0;
    virtual void setGroupDividerEnabled(bool groupDividerEnabled)=0;
};
}/*endof namespace*/
#endif
