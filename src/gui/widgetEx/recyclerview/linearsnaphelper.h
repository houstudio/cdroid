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
#ifndef __LINEAR_SNAP_HELPER_H__
#define __LINEAR_SNAP_HELPER_H__
#include <widgetEx/recyclerview/snaphelper.h>
namespace cdroid{
class LinearSnapHelper:public SnapHelper {
private:
    static constexpr float INVALID_DISTANCE = 1.f;

    // Orientation helpers are lazily created per LayoutManager.
    OrientationHelper* mVerticalHelper;
    OrientationHelper* mHorizontalHelper;
private:
    int distanceToCenter(RecyclerView::LayoutManager& layoutManager, View& targetView, OrientationHelper& helper);
    int estimateNextPositionDiffForFling(RecyclerView::LayoutManager& layoutManager,
            OrientationHelper& helper, int velocityX, int velocityY);
    View* findCenterView(RecyclerView::LayoutManager& layoutManager, OrientationHelper& helper);
    float computeDistancePerChild(RecyclerView::LayoutManager& layoutManager, OrientationHelper& helper);
    OrientationHelper& getVerticalHelper(RecyclerView::LayoutManager& layoutManager);
    OrientationHelper& getHorizontalHelper(RecyclerView::LayoutManager& layoutManager);
public:
    LinearSnapHelper();
    ~LinearSnapHelper()override;
    void calculateDistanceToFinalSnap(RecyclerView::LayoutManager& layoutManager,View& targetView,int distance[2])override;
    int findTargetSnapPosition(RecyclerView::LayoutManager& layoutManager, int velocityX,int velocityY)override;
    View* findSnapView(RecyclerView::LayoutManager& layoutManager)override;
};
}/*endof namespace*/
#endif/*__LINEAR_SNAP_HELPER_H__*/
