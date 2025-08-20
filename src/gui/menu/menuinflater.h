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
#ifndef __MENU_INFLATER_H__
#define __MENU_INFLATER_H__
#include <menu/menu.h>
namespace cdroid{ 
class Context;
class XmlPullParser;
class AttributeSet;
class ColorStateList;
class ActionProvider;
class MenuInflater {
private:
    class MenuState;
    static constexpr int NO_ID = 0;
    //static final Class<?>[] ACTION_VIEW_CONSTRUCTOR_SIGNATURE = new Class[] {Context.class};
    //static final Class<?>[] ACTION_PROVIDER_CONSTRUCTOR_SIGNATURE = ACTION_VIEW_CONSTRUCTOR_SIGNATURE;
    //Object[] mActionViewConstructorArguments;
    //Object[] mActionProviderConstructorArguments;

    Context* mContext;
    void* mRealOwner;
private:
    void parseMenu(XmlPullParser& parser,const AttributeSet& attrs, Menu* menu);
    void registerMenu(MenuItem* item,const AttributeSet& set);
    void registerMenu(SubMenu* subMenu,const AttributeSet& set);
    Object* getRealOwner();
    Object* findRealOwner(Object* owner);
public:
    MenuInflater(Context* context);
    /**
     * Constructs a menu inflater.
     *
     * @see Activity#getMenuInflater()
     * @hide
     */
    MenuInflater(Context* context, Object realOwner);

    /**
     * Inflate a menu hierarchy from the specified XML resource. Throws
     * {@link InflateException} if there is an error.
     *
     * @param menuRes Resource ID for an XML layout resource to load (e.g.,
     *            <code>R.menu.main_activity</code>)
     * @param menu The Menu to inflate into. The items and submenus will be
     *            added to this Menu.
     */
    void inflate(const std::string&menuRes, Menu* menu);

    Context* getContext();
};/*endof MenuInflater*/

/**
 * State for the current menu.
 * <p>
 * Groups can not be nested unless there is another menu (which will have
 * its state class).
 */
class MenuInflater::MenuState {
private:
    friend MenuInflater;
    Menu* menu;
    Context*mContext;
    int groupId;
    int groupCategory;
    int groupOrder;
    int groupCheckable;
    bool groupVisible;
    bool groupEnabled;
    bool itemAdded;
    int itemId;
    int itemCategoryOrder;
    std::string itemTitle;
    std::string itemTitleCondensed;
    std::string itemIconResId;
    ColorStateList* itemIconTintList;
    int mItemIconBlendMode;
    int itemAlphabeticModifiers;
    int itemAlphabeticShortcut;
    int itemNumericShortcut;
    int itemNumericModifiers;
    int itemCheckable;
    bool itemChecked;
    bool itemVisible;
    bool itemEnabled;

    int itemShowAsAction;
    std::string itemActionViewLayout;
    std::string itemActionViewClassName;
    std::string itemActionProviderClassName;
    std::string itemListenerMethodName;

    ActionProvider* itemActionProvider;

    std::string itemContentDescription;
    std::string itemTooltipText;

    static constexpr int defaultGroupId = NO_ID;
    static constexpr int defaultItemId = NO_ID;
    static constexpr int defaultItemCategory = 0;
    static constexpr int defaultItemOrder = 0;
    static constexpr int defaultItemCheckable = 0;
    static constexpr bool defaultItemChecked = false;
    static constexpr bool defaultItemVisible = true;
    static constexpr bool defaultItemEnabled = true;
private:
    char getShortcut(const std::string& shortcutString);
    void setItem(MenuItem* item);
public:
    MenuState(Menu* menu,Context*ctx);
    virtual ~MenuState();
    void resetGroup();

    void readGroup(const AttributeSet& attrs);
    void readItem(const AttributeSet& attrs);

    MenuItem* addItem();
    SubMenu* addSubMenuItem();

    bool hasAddedItem() const;
};
}/*endof namespace*/
#endif 
