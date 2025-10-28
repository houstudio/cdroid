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
#ifndef __SNAP_HELPER_H__
#define __SNAP_HELPER_H__
#include <widgetEx/recyclerview/recyclerview.h>

namespace cdroid{
class OrientationHelper;
class LinearSmoothScroller;
class SnapHelper{// extends RecyclerView.OnFlingListener {
public:
    static constexpr float MILLISECONDS_PER_INCH = 100.f;
private:
    bool mScrolled;
    Scroller* mGravityScroller;
    // Handles the snap on scroll case.
    RecyclerView::OnScrollListener mScrollListener;
protected:
    RecyclerView* mRecyclerView;
private:
    void setupCallbacks();
    void destroyCallbacks();
    bool snapFromFling(RecyclerView::LayoutManager& layoutManager, int velocityX,int velocityY);
protected:
    virtual void snapToTargetExistingView();
    RecyclerView::SmoothScroller* createScroller(RecyclerView::LayoutManager& layoutManager);
    LinearSmoothScroller* createSnapScroller(RecyclerView::LayoutManager& layoutManager);
public:
    SnapHelper();
    virtual ~SnapHelper();
    bool onFling(int velocityX, int velocityY);
    void attachToRecyclerView(RecyclerView* recyclerView);

    void calculateScrollDistance(int velocityX, int velocityY,int snapDistance[2]);
    virtual void calculateDistanceToFinalSnap(RecyclerView::LayoutManager& layoutManager,View& targetView,int distance[2])=0;
    virtual View* findSnapView(RecyclerView::LayoutManager& layoutManager)=0;

    virtual int findTargetSnapPosition(RecyclerView::LayoutManager& layoutManager, int velocityX,
            int velocityY)=0;
};
}/*endof namespace*/
#endif/*__SNAP_HELPER_H__*/
