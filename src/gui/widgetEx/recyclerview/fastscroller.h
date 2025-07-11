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
#ifndef __RECYCLERVIEW_FASTSCROLLER_H__
#define __RECYCLERVIEW_FASTSCROLLER_H__
#include <widgetEx/recyclerview/recyclerview.h>
namespace cdroid{
class RecyclerView::FastScroller:public RecyclerView::ItemDecoration{// implements RecyclerView.OnItemTouchListener {
    // Scroll thumb not showing
private:
    static constexpr int STATE_HIDDEN = 0;
    // Scroll thumb visible and moving along with the scrollbar
    static constexpr int STATE_VISIBLE = 1;
    // Scroll thumb being dragged by user
    static constexpr int STATE_DRAGGING = 2;

    static constexpr int DRAG_NONE = 0;
    static constexpr int DRAG_X = 1;
    static constexpr int DRAG_Y = 2;

    static constexpr int ANIMATION_STATE_OUT = 0;
    static constexpr int ANIMATION_STATE_FADING_IN = 1;
    static constexpr int ANIMATION_STATE_IN = 2;
    static constexpr int ANIMATION_STATE_FADING_OUT = 3;

    static constexpr int SHOW_DURATION_MS = 500;
    static constexpr int HIDE_DELAY_AFTER_VISIBLE_MS = 1500;
    static constexpr int HIDE_DELAY_AFTER_DRAGGING_MS = 1200;
    static constexpr int HIDE_DURATION_MS = 500;
    static constexpr int SCROLLBAR_FULL_OPAQUE = 255;
private:
    bool mCanceled;/*used by AniamtionUpdateListener*/
    int mScrollbarMinimumRange;
    int mMargin;

    // Final values for the vertical scroll bar
    StateListDrawable* mVerticalThumbDrawable;
    Drawable* mVerticalTrackDrawable;
    int mVerticalThumbWidth;
    int mVerticalTrackWidth;

    // Final values for the horizontal scroll bar
    StateListDrawable* mHorizontalThumbDrawable;
    Drawable* mHorizontalTrackDrawable;
    int mHorizontalThumbHeight;
    int mHorizontalTrackHeight;
    int mRecyclerViewWidth = 0;
    int mRecyclerViewHeight = 0;

    RecyclerView* mRecyclerView;
    bool mNeedVerticalScrollbar = false;
    bool mNeedHorizontalScrollbar = false;
    int mState = STATE_HIDDEN;
    int mDragState = DRAG_NONE;
    int mAnimationState = ANIMATION_STATE_OUT;
    int mVerticalRange[2];
    int mHorizontalRange[2];
    ValueAnimator* mShowHideAnimator;
    Runnable mHideRunnable;
    RecyclerView::OnItemTouchListener mOnItemTouchListener;
    RecyclerView::OnScrollListener mOnScrollListener;
protected:
    // Dynamic values for the vertical scroll bar
    int mVerticalThumbHeight;
    int mVerticalThumbCenterY;
    float mVerticalDragY;

    // Dynamic values for the horizontal scroll bar
    int mHorizontalThumbWidth;
    int mHorizontalThumbCenterX;
    float mHorizontalDragX;
private:
    friend RecyclerView;
    void setupCallbacks();
    void destroyCallbacks();
    void requestRedraw();
    void setState(int state);
    bool isLayoutRTL()const;
    void cancelHide();
    void resetHideDelay(int delay);
    void drawVerticalScrollbar(Canvas& canvas);
    void drawHorizontalScrollbar(Canvas& canvas);
    void verticalScrollTo(float y);
    void horizontalScrollTo(float x);
    int scrollTo(float oldDragPos, float newDragPos, int scrollbarRange[2], int scrollRange,
            int scrollOffset, int viewLength);
    void getVerticalRange(int out[2]);
    void getHorizontalRange(int out[2]);
protected:
    FastScroller(RecyclerView*recyclerView, StateListDrawable* verticalThumbDrawable,
            Drawable* verticalTrackDrawable, StateListDrawable* horizontalThumbDrawable,
            Drawable* horizontalTrackDrawable, int defaultWidth, int scrollbarMinimumRange,
            int margin);
    ~FastScroller()override;
    bool isVisible()const;
    void hide(int duration);
    void updateScrollPosition(int offsetX, int offsetY);
    bool isPointInsideVerticalThumb(float x, float y);
    bool isPointInsideHorizontalThumb(float x, float y);
    Drawable* getHorizontalTrackDrawable();
    Drawable* getHorizontalThumbDrawable();
    Drawable* getVerticalTrackDrawable();
    Drawable* getVerticalThumbDrawable();

    /*RecyclerView::OnItemTouchListener */
    bool onInterceptTouchEvent(RecyclerView& recyclerView,MotionEvent& ev);
    void onTouchEvent(RecyclerView& recyclerView, MotionEvent& me);
    void onRequestDisallowInterceptTouchEvent(bool disallowIntercept);
public:
    void attachToRecyclerView(RecyclerView* recyclerView);
    bool isDragging()const;
    void show();
    void onDrawOver(Canvas& canvas, RecyclerView& parent, RecyclerView::State& state)override;
};
}/*endof namspace*/
#endif/*__RECYCLERVIEW_FASTSCROLLER_H__*/

