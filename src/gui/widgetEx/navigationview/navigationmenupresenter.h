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
#ifndef __NAVIGATION_MENU_PRESENTER_H__
#define __NAVIGATION_MENU_PRESENTER_H__
// Port of com.google.android.material.internal.NavigationMenuPresenter (the
// MenuPresenter driving NavigationView). CDROID substrate notes: material
// inflates its rows from design_navigation_*.xml through a RecyclerView
// adapter — here the same four view types are built in code with the same
// theme attrs/dimens; window insets have no CDROID counterpart (the
// padding-from-inset paths are inert with a 0 default inset).
#include <menu/menuitemimpl.h>
#include <menu/menupresenter.h>
#include <menu/menubuilder.h>
#include <core/bundle.h>
#include <core/parcelable.h>
#include <widget/linearlayout.h>
#include <widgetEx/navigationview/navigationmenuview.h>

namespace cdroid{

class NavigationMenuItemView;

// AOSP's presenter state is a Bundle (a Parcelable on Android); CDROID's
// Bundle does not implement Parcelable, so this wrapper carries it through
// the MenuPresenter interface.
class NavigationMenuPresenterState:public Parcelable{
public:
    Bundle menuState;
};

class NavigationMenuPresenter:public MenuPresenter{
public:
    static constexpr int NO_TEXT_APPEARANCE_SET = 0;
private:
    static constexpr const char* STATE_HIERARCHY = "android:menu:list";
    static constexpr const char* STATE_ADAPTER = "android:menu:adapter";
    static constexpr const char* STATE_HEADER = "android:menu:header";
private:
    NavigationMenuView* mMenuView = nullptr;
    LinearLayout* mHeaderLayout = nullptr;
    MenuPresenter::Callback mCallback;
    MenuBuilder* mMenu = nullptr;
    int mId = 0;

    class NavigationMenuAdapter;
    NavigationMenuAdapter* mAdapter = nullptr;
    LayoutInflater* mLayoutInflater = nullptr;

    std::function<void(View&)> mOnClickListener;
private:
    void updateAllTextMenuItems();
    void updateAllSubHeaderMenuItems();
    void updateAllDividerMenuItems();
    void updateTopPadding();
    bool hasHeader()const;
public: // used by the NORMAL view holder
    /** The material field onClickListener, handed to the NORMAL item views. */
    const std::function<void(View&)>& itemClickListener();
public: // material keeps these package-private for NavigationView
    int mSubheaderTextAppearance = NO_TEXT_APPEARANCE_SET;
    RefPtr<ColorStateList> mSubheaderColor;
    int mTextAppearance = NO_TEXT_APPEARANCE_SET;
    bool mTextAppearanceActiveBoldEnabled = true;
    RefPtr<ColorStateList> mTextColor;
    RefPtr<ColorStateList> mIconTintList;
    Drawable* mItemBackground = nullptr;
    Drawable* mItemForeground = nullptr;
    int mItemHorizontalPadding = 0;
    int mItemVerticalPadding = 0;
    int mItemIconPadding = 0;
    int mItemIconSize = 0;
    int mDividerInsetStart = 0;
    int mDividerInsetEnd = 0;
    int mSubheaderInsetStart = 0;
    int mSubheaderInsetEnd = 0;
    bool mHasCustomItemIconSize = false;
    bool mIsBehindStatusBar = true;
private:
    int mItemMaxLines = 0;
    /** Padding to be inserted at the top of the list to avoid the first menu
        item from being placed underneath the status bar. */
    int mPaddingTopDefault = 0;
    /** Padding for separators between items */
    int mPaddingSeparator = 0;
    int mOverScrollMode = -1;
public:
    ~NavigationMenuPresenter()override;

    void initForMenu(Context* context, MenuBuilder* menu)override;
    NavigationMenuView* getMenuView(ViewGroup* root)override;
    void updateMenuView(bool cleared)override;
    void setCallback(const Callback& cb)override;
    bool onSubMenuSelected(SubMenuBuilder* subMenu)override;
    void onCloseMenu(MenuBuilder* menu, bool allMenusAreClosing)override;
    bool flagActionItems()override;
    bool expandItemActionView(MenuBuilder& menu, MenuItemImpl& item)override;
    bool collapseItemActionView(MenuBuilder& menu, MenuItemImpl& item)override;
    int getId()const override;
    void setId(int id);
    Parcelable* onSaveInstanceState()override;
    void onRestoreInstanceState(Parcelable& state)override;

    void setCheckedItem(MenuItemImpl* item);
    MenuItemImpl* getCheckedItem();

    View* inflateHeaderView(int res);
    void addHeaderView(View* view);
    void removeHeaderView(View* view);
    int getHeaderCount();
    View* getHeaderView(int index);

    void setSubheaderColor(const RefPtr<ColorStateList>& subheaderColor);
    void setSubheaderTextAppearance(int resId);
    const RefPtr<ColorStateList> getItemTintList()const;
    void setItemIconTintList(const RefPtr<ColorStateList>& tint);
    const RefPtr<ColorStateList> getItemTextColor()const;
    void setItemTextColor(const RefPtr<ColorStateList>& textColor);
    void setItemTextAppearance(int resId);
    void setItemTextAppearanceActiveBoldEnabled(bool isBold);
    Drawable* getItemBackground()const;
    void setItemBackground(Drawable* itemBackground);
    void setItemForeground(Drawable* itemForeground);
    int getItemHorizontalPadding()const;
    void setItemHorizontalPadding(int itemHorizontalPadding);
    int getItemVerticalPadding()const;
    void setItemVerticalPadding(int itemVerticalPadding);
    int getDividerInsetStart()const;
    void setDividerInsetStart(int dividerInsetStart);
    int getDividerInsetEnd()const;
    void setDividerInsetEnd(int dividerInsetEnd);
    int getSubheaderInsetStart()const;
    void setSubheaderInsetStart(int subheaderInsetStart);
    int getSubheaderInsetEnd()const;
    void setSubheaderInsetEnd(int subheaderInsetEnd);
    int getItemIconPadding()const;
    void setItemIconPadding(int itemIconPadding);
    void setItemMaxLines(int itemMaxLines);
    int getItemMaxLines()const;
    void setItemIconSize(int itemIconSize);
    void setUpdateSuspended(bool updateSuspended);
    /** Updates the top padding depending on if this view is drawn behind the status bar. */
    void setBehindStatusBar(bool behindStatusBar);
    /** True if the NavigationView will be drawn behind the status bar */
    bool isBehindStatusBar()const;
    void setOverScrollMode(int overScrollMode);
};

}//namespace cdroid
#endif/*__NAVIGATION_MENU_PRESENTER_H__*/
