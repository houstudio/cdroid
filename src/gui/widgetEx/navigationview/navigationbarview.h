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
#ifndef __NAVIGATION_BAR_VIEW_H__
#define __NAVIGATION_BAR_VIEW_H__
// Port of com.google.android.material.navigation.NavigationBarView — the
// abstract base of BottomNavigationView. CDROID substrate: the menu runs on
// CDROID's MenuBuilder; items render into a horizontal LinearLayout of
// icon+label buttons (material's item views/badge/active-indicator machinery
// is stubbed per AGENTS.md).
#include <widget/framelayout.h>
#include <widget/linearlayout.h>
#include <cdroid.h>

namespace cdroid{
class Menu;
class MenuBuilder;
class MenuItem;
class ColorStateList;

class NavigationBarView : public FrameLayout {
public:
    enum LabelVisibility{
        LABEL_VISIBILITY_AUTO = -1,
        LABEL_VISIBILITY_SELECTED = 0,
        LABEL_VISIBILITY_LABELED = 1,
        LABEL_VISIBILITY_UNLABELED = 2
    };
    enum ItemGravity{
        ITEM_GRAVITY_TOP_CENTER=Gravity::TOP | Gravity::CENTER_HORIZONTAL,
        ITEM_GRAVITY_CENTER = Gravity::CENTER,
        ITEM_GRAVITY_START_CENTER = Gravity::START | Gravity::CENTER_VERTICAL
    };
    enum ItemIconGravity{
        ITEM_ICON_GRAVITY_TOP=0,
        ITEM_ICON_GRAVITY_START=1
    };
    /** Called when an item is selected. */
    class OnItemSelectedListener {
    public:
        virtual ~OnItemSelectedListener() = default;
        virtual bool onNavigationItemSelected(MenuItem* item) = 0;
    };
    /** Called when the currently selected item is reselected. */
    class OnItemReselectedListener {
    public:
        virtual ~OnItemReselectedListener() = default;
        virtual void onNavigationItemReselected(MenuItem* item) = 0;
    };
private:
    MenuBuilder* mMenu;
    LinearLayout* mMenuView;   // horizontal item strip
    OnItemSelectedListener* mItemSelectedListener;
    OnItemReselectedListener* mItemReselectedListener;
    // Item presentation (material NavigationBarMenuView fields).
    RefPtr<ColorStateList> mItemIconTint;
    RefPtr<ColorStateList> mItemTextColor;
    Drawable* mItemBackground;
    int mItemIconSize;
    int mLabelVisibilityMode;  // LABEL_VISIBILITY_*
    int mItemGravity;          // ITEM_GRAVITY_*
    int mItemIconGravity;      // ITEM_ICON_GRAVITY_*
    // material NavigationBarMenuView presentation fields (merged into this
    // substrate class; style res ids, 0 = none).
    int mItemTextAppearanceInactive;
    int mItemTextAppearanceActive;
    int mHorizontalItemTextAppearanceInactive;
    int mHorizontalItemTextAppearanceActive;
    bool mItemTextAppearanceActiveBoldEnabled;
    RefPtr<ColorStateList> mItemRippleColor;
    int mItemPaddingTop;            // -1 = substrate default
    int mItemPaddingBottom;         // -1 = substrate default
    int mActiveIndicatorLabelPadding;
    int mIconLabelHorizontalSpacing;
    int mActiveIndicatorWidth;      // 0 = no active indicator
    int mActiveIndicatorHeight;
    int mActiveIndicatorMarginX;
    int mActiveIndicatorColor;
protected:
    NavigationBarView(Context* context, const AttributeSet* attrs, int defStyleAttr);
    virtual int getMaxItemCount() const = 0;
    void updateMenuView();
    virtual View* createItemView(MenuItem* item);
    bool onMenuItemClick(MenuItem* item);
public:
    ~NavigationBarView() override;

    void setOnItemSelectedListener(OnItemSelectedListener* listener);
    void setOnItemReselectedListener(OnItemReselectedListener* listener);
    Menu* getMenu();
    ViewGroup* getMenuViewGroup();
    void refreshMenuView();

    const RefPtr<ColorStateList> getItemIconTintList() const;
    void setItemIconTintList(const RefPtr<ColorStateList>& tint);
    int getItemIconSize() const;
    void setItemIconSize(int iconSize);
    void setItemIconSizeRes(int iconSizeRes);
    const RefPtr<ColorStateList> getItemTextColor() const;
    void setItemTextColor(const RefPtr<ColorStateList>& textColor);
    Drawable* getItemBackground() const;
    void setItemBackgroundResource(int resId);
    void setItemBackground(Drawable* background);
    int getLabelVisibilityMode() const;
    void setLabelVisibilityMode(int labelVisibilityMode);
    /** Returns the gravity of the items within the navigation bar (ITEM_GRAVITY_*). */
    int getItemGravity() const;
    void setItemGravity(int itemGravity);
    /** Returns the icon gravity which determines the item layout configuration (ITEM_ICON_GRAVITY_*). */
    int getItemIconGravity() const;
    void setItemIconGravity(int itemIconGravity);

    int getItemTextAppearanceInactive() const;
    void setItemTextAppearanceInactive(int textAppearanceRes);
    int getItemTextAppearanceActive() const;
    void setItemTextAppearanceActive(int textAppearanceRes);
    void setItemTextAppearanceActiveBoldEnabled(bool isBold);
    const RefPtr<ColorStateList> getItemRippleColor() const;
    void setItemRippleColor(const RefPtr<ColorStateList>& itemRippleColor);
    int getItemPaddingTop() const;
    void setItemPaddingTop(int paddingTop);
    int getItemPaddingBottom() const;
    void setItemPaddingBottom(int paddingBottom);
    /** Horizontal gap between the icon and the label in icon-start items
        (app:iconLabelHorizontalSpacing); ignored in the icon-top layout. */
    int getIconLabelHorizontalSpacing() const;
    void setIconLabelHorizontalSpacing(int spacing);
    int getActiveIndicatorLabelPadding() const;
    void setActiveIndicatorLabelPadding(int activeIndicatorLabelPadding);
};

}//namespace cdroid
#endif/*__NAVIGATION_BAR_VIEW_H__*/
