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
#ifndef __NESTED_SCROLLING_HELPERS_H__
#define __NESTED_SCROLLING_HELPERS_H__
#include <view/viewgroup.h>

namespace cdroid{

class NestedScrollingParentHelper {
private:
    ViewGroup* mViewGroup;
    int mNestedScrollAxes;
public:
    NestedScrollingParentHelper(ViewGroup* viewGroup); 
    void onNestedScrollAccepted(View* child,View* target,int axes);
    void onNestedScrollAccepted(View* child,View* target,int axes,int type);
    int  getNestedScrollAxes()const;
    void onStopNestedScroll(View* target);
    void onStopNestedScroll(View* target,int type); 
};

class NestedScrollingChildHelper {
private :
   ViewGroup* mNestedScrollingParentTouch;
   ViewGroup* mNestedScrollingParentNonTouch;
   View* mView;
   bool mIsNestedScrollingEnabled;
   int  mTempNestedScrollConsumed[2];
private:
    ViewGroup* getNestedScrollingParentForType(int type)const;
    void setNestedScrollingParentForType(int type, ViewGroup* p);
    bool dispatchNestedScrollInternal(int dxConsumed, int dyConsumed,int dxUnconsumed,
            int dyUnconsumed,int* offsetInWindow,int type, int*consumed);
public:
    NestedScrollingChildHelper(View* view);
    void setNestedScrollingEnabled(bool enabled);

    bool isNestedScrollingEnabled()const;
    bool hasNestedScrollingParent()const;
    bool hasNestedScrollingParent(int type)const;
    bool startNestedScroll( int axes);
    bool startNestedScroll( int axes, int type);
    void stopNestedScroll();

    void stopNestedScroll(int type);
    bool dispatchNestedScroll(int dxConsumed, int dyConsumed,int dxUnconsumed, int dyUnconsumed, int* offsetInWindow);
    bool dispatchNestedScroll(int dxConsumed, int dyConsumed,int dxUnconsumed, int dyUnconsumed, int* offsetInWindow,int type);
    bool dispatchNestedScroll(int dxConsumed, int dyConsumed,int dxUnconsumed, int dyUnconsumed, int* offsetInWindow,int type,int*cvomsumed);

    bool dispatchNestedPreScroll(int dx, int dy, int* consumed ,int* offsetInWindow);
    bool dispatchNestedPreScroll(int dx, int dy, int* consumed ,int* offsetInWindow,int type);
    bool dispatchNestedFling(float velocityX, float velocityY, bool consumed);
    bool dispatchNestedPreFling(float velocityX, float velocityY);
    void onDetachedFromWindow();
    void onStopNestedScroll( View* child);
};
}//endof namespace

#endif//__NESTED_SCROLLING_HELPERS_H__
