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
#ifndef __NESTED_SCROLLVIEW_H__
#define __NESTED_SCROLLVIEW_H__
#include <widget/framelayout.h>
#include <widget/edgeeffect.h>
#include <widget/overscroller.h>

namespace cdroid{
class NestedScrollingParentHelper;
class NestedScrollingChildHelper;

class NestedScrollView:public FrameLayout{
public:	
    DECLARE_UIEVENT(void,OnScrollChangeListener,NestedScrollView&v,int ,int,int,int);
private:
    static constexpr int ANIMATED_SCROLL_GAP = 250;
    static constexpr float MAX_SCROLL_FACTOR = 0.5f;
    static constexpr int INVALID_POINTER = -1;
    static constexpr float SCROLL_FRICTION = 0.015f;
    static constexpr float INFLEXION = 0.35f;
    static constexpr float DECELERATION_RATE = 2.3582018154259448f;//(float) (Math.log(0.78) / Math.log(0.9));
    static constexpr float FLING_DESTRENTCH_FACTOR = 4.f;
private:
    int64_t mLastScroll;
 
    Rect mTempRect;
    OverScroller* mScroller;
    EdgeEffect* mEdgeGlowTop;
    EdgeEffect* mEdgeGlowBottom;
	
    int mLastMotionY;
    float mPhysicalCoeff;
    bool mIsLayoutDirty;
    bool mIsLaidOut;
    View* mChildToScrollTo;
    bool mIsBeingDragged;
    VelocityTracker* mVelocityTracker;
    bool mFillViewport;
    bool mSmoothScrollingEnabled;
	
