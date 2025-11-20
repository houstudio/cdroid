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
#ifndef __HORIZONTAL_SCROLLVIEW_H__
#define __HORIZONTAL_SCROLLVIEW_H__
#include <widget/framelayout.h>
#include <widget/overscroller.h>
#include <widget/edgeeffect.h>

namespace cdroid{

class HorizontalScrollView:public FrameLayout{
private:
    static constexpr int INVALID_POINTER = -1;
    int64_t mLastScroll;
    OverScroller* mScroller;
    VelocityTracker* mVelocityTracker;
    EdgeEffect* mEdgeGlowLeft;
    EdgeEffect* mEdgeGlowRight;
    int mLastMotionX;
    bool mIsLayoutDirty = true;
    View* mChildToScrollTo = nullptr;
    bool mIsBeingDragged = false;

    bool mFillViewport;

    bool mSmoothScrollingEnabled = true;

    int mScrollDuration;
    int mTouchSlop;
    int mMinimumVelocity;
    int mMaximumVelocity;

    int mOverscrollDistance;
    int mOverflingDistance;

    float mHorizontalScrollFactor;
    int mActivePointerId;
    Rect mTempRect;
private:
    static constexpr int ANIMATED_SCROLL_GAP = 250;
    static constexpr float MAX_SCROLL_FACTOR =0.5f;
    static constexpr float FLING_DESTRETCH_FACTOR = 4.f;
    void initScrollView(const AttributeSet*);
    bool canScroll();
    bool inChild(int x, int y);
    void initOrResetVelocityTracker();
    void initVelocityTrackerIfNotExists();
    void recycleVelocityTracker();
    void onSecondaryPointerUp(MotionEvent& ev);
    int getScrollRange();
    View* findFocusableViewInMyBounds(bool leftFocus,int left, View* preferredFocusable);
    View* findFocusableViewInBounds(bool leftFocus, int left, int right);
    bool scrollAndFocus(int direction, int left, int right);
    bool isOffScreen(View* descendant);
    bool isWithinDeltaOfScreen(View* descendant, int delta);
    void doScrollX(int delta);
    int  consumeFlingInStretch(int unconsumed);
    void scrollToChild(View* child);
    bool scrollToChildRect(Rect rect, bool immediate);
    bool isViewDescendantOf(View* child, View* parent);
    bool shouldDisplayEdgeEffects()const;
    bool shouldAbsorb(EdgeEffect* edgeEffect, int velocity);
protected:
    float getLeftFadingEdgeStrength()override;
    float getRightFadingEdgeStrength()override;
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec)override;
    void onOverScrolled(int scrollX, int scrollY,bool clampedX, bool clampedY)override;
    int computeHorizontalScrollRange()override;
    int computeHorizontalScrollOffset()override;
    void measureChild(View* child, int parentWidthMeasureSpec,int parentHeightMeasureSpec)override;
    void measureChildWithMargins(View* child, int parentWidthMeasureSpec, int widthUsed,
            int parentHeightMeasureSpec, int heightUsed)override;
    int computeScrollDeltaToGetChildRectOnScreen(Rect& rect);
    bool onRequestFocusInDescendants(int direction,Rect* previouslyFocusedRect)override;
    void onLayout(bool changed, int l, int t, int w, int h)override;
    void onSizeChanged(int w, int h, int oldw, int oldh)override;
    void draw(Canvas& canvas)override;
public:
    HorizontalScrollView(int w,int h);
    HorizontalScrollView(Context*ctx,const AttributeSet&atts);
    ~HorizontalScrollView()override;
    void setEdgeEffectColor(int color);
    void setLeftEdgeEffectColor(int color);
    void setRightEdgeEffectColor(int color);
    int getLeftEdgeEffectColor()const;
    int getRightEdgeEffectColor()const;
    int getMaxScrollAmount();
    void addView(View* child)override;
    void addView(View* child, int index)override;
    void addView(View* child, ViewGroup::LayoutParams* params)override;
    void addView(View* child, int index,ViewGroup::LayoutParams* params)override;
    bool isFillViewport()const;
    void setFillViewport(bool fillViewport);
    bool isSmoothScrollingEnabled()const;
    void setSmoothScrollingEnabled(bool smoothScrollingEnabled);
    bool dispatchKeyEvent(KeyEvent& event)override;
    bool executeKeyEvent(KeyEvent& event);
    void requestDisallowInterceptTouchEvent(bool disallowIntercept)override;
    bool onInterceptTouchEvent(MotionEvent& ev)override;
    bool onTouchEvent(MotionEvent& ev)override;
    bool onGenericMotionEvent(MotionEvent& event)override;
    bool shouldDelayChildPressedState()override;
    bool pageScroll(int direction);
    bool fullScroll(int direction);
    bool arrowScroll(int direction);
    void smoothScrollBy(int dx, int dy);
    void smoothScrollTo(int x, int y);
    void computeScroll()override;
    void requestChildFocus(View* child, View* focused)override;
    bool requestChildRectangleOnScreen(View* child, Rect rectangle,bool immediate);
    void requestLayout()override;
    void fling(int velocityX);
    void scrollTo(int x, int y)override;
};

}
#endif
