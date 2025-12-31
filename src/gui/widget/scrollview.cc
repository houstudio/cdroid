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
#include <widget/scrollview.h>
#include <widget/R.h>
#include <view/focusfinder.h>
#include <view/accessibility/accessibilitynodeinfo.h>
#include <view/hapticscrollfeedbackprovider.h>
#include <porting/cdlog.h>
namespace cdroid {

DECLARE_WIDGET2(ScrollView,"cdroid:attr/scrollViewStyle")

ScrollView::ScrollView(int w,int h):FrameLayout(w,h){
    initScrollView();
    AttributeSet attrs;
    mIsBeingDragged=false;
    mActivePointerId=INVALID_POINTER;
}

ScrollView::ScrollView(Context*context,const AttributeSet&atts)
  :FrameLayout(context,atts){
    initScrollView();
    setFillViewport(atts.getBoolean("fillViewport", false));
    mScrollDuration = atts.getInt("scrollDuration",400);
}

ScrollView::~ScrollView(){
    recycleVelocityTracker();
    delete mSavedState;
    delete mHapticScrollFeedbackProvider;
    delete mScroller;
    delete mEdgeGlowTop;
    delete mEdgeGlowBottom;
}

bool ScrollView::shouldDelayChildPressedState(){
    return true;
}

float ScrollView::getTopFadingEdgeStrength() {
    if (getChildCount() == 0) {
        return 0.0f;
    }

    const int length = getVerticalFadingEdgeLength();
    if (mScrollY < length) {
        return mScrollY / (float) length;
    }

    return 1.0f;
}

float ScrollView::getBottomFadingEdgeStrength() {
    if (getChildCount() == 0) {
        return 0.0f;
    }

    const int length = getVerticalFadingEdgeLength();
    const int bottomEdge = getHeight() - mPaddingBottom;
    const int span = getChildAt(0)->getBottom() - mScrollY - bottomEdge;
    if (span < length) {
        return span / (float) length;
    }

    return 1.0f;
}

void ScrollView::setEdgeEffectColor(int color){
    setTopEdgeEffectColor(color);
    setBottomEdgeEffectColor(color);
}

void ScrollView::setBottomEdgeEffectColor(int color){
    mEdgeGlowBottom->setColor(color);
}

void ScrollView::setTopEdgeEffectColor(int color){
    mEdgeGlowTop->setColor(color);
}

int  ScrollView::getTopEdgeEffectColor()const{
    return mEdgeGlowTop->getColor();
}

int ScrollView::getBottomEdgeEffectColor()const{
    return mEdgeGlowBottom->getColor();
}

int ScrollView::getMaxScrollAmount() {
    return (int) (MAX_SCROLL_FACTOR * (mBottom-mTop));
}

void ScrollView::initScrollView() {
    mScroller = new OverScroller(getContext());
    setFocusable(true);
    setDescendantFocusability(FOCUS_AFTER_DESCENDANTS);
    setWillNotDraw(false);
    mFillViewport = false;
    mSmoothScrollingEnabled = true;
    mVelocityTracker = nullptr;
    mLastScroll = 0;
    mSavedState = nullptr;
    mEdgeGlowTop = new EdgeEffect(mContext);
    mEdgeGlowBottom = new EdgeEffect(mContext);
    ViewConfiguration& configuration = ViewConfiguration::get(mContext);
    mTouchSlop = configuration.getScaledTouchSlop();
    mMinimumVelocity = configuration.getScaledMinimumFlingVelocity();
    mMaximumVelocity = configuration.getScaledMaximumFlingVelocity();
    mOverscrollDistance= configuration.getScaledOverscrollDistance();
    mOverflingDistance = configuration.getScaledOverflingDistance();
    mVerticalScrollFactor= configuration.getScaledVerticalScrollFactor();
    mScrollOffset [0]  = mScrollOffset [1] = 0;
    mScrollConsumed[0] = mScrollConsumed[1]= 0;
    mScrollDuration = 400;
    mHapticScrollFeedbackProvider = nullptr;
}

void ScrollView::addView(View* child) {
    if (getChildCount() > 0) {
        LOGE("ScrollView can host only one direct child");
    }
    FrameLayout::addView(child);
}

void ScrollView::addView(View* child, int index) {
    if (getChildCount() > 0) {
        LOGE("ScrollView can host only one direct child");
    }
    FrameLayout::addView(child, index);
}

void ScrollView::addView(View* child, ViewGroup::LayoutParams* params) {
    if (getChildCount() > 0) {
        LOGE("ScrollView can host only one direct child");
    }
    FrameLayout::addView(child, params);
}

void ScrollView::addView(View* child, int index, ViewGroup::LayoutParams* params) {
    if (getChildCount() > 0) {
        LOGE("ScrollView can host only one direct child");
    }
    FrameLayout::addView(child, index, params);
}

bool ScrollView::canScroll() {
    const View* child = getChildAt(0);
    if (child != nullptr) {
        const int childHeight = child->getHeight();
        return getHeight() < childHeight + mPaddingTop + mPaddingBottom;
    }
    return false;
}

bool ScrollView::isFillViewport()const {
    return mFillViewport;
}

void ScrollView::setFillViewport(bool fillViewport) {
    if (fillViewport != mFillViewport) {
        mFillViewport = fillViewport;
        requestLayout();
    }
}

bool ScrollView::isSmoothScrollingEnabled()const {
    return mSmoothScrollingEnabled;
}

void ScrollView::setSmoothScrollingEnabled(bool smoothScrollingEnabled) {
    mSmoothScrollingEnabled = smoothScrollingEnabled;
}

void ScrollView::onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
    FrameLayout::onMeasure(widthMeasureSpec, heightMeasureSpec);

    if (!mFillViewport)  return;

    const int heightMode = MeasureSpec::getMode(heightMeasureSpec);
    if (heightMode == MeasureSpec::UNSPECIFIED) {
        return;
    }
    if (getChildCount() > 0) {
        View* child = getChildAt(0);
        const LayoutParams* lp = (const LayoutParams*) child->getLayoutParams();

        const int widthPadding = mPaddingLeft + mPaddingRight + lp->leftMargin + lp->rightMargin;
        const int heightPadding = mPaddingTop + mPaddingBottom + lp->topMargin + lp->bottomMargin;

        const int desiredHeight = getMeasuredHeight() - heightPadding;
        if (child->getMeasuredHeight() < desiredHeight) {
            const int childWidthMeasureSpec = getChildMeasureSpec(widthMeasureSpec, widthPadding, lp->width);
            const int childHeightMeasureSpec = MeasureSpec::makeMeasureSpec(desiredHeight, MeasureSpec::EXACTLY);
            child->measure(childWidthMeasureSpec, childHeightMeasureSpec);
        }
    }
}

bool ScrollView::dispatchKeyEvent(KeyEvent& event) {
    // Let the focused view and/or our descendants get the key first
    return FrameLayout::dispatchKeyEvent(event) || executeKeyEvent(event);
}

