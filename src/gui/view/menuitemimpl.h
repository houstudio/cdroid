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
#ifndef __MENUITEM_IMPL_H__
#define __MENUITEM_IMPL_H__
#include <view/menuitem.h>
namespace cdroid{
class MenuBuilder;
class SubMenuBuilder;
class MenuItemImpl:public MenuItem {
private:
    static constexpr int SHOW_AS_ACTION_MASK = SHOW_AS_ACTION_NEVER |
            SHOW_AS_ACTION_IF_ROOM |
            SHOW_AS_ACTION_ALWAYS;
    static constexpr int CHECKABLE      = 0x00000001;
    static constexpr int CHECKED        = 0x00000002;
    static constexpr int EXCLUSIVE      = 0x00000004;
    static constexpr int HIDDEN         = 0x00000008;
    static constexpr int ENABLED        = 0x00000010;
    static constexpr int IS_ACTION      = 0x00000020;
    static constexpr int NO_ICON = 0;

    int mId;
    int mGroup;
    int mCategoryOrder;
    int mOrdering;
    CharSequence mTitle;
    CharSequence mTitleCondensed;
    Intent mIntent;
    char mShortcutNumericChar;
    int mShortcutNumericModifiers = KeyEvent::META_CTRL_ON;
    char mShortcutAlphabeticChar;
    int mShortcutAlphabeticModifiers = KeyEvent::META_CTRL_ON;

    Drawable* mIconDrawable;
    int mIconResId = NO_ICON;

    ColorStateList* mIconTintList = nullptr;
    PorterDuff.Mode mIconTintMode = nullptr;
    bool mHasIconTint = false;
    bool mHasIconTintMode = false;
    bool mNeedToApplyIconTint = false;

    /** The menu to which this item belongs */
    MenuBuilder* mMenu;
    /** If this item should launch a sub menu, this is the sub menu to launch */
    SubMenuBuilder* mSubMenu;

    Runnable mItemCallback;
    MenuItem::OnMenuItemClickListener mClickListener;

    int mFlags = ENABLED;

    int mShowAsAction = SHOW_AS_ACTION_NEVER;

    View* mActionView;
    ActionProvider* mActionProvider;
    OnActionExpandListener mOnActionExpandListener;
    bool mIsActionViewExpanded = false;

    ContextMenuInfo* mMenuInfo;

    std::string mContentDescription;
    std::string mTooltipText;

private:
    static void appendModifier(StringBuilder sb, int mask, int modifier, const std::string& label);
    Drawable* applyIconTintIfNecessary(Drawable* icon);
public:
    MenuItemImpl(MenuBuilder* menu, int group, int id, int categoryOrder, int ordering,
            const std::string& title, int showAsAction);

    bool invoke();

    bool isEnabled()override;
    MenuItem& setEnabled(bool enabled)override;

    int getGroupId() override;
    int getItemId() override;
    int getOrder() override;
    int getOrdering() override;

    Intent* getIntent()override;
    MenuItem& setIntent(Intent* intent);

    Runnable getCallback();
    MenuItem& setCallback(Runnable callback);

    char getAlphabeticShortcut() override;
    int getAlphabeticModifiers() override;

    MenuItem& setAlphabeticShortcut(char alphaChar) override;
    MenuItem& setAlphabeticShortcut(char alphaChar, int alphaModifiers)override;

    char getNumericShortcut();
    int getNumericModifiers();

    MenuItem& setNumericShortcut(char numericChar) override;
    MenuItem& setNumericShortcut(char numericChar, int numericModifiers) override;
    MenuItem& setShortcut(char numericChar, char alphaChar) override;
    MenuItem& setShortcut(char numericChar, char alphaChar, int numericModifiers,int alphaModifiers) override;

    char getShortcut();
    std::string getShortcutLabel();
    bool shouldShowShortcut();

    SubMenu* getSubMenu()override;
    bool hasSubMenu()override;
    void setSubMenu(SubMenuBuilder* subMenu)override;

    std::string getTitle();
    std::string getTitleForItemView(MenuView::ItemView* itemView);
    MenuItem& setTitle(const std::string& title);

    std::string getTitleCondensed();
    MenuItem& setTitleCondensed(const std::string& title);

    Drawable* getIcon();
    MenuItem& setIcon(Drawable* icon);
    MenuItem& setIcon(int iconResId);

    MenuItem& setIconTintList(const ColorStateList* iconTintList);
    ColorStateList* getIconTintList();

    MenuItem& setIconTintMode(PorterDuff.Mode iconTintMode);
    PorterDuff.Mode getIconTintMode();

    bool isCheckable() const override;
    MenuItem& setCheckable(bool checkable)override;

    void setExclusiveCheckable(bool exclusive) override;
    bool isExclusiveCheckable() override;

    bool isChecked() const override;
    MenuItem& setChecked(bool checked)override;
    void setCheckedInt(bool checked)override;

    bool isVisible()override;
    bool setVisibleInt(bool shown);
    MenuItem& setVisible(bool shown)override;

    MenuItem& setOnMenuItemClickListener(const MenuItem::OnMenuItemClickListener& clickListener);

    void setMenuInfo(ContextMenuInfo* menuInfo);
    ContextMenuInfo* getMenuInfo();

    void actionFormatChanged();

    bool shouldShowIcon();

    bool isActionButton();

    bool requestsActionButton();
    bool requiresActionButton();

    bool requiresOverflow();

    void setIsActionButton(bool isActionButton);

    bool showsTextAsAction();

    void setShowAsAction(int actionEnum);

    MenuItem& setActionView(View* view);
    MenuItem& setActionView(int resId);

    View* getActionView();

    ActionProvider* getActionProvider();
    MenuItem& setActionProvider(ActionProvider* actionProvider);

    MenuItem& setShowAsActionFlags(int actionEnum);

    bool expandActionView();
    bool collapseActionView();

    MenuItem& setOnActionExpandListener(OnActionExpandListener listener);

    bool hasCollapsibleActionView();
    void setActionViewExpanded(bool isExpanded);

    bool isActionViewExpanded() {
    urn mIsActionViewExpanded;
    
    MenuItem& setContentDescription(const std::string&contentDescription);
    std::string getContentDescription();

    MenuItem& setTooltipText(const std::string& tooltipText);
    std::string getTooltipText();
};
}/*endof namespace*/
#endif/*__MENUITEM_IMPL_H__*/
