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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  0211-1301  USA
 *********************************************************************************/
#ifndef __CDROID_APP_BUTTONBARLAYOUT_H__
#define __CDROID_APP_BUTTONBARLAYOUT_H__
#include <widget/linearlayout.h>

namespace cdroid{
/** An extension of LinearLayout that automatically switches to vertical
 *  orientation when it can't fit its child views horizontally. */
class ButtonBarLayout:public LinearLayout{
private:
    /** Amount of the second button to "peek" above the fold when stacked. */
    static constexpr int PEEK_BUTTON_DP = 16;
    /** Whether the current configuration allows stacking. */
    bool mAllowStacking;
    int mLastWidthSize = -1;
    int mMinimumHeight = 0;

    int getNextVisibleChildIndex(int index);
    void setStacked(bool stacked);
    bool isStacked()const;
public:
    ButtonBarLayout(Context* context,const AttributeSet* attrs);
    ButtonBarLayout(Context* context,const AttributeSet* attrs,int defStyleAttr);
    void setAllowStacking(bool allowStacking);
protected:
    void onMeasure(int widthMeasureSpec,int heightMeasureSpec)override;
public:
    int getMinimumHeight()override;
};
};//endof namespace
#endif//__CDROID_APP_BUTTONBARLAYOUT_H__