bool ScrollView::executeKeyEvent(KeyEvent& event) {
    if (!canScroll()) {
        if (isFocused() && (event.getKeyCode() !=KeyEvent::KEYCODE_BACK)
                &&(event.getKeyCode() != KeyEvent::KEYCODE_ESCAPE) ) {
            View* currentFocused = findFocus();
            if (currentFocused == this) currentFocused = nullptr;
            View* nextFocused = FocusFinder::getInstance().findNextFocus(this, currentFocused, View::FOCUS_DOWN);
            return nextFocused != nullptr  && nextFocused != this
                   && nextFocused->requestFocus(View::FOCUS_DOWN);
        }
        return false;
    }

    bool handled = false;
    if (event.getAction() == KeyEvent::ACTION_DOWN) {
        switch (event.getKeyCode()) {
        case KeyEvent::KEYCODE_DPAD_UP:
            if (!event.isAltPressed()) {
                handled = arrowScroll(View::FOCUS_UP);
            } else {
                handled = fullScroll(View::FOCUS_UP);
            }
            break;
        case KeyEvent::KEYCODE_DPAD_DOWN:
            if (!event.isAltPressed()) {
                handled = arrowScroll(View::FOCUS_DOWN);
            } else {
                handled = fullScroll(View::FOCUS_DOWN);
            }
            break;
        case KeyEvent::KEYCODE_SPACE:
            pageScroll(event.isShiftPressed() ? View::FOCUS_UP : View::FOCUS_DOWN);
            break;
        case KeyEvent::KEYCODE_MOVE_HOME:
            handled = fullScroll(View::FOCUS_UP);
            break;
        case KeyEvent::KEYCODE_MOVE_END:
            handled = fullScroll(View::FOCUS_DOWN);
            break;
        case KeyEvent::KEYCODE_PAGE_UP:
            handled = pageScroll(View::FOCUS_UP);
            break;
        case KeyEvent::KEYCODE_PAGE_DOWN:
            handled = pageScroll(View::FOCUS_DOWN);
            break;
        }
    }
    return handled;
}

bool ScrollView::inChild(int x, int y) {
    if (getChildCount() > 0) {
        const View* child = getChildAt(0);
        return !(y < child->getTop() - mScrollY
                 || y >= child->getBottom() - mScrollY
                 || x < child->getLeft()
                 || x >= child->getRight());
    }
    return false;
}

void ScrollView::initOrResetVelocityTracker() {
    if (mVelocityTracker == nullptr) {
        mVelocityTracker = VelocityTracker::obtain();
    } else {
        mVelocityTracker->clear();
    }
}

void ScrollView::initVelocityTrackerIfNotExists() {
    if (mVelocityTracker == nullptr) {
        mVelocityTracker = VelocityTracker::obtain();
    }
}

void ScrollView::initDifferentialFlingHelperIfNotExists() {
    /*if (mDifferentialMotionFlingHelper == nullptr) {
        mDifferentialMotionFlingHelper =
                new DifferentialMotionFlingHelper(
                        mContext, new DifferentialFlingTarget());
    }*/
}

void ScrollView::initHapticScrollFeedbackProviderIfNotExists() {
    if (mHapticScrollFeedbackProvider == nullptr) {
        mHapticScrollFeedbackProvider = new HapticScrollFeedbackProvider(this);
    }
}

void ScrollView::recycleVelocityTracker() {
    if (mVelocityTracker != nullptr) {
        mVelocityTracker->recycle();
        mVelocityTracker = nullptr;
    }
}

void ScrollView::requestDisallowInterceptTouchEvent(bool disallowIntercept) {
    if (disallowIntercept) {
        recycleVelocityTracker();
    }
    FrameLayout::requestDisallowInterceptTouchEvent(disallowIntercept);
}

bool ScrollView::onInterceptTouchEvent(MotionEvent& ev) {
    const int action = ev.getAction();
    if ((action == MotionEvent::ACTION_MOVE) && (mIsBeingDragged)) {
        return true;
    }

    if (FrameLayout::onInterceptTouchEvent(ev)) {
        return true;
    }

    /* Don't try to intercept touch if we can't scroll anyway. */
    if (getScrollY() == 0 && !canScrollVertically(1)) {
        return false;
    }

    switch (action & MotionEvent::ACTION_MASK) {
    case MotionEvent::ACTION_MOVE: {
        /* mIsBeingDragged == false, otherwise the shortcut would have caught it. Check
        * whether the user has moved far enough from his original down touch. */

        /* Locally do absolute value. mLastMotionY is set to the y value of the down event.*/
        const int activePointerId = mActivePointerId;
        if (activePointerId == INVALID_POINTER) {
            // If we don't have a valid id, the touch down wasn't on content.
            break;
        }

        const int pointerIndex = ev.findPointerIndex(activePointerId);
        if (pointerIndex == INVALID_POINTER) {
            LOGE("Invalid pointerId=%d  in onInterceptTouchEvent",activePointerId);
            break;
        }
        const int y = (int) ev.getY(pointerIndex);
        const int yDiff = std::abs(y - mLastMotionY);
        if (yDiff > mTouchSlop && (getNestedScrollAxes() & SCROLL_AXIS_VERTICAL) == 0) {
            mIsBeingDragged = true;
            mLastMotionY = y;
            initVelocityTrackerIfNotExists();
            mVelocityTracker->addMovement(ev);
            mNestedYOffset = 0;
            /*if (mScrollStrictSpan == nullptr) {
                mScrollStrictSpan = StrictMode.enterCriticalSpan("ScrollView-scroll");
            }*/
            ViewGroup* parent = getParent();
            if (parent) parent->requestDisallowInterceptTouchEvent(true);
        }
        break;
    }

    case MotionEvent::ACTION_DOWN: {
        int y = (int) ev.getY();
        if (!inChild((int) ev.getX(), (int) y)) {
            mIsBeingDragged = false;
            recycleVelocityTracker();
            break;
        }

        /*
         * Remember location of down touch.
         * ACTION_DOWN always refers to pointer index 0.
         */
        mLastMotionY = y;
        mActivePointerId = ev.getPointerId(0);

        initOrResetVelocityTracker();
        mVelocityTracker->addMovement(ev);
        /*
         * If being flinged and user touches the screen, initiate drag;
         * otherwise don't. mScroller.isFinished should be false when
         * being flinged. We need to call computeScrollOffset() first so that
         * isFinished() is correct.
        */
        mScroller->computeScrollOffset();
        mIsBeingDragged = !mScroller->isFinished()|| !mEdgeGlowBottom->isFinished()
                   || !mEdgeGlowTop->isFinished();
        // Catch the edge effect if it is active.
        if (!mEdgeGlowTop->isFinished()) {
             mEdgeGlowTop->onPullDistance(0.f, ev.getX() / getWidth());
        }
        if (!mEdgeGlowBottom->isFinished()) {
             mEdgeGlowBottom->onPullDistance(0.f, 1.f - ev.getX() / getWidth());
        }
        /*if (mIsBeingDragged && mScrollStrictSpan == nullptr) {
            mScrollStrictSpan = StrictMode.enterCriticalSpan("ScrollView-scroll");
        }*/
        startNestedScroll(SCROLL_AXIS_VERTICAL);
        break;
    }

    case MotionEvent::ACTION_CANCEL:
    case MotionEvent::ACTION_UP:
        /* Release the drag */
        mIsBeingDragged = false;
        mActivePointerId = INVALID_POINTER;
        recycleVelocityTracker();
        if (mScroller->springBack(mScrollX, mScrollY, 0, 0, 0, getScrollRange())) {
            postInvalidateOnAnimation();
        }
        stopNestedScroll();
        break;
    case MotionEvent::ACTION_POINTER_UP:
        onSecondaryPointerUp(ev);
        break;
    }

    /* The only time we want to intercept motion events is if we are in the drag mode. */
    return mIsBeingDragged;
}

