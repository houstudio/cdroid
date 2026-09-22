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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  0210-1301  USA
*/
#ifndef __WEARABLE_DRAWER_ACTION_MENU_H__
#define __WEARABLE_DRAWER_ACTION_MENU_H__
#include <menu/menu.h>
#include <menu/menuitem.h>
#include <functional>
#include <string>
#include <vector>
namespace cdroid{

class Context;
class Drawable;
class Intent;
class ActionProvider;
class ContextMenuInfo;
class SubMenu;
class View;

/** A Menu for WearableActionDrawerView. The full Menu/MenuItem APIs are not implemented
 *  upstream: for Menu, add/clear/removeItem/findItem/size/getItem carry behavior and the rest
 *  throw or are inert. androidx.wear.widget.drawer.WearableActionDrawerMenu.java (lines 38-475;
 *  package-private upstream, consumed by WearableActionDrawerView). */
class WearableActionDrawerMenu: public Menu{
public:
    /** Package-private upstream (lines 217-226). WearableActionDrawerView is the only
        implementor and installs itself at construction time; the struct-of-callbacks shape
        follows MenuBuilder::Callback. */
    struct WearableActionDrawerMenuListener {
        std::function<void(int position)> menuItemChanged;
        std::function<void(int position)> menuItemAdded;
        std::function<void(int position)> menuItemRemoved;
        std::function<void()> menuChanged;
    };

    /** (lines 228-474) Setting/getting title and icon, getItemId and
        setOnMenuItemClickListener carry behavior; the rest throw or are inert like upstream. */
    class WearableActionDrawerMenuItem: public MenuItem{
    public:
        /** Private interface upstream (lines 470-473); the menu installs one callback that maps
            the changed item back to its index. Nullable (items accept a null listener). */
        using MenuItemChangedListener = std::function<void(WearableActionDrawerMenuItem& item)>;

        WearableActionDrawerMenuItem(Context* context, int id, const std::string& title,
                const MenuItemChangedListener& listener);
        ~WearableActionDrawerMenuItem() override;

        int getItemId() const override;

        MenuItem& setTitle(const std::string& title) override;

        std::string getTitle() override;

        MenuItem& setTitleCondensed(const std::string& title) override;

        std::string getTitleCondensed() override;

        MenuItem& setIcon(Drawable* icon) override;

        // AOSP setIcon(@DrawableRes int)
        MenuItem& setIcon(int iconRes) override;

        Drawable* getIcon() override;

        MenuItem& setOnMenuItemClickListener(const OnMenuItemClickListener& menuItemClickListener) override;

        int getGroupId() const override;

        int getOrder() const override;

        MenuItem& setIntent(Intent* intent) override; // upstream throws (line 316)

        Intent* getIntent() override;

        MenuItem& setShortcut(int numericChar, int alphaChar) override; // upstream throws (line 326)

        MenuItem& setCheckable(bool checkable) override;

        bool isCheckable() const override;

        MenuItem& setChecked(bool checked) override;

        bool isChecked() const override;

        MenuItem& setVisible(bool visible) override;

        bool isVisible() const override;

        MenuItem& setEnabled(bool enabled) override;

        bool isEnabled() const override;

        ContextMenuInfo* getMenuInfo() override;

        void setShowAsAction(int actionEnum) override;

        MenuItem& setShowAsActionFlags(int actionEnum) override;

        MenuItem& setActionView(View* view) override;

        MenuItem& setActionView(int resId) override;

        View* getActionView() override;

        MenuItem& setActionProvider(ActionProvider* actionProvider) override;

        ActionProvider* getActionProvider() override;

        bool expandActionView() override;

        bool collapseActionView() override;

        bool isActionViewExpanded() const override;

        MenuItem& setOnActionExpandListener(const OnActionExpandListener& listener) override;

        /** Invokes the item by calling the listener if set.
            @return true if the invocation was handled, false otherwise.
            (lines 460-468; package-private upstream, called by
            WearableActionDrawerView::onMenuItemClicked) */
        bool invoke();

    private:
        const int mId;

        Context* mContext;
        const MenuItemChangedListener mItemChangedListener;
        std::string mTitle;
        Drawable* mIconDrawable = nullptr; // owned; upstream holds a GC reference
        OnMenuItemClickListener mClickListener;
    };

    WearableActionDrawerMenu(Context* context, const WearableActionDrawerMenuListener& listener);
    ~WearableActionDrawerMenu() override;

    MenuItem* add(const std::string& title) override;

    MenuItem* add(int groupId, int itemId, int order, const std::string& title) override;

    SubMenu* addSubMenu(const std::string& title) override;

    SubMenu* addSubMenu(int groupId, int itemId, int order, const std::string& title) override;

    void close() override;

    void removeItem(int id) override;

    void removeGroup(int groupId) override;

    void clear() override;

    void setGroupCheckable(int group, bool checkable, bool exclusive) override;

    void setGroupVisible(int group, bool visible) override;

    void setOptionalIconsVisible(bool visible) override;

    void setGroupEnabled(int group, bool enabled) override;

    bool hasVisibleItems() const override;

    MenuItem* findItem(int id) const override;

    int size() const override;

    MenuItem* getItem(int index) override;

    bool performShortcut(int keyCode, KeyEvent& event, int flags) override;

    bool isShortcutKey(int keyCode, const KeyEvent& event) override;

    bool performIdentifierAction(int id, int flags) override;

    void setQwertyMode(bool isQwerty) override;

    void setGroupDividerEnabled(bool groupDividerEnabled) override;

private:
    Context* mContext;
    /** Upstream holds List<WearableActionDrawerMenuItem> (GC'd); CDROID owns and deletes the
        items, handing out borrowed pointers like MenuBuilder. */
    std::vector<WearableActionDrawerMenuItem*> mItems;
    const WearableActionDrawerMenuListener mListener;
    /** The anonymous MenuItemChangedListener (lines 45-55), reporting the item's index. */
    const WearableActionDrawerMenuItem::MenuItemChangedListener mItemChangedListener;

    int findItemIndex(int id) const;
};

}/*endof namespace*/
#endif/*__WEARABLE_DRAWER_ACTION_MENU_H__*/
