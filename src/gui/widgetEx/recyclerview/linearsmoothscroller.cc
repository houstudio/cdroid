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
#include <widgetEx/recyclerview/linearsmoothscroller.h>
namespace cdroid{

LinearSmoothScroller::LinearSmoothScroller(Context* context) {
    mDisplayMetrics = context->getDisplayMetrics();
    mLinearInterpolator = LinearInterpolator::Instance;
    mDecelerateInterpolator = DecelerateInterpolator::Instance;
    mTargetVectorUsable = false;
    mHasCalculatedMillisPerPixel = false;
}

LinearSmoothScroller::~LinearSmoothScroller(){
    //delete mLinearInterpolator;
    //delete mDecelerateInterpolator;
}

void LinearSmoothScroller::onStart() {

}

void LinearSmoothScroller::onTargetFound(View* targetView, RecyclerView::State& state, Action& action) {
    const int dx = calculateDxToMakeVisible(targetView, getHorizontalSnapPreference());
    const int dy = calculateDyToMakeVisible(targetView, getVerticalSnapPreference());
    const int distance = (int) std::sqrt(dx * dx + dy * dy);
    const int time = calculateTimeForDeceleration(distance);
    if (time > 0) {
        action.update(-dx, -dy, time, mDecelerateInterpolator);
    }
}

void LinearSmoothScroller::onSeekTargetStep(int dx, int dy, RecyclerView::State& state, Action& action) {
    // TODO(b/72745539): Is there ever a time when onSeekTargetStep should be called when
    // getChildCount returns 0?  Should this logic be extracted out of this method such that
    // this method is not called if getChildCount() returns 0?
    if (getChildCount() == 0) {
        stop();
        return;
    }
    //noinspection PointlessBooleanExpression
    if( mTargetVectorUsable/*TargetVector != nullptr*/ && ((mTargetVector.x * dx < 0 || mTargetVector.y * dy < 0))) {
        FATAL("Scroll happened in the opposite direction"
              " of the target. Some calculations are wrong");
    }
    mInterimTargetDx = clampApplyScroll(mInterimTargetDx, dx);
    mInterimTargetDy = clampApplyScroll(mInterimTargetDy, dy);

    if (mInterimTargetDx == 0 && mInterimTargetDy == 0) {
        updateActionForInterimTarget(action);
    } // everything is valid, keep going

}

void LinearSmoothScroller::onStop() {
    mInterimTargetDx = mInterimTargetDy = 0;
    mTargetVector.set(0,0);// = nullptr;
    mTargetVectorUsable = false;
}

float LinearSmoothScroller::calculateSpeedPerPixel(const DisplayMetrics& displayMetrics) const{
    return MILLISECONDS_PER_INCH / displayMetrics.densityDpi;
}

float LinearSmoothScroller::getSpeedPerPixel(){
    if (!mHasCalculatedMillisPerPixel) {
        mMillisPerPixel = calculateSpeedPerPixel(mDisplayMetrics);
        mHasCalculatedMillisPerPixel = true;
    }
    return mMillisPerPixel;
}

float LinearSmoothScroller::calculateSpeedPerPixel(DisplayMetrics& displayMetrics) {
    return MILLISECONDS_PER_INCH / displayMetrics.densityDpi;
}

int LinearSmoothScroller::calculateTimeForDeceleration(int dx) {
    // we want to cover same area with the linear interpolator for the first 10% of the
    // interpolation. After that, deceleration will take control.
    // area under curve (1-(1-x)^2) can be calculated as (1 - x/3) * x * x
    // which gives 0.100028 when x = .3356
    // this is why we divide linear scrolling time with .3356
    return  (int) std::ceil(calculateTimeForScrolling(dx) / .3356);
}

int LinearSmoothScroller::calculateTimeForScrolling(int dx) {
    // In a case where dx is very small, rounding may return 0 although dx > 0.
    // To avoid that issue, ceil the result so that if dx > 0, we'll always return positive
    // time.
    return (int) std::ceil(std::abs(dx) * getSpeedPerPixel());
}

int LinearSmoothScroller::getHorizontalSnapPreference() {
    return (mTargetVectorUsable==false) || (mTargetVector.x == 0) ? SNAP_TO_ANY :
            mTargetVector.x > 0 ? SNAP_TO_END : SNAP_TO_START;
}

int LinearSmoothScroller::getVerticalSnapPreference() {
    return (mTargetVectorUsable == false) || (mTargetVector.y == 0) ? SNAP_TO_ANY :
            mTargetVector.y > 0 ? SNAP_TO_END : SNAP_TO_START;
}

void LinearSmoothScroller::updateActionForInterimTarget(Action& action) {
    // find an interim target position
    PointF scrollVector;
    mTargetVectorUsable = computeScrollVectorForPosition(getTargetPosition(),scrollVector);
    if ((mTargetVectorUsable==false) || (scrollVector.x == 0 && scrollVector.y == 0)) {
        const int target = getTargetPosition();
        action.jumpTo(target);
        stop();
        return;
    }
    normalize(scrollVector);
    mTargetVector = scrollVector;

    mInterimTargetDx = (int) (TARGET_SEEK_SCROLL_DISTANCE_PX * scrollVector.x);
    mInterimTargetDy = (int) (TARGET_SEEK_SCROLL_DISTANCE_PX * scrollVector.y);
    const int time = calculateTimeForScrolling(TARGET_SEEK_SCROLL_DISTANCE_PX);
    // To avoid UI hiccups, trigger a smooth scroll to a distance little further than the
    // interim target. Since we track the distance travelled in onSeekTargetStep callback, it
    // won't actually scroll more than what we need.
    action.update((int) (mInterimTargetDx * TARGET_SEEK_EXTRA_SCROLL_RATIO),
            (int) (mInterimTargetDy * TARGET_SEEK_EXTRA_SCROLL_RATIO),
            (int) (time * TARGET_SEEK_EXTRA_SCROLL_RATIO), mLinearInterpolator);
}

int LinearSmoothScroller::clampApplyScroll(int tmpDt, int dt) {
    const int before = tmpDt;
    tmpDt -= dt;
    if (before * tmpDt <= 0) { // changed sign, reached 0 or was 0, reset
        return 0;
    }
    return tmpDt;
}

int LinearSmoothScroller::calculateDtToFit(int viewStart, int viewEnd, int boxStart, int boxEnd, int
        snapPreference) {
    int dtStart,dtEnd;
    switch (snapPreference) {
    case SNAP_TO_START:return boxStart - viewStart;
    case SNAP_TO_END:  return boxEnd - viewEnd;
    case SNAP_TO_ANY:
        dtStart = boxStart - viewStart;
        if (dtStart > 0) {
            return dtStart;
        }
        dtEnd = boxEnd - viewEnd;
        if (dtEnd < 0) {
            return dtEnd;
        }
        break;
    default:
        FATAL("snap preference should be one of the"
               " constants defined in SmoothScroller, starting with SNAP_");
    }
    return 0;
}

int LinearSmoothScroller::calculateDyToMakeVisible(View* view, int snapPreference) {
    RecyclerView::LayoutManager* layoutManager = getLayoutManager();
    if (layoutManager == nullptr || !layoutManager->canScrollVertically()) {
        return 0;
    }
    const RecyclerView::LayoutParams* params = (RecyclerView::LayoutParams*)view->getLayoutParams();
    const int top = layoutManager->getDecoratedTop(view) - params->topMargin;
    const int bottom = layoutManager->getDecoratedBottom(view) + params->bottomMargin;
    const int start = layoutManager->getPaddingTop();
    const int end = layoutManager->getHeight() - layoutManager->getPaddingBottom();
    return calculateDtToFit(top, bottom, start, end, snapPreference);
}

int LinearSmoothScroller::calculateDxToMakeVisible(View* view, int snapPreference) {
    RecyclerView::LayoutManager* layoutManager = getLayoutManager();
    if (layoutManager == nullptr || !layoutManager->canScrollHorizontally()) {
        return 0;
    }
    const RecyclerView::LayoutParams* params = (RecyclerView::LayoutParams*)view->getLayoutParams();
    const int left = layoutManager->getDecoratedLeft(view) - params->leftMargin;
    const int right = layoutManager->getDecoratedRight(view) + params->rightMargin;
    const int start = layoutManager->getPaddingLeft();
    const int end = layoutManager->getWidth() - layoutManager->getPaddingRight();
    return calculateDtToFit(left, right, start, end, snapPreference);
}

}/*endof namespace*/