bool ScrollView::shouldDisplayEdgeEffects()const{
    return getOverScrollMode() != OVER_SCROLL_NEVER;
}

bool ScrollView::onTouchEvent(MotionEvent& ev) {
    initVelocityTrackerIfNotExists();

    MotionEvent* vtev = MotionEvent::obtain(ev);

    const int actionMasked = ev.getActionMasked();

    if (actionMasked == MotionEvent::ACTION_DOWN) {
        mNestedYOffset = 0;
    }
    vtev->offsetLocation(0, mNestedYOffset);

    switch (actionMasked) {
    case MotionEvent::ACTION_DOWN:
        if (getChildCount() == 0) {
            return false;
        }
        if ((mIsBeingDragged = !mScroller->isFinished())) {
            ViewGroup* parent = getParent();
            if (parent) parent->requestDisallowInterceptTouchEvent(true);
        }

        // If being flinged and user touches, stop the fling. isFinished
        // will be false if being flinged.
        if (!mScroller->isFinished()) {
            mScroller->abortAnimation();
            /*if (mFlingStrictSpan != null) {
                 mFlingStrictSpan.finish();
                 mFlingStrictSpan = null;
            }*/
        }

        // Remember where the motion event started
        mLastMotionY = (int) ev.getY();
        mActivePointerId = ev.getPointerId(0);
        LOGV("ACTION_DOWN mActivePointerId=%d",mActivePointerId);
        startNestedScroll(SCROLL_AXIS_VERTICAL);
        break;
    case MotionEvent::ACTION_MOVE: {
        const int activePointerIndex = ev.findPointerIndex(mActivePointerId);
        if (activePointerIndex == -1) {
            LOGE("Invalid pointerId=%d in onTouchEvent",mActivePointerId);
            break;
        }

        const int y = (int) ev.getY(activePointerIndex);
        int deltaY = mLastMotionY - y;
        if (dispatchNestedPreScroll(0, deltaY, mScrollConsumed, mScrollOffset)) {
            deltaY -= mScrollConsumed[1];
            vtev->offsetLocation(0, mScrollOffset[1]);
            mNestedYOffset += mScrollOffset[1];
        }
        if (!mIsBeingDragged && std::abs(deltaY) > mTouchSlop) {
            ViewGroup* parent = getParent();
            if (parent) parent->requestDisallowInterceptTouchEvent(true);
            mIsBeingDragged = true;
            if (deltaY > 0) {
                deltaY -= mTouchSlop;
            } else {
                deltaY += mTouchSlop;
            }
        }
        bool hitTopLimit = false;
        bool hitBottomLimit= false;
        if (mIsBeingDragged) {
            // Scroll to follow the motion event
            mLastMotionY = y - mScrollOffset[1];

            const int oldY = mScrollY;
            const int range = getScrollRange();
            const int overscrollMode = getOverScrollMode();
            const bool canOverscroll = (overscrollMode == OVER_SCROLL_ALWAYS) ||
                        (overscrollMode == OVER_SCROLL_IF_CONTENT_SCROLLS && range > 0);

            const float displacement =ev.getX(activePointerIndex)/getWidth();
            if(canOverscroll){
                int consumed = 0;
                if (deltaY < 0 && mEdgeGlowBottom->getDistance() != 0.f) {
                    consumed = std::round(getHeight()
                               * mEdgeGlowBottom->onPullDistance((float) deltaY / getHeight(),
                               1.f - displacement));
                } else if (deltaY > 0 && mEdgeGlowTop->getDistance() != 0.f) {
                     consumed = std::round(-getHeight()
                               * mEdgeGlowTop->onPullDistance((float) -deltaY / getHeight(),
                                displacement));
                }
                deltaY -= consumed;
            }
            // Calling overScrollBy will call onOverScrolled, which
            // calls onScrollChanged if applicable.
            if (overScrollBy(0, deltaY, 0, mScrollY, 0, range, 0, mOverscrollDistance, true)
                    && !hasNestedScrollingParent()) {
                // Break our velocity if we hit a scroll barrier.
                mVelocityTracker->clear();
            }

            const int scrolledDeltaY = mScrollY - oldY;
            const int unconsumedY = deltaY - scrolledDeltaY;
            if (dispatchNestedScroll(0, scrolledDeltaY, 0, unconsumedY, mScrollOffset)) {
                mLastMotionY -= mScrollOffset[1];
                vtev->offsetLocation(0, mScrollOffset[1]);
                mNestedYOffset += mScrollOffset[1];
            } else if (canOverscroll && deltaY!=0.f) {
                const int pulledToY = oldY + deltaY;
                if (pulledToY < 0) {
                    mEdgeGlowTop->onPullDistance((float) -deltaY / getHeight(),displacement);
                    if (!mEdgeGlowBottom->isFinished()) mEdgeGlowBottom->onRelease();
                    hitTopLimit = true;
                } else if (pulledToY > range) {
                    mEdgeGlowBottom->onPullDistance((float) deltaY / getHeight(),1.f-displacement);
                    if(!mEdgeGlowTop->isFinished()) mEdgeGlowTop->onRelease();
                    hitBottomLimit =true;
                }
                if (shouldDisplayEdgeEffects() && (!mEdgeGlowTop->isFinished() || !mEdgeGlowBottom->isFinished())) {
                    postInvalidateOnAnimation();
                }
            }
        }
        // TODO: b/360198915 - Add unit tests.
        if (true/*enableScrollFeedbackForTouch()*/) {
            if (hitTopLimit || hitBottomLimit) {
                initHapticScrollFeedbackProviderIfNotExists();
                mHapticScrollFeedbackProvider->onScrollLimit(vtev->getDeviceId(),
                        vtev->getSource(), MotionEvent::AXIS_Y,
                        /* isStart= */ hitTopLimit);
            } else if (std::abs(deltaY) != 0) {
                initHapticScrollFeedbackProviderIfNotExists();
                mHapticScrollFeedbackProvider->onScrollProgress(vtev->getDeviceId(),
                        vtev->getSource(), MotionEvent::AXIS_Y, deltaY);
            }
        }
    } break;
    case MotionEvent::ACTION_UP:
        if (mIsBeingDragged) {
            VelocityTracker* velocityTracker = mVelocityTracker;
            velocityTracker->computeCurrentVelocity(1000, mMaximumVelocity);
            const int initialVelocity = (int) velocityTracker->getYVelocity(mActivePointerId);
            LOGV("initialVelocity=%d/%d/%d",initialVelocity,mMinimumVelocity,mMaximumVelocity);
            if ((std::abs(initialVelocity) > mMinimumVelocity)) {
                flingWithNestedDispatch(-initialVelocity);
            } else if (mScroller->springBack(mScrollX, mScrollY, 0, 0, 0,getScrollRange())) {
                postInvalidateOnAnimation();
            }
            mActivePointerId = INVALID_POINTER;
            endDrag();
            velocityTracker->clear();
        }
        break;
    case MotionEvent::ACTION_CANCEL:
        if (mIsBeingDragged && getChildCount() > 0) {
            if (mScroller->springBack(mScrollX, mScrollY, 0, 0, 0, getScrollRange())) {
                postInvalidateOnAnimation();
            }
            mActivePointerId = INVALID_POINTER;
            endDrag();
        }
        break;
    case MotionEvent::ACTION_POINTER_DOWN: {
        const int index = ev.getActionIndex();
        mLastMotionY = (int) ev.getY(index);
        mActivePointerId = ev.getPointerId(index);
        LOGV("POINTER_DOWN mActivePointerId=%d",mActivePointerId);
        break;
    }
    case MotionEvent::ACTION_POINTER_UP:
        onSecondaryPointerUp(ev);
        mLastMotionY = (int) ev.getY(ev.findPointerIndex(mActivePointerId));
        break;
    }

    if (mVelocityTracker) mVelocityTracker->addMovement(*vtev);
    vtev->recycle();
    return true;
}

