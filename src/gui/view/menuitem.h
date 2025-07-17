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
namespace cdroid{
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
        bool onMenuItemActionExpand(MenuItem& item);
        bool onMenuItemActionCollapse(MenuItem& item);
    };
    virtual ~MenuItem()=default;
    virtual int getItemId()=0;

    virtual int getGroupId()=0;

    virtual int getOrder()=0;

    virtual MenuItem& setTitle(const std::string& title)=0;

    virtual std::string getTitle()=0;

    virtual MenuItem& setTitleCondensed(const std::string& title)=0;

    virtual std::string& getTitleCondensed();

    virtual MenuItem& setIcon(Drawable* icon);

    virtual MenuItem& setIcon(int iconRes);

    virtual Drawable* getIcon();

    virtual MenuItem& setIconTintList(ColorStateList* tint) { return *this; }

    virtual ColorStateList* getIconTintList() { return nullptr; }

    virtual MenuItem& setIconTintMode(int tintMode) { return *this; }

    int getIconTintMode() { return 0; }

    virtual MenuItem& setIntent(Intent* intent);

    Intent* getIntent();

    virtual MenuItem& setShortcut(char numericChar, char alphaChar);

    virtual MenuItem& setShortcut(char numericChar, char alphaChar, int numericModifiers, int alphaModifiers) {
        if ((alphaModifiers & Menu::SUPPORTED_MODIFIERS_MASK) == KeyEvent::META_CTRL_ON
                && (numericModifiers & Menu::SUPPORTED_MODIFIERS_MASK) == KeyEvent::META_CTRL_ON) {
            return setShortcut(numericChar, alphaChar);
        } else {
            return *this;
        }
    }

    virtual MenuItem& setNumericShortcut(char numericChar);

    virtual MenuItem& setNumericShortcut(char numericChar, int numericModifiers) {
        if ((numericModifiers & Menu::SUPPORTED_MODIFIERS_MASK) == KeyEvent::META_CTRL_ON) {
            return setNumericShortcut(numericChar);
        } else {
            return *this;
        }
    }

    virtual char getNumericShortcut();

    virtual int getNumericModifiers() {
        return KeyEvent::META_CTRL_ON;
    }

    virtual MenuItem& setAlphabeticShortcut(char alphaChar);

    virtual MenuItem& setAlphabeticShortcut(char alphaChar, int alphaModifiers) {
        if ((alphaModifiers & Menu::SUPPORTED_MODIFIERS_MASK) == KeyEvent::META_CTRL_ON) {
            return setAlphabeticShortcut(alphaChar);
        } else {
            return *this;
        }
    }

    virtual char getAlphabeticShortcut();

    virtual int getAlphabeticModifiers() {
        return KeyEvent::META_CTRL_ON;
    }

    virtual MenuItem& setCheckable(bool checkable)=0;

    virtual bool isCheckable()=0;

    virtual MenuItem& setChecked(bool checked)=0;

    virtual bool isChecked()=0;

    virtual MenuItem& setVisible(bool visible)=0;

    virtual bool isVisible()=0;

    virtual MenuItem& setEnabled(bool enabled)=0;

    virtual bool isEnabled()=0;

    virtual bool hasSubMenu()=0;
                                               
    virtual SubMenu* getSubMenu()=0;

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

    virtual MenuItem& setContentDescription(const std::string& contentDescription) {
        return *this;
    }

    virtual std::string getContentDescription() {
        return std::string();
    }

    virtual MenuItem& setTooltipText(const std::string& tooltipText) {
        return *this;
    }

    virtual std::string getTooltipText() {
        return std::string();
    }

    virtual bool requiresActionButton() {
        return false;
    }

    virtual bool requiresOverflow() {
        return true;
    }
};
}/*endof namespace*/
#endif/*__MENU_ITEM_H__*/

