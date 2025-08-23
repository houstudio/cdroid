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
#ifndef __MENU_ITEM_H__
#define __MENU_ITEM_H__
#include <string>
#include <functional>
namespace cdroid{
class Drawable;
class View;
class SubMenu;
class Intent;
class ColorStateList;
class ActionProvider;
class ContextMenuInfo;
class MenuItem {
public:
    /** Never show this item as a button in an Action Bar. */
    static constexpr int SHOW_AS_ACTION_NEVER = 0;
    static constexpr int SHOW_AS_ACTION_IF_ROOM = 1;
    static constexpr int SHOW_AS_ACTION_ALWAYS = 2;
    static constexpr int SHOW_AS_ACTION_WITH_TEXT = 4;
    static constexpr int SHOW_AS_ACTION_COLLAPSE_ACTION_VIEW = 8;

    using  OnMenuItemClickListener= std::function<bool(MenuItem&)>;

    struct OnActionExpandListener {
        std::function<bool(MenuItem&)> onMenuItemActionExpand;
        std::function<bool(MenuItem&)> onMenuItemActionCollapse;
    };
    virtual ~MenuItem()=default;
    virtual int getItemId()const=0;

    virtual int getGroupId()const=0;

    virtual int getOrder()const=0;

    virtual MenuItem& setTitle(const std::string& title)=0;

    virtual std::string getTitle()=0;

    virtual MenuItem& setTitleCondensed(const std::string& title)=0;

    virtual std::string getTitleCondensed()=0;

    virtual MenuItem& setIcon(Drawable* icon)=0;

    virtual MenuItem& setIcon(const std::string& iconRes)=0;

    virtual Drawable* getIcon()=0;

    virtual MenuItem& setIconTintList(const ColorStateList* tint);

    virtual const ColorStateList* getIconTintList();

    virtual MenuItem& setIconTintMode(int tintMode);

    virtual int getIconTintMode();

    virtual MenuItem& setIntent(Intent* intent);

    virtual Intent* getIntent()=0;

    virtual MenuItem& setShortcut(int numericChar, int alphaChar);

    virtual MenuItem& setShortcut(int numericChar, int alphaChar, int numericModifiers, int alphaModifiers);

    virtual MenuItem& setNumericShortcut(int numericChar);

    virtual MenuItem& setNumericShortcut(int numericChar, int numericModifiers);

    virtual int getNumericShortcut()const;

    virtual int getNumericModifiers()const;

    virtual MenuItem& setAlphabeticShortcut(int alphaChar);

    virtual MenuItem& setAlphabeticShortcut(int alphaChar, int alphaModifiers);

    virtual int getAlphabeticShortcut()const;

    virtual int getAlphabeticModifiers()const;

    virtual MenuItem& setCheckable(bool checkable)=0;

    virtual bool isCheckable()const=0;

    virtual MenuItem& setChecked(bool checked)=0;

    virtual bool isChecked()const=0;

    virtual MenuItem& setVisible(bool visible)=0;

    virtual bool isVisible()const=0;

    virtual MenuItem& setEnabled(bool enabled)=0;

    virtual bool isEnabled()const =0;

    virtual bool hasSubMenu()const;
                                               
    virtual SubMenu* getSubMenu();

    virtual MenuItem& setOnMenuItemClickListener(const OnMenuItemClickListener& menuItemClickListener)=0;

    virtual ContextMenuInfo* getMenuInfo()=0;

    virtual void setShowAsAction(int actionEnum)=0;

    virtual MenuItem& setShowAsActionFlags(int actionEnum)=0;

    virtual MenuItem& setActionView(View* view)=0;

    virtual MenuItem& setActionView(const std::string& resId)=0;

    virtual View* getActionView()=0;

    virtual MenuItem& setActionProvider(ActionProvider* actionProvider)=0;

    virtual ActionProvider* getActionProvider()=0;

    virtual bool expandActionView()=0;

    virtual bool collapseActionView()=0;

    virtual bool isActionViewExpanded()=0;

    virtual MenuItem& setOnActionExpandListener(OnActionExpandListener listener);

    virtual MenuItem& setContentDescription(const std::string& contentDescription);

    virtual std::string getContentDescription();

    virtual MenuItem& setTooltipText(const std::string& tooltipText);

    virtual std::string getTooltipText();

    virtual bool requiresActionButton();

    virtual bool requiresOverflow();
};
}/*endof namespace*/
#endif/*__MENU_ITEM_H__*/