void ScrollView::onSecondaryPointerUp(MotionEvent& ev) {
    const int pointerIndex = (ev.getAction() & MotionEvent::ACTION_POINTER_INDEX_MASK) >>
                       MotionEvent::ACTION_POINTER_INDEX_SHIFT;
    const int pointerId = ev.getPointerId(pointerIndex);
    if (pointerId == mActivePointerId) {
        // This was our active pointer going up. Choose a new
        // active pointer and adjust accordingly.
        // TODO: Make this decision more intelligent.
        const int newPointerIndex = pointerIndex == 0 ? 1 : 0;
        mLastMotionY = (int) ev.getY(newPointerIndex);
        mActivePointerId = ev.getPointerId(newPointerIndex);
        if (mVelocityTracker) mVelocityTracker->clear();
    }
}

bool ScrollView::onGenericMotionEvent(MotionEvent& event) {
    int  delta,axis;
    float axisValue;
    switch (event.getAction()) {
    case MotionEvent::ACTION_SCROLL:
        if (event.isFromSource(InputDevice::SOURCE_CLASS_POINTER)) {
            axis = MotionEvent::AXIS_VSCROLL;
        } else if (event.isFromSource(InputDevice::SOURCE_ROTARY_ENCODER)) {
            axis = MotionEvent::AXIS_SCROLL;
        } else {
            axis = -1;
        }
        axisValue = (axis==-1)?0:event.getAxisValue(axis,0);
        delta = std::round(axisValue * mVerticalScrollFactor);
        if (delta != 0) {
            // Tracks whether or not we should attempt fling for this event.
            // Fling should not be attempted if the view is already at the limit of scroll,
            // since it conflicts with EdgeEffect.
            bool hitLimit = false;
            const int range = getScrollRange();
            const int oldScrollY = mScrollY;
            int newScrollY = oldScrollY - delta;
            const int overscrollMode = getOverScrollMode();
            bool canOverscroll = !event.isFromSource(InputDevice::SOURCE_MOUSE)
                   && (overscrollMode == OVER_SCROLL_ALWAYS
                  || (overscrollMode == OVER_SCROLL_IF_CONTENT_SCROLLS && range > 0));
            bool absorbed = false;
            if (newScrollY < 0) {
                if (canOverscroll) {
                    mEdgeGlowTop->onPullDistance(-(float) newScrollY / getHeight(), 0.5f);
                    mEdgeGlowTop->onRelease();
                    invalidate();
                    absorbed = true;
                }
                newScrollY = 0;
                hitLimit = true;
            } else if (newScrollY > range) {
                if (canOverscroll) {
                    mEdgeGlowBottom->onPullDistance(
                          (float) (newScrollY - range) / getHeight(), 0.5f);
                    mEdgeGlowBottom->onRelease();
                    invalidate();
                    absorbed = true;
                }
                newScrollY = range;
            }
            if (newScrollY != oldScrollY) {
                FrameLayout::scrollTo(mScrollX,mScrollY);
                if(hitLimit){
                    initHapticScrollFeedbackProviderIfNotExists();
                    mHapticScrollFeedbackProvider->onScrollLimit(
                        event.getDeviceId(), event.getSource(), axis,
                        /* isStart= */ newScrollY == 0);
                }else{
                    initHapticScrollFeedbackProviderIfNotExists();
                    mHapticScrollFeedbackProvider->onScrollProgress(
                        event.getDeviceId(), event.getSource(), axis, delta);
                    initDifferentialFlingHelperIfNotExists();
                    //mDifferentialMotionFlingHelper->onMotionEvent(event, axis);
                }
                return true;
            }
            if(absorbed) return true;
        }
        break;
    }
    return FrameLayout::onGenericMotionEvent(event);
}

void ScrollView::onOverScrolled(int scrollX, int scrollY, bool clampedX, bool clampedY) {
    if (!mScroller->isFinished()) {
        const int oldX = mScrollX;
        const int oldY = mScrollY;
        mScrollX = scrollX;
        mScrollY = scrollY;
        invalidateParentIfNeeded();
        onScrollChanged(mScrollX, mScrollY, oldX, oldY);
        if (clampedY) {
            mScroller->springBack(mScrollX, mScrollY, 0, 0, 0, getScrollRange());
        }
    } else {
        FrameLayout::scrollTo(scrollX, scrollY);
    }
    awakenScrollBars();
}

bool ScrollView::performAccessibilityActionInternal(int action, Bundle* arguments){
    if (FrameLayout::performAccessibilityActionInternal(action, arguments)) {
        return true;
    }
    if (!isEnabled()) {
        return false;
    }
    switch (action) {
    case AccessibilityNodeInfo::ACTION_SCROLL_FORWARD:
    case R::id::accessibilityActionScrollDown: {
            const int viewportHeight = getHeight() - mPaddingBottom - mPaddingTop;
            const int targetScrollY = std::min(mScrollY + viewportHeight, getScrollRange());
            if (targetScrollY != mScrollY) {
                smoothScrollTo(0, targetScrollY);
                return true;
            }
        } return false;
    case AccessibilityNodeInfo::ACTION_SCROLL_BACKWARD:
    case R::id::accessibilityActionScrollUp: {
            const int viewportHeight = getHeight() - mPaddingBottom - mPaddingTop;
            const int targetScrollY = std::max(mScrollY - viewportHeight, 0);
            if (targetScrollY != mScrollY) {
                smoothScrollTo(0, targetScrollY);
                return true;
            }
        } return false;
    }
    return false;
}

std::string ScrollView::getAccessibilityClassName() const{
    return "ScrollView";
}

