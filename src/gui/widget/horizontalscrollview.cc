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
#include <widget/horizontalscrollview.h>
#include <focusfinder.h>
#include <systemclock.h>
#include <cdlog.h>

namespace cdroid{

DECLARE_WIDGET2(HorizontalScrollView,"cdroid:attr/horizontalScrollViewStyle")
 
HorizontalScrollView::HorizontalScrollView(int w,int h):FrameLayout(w,h){
    initScrollView(nullptr);
}

HorizontalScrollView::HorizontalScrollView(Context*ctx,const AttributeSet&atts)
  :FrameLayout(ctx,atts){
    initScrollView(&atts);
    setFillViewport(atts.getBoolean("fillViewport", false));
    mScrollDuration=atts.getInt("scrollDuration",300);
}

HorizontalScrollView::~HorizontalScrollView(){
    recycleVelocityTracker();
    delete mSavedState;
    delete mScroller;
    delete mEdgeGlowLeft;
    delete mEdgeGlowRight;
}

float HorizontalScrollView::getLeftFadingEdgeStrength() {
    if (getChildCount() == 0) {
        return 0.0f;
    }
    const int length = getHorizontalFadingEdgeLength();
    if (mScrollX < length) {
        return mScrollX / (float) length;
    }
    return 1.0f;
}

float HorizontalScrollView::getRightFadingEdgeStrength() {
    if (getChildCount() == 0) {
        return 0.0f;
    }

    const int length = getHorizontalFadingEdgeLength();
    const int rightEdge = getWidth() - mPaddingRight;
    const int span = getChildAt(0)->getRight() - mScrollX - rightEdge;
    if (span < length) {
        return span / (float) length;
    }

    return 1.0f;
}

void HorizontalScrollView::setEdgeEffectColor(int color) {
    setLeftEdgeEffectColor(color);
    setRightEdgeEffectColor(color);
}

void HorizontalScrollView::setLeftEdgeEffectColor(int color) {
    mEdgeGlowLeft->setColor(color);
}

void HorizontalScrollView::setRightEdgeEffectColor(int color) {
    mEdgeGlowRight->setColor(color);
}

int HorizontalScrollView::getLeftEdgeEffectColor()const{
    return mEdgeGlowLeft->getColor();
}

int HorizontalScrollView::getRightEdgeEffectColor()const{
    return mEdgeGlowRight->getColor();
}

int HorizontalScrollView::getMaxScrollAmount() {
    return (int) (MAX_SCROLL_FACTOR * (mRight-mLeft));
}

void HorizontalScrollView::initScrollView(const AttributeSet*atts) {
    mScroller = new OverScroller(getContext());
    mVelocityTracker = nullptr;
    mChildToScrollTo = nullptr;
    mIsLayoutDirty= false;
    mFillViewport = false;
    mIsBeingDragged = false;
    mActivePointerId = INVALID_POINTER;
    mSmoothScrollingEnabled = true;
    setFocusable(true);
    setDescendantFocusability(FOCUS_AFTER_DESCENDANTS);
    setWillNotDraw(false);
    mLastScroll = 0;
    mSavedState = nullptr;
    mEdgeGlowLeft = new EdgeEffect(mContext,atts);
    mEdgeGlowRight= new EdgeEffect(mContext,atts);
    ViewConfiguration&configuration=ViewConfiguration::get(mContext);
    mTouchSlop = configuration.getScaledTouchSlop();
    mMinimumVelocity = configuration.getScaledMinimumFlingVelocity();
    mMaximumVelocity = configuration.getScaledMaximumFlingVelocity();
    mOverscrollDistance= configuration.getScaledOverscrollDistance();
    mOverflingDistance = configuration.getScaledOverflingDistance();
    mHorizontalScrollFactor = configuration.getScaledHorizontalScrollFactor();
    mScrollDuration = 400;
}

void HorizontalScrollView::addView(View* child){
    if(getChildCount()==0)
        FrameLayout::addView(child);
}

void HorizontalScrollView::addView(View* child, int index){
    if(getChildCount()==0)
        FrameLayout::addView(child,index);
}

void HorizontalScrollView::addView(View* child, ViewGroup::LayoutParams* params){
    if(getChildCount()==0)
        FrameLayout::addView(child,params);
} 

void HorizontalScrollView::addView(View* child, int index,ViewGroup::LayoutParams* params){
    if(getChildCount()==0)
        FrameLayout::addView(child,index,params);
}

bool HorizontalScrollView::canScroll() {
    View* child = getChildAt(0);
    if (child != nullptr) {
        const int childWidth = child->getWidth();
        return getWidth() < childWidth + mPaddingLeft + mPaddingRight ;
    }
    return false;
}

bool HorizontalScrollView::isFillViewport()const{
    return mFillViewport;
}

void HorizontalScrollView::setFillViewport(bool fillViewport) {
    if (fillViewport != mFillViewport) {
        mFillViewport = fillViewport;
        requestLayout();
    }
}

bool HorizontalScrollView::isSmoothScrollingEnabled()const{
    return mSmoothScrollingEnabled;
}

void HorizontalScrollView::setSmoothScrollingEnabled(bool smoothScrollingEnabled) {
    mSmoothScrollingEnabled = smoothScrollingEnabled;
}

void HorizontalScrollView::onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
    FrameLayout::onMeasure(widthMeasureSpec, heightMeasureSpec);

    if (!mFillViewport) return;

    int widthMode = MeasureSpec::getMode(widthMeasureSpec);
    if (widthMode == MeasureSpec::UNSPECIFIED) {
        return;
    }

    if (getChildCount() > 0) {
        View* child = getChildAt(0);
        int widthPadding;
        int heightPadding;
        LayoutParams* lp = (LayoutParams*) child->getLayoutParams();
        widthPadding = mPaddingLeft + mPaddingRight + lp->leftMargin + lp->rightMargin;
        heightPadding = mPaddingTop + mPaddingBottom + lp->topMargin + lp->bottomMargin;

        const int desiredWidth = getMeasuredWidth() - widthPadding;
        if (child->getMeasuredWidth() < desiredWidth) {
            int childWidthMeasureSpec = MeasureSpec::makeMeasureSpec(
                desiredWidth, MeasureSpec::EXACTLY);
            int childHeightMeasureSpec = getChildMeasureSpec(
                        heightMeasureSpec, heightPadding, lp->height);
            child->measure(childWidthMeasureSpec, childHeightMeasureSpec);
        }
    }
}

