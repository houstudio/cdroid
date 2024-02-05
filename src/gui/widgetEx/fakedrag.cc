#include <widgetEx/viewpager2.h>
#include <widgetEx/fakedrag.h>
#include <widgetEx/recyclerview/recyclerview.h>
#include <widgetEx/scrolleventadapter.h>

namespace cdroid{
FakeDrag::FakeDrag(ViewPager2* viewPager, ScrollEventAdapter* scrollEventAdapter, RecyclerView* recyclerView) {
    mViewPager = viewPager;
    mScrollEventAdapter = scrollEventAdapter;
    mRecyclerView = recyclerView;
}

bool FakeDrag::isFakeDragging() {
    return mScrollEventAdapter->isFakeDragging();
}

bool FakeDrag::beginFakeDrag() {
    if (mScrollEventAdapter->isDragging()) {
        return false;
    }
    mRequestedDragDistance = mActualDraggedDistance = 0;
    mFakeDragBeginTime = SystemClock::uptimeMillis();
    beginFakeVelocityTracker();

    mScrollEventAdapter->notifyBeginFakeDrag();
    if (!mScrollEventAdapter->isIdle()) {
        // Stop potentially running settling animation
        mRecyclerView->stopScroll();
    }
    addFakeMotionEvent(mFakeDragBeginTime, MotionEvent::ACTION_DOWN, 0, 0);
    return true;
}

bool FakeDrag::fakeDragBy(float offsetPxFloat) {
    if (!mScrollEventAdapter->isFakeDragging()) {
        // Can happen legitimately if user started dragging during fakeDrag and app is still
        // sending fakeDragBy commands
        return false;
    }
    // Subtract the offset, because content scrolls in the opposite direction of finger motion
    mRequestedDragDistance -= offsetPxFloat;
    // Calculate amount of pixels to scroll ...
    int offsetPx = std::round(mRequestedDragDistance - mActualDraggedDistance);
    // ... and keep track of pixels scrolled so we don't get rounding errors
    mActualDraggedDistance += offsetPx;
    long time = SystemClock::uptimeMillis();

    const bool isHorizontal = mViewPager->getOrientation() == ViewPager2::ORIENTATION_HORIZONTAL;
    // Scroll deltas use pixels:
    const int offsetX = isHorizontal ? offsetPx : 0;
    const int offsetY = isHorizontal ? 0 : offsetPx;
    // Motion events get the raw float distance:
    const float x = isHorizontal ? mRequestedDragDistance : 0;
    const float y = isHorizontal ? 0 : mRequestedDragDistance;

    mRecyclerView->scrollBy(offsetX, offsetY);
    addFakeMotionEvent(time, MotionEvent::ACTION_MOVE, x, y);
    return true;
}

bool FakeDrag::endFakeDrag() {
    if (!mScrollEventAdapter->isFakeDragging()) {
        // Happens legitimately if user started dragging during fakeDrag
        return false;
    }

    mScrollEventAdapter->notifyEndFakeDrag();

    // Compute the velocity of the fake drag
    int pixelsPerSecond = 1000;
    VelocityTracker* velocityTracker = mVelocityTracker;
    velocityTracker->computeCurrentVelocity(pixelsPerSecond, mMaximumVelocity);
    int xVelocity = (int) velocityTracker->getXVelocity();
    int yVelocity = (int) velocityTracker->getYVelocity();
    // And fling or snap the ViewPager2 to its destination
    if (!mRecyclerView->fling(xVelocity, yVelocity)) {
        // Velocity too low, trigger snap to page manually
        mViewPager->snapToPage();
    }
    return true;
}

void FakeDrag::beginFakeVelocityTracker() {
    if (mVelocityTracker == nullptr) {
        mVelocityTracker = VelocityTracker::obtain();
        ViewConfiguration& configuration = ViewConfiguration::get(mViewPager->getContext());
        mMaximumVelocity = configuration.getScaledMaximumFlingVelocity();
    } else {
        mVelocityTracker->clear();
    }
}

void FakeDrag::addFakeMotionEvent(long time, int action, float x, float y) {
    MotionEvent* ev = MotionEvent::obtain(mFakeDragBeginTime, time, action, x, y, 0);
    mVelocityTracker->addMovement(*ev);
    ev->recycle();
}
}/*endof namespace*/