    int mTouchSlop;
    int mMinimumVelocity;
    int mMaximumVelocity;
    int mActivePointerId = INVALID_POINTER;
    int mScrollOffset[2];
    int mScrollConsumed[2];
    int mNestedYOffset;
    int mLastScrollerY;
    NestedScrollingParentHelper* mParentHelper;
    NestedScrollingChildHelper* mChildHelper;
    float mVerticalScrollFactor;
    OnScrollChangeListener mOnScrollChangeListener;
private:
    void initScrollView(const AttributeSet*attrs);
    bool canScroll();
    bool inChild(int x, int y)const;
    void initOrResetVelocityTracker();
    void initVelocityTrackerIfNotExists();
    void recycleVelocityTracker();
    bool shouldAbsorb(EdgeEffect* edgeEffect, int velocity) const;
    float getSplineFlingDistance(int velocity) const;
    bool edgeEffectFling(int velocityY);
    bool stopGlowAnimations(MotionEvent& e);
    void onSecondaryPointerUp(MotionEvent& ev);
    float getVerticalScrollFactorCompat();
    View* findFocusableViewInBounds(bool topFocus, int top, int bottom);
    bool scrollAndFocus(int direction, int top, int bottom);
    bool isOffScreen(View* descendant);
    bool isWithinDeltaOfScreen(View* descendant, int delta, int height);
    void doScrollY(int delta);
    void scrollToChild(View* child);
    bool scrollToChildRect(const Rect& rect, bool immediate);
    bool onRequestFocusInDescendants(int direction, Rect* previouslyFocusedRect)override;
    static bool isViewDescendantOf(View* child, View* parent);
    void flingWithNestedDispatch(int velocityY);
    void endDrag();
    void ensureGlows();
    static int clamp(int n, int my, int child);
protected:
    float getTopFadingEdgeStrength()override;
    float getBottomFadingEdgeStrength()override;
    void onScrollChanged(int l, int t, int oldl, int oldt)override;
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec)override;
    void onOverScrolled(int scrollX, int scrollY,bool clampedX, bool clampedY)override;
    bool overScrollBy(int deltaX, int deltaY, int scrollX, int scrollY,
        int scrollRangeX, int scrollRangeY,int maxOverScrollX, int maxOverScrollY,bool isTouchEvent)override;
    int getScrollRange();
    void measureChild(View* child, int parentWidthMeasureSpec,int parentHeightMeasureSpec)override;
    void measureChildWithMargins(View* child, int parentWidthMeasureSpec, int widthUsed,
            int parentHeightMeasureSpec, int heightUsed)override;
    int computeScrollDeltaToGetChildRectOnScreen(Rect rect);
    void onLayout(bool changed, int l, int t, int w, int h)override;
    void onSizeChanged(int w, int h, int oldw, int oldh)override;
public:
    NestedScrollView(int w,int h);
    NestedScrollView(Context* context,const AttributeSet&attrs);
    ~NestedScrollView()override;
    bool startNestedScroll(int axes, int type);
    void stopNestedScroll(int type);
    bool hasNestedScrollingParent(int type)const;
    bool dispatchNestedScroll(int dxConsumed, int dyConsumed, int dxUnconsumed,
        int dyUnconsumed, int offsetInWindow[], int type);
    bool dispatchNestedPreScroll(int dx, int dy, int consumed[], int offsetInWindow[],int type);
    void setNestedScrollingEnabled(bool enabled);
    bool isNestedScrollingEnabled()const;
    bool startNestedScroll(int axes);
    void stopNestedScroll();
    bool hasNestedScrollingParent()const;
    bool dispatchNestedScroll(int dxConsumed, int dyConsumed, int dxUnconsumed,
        int dyUnconsumed, int offsetInWindow[])override;
    bool dispatchNestedPreScroll(int dx, int dy, int consumed[], int offsetInWindow[])override;
    bool dispatchNestedFling(float velocityX, float velocityY, bool consumed)override;
    bool dispatchNestedPreFling(float velocityX, float velocityY)override;
    bool onStartNestedScroll(View* child,View* target, int axes,int type)override;
    void onNestedScrollAccepted(View* child,View* target, int axes,int type)override;
    void onStopNestedScroll(View* target, int type)override;
    void onNestedScroll(View* target, int dxConsumed, int dyConsumed, int dxUnconsumed, int dyUnconsumed, int type)override;
    void onNestedPreScroll(View* target, int dx, int dy,int consumed[], int type)override;
    bool onStartNestedScroll(View* child, View* target, int nestedScrollAxes)override;
    void onNestedScrollAccepted(View* child, View* target, int nestedScrollAxes)override;
    void onStopNestedScroll(View* target)override;
    void onNestedScroll(View* target, int dxConsumed, int dyConsumed, int dxUnconsumed,int dyUnconsumed)override;
    void onNestedPreScroll(View* target, int dx, int dy, int consumed[])override;
    bool onNestedFling(View* target, float velocityX, float velocityY, bool consumed)override;
    bool onNestedPreFling(View* target, float velocityX, float velocityY)override;
    int getNestedScrollAxes()override;
    bool shouldDelayChildPressedState()override;
    int getMaxScrollAmount();
    void addView(View* child)override;
    void addView(View* child, int index)override;
    void addView(View* child, ViewGroup::LayoutParams* params)override;
    void addView(View* child, int index, ViewGroup::LayoutParams* params)override;
    void setOnScrollChangeListener(const OnScrollChangeListener& l);
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
    bool pageScroll(int direction);
    bool fullScroll(int direction);
    bool arrowScroll(int direction);
    void smoothScrollBy(int dx, int dy);
    void smoothScrollTo(int x, int y);
    int computeVerticalScrollRange()override;

    int computeVerticalScrollOffset()override;
    int computeVerticalScrollExtent()override;
    int computeHorizontalScrollRange()override;
    int computeHorizontalScrollOffset()override;
    int computeHorizontalScrollExtent()override;

    void computeScroll()override;
    void requestChildFocus(View* child, View* focused)override;
    bool requestChildRectangleOnScreen(View* child, Rect rectangle, bool immediate);
    void requestLayout()override;
    void onAttachedToWindow()override;
    void fling(int velocityY);
    void scrollTo(int x, int y)override;
    void draw(Canvas& canvas)override;
};
}/*endof namespace*/
#endif//__NESTED_SCROLLVIEW_H__