bool HorizontalScrollView::dispatchKeyEvent(KeyEvent& event) {
        // Let the focused view and/or our descendants get the key first
    return FrameLayout::dispatchKeyEvent(event) || executeKeyEvent(event);
}

bool HorizontalScrollView::executeKeyEvent(KeyEvent& event) {

    LOGV("%s.%s",event.getLabel(),KeyEvent::actionToString(event.getAction()).c_str());
    if (!canScroll()) {
        if (isFocused()) {
            View* currentFocused = findFocus();
            if (currentFocused == this) currentFocused = nullptr;
            View* nextFocused = FocusFinder::getInstance().findNextFocus(this,
                            currentFocused, View::FOCUS_RIGHT);
            return nextFocused != nullptr && nextFocused != this &&
                nextFocused->requestFocus(View::FOCUS_RIGHT);
        }
        return false;
    }

    bool handled = false;
    if (event.getAction() == KeyEvent::ACTION_DOWN) {
        switch (event.getKeyCode()) {
        case KeyEvent::KEYCODE_DPAD_LEFT:
            if (!event.isAltPressed()) {
                handled = arrowScroll(View::FOCUS_LEFT);
            } else {
                handled = fullScroll(View::FOCUS_LEFT);
            }
            break;
        case KeyEvent::KEYCODE_DPAD_RIGHT:
            if (!event.isAltPressed()) {
                handled = arrowScroll(View::FOCUS_RIGHT);
            } else {
                handled = fullScroll(View::FOCUS_RIGHT);
            }
            break;
        case KeyEvent::KEYCODE_SPACE:
            pageScroll(event.isShiftPressed() ? View::FOCUS_LEFT : View::FOCUS_RIGHT);
            break;
        }
    }

    return handled;
}

bool HorizontalScrollView::inChild(int x, int y) {
    if (getChildCount() > 0) {
        View* child = getChildAt(0);
        return !(y < child->getTop()
                || y >= child->getBottom()
                || x < child->getLeft() - mScrollX
                || x >= child->getRight() - mScrollX);
    }
    return false;
}

void HorizontalScrollView::initOrResetVelocityTracker() {
    if (mVelocityTracker == nullptr) {
        mVelocityTracker = VelocityTracker::obtain();
    } else {
        mVelocityTracker->clear();
    }
}

void HorizontalScrollView::initVelocityTrackerIfNotExists() {
    if (mVelocityTracker == nullptr) {
        mVelocityTracker = VelocityTracker::obtain();
    }
}

void HorizontalScrollView::recycleVelocityTracker() {
    if (mVelocityTracker != nullptr) {
        mVelocityTracker->recycle();
        mVelocityTracker = nullptr;
    }
}

void HorizontalScrollView::requestDisallowInterceptTouchEvent(bool disallowIntercept){
    if (disallowIntercept) {
        recycleVelocityTracker();
    }
    FrameLayout::requestDisallowInterceptTouchEvent(disallowIntercept);
}

bool HorizontalScrollView::onInterceptTouchEvent(MotionEvent& ev){
    const int action = ev.getAction();
    if ((action == MotionEvent::ACTION_MOVE) && (mIsBeingDragged)) {
        return true;
    }

    if (FrameLayout::onInterceptTouchEvent(ev))  return true;

    switch (action & MotionEvent::ACTION_MASK) {
    case MotionEvent::ACTION_MOVE: {
        /* mIsBeingDragged == false, otherwise the shortcut would have caught it. Check
         * whether the user has moved far enough from his original down touch. */

        /*Locally do absolute value. mLastMotionX is set to the x value
        * of the down event.*/
        const int activePointerId = mActivePointerId;
        if (activePointerId == INVALID_POINTER) {
            // If we don't have a valid id, the touch down wasn't on content.
            break;
        }

        const int pointerIndex = ev.findPointerIndex(activePointerId);
        if (pointerIndex == -1) {
            LOGE("Invalid pointerId=%d in onInterceptTouchEvent",activePointerId);
            break;
        }

        const int x = (int) ev.getX(pointerIndex);
        const int xDiff = (int) std::abs(x - mLastMotionX);
        if (xDiff > mTouchSlop) {
            mIsBeingDragged = true;
            mLastMotionX = x;
            initVelocityTrackerIfNotExists();
            mVelocityTracker->addMovement(ev);
            if (mParent != nullptr) mParent->requestDisallowInterceptTouchEvent(true);
        }
        break;
    }

    case MotionEvent::ACTION_DOWN: {
        const int x = (int) ev.getX();
        if (!inChild((int) x, (int) ev.getY())) {
            mIsBeingDragged = false;
            recycleVelocityTracker();
            break;
        }

        /* Remember location of down touch.
         * ACTION_DOWN always refers to pointer index 0. */
        mLastMotionX = x;
        mActivePointerId = ev.getPointerId(0);

        initOrResetVelocityTracker();
        mVelocityTracker->addMovement(ev);

        /*If being flinged and user touches the screen, initiate drag;
        * otherwise don't.  mScroller.isFinished should be false when
        * being flinged. */
        mIsBeingDragged = !mScroller->isFinished()||!mEdgeGlowLeft->isFinished()||!mEdgeGlowRight->isFinished();
        if(shouldDisplayEdgeEffects()){
            if (!mEdgeGlowLeft->isFinished()) {
                mEdgeGlowLeft->onPullDistance(0.f, 1.f - ev.getY() / getHeight());
            }
            if (!mEdgeGlowRight->isFinished()) {
               mEdgeGlowRight->onPullDistance(0.f, ev.getY() / getHeight());
            }
        }
        break;
    }

    case MotionEvent::ACTION_CANCEL:
    case MotionEvent::ACTION_UP:
        /* Release the drag */
        mIsBeingDragged = false;
        mActivePointerId = INVALID_POINTER;
        if (mScroller->springBack(mScrollX, mScrollY, 0, getScrollRange(), 0, 0)) {
            postInvalidateOnAnimation();
        }
        break;
    case MotionEvent::ACTION_POINTER_DOWN: {
        const int index = ev.getActionIndex();
        mLastMotionX = (int) ev.getX(index);
        mActivePointerId = ev.getPointerId(index);
        break;
    }
    case MotionEvent::ACTION_POINTER_UP:
        onSecondaryPointerUp(ev);
        mLastMotionX = (int) ev.getX(ev.findPointerIndex(mActivePointerId));
        break;
    }

    /*The only time we want to intercept motion events is if we are in the
    * drag mode.*/
    return mIsBeingDragged;
}

