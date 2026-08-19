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
};

}//namespace cdroid
#endif/*__NAVIGATION_BAR_VIEW_H__*/