void ScrollView::onInitializeAccessibilityNodeInfoInternal(AccessibilityNodeInfo& info) {
    FrameLayout::onInitializeAccessibilityNodeInfoInternal(info);
    if (isEnabled()) {
        const int scrollRange = getScrollRange();
        if (scrollRange > 0) {
            info.setScrollable(true);
            if (mScrollY > 0) {
                info.addAction(AccessibilityNodeInfo::ACTION_SCROLL_BACKWARD);
                info.addAction(R::id::accessibilityActionScrollUp);//AccessibilityNodeInfo::AccessibilityAction::ACTION_SCROLL_UP);
            }
            if (mScrollY < scrollRange) {
                info.addAction(AccessibilityNodeInfo::ACTION_SCROLL_FORWARD);
                info.addAction(R::id::accessibilityActionScrollDown);//AccessibilityNodeInfo::AccessibilityAction::ACTION_SCROLL_DOWN);
            }
        }
    }
}
void ScrollView::onInitializeAccessibilityEventInternal(AccessibilityEvent& event) {
    FrameLayout::onInitializeAccessibilityEventInternal(event);
    const bool scrollable = getScrollRange() > 0;
    event.setScrollable(scrollable);
    event.setScrollX(mScrollX);
    event.setScrollY(mScrollY);
    event.setMaxScrollX(mScrollX);
    event.setMaxScrollY(getScrollRange());
}

int ScrollView::getScrollRange() {
    if (getChildCount() > 0) {
        View* child = getChildAt(0);
        return std::max(0,child->getHeight() - (getHeight() - mPaddingBottom - mPaddingTop));
    }
    return 0;
}

View* ScrollView::findFocusableViewInBounds(bool topFocus, int top, int bottom) {
    std::vector<View*> focusables = getFocusables(View::FOCUS_FORWARD);
    View* focusCandidate = nullptr;

    /*
     * A fully contained focusable is one where its top is below the bound's
     * top, and its bottom is above the bound's bottom. A partially
     * contained focusable is one where some part of it is within the
     * bounds, but it also has some part that is not within bounds.  A fully contained
     * focusable is preferred to a partially contained focusable.
     */
    bool foundFullyContainedFocusable = false;

    int count = focusables.size();
    for (int i = 0; i < count; i++) {
        View* view = focusables[i];
        int viewTop = view->getTop();
        int viewBottom = view->getBottom();

        if (top < viewBottom && viewTop < bottom) {
            /* the focusable is in the target area, it is a candidate for focusing */

            const bool viewIsFullyContained = (top < viewTop) && (viewBottom < bottom);

            if (focusCandidate == nullptr) {
                /* No candidate, take this one */
                focusCandidate = view;
                foundFullyContainedFocusable = viewIsFullyContained;
            } else {
                const bool viewIsCloserToBoundary =
                    (topFocus && viewTop < focusCandidate->getTop()) ||
                    (!topFocus && viewBottom > focusCandidate->getBottom());

                if (foundFullyContainedFocusable) {
                    if (viewIsFullyContained && viewIsCloserToBoundary) {
                        /* We're dealing with only fully contained views, so
                         * it has to be closer to the boundary to beat our candidate */
                        focusCandidate = view;
                    }
                } else {
                    if (viewIsFullyContained) {
                        /* Any fully contained view beats a partially contained view */
                        focusCandidate = view;
                        foundFullyContainedFocusable = true;
                    } else if (viewIsCloserToBoundary) {
                        /* Partially contained view beats another partially
                         * contained view if it's closer  */
                        focusCandidate = view;
                    }
                }
            }
        }
    }

    return focusCandidate;
}

bool ScrollView::pageScroll(int direction) {
    const bool down = direction == View::FOCUS_DOWN;
    const int height = getHeight();
    Rect mTempRect;
    if (down) {
        mTempRect.top = getScrollY() + height;
        const int count = getChildCount();
        if (count > 0) {
            View* view = getChildAt(count - 1);
            if (mTempRect.top + height > view->getBottom()) {
                mTempRect.top = view->getBottom() - height;
            }
        }
    } else {
        mTempRect.top = getScrollY() - height;
        if (mTempRect.top < 0) {
            mTempRect.top = 0;
        }
    }
    mTempRect.height = height;

    return scrollAndFocus(direction, mTempRect.top, mTempRect.bottom());
}

bool ScrollView::fullScroll(int direction) {
    const bool down = direction == View::FOCUS_DOWN;
    const int height = getHeight();
    Rect mTempRect;

    mTempRect.top = 0;
    mTempRect.height = height;

    if (down) {
        const int count = getChildCount();
        if (count > 0) {
            View* view = getChildAt(count - 1);
            mTempRect.top = view->getBottom() + mPaddingBottom - height;
        }
    }
    return scrollAndFocus(direction, mTempRect.top, mTempRect.bottom());
}

bool ScrollView::scrollAndFocus(int direction, int top, int bottom) {
    bool handled = true;

    const int height = getHeight();
    const int containerTop = getScrollY();
    const int containerBottom = containerTop + height;
    const bool up = (direction == View::FOCUS_UP);

    View* newFocused = findFocusableViewInBounds(up, top, bottom);
    if (newFocused == nullptr) {
        newFocused = this;
    }

    if (top >= containerTop && bottom <= containerBottom) {
        handled = false;
    } else {
        const int delta = up ? (top - containerTop) : (bottom - containerBottom);
        doScrollY(delta);
    }
    if (newFocused != findFocus()) newFocused->requestFocus(direction);
    return handled;
}

bool ScrollView::arrowScroll(int direction) {
    View* currentFocused = findFocus();
    if (currentFocused == this) currentFocused = nullptr;

    View* nextFocused = FocusFinder::getInstance().findNextFocus(this, currentFocused, direction);

    const int maxJump = getMaxScrollAmount();

    if ((nextFocused != nullptr) && isWithinDeltaOfScreen(nextFocused, maxJump, getHeight())) {
        Rect mTempRect;
        nextFocused->getDrawingRect(mTempRect);
        offsetDescendantRectToMyCoords(nextFocused, mTempRect);
        const int scrollDelta = computeScrollDeltaToGetChildRectOnScreen(mTempRect);
        doScrollY(scrollDelta);
        nextFocused->requestFocus(direction);
    } else {
        // no new focus
        int scrollDelta = maxJump;

        if ((direction == View::FOCUS_UP) && (getScrollY() < scrollDelta)) {
            scrollDelta = getScrollY();
        } else if (direction == View::FOCUS_DOWN) {
            if (getChildCount() > 0) {
                const int daBottom = getChildAt(0)->getBottom();
                const int screenBottom = getScrollY() + getHeight() - mPaddingBottom;
                if (daBottom - screenBottom < maxJump) {
                    scrollDelta = daBottom - screenBottom;
                }
            }
        }
        if (scrollDelta == 0) {
            return false;
        }
        doScrollY(direction == View::FOCUS_DOWN ? scrollDelta : -scrollDelta);
    }

    if ((currentFocused != nullptr) && currentFocused->isFocused()
            && isOffScreen(currentFocused)) {
        // previously focused item still has focus and is off screen, give
        // it up (take it back to ourselves)
        // (also, need to temporarily force FOCUS_BEFORE_DESCENDANTS so we are
        // sure to
        // get it)
        const int descendantFocusability = getDescendantFocusability();  // save
        setDescendantFocusability(ViewGroup::FOCUS_BEFORE_DESCENDANTS);
        requestFocus();
        setDescendantFocusability(descendantFocusability);  // restore
    }
    return true;
}

bool ScrollView::isOffScreen(const View* descendant) {
    return !isWithinDeltaOfScreen(descendant, 0, getHeight());
}