bool HorizontalScrollView::onTouchEvent(MotionEvent& ev) {
    initVelocityTrackerIfNotExists();
    mVelocityTracker->addMovement(ev);

    const int action = ev.getAction();

    switch (action & MotionEvent::ACTION_MASK) {
    case MotionEvent::ACTION_DOWN:
        if (getChildCount() == 0) {
            return false;
        }
        if (!mScroller->isFinished()) {
            ViewGroup* parent = getParent();
            if (parent != nullptr) {
                parent->requestDisallowInterceptTouchEvent(true);
            }
        }

        // If being flinged and user touches, stop the fling. isFinished
        // will be false if being flinged.
        if (!mScroller->isFinished()) {
            mScroller->abortAnimation();
        }

        // Remember where the motion event started
        mLastMotionX = (int) ev.getX();
        mActivePointerId = ev.getPointerId(0);
        break;
    case MotionEvent::ACTION_MOVE:{
        const int activePointerIndex = ev.findPointerIndex(mActivePointerId);
        if (activePointerIndex == -1) {
            LOGE("Invalid pointerId=%d in onTouchEvent" , mActivePointerId);
            break;
        }

        const int x = (int) ev.getX(activePointerIndex);
        int deltaX = mLastMotionX - x;
        if (!mIsBeingDragged && std::abs(deltaX) > mTouchSlop) {
            ViewGroup* parent = getParent();
            if (parent != nullptr) {
                parent->requestDisallowInterceptTouchEvent(true);
            }
            mIsBeingDragged = true;
            if (deltaX > 0) {
                deltaX -= mTouchSlop;
            } else {
                deltaX += mTouchSlop;
            }
        }
        if (mIsBeingDragged) {
            // Scroll to follow the motion event
            mLastMotionX = x;

            const int oldX = mScrollX;
            const int oldY = mScrollY;
            const int range = getScrollRange();
            const int overscrollMode = getOverScrollMode();
            const bool canOverscroll = overscrollMode == OVER_SCROLL_ALWAYS ||
                    (overscrollMode == OVER_SCROLL_IF_CONTENT_SCROLLS && range > 0);

            const float displacement = ev.getY(activePointerIndex)/getHeight();
            if (canOverscroll && shouldDisplayEdgeEffects()) {
                int consumed = 0;
                if ((deltaX < 0) && (mEdgeGlowRight->getDistance() != 0.f)) {
                    consumed = std::round(getWidth()
                          * mEdgeGlowRight->onPullDistance((float) deltaX / getWidth(),
                            displacement));
                } else if ((deltaX > 0) && (mEdgeGlowLeft->getDistance() != 0.f)) {
                    consumed = std::round(-getWidth()
                           * mEdgeGlowLeft->onPullDistance((float) -deltaX / getWidth(),
                             1.f - displacement));
                }
                deltaX -= consumed;
            }
            // Calling overScrollBy will call onOverScrolled, which
            // calls onScrollChanged if applicable.
            overScrollBy(deltaX, 0, mScrollX, 0, range, 0, mOverscrollDistance, 0, true);

            if (canOverscroll && (deltaX!=0.f)) {
                const int pulledToX = oldX + deltaX;
                if (pulledToX < 0) {
                    mEdgeGlowLeft->onPullDistance((float) -deltaX / getWidth(),1.f - displacement);
                    if (!mEdgeGlowRight->isFinished()) {
                        mEdgeGlowRight->onRelease();
                    }
                } else if (pulledToX > range) {
                    mEdgeGlowRight->onPullDistance((float) deltaX / getWidth(),displacement);
                    if (!mEdgeGlowLeft->isFinished()) {
                        mEdgeGlowLeft->onRelease();
                    }
                }
                if (shouldDisplayEdgeEffects() && (!mEdgeGlowLeft->isFinished() || !mEdgeGlowRight->isFinished())) {
                    postInvalidateOnAnimation();
                }
            }
        }
        break;
    }
    case MotionEvent::ACTION_UP:
        if (mIsBeingDragged) {
            VelocityTracker* velocityTracker = mVelocityTracker;
            velocityTracker->computeCurrentVelocity(1000, mMaximumVelocity);
            const int initialVelocity = (int)velocityTracker->getXVelocity(mActivePointerId);

            if (getChildCount() > 0) {
                if ((std::abs(initialVelocity) > mMinimumVelocity)) {
                    fling(-initialVelocity);
                } else {
                    if (mScroller->springBack(mScrollX, mScrollY, 0,
                            getScrollRange(), 0, 0)) {
                        postInvalidateOnAnimation();
                    }
                }
            }

            mActivePointerId = INVALID_POINTER;
            mIsBeingDragged = false;
            recycleVelocityTracker();

            if (shouldDisplayEdgeEffects()) {
                mEdgeGlowLeft->onRelease();
                mEdgeGlowRight->onRelease();
            }
        }
        break;
    case MotionEvent::ACTION_CANCEL:
        if (mIsBeingDragged && getChildCount() > 0) {
            if (mScroller->springBack(mScrollX, mScrollY, 0, getScrollRange(), 0, 0)) {
                postInvalidateOnAnimation();
            }
            mActivePointerId = INVALID_POINTER;
            mIsBeingDragged = false;
            recycleVelocityTracker();

            if (shouldDisplayEdgeEffects()) {
                mEdgeGlowLeft->onRelease();
                mEdgeGlowRight->onRelease();
            }
        }
        break;
    case MotionEvent::ACTION_POINTER_UP:
        onSecondaryPointerUp(ev);
        break;
    }
    return true;
}

