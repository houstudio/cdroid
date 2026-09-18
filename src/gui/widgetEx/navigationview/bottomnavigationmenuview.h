/*********************************************************************************
 * Copyright (C) 2019] [houzh@msn.com]
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
#ifndef __BOTTOM_NAVIGATION_MENU_VIEW_H__
#define __BOTTOM_NAVIGATION_MENU_VIEW_H__
// Port of com.google.android.material.bottomnavigation.BottomNavigationMenuView
// - the concrete menu view for BottomNavigationView: the classic equal-width
// measure pass and, with ITEM_ICON_GRAVITY_TOP + shifting + horizontal
// translation, material's active/inactive width allocation.
#include <widgetEx/navigationview/navigationbarmenuview.h>

namespace cdroid{

class BottomNavigationMenuView : public NavigationBarMenuView {
private:
    int mInactiveItemMaxWidth;
    int mInactiveItemMinWidth;
    int mActiveItemMaxWidth;
    int mActiveItemMinWidth;

    bool mItemHorizontalTranslationEnabled = false;
    std::vector<int> mTempChildWidths;
protected:
    NavigationBarItemView* createNavigationBarItemView(Context* context) override;
public:
    BottomNavigationMenuView(Context* context, const AttributeSet* attrs = nullptr);

    void setItemHorizontalTranslationEnabled(bool itemHorizontalTranslationEnabled);
    bool isItemHorizontalTranslationEnabled() const;

    void onMeasure(int widthMeasureSpec, int heightMeasureSpec) override;
    void onLayout(bool changed, int left, int top, int right, int bottom) override;
};

} // namespace cdroid
#endif /* __BOTTOM_NAVIGATION_MENU_VIEW_H__ */
