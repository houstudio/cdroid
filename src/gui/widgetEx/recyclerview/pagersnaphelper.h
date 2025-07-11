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
#ifndef __PAGER_SNAP_HELPER_H__
#define __PAGER_SNAP_HELPER_H__
#include <widgetEx/recyclerview/snaphelper.h>
namespace cdroid{
class PagerSnapHelper:public SnapHelper {
public:
    static constexpr int MAX_SCROLL_ON_FLING_DURATION = 100; // ms
private:
    OrientationHelper* mVerticalHelper;
    OrientationHelper* mHorizontalHelper;
private:
    bool isForwardFling(RecyclerView::LayoutManager& layoutManager, int velocityX,int velocityY)const;
    bool isReverseLayout(RecyclerView::LayoutManager& layoutManager)const;
    int distanceToCenter(RecyclerView::LayoutManager& layoutManager, View& targetView, OrientationHelper& helper);
    View* findCenterView(RecyclerView::LayoutManager& layoutManager, OrientationHelper& helper);
    View* findStartView(RecyclerView::LayoutManager& layoutManager, OrientationHelper& helper);
    OrientationHelper* getOrientationHelper(RecyclerView::LayoutManager& layoutManager);
    OrientationHelper& getVerticalHelper(RecyclerView::LayoutManager& layoutManager);
    OrientationHelper& getHorizontalHelper(RecyclerView::LayoutManager& layoutManager);
protected:
    LinearSmoothScroller* createSnapScroller(RecyclerView::LayoutManager& layoutManager);
public:
    PagerSnapHelper();
    ~PagerSnapHelper()override;
    void calculateDistanceToFinalSnap(RecyclerView::LayoutManager& layoutManager, View& targetView,int distance[2])override;
    View* findSnapView(RecyclerView::LayoutManager& layoutManager)override;
    int findTargetSnapPosition(RecyclerView::LayoutManager& layoutManager, int velocityX,int velocityY)override;
};
}/*endof namespace*/
#endif/*__PAGER_SNAP_HELPER_H__*/