void HorizontalScrollView::onSecondaryPointerUp(MotionEvent& ev) {
    const int pointerIndex = (ev.getAction() & MotionEvent::ACTION_POINTER_INDEX_MASK) >>
            MotionEvent::ACTION_POINTER_INDEX_SHIFT;
    const int pointerId = ev.getPointerId(pointerIndex);
    if (pointerId == mActivePointerId) {
        // This was our active pointer going up. Choose a new
        // active pointer and adjust accordingly.
        // TODO: Make this decision more intelligent.
        const int newPointerIndex = pointerIndex == 0 ? 1 : 0;
        mLastMotionX = (int) ev.getX(newPointerIndex);
        mActivePointerId = ev.getPointerId(newPointerIndex);
        if (mVelocityTracker != nullptr) mVelocityTracker->clear();
    }
}

bool HorizontalScrollView::onGenericMotionEvent(MotionEvent& event){
    switch (event.getAction()) {
    case MotionEvent::ACTION_SCROLL: {
        if (!mIsBeingDragged) {
            float axisValue;
            if (event.isFromSource(InputDevice::SOURCE_CLASS_POINTER)) {
                if ((event.getMetaState() & KeyEvent::META_SHIFT_ON) != 0) {
                    axisValue = -event.getAxisValue((int)MotionEvent::AXIS_VSCROLL,0);
                } else {
                    axisValue = event.getAxisValue((int)MotionEvent::AXIS_HSCROLL,0);
                }
            } else if (event.isFromSource(InputDevice::SOURCE_ROTARY_ENCODER)) {
                axisValue = event.getAxisValue((int)MotionEvent::AXIS_SCROLL,0);
            } else {
                axisValue = 0;
            }

            int delta = std::round(axisValue * mHorizontalScrollFactor);
            if (delta != 0) {
                const int range = getScrollRange();
                int oldScrollX = mScrollX;
                int newScrollX = oldScrollX + delta;
                const int overscrollMode = getOverScrollMode();
                bool canOverscroll = !event.isFromSource(InputDevice::SOURCE_MOUSE)
                                && (overscrollMode == OVER_SCROLL_ALWAYS
                                || (overscrollMode == OVER_SCROLL_IF_CONTENT_SCROLLS && range > 0));
                bool absorbed = false;
                if (newScrollX < 0) {
                    if(canOverscroll){
                        mEdgeGlowLeft->onPullDistance(-(float) newScrollX / getWidth(),0.5f);
                        mEdgeGlowLeft->onRelease();
                        invalidate();
                        absorbed = true;
                    }
                    newScrollX = 0;
                } else if (newScrollX > range) {
                    if(canOverscroll){
                        mEdgeGlowRight->onPullDistance((float) (newScrollX - range) / getWidth(), 0.5f);
                        mEdgeGlowRight->onRelease();
                        invalidate();
                        absorbed = true;
                    }
                    newScrollX = range;
                }
                if (newScrollX != oldScrollX) {
                    FrameLayout::scrollTo(newScrollX, mScrollY);
                    return true;
                }
                if(absorbed)return true;
            }
        }
     }
   }
   return FrameLayout::onGenericMotionEvent(event);
}

bool HorizontalScrollView::shouldDelayChildPressedState(){
    return true;
}

void HorizontalScrollView::onOverScrolled(int scrollX, int scrollY,bool clampedX, bool clampedY){
    if (!mScroller->isFinished()) {
        const int oldX = mScrollX;
        const int oldY = mScrollY;
        mScrollX = scrollX;
        mScrollY = scrollY;
        invalidateParentIfNeeded();
        onScrollChanged(mScrollX, mScrollY, oldX, oldY);
        if (clampedX) {
            mScroller->springBack(mScrollX, mScrollY, 0, getScrollRange(), 0, 0);
        }
    } else {
        FrameLayout::scrollTo(scrollX, scrollY);
    }
    awakenScrollBars();
}

int HorizontalScrollView::getScrollRange() {
    int scrollRange = 0;
    if (getChildCount() > 0) {
        View* child = getChildAt(0);
        scrollRange = std::max(0, child->getWidth() - (getWidth() - mPaddingLeft - mPaddingRight));
    }
    return scrollRange;
}

View* HorizontalScrollView::findFocusableViewInMyBounds(bool leftFocus,int left, View* preferredFocusable){
    const int fadingEdgeLength = getHorizontalFadingEdgeLength() / 2;
    const int leftWithoutFadingEdge = left + fadingEdgeLength;
    const int rightWithoutFadingEdge = left + getWidth() - fadingEdgeLength;

    if ((preferredFocusable != nullptr)
            && (preferredFocusable->getLeft() < rightWithoutFadingEdge)
            && (preferredFocusable->getRight() > leftWithoutFadingEdge)) {
        return preferredFocusable;
    }

    return findFocusableViewInBounds(leftFocus, leftWithoutFadingEdge,rightWithoutFadingEdge);
}

