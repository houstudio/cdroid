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
#ifndef __FAKE_DRAG_H__
#define __FAKE_DRAG_H__
#include <widgetEx/recyclerview/recyclerview.h>
class ViewPager2;
namespace cdroid{
class ScrollEventAdapter;
class FakeDrag {
private:
    ViewPager2* mViewPager;
    ScrollEventAdapter* mScrollEventAdapter;
    RecyclerView* mRecyclerView;
    VelocityTracker* mVelocityTracker;
    int mMaximumVelocity;
    float mRequestedDragDistance;
    int mActualDraggedDistance;
    nsecs_t mFakeDragBeginTime;
private:
    void beginFakeVelocityTracker();
    void addFakeMotionEvent(int64_t time, int action, float x, float y);
public:
    FakeDrag(ViewPager2* viewPager, ScrollEventAdapter* scrollEventAdapter, RecyclerView* recyclerView);
    bool isFakeDragging();
    bool beginFakeDrag();
    bool fakeDragBy(float offsetPxFloat);
    bool endFakeDrag();
};
}/*endof namespace*/
#endif
