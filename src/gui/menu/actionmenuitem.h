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
#ifndef __ACTION_MENUITEM_H__
#define __ACTION_MENUITEM_H__
#include <menu/menuitem.h>
namespace cdroid{
class Intent;
class Context;
class ActionMenuItem:public MenuItem {
private:
    static constexpr int NO_ICON = 0;
    static constexpr int CHECKABLE      = 0x00000001;
    static constexpr int CHECKED        = 0x00000002;
    static constexpr int EXCLUSIVE      = 0x00000004;
    static constexpr int HIDDEN         = 0x00000008;
    static constexpr int ENABLED        = 0x00000010;
private:
    int mId;
    int mGroup;
    int mCategoryOrder;
    int mOrdering;

    std::string mTitle;
    std::string mTitleCondensed;
    Intent* mIntent;
    int mShortcutNumericChar;
    int mShortcutNumericModifiers = KeyEvent::META_CTRL_ON;
    int mShortcutAlphabeticChar;
    int mShortcutAlphabeticModifiers = KeyEvent::META_CTRL_ON;
    int mFlags = ENABLED;
    int mIconTintMode;
    Drawable* mIconDrawable;
    const ColorStateList* mIconTintList = nullptr;
    bool mHasIconTint = false;
    bool mHasIconTintMode = false;

    Context* mContext;
    OnMenuItemClickListener mClickListener;

    std::string mIconResId;
    std::string mContentDescription;
    std::string mTooltipText;
private:
    void applyIconTint();
public:
    ActionMenuItem(Context* context, int group, int id, int categoryOrder, int ordering,const std::string& title);

    int getAlphabeticShortcut()const override;
    int getAlphabeticModifiers()const override;

    int getGroupId()const override;
    Drawable* getIcon() override;
    Intent* getIntent() override;
    int getItemId()const override;
    ContextMenuInfo* getMenuInfo() override;
    int getNumericShortcut() const override;

    int getNumericModifiers() const override;

    int getOrder()const override;

    SubMenu* getSubMenu() override;

    std::string getTitle() override;

    std::string getTitleCondensed() override;

    bool hasSubMenu()const override;

    bool isCheckable() const override;

    bool isChecked() const override;
    bool isEnabled() const override;
    bool isVisible()const override;
    MenuItem& setAlphabeticShortcut(int alphaChar) override;

    MenuItem& setAlphabeticShortcut(int alphachar, int alphaModifiers) override;
    MenuItem& setCheckable(bool checkable) override;
    ActionMenuItem& setExclusiveCheckable(bool exclusive);
    MenuItem& setChecked(bool checked);

    MenuItem& setEnabled(bool enabled) override;

    MenuItem& setIcon(Drawable* icon) override;

    MenuItem& setIcon(const std::string& iconRes) override;

    MenuItem& setIconTintList(const ColorStateList* iconTintList) override;
    const ColorStateList* getIconTintList() override;
    MenuItem& setIconTintMode(int iconTintMode) override;
    int getIconTintMode()const override;

    MenuItem& setIntent(Intent* intent) override;

    MenuItem& setNumericShortcut(int numericChar) override;
    MenuItem& setNumericShortcut(int numericChar, int numericModifiers) override;

    MenuItem& setOnMenuItemClickListener(const OnMenuItemClickListener& menuItemClickListener) override;

    MenuItem& setShortcut(int numericChar, int alphaChar) override;
    MenuItem& setShortcut(int numericChar, int alphaChar, int numericModifiers, int alphaModifiers) override;

    MenuItem& setTitle(const std::string& title) override;

    MenuItem& setTitleCondensed(const std::string& title) override;

    MenuItem& setVisible(bool visible) override;

    bool invoke();

    void setShowAsAction(int show) override;

    MenuItem& setActionView(View* actionView) override;
    View* getActionView() override;

    MenuItem& setActionView(const std::string& resId) override;
    ActionProvider* getActionProvider() override;
    MenuItem& setActionProvider(ActionProvider* actionProvider) override;

    MenuItem& setShowAsActionFlags(int actionEnum) override;
    bool expandActionView() override;

    bool collapseActionView() override;
    bool isActionViewExpanded()const override;

    MenuItem& setOnActionExpandListener(const OnActionExpandListener& listener) override;

    MenuItem& setContentDescription(const std::string& contentDescription) override;
    std::string getContentDescription() override;

    MenuItem& setTooltipText(const std::string& tooltipText) override;
    std::string getTooltipText() override;
};
}/*endof namespace*/
#endif/*__ACTION_MENUITEM_H__*/