bool ScrollView::isWithinDeltaOfScreen(const View* descendant, int delta, int height) {
    Rect mTempRect;
    descendant->getDrawingRect(mTempRect);
    offsetDescendantRectToMyCoords(descendant, mTempRect);
    return (mTempRect.bottom() + delta) >= getScrollY()
           && (mTempRect.top - delta) <= (getScrollY() + height);
}

void ScrollView::doScrollY(int delta) {
    if (delta != 0) {
        if (mSmoothScrollingEnabled) {
            smoothScrollBy(0, delta);
        } else {
            scrollBy(0, delta);
        }
    }
}

void ScrollView::smoothScrollBy(int dx, int dy) {
    const int64_t duration = SystemClock::uptimeMillis() - mLastScroll;
    if (getChildCount() == 0) return;
    if (duration > ANIMATED_SCROLL_GAP) {
        const int height = getHeight() - mPaddingBottom - mPaddingTop;
        const int bottom = getChildAt(0)->getHeight();
        const int maxY = std::max(0, bottom - height);
        const int scrollY = mScrollY;
        dy = std::max(0, std::min(scrollY + dy, maxY)) - scrollY;

        mScroller->startScroll(mScrollX, scrollY, 0, dy,mScrollDuration);
        postInvalidateOnAnimation();
    } else {
        if (!mScroller->isFinished()) {
             mScroller->abortAnimation();
             /*if (mFlingStrictSpan != nullptr) {
                 mFlingStrictSpan->finish();
                 mFlingStrictSpan = nullptr;
             }*/
         }
        scrollBy(dx, dy);
    }
    mLastScroll = SystemClock::uptimeMillis();
}

void ScrollView::smoothScrollTo(int x, int y) {
    smoothScrollBy(x - mScrollX, y - mScrollY);
}

int ScrollView::computeVerticalScrollRange() {
    const int count = getChildCount();
    const int contentHeight = getHeight() - mPaddingBottom - mPaddingTop;
    if (count == 0) {
        return contentHeight;
    }

    int scrollRange = getChildAt(0)->getBottom();
    const int scrollY = mScrollY;
    const int overscrollBottom = std::max(0, scrollRange - contentHeight);
    if (scrollY < 0) {
        scrollRange -= scrollY;
    } else if (scrollY > overscrollBottom) {
        scrollRange += scrollY - overscrollBottom;
    }

    return scrollRange;
}

int ScrollView::computeVerticalScrollOffset() {
    return std::max(0, FrameLayout::computeVerticalScrollOffset());
}

void ScrollView::measureChild(View* child, int parentWidthMeasureSpec,int parentHeightMeasureSpec) {
    const LayoutParams* lp = (const LayoutParams*)child->getLayoutParams();

    const int verticalPadding = mPaddingTop + mPaddingBottom;
    const int childWidthMeasureSpec = getChildMeasureSpec(parentWidthMeasureSpec, mPaddingLeft
                            + mPaddingRight, lp->width);
    const int childHeightMeasureSpec = MeasureSpec::makeSafeMeasureSpec(
                                 std::max(0, MeasureSpec::getSize(parentHeightMeasureSpec) - verticalPadding),
                                 MeasureSpec::UNSPECIFIED);
    child->measure(childWidthMeasureSpec, childHeightMeasureSpec);
}

void ScrollView::measureChildWithMargins(View* child, int parentWidthMeasureSpec, int widthUsed,
        int parentHeightMeasureSpec, int heightUsed) {
    const MarginLayoutParams* lp = (const MarginLayoutParams*) child->getLayoutParams();

    const int usedTotal = mPaddingTop + mPaddingBottom + lp->topMargin + lp->bottomMargin +  heightUsed;
    const int childWidthMeasureSpec = getChildMeasureSpec(parentWidthMeasureSpec,
                               mPaddingLeft + mPaddingRight + lp->leftMargin + lp->rightMargin
                               + widthUsed, lp->width);
    const int childHeightMeasureSpec = MeasureSpec::makeSafeMeasureSpec(
                                std::max(0, MeasureSpec::getSize(parentHeightMeasureSpec) - usedTotal),
                                MeasureSpec::UNSPECIFIED);

    child->measure(childWidthMeasureSpec, childHeightMeasureSpec);
}

void ScrollView::computeScroll() {
    if (mScroller->computeScrollOffset()) {
        const int oldX = mScrollX;
        const int oldY = mScrollY;
        const int x = mScroller->getCurrX();
        const int y = mScroller->getCurrY();
        const int deltaY = consumeFlingInStretch(y - oldY);

        if ( (oldX != x) || (deltaY != 0) ) {
            const int range = getScrollRange();
            const int overscrollMode = getOverScrollMode();
            const bool canOverscroll = overscrollMode == OVER_SCROLL_ALWAYS ||
                    (overscrollMode == OVER_SCROLL_IF_CONTENT_SCROLLS && range > 0);

            overScrollBy(x - oldX, deltaY, oldX, oldY, 0, range, 0, mOverflingDistance, false);
            onScrollChanged(mScrollX, mScrollY, oldX, oldY);

            if (canOverscroll && (deltaY != 0)) {
                if ((y < 0) && (oldY >= 0)) {
                    mEdgeGlowTop->onAbsorb((int) mScroller->getCurrVelocity());
                } else if (y > range && oldY <= range) {
                    mEdgeGlowBottom->onAbsorb((int) mScroller->getCurrVelocity());
                }
            }
        }

        if (!awakenScrollBars()) {
            // Keep on drawing until the animation has finished.
            postInvalidateOnAnimation();
        }
        // For variable refresh rate project to track the current velocity of this View
        if (true/*viewVelocityApi()*/) {
            setFrameContentVelocity(std::abs(mScroller->getCurrVelocity()));
        }
    }/* else {
        if (mFlingStrictSpan != nullptr) {
            mFlingStrictSpan->finish();
            mFlingStrictSpan = nullptr;
        }
    }*/
}

void ScrollView::setOverScrollMode(int mode){
    if (mode != OVER_SCROLL_NEVER) {
        if (mEdgeGlowTop == nullptr) {
            Context* context = getContext();
            mEdgeGlowTop = new EdgeEffect(context);
            mEdgeGlowBottom = new EdgeEffect(context);
        }
    } else {
        mEdgeGlowTop = nullptr;
        mEdgeGlowBottom = nullptr;
    }
    FrameLayout::setOverScrollMode(mode);
}

bool ScrollView::onStartNestedScroll(View* child, View* target, int nestedScrollAxes){
    return (nestedScrollAxes & SCROLL_AXIS_VERTICAL) != 0;
}

void ScrollView::onNestedScrollAccepted(View* child, View* target, int axes){
    FrameLayout::onNestedScrollAccepted(child, target, axes);
    startNestedScroll(SCROLL_AXIS_VERTICAL);
}

void ScrollView::onStopNestedScroll(View* target) {
    FrameLayout::onStopNestedScroll(target);
}

void ScrollView::onNestedScroll(View* target, int dxConsumed, int dyConsumed,
            int dxUnconsumed, int dyUnconsumed){
    const int oldScrollY = mScrollY;
    scrollBy(0, dyUnconsumed);
    const int myConsumed = mScrollY - oldScrollY;
    const int myUnconsumed = dyUnconsumed - myConsumed;
    dispatchNestedScroll(0, myConsumed, 0, myUnconsumed, nullptr);
}

