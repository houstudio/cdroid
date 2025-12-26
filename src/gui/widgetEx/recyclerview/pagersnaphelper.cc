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
#include <widgetEx/recyclerview/pagersnaphelper.h>
#include <widgetEx/recyclerview/linearsmoothscroller.h>
#include <widgetEx/recyclerview/orientationhelper.h>

namespace cdroid{

////////////////////////////////////////////////////////////////////////////////////////////////////////
PagerSnapHelper::PagerSnapHelper(){
    mVerticalHelper = nullptr;
    mHorizontalHelper = nullptr;
}
PagerSnapHelper::~PagerSnapHelper(){
    delete mVerticalHelper;
    delete mHorizontalHelper;
}

void PagerSnapHelper::calculateDistanceToFinalSnap(RecyclerView::LayoutManager& layoutManager,
       View& targetView,int out[2]) {
    out[0] = out[1] = 0;
    if (layoutManager.canScrollHorizontally()) {
        out[0] = distanceToCenter(layoutManager, targetView,getHorizontalHelper(layoutManager));
    }

    if (layoutManager.canScrollVertically()) {
        out[1] = distanceToCenter(layoutManager, targetView,getVerticalHelper(layoutManager));
    }
}

View* PagerSnapHelper::findSnapView(RecyclerView::LayoutManager& layoutManager) {
    if (layoutManager.canScrollVertically()) {
        return findCenterView(layoutManager, getVerticalHelper(layoutManager));
    } else if (layoutManager.canScrollHorizontally()) {
        return findCenterView(layoutManager, getHorizontalHelper(layoutManager));
    }
    return nullptr;
}

int PagerSnapHelper::findTargetSnapPosition(RecyclerView::LayoutManager& layoutManager, int velocityX,
        int velocityY) {
    const int itemCount = layoutManager.getItemCount();
    OrientationHelper* orientationHelper = getOrientationHelper(layoutManager);
    if ( (itemCount == 0) || (orientationHelper==nullptr) ) {
        return RecyclerView::NO_POSITION;
    }

    // A child that is exactly in the center is eligible for both before and after
    View* closestChildBeforeCenter = nullptr;
    int distanceBefore = INT_MIN;
    View* closestChildAfterCenter = nullptr;
    int distanceAfter = INT_MAX;

    // Find the first view before the center, and the first view after the center
    const int childCount = layoutManager.getChildCount();
    for (int i = 0; i < childCount; i++) {
        View* child = layoutManager.getChildAt(i);
        if (child == nullptr) {
            continue;
        }
        const int distance = distanceToCenter(layoutManager, *child, *orientationHelper);

        if (distance <= 0 && distance > distanceBefore) {
            // Child is before the center and closer then the previous best
            distanceBefore = distance;
            closestChildBeforeCenter = child;
        }
        if (distance >= 0 && distance < distanceAfter) {
            // Child is after the center and closer then the previous best
            distanceAfter = distance;
            closestChildAfterCenter = child;
        }
    }

    // Return the position of the first child from the center, in the direction of the fling
    const bool forwardDirection = isForwardFling(layoutManager, velocityX, velocityY);
    if (forwardDirection && closestChildAfterCenter != nullptr) {
        return layoutManager.getPosition(closestChildAfterCenter);
    } else if (!forwardDirection && closestChildBeforeCenter != nullptr) {
        return layoutManager.getPosition(closestChildBeforeCenter);
    }

    // There is no child in the direction of the fling. Either it doesn't exist (start/end of
    // the list), or it is not yet attached (very rare case when children are larger then the
    // viewport). Extrapolate from the child that is visible to get the position of the view to
    // snap to.
    View* visibleView = forwardDirection ? closestChildBeforeCenter : closestChildAfterCenter;
    if (visibleView == nullptr) {
        return RecyclerView::NO_POSITION;
    }
    const int visiblePosition = layoutManager.getPosition(visibleView);
    const int snapToPosition = visiblePosition
            + (isReverseLayout(layoutManager) == forwardDirection ? -1 : +1);

    if ( (snapToPosition < 0) || (snapToPosition >= itemCount) ){
        return RecyclerView::NO_POSITION;
    }
    return snapToPosition;    
}

bool PagerSnapHelper::isForwardFling(RecyclerView::LayoutManager& layoutManager, int velocityX,int velocityY)const {
    if (layoutManager.canScrollHorizontally()) {
        return velocityX > 0;
    } else {
        return velocityY > 0;
    }
}

bool PagerSnapHelper::isReverseLayout(RecyclerView::LayoutManager& layoutManager)const {
    const int itemCount = layoutManager.getItemCount();
    PointF vectorForEnd;
    if (layoutManager.computeScrollVectorForPosition(itemCount - 1,vectorForEnd)){
        return vectorForEnd.x < 0 || vectorForEnd.y < 0;
    }
    return false;
}

class MyPageSmoothScroller:public LinearSmoothScroller{
private:
    SnapHelper* mSnapHelper;
    RecyclerView* mRecyclerView;
public:
    MyPageSmoothScroller(SnapHelper* snap,RecyclerView* rv)
	    :LinearSmoothScroller(rv->getContext()){
        mSnapHelper = snap;
        mRecyclerView = rv;
    }
protected:
    void onTargetFound(View* targetView, RecyclerView::State& state, Action& action) override{
        int snapDistances[2];
        RecyclerView::LayoutManager*layoutManger = mRecyclerView->getLayoutManager();
        mSnapHelper->calculateDistanceToFinalSnap(*layoutManger,*targetView,snapDistances);
        const int dx = snapDistances[0];
        const int dy = snapDistances[1];
        int time = calculateTimeForDeceleration(std::max(std::abs(dx), std::abs(dy)));
        if (time > 0) {
            action.update(dx, dy, time, mDecelerateInterpolator);
        }
    }