View* HorizontalScrollView::findFocusableViewInBounds(bool leftFocus, int left, int right){
    View* focusCandidate = nullptr;
    std::vector<View*> focusables = getFocusables(View::FOCUS_FORWARD);

    /* A fully contained focusable is one where its left is below the bound's
     * left, and its right is above the bound's right. A partially
     * contained focusable is one where some part of it is within the
     * bounds, but it also has some part that is not within bounds.  A fully contained
     * focusable is preferred to a partially contained focusable.*/
    bool foundFullyContainedFocusable = false;

    const int count = focusables.size();
    for (int i = 0; i < count; i++) {
        View* view = focusables[i];
        int viewLeft = view->getLeft();
        int viewRight = view->getRight();

        if (left < viewRight && viewLeft < right) {
            /* the focusable is in the target area, it is a candidate for focusing */

            const bool viewIsFullyContained = (left < viewLeft) &&  (viewRight < right);

            if (focusCandidate == nullptr) {
                /* No candidate, take this one */
                focusCandidate = view;
                foundFullyContainedFocusable = viewIsFullyContained;
            } else {
                const bool viewIsCloserToBoundary =(leftFocus && viewLeft < focusCandidate->getLeft()) ||
                                    (!leftFocus && viewRight > focusCandidate->getRight());

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
                             * contained view if it's closer*/
                        focusCandidate = view;
                    }
                }
            }
        }
    }
    return focusCandidate;
}

bool HorizontalScrollView::pageScroll(int direction) {
    const bool right = direction == View::FOCUS_RIGHT;
    const int width = getWidth();
    Rect mTempRect{};

    if (right) {
        mTempRect.left = getScrollX() + width;
        int count = getChildCount();
        if (count > 0) {
            View* view = getChildAt(0);
            if (mTempRect.left + width > view->getRight()) {
                mTempRect.left = view->getRight() - width;
            }
        }
    } else {
        mTempRect.left = getScrollX() - width;
        if (mTempRect.left < 0) {
            mTempRect.left = 0;
        }
    }
    mTempRect.width =  width;

    return scrollAndFocus(direction, mTempRect.left, mTempRect.right());
}

bool HorizontalScrollView::fullScroll(int direction) {
    const bool right = direction == View::FOCUS_RIGHT;
    const int width = getWidth();
    Rect mTempRect{};

    mTempRect.left = 0;
    mTempRect.width = width;

    if (right) {
        const int count = getChildCount();
        if (count > 0) {
            View* view = getChildAt(0);
            mTempRect.left = view->getRight() - width;
        }
    }

    return scrollAndFocus(direction, mTempRect.left, mTempRect.right());
}

bool HorizontalScrollView::scrollAndFocus(int direction, int left, int right){
    bool handled = true;

    const int width = getWidth();
    const int containerLeft = getScrollX();
    const int containerRight = containerLeft + width;
    const bool goLeft = direction == View::FOCUS_LEFT;

    View* newFocused = findFocusableViewInBounds(goLeft, left, right);
    if (newFocused == nullptr) {
        newFocused = this;
    }

    if (left >= containerLeft && right <= containerRight) {
        handled = false;
    } else {
        int delta = goLeft ? (left - containerLeft) : (right - containerRight);
        doScrollX(delta);
    }

    if (newFocused != findFocus()) newFocused->requestFocus(direction);

    return handled;
}

bool HorizontalScrollView::arrowScroll(int direction){
    View* currentFocused = findFocus();
    if (currentFocused == this) currentFocused = nullptr;

    View* nextFocused = FocusFinder::getInstance().findNextFocus(this, currentFocused, direction);

    const int maxJump = getMaxScrollAmount();

    if ((nextFocused != nullptr) && isWithinDeltaOfScreen(nextFocused, maxJump)) {
        Rect mTempRect;
        nextFocused->getDrawingRect(mTempRect);
        offsetDescendantRectToMyCoords(nextFocused, mTempRect);
        const int scrollDelta = computeScrollDeltaToGetChildRectOnScreen(mTempRect);
        doScrollX(scrollDelta);
        nextFocused->requestFocus(direction);
    } else {
        // no new focus
        int scrollDelta = maxJump;

        if ((direction == View::FOCUS_LEFT) && (getScrollX() < scrollDelta)) {
            scrollDelta = getScrollX();
        } else if ((direction == View::FOCUS_RIGHT) && (getChildCount() > 0)) {

            const int daRight = getChildAt(0)->getRight();
            const int screenRight = getScrollX() + getWidth();

            if (daRight - screenRight < maxJump) {
                scrollDelta = daRight - screenRight;
            }
        }
        if (scrollDelta == 0) {
            return false;
        }
        doScrollX(direction == View::FOCUS_RIGHT ? scrollDelta : -scrollDelta);
    }

    if ((currentFocused != nullptr) && currentFocused->isFocused()
            && isOffScreen(currentFocused)) {
        // previously focused item still has focus and is off screen, give
        // it up (take it back to ourselves)
        // (also, need to temporarily force FOCUS_BEFORE_DESCENDANTS so we are
        // sure to get it)
        const int descendantFocusability = getDescendantFocusability();  // save
        setDescendantFocusability(ViewGroup::FOCUS_BEFORE_DESCENDANTS);
        requestFocus();
        setDescendantFocusability(descendantFocusability);  // restore
    }
    return true;
}

bool HorizontalScrollView::isOffScreen(View* descendant){
    return !isWithinDeltaOfScreen(descendant, 0);
}

bool HorizontalScrollView::isWithinDeltaOfScreen(View* descendant, int delta){
    Rect mTempRect;
    descendant->getDrawingRect(mTempRect);
    offsetDescendantRectToMyCoords(descendant, mTempRect);

    return (mTempRect.right() + delta) >= getScrollX()
            && (mTempRect.left - delta) <= (getScrollX() + getWidth());
}

void HorizontalScrollView::doScrollX(int delta) {
    if (delta != 0) {
        if (mSmoothScrollingEnabled) {
            smoothScrollBy(delta, 0);
        } else {
            scrollBy(delta, 0);
        }
    }
}

void HorizontalScrollView::smoothScrollBy(int dx, int dy) {
    if (getChildCount() == 0) {
        // Nothing to do.
        return;
    }
    const auto duration = SystemClock::uptimeMillis() - mLastScroll;
    if (duration > ANIMATED_SCROLL_GAP) {
        const int width = getWidth() - mPaddingRight - mPaddingLeft;
        const int right = getChildAt(0)->getWidth();
        const int maxX = std::max(0, right - width);
        const int scrollX = mScrollX;
        dx = std::max(0, std::min(scrollX + dx, maxX)) - scrollX;

        mScroller->startScroll(scrollX, mScrollY, dx, 0,mScrollDuration);
        postInvalidateOnAnimation();
    } else {
        if (!mScroller->isFinished()) mScroller->abortAnimation();
        scrollBy(dx, dy);
    }
    mLastScroll = SystemClock::uptimeMillis();
}