bool ScrollView::onNestedFling(View* target, float velocityX, float velocityY, bool consumed){
    if (!consumed) {
        flingWithNestedDispatch((int) velocityY);
        return true;
    }
    return false;
}

int ScrollView::consumeFlingInStretch(int unconsumed) {
    const int scrollY = getScrollY();
    if((scrollY < 0)||(scrollY > getScrollRange())){
        return unconsumed;
    }
    if (unconsumed > 0 && mEdgeGlowTop && mEdgeGlowTop->getDistance() != 0.f) {
         const int size = getHeight();
         const float deltaDistance = -unconsumed * FLING_DESTRETCH_FACTOR / size;
         const int consumed = std::round(-size / FLING_DESTRETCH_FACTOR
                 * mEdgeGlowTop->onPullDistance(deltaDistance, 0.5f));
         mEdgeGlowTop->onRelease();
         if (consumed != unconsumed) {
             mEdgeGlowTop->finish();
         }
         return unconsumed - consumed;
     }
     if (unconsumed < 0 && mEdgeGlowBottom && mEdgeGlowBottom->getDistance() != 0.f) {
         const int size = getHeight();
         const float deltaDistance = unconsumed * FLING_DESTRETCH_FACTOR / size;
         const int consumed = std::round(size / FLING_DESTRETCH_FACTOR
                 * mEdgeGlowBottom->onPullDistance(deltaDistance, 0.5f));
         mEdgeGlowBottom->onRelease();
         if (consumed != unconsumed) {
             mEdgeGlowBottom->finish();
         }
         return unconsumed - consumed;
     }
     return unconsumed;
}

void ScrollView::scrollToDescendant(View* child) {
    if(child == nullptr) return;
    if (!mIsLayoutDirty) {
        Rect mTempRect;
        child->getDrawingRect(mTempRect);
        /* Offset from child's local coordinates to ScrollView coordinates */
        offsetDescendantRectToMyCoords(child, mTempRect);
        const int scrollDelta = computeScrollDeltaToGetChildRectOnScreen(mTempRect);

        if (scrollDelta != 0) {
            scrollBy(0, scrollDelta);
        }
    } else {
        mChildToScrollTo = child;
    }
}

bool ScrollView::scrollToChildRect(const Rect& rect, bool immediate){
    const int delta = computeScrollDeltaToGetChildRectOnScreen(rect);
    const bool scroll = delta != 0;
    if (scroll) {
        if (immediate) {
            scrollBy(0, delta);
        } else {
            smoothScrollBy(0, delta);
        }
    }
    return scroll;
}

int ScrollView::computeScrollDeltaToGetChildRectOnScreen(const Rect& rect){
     if (getChildCount() == 0) return 0;

        const int height = getHeight();
        int screenTop = getScrollY();
        int screenBottom = screenTop + height;

        const int fadingEdge = getVerticalFadingEdgeLength();

        // leave room for top fading edge as long as rect isn't at very top
        if (rect.top > 0) {
            screenTop += fadingEdge;
        }

        // leave room for bottom fading edge as long as rect isn't at very bottom
        if (rect.bottom() < getChildAt(0)->getHeight()) {
            screenBottom -= fadingEdge;
        }

        int scrollYDelta = 0;

        if (rect.bottom() > screenBottom && rect.top > screenTop) {
            // need to move down to get it in view: move down just enough so
            // that the entire rectangle is in view (or at least the first
            // screen size chunk).

            if (rect.height > height) {
                // just enough to get screen size chunk on
                scrollYDelta += (rect.top - screenTop);
            } else {
                // get entire rect at bottom of screen
                scrollYDelta += (rect.bottom() - screenBottom);
            }

            // make sure we aren't scrolling beyond the end of our content
            int bottom = getChildAt(0)->getBottom();
            int distanceToBottom = bottom - screenBottom;
            scrollYDelta = std::min(scrollYDelta, distanceToBottom);

        } else if (rect.top < screenTop && rect.bottom() < screenBottom) {
            // need to move up to get it in view: move up just enough so that
            // entire rectangle is in view (or at least the first screen
            // size chunk of it).

            if (rect.height > height) {
                // screen size chunk
                scrollYDelta -= (screenBottom - rect.bottom());
            } else {
                // entire rect at top
                scrollYDelta -= (screenTop - rect.top);
            }

            // make sure we aren't scrolling any further than the top our content
            scrollYDelta = std::max(scrollYDelta, -getScrollY());
        }
        return scrollYDelta;
}

void ScrollView::requestChildFocus(View* child, View* focused){
    if (focused != nullptr && focused->getRevealOnFocusHint()) {
        if (!mIsLayoutDirty) {
            scrollToDescendant(focused);
        } else {
            // The child may not be laid out yet, we can't compute the scroll yet
            mChildToScrollTo = focused;
        }
    }
    FrameLayout::requestChildFocus(child, focused);
}

bool ScrollView::onRequestFocusInDescendants(int direction,Rect* previouslyFocusedRect){
    if (direction == View::FOCUS_FORWARD) {
        direction = View::FOCUS_DOWN;
    } else if (direction == View::FOCUS_BACKWARD) {
        direction = View::FOCUS_UP;
    }

    View* nextFocus = previouslyFocusedRect == nullptr ?
            FocusFinder::getInstance().findNextFocus(this, nullptr, direction) :
            FocusFinder::getInstance().findNextFocusFromRect(this,previouslyFocusedRect, direction);

    if (nextFocus == nullptr) {
        return false;
    }

    if (isOffScreen(nextFocus)) {
        return false;
    }
    return nextFocus->requestFocus(direction, previouslyFocusedRect);
}

bool ScrollView::requestChildRectangleOnScreen(View* child, Rect& rectangle, bool immediate){
    rectangle.offset(child->getLeft() - child->getScrollX(),
            child->getTop() - child->getScrollY());
    return scrollToChildRect(rectangle, immediate);
}

void ScrollView::requestLayout() {
    mIsLayoutDirty = true;
    FrameLayout::requestLayout();
}

void ScrollView::onLayout(bool changed, int l, int t, int w, int h){
    FrameLayout::onLayout(changed, l, t, w, h);
    mIsLayoutDirty = false;
    // Give a child focus if it needs it
    if ((mChildToScrollTo != nullptr) && isViewDescendantOf(mChildToScrollTo, this)) {
        scrollToDescendant(mChildToScrollTo);
    }
    mChildToScrollTo = nullptr;

    if (!isLaidOut()) {
        if (mSavedState != nullptr) {
            mScrollY = mSavedState->scrollPosition;
            mSavedState = nullptr;
        }// mScrollY default value is "0"
        const int childHeight = (getChildCount() > 0) ? getChildAt(0)->getMeasuredHeight() : 0;
        const int scrollRange = std::max(0, childHeight - (h - mPaddingBottom - mPaddingTop));

        // Don't forget to clamp
        if (mScrollY > scrollRange) {
            mScrollY = scrollRange;
        } else if (mScrollY < 0) {
            mScrollY = 0;
        }
    }
    // Calling this with the present values causes it to re-claim them
    scrollTo(mScrollX, mScrollY);
}

