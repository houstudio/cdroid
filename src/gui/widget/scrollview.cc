#include <widget/scrollview.h>
#include <focusfinder.h>
#include <cdlog.h>
namespace cdroid {

DECLARE_WIDGET2(ScrollView,"cdroid:attr/scrollViewStyle")

ScrollView::ScrollView(int w,int h):FrameLayout(w,h){
    initScrollView();
    mIsBeingDragged=false;
    mActivePointerId=INVALID_POINTER;
}

ScrollView::ScrollView(Context*ctx,const AttributeSet&atts)
  :FrameLayout(ctx,atts){
    initScrollView();
}

ScrollView::~ScrollView(){
    delete mScroller;
}

float ScrollView::getTopFadingEdgeStrength() {
    if (getChildCount() == 0) {
        return 0.0f;
    }

    int length = getVerticalFadingEdgeLength();
    if (mScrollY < length) {
        return mScrollY / (float) length;
    }

    return 1.0f;
}

float ScrollView::getBottomFadingEdgeStrength() {
    if (getChildCount() == 0) {
        return 0.0f;
    }

    int length = getVerticalFadingEdgeLength();
    int bottomEdge = getHeight() - mPaddingBottom;
    int span = getChildAt(0)->getBottom() - mScrollY - bottomEdge;
    if (span < length) {
        return span / (float) length;
    }

    return 1.0f;
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
    mVelocityTracker = nullptr;
    mEdgeGlowTop = mEdgeGlowBottom =nullptr;
    ViewConfiguration& configuration = ViewConfiguration::get(mContext);
    mTouchSlop = configuration.getScaledTouchSlop();
    mMinimumVelocity = configuration.getScaledMinimumFlingVelocity();
    mMaximumVelocity = configuration.getScaledMaximumFlingVelocity();
    mOverscrollDistance= configuration.getScaledOverscrollDistance();
    mOverflingDistance = configuration.getScaledOverflingDistance();
    mVerticalScrollFactor= configuration.getScaledVerticalScrollFactor();
    mScrollOffset[0]   = mScrollOffset[1] = 0;
    mScrollConsumed[0] = mScrollConsumed[1] = 0;
}

View& ScrollView::addView(View* child) {
    if (getChildCount() > 0) {
        LOGE("ScrollView can host only one direct child");
    }
    return FrameLayout::addView(child);
}

View& ScrollView::addView(View* child, int index) {
    if (getChildCount() > 0) {
        LOGE("ScrollView can host only one direct child");
    }
    return FrameLayout::addView(child, index);
}

View& ScrollView::addView(View* child, ViewGroup::LayoutParams* params) {
    if (getChildCount() > 0) {
        LOGE("ScrollView can host only one direct child");
    }
    return FrameLayout::addView(child, params);
}

View& ScrollView::addView(View* child, int index, ViewGroup::LayoutParams* params) {
    if (getChildCount() > 0) {
        LOGE("ScrollView can host only one direct child");
    }

    return FrameLayout::addView(child, index, params);
}

bool ScrollView::canScroll() {
    View* child = getChildAt(0);
    if (child != nullptr) {
        int childHeight = child->getHeight();
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

    int heightMode = MeasureSpec::getMode(heightMeasureSpec);
    if (heightMode == MeasureSpec::UNSPECIFIED) {
        return;
    }

    if (getChildCount() > 0) {
        View* child = getChildAt(0);
        int widthPadding;
        int heightPadding;
        LayoutParams* lp = (LayoutParams*) child->getLayoutParams();

        widthPadding = mPaddingLeft + mPaddingRight + lp->leftMargin + lp->rightMargin;
        heightPadding = mPaddingTop + mPaddingBottom + lp->topMargin + lp->bottomMargin;

        int desiredHeight = getMeasuredHeight() - heightPadding;
        if (child->getMeasuredHeight() < desiredHeight) {
            int childWidthMeasureSpec = getChildMeasureSpec(widthMeasureSpec, widthPadding, lp->width);
            int childHeightMeasureSpec = MeasureSpec::makeMeasureSpec(desiredHeight, MeasureSpec::EXACTLY);
            child->measure(childWidthMeasureSpec, childHeightMeasureSpec);
        }
    }
}

bool ScrollView::executeKeyEvent(KeyEvent& event) {
    if (!canScroll()) {
        if (isFocused() && event.getKeyCode() !=KEY_BACK) {
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
        case KEY_DPAD_UP:
            if (!event.isAltPressed()) {
                handled = arrowScroll(View::FOCUS_UP);
            } else {
                handled = fullScroll(View::FOCUS_UP);
            }
            break;
        case KEY_DPAD_DOWN:
            if (!event.isAltPressed()) {
                handled = arrowScroll(View::FOCUS_DOWN);
            } else {
                handled = fullScroll(View::FOCUS_DOWN);
            }
            break;
        case KEY_SPACE:
            pageScroll(event.isShiftPressed() ? View::FOCUS_UP : View::FOCUS_DOWN);
            break;
        }
    }
    return handled;
}

bool ScrollView::inChild(int x, int y) {
    if (getChildCount() > 0) {
        View* child = getChildAt(0);
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

void ScrollView::recycleVelocityTracker() {
        if (mVelocityTracker != nullptr) {
            mVelocityTracker->recycle();
            mVelocityTracker = nullptr;
        }
    }

bool ScrollView::onInterceptTouchEvent(MotionEvent& ev) {
    int action = ev.getAction();
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
        int activePointerId = mActivePointerId;
        if (activePointerId == INVALID_POINTER) {
            // If we don't have a valid id, the touch down wasn't on content.
            break;
        }

        int pointerIndex = ev.findPointerIndex(activePointerId);
        if (pointerIndex == -1) {
            LOGE("Invalid pointerId=%d  in onInterceptTouchEvent",activePointerId);
            break;
        }
        int y = (int) ev.getY(pointerIndex);
        int yDiff = std::abs(y - mLastMotionY);
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
        mIsBeingDragged = !mScroller->isFinished();
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

bool ScrollView::onTouchEvent(MotionEvent& ev) {
    initVelocityTrackerIfNotExists();

    MotionEvent* vtev=MotionEvent::obtain(ev);

    int actionMasked = ev.getActionMasked();

    if (actionMasked == MotionEvent::ACTION_DOWN) {
        mNestedYOffset = 0;
    }
    vtev->offsetLocation(0, mNestedYOffset);

    switch (actionMasked) {
    case MotionEvent::ACTION_DOWN: {
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
    }
    case MotionEvent::ACTION_MOVE: {
        int activePointerIndex = ev.findPointerIndex(mActivePointerId);
        if (activePointerIndex == -1) {
            LOGE("Invalid pointerId=%d in onTouchEvent",mActivePointerId);
            break;
        }

        int y = (int) ev.getY(activePointerIndex);
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
        if (mIsBeingDragged) {
            // Scroll to follow the motion event
            mLastMotionY = y - mScrollOffset[1];

            int oldY = mScrollY;
            int range = getScrollRange();
            int overscrollMode = getOverScrollMode();
            bool canOverscroll = overscrollMode == OVER_SCROLL_ALWAYS ||
                                 (overscrollMode == OVER_SCROLL_IF_CONTENT_SCROLLS && range > 0);

            // Calling overScrollBy will call onOverScrolled, which
            // calls onScrollChanged if applicable.
            if (overScrollBy(0, deltaY, 0, mScrollY, 0, range, 0, mOverscrollDistance, true)
                    && !hasNestedScrollingParent()) {
                // Break our velocity if we hit a scroll barrier.
                mVelocityTracker->clear();
            }

            int scrolledDeltaY = mScrollY - oldY;
            int unconsumedY = deltaY - scrolledDeltaY;
            if (dispatchNestedScroll(0, scrolledDeltaY, 0, unconsumedY, mScrollOffset)) {
                mLastMotionY -= mScrollOffset[1];
                vtev->offsetLocation(0, mScrollOffset[1]);
                mNestedYOffset += mScrollOffset[1];
            } else if (canOverscroll) {
                int pulledToY = oldY + deltaY;
                if (pulledToY < 0) {
                    mEdgeGlowTop->onPull((float) deltaY / getHeight(),ev.getX(activePointerIndex) / getWidth());
                    if (!mEdgeGlowBottom->isFinished()) mEdgeGlowBottom->onRelease();
                } else if (pulledToY > range) {
                    mEdgeGlowBottom->onPull((float) deltaY / getHeight(),1.f - ev.getX(activePointerIndex) / getWidth());
                    if(!mEdgeGlowTop->isFinished()) mEdgeGlowTop->onRelease();
                }
                if (mEdgeGlowTop && (!mEdgeGlowTop->isFinished() || !mEdgeGlowBottom->isFinished())) {
                    postInvalidateOnAnimation();
                }
            }
        }
    } break;
    case MotionEvent::ACTION_UP:
        if (mIsBeingDragged) {
            VelocityTracker* velocityTracker = mVelocityTracker;
            velocityTracker->computeCurrentVelocity(1000, mMaximumVelocity);
            int initialVelocity = (int) velocityTracker->getYVelocity(mActivePointerId);

            if ((std::abs(initialVelocity) > mMinimumVelocity)) {
                flingWithNestedDispatch(-initialVelocity);
            } else if (mScroller->springBack(mScrollX, mScrollY, 0, 0, 0,getScrollRange())) {
                postInvalidateOnAnimation();
            }
            mActivePointerId = INVALID_POINTER;
            endDrag();
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
        int index = ev.getActionIndex();
        mLastMotionY = (int) ev.getY(index);
        mActivePointerId = ev.getPointerId(index);
        LOGD("POINTER_DOWN mActivePointerId=%d",mActivePointerId);
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
    int pointerIndex = (ev.getAction() & MotionEvent::ACTION_POINTER_INDEX_MASK) >>
                       MotionEvent::ACTION_POINTER_INDEX_SHIFT;
    int pointerId = ev.getPointerId(pointerIndex);
    if (pointerId == mActivePointerId) {
        // This was our active pointer going up. Choose a new
        // active pointer and adjust accordingly.
        // TODO: Make this decision more intelligent.
        int newPointerIndex = pointerIndex == 0 ? 1 : 0;
        mLastMotionY = (int) ev.getY(newPointerIndex);
        mActivePointerId = ev.getPointerId(newPointerIndex);
        if (mVelocityTracker) mVelocityTracker->clear();
    }
}

bool ScrollView::onGenericMotionEvent(MotionEvent& event) {
    return false;
}

void ScrollView::onOverScrolled(int scrollX, int scrollY, bool clampedX, bool clampedY) {
    if (mScroller->isFinished()) {
        int oldX = mScrollX;
        int oldY = mScrollY;
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

int ScrollView::getScrollRange() {
    int scrollRange = 0;
    if (getChildCount() > 0) {
        View* child = getChildAt(0);
        scrollRange = std::max(0,child->getHeight() - (getHeight() - mPaddingBottom - mPaddingTop));
    }
    return scrollRange;
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

            bool viewIsFullyContained = (top < viewTop) && (viewBottom < bottom);

            if (focusCandidate == nullptr) {
                /* No candidate, take this one */
                focusCandidate = view;
                foundFullyContainedFocusable = viewIsFullyContained;
            } else {
                bool viewIsCloserToBoundary =
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
    bool down = direction == View::FOCUS_DOWN;
    int height = getHeight();

    if (down) {
        mTempRect.top = getScrollY() + height;
        int count = getChildCount();
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
    bool down = direction == View::FOCUS_DOWN;
    int height = getHeight();

    mTempRect.top = 0;
    mTempRect.height = height;

    if (down) {
        int count = getChildCount();
        if (count > 0) {
            View* view = getChildAt(count - 1);
            mTempRect.top = mTempRect.bottom() - height;
            mTempRect.height = view->getBottom() + mPaddingBottom-mTempRect.top;
        }
    }
    return scrollAndFocus(direction, mTempRect.top, mTempRect.bottom());
}

bool ScrollView::scrollAndFocus(int direction, int top, int bottom) {
    bool handled = true;

    int height = getHeight();
    int containerTop = getScrollY();
    int containerBottom = containerTop + height;
    bool up = direction == View::FOCUS_UP;

    View* newFocused = findFocusableViewInBounds(up, top, bottom);
    if (newFocused == nullptr) {
        newFocused = this;
    }

    if (top >= containerTop && bottom <= containerBottom) {
        handled = false;
    } else {
        int delta = up ? (top - containerTop) : (bottom - containerBottom);
        doScrollY(delta);
    }
    if (newFocused != findFocus()) newFocused->requestFocus(direction);
    return handled;
}

bool ScrollView::arrowScroll(int direction) {
    View* currentFocused = findFocus();
    if (currentFocused == this) currentFocused = nullptr;

    View* nextFocused = FocusFinder::getInstance().findNextFocus(this, currentFocused, direction);

    int maxJump = getMaxScrollAmount();

    if (nextFocused != nullptr && isWithinDeltaOfScreen(nextFocused, maxJump, getHeight())) {
        nextFocused->getDrawingRect(mTempRect);
        offsetDescendantRectToMyCoords(nextFocused, mTempRect);
        int scrollDelta = computeScrollDeltaToGetChildRectOnScreen(mTempRect);
        doScrollY(scrollDelta);
        nextFocused->requestFocus(direction);
    } else {
        // no new focus
        int scrollDelta = maxJump;

        if (direction == View::FOCUS_UP && getScrollY() < scrollDelta) {
            scrollDelta = getScrollY();
        } else if (direction == View::FOCUS_DOWN) {
            if (getChildCount() > 0) {
                int daBottom = getChildAt(0)->getBottom();
                int screenBottom = getScrollY() + getHeight() - mPaddingBottom;
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

    if (currentFocused != nullptr && currentFocused->isFocused()
            && isOffScreen(currentFocused)) {
        // previously focused item still has focus and is off screen, give
        // it up (take it back to ourselves)
        // (also, need to temporarily force FOCUS_BEFORE_DESCENDANTS so we are
        // sure to
        // get it)
        int descendantFocusability = getDescendantFocusability();  // save
        setDescendantFocusability(ViewGroup::FOCUS_BEFORE_DESCENDANTS);
        requestFocus();
        setDescendantFocusability(descendantFocusability);  // restore
    }
    return true;
}

bool ScrollView::isOffScreen(View* descendant) {
    return !isWithinDeltaOfScreen(descendant, 0, getHeight());
}

bool ScrollView::isWithinDeltaOfScreen(View* descendant, int delta, int height) {
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
    long duration =SystemClock::uptimeMillis() - mLastScroll;
    if (getChildCount() == 0) return;
    if (duration > ANIMATED_SCROLL_GAP) {
        int height = getHeight() - mPaddingBottom - mPaddingTop;
        int bottom = getChildAt(0)->getHeight();
        int maxY = std::max(0, bottom - height);
        int scrollY = mScrollY;
        dy = std::max(0, std::min(scrollY + dy, maxY)) - scrollY;

        mScroller->startScroll(mScrollX, scrollY, 0, dy);
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
    int count = getChildCount();
    int contentHeight = getHeight() - mPaddingBottom - mPaddingTop;
    if (count == 0) {
        return contentHeight;
    }

    int scrollRange = getChildAt(0)->getBottom();
    int scrollY = mScrollY;
    int overscrollBottom = std::max(0, scrollRange - contentHeight);
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
    LayoutParams* lp = (LayoutParams*)child->getLayoutParams();

    int childWidthMeasureSpec;
    int childHeightMeasureSpec;

    childWidthMeasureSpec = getChildMeasureSpec(parentWidthMeasureSpec, mPaddingLeft
                            + mPaddingRight, lp->width);
    int verticalPadding = mPaddingTop + mPaddingBottom;
    childHeightMeasureSpec = MeasureSpec::makeSafeMeasureSpec(
                                 std::max(0, MeasureSpec::getSize(parentHeightMeasureSpec) - verticalPadding),
                                 MeasureSpec::UNSPECIFIED);
    child->measure(childWidthMeasureSpec, childHeightMeasureSpec);
}

void ScrollView::measureChildWithMargins(View* child, int parentWidthMeasureSpec, int widthUsed,
        int parentHeightMeasureSpec, int heightUsed) {
    MarginLayoutParams* lp = (MarginLayoutParams*) child->getLayoutParams();

    int childWidthMeasureSpec = getChildMeasureSpec(parentWidthMeasureSpec,
                               mPaddingLeft + mPaddingRight + lp->leftMargin + lp->rightMargin
                               + widthUsed, lp->width);
    int usedTotal = mPaddingTop + mPaddingBottom + lp->topMargin + lp->bottomMargin +  heightUsed;
    int childHeightMeasureSpec = MeasureSpec::makeSafeMeasureSpec(
                                std::max(0, MeasureSpec::getSize(parentHeightMeasureSpec) - usedTotal),
                                MeasureSpec::UNSPECIFIED);

    child->measure(childWidthMeasureSpec, childHeightMeasureSpec);
}

void ScrollView::computeScroll() {

}

void ScrollView::scrollToChild(View* child) {
    child->getDrawingRect(mTempRect);

    /* Offset from child's local coordinates to ScrollView coordinates */
    offsetDescendantRectToMyCoords(child, mTempRect);

    int scrollDelta = computeScrollDeltaToGetChildRectOnScreen(mTempRect);

    if (scrollDelta != 0) {
        scrollBy(0, scrollDelta);
    }
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
    int oldScrollY = mScrollY;
    scrollBy(0, dyUnconsumed);
    int myConsumed = mScrollY - oldScrollY;
    int myUnconsumed = dyUnconsumed - myConsumed;
    dispatchNestedScroll(0, myConsumed, 0, myUnconsumed, nullptr);
}

bool ScrollView::onNestedFling(View* target, float velocityX, float velocityY, bool consumed){
    if (!consumed) {
        flingWithNestedDispatch((int) velocityY);
        return true;
    }
    return false;
}

bool ScrollView::scrollToChildRect(Rect& rect, bool immediate){
    int delta = computeScrollDeltaToGetChildRectOnScreen(rect);
     bool scroll = delta != 0;
    if (scroll) {
        if (immediate) {
            scrollBy(0, delta);
        } else {
            smoothScrollBy(0, delta);
        }
    }
    return scroll;
}

int ScrollView::computeScrollDeltaToGetChildRectOnScreen(Rect& rect){
     if (getChildCount() == 0) return 0;

        int height = getHeight();
        int screenTop = getScrollY();
        int screenBottom = screenTop + height;

        int fadingEdge = getVerticalFadingEdgeLength();

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
            scrollToChild(focused);
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
    if (mChildToScrollTo != nullptr && isViewDescendantOf(mChildToScrollTo, this)) {
        scrollToChild(mChildToScrollTo);
    }
    mChildToScrollTo = nullptr;

    if (!isLaidOut()) {
        /*if (mSavedState != nullptr) {
            mScrollY = mSavedState.scrollPosition;
            mSavedState = nullptr;
        } */// mScrollY default value is "0"
        int childHeight = (getChildCount() > 0) ? getChildAt(0)->getMeasuredHeight() : 0;
        int scrollRange = std::max(0, childHeight - (h - mPaddingBottom - mPaddingTop));

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

bool ScrollView::isViewDescendantOf(View* child, View* parent){
    if (child == parent) {
        return true;
    }

    ViewGroup* theParent = child->getParent();
    return dynamic_cast<ViewGroup*>(theParent) && isViewDescendantOf(theParent, parent);
}

void ScrollView::fling(int velocityY) {
    if (getChildCount() > 0) {
        int height = getHeight() - mPaddingBottom - mPaddingTop;
        int bottom = getChildAt(0)->getHeight();

        mScroller->fling(mScrollX, mScrollY, 0, velocityY, 0, 0, 0,
                         std::max(0, bottom - height), 0, height/2);

        /*if (mFlingStrictSpan == null) {
            mFlingStrictSpan = StrictMode.enterCriticalSpan("ScrollView-fling");
        }*/

        postInvalidateOnAnimation();
    }
}

void ScrollView::flingWithNestedDispatch(int velocityY) {
    bool canFling = (mScrollY > 0 || velocityY > 0) &&
        (mScrollY < getScrollRange() || velocityY < 0);
    if (!dispatchNestedPreFling(0, velocityY)) {
        dispatchNestedFling(0, velocityY, canFling);
        if (canFling) {
            fling(velocityY);
        }
    }
}

void ScrollView::endDrag() {
    mIsBeingDragged = false;

    recycleVelocityTracker();

    if (mEdgeGlowTop != nullptr) {
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
    if (mEdgeGlowTop != nullptr) {
        int scrollY = mScrollY;
        bool clipToPadding = getClipToPadding();
        if (!mEdgeGlowTop->isFinished()) {
            int width;
            int height;
            float translateX;
            float translateY;
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
            int width;
            int height;
            float translateX;
            float translateY;
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
            canvas.translate(-width + translateX,
                    std::max(getScrollRange(), scrollY) + height + translateY);
            canvas.rotate_degrees(180);//, width, 0);
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
       return child-my;
   }
   return n;
}

}