void HorizontalScrollView::smoothScrollTo(int x, int y){
    smoothScrollBy(x - mScrollX, y - mScrollY);
}

int HorizontalScrollView::computeHorizontalScrollRange() {
    const int count = getChildCount();
    const int contentWidth = getWidth() - mPaddingLeft - mPaddingRight;
    if (count == 0) {
        return contentWidth;
    }

    int scrollRange = getChildAt(0)->getRight();
    const int scrollX = mScrollX;
    const int overscrollRight = std::max(0, scrollRange - contentWidth);
    if (scrollX < 0) {
        scrollRange -= scrollX;
    } else if (scrollX > overscrollRight) {
        scrollRange += scrollX - overscrollRight;
    }

    return scrollRange;
}

int HorizontalScrollView::computeHorizontalScrollOffset() {
    return std::max(0, FrameLayout::computeHorizontalScrollOffset());
}

void HorizontalScrollView::measureChild(View* child, int parentWidthMeasureSpec,
        int parentHeightMeasureSpec) {
    LayoutParams* lp = (LayoutParams*)child->getLayoutParams();

    const int horizontalPadding = mPaddingLeft + mPaddingRight;
    const int childWidthMeasureSpec = MeasureSpec::makeSafeMeasureSpec(
        std::max(0, MeasureSpec::getSize(parentWidthMeasureSpec) - horizontalPadding),
        MeasureSpec::UNSPECIFIED);

    const int childHeightMeasureSpec = getChildMeasureSpec(parentHeightMeasureSpec,
                mPaddingTop + mPaddingBottom, lp->height);
    child->measure(childWidthMeasureSpec, childHeightMeasureSpec);
}

void HorizontalScrollView::measureChildWithMargins(View* child, int parentWidthMeasureSpec, int widthUsed,
            int parentHeightMeasureSpec, int heightUsed) {
    MarginLayoutParams* lp = (MarginLayoutParams*) child->getLayoutParams();

    const int childHeightMeasureSpec = getChildMeasureSpec(parentHeightMeasureSpec,
            mPaddingTop + mPaddingBottom + lp->topMargin + lp->bottomMargin+ heightUsed, lp->height);

    const int usedTotal = mPaddingLeft + mPaddingRight + lp->leftMargin + lp->rightMargin + widthUsed;
    const int childWidthMeasureSpec = MeasureSpec::makeSafeMeasureSpec(
                    std::max(0, MeasureSpec::getSize(parentWidthMeasureSpec) - usedTotal),
                    MeasureSpec::UNSPECIFIED);

    child->measure(childWidthMeasureSpec, childHeightMeasureSpec);
}

void HorizontalScrollView::computeScroll(){
    if (mScroller->computeScrollOffset()) {
        const int oldX = mScrollX;
        const int oldY = mScrollY;
        const int x = mScroller->getCurrX();
        const int y = mScroller->getCurrY();
        const int deltaX = consumeFlingInStretch(x-oldX);
        if ((oldX != x) || (oldY != y)) {
            const int range = getScrollRange();
            const int overscrollMode = getOverScrollMode();
            const bool canOverscroll = overscrollMode == OVER_SCROLL_ALWAYS ||
                        (overscrollMode == OVER_SCROLL_IF_CONTENT_SCROLLS && range > 0);

            overScrollBy(x - oldX, y - oldY, oldX, oldY, range, 0, mOverflingDistance, 0, false);
            onScrollChanged(mScrollX, mScrollY, oldX, oldY);

            if (canOverscroll && (deltaX !=0)) {
                if (x < 0 && oldX >= 0) {
                    mEdgeGlowLeft->onAbsorb((int) mScroller->getCurrVelocity());
                } else if (x > range && oldX <= range) {
                    mEdgeGlowRight->onAbsorb((int) mScroller->getCurrVelocity());
                }
             }
        }

        if (!awakenScrollBars()) {
            postInvalidateOnAnimation();
        }
        // For variable refresh rate project to track the current velocity of this View
        if (true/*viewVelocityApi()*/) {
            setFrameContentVelocity(std::abs(mScroller->getCurrVelocity()));
        }
    }
}

/**
 * Used by consumeFlingInHorizontalStretch() and consumeFlinInVerticalStretch() for
 * consuming deltas from EdgeEffects
 * @param unconsumed The unconsumed delta that the EdgeEffets may consume
 * @return The unconsumed delta after the EdgeEffects have had an opportunity to consume.
 */
int HorizontalScrollView::consumeFlingInStretch(int unconsumed) {
    const int scrollX = getScrollX();
    if((scrollX < 0)||(scrollX>getScrollRange())){
        return unconsumed;
    }
    if (unconsumed > 0 && mEdgeGlowLeft && mEdgeGlowLeft->getDistance() != 0.f) {
        const int size = getWidth();
        const float deltaDistance = -unconsumed * FLING_DESTRETCH_FACTOR / size;
        const int consumed = std::round(float(-size) / FLING_DESTRETCH_FACTOR
                * mEdgeGlowLeft->onPullDistance(deltaDistance, 0.5f));
        if (consumed != unconsumed) {
            mEdgeGlowLeft->finish();
         }
        return unconsumed - consumed;
    }
    if (unconsumed < 0 && mEdgeGlowRight && mEdgeGlowRight->getDistance() != .0f) {
        const int size = getWidth();
        const float deltaDistance = unconsumed * FLING_DESTRETCH_FACTOR / size;
        const int consumed = std::round(float(size) / FLING_DESTRETCH_FACTOR
                * mEdgeGlowRight->onPullDistance(deltaDistance, 0.5f));
        if (consumed != unconsumed) {
            mEdgeGlowRight->finish();
        }
        return unconsumed - consumed;
    }
    return unconsumed;
}

