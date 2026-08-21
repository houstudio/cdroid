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
// inset-scrim machinery is not ported (the inset-scrim flags remain as inert
// state); everything else — the NavigationMenu menu, the presenter/adapter
// data flow and the public API — follows material 1:1.
#include <widget/framelayout.h>
#include <widgetEx/navigationview/navigationmenupresenter.h>
#include <widgetEx/navigationview/navigationmenu.h>

namespace cdroid{

class NavigationView : public FrameLayout {
public:
    class OnNavigationItemSelectedListener;   // defined below
private:
    static constexpr int PRESENTER_NAVIGATION_VIEW_ID = 1;
private:
    NavigationMenu* mMenu;
    NavigationMenuPresenter* mPresenter;
    OnNavigationItemSelectedListener* mListener = nullptr;
    int mMaxWidth;
    bool mTopInsetScrimEnabled = true;
    bool mBottomInsetScrimEnabled = true;
protected:
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec) override;
public:
    /** Called when an item in the navigation menu is selected. */
    class OnNavigationItemSelectedListener {
    public:
        virtual ~OnNavigationItemSelectedListener() = default;
        virtual bool onNavigationItemSelected(MenuItem* item) = 0;
    };

    NavigationView(Context* context, const AttributeSet& attrs);
    NavigationView(Context* context, const AttributeSet* attrs, int defStyleAttr);
    ~NavigationView() override;

    void setOverScrollMode(int overScrollMode) override;
    void setNavigationItemSelectedListener(OnNavigationItemSelectedListener* listener);

    void inflateMenu(int resId);
    Menu* getMenu();

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
    void setItemHorizontalPaddingResource(int paddingResource);
    int getItemVerticalPadding() const;
    void setItemVerticalPadding(int padding);
    void setItemVerticalPaddingResource(int paddingResource);
    int getItemIconPadding() const;
    void setItemIconPadding(int padding);
    void setItemIconPaddingResource(int paddingResource);

    void setCheckedItem(int id);
    void setCheckedItem(MenuItem* checkedItem);
    MenuItem* getCheckedItem();

    void setItemTextAppearance(int resId);
    void setItemTextAppearanceActiveBoldEnabled(bool isBold);
    void setItemIconSize(int iconSize);
    void setItemMaxLines(int itemMaxLines);
    int getItemMaxLines();

    /** Whether or not the NavigationView will draw a scrim behind the window's top inset. */
    bool isTopInsetScrimEnabled() const;
    void setTopInsetScrimEnabled(bool enabled);
    /** Whether or not the NavigationView will draw a scrim behind the window's bottom inset. */
    bool isBottomInsetScrimEnabled() const;
    void setBottomInsetScrimEnabled(bool enabled);

    /** Get the distance between the start edge of the NavigationView and the start of a menu divider. */
    int getDividerInsetStart() const;
    void setDividerInsetStart(int dividerInsetStart);
    /** Get the distance between the end of a divider and the end of the NavigationView. */
    int getDividerInsetEnd() const;
    void setDividerInsetEnd(int dividerInsetEnd);
    /** Get the distance between the start of the NavigationView and the start of a menu subheader. */
    int getSubheaderInsetStart() const;
    void setSubheaderInsetStart(int subheaderInsetStart);
    /** Get the distance between the end of a menu subheader and the end of the NavigationView. */
    int getSubheaderInsetEnd() const;
    void setSubheaderInsetEnd(int subheaderInsetEnd);
private:
    void init(Context* context, const AttributeSet* attrs, int defStyleAttr);
    RefPtr<ColorStateList> createDefaultColorStateList(int baseColorThemeAttr);
    bool onMenuItemSelected(MenuItem& item);
};

}//namespace cdroid
#endif/*__NAVIGATION_VIEW_H__*/