void ScrollView::onSizeChanged(int w, int h, int oldw, int oldh) {
    FrameLayout::onSizeChanged(w, h, oldw, oldh);

    View* currentFocused = findFocus();
    if ((nullptr == currentFocused) || (this == currentFocused))
        return;

    // If the currently-focused view was visible on the screen when the
    // screen was at the old height, then scroll the screen to make that
    // view visible with the new screen height.
    if (isWithinDeltaOfScreen(currentFocused, 0, oldh)) {
        Rect mTempRect;
        currentFocused->getDrawingRect(mTempRect);
        offsetDescendantRectToMyCoords(currentFocused, mTempRect);
        const int scrollDelta = computeScrollDeltaToGetChildRectOnScreen(mTempRect);
        doScrollY(scrollDelta);
    }
}

bool ScrollView::isViewDescendantOf(View* child, View* parent){
    if (child == parent) {
        return true;
    }

    ViewGroup* theParent = child->getParent();
    return dynamic_cast<ViewGroup*>(theParent) && isViewDescendantOf(theParent, parent);
}

void ScrollView::fling(int velocityY) {
    if (getChildCount() > 0) {
        const int height = getHeight() - mPaddingBottom - mPaddingTop;
        const int bottom = getChildAt(0)->getHeight();

        mScroller->fling(mScrollX, mScrollY, 0, velocityY, 0, 0, 0,
                         std::max(0, bottom - height), 0, height/2);

        // For variable refresh rate project to track the current velocity of this View
        if (true/*viewVelocityApi()*/) {
            setFrameContentVelocity(std::abs(mScroller->getCurrVelocity()));
        }
        /*if (mFlingStrictSpan == null) {
            mFlingStrictSpan = StrictMode.enterCriticalSpan("ScrollView-fling");
        }*/

        postInvalidateOnAnimation();
    }
}

void ScrollView::flingWithNestedDispatch(int velocityY) {
    const bool canFling = (mScrollY > 0 || velocityY > 0) &&
        (mScrollY < getScrollRange() || velocityY < 0);
    if (!dispatchNestedPreFling(0, velocityY)) {
        const bool consumed = dispatchNestedFling(0, velocityY, canFling);
        if (canFling) {
            fling(velocityY);
        }else if(!consumed){
            if (!mEdgeGlowTop->isFinished()) {
                if (shouldAbsorb(mEdgeGlowTop, -velocityY)) {
                    mEdgeGlowTop->onAbsorb(-velocityY);
                } else {
                    fling(velocityY);
                }
            } else if (!mEdgeGlowBottom->isFinished()) {
                if (shouldAbsorb(mEdgeGlowBottom, velocityY)) {
                    mEdgeGlowBottom->onAbsorb(velocityY);
                } else {
                    fling(velocityY);
                }
            }
        }
    }
}

bool ScrollView::shouldAbsorb(EdgeEffect* edgeEffect, int velocity) {
    if (velocity > 0) {
        return true;
    }
    const float distance = edgeEffect->getDistance() * getHeight();
    // This is flinging without the spring, so let's see if it will fling past the overscroll
    const float flingDistance = (float) mScroller->getSplineFlingDistance(-velocity);

    return flingDistance < distance;
}

void ScrollView::endDrag() {
    mIsBeingDragged = false;

    recycleVelocityTracker();

    if (shouldDisplayEdgeEffects()) {
        mEdgeGlowTop->onRelease();
        mEdgeGlowBottom->onRelease();
    }

    /*if (mScrollStrictSpan != nullptr) {
        mScrollStrictSpan->finish();
        mScrollStrictSpan = nullptr;
    }*/
}

void ScrollView::scrollTo(int x, int y){
    if (getChildCount() > 0) {
        View* child = getChildAt(0);
        x = clamp(x, getWidth() - mPaddingRight - mPaddingLeft, child->getWidth());
        y = clamp(y, getHeight()- mPaddingBottom - mPaddingTop, child->getHeight());
        if (x != mScrollX || y != mScrollY) {
	    FrameLayout::scrollTo(x, y);
        }
    }
}

void ScrollView::draw(Canvas& canvas){
    FrameLayout::draw(canvas);
    if (shouldDisplayEdgeEffects()) {
        const int scrollY = mScrollY;
        int width,height;
        float translateX,translateY;
        const bool clipToPadding = getClipToPadding();
        if (!mEdgeGlowTop->isFinished()) {
            canvas.save();
            if (clipToPadding) {
                width = getWidth() - mPaddingLeft - mPaddingRight;
                height = getHeight() - mPaddingTop - mPaddingBottom;
                translateX = mPaddingLeft;
                translateY = mPaddingTop;
            } else {
                width = getWidth();
                height = getHeight();
                translateX = 0;
                translateY = 0;
            }
            canvas.translate(translateX, std::min(0, scrollY) + translateY);
            mEdgeGlowTop->setSize(width, height);
            if (mEdgeGlowTop->draw(canvas)) {
                postInvalidateOnAnimation();
            }
            canvas.restore();
        }
        if (!mEdgeGlowBottom->isFinished()) {
            canvas.save();
            if (clipToPadding) {
                width = getWidth() - mPaddingLeft - mPaddingRight;
                height = getHeight() - mPaddingTop - mPaddingBottom;
                translateX = mPaddingLeft;
                translateY = mPaddingTop;
            } else {
                width = getWidth();
                height = getHeight();
                translateX = 0;
                translateY = 0;
            }
            canvas.translate(width + translateX,
                    std::max(getScrollRange(), scrollY) + height + translateY);
            canvas.rotate_degrees(180);
            mEdgeGlowBottom->setSize(width, height);
            if (mEdgeGlowBottom->draw(canvas)) {
                postInvalidateOnAnimation();
            }
            canvas.restore();
        }
    }    
}

int ScrollView::clamp(int n, int my, int child){
   if (my >= child || n < 0) {
      return 0;
   }
   if ((my+n) > child) {
       return child - my;
   }
   return n;
}

void ScrollView::onRestoreInstanceState(Parcelable& state) {
    /*if (mContext.getApplicationInfo().targetSdkVersion <= Build.VERSION_CODES.JELLY_BEAN_MR2) {
        // Some old apps reused IDs in ways they shouldn't have.
        // Don't break them, but they don't get scroll state restoration.
        FrameLayout::onRestoreInstanceState(state);
        return;
    }*/
    SavedState* ss = (SavedState*)&state;
    FrameLayout::onRestoreInstanceState(*ss->getSuperState());
    mSavedState = ss;
    requestLayout();
}

Parcelable* ScrollView::onSaveInstanceState() {
    /*if (mContext.getApplicationInfo().targetSdkVersion <= Build.VERSION_CODES.JELLY_BEAN_MR2) {
        // Some old apps reused IDs in ways they shouldn't have.
        // Don't break them, but they don't get scroll state restoration.
        return FrameLayout::onSaveInstanceState();
    }*/
    Parcelable* superState = FrameLayout::onSaveInstanceState();
    SavedState* ss = new SavedState(superState);
    ss->scrollPosition = mScrollY;
    return ss;
}
}