void HorizontalScrollView::scrollToChild(View* child) {
    Rect mTempRect;
    child->getDrawingRect(mTempRect);

    /* Offset from child's local coordinates to ScrollView coordinates */
    offsetDescendantRectToMyCoords(child, mTempRect);

    const int scrollDelta = computeScrollDeltaToGetChildRectOnScreen(mTempRect);

    if (scrollDelta != 0) {
        scrollBy(scrollDelta, 0);
    }
}

bool HorizontalScrollView::scrollToChildRect(const Rect& rect, bool immediate){
    const int delta = computeScrollDeltaToGetChildRectOnScreen(rect);
    const bool scroll = delta != 0;
    if (scroll) {
        if (immediate) {
            scrollBy(delta, 0);
        } else {
            smoothScrollBy(delta, 0);
        }
    }
    return scroll;
}

int HorizontalScrollView::computeScrollDeltaToGetChildRectOnScreen(const Rect& rect){
    if (getChildCount() == 0) return 0;

    const int width = getWidth();
    int screenLeft = getScrollX();
    int screenRight = screenLeft + width;

    const int fadingEdge = getHorizontalFadingEdgeLength();

    // leave room for left fading edge as long as rect isn't at very left
    if (rect.left > 0) {
        screenLeft += fadingEdge;
    }

    // leave room for right fading edge as long as rect isn't at very right
    if (rect.right() < getChildAt(0)->getWidth()) {
        screenRight -= fadingEdge;
    }

    int scrollXDelta = 0;

    if (rect.right() > screenRight && rect.left > screenLeft) {
        // need to move right to get it in view: move right just enough so
        // that the entire rectangle is in view (or at least the first
        // screen size chunk).

        if (rect.width > width) {
            // just enough to get screen size chunk on
            scrollXDelta += (rect.left - screenLeft);
        } else {
            // get entire rect at right of screen
            scrollXDelta += (rect.right() - screenRight);
        }

        // make sure we aren't scrolling beyond the end of our content
        const int right = getChildAt(0)->getRight();
        const int distanceToRight = right - screenRight;
        scrollXDelta = std::min(scrollXDelta, distanceToRight);

    } else if (rect.left < screenLeft && rect.right() < screenRight) {
        // need to move right to get it in view: move right just enough so that
        // entire rectangle is in view (or at least the first screen
        // size chunk of it).

        if (rect.width > width) {
            // screen size chunk
            scrollXDelta -= (screenRight - rect.right());
        } else {
            // entire rect at left
            scrollXDelta -= (screenLeft - rect.left);
        }

        // make sure we aren't scrolling any further than the left our content
        scrollXDelta = std::max(scrollXDelta, -getScrollX());
    }
    return scrollXDelta;
}

void HorizontalScrollView::requestChildFocus(View* child, View* focused){
     if (focused != nullptr && focused->getRevealOnFocusHint()) {
        if (!mIsLayoutDirty) {
            scrollToChild(focused);
        } else {
            // The child may not be laid out yet, we can't compute the scroll yet
            mChildToScrollTo = focused;
        }
    }
    FrameLayout::requestChildFocus(child, focused);
}

