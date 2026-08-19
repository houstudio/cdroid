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
#ifndef __BOTTOM_NAVIGATION_VIEW_H__
#define __BOTTOM_NAVIGATION_VIEW_H__
// Port of com.google.android.material.bottomnavigation.BottomNavigationView.
#include <widgetEx/navigationview/navigationbarview.h>

namespace cdroid{

class BottomNavigationView : public NavigationBarView {
private:
    static constexpr int MAX_ITEM_COUNT = 6;
protected:
    int getMaxItemCount() const override { return MAX_ITEM_COUNT; }
public:
    BottomNavigationView(Context* context, const AttributeSet& attrs);
    BottomNavigationView(Context* context, const AttributeSet* attrs, int defStyleAttr);
    bool onTouchEvent(MotionEvent& event) override;
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec) override;
};

}//namespace cdroid
#endif/*__BOTTOM_NAVIGATION_VIEW_H__*/
