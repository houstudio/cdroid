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
*/
#include <widgetEx/wear/swipedismisscontroller.h>
#include <widgetEx/wear/backbuttondismisscontroller.h>
#include <widgetEx/wear/swipedismisstransitionhelper.h>
namespace cdroid{

SwipeDismissController::SwipeDismissController(Context* context, DismissibleFrameLayout* layout)
    :DismissController(context, layout){

    ViewConfiguration& vc = ViewConfiguration::get(context);
    mSlop = vc.getScaledTouchSlop();
    mMinFlingVelocity = vc.getScaledMinimumFlingVelocity();
    mGestureThresholdPx = context->getDisplayMetrics().widthPixels * EDGE_SWIPE_THRESHOLD;

    mSwipeDismissTransitionHelper = new SwipeDismissTransitionHelper(context, layout);
}

SwipeDismissController::~SwipeDismissController(){
    delete mSwipeDismissTransitionHelper;
}

void SwipeDismissController::requestDisallowInterceptTouchEvent(bool disallowIntercept) {
    if (mLayout->getParent() != nullptr)  {
        mLayout->getParent()->requestDisallowInterceptTouchEvent(disallowIntercept);
    }
}

void SwipeDismissController::setDismissMinDragWidthRatio(float ratio) {
    mDismissMinDragWidthRatio = ratio;
}

float SwipeDismissController::getDismissMinDragWidthRatio() const{
    return mDismissMinDragWidthRatio;
}

bool SwipeDismissController::onInterceptTouchEvent(MotionEvent& ev) {
    checkGesture(ev);
    if (mBlockGesture) {
        return true;
    }
    // Offset because the view is translated during swipe, match X with raw X. Active touch
    // coordinates are mostly used by the velocity tracker, so offset it to match the raw
    // coordinates which is what is primarily used elsewhere.
    float offsetX = ev.getRawX() - ev.getX();
    float offsetY = 0.0f, dx = 0, x = 0,y = 0;
    int actionIndex = -1;
    int pointerIndex= -1;
    int pointerId   = -1;
    ev.offsetLocation(offsetX, offsetY);

    switch (ev.getActionMasked()) {
    case MotionEvent::ACTION_DOWN:
        resetSwipeDetectMembers();
        mDownX = ev.getRawX();
        mDownY = ev.getRawY();
        mActiveTouchId = ev.getPointerId(0);
        mSwipeDismissTransitionHelper->obtainVelocityTracker();
        mSwipeDismissTransitionHelper->getVelocityTracker()->addMovement(ev);
        break;

    case MotionEvent::ACTION_POINTER_DOWN:
        actionIndex = ev.getActionIndex();
        mActiveTouchId = ev.getPointerId(actionIndex);
        break;
    case MotionEvent::ACTION_POINTER_UP:
        actionIndex = ev.getActionIndex();
        pointerId = ev.getPointerId(actionIndex);
        if (pointerId == mActiveTouchId) {
            // This was our active pointer going up. Choose a new active pointer.
            int newActionIndex = actionIndex == 0 ? 1 : 0;
            mActiveTouchId = ev.getPointerId(newActionIndex);
        }
        break;

    case MotionEvent::ACTION_CANCEL:
    case MotionEvent::ACTION_UP:
        resetSwipeDetectMembers();
        break;

    case MotionEvent::ACTION_MOVE:
        if ((mSwipeDismissTransitionHelper->getVelocityTracker() == nullptr) || mDiscardIntercept) {
            break;
        }

        pointerIndex = ev.findPointerIndex(mActiveTouchId);
        if (pointerIndex == -1) {
            LOGE("Invalid pointer index: ignoring.");
            mDiscardIntercept = true;
            break;
        }
        dx = ev.getRawX() - mDownX;
        x = ev.getX(pointerIndex);
        y = ev.getY(pointerIndex);

        if ((dx != 0) && (mDownX >= mGestureThresholdPx) && canScroll(mLayout, false, dx, x, y)) {
            mDiscardIntercept = true;
            break;
        }
        updateSwiping(ev);
        break;
    }
    ev.offsetLocation(-offsetX, -offsetY);
    return (!mDiscardIntercept && mSwiping);
}

bool SwipeDismissController::canScrollHorizontally(int direction) {
    // This view can only be swiped horizontally from left to right - this means a negative
    // SCROLLING direction. We return false if the view is not visible to avoid capturing swipe
    // gestures when the view is hidden.
    return (direction < 0) && (mLayout->getVisibility() == View::VISIBLE);
}

bool SwipeDismissController::isPotentialSwipe(float dx, float dy) const{
    return (dx * dx) + (dy * dy) > mSlop * mSlop;
}

bool SwipeDismissController::onTouchEvent(MotionEvent& ev) {
    checkGesture(ev);
    if (mBlockGesture) {
        return true;
    }

    if (mSwipeDismissTransitionHelper->getVelocityTracker() == nullptr) {
        return false;
    }

    // Offset because the view is translated during swipe, match X with raw X. Active touch
    // coordinates are mostly used by the velocity tracker, so offset it to match the raw
    // coordinates which is what is primarily used elsewhere.
    float offsetX = ev.getRawX() - ev.getX();
    float offsetY = 0.0f;
    ev.offsetLocation(offsetX, offsetY);
    switch (ev.getActionMasked()) {
    case MotionEvent::ACTION_UP:
        updateDismiss(ev);
        // Fall through, don't update gesture tracker with the event for ACTION_CANCEL
    case MotionEvent::ACTION_CANCEL:
        if (mDismissed) {
            mSwipeDismissTransitionHelper->animateDismissal(mDismissListener);
        } else if (mSwiping
                // Only trigger animation if we had a MOVE event that would shift the
                // underlying view, otherwise the animation would be janky.
                && mLastX != INT_MIN) {
            mSwipeDismissTransitionHelper->animateRecovery(mDismissListener);
        }
        resetSwipeDetectMembers();
        break;
    case MotionEvent::ACTION_MOVE:
        mSwipeDismissTransitionHelper->getVelocityTracker()->addMovement(ev);
        mLastX = ev.getRawX();
        updateSwiping(ev);
        if (mSwiping) {
            mSwipeDismissTransitionHelper->onSwipeProgressChanged(ev.getRawX() - mDownX, ev);
            break;
        }
    }
    ev.offsetLocation(-offsetX, -offsetY);
    return true;
}

/** Resets internal members when canceling or finishing a given gesture. */
void SwipeDismissController::resetSwipeDetectMembers() {
    if (mSwipeDismissTransitionHelper->getVelocityTracker() != nullptr) {
        mSwipeDismissTransitionHelper->getVelocityTracker()->recycle();
    }
    mSwipeDismissTransitionHelper->resetVelocityTracker();
    mDownX = 0;
    mDownY = 0;
    mSwiping = false;
    mLastX = INT_MIN;
    mDismissed = false;
    mDiscardIntercept = false;
}

void SwipeDismissController::updateSwiping(MotionEvent& ev) {
    if (!mSwiping) {
        const float deltaX = ev.getRawX() - mDownX;
        const float deltaY = ev.getRawY() - mDownY;
        if (isPotentialSwipe(deltaX, deltaY)) {
            mSwiping = (deltaX > mSlop * 2) && (std::abs(deltaY) < std::abs(deltaX));
        } else {
            mSwiping = false;
        }
    }
}

void SwipeDismissController::updateDismiss(MotionEvent& ev) {
    float deltaX = ev.getRawX(0) - mDownX;
    // Don't add the motion event as an UP event would clear the velocity tracker
    VelocityTracker* velocityTracker = mSwipeDismissTransitionHelper->getVelocityTracker();
    velocityTracker->computeCurrentVelocity(VELOCITY_UNIT);
    float xVelocity = velocityTracker->getXVelocity();
    float yVelocity = velocityTracker->getYVelocity();
    if (mLastX == INT_MIN) {
        // If there's no changes to mLastX, we have only one point of data, and therefore no
        // velocity. Estimate velocity from just the up and down event in that case.
        xVelocity = deltaX / ((ev.getEventTime() - ev.getDownTime()) / 1000.f);
    }

    if (!mDismissed) {
        if ((deltaX > (mLayout->getWidth() * mDismissMinDragWidthRatio)
                && ev.getRawX(0) >= mLastX)
                || (xVelocity >= mMinFlingVelocity
                && xVelocity > std::abs(yVelocity)))  {
            mDismissed = true;
        }
    }
    // Check if the user tried to undo this.
    if (mDismissed && mSwiping) {
        // Check if the user's finger is actually flinging back to left
        if (xVelocity < -mMinFlingVelocity) {
            mDismissed = false;
        }
    }
}

bool SwipeDismissController::canScroll(View* v, bool checkV, float dx, float x, float y) {
    if (dynamic_cast<ViewGroup*>(v)) {
        ViewGroup* group = (ViewGroup*) v;
        const int scrollX = v->getScrollX();
        const int scrollY = v->getScrollY();
        const int count = group->getChildCount();
        for (int i = count - 1; i >= 0; i--) {
            View* child = group->getChildAt(i);
            if (x + scrollX >= child->getLeft()
                    && x + scrollX < child->getRight()
                    && y + scrollY >= child->getTop()
                    && y + scrollY < child->getBottom()
                    && canScroll(
                    child, true, dx, x + scrollX - child->getLeft(),
                    y + scrollY - child->getTop())) {
                return true;
            }
        }
    }

    return checkV && v->canScrollHorizontally((int) -dx);
}

void SwipeDismissController::checkGesture(MotionEvent& ev) {
    if (ev.getActionMasked() == MotionEvent::ACTION_DOWN) {
        mBlockGesture = mSwipeDismissTransitionHelper->isAnimating();
    }
}
}/*endof namespace*/
