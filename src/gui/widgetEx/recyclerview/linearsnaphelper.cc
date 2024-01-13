#include <widgetEx/recyclerview/linearsnaphelper.h>
#include <widgetEx/recyclerview/orientationhelper.h>

namespace cdroid{

LinearSnapHelper::LinearSnapHelper(){
    mVerticalHelper = nullptr;
	mHorizontalHelper = nullptr;
}
LinearSnapHelper::~LinearSnapHelper(){
    delete mVerticalHelper;
	delete mHorizontalHelper;
}

void LinearSnapHelper::calculateDistanceToFinalSnap(
        RecyclerView::LayoutManager& layoutManager,View& targetView,int out[2]) {
    if (layoutManager.canScrollHorizontally()) {
        out[0] = distanceToCenter(layoutManager, targetView, getHorizontalHelper(layoutManager));
    } else {
        out[0] = 0;
    }

    if (layoutManager.canScrollVertically()) {
        out[1] = distanceToCenter(layoutManager, targetView, getVerticalHelper(layoutManager));
    } else {
        out[1] = 0;
    }
}

int LinearSnapHelper::findTargetSnapPosition(RecyclerView::LayoutManager& layoutManager, int velocityX, int velocityY) {
    PointF vectorForEnd;

    const int itemCount = layoutManager.getItemCount();
    if (itemCount == 0) {
        return RecyclerView::NO_POSITION;
    }
    if(!layoutManager.computeScrollVectorForPosition(itemCount - 1,&vectorForEnd)){
        return RecyclerView::NO_POSITION;
    }
    View* currentView = findSnapView(layoutManager);
    if (currentView == nullptr) {
        return RecyclerView::NO_POSITION;
    }

    const int currentPosition = layoutManager.getPosition(currentView);
    if (currentPosition == RecyclerView::NO_POSITION) {
        return RecyclerView::NO_POSITION;
    }

    int vDeltaJump, hDeltaJump;
    if (layoutManager.canScrollHorizontally()) {
        hDeltaJump = estimateNextPositionDiffForFling(layoutManager,
                getHorizontalHelper(layoutManager), velocityX, 0);
        if (vectorForEnd.x < 0) {
            hDeltaJump = -hDeltaJump;
        }
    } else {
        hDeltaJump = 0;
    }
    if (layoutManager.canScrollVertically()) {
        vDeltaJump = estimateNextPositionDiffForFling(layoutManager,
                getVerticalHelper(layoutManager), 0, velocityY);
        if (vectorForEnd.y < 0) {
            vDeltaJump = -vDeltaJump;
        }
    } else {
        vDeltaJump = 0;
    }

    int deltaJump = layoutManager.canScrollVertically() ? vDeltaJump : hDeltaJump;
    if (deltaJump == 0) {
        return RecyclerView::NO_POSITION;
    }

    int targetPos = currentPosition + deltaJump;
    if (targetPos < 0) {
        targetPos = 0;
    }
    if (targetPos >= itemCount) {
        targetPos = itemCount - 1;
    }
    return targetPos;
}

View* LinearSnapHelper::findSnapView(RecyclerView::LayoutManager& layoutManager) {
    if (layoutManager.canScrollVertically()) {
        return findCenterView(layoutManager, getVerticalHelper(layoutManager));
    } else if (layoutManager.canScrollHorizontally()) {
        return findCenterView(layoutManager, getHorizontalHelper(layoutManager));
    }
    return nullptr;
}

int LinearSnapHelper::distanceToCenter(RecyclerView::LayoutManager& layoutManager,
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

int LinearSnapHelper::estimateNextPositionDiffForFling(RecyclerView::LayoutManager& layoutManager,
        OrientationHelper& helper, int velocityX, int velocityY) {
    int distances[2];
    calculateScrollDistance(velocityX, velocityY,distances);
    float distancePerChild = computeDistancePerChild(layoutManager, helper);
    if (distancePerChild <= 0) {
        return 0;
    }
    const int distance =
            std::abs(distances[0]) > std::abs(distances[1]) ? distances[0] : distances[1];
    return (int) std::round(distance / distancePerChild);
}

View* LinearSnapHelper::findCenterView(RecyclerView::LayoutManager& layoutManager,
        OrientationHelper& helper) {
    int childCount = layoutManager.getChildCount();
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

float LinearSnapHelper::computeDistancePerChild(RecyclerView::LayoutManager& layoutManager,
        OrientationHelper& helper) {
    View* minPosView = nullptr;
    View* maxPosView = nullptr;
    int minPos = INT_MAX;//Integer.MAX_VALUE;
    int maxPos = INT_MIN;//Integer.MIN_VALUE;
    int childCount = layoutManager.getChildCount();
    if (childCount == 0) {
        return INVALID_DISTANCE;
    }

    for (int i = 0; i < childCount; i++) {
        View* child = layoutManager.getChildAt(i);
        const int pos = layoutManager.getPosition(child);
        if (pos == RecyclerView::NO_POSITION) {
            continue;
        }
        if (pos < minPos) {
            minPos = pos;
            minPosView = child;
        }
        if (pos > maxPos) {
            maxPos = pos;
            maxPosView = child;
        }
    }
    if (minPosView == nullptr || maxPosView == nullptr) {
        return INVALID_DISTANCE;
    }
    int start = std::min(helper.getDecoratedStart(minPosView),
            helper.getDecoratedStart(maxPosView));
    int end = std::max(helper.getDecoratedEnd(minPosView),
            helper.getDecoratedEnd(maxPosView));
    int distance = end - start;
    if (distance == 0) {
        return INVALID_DISTANCE;
    }
    return 1.f * distance / ((maxPos - minPos) + 1);
}

OrientationHelper& LinearSnapHelper::getVerticalHelper(RecyclerView::LayoutManager& layoutManager) {
    if (mVerticalHelper == nullptr || mVerticalHelper->getLayoutManager() != &layoutManager) {
        mVerticalHelper = OrientationHelper::createVerticalHelper(&layoutManager);
    }
    return *mVerticalHelper;
}

OrientationHelper& LinearSnapHelper::getHorizontalHelper(RecyclerView::LayoutManager& layoutManager) {
    if (mHorizontalHelper == nullptr || mHorizontalHelper->getLayoutManager() != &layoutManager) {
        mHorizontalHelper = OrientationHelper::createHorizontalHelper(&layoutManager);
    }
    return *mHorizontalHelper;
}

}/*endof namespace*/