    float calculateSpeedPerPixel(DisplayMetrics& displayMetrics) override{
        return SnapHelper::MILLISECONDS_PER_INCH / displayMetrics.densityDpi;
    }

    int calculateTimeForScrolling(int dx) override{
        return std::min((int)PagerSnapHelper::MAX_SCROLL_ON_FLING_DURATION,
		   LinearSmoothScroller::calculateTimeForScrolling(dx));
    }
};

LinearSmoothScroller* PagerSnapHelper::createSnapScroller(RecyclerView::LayoutManager& layoutManager) {
    PointF scrollVector;
    if(!layoutManager.computeScrollVectorForPosition(0,scrollVector)){
        return nullptr;
    }
    return new MyPageSmoothScroller(this,mRecyclerView);
}

int PagerSnapHelper::distanceToCenter(RecyclerView::LayoutManager& layoutManager,
        View& targetView, OrientationHelper& helper) {
    const int childCenter = helper.getDecoratedStart(&targetView)
            + (helper.getDecoratedMeasurement(&targetView) / 2);
    int containerCenter;
    if (layoutManager.getClipToPadding()) {
        containerCenter = helper.getStartAfterPadding() + helper.getTotalSpace() / 2;
    } else {
        containerCenter = helper.getEnd() / 2;
    }
    return childCenter - containerCenter;
}


View* PagerSnapHelper::findCenterView(RecyclerView::LayoutManager& layoutManager,
        OrientationHelper& helper) {
    const int childCount = layoutManager.getChildCount();
    if (childCount == 0) {
        return nullptr;
    }

    View* closestChild = nullptr;
    int center;
    if (layoutManager.getClipToPadding()) {
        center = helper.getStartAfterPadding() + helper.getTotalSpace() / 2;
    } else {
        center = helper.getEnd() / 2;
    }
    int absClosest = INT_MAX;//Integer.MAX_VALUE;

    for (int i = 0; i < childCount; i++) {
        View* child = layoutManager.getChildAt(i);
        int childCenter = helper.getDecoratedStart(child)
                + (helper.getDecoratedMeasurement(child) / 2);
        int absDistance = std::abs(childCenter - center);

        /** if child center is closer than previous closest, set it as closest  **/
        if (absDistance < absClosest) {
            absClosest = absDistance;
            closestChild = child;
        }
    }
    return closestChild;
}

View* PagerSnapHelper::findStartView(RecyclerView::LayoutManager& layoutManager,OrientationHelper& helper) {
    int childCount = layoutManager.getChildCount();
    if (childCount == 0) {
        return nullptr;
    }

    View* closestChild = nullptr;
    int startest = INT_MAX;// Integer.MAX_VALUE;

    for (int i = 0; i < childCount; i++) {
        View* child = layoutManager.getChildAt(i);
        int childStart = helper.getDecoratedStart(child);

        /** if child is more to start than previous closest, set it as closest  **/
        if (childStart < startest) {
            startest = childStart;
            closestChild = child;
        }
    }
    return closestChild;
}

OrientationHelper* PagerSnapHelper::getOrientationHelper(RecyclerView::LayoutManager& layoutManager){
    if (layoutManager.canScrollVertically()) {
        return &getVerticalHelper(layoutManager);
    } else if (layoutManager.canScrollHorizontally()) {
        return &getHorizontalHelper(layoutManager);
    } else {
        return nullptr;
    }
}

OrientationHelper& PagerSnapHelper::getVerticalHelper(RecyclerView::LayoutManager& layoutManager) {
    if (mVerticalHelper == nullptr || mVerticalHelper->getLayoutManager() != &layoutManager) {
        mVerticalHelper = OrientationHelper::createVerticalHelper(&layoutManager);
    }
    return *mVerticalHelper;
}

OrientationHelper& PagerSnapHelper::getHorizontalHelper(RecyclerView::LayoutManager& layoutManager) {
    if (mHorizontalHelper == nullptr || mHorizontalHelper->getLayoutManager() != &layoutManager) {
        mHorizontalHelper = OrientationHelper::createHorizontalHelper(&layoutManager);
    }
    return *mHorizontalHelper;
}

}/*endof namespace*/
