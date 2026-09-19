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
#ifndef __NAVIGATION_MENU_ITEM_VIEW_H__
#define __NAVIGATION_MENU_ITEM_VIEW_H__
// Port of com.google.android.material.internal.NavigationMenuItemView (the
// NORMAL row of the navigation menu). CDROID substrate notes: material
// inflates design_navigation_item.xml + design_navigation_menu_item.xml
// (CheckedTextView + ViewStub action area); those layouts are reproduced in
// the constructor with the same theme attrs and dimens, and ForegroundLinearLayout
// collapses into plain LinearLayout (View already carries the foreground).
#include <widget/linearlayout.h>
#include <widget/framelayout.h>
#include <widget/checkedtextview.h>
#include <menu/menuview.h>

namespace cdroid{

class NavigationMenuItemView:public LinearLayout,public MenuView::ItemView{
private:
    static constexpr int DEFAULT_ICON_SIZE_DP = 24;  // design_navigation_icon_size
    static constexpr int ICON_PADDING_DP = 32;       // design_navigation_icon_padding
private:
    int mIconSize;
    bool mNeedsEmptyIcon;
    bool mCheckable;
    bool mIsBold = true;
    CheckedTextView* mTextView;
    FrameLayout* mActionArea = nullptr;
    MenuItemImpl* mItemData = nullptr;
    RefPtr<ColorStateList> mIconTintList;
    bool mHasIconTintList = false;
private:
    bool shouldExpandActionArea();
    void adjustAppearance();
    StateListDrawable* createDefaultBackground();
    void setActionView(View* actionView);
protected:
    std::vector<int> onCreateDrawableState(int extraSpace)override;
public:
    NavigationMenuItemView(Context* context);

    void initialize(MenuItemImpl* itemData,int menuType)override;
    // material: initialize(item, isBold)
    void initialize(MenuItemImpl* itemData,bool isBold);
    void recycle();
    MenuItemImpl* getItemData()override;
    void setTitle(const std::string& title)override;
    void setEnabled(bool enabled)override;
    void setCheckable(bool checkable)override;
    void setChecked(bool checked)override;
    void setShortcut(bool showShortcut,int shortcutKey)override;
    void setIcon(Drawable* icon)override;
    bool prefersCondensedTitle()const override;
    bool showsIcon()override;
    void setIconTintList(const RefPtr<ColorStateList>& tintList);
    void setTextAppearance(int textAppearance);
    void setTextColor(const RefPtr<ColorStateList>& colors);
    void setNeedsEmptyIcon(bool needsEmptyIcon);
    void setHorizontalPadding(int padding);
    void setIconPadding(int padding);
    void setMaxLines(int maxLines);
    void setIconSize(int iconSize);
};

}//namespace cdroid
#endif/*__NAVIGATION_MENU_ITEM_VIEW_H__*/
