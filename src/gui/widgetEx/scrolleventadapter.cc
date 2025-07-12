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
#include <widgetEx/scrolleventadapter.h>

namespace cdroid{

ScrollEventAdapter::ScrollEventAdapter(ViewPager2* viewPager) {
    mViewPager = viewPager;
    mRecyclerView = mViewPager->mRecyclerView;
    //noinspection ConstantConditions
    mLayoutManager = (LinearLayoutManager*) mRecyclerView->getLayoutManager();
    resetState();
}

void ScrollEventAdapter::resetState() {
    mAdapterState = STATE_IDLE;
    mScrollState = RecyclerView::SCROLL_STATE_IDLE;
    mScrollValues.reset();
    mDragStartPosition = NO_POSITION;
    mTarget = NO_POSITION;
    mDispatchSelected = false;
    mScrollHappened = false;
    mFakeDragging = false;
    mDataSetChangeHappened = false;
}

void ScrollEventAdapter::onScrollStateChanged(RecyclerView& recyclerView, int newState) {
    // User started a drag (not dragging -> dragging)
    if ((mAdapterState != STATE_IN_PROGRESS_MANUAL_DRAG
            || mScrollState !=  RecyclerView::SCROLL_STATE_DRAGGING)
            && newState == RecyclerView::SCROLL_STATE_DRAGGING) {
        startDrag(false);
        return;
    }

    if (isInAnyDraggingState() && (newState == RecyclerView::SCROLL_STATE_SETTLING)) {
        // Only go through the settling phase if the drag actually moved the page
        if (mScrollHappened) {
            dispatchStateChanged(RecyclerView::SCROLL_STATE_SETTLING);
            // Determine target page and dispatch onPageSelected on next scroll event
            mDispatchSelected = true;
        }
        return;
    }

    // Drag is finished (dragging || settling -> idle)
    if (isInAnyDraggingState() && (newState == RecyclerView::SCROLL_STATE_IDLE)) {
        bool dispatchIdle = false;
        updateScrollEventValues();
        if (!mScrollHappened) {
            if (mScrollValues.mPosition != RecyclerView::NO_POSITION) {
                dispatchScrolled(mScrollValues.mPosition, 0.f, 0);
            }
            dispatchIdle = true;
        } else if (mScrollValues.mOffsetPx == 0) {
            dispatchIdle = true;
            if (mDragStartPosition != mScrollValues.mPosition) {
                dispatchSelected(mScrollValues.mPosition);
            }
        }
        if (dispatchIdle) {
            dispatchStateChanged(RecyclerView::SCROLL_STATE_IDLE);
            resetState();
        }
    }

    if ((mAdapterState == STATE_IN_PROGRESS_SMOOTH_SCROLL)
            && (newState == RecyclerView::SCROLL_STATE_IDLE) && mDataSetChangeHappened) {
        updateScrollEventValues();
        if (mScrollValues.mOffsetPx == 0) {
            if (mTarget != mScrollValues.mPosition) {
                dispatchSelected( (mScrollValues.mPosition == NO_POSITION) ? 0 : mScrollValues.mPosition);
            }
            dispatchStateChanged(RecyclerView::SCROLL_STATE_IDLE);
            resetState();
        }
    }
}

void ScrollEventAdapter::onScrolled(RecyclerView& recyclerView, int dx, int dy) {
    mScrollHappened = true;
    updateScrollEventValues();

    if (mDispatchSelected) {
        // Drag started settling, need to calculate target page and dispatch onPageSelected now
        mDispatchSelected = false;
        bool scrollingForward = (dy > 0) || (dy == 0 && dx < 0 == mViewPager->isRtl());

        mTarget = (scrollingForward && (mScrollValues.mOffsetPx != 0))
                ? (mScrollValues.mPosition + 1) : mScrollValues.mPosition;
        if (mDragStartPosition != mTarget) {
            dispatchSelected(mTarget);
        }
    } else if (mAdapterState == STATE_IDLE) {
        int position = mScrollValues.mPosition;
        // Contract forbids us to send position = -1 though
        dispatchSelected(position == NO_POSITION ? 0 : position);
    }

    // If position = -1, there are no items. Contract says to send position = 0 instead.
    dispatchScrolled(mScrollValues.mPosition == NO_POSITION ? 0 : mScrollValues.mPosition,
            mScrollValues.mOffset, mScrollValues.mOffsetPx);

    if ((mScrollValues.mPosition == mTarget || mTarget == NO_POSITION)
            && mScrollValues.mOffsetPx == 0 && !(mScrollState == RecyclerView::SCROLL_STATE_DRAGGING)) {
        dispatchStateChanged(RecyclerView::SCROLL_STATE_IDLE);
        resetState();
    }
}

void ScrollEventAdapter::updateScrollEventValues() {
    ScrollEventValues& values = mScrollValues;

    values.mPosition = mLayoutManager->findFirstVisibleItemPosition();
    if (values.mPosition == RecyclerView::NO_POSITION) {
        values.reset();
        return;
    }
    View* firstVisibleView = mLayoutManager->findViewByPosition(values.mPosition);
    if (firstVisibleView == nullptr) {
        values.reset();
        return;
    }

    int leftDecorations = mLayoutManager->getLeftDecorationWidth(firstVisibleView);
    int rightDecorations = mLayoutManager->getRightDecorationWidth(firstVisibleView);
    int topDecorations = mLayoutManager->getTopDecorationHeight(firstVisibleView);
    int bottomDecorations = mLayoutManager->getBottomDecorationHeight(firstVisibleView);

    LayoutParams* params = firstVisibleView->getLayoutParams();
    if (dynamic_cast<MarginLayoutParams*>(params)) {
        MarginLayoutParams* margin = (MarginLayoutParams*) params;
        leftDecorations += margin->leftMargin;
        rightDecorations += margin->rightMargin;
        topDecorations += margin->topMargin;
        bottomDecorations += margin->bottomMargin;
    }

    int decoratedHeight = firstVisibleView->getHeight() + topDecorations + bottomDecorations;
    int decoratedWidth = firstVisibleView->getWidth() + leftDecorations + rightDecorations;

    const bool isHorizontal = mLayoutManager->getOrientation() == ViewPager2::ORIENTATION_HORIZONTAL;
    int start, sizePx;
    if (isHorizontal) {
        sizePx = decoratedWidth;
        start = firstVisibleView->getLeft() - leftDecorations - mRecyclerView->getPaddingLeft();
        if (mViewPager->isRtl()) {
            start = -start;
        }
    } else {
        sizePx = decoratedHeight;
        start = firstVisibleView->getTop() - topDecorations - mRecyclerView->getPaddingTop();
    }

    values.mOffsetPx = -start;
    if (values.mOffsetPx < 0) {
        // We're in an error state. Figure out if this might have been caused
        // by animateLayoutChanges and throw a descriptive exception if so
        /*if (new AnimateLayoutChangeDetector(mLayoutManager).mayHaveInterferingAnimations()) {
            FATAL("Page(s) contain a ViewGroup with a "
                    "LayoutTransition (or animateLayoutChanges=\"true\"), which interferes "
                    "with the scrolling animation. Make sure to call getLayoutTransition()"
                    ".setAnimateParentHierarchy(false) on all ViewGroups with a "
                    "LayoutTransition before an animation is started.");
        }*/
        // Throw a generic exception otherwise
        FATAL("Page can only be offset by a positive amount, not by %d", values.mOffsetPx);
    }
    values.mOffset = sizePx == 0 ? 0 : (float) values.mOffsetPx / sizePx;
}

void ScrollEventAdapter::startDrag(bool isFakeDrag) {
    mFakeDragging = isFakeDrag;
    mAdapterState = isFakeDrag ? STATE_IN_PROGRESS_FAKE_DRAG : STATE_IN_PROGRESS_MANUAL_DRAG;
    if (mTarget != NO_POSITION) {
        // Target was set means we were settling to that target
        // Update "drag start page" to reflect the page that ViewPager2 thinks it is at
        mDragStartPosition = mTarget;
        // Reset target because drags have no target until released
        mTarget = NO_POSITION;
    } else if (mDragStartPosition == NO_POSITION) {
        // ViewPager2 was at rest, set "drag start page" to current page
        mDragStartPosition = getPosition();
    }
    dispatchStateChanged(RecyclerView::SCROLL_STATE_DRAGGING);
}

void ScrollEventAdapter::notifyDataSetChangeHappened() {
    mDataSetChangeHappened = true;
}

void ScrollEventAdapter::notifyProgrammaticScroll(int target, bool smooth) {
    mAdapterState = smooth
            ? STATE_IN_PROGRESS_SMOOTH_SCROLL
            : STATE_IN_PROGRESS_IMMEDIATE_SCROLL;
    // mFakeDragging is true when a fake drag is interrupted by an a11y command
    // set it to false so endFakeDrag won't fling the RecyclerView
    mFakeDragging = false;
    bool hasNewTarget = mTarget != target;
    mTarget = target;
    dispatchStateChanged(RecyclerView::SCROLL_STATE_SETTLING);
    if (hasNewTarget) {
        dispatchSelected(target);
    }
}

void ScrollEventAdapter::notifyBeginFakeDrag() {
    mAdapterState = STATE_IN_PROGRESS_FAKE_DRAG;
    startDrag(true);
}

void ScrollEventAdapter::notifyEndFakeDrag() {
    if (isDragging() && !mFakeDragging) {
        // Real drag has already taken over, no need to post process the fake drag
        return;
    }
    mFakeDragging = false;
    updateScrollEventValues();
    if (mScrollValues.mOffsetPx == 0) {
        // We're snapped, so dispatch an IDLE event
        if (mScrollValues.mPosition != mDragStartPosition) {
            dispatchSelected(mScrollValues.mPosition);
        }
        dispatchStateChanged(RecyclerView::SCROLL_STATE_IDLE);
        resetState();
    } else {
        // We're not snapped, so dispatch a SETTLING event
        dispatchStateChanged(RecyclerView::SCROLL_STATE_SETTLING);
    }
}

void ScrollEventAdapter::setOnPageChangeCallback(const ViewPager2::OnPageChangeCallback& callback) {
    mCallback = callback;
}

int ScrollEventAdapter::getScrollState() {
    return mScrollState;
}

bool ScrollEventAdapter::isIdle() {
    return mScrollState == RecyclerView::SCROLL_STATE_IDLE;
}

bool ScrollEventAdapter::isDragging() {
    return mScrollState == RecyclerView::SCROLL_STATE_DRAGGING;
}

bool ScrollEventAdapter::isFakeDragging() {
    return mFakeDragging;
}

bool ScrollEventAdapter::isInAnyDraggingState() {
    return mAdapterState == STATE_IN_PROGRESS_MANUAL_DRAG
            || mAdapterState == STATE_IN_PROGRESS_FAKE_DRAG;
}

double ScrollEventAdapter::getRelativeScrollPosition() {
    updateScrollEventValues();
    return mScrollValues.mPosition + (double) mScrollValues.mOffset;
}

void ScrollEventAdapter::dispatchStateChanged(int state) {
    if (mAdapterState == STATE_IN_PROGRESS_IMMEDIATE_SCROLL
            && mScrollState == RecyclerView::SCROLL_STATE_IDLE) {
        return;
    }
    if (mScrollState == state) {
        return;
    }

    mScrollState = state;
    if (mCallback.onPageScrollStateChanged) {
        mCallback.onPageScrollStateChanged(state);
    }
}

void ScrollEventAdapter::dispatchSelected(int target) {
    if (mCallback.onPageSelected) {
        mCallback.onPageSelected(target);
    }
}

void ScrollEventAdapter::dispatchScrolled(int position, float offset, int offsetPx) {
    if (mCallback.onPageScrolled) {
        mCallback.onPageScrolled(position, offset, offsetPx);
    }
}

int ScrollEventAdapter::getPosition() {
    return mLayoutManager->findFirstVisibleItemPosition();
}

ScrollEventAdapter::ScrollEventValues::ScrollEventValues() {
}

void ScrollEventAdapter::ScrollEventValues::reset() {
    mPosition = RecyclerView::NO_POSITION;
    mOffset = 0.f;
    mOffsetPx = 0;
}
}/*endof namespace*/
