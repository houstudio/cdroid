/*********************************************************************************
 * Copyright (C) [2026] [houzh@msn.com]
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
#include <view/weargestureinterceptiondetector.h>
#include <view/motionevent.h>
#include <view/view.h>
#include <view/viewgroup.h>
#include <view/viewconfiguration.h>

namespace cdroid {

// WearGestureInterceptionDetector.java:53-57
WearGestureInterceptionDetector::WearGestureInterceptionDetector(Context* context, View* installedDecorView) {
    mTouchSlop = ViewConfiguration::get(context).getScaledTouchSlop();
    mInstalledDecorView = installedDecorView;
    mSwipingStartThreshold = mTouchSlop * 2;
}

// WearGestureInterceptionDetector.java:77-86
int WearGestureInterceptionDetector::getIndexForValidPointer(MotionEvent& ev) {
    const int pointerIndex = ev.findPointerIndex(mActivePointerId);
    if (pointerIndex == -1) {
        mDiscardIntercept = true;
    }
    return pointerIndex;
}

// WearGestureInterceptionDetector.java:88-98
void WearGestureInterceptionDetector::updateSwiping(MotionEvent& ev) {
    if (mSwiping) {
        return;
    }
    const float deltaX = ev.getRawX() - mDownX;
    const float deltaY = ev.getRawY() - mDownY;
    // Check if we have left the touch slop area.
    if ((deltaX * deltaX) + (deltaY * deltaY) > (mTouchSlop * mTouchSlop)) {
        mSwiping = deltaX > mSwipingStartThreshold && std::abs(deltaY) < std::abs(deltaX);
    }
}

// WearGestureInterceptionDetector.java:100-114
void WearGestureInterceptionDetector::updateDiscardIntercept(MotionEvent& ev, int pointerIndex) {
    if (!mSwiping) {
        // Don't look at canScroll until we have passed the touch slop
        return;
    }
    if (mDiscardIntercept) {
        return;
    }
    const bool checkLeft = mDownX < ev.getRawX();
    const float x = ev.getX(pointerIndex);
    const float y = ev.getY(pointerIndex);
    if (canScroll(mInstalledDecorView, false, checkLeft, x, y)) {
        mDiscardIntercept = true;
    }
}

// WearGestureInterceptionDetector.java:116-122
void WearGestureInterceptionDetector::resetMembers() {
    mDownX = 0;
    mDownY = 0;
    mSwiping = false;
    mDiscardIntercept = false;
}

// WearGestureInterceptionDetector.java:124-127
bool WearGestureInterceptionDetector::isIntercepting() const {
    return !mDiscardIntercept && mSwiping;
}

// WearGestureInterceptionDetector.java:129-167
bool WearGestureInterceptionDetector::onInterceptTouchEvent(MotionEvent& ev) {
    switch (ev.getActionMasked()) {
    case MotionEvent::ACTION_DOWN:
        resetMembers();
        mDownX = ev.getRawX();
        mDownY = ev.getRawY();
        mActivePointerId = ev.getPointerId(0);
        break;
    case MotionEvent::ACTION_POINTER_DOWN:
        mActivePointerId = ev.getPointerId(ev.getActionIndex());
        break;
    case MotionEvent::ACTION_POINTER_UP: {
        const int associatedPointerIndex = ev.getActionIndex();
        if (ev.getPointerId(associatedPointerIndex) == mActivePointerId) {
            // This was our active pointer going up.
            // Choose the first available pointer index.
            const int newActionIndex = associatedPointerIndex == 0 ? 1 : 0;
            mActivePointerId = ev.getPointerId(newActionIndex);
        }
        break;
    }
    case MotionEvent::ACTION_MOVE:
        if (mDiscardIntercept) {
            break;
        }
        {
            const int pointerIndex = getIndexForValidPointer(ev);
            if (pointerIndex == -1) {
                break;
            }
            updateSwiping(ev);
            updateDiscardIntercept(ev, pointerIndex);
        }
        break;
    case MotionEvent::ACTION_CANCEL:
    case MotionEvent::ACTION_UP:
        resetMembers();
        break;
    default:
        break;
    }
    return isIntercepting();
}

// WearGestureInterceptionDetector.java:169-210
bool WearGestureInterceptionDetector::canScroll(View* v, bool checkSelf, bool checkLeft, float x, float y) {
    if (dynamic_cast<ViewGroup*>(v) != nullptr) {
        ViewGroup* group = static_cast<ViewGroup*>(v);
        const int scrollX = v->getScrollX();
        const int scrollY = v->getScrollY();
        const int count = group->getChildCount();
        for (int i = count - 1; i >= 0; i--) {
            View* child = group->getChildAt(i);

            if (x + scrollX < child->getLeft()
                    || x + scrollX >= child->getRight()
                    || y + scrollY < child->getTop()
                    || y + scrollY >= child->getBottom()) {
                // This child is out of bound, don't bother checking.
                continue;
            }

            // Recursively check until finding the first scrollable or none is scrollable.
            if (canScroll(
                        /* view= */ child,
                        /* checkSelf= */ true,
                        /* checkLeft= */ checkLeft,
                        /* x= */ x + scrollX - child->getLeft(),
                        /* y= */ y + scrollY - child->getTop())) {
                return true;
            }
        }
    }

    return checkSelf && v->canScrollHorizontally(checkLeft ? -1 : 1);
}

} // namespace cdroid
