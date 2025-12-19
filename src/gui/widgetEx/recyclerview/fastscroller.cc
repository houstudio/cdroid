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
#include <widgetEx/recyclerview/fastscroller.h>
namespace cdroid{

RecyclerView::FastScroller::FastScroller(RecyclerView*recyclerView, StateListDrawable* verticalThumbDrawable,
        Drawable* verticalTrackDrawable, StateListDrawable* horizontalThumbDrawable,
        Drawable* horizontalTrackDrawable, int defaultWidth, int scrollbarMinimumRange,
        int margin) {
    mShowHideAnimator = ValueAnimator::ofFloat({0.f, 1.f});
    mVerticalThumbDrawable = verticalThumbDrawable;
    mVerticalTrackDrawable = verticalTrackDrawable;
    mHorizontalThumbDrawable = horizontalThumbDrawable;
    mHorizontalTrackDrawable = horizontalTrackDrawable;
    mVerticalThumbWidth = std::max(defaultWidth, verticalThumbDrawable->getIntrinsicWidth());
    mVerticalTrackWidth = std::max(defaultWidth, verticalTrackDrawable->getIntrinsicWidth());
    mHorizontalThumbHeight = std::max(defaultWidth, horizontalThumbDrawable->getIntrinsicWidth());
    mHorizontalTrackHeight = std::max(defaultWidth, horizontalTrackDrawable->getIntrinsicWidth());
    mScrollbarMinimumRange = scrollbarMinimumRange;
    mMargin = margin;
    mVerticalThumbDrawable->setAlpha(SCROLLBAR_FULL_OPAQUE);
    mVerticalTrackDrawable->setAlpha(SCROLLBAR_FULL_OPAQUE);

    mHideRunnable =[this](){
        hide(HIDE_DURATION_MS);
    };
    Animator::AnimatorListener asl;
    asl.onAnimationEnd=[this](Animator& animation,bool isReverse){
        if (mCanceled) {
            mCanceled = false;
            return;
        }
        if (GET_VARIANT(mShowHideAnimator->getAnimatedValue(),float) == 0.f) {
            mAnimationState = ANIMATION_STATE_OUT;
            setState(STATE_HIDDEN);
        } else {
            mAnimationState = ANIMATION_STATE_IN;
            requestRedraw();
        }
    };
    asl.onAnimationCancel=[this](Animator&animation){
        mCanceled = true;
    };
    mShowHideAnimator->addListener(asl);

    const ValueAnimator::AnimatorUpdateListener aul([this](ValueAnimator& valueAnimator){
        const int alpha = int(SCROLLBAR_FULL_OPAQUE * GET_VARIANT(valueAnimator.getAnimatedValue(),float));
        mVerticalThumbDrawable->setAlpha(alpha);
        mVerticalTrackDrawable->setAlpha(alpha);
        requestRedraw();
    });
    mShowHideAnimator->addUpdateListener(aul);

    mOnScrollListener.onScrolled = [this](RecyclerView& recyclerView, int dx, int dy){
         updateScrollPosition(recyclerView.computeHorizontalScrollOffset(),
                    recyclerView.computeVerticalScrollOffset());
    };
    mOnItemTouchListener.onInterceptTouchEvent = [this](RecyclerView&rv,MotionEvent&e){
        return onInterceptTouchEvent(rv,e);
    };
    mOnItemTouchListener.onTouchEvent=[this](RecyclerView&rv,MotionEvent&e){
        onTouchEvent(rv,e);
    };
    mOnItemTouchListener.onRequestDisallowInterceptTouchEvent=[this](bool disallowIntercept){
        onRequestDisallowInterceptTouchEvent(disallowIntercept);
    };
    attachToRecyclerView(recyclerView);
}

RecyclerView::FastScroller::~FastScroller(){
    delete mVerticalThumbDrawable;
    delete mVerticalTrackDrawable;
    delete mHorizontalThumbDrawable;
    delete mHorizontalTrackDrawable;
    delete mShowHideAnimator;
}

void RecyclerView::FastScroller::attachToRecyclerView(RecyclerView* recyclerView) {
    if (mRecyclerView == recyclerView) {
        return; // nothing to do
    }
    if (mRecyclerView != nullptr) {
        destroyCallbacks();
    }
    mRecyclerView = recyclerView;
    if (mRecyclerView != nullptr) {
        setupCallbacks();
    }
}

void RecyclerView::FastScroller::setupCallbacks() {
    mRecyclerView->addItemDecoration(this);
    mRecyclerView->addOnItemTouchListener(mOnItemTouchListener);
    mRecyclerView->addOnScrollListener(mOnScrollListener);
}

void RecyclerView::FastScroller::destroyCallbacks() {
    mRecyclerView->removeItemDecoration(this);
    mRecyclerView->removeOnItemTouchListener(mOnItemTouchListener);
    mRecyclerView->removeOnScrollListener(mOnScrollListener);
    cancelHide();
}

void RecyclerView::FastScroller::requestRedraw() {
    mRecyclerView->invalidate();
}

void RecyclerView::FastScroller::setState(int state) {
    if (state == STATE_DRAGGING && mState != STATE_DRAGGING) {
        mVerticalThumbDrawable->setState(StateSet::get(StateSet::VIEW_STATE_PRESSED));//PRESSED_STATE_SET);
        cancelHide();
    }

    if (state == STATE_HIDDEN) {
        requestRedraw();
    } else {
        show();
    }

    if (mState == STATE_DRAGGING && state != STATE_DRAGGING) {
        mVerticalThumbDrawable->setState(StateSet::NOTHING);//EMPTY_STATE_SET);
        resetHideDelay(HIDE_DELAY_AFTER_DRAGGING_MS);
    } else if (state == STATE_VISIBLE) {
        resetHideDelay(HIDE_DELAY_AFTER_VISIBLE_MS);
    }
    mState = state;
}

bool RecyclerView::FastScroller::isLayoutRTL() const{
    return mRecyclerView->getLayoutDirection() == View::LAYOUT_DIRECTION_RTL;
}

bool RecyclerView::FastScroller::isDragging() const{
    return mState == STATE_DRAGGING;
}

bool RecyclerView::FastScroller::isVisible() const{
    return mState == STATE_VISIBLE;
}

void RecyclerView::FastScroller::show() {
    switch (mAnimationState) {
    case ANIMATION_STATE_FADING_OUT:
        mShowHideAnimator->cancel();
        // fall through
    case ANIMATION_STATE_OUT:
        mAnimationState = ANIMATION_STATE_FADING_IN;
        mShowHideAnimator->setFloatValues({GET_VARIANT(mShowHideAnimator->getAnimatedValue(),float), 1.f});
        mShowHideAnimator->setDuration(SHOW_DURATION_MS);
        mShowHideAnimator->setStartDelay(0);
        mShowHideAnimator->start();
        break;
    }
}

void RecyclerView::FastScroller::hide(int duration) {
    switch (mAnimationState) {
    case ANIMATION_STATE_FADING_IN:
        mShowHideAnimator->cancel();
        // fall through
    case ANIMATION_STATE_IN:
        mAnimationState = ANIMATION_STATE_FADING_OUT;
        mShowHideAnimator->setFloatValues({GET_VARIANT(mShowHideAnimator->getAnimatedValue(),float), 0.f});
        mShowHideAnimator->setDuration(duration);
        mShowHideAnimator->start();
        break;
    }
}

void RecyclerView::FastScroller::cancelHide() {
    mRecyclerView->removeCallbacks(mHideRunnable);
}

void RecyclerView::FastScroller::resetHideDelay(int delay) {
    cancelHide();
    mRecyclerView->postDelayed(mHideRunnable, delay);
}

void RecyclerView::FastScroller::onDrawOver(Canvas& canvas, RecyclerView& parent, RecyclerView::State& state) {
    if (mRecyclerViewWidth != mRecyclerView->getWidth()
            || mRecyclerViewHeight != mRecyclerView->getHeight()) {
        mRecyclerViewWidth = mRecyclerView->getWidth();
        mRecyclerViewHeight = mRecyclerView->getHeight();
        // This is due to the different events ordering when keyboard is opened or
        // retracted vs rotate. Hence to avoid corner cases we just disable the
        // scroller when size changed, and wait until the scroll position is recomputed
        // before showing it back.
        setState(STATE_HIDDEN);
        return;
    }

    if (mAnimationState != ANIMATION_STATE_OUT) {
        if (mNeedVerticalScrollbar) {
            drawVerticalScrollbar(canvas);
        }
        if (mNeedHorizontalScrollbar) {
            drawHorizontalScrollbar(canvas);
        }
    }
}

void RecyclerView::FastScroller::drawVerticalScrollbar(Canvas& canvas) {
    int viewWidth = mRecyclerViewWidth;

    int left = viewWidth - mVerticalThumbWidth;
    int top = mVerticalThumbCenterY - mVerticalThumbHeight / 2;
    mVerticalThumbDrawable->setBounds(0, 0, mVerticalThumbWidth, mVerticalThumbHeight);
    mVerticalTrackDrawable->setBounds(0, 0, mVerticalTrackWidth, mRecyclerViewHeight);

    if (isLayoutRTL()) {
        mVerticalTrackDrawable->draw(canvas);
        canvas.translate(mVerticalThumbWidth, top);
        canvas.scale(-1, 1);
        mVerticalThumbDrawable->draw(canvas);
        canvas.scale(1, 1);
        canvas.translate(-mVerticalThumbWidth, -top);
    } else {
        canvas.translate(left, 0);
        mVerticalTrackDrawable->draw(canvas);
        canvas.translate(0, top);
        mVerticalThumbDrawable->draw(canvas);
        canvas.translate(-left, -top);
    }
}

void RecyclerView::FastScroller::drawHorizontalScrollbar(Canvas& canvas) {
    int viewHeight = mRecyclerViewHeight;

    int top = viewHeight - mHorizontalThumbHeight;
    int left = mHorizontalThumbCenterX - mHorizontalThumbWidth / 2;
    mHorizontalThumbDrawable->setBounds(0, 0, mHorizontalThumbWidth, mHorizontalThumbHeight);
    mHorizontalTrackDrawable->setBounds(0, 0, mRecyclerViewWidth, mHorizontalTrackHeight);

    canvas.translate(0, top);
    mHorizontalTrackDrawable->draw(canvas);
    canvas.translate(left, 0);
    mHorizontalThumbDrawable->draw(canvas);
    canvas.translate(-left, -top);
}

void RecyclerView::FastScroller::updateScrollPosition(int offsetX, int offsetY) {
    int verticalContentLength = mRecyclerView->computeVerticalScrollRange();
    int verticalVisibleLength = mRecyclerViewHeight;
    mNeedVerticalScrollbar = verticalContentLength - verticalVisibleLength > 0
        && mRecyclerViewHeight >= mScrollbarMinimumRange;

    int horizontalContentLength = mRecyclerView->computeHorizontalScrollRange();
    int horizontalVisibleLength = mRecyclerViewWidth;
    mNeedHorizontalScrollbar = horizontalContentLength - horizontalVisibleLength > 0
        && mRecyclerViewWidth >= mScrollbarMinimumRange;

    if (!mNeedVerticalScrollbar && !mNeedHorizontalScrollbar) {
        if (mState != STATE_HIDDEN) {
            setState(STATE_HIDDEN);
        }
        return;
    }

    if (mNeedVerticalScrollbar) {
        float middleScreenPos = offsetY + verticalVisibleLength / 2.0f;
        mVerticalThumbCenterY =
            (int) ((verticalVisibleLength * middleScreenPos) / verticalContentLength);
        mVerticalThumbHeight = std::min(verticalVisibleLength,
            (verticalVisibleLength * verticalVisibleLength) / verticalContentLength);
    }

    if (mNeedHorizontalScrollbar) {
        float middleScreenPos = offsetX + horizontalVisibleLength / 2.0f;
        mHorizontalThumbCenterX =
            (int) ((horizontalVisibleLength * middleScreenPos) / horizontalContentLength);
        mHorizontalThumbWidth = std::min(horizontalVisibleLength,
            (horizontalVisibleLength * horizontalVisibleLength) / horizontalContentLength);
    }

    if (mState == STATE_HIDDEN || mState == STATE_VISIBLE) {
        setState(STATE_VISIBLE);
    }
}

bool RecyclerView::FastScroller::onInterceptTouchEvent(RecyclerView& recyclerView,MotionEvent& ev) {
    bool handled;
    if (mState == STATE_VISIBLE) {
        bool insideVerticalThumb = isPointInsideVerticalThumb(ev.getX(), ev.getY());
        bool insideHorizontalThumb = isPointInsideHorizontalThumb(ev.getX(), ev.getY());
        if (ev.getAction() == MotionEvent::ACTION_DOWN
                && (insideVerticalThumb || insideHorizontalThumb)) {
            if (insideHorizontalThumb) {
                mDragState = DRAG_X;
                mHorizontalDragX = ev.getX();
            } else if (insideVerticalThumb) {
                mDragState = DRAG_Y;
                mVerticalDragY = ev.getY();
            }

            setState(STATE_DRAGGING);
            handled = true;
        } else {
            handled = false;
        }
    } else if (mState == STATE_DRAGGING) {
        handled = true;
    } else {
        handled = false;
    }
    return handled;
}

void RecyclerView::FastScroller::onTouchEvent(RecyclerView& recyclerView,MotionEvent& me) {
    if (mState == STATE_HIDDEN) {
        return;
    }

    if (me.getAction() == MotionEvent::ACTION_DOWN) {
        bool insideVerticalThumb = isPointInsideVerticalThumb(me.getX(), me.getY());
        bool insideHorizontalThumb = isPointInsideHorizontalThumb(me.getX(), me.getY());
        if (insideVerticalThumb || insideHorizontalThumb) {
            if (insideHorizontalThumb) {
                mDragState = DRAG_X;
                mHorizontalDragX = me.getX();
            } else if (insideVerticalThumb) {
                mDragState = DRAG_Y;
                mVerticalDragY = me.getY();
            }
            setState(STATE_DRAGGING);
        }
    } else if (me.getAction() == MotionEvent::ACTION_UP && mState == STATE_DRAGGING) {
        mVerticalDragY = 0;
        mHorizontalDragX = 0;
        setState(STATE_VISIBLE);
        mDragState = DRAG_NONE;
    } else if (me.getAction() == MotionEvent::ACTION_MOVE && mState == STATE_DRAGGING) {
        show();
        if (mDragState == DRAG_X) {
            horizontalScrollTo(me.getX());
        }
        if (mDragState == DRAG_Y) {
            verticalScrollTo(me.getY());
        }
    }
}

void RecyclerView::FastScroller::onRequestDisallowInterceptTouchEvent(bool disallowIntercept) { }

void RecyclerView::FastScroller::verticalScrollTo(float y) {
    int scrollbarRange[2];
    getVerticalRange(scrollbarRange);
    y = std::max(float(scrollbarRange[0]), std::min(float(scrollbarRange[1]), y));
    if (std::abs(mVerticalThumbCenterY - y) < 2) {
        return;
    }
    const int scrollingBy = scrollTo(mVerticalDragY, y, scrollbarRange,
            mRecyclerView->computeVerticalScrollRange(),
            mRecyclerView->computeVerticalScrollOffset(), mRecyclerViewHeight);
    if (scrollingBy != 0) {
        mRecyclerView->scrollBy(0, scrollingBy);
    }
    mVerticalDragY = y;
}

void RecyclerView::FastScroller::horizontalScrollTo(float x) {
    int scrollbarRange[2];
    getHorizontalRange(scrollbarRange);
    x = std::max(float(scrollbarRange[0]), std::min(float(scrollbarRange[1]), x));
    if (std::abs(mHorizontalThumbCenterX - x) < 2) {
        return;
    }

    const int scrollingBy = scrollTo(mHorizontalDragX, x, scrollbarRange,
            mRecyclerView->computeHorizontalScrollRange(),
            mRecyclerView->computeHorizontalScrollOffset(), mRecyclerViewWidth);
    if (scrollingBy != 0) {
        mRecyclerView->scrollBy(scrollingBy, 0);
    }

    mHorizontalDragX = x;
}

int RecyclerView::FastScroller::scrollTo(float oldDragPos, float newDragPos, int scrollbarRange[2],  int scrollRange,
        int scrollOffset, int viewLength) {
    int scrollbarLength = scrollbarRange[1] - scrollbarRange[0];
    if (scrollbarLength == 0) {
        return 0;
    }
    float percentage = ((newDragPos - oldDragPos) / (float) scrollbarLength);
    int totalPossibleOffset = scrollRange - viewLength;
    int scrollingBy = (int) (percentage * totalPossibleOffset);
    int absoluteOffset = scrollOffset + scrollingBy;
    if (absoluteOffset < totalPossibleOffset && absoluteOffset >= 0) {
        return scrollingBy;
    } else {
        return 0;
    }
}

bool RecyclerView::FastScroller::isPointInsideVerticalThumb(float x, float y) {
    return (isLayoutRTL() ? x <= mVerticalThumbWidth / 2
        : x >= mRecyclerViewWidth - mVerticalThumbWidth)
        && y >= mVerticalThumbCenterY - mVerticalThumbHeight / 2
        && y <= mVerticalThumbCenterY + mVerticalThumbHeight / 2;
}

bool RecyclerView::FastScroller::isPointInsideHorizontalThumb(float x, float y) {
    return (y >= mRecyclerViewHeight - mHorizontalThumbHeight)
        && x >= mHorizontalThumbCenterX - mHorizontalThumbWidth / 2
        && x <= mHorizontalThumbCenterX + mHorizontalThumbWidth / 2;
}

Drawable* RecyclerView::FastScroller::getHorizontalTrackDrawable() {
    return mHorizontalTrackDrawable;
}

Drawable* RecyclerView::FastScroller::getHorizontalThumbDrawable() {
    return mHorizontalThumbDrawable;
}

Drawable* RecyclerView::FastScroller::getVerticalTrackDrawable() {
    return mVerticalTrackDrawable;
}

Drawable* RecyclerView::FastScroller::getVerticalThumbDrawable() {
    return mVerticalThumbDrawable;
}

void RecyclerView::FastScroller::getVerticalRange(int out[2]) {
    out[0] = mVerticalRange[0] = mMargin;
    out[1] = mVerticalRange[1] = mRecyclerViewHeight - mMargin;
}

void RecyclerView::FastScroller::getHorizontalRange(int out[2]) {
    out[0] = mHorizontalRange[0] = mMargin;
    out[1] = mHorizontalRange[1] = mRecyclerViewWidth - mMargin;
}


}/*endof namespace*/
