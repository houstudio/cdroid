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
#ifndef __NAVIGATION_VIEW_H__
#define __NAVIGATION_VIEW_H__
// Port of com.google.android.material.navigation.NavigationView (the drawer
// side-sheet menu). CDROID substrate notes: the material shape/back-handler/
// inset-scrim machinery is not ported (stubbed per AGENTS.md); the menu runs
// on CDROID's MenuBuilder family with a presenter that renders items into a
// vertical LinearLayout (the androidx RecyclerView adapter is simplified —
// data flow and public API stay 1:1).
#include <widget/framelayout.h>
#include <widget/linearlayout.h>

namespace cdroid{
class Menu;
class MenuView;
class MenuBuilder;
class MenuInflater;
class MenuItem;
class ColorStateList;

class NavigationView : public FrameLayout {
public:
    enum LabelVisibility{
         LABEL_VISIBILITY_AUTO = -1,
         LABEL_VISIBILITY_SELECTED = 0,
         LABEL_VISIBILITY_LABELED = 1,
         LABEL_VISIBILITY_UNLABELED = 2
    };
    enum ItemGravity{
        ITEM_GRAVITY_TOP_CENTER = Gravity::TOP | Gravity::CENTER_HORIZONTAL,
        ITEM_GRAVITY_CENTER = Gravity::CENTER,
        ITEM_GRAVITY_START_CENTER = Gravity::START | Gravity::CENTER_VERTICAL
    };
    enum ItemIconGravity{
        ITEM_ICON_GRAVITY_TOP = 0,
        ITEM_ICON_GRAVITY_START = 1
    };
    /** Called when an item in the navigation menu is selected. */
    class OnNavigationItemSelectedListener {
    public:
        virtual ~OnNavigationItemSelectedListener() = default;
        virtual bool onNavigationItemSelected(MenuItem* item) = 0;
    };
private:
    class NavigationMenuPresenter;   // defined in the .cc (menu view holder)
    NavigationMenuPresenter* mPresenter;
    MenuBuilder* mMenu;
    class OnNavigationItemSelectedListener* mListener;
    int mMaxWidth;
    // Item presentation (mirrors the material presenter's fields).
    RefPtr<ColorStateList> mItemIconTint;
    RefPtr<ColorStateList> mItemTextColor;
    Drawable* mItemBackground;
    int mItemIconSize;
    int mItemHorizontalPadding;
    int mItemVerticalPadding;
    int mItemIconPadding;
    int mItemTextAppearance;   // style res id, 0 = none
    int mItemMaxLines;
    LinearLayout* mHeaderView;  // stacked above the menu list
    LinearLayout* mMenuView;   // presenter's menu view (vertical list)
protected:
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec) override;
public:
    NavigationView(Context* context, const AttributeSet& attrs);
    NavigationView(Context* context, const AttributeSet* attrs, int defStyleAttr);
    ~NavigationView() override;

    void setNavigationItemSelectedListener(OnNavigationItemSelectedListener* listener);

    void inflateMenu(int resId);
    Menu* getMenu();
    //MenuView*getMenuView();
    ViewGroup*getMenuViewGroup();

    View* inflateHeaderView(int res);
    void addHeaderView(View* view);
    void removeHeaderView(View* view);
    int getHeaderCount()const;
    View* getHeaderView(int index)const;

    const RefPtr<ColorStateList> getItemIconTintList() const;
    void setItemIconTintList(const RefPtr<ColorStateList>& tint);
    const RefPtr<ColorStateList> getItemTextColor() const;
    void setItemTextColor(const RefPtr<ColorStateList>& textColor);
    Drawable* getItemBackground() const;
    void setItemBackgroundResource(int resId);
    void setItemBackground(Drawable* itemBackground);
    int getItemHorizontalPadding() const;
    void setItemHorizontalPadding(int padding);
    int getItemVerticalPadding() const;
    void setItemVerticalPadding(int padding);
    int getItemIconPadding() const;
    void setItemIconPadding(int padding);
    void setItemIconSize(int iconSize);
    void setItemIconSizeResource(int resId);
    void setItemTextAppearance(int resId);
    void setItemMaxLines(int maxLines);
private:
    void init(Context* context, const AttributeSet* attrs, int defStyleAttr);
    void updateMenuView();      // presenter.updateMenuView: rebuild item views
    View* createItemView(MenuItem* item);
    bool onMenuItemClick(MenuItem* item);
};

}//namespace cdroid
#endif/*__NAVIGATION_VIEW_H__*/
