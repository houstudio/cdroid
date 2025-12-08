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
#include <widget/viewdraghelper.h>
namespace cdroid{

class VDInterpolator:public Interpolator {
public:
    float getInterpolation(float t)const override{
        t -= 1.0f;
        return t * t * t * t * t + 1.0f;
    }
};

static VDInterpolator sInterpolator;

ViewDragHelper::ViewDragHelper(Context* context,ViewGroup* forParent,Callback* cb){
    LOGE_IF(forParent == nullptr||cb==nullptr,"Parent view & Callback view may not be null");

    mParentView = forParent;
    mCallback = cb;
    ViewConfiguration vc = ViewConfiguration::get(context);
    const float density = context->getDisplayMetrics().density;
    mEdgeSize = mDefaultEdgeSize = (int) (EDGE_SIZE * density + 0.5f);
    mTrackingEdges= 0;
    mCapturedView = nullptr;
    mPointersDown = INVALID_POINTER;

    mDragState = STATE_IDLE;
    mVelocityTracker = nullptr;
    mInterpolator = nullptr;
    mTouchSlop   = vc.getScaledTouchSlop();
    mMaxVelocity = vc.getScaledMaximumFlingVelocity();
    mMinVelocity = vc.getScaledMinimumFlingVelocity();
    mScroller = new OverScroller(context,&sInterpolator,true);
    mSetIdleRunnable = [this](){
        setDragState(STATE_IDLE);
    };
}

ViewDragHelper::~ViewDragHelper(){
    delete mScroller;
    delete mCallback;
    if( mVelocityTracker){
        mVelocityTracker->recycle();
    }
}

ViewDragHelper *ViewDragHelper::create(ViewGroup* forParent,Callback* cb) {
    return new ViewDragHelper(forParent->getContext(), forParent, cb);
}

ViewDragHelper* ViewDragHelper::create(ViewGroup* forParent, float sensitivity,Callback* cb) {
    ViewDragHelper* helper = create(forParent, cb);
    helper->mTouchSlop = (int) (helper->mTouchSlop * (1.f / sensitivity));
    return helper;
}

void ViewDragHelper::setMinVelocity(float minVel) {
    mMinVelocity = minVel;
}

float ViewDragHelper::getMinVelocity()const{
    return mMinVelocity;
}

int ViewDragHelper::getViewDragState()const{
    return mDragState;
}

void ViewDragHelper::setEdgeTrackingEnabled(int edgeFlags){
    mTrackingEdges = edgeFlags;
}

int ViewDragHelper::getEdgeSize()const{
    return mEdgeSize;
}

void ViewDragHelper::setEdgeSize(int size){
    mEdgeSize = size;
}

int ViewDragHelper::getDefaultEdgeSize()const{
    return mDefaultEdgeSize;
}

void ViewDragHelper::captureChildView(View* childView, int activePointerId) {
    LOGE_IF(childView->getParent() != mParentView,"captureChildView: parameter must be a descendant "
                 "of the ViewDragHelper's tracked parent view (%p:%d)",mParentView,mParentView->getId());

    mCapturedView = childView;
    mActivePointerId = activePointerId;
    mCallback->onViewCaptured(*childView, activePointerId);
    setDragState(STATE_DRAGGING);
}

View* ViewDragHelper::getCapturedView()const{
    return mCapturedView;
}

int ViewDragHelper::getActivePointerId()const{
    return mActivePointerId;
}

int ViewDragHelper::getTouchSlop()const{
    return mTouchSlop; 
}

void ViewDragHelper::cancel() {
    mActivePointerId = INVALID_POINTER;
    clearMotionHistory();

    if (mVelocityTracker != nullptr) {
        mVelocityTracker->recycle();
        mVelocityTracker = nullptr;
    }
}

/* {@link #cancel()}, but also abort all motion in progress and snap to the end of any
 * animation. */
void ViewDragHelper::abort() {
    cancel();
    if (mDragState == STATE_SETTLING) {
        const int oldX = mScroller->getCurrX();
        const int oldY = mScroller->getCurrY();
        mScroller->abortAnimation();
        const int newX = mScroller->getCurrX();
        const int newY = mScroller->getCurrY();
        mCallback->onViewPositionChanged(*mCapturedView, newX, newY, newX - oldX, newY - oldY);
    }
    mInterpolator = &sInterpolator;
    setDragState(STATE_IDLE);
}

bool ViewDragHelper::smoothSlideViewTo(View* child, int finalLeft, int finalTop){
    mCapturedView = child;
    mActivePointerId = INVALID_POINTER;

    const bool continueSliding = forceSettleCapturedViewAt(finalLeft, finalTop, 0, 0);
    if (!continueSliding && (mDragState == STATE_IDLE) && mCapturedView ) {
        // If we're in an IDLE state to begin with and aren't moving anywhere, we
        // end up having a non-null capturedView with an IDLE dragState
        mCapturedView = nullptr;
    }

    return continueSliding;
}

bool ViewDragHelper::smoothSlideViewTo(View* child, int finalLeft, int finalTop, int duration, Interpolator* interpolator) {
    mCapturedView = child;
    mActivePointerId = INVALID_POINTER;

    const bool continueSliding = forceSettleCapturedViewAt(finalLeft, finalTop, duration, interpolator);
    if (!continueSliding && (mDragState == STATE_IDLE) && (mCapturedView != nullptr)) {
        // If we're in an IDLE state to begin with and aren't moving anywhere, we
        // end up having a non-null capturedView with an IDLE dragState
        mCapturedView = nullptr;
    }

    return continueSliding;
}

bool ViewDragHelper::settleCapturedViewAt(int finalLeft, int finalTop) {
    LOGE_IF(!mReleaseInProgress,"Cannot settleCapturedViewAt outside of a call to Callback#onViewReleased");

    return forceSettleCapturedViewAt(finalLeft, finalTop,
            (int) mVelocityTracker->getXVelocity(mActivePointerId),
            (int) mVelocityTracker->getYVelocity(mActivePointerId));
}

bool ViewDragHelper::forceSettleCapturedViewAt(int finalLeft, int finalTop, int xvel, int yvel){
    const int startLeft = mCapturedView->getLeft();
    const int startTop = mCapturedView->getTop();
    const int dx = finalLeft - startLeft;
    const int dy = finalTop - startTop;

    if (dx == 0 && dy == 0) {
        // Nothing to do. Send callbacks, be done.
        mScroller->abortAnimation();
        setDragState(STATE_IDLE);
        return false;
    }

    const int duration = computeSettleDuration(mCapturedView, dx, dy, xvel, yvel);
    mScroller->startScroll(startLeft, startTop, dx, dy, duration);

    setDragState(STATE_SETTLING);
    return true;
}

bool ViewDragHelper::forceSettleCapturedViewAt(int finalLeft, int finalTop, int duration, Interpolator* interpolator) {
    const int startLeft = mCapturedView->getLeft();
    const int startTop = mCapturedView->getTop();
    const int dx = finalLeft - startLeft;
    const int dy = finalTop - startTop;

    if (dx == 0 && dy == 0) {
        // Nothing to do. Send callbacks, be done.
        mScroller->abortAnimation();
        setDragState(STATE_IDLE);
        return false;
    }

    // mScroller's interpolator delegates to mInterpolator, update it before start animation.
    if (interpolator != nullptr) {
        mInterpolator = interpolator;
    } else {
        mInterpolator = &sInterpolator;
    }
    mScroller->startScroll(startLeft, startTop, dx, dy, duration);

    setDragState(STATE_SETTLING);
    return true;
}

int ViewDragHelper::computeSettleDuration(View* child, int dx, int dy, int xvel, int yvel){
    xvel = clampMag(xvel, (int) mMinVelocity, (int) mMaxVelocity);
    yvel = clampMag(yvel, (int) mMinVelocity, (int) mMaxVelocity);
    const int absDx = std::abs(dx);
    const int absDy = std::abs(dy);
    const int absXVel = std::abs(xvel);
    const int absYVel = std::abs(yvel);
    const int addedVel = absXVel + absYVel;
    const int addedDistance = absDx + absDy;

    const float xweight = xvel != 0 ? (float) absXVel / addedVel :
                (float) absDx / addedDistance;
    const float yweight = yvel != 0 ? (float) absYVel / addedVel :
                (float) absDy / addedDistance;

    const int xduration = computeAxisDuration(dx, xvel, mCallback->getViewHorizontalDragRange(*child));
    const int yduration = computeAxisDuration(dy, yvel, mCallback->getViewVerticalDragRange(*child));

    return (int) (xduration * xweight + yduration * yweight);
}

int ViewDragHelper::computeAxisDuration(int delta, int velocity, int motionRange){
    if (delta == 0) return 0;

    const int width = mParentView->getWidth();
    const int halfWidth = width / 2;
    const float distanceRatio = std::min(1.f, (float) std::abs(delta) / width);
    const float distance = halfWidth + halfWidth  * distanceInfluenceForSnapDuration(distanceRatio);

    int duration;
    velocity = std::abs(velocity);
    if (velocity > 0) {
        duration = 4 * std::round(1000 * std::abs(distance / velocity));
    } else {
        const float range = (float) std::abs(delta) / motionRange;
        duration = (int) ((range + 1) * BASE_SETTLE_DURATION);
    }
    return std::min(duration, (int)MAX_SETTLE_DURATION);
}

int ViewDragHelper::clampMag(int value, int absMin, int absMax) {
    const int absValue = std::abs(value);
    if (absValue < absMin) return 0;
    if (absValue > absMax) return value > 0 ? absMax : -absMax;
    return value;
}

float ViewDragHelper::clampMag(float value, float absMin, float absMax) {
    const float absValue = std::abs(value);
    if (absValue < absMin) return 0;
    if (absValue > absMax) return value > 0 ? absMax : -absMax;
    return value;
}

float ViewDragHelper::distanceInfluenceForSnapDuration(float f) {
    f -= 0.5f; // center the values about 0.
    f *= 0.3f * (float) M_PI / 2.0f;
    return (float) std::sin(f);
}

void ViewDragHelper::flingCapturedView(int minLeft, int minTop, int maxLeft, int maxTop){
    LOGE_IF(!mReleaseInProgress,"Cannot flingCapturedView outside of a call to Callback#onViewReleased");

    mScroller->fling(mCapturedView->getLeft(), mCapturedView->getTop(),
            (int) mVelocityTracker->getXVelocity(mActivePointerId),
            (int) mVelocityTracker->getYVelocity(mActivePointerId),
            minLeft, maxLeft, minTop, maxTop);

    setDragState(STATE_SETTLING);
}

bool ViewDragHelper::continueSettling(bool deferCallbacks){
    if (mDragState == STATE_SETTLING) {
        bool keepGoing = mScroller->computeScrollOffset();
        const int x = mScroller->getCurrX();
        const int y = mScroller->getCurrY();
        const int dx = x - mCapturedView->getLeft();
        const int dy = y - mCapturedView->getTop();

        if (dx != 0) {
            mCapturedView->offsetLeftAndRight(dx);
        }
        if (dy != 0) {
            mCapturedView->offsetTopAndBottom(dy);
        }

        if (dx != 0 || dy != 0) {
            mCallback->onViewPositionChanged(*mCapturedView, x, y, dx, dy);
        }

        if (keepGoing && x == mScroller->getFinalX() && y == mScroller->getFinalY()) {
            // Close enough. The interpolator/scroller might think we're still moving
            // but the user sure doesn't.
            mScroller->abortAnimation();
            keepGoing = false;
        }

        if (!keepGoing) {
            if (deferCallbacks) {
                mParentView->post(mSetIdleRunnable);
            } else {
                setDragState(STATE_IDLE);
            }
        }
    }

    return mDragState == STATE_SETTLING;
}

void ViewDragHelper::dispatchViewReleased(float xvel, float yvel) {
    mReleaseInProgress = true;
    mCallback->onViewReleased(*mCapturedView, xvel, yvel);
    mReleaseInProgress = false;

    if (mDragState == STATE_DRAGGING) {
        // onViewReleased didn't call a method that would have changed this. Go idle.
        setDragState(STATE_IDLE);
    }
}

void ViewDragHelper::clearMotionHistory() {
    std::fill(mInitialMotionX.begin(),mInitialMotionX.end(),0);
    std::fill(mInitialMotionY.begin(),mInitialMotionY.end(),0);
    std::fill(mLastMotionX.begin(),mLastMotionX.end(),0);
    std::fill(mLastMotionY.begin(),mLastMotionY.end(),0);
    std::fill(mInitialEdgesTouched.begin(),mInitialEdgesTouched.end(),0);
    std::fill(mEdgeDragsInProgress.begin(),mEdgeDragsInProgress.end(),0);
    std::fill(mEdgeDragsLocked.begin(),mEdgeDragsLocked.end(),0);
}

void ViewDragHelper::clearMotionHistory(int pointerId) {
    if (mInitialMotionX.size()==0 || !isPointerDown(pointerId)) {
        return;
    }
    mInitialMotionX[pointerId] = 0;
    mInitialMotionY[pointerId] = 0;
    mLastMotionX[pointerId] = 0;
    mLastMotionY[pointerId] = 0;
    mInitialEdgesTouched[pointerId] = 0;
    mEdgeDragsInProgress[pointerId] = 0;
    mEdgeDragsLocked[pointerId] = 0;
    mPointersDown &= ~(1 << pointerId);
}

void ViewDragHelper::ensureMotionHistorySizeForId(int pointerId) {
    if (mInitialMotionX.size() <= pointerId) {
        mInitialMotionX.resize(pointerId + 1);
        mInitialMotionY.resize(pointerId + 1);
        mLastMotionX.resize(pointerId + 1);
        mLastMotionY.resize(pointerId + 1);
        mInitialEdgesTouched.resize(pointerId + 1);
        mEdgeDragsInProgress.resize(pointerId + 1);
        mEdgeDragsLocked.resize(pointerId + 1);
    }
}

void ViewDragHelper::saveInitialMotion(float x, float y, int pointerId) {
    ensureMotionHistorySizeForId(pointerId);
    mInitialMotionX[pointerId] = mLastMotionX[pointerId] = x;
    mInitialMotionY[pointerId] = mLastMotionY[pointerId] = y;
    mInitialEdgesTouched[pointerId] = getEdgesTouched((int) x, (int) y);
    mPointersDown |= (1 << pointerId);
} 

void ViewDragHelper::saveLastMotion(MotionEvent& ev) {
    const size_t pointerCount = ev.getPointerCount();
    for (size_t i = 0; i < pointerCount; i++) {
        const int pointerId = ev.getPointerId(i);
        // If pointer is invalid then skip saving on ACTION_MOVE.
        if (!isValidPointerForActionMove(pointerId)) {
            continue;
        }
        const float x = ev.getX(i);
        const float y = ev.getY(i);
        mLastMotionX[pointerId] = x;
        mLastMotionY[pointerId] = y;
    }
}

bool ViewDragHelper::isPointerDown(int pointerId)const{
    return (mPointersDown & (1 << pointerId)) != 0;
}


void ViewDragHelper::setDragState(int state) {
    mParentView->removeCallbacks(mSetIdleRunnable);
    if (mDragState != state) {
        mDragState = state;
        mCallback->onViewDragStateChanged(state);
        if (mDragState == STATE_IDLE) {
            mCapturedView = nullptr;
        }
    }
}

bool ViewDragHelper::tryCaptureViewForDrag(View* toCapture, int pointerId){
    if (toCapture == mCapturedView && mActivePointerId == pointerId) {
        // Already done!
        return true;
    }
    if (toCapture && mCallback->tryCaptureView(*toCapture, pointerId)) {
        mActivePointerId = pointerId;
        captureChildView(toCapture, pointerId);
        return true;
    }
    return false;
}

bool ViewDragHelper::canScroll(View* v, bool checkV, int dx, int dy, int x, int y){
    if ( dynamic_cast<ViewGroup*>(v)) {
        ViewGroup* group = (ViewGroup*) v;
        const int scrollX = v->getScrollX();
        const int scrollY = v->getScrollY();
        const int count = group->getChildCount();
        // Count backwards - let topmost views consume scroll distance first.
        for (int i = count - 1; i >= 0; i--) {
            // TODO: Add versioned support here for transformed views.
            // This will not work for transformed views in Honeycomb+
            View* child = group->getChildAt(i);
            if ((x + scrollX >= child->getLeft()) && (x + scrollX < child->getRight())
                    && (y + scrollY >= child->getTop()) && (y + scrollY < child->getBottom())
                    && canScroll(child, true, dx, dy, x + scrollX - child->getLeft(),
                        y + scrollY - child->getTop())) {
                return true;
            }
        }
    }

    return checkV && (v->canScrollHorizontally(-dx) || v->canScrollVertically(-dy));
}

bool ViewDragHelper::shouldInterceptTouchEvent(MotionEvent& ev){
    const int action = ev.getActionMasked();
    const int actionIndex = ev.getActionIndex();

    if (action == MotionEvent::ACTION_DOWN) {
        // Reset things for a new event stream, just in case we didn't get
        // the whole previous stream.
        cancel();
    }

    if (mVelocityTracker == nullptr) {
        mVelocityTracker = VelocityTracker::obtain();
    }
    mVelocityTracker->addMovement(ev);

    switch (action) {
    case MotionEvent::ACTION_DOWN: {
        const float x = ev.getX();
        const float y = ev.getY();
        const int pointerId = ev.getPointerId(0);
        saveInitialMotion(x, y, pointerId);
        View* toCapture = findTopChildUnder((int) x, (int) y);

        // Catch a settling view if possible.
        if (toCapture == mCapturedView && mDragState == STATE_SETTLING) {
           tryCaptureViewForDrag(toCapture, pointerId);
        }

        const int edgesTouched = mInitialEdgesTouched[pointerId];
        if ((edgesTouched & mTrackingEdges) != 0) {
            mCallback->onEdgeTouched(edgesTouched & mTrackingEdges, pointerId);
        }
        break;
    }
    case MotionEvent::ACTION_POINTER_DOWN: {
        const int pointerId = ev.getPointerId(actionIndex);
        const float x = ev.getX(actionIndex);
        const float y = ev.getY(actionIndex);

        saveInitialMotion(x, y, pointerId);
        // A ViewDragHelper can only manipulate one view at a time.
        if (mDragState == STATE_IDLE) {
            const int edgesTouched = mInitialEdgesTouched[pointerId];
            if ((edgesTouched & mTrackingEdges) != 0) {
                mCallback->onEdgeTouched(edgesTouched & mTrackingEdges, pointerId);
            }
        } else if (mDragState == STATE_SETTLING) {
            // Catch a settling view if possible.
            View* toCapture = findTopChildUnder((int) x, (int) y);
            if (toCapture == mCapturedView) {
                tryCaptureViewForDrag(toCapture, pointerId);
            }
        }
        break;
    }

    case MotionEvent::ACTION_MOVE: {
        if (mInitialMotionX.size()==0 || mInitialMotionY.size()==0)break;
        // First to cross a touch slop over a draggable view wins. Also report edge drags.
        const int pointerCount = ev.getPointerCount();
        for (int i = 0; i < pointerCount; i++) {
            const int pointerId = ev.getPointerId(i);
            // If pointer is invalid then skip the ACTION_MOVE.
            if (!isValidPointerForActionMove(pointerId)) continue;

            const float x = ev.getX(i);
            const float y = ev.getY(i);
            const float dx = x - mInitialMotionX[pointerId];
            const float dy = y - mInitialMotionY[pointerId];
            View* toCapture = findTopChildUnder((int) x, (int) y);
            const bool pastSlop = toCapture && checkTouchSlop(toCapture, dx, dy);
            if (pastSlop) {
                // check the callback's
                // getView[Horizontal|Vertical]DragRange methods to know
                // if you can move at all along an axis, then see if it
                // would clamp to the same value. If you can't move at
                // all in every dimension with a nonzero range, bail.
                const int oldLeft = toCapture->getLeft();
                const int targetLeft = oldLeft + (int) dx;
                const int newLeft = mCallback->clampViewPositionHorizontal(*toCapture,targetLeft, (int) dx);
                const int oldTop = toCapture->getTop();
                const int targetTop = oldTop + (int) dy;
                const int newTop = mCallback->clampViewPositionVertical(*toCapture, targetTop,(int) dy);
                const int hDragRange = mCallback->getViewHorizontalDragRange(*toCapture);
                const int vDragRange = mCallback->getViewVerticalDragRange(*toCapture);
                if ((hDragRange == 0 || (hDragRange > 0 && newLeft == oldLeft))
                        && (vDragRange == 0 || (vDragRange > 0 && newTop == oldTop))) {
                    break;
                }
            }
            reportNewEdgeDrags(dx, dy, pointerId);
            if (mDragState == STATE_DRAGGING) {
                // Callback might have started an edge drag
                break;
            }

            if (pastSlop && tryCaptureViewForDrag(toCapture, pointerId)) {
                break;
            }
        }
        saveLastMotion(ev);
        break;
        }

    case MotionEvent::ACTION_POINTER_UP: {
        const int pointerId = ev.getPointerId(actionIndex);
        clearMotionHistory(pointerId);
        break;
        }
    case MotionEvent::ACTION_UP:
    case MotionEvent::ACTION_CANCEL: {
        cancel();
        break;
        }
    }

    return mDragState == STATE_DRAGGING;
}

void ViewDragHelper::processTouchEvent(MotionEvent& ev){
    const int action = ev.getActionMasked();
    const int actionIndex = ev.getActionIndex();

    if (action == MotionEvent::ACTION_DOWN) {
        // Reset things for a new event stream, just in case we didn't get
        // the whole previous stream.
        cancel();
    }

    if (mVelocityTracker == nullptr) {
        mVelocityTracker = VelocityTracker::obtain();
    }
    mVelocityTracker->addMovement(ev);

    switch (action) {
    case MotionEvent::ACTION_DOWN: {
        const float x = ev.getX();
        const float y = ev.getY();
        const int pointerId = ev.getPointerId(0);
        View* toCapture = findTopChildUnder((int) x, (int) y);

        saveInitialMotion(x, y, pointerId);

        // Since the parent is already directly processing this touch event,
        // there is no reason to delay for a slop before dragging.
        // Start immediately if possible.
        tryCaptureViewForDrag(toCapture, pointerId);

        const int edgesTouched = mInitialEdgesTouched[pointerId];
        if ((edgesTouched & mTrackingEdges) != 0) {
            mCallback->onEdgeTouched(edgesTouched & mTrackingEdges, pointerId);
        }
        break;
    }

    case MotionEvent::ACTION_POINTER_DOWN: {
        const int pointerId = ev.getPointerId(actionIndex);
        const float x = ev.getX(actionIndex);
        const float y = ev.getY(actionIndex);

        saveInitialMotion(x, y, pointerId);

        // A ViewDragHelper can only manipulate one view at a time.
        if (mDragState == STATE_IDLE) {
            // If we're idle we can do anything! Treat it like a normal down event.

            View* toCapture = findTopChildUnder((int) x, (int) y);
            tryCaptureViewForDrag(toCapture, pointerId);

            const int edgesTouched = mInitialEdgesTouched[pointerId];
            if ((edgesTouched & mTrackingEdges) != 0) {
                mCallback->onEdgeTouched(edgesTouched & mTrackingEdges, pointerId);
            }
        } else if (isCapturedViewUnder((int) x, (int) y)) {
            // We're still tracking a captured view. If the same view is under this
            // point, we'll swap to controlling it with this pointer instead.
            // (This will still work if we're "catching" a settling view.)

            tryCaptureViewForDrag(mCapturedView, pointerId);
        }
        break;
    }

    case MotionEvent::ACTION_MOVE:
        if (mDragState == STATE_DRAGGING) {
            // If pointer is invalid then skip the ACTION_MOVE.
            if (!isValidPointerForActionMove(mActivePointerId)) break;

            const int index = ev.findPointerIndex(mActivePointerId);
            const float x = ev.getX(index);
            const float y = ev.getY(index);
            const int idx = (int) (x - mLastMotionX[mActivePointerId]);
            const int idy = (int) (y - mLastMotionY[mActivePointerId]);

            dragTo(mCapturedView->getLeft() + idx, mCapturedView->getTop() + idy, idx, idy);

            saveLastMotion(ev);
        } else {
            // Check to see if any pointer is now over a draggable view.
            const size_t pointerCount = ev.getPointerCount();
            for (size_t i = 0; i < pointerCount; i++) {
                const int pointerId = ev.getPointerId(i);

                // If pointer is invalid then skip the ACTION_MOVE.
                if (!isValidPointerForActionMove(pointerId)) continue;

                const float x = ev.getX(i);
                const float y = ev.getY(i);
                const float dx = x - mInitialMotionX[pointerId];
                const float dy = y - mInitialMotionY[pointerId];

                reportNewEdgeDrags(dx, dy, pointerId);
                if (mDragState == STATE_DRAGGING) {
                    // Callback might have started an edge drag.
                    break;
                }

                View* toCapture = findTopChildUnder((int) x, (int) y);
                if (checkTouchSlop(toCapture, dx, dy)
                        && tryCaptureViewForDrag(toCapture, pointerId)) {
                    break;
                }
            }
            saveLastMotion(ev);
        }
        break;

    case MotionEvent::ACTION_POINTER_UP: {
        const int pointerId = ev.getPointerId(actionIndex);
        if (mDragState == STATE_DRAGGING && pointerId == mActivePointerId) {
            // Try to find another pointer that's still holding on to the captured view.
            int newActivePointer = INVALID_POINTER;
            const size_t pointerCount = ev.getPointerCount();
            for (size_t i = 0; i < pointerCount; i++) {
                const int id = ev.getPointerId(i);
                if (id == mActivePointerId) {
                    // This one's going away, skip.
                    continue;
                }

                const float x = ev.getX(i);
                const float y = ev.getY(i);
                if (findTopChildUnder((int) x, (int) y) == mCapturedView
                        && tryCaptureViewForDrag(mCapturedView, id)) {
                    newActivePointer = mActivePointerId;
                    break;
                }
            }

            if (newActivePointer == INVALID_POINTER) {
                // We didn't find another pointer still touching the view, release it.
                releaseViewForPointerUp();
            }
        }
        clearMotionHistory(pointerId);
        break;
    }

    case MotionEvent::ACTION_UP:
        if (mDragState == STATE_DRAGGING) {
            releaseViewForPointerUp();
        }
        cancel();
        break;

    case MotionEvent::ACTION_CANCEL:
        if (mDragState == STATE_DRAGGING) {
            dispatchViewReleased(0, 0);
        }
        cancel();
        break;
    }
}


void ViewDragHelper::reportNewEdgeDrags(float dx, float dy, int pointerId){
    int dragsStarted = 0;
    if (checkNewEdgeDrag(dx, dy, pointerId, EDGE_LEFT)) {
        dragsStarted |= EDGE_LEFT;
    }
    if (checkNewEdgeDrag(dy, dx, pointerId, EDGE_TOP)) {
        dragsStarted |= EDGE_TOP;
    }
    if (checkNewEdgeDrag(dx, dy, pointerId, EDGE_RIGHT)) {
        dragsStarted |= EDGE_RIGHT;
    }
    if (checkNewEdgeDrag(dy, dx, pointerId, EDGE_BOTTOM)) {
        dragsStarted |= EDGE_BOTTOM;
    }

    if (dragsStarted != 0) {
        mEdgeDragsInProgress[pointerId] |= dragsStarted;
        mCallback->onEdgeDragStarted(dragsStarted, pointerId);
    }
}

bool ViewDragHelper::checkNewEdgeDrag(float delta, float odelta, int pointerId, int edge){
    const float absDelta  = std::abs(delta);
    const float absODelta = std::abs(odelta);

    if ((mInitialEdgesTouched[pointerId] & edge) != edge  || (mTrackingEdges & edge) == 0
            || (mEdgeDragsLocked[pointerId] & edge) == edge
            || (mEdgeDragsInProgress[pointerId] & edge) == edge
            || (absDelta <= mTouchSlop && absODelta <= mTouchSlop)) {
        return false;
    }
    if (absDelta < absODelta * 0.5f && mCallback->onEdgeLock(edge)) {
        mEdgeDragsLocked[pointerId] |= edge;
        return false;
    }
    return (mEdgeDragsInProgress[pointerId] & edge) == 0 && absDelta > mTouchSlop;
}

bool ViewDragHelper::checkTouchSlop(View* child, float dx, float dy){
    if (child == nullptr) return false;
     
    const bool checkHorizontal= mCallback->getViewHorizontalDragRange(*child) > 0;
    const bool checkVertical  = mCallback->getViewVerticalDragRange(*child) > 0;

    if (checkHorizontal && checkVertical) {
        return dx * dx + dy * dy > mTouchSlop * mTouchSlop;
    } else if (checkHorizontal) {
        return std::abs(dx) > mTouchSlop;
    } else if (checkVertical) {
        return std::abs(dy) > mTouchSlop;
    }
    return false;
}

bool ViewDragHelper::checkTouchSlop(int directions){
    const int count = (int)mInitialMotionX.size();
    for (int i = 0; i < count; i++) {
        if (checkTouchSlop(directions, i)) {
            return true;
        }
    }
    return false;
}

bool ViewDragHelper::checkTouchSlop(int directions, int pointerId){
    if (!isPointerDown(pointerId)) return false;

    const bool checkHorizontal = (directions & DIRECTION_HORIZONTAL) == DIRECTION_HORIZONTAL;
    const bool checkVertical = (directions & DIRECTION_VERTICAL) == DIRECTION_VERTICAL;

    const float dx = mLastMotionX[pointerId] - mInitialMotionX[pointerId];
    const float dy = mLastMotionY[pointerId] - mInitialMotionY[pointerId];

    if (checkHorizontal && checkVertical) {
        return dx * dx + dy * dy > mTouchSlop * mTouchSlop;
    } else if (checkHorizontal) {
        return std::abs(dx) > mTouchSlop;
    } else if (checkVertical) {
        return std::abs(dy) > mTouchSlop;
    }
    return false;    
}

bool ViewDragHelper::isEdgeTouched(int edges)const{
    const int count = (int)mInitialEdgesTouched.size();
    for (int i = 0; i < count; i++) {
        if (isEdgeTouched(edges, i)) {
            return true;
        }
    }
    return false;
}

bool ViewDragHelper::isEdgeTouched(int edges, int pointerId)const{
    return isPointerDown(pointerId) && (mInitialEdgesTouched[pointerId] & edges) != 0;
}

void ViewDragHelper::releaseViewForPointerUp() {
    mVelocityTracker->computeCurrentVelocity(1000, mMaxVelocity);
    const float xvel = clampMag(
            mVelocityTracker->getXVelocity(mActivePointerId),
            mMinVelocity, mMaxVelocity);
    const float yvel = clampMag(
            mVelocityTracker->getYVelocity(mActivePointerId),
            mMinVelocity, mMaxVelocity);
    dispatchViewReleased(xvel, yvel);
}

void ViewDragHelper::dragTo(int left, int top, int dx, int dy) {
    int clampedX = left;
    int clampedY = top;
    const int oldLeft = mCapturedView->getLeft();
    const int oldTop  = mCapturedView->getTop();
    if (dx != 0) {
        clampedX = mCallback->clampViewPositionHorizontal(*mCapturedView, left, dx);
        mCapturedView->offsetLeftAndRight(clampedX - oldLeft);
    }
    if (dy != 0) {
        clampedY = mCallback->clampViewPositionVertical(*mCapturedView, top, dy);
        mCapturedView->offsetTopAndBottom(clampedY - oldTop);
    }

    if (dx != 0 || dy != 0) {
        const int clampedDx = clampedX - oldLeft;
        const int clampedDy = clampedY - oldTop;
        mCallback->onViewPositionChanged(*mCapturedView, clampedX, clampedY,
                clampedDx, clampedDy);
    }
}

bool ViewDragHelper::isCapturedViewUnder(int x, int y)const{
    return isViewUnder(mCapturedView, x, y);
}

bool ViewDragHelper::isViewUnder(View* view, int x, int y)const{
    return view && (x >= view->getLeft()) && (x < view->getRight())
                && (y >= view->getTop()) && (y < view->getBottom());
}

View* ViewDragHelper::findTopChildUnder(int x, int y) {
    const int childCount = mParentView->getChildCount();
    for (int i = childCount - 1; i >= 0; i--) {
        View* child = mParentView->getChildAt(mCallback->getOrderedChildIndex(i));
        if (x >= child->getLeft() && x < child->getRight()
                && y >= child->getTop() && y < child->getBottom()) {
            return child;
        }
    }
    return nullptr;
}

int ViewDragHelper::getEdgesTouched(int x, int y)const{
    int result = 0;

    if (x < mParentView->getLeft() + mEdgeSize) result |= EDGE_LEFT;
    if (y < mParentView->getTop() + mEdgeSize) result |= EDGE_TOP;
    if (x > mParentView->getRight() - mEdgeSize) result |= EDGE_RIGHT;
    if (y > mParentView->getBottom() - mEdgeSize) result |= EDGE_BOTTOM;

    return result;
}

bool ViewDragHelper::isValidPointerForActionMove(int pointerId)const{
    if(!isPointerDown(pointerId)){
        LOGE("Ignoring pointerId= %d because ACTION_DOWN was not received "
                    "for this pointer before ACTION_MOVE. It likely happened because "
                    " ViewDragHelper did not receive all the events in the event stream.",pointerId);
        return false;
    }
    return true;
}

void ViewDragHelper::Callback::onViewDragStateChanged(int state){
}

void ViewDragHelper::Callback::onViewPositionChanged(View& changedView, int left, int top,int dx,int dy){
}

void ViewDragHelper::Callback::onViewCaptured(View& capturedChild, int activePointerId){
}

void ViewDragHelper::Callback::onViewReleased(View&releasedChild,float xvel,float yvel){
}

void ViewDragHelper::Callback::onEdgeTouched(int edgeFlags, int pointerId){
}

bool ViewDragHelper::Callback::onEdgeLock(int edgeFlags){
    return false;
}

void ViewDragHelper::Callback::onEdgeDragStarted(int edgeFlags, int pointerId){
}

int ViewDragHelper::Callback::getOrderedChildIndex(int index){
    return index;
}

int ViewDragHelper::Callback::getViewHorizontalDragRange(View& child){
    return 0;
}

int ViewDragHelper::Callback::getViewVerticalDragRange(View& child){
    return 0;
}

int ViewDragHelper::Callback::clampViewPositionHorizontal(View& child, int left, int dx){
    return 0;
}

int ViewDragHelper::Callback::clampViewPositionVertical(View& child, int top, int dy){
    return 0;
}

}//endof namespace
