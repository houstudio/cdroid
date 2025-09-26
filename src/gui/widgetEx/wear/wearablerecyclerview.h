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
*/
#ifndef __WEARABLE_RECYCLERVIEW_H__
#define __WEARABLE_RECYCLERVIEW_H__
#include <widgetEx/wear/scrollmanager.h>
#include <widgetEx/recyclerview/recyclerview.h>
namespace cdroid{
class WearableRecyclerView:public RecyclerView {
private:
    static constexpr int NO_VALUE = INT_MIN;

    ScrollManager* mScrollManager;
    bool mCircularScrollingEnabled;
    bool mEdgeItemsCenteringEnabled;
    bool mCenterEdgeItemsWhenThereAreChildren;

    int mOriginalPaddingTop = NO_VALUE;
    int mOriginalPaddingBottom = NO_VALUE;

    ViewTreeObserver::OnPreDrawListener mPaddingPreDrawListener;
private:
    void setupOriginalPadding();
protected:
    void onAttachedToWindow() override;
    void onDetachedFromWindow() override;
public:
    WearableRecyclerView(int w,int h);
    WearableRecyclerView(Context* context, const AttributeSet& attrs);
    ~WearableRecyclerView()override;
    void setupCenteredPadding();

    bool onTouchEvent(MotionEvent& event) override;
    void setCircularScrollingGestureEnabled(bool circularScrollingGestureEnabled);
    bool isCircularScrollingGestureEnabled()const;

    void setScrollDegreesPerScreen(float degreesPerScreen);
    float getScrollDegreesPerScreen()const;

    void setBezelFraction(float fraction);
    float getBezelFraction()const;

    void setEdgeItemsCenteringEnabled(bool isEnabled);
    bool isEdgeItemsCenteringEnabled()const;
};
}/*endof namespace*/
#endif/*__WEARABLE_RECYCLERVIEW_H__*/
