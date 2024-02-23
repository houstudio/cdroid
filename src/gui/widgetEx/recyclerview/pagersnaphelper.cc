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
    if (itemCount == 0) {
        return RecyclerView::NO_POSITION;
    }

    View* mStartMostChildView = nullptr;
    if (layoutManager.canScrollVertically()) {
        mStartMostChildView = findStartView(layoutManager, getVerticalHelper(layoutManager));
    } else if (layoutManager.canScrollHorizontally()) {
        mStartMostChildView = findStartView(layoutManager, getHorizontalHelper(layoutManager));
    }

    if (mStartMostChildView == nullptr) {
        return RecyclerView::NO_POSITION;
    }
    const int centerPosition = layoutManager.getPosition(mStartMostChildView);
    if (centerPosition == RecyclerView::NO_POSITION) {
        return RecyclerView::NO_POSITION;
    }

    bool forwardDirection;
    if (layoutManager.canScrollHorizontally()) {
        forwardDirection = velocityX > 0;
    } else {
        forwardDirection = velocityY > 0;
    }
    bool reverseLayout = false;
    PointF vectorForEnd;
    if (layoutManager.computeScrollVectorForPosition(itemCount - 1,&vectorForEnd)){
        reverseLayout = vectorForEnd.x < 0 || vectorForEnd.y < 0;
    }
    return reverseLayout
            ? (forwardDirection ? centerPosition - 1 : centerPosition)
            : (forwardDirection ? centerPosition + 1 : centerPosition);
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
    void onTargetFound(View* targetView, RecyclerView::State& state, Action& action) {
        int snapDistances[2];
	RecyclerView::LayoutManager*layoutManger = mRecyclerView->getLayoutManager();
	mSnapHelper->calculateDistanceToFinalSnap(*layoutManger,*targetView,snapDistances);
        int dx = snapDistances[0];
        int dy = snapDistances[1];
        int time = calculateTimeForDeceleration(std::max(std::abs(dx), std::abs(dy)));
        if (time > 0) {
            action.update(dx, dy, time, mDecelerateInterpolator);
        }
    }

    float calculateSpeedPerPixel(DisplayMetrics& displayMetrics) {
        return SnapHelper::MILLISECONDS_PER_INCH / displayMetrics.densityDpi;
    }

    int calculateTimeForScrolling(int dx) {
        return std::min((int)PagerSnapHelper::MAX_SCROLL_ON_FLING_DURATION,
		   LinearSmoothScroller::calculateTimeForScrolling(dx));
    }
};

LinearSmoothScroller* PagerSnapHelper::createSnapScroller(RecyclerView::LayoutManager& layoutManager) {
    if(!layoutManager.computeScrollVectorForPosition(0,nullptr)){
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