bool HorizontalScrollView::onRequestFocusInDescendants(int direction,Rect*previouslyFocusedRect){
    if (direction == View::FOCUS_FORWARD) {
        direction = View::FOCUS_RIGHT;
    } else if (direction == View::FOCUS_BACKWARD) {
        direction = View::FOCUS_LEFT;
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

    return nextFocus->requestFocus(direction,previouslyFocusedRect);
}

bool HorizontalScrollView::requestChildRectangleOnScreen(View* child, Rect rectangle,bool immediate){
     rectangle.offset(child->getLeft() - child->getScrollX(),
            child->getTop() - child->getScrollY());

    return scrollToChildRect(rectangle, immediate);
}

void HorizontalScrollView::requestLayout() {
    mIsLayoutDirty = true;
    FrameLayout::requestLayout();
}

void HorizontalScrollView::onLayout(bool changed, int l, int t, int w, int h){
    int childWidth = 0;
    int childMargins = 0;

    if (getChildCount() > 0) {
        childWidth = getChildAt(0)->getMeasuredWidth();
        LayoutParams* childParams = (LayoutParams*) getChildAt(0)->getLayoutParams();
        childMargins = childParams->leftMargin + childParams->rightMargin;
    }

    const int available = w-getPaddingLeftWithForeground() - getPaddingRightWithForeground() - childMargins;

    const bool forceLeftGravity = (childWidth > available);

    layoutChildren(l, t, w, h, forceLeftGravity);

    mIsLayoutDirty = false;
    // Give a child focus if it needs it
    if (mChildToScrollTo != nullptr && isViewDescendantOf(mChildToScrollTo, this)) {
        scrollToChild(mChildToScrollTo);
    }
    mChildToScrollTo = nullptr;

    if (!isLaidOut()) {
        const int scrollRange = std::max(0, childWidth - (w - mPaddingLeft - mPaddingRight));
        if (mSavedState != nullptr) {
            mScrollX = isLayoutRtl()
                    ? scrollRange - mSavedState->scrollOffsetFromStart
                    : mSavedState->scrollOffsetFromStart;
            mSavedState = nullptr;
        } else {
            if (isLayoutRtl()) {
                mScrollX = scrollRange - mScrollX;
            } // mScrollX default value is "0" for LTR
        }
        // Don't forget to clamp
        if (mScrollX > scrollRange) {
            mScrollX = scrollRange;
        } else if (mScrollX < 0) {
            mScrollX = 0;
        }
    }

    // Calling this with the present values causes it to re-claim them
    scrollTo(mScrollX, mScrollY);
}

void HorizontalScrollView::onSizeChanged(int w, int h, int oldw, int oldh) {
    FrameLayout::onSizeChanged(w, h, oldw, oldh);

    View* currentFocused = findFocus();
    if (nullptr == currentFocused || this == currentFocused)
        return;

    const int maxJump = mRight - mLeft;

    if (isWithinDeltaOfScreen(currentFocused, maxJump)) {
        Rect mTempRect;
        currentFocused->getDrawingRect(mTempRect);
        offsetDescendantRectToMyCoords(currentFocused, mTempRect);
        int scrollDelta = computeScrollDeltaToGetChildRectOnScreen(mTempRect);
        doScrollX(scrollDelta);
    }
}

bool HorizontalScrollView::isViewDescendantOf(View* child, View* parent){
    if (child == parent) {
        return true;
    }

    ViewGroup* theParent = child->getParent();
    return theParent && isViewDescendantOf(theParent, parent);
}

void HorizontalScrollView::fling(int velocityX){
    if (getChildCount() > 0) {
        const int width = getWidth() - mPaddingRight - mPaddingLeft;
        const int right = getChildAt(0)->getWidth() - mPaddingLeft;
        const int maxScroll =std::max(0,right-width);
        bool shouldFling = false;
        if(mScrollX==0 && !mEdgeGlowLeft->isFinished()){
            if (shouldAbsorb(mEdgeGlowLeft, -velocityX)) {
                mEdgeGlowLeft->onAbsorb(-velocityX);
            } else {
                shouldFling = true;
            }
        }else if((mScrollX==maxScroll) && !mEdgeGlowRight->isFinished()){
            if (shouldAbsorb(mEdgeGlowRight, velocityX)) {
                mEdgeGlowRight->onAbsorb(velocityX);
            } else {
                shouldFling = true;
            }
            if(shouldDisplayEdgeEffects())
            mEdgeGlowRight->onAbsorb(velocityX);
        }else{
            shouldFling = true;
        }
        if(shouldFling){
            mScroller->fling(mScrollX, mScrollY, velocityX, 0, 0,std::max(0, right - width), 0, 0, width/2, 0);

            // For variable refresh rate project to track the current velocity of this View
            if (true/*viewVelocityApi()*/) {
                setFrameContentVelocity(std::abs(mScroller->getCurrVelocity()));
            }
            const bool movingRight = velocityX > 0;

            View* currentFocused = findFocus();
            View* newFocused = findFocusableViewInMyBounds(movingRight, mScroller->getFinalX(), currentFocused);

            if (newFocused == nullptr) newFocused = this;
            if (newFocused != currentFocused) {
                newFocused->requestFocus(movingRight ? View::FOCUS_RIGHT : View::FOCUS_LEFT);
            }
        }
        postInvalidateOnAnimation();
    }
}

bool HorizontalScrollView::shouldAbsorb(EdgeEffect* edgeEffect, int velocity) {
    if (velocity > 0) {
        return true;
    }
    const float distance = edgeEffect->getDistance() * getWidth();

    // This is flinging without the spring, so let's see if it will fling past the overscroll
    const float flingDistance = (float) mScroller->getSplineFlingDistance(-velocity);

    return flingDistance < distance;
}

static int clamp(int n, int my, int child) {
    if (my >= child || n < 0) {
        return 0;
    }
    if ((my + n) > child) {
        return child - my;
    }
    return n;
}

void HorizontalScrollView::scrollTo(int x, int y){
    if (getChildCount() > 0) {
        View* child = getChildAt(0);
        x = clamp(x, getWidth() - mPaddingRight - mPaddingLeft, child->getWidth());
        y = clamp(y, getHeight() - mPaddingBottom - mPaddingTop, child->getHeight());
        if (x != mScrollX || y != mScrollY) {
            FrameLayout::scrollTo(x, y);
        }
    }
}

bool HorizontalScrollView::shouldDisplayEdgeEffects()const{
    return getOverScrollMode() != OVER_SCROLL_NEVER;
}

void HorizontalScrollView::draw(Canvas& canvas){
    FrameLayout::draw(canvas);
    if (shouldDisplayEdgeEffects()){
        const int scrollX = mScrollX;
        if (!mEdgeGlowLeft->isFinished()) {
            const int height = getHeight() - mPaddingTop - mPaddingBottom;

            canvas.save();
            canvas.rotate_degrees(270);
            canvas.translate(-height + mPaddingTop, std::min(0, scrollX));
            mEdgeGlowLeft->setSize(height, getWidth());
            if (mEdgeGlowLeft->draw(canvas)) {
                postInvalidateOnAnimation();
            }
            canvas.restore();
        }
        if (!mEdgeGlowRight->isFinished()) {
            const int width = getWidth();
            const int height = getHeight() - mPaddingTop - mPaddingBottom;

            canvas.save();
            canvas.rotate_degrees(90);
            canvas.translate(-mPaddingTop, -(std::max(getScrollRange(), scrollX) + width));
            mEdgeGlowRight->setSize(height, width);
            if (mEdgeGlowRight->draw(canvas)) {
                postInvalidateOnAnimation();
            }
            canvas.restore();
        }
    }
}

void HorizontalScrollView::onRestoreInstanceState(Parcelable& state) {
    /*if (mContext.getApplicationInfo().targetSdkVersion <= Build.VERSION_CODES.JELLY_BEAN_MR2) {
        // Some old apps reused IDs in ways they shouldn't have.
        // Don't break them, but they don't get scroll state restoration.
        FrameLayout::onRestoreInstanceState(state);
        return;
    }*/
    SavedState* ss = (SavedState*) &state;
    FrameLayout::onRestoreInstanceState(*ss->getSuperState());
    mSavedState = ss;
    requestLayout();
}

Parcelable* HorizontalScrollView::onSaveInstanceState() {
    /*if (mContext.getApplicationInfo().targetSdkVersion <= Build.VERSION_CODES.JELLY_BEAN_MR2) {
        // Some old apps reused IDs in ways they shouldn't have.
        // Don't break them, but they don't get scroll state restoration.
        return FrameLayout::onSaveInstanceState();
    }*/
    Parcelable* superState = FrameLayout::onSaveInstanceState();
    SavedState* ss = new SavedState(superState);
    ss->scrollOffsetFromStart = isLayoutRtl() ? -mScrollX : mScrollX;
    return ss;
}
}//namespace 
