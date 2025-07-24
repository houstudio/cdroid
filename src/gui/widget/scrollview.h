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
#ifndef __SCROLL_VIEW__
#define __SCROLL_VIEW__
#include <widget/framelayout.h>
#include <widget/overscroller.h>
#include <widget/edgeeffect.h>
namespace cdroid{
class HapticScrollFeedbackProvider;
class ScrollView:public FrameLayout{
private:
    static constexpr int INVALID_POINTER=-1;
    int64_t mLastScroll;
    int mLastMotionY;
    bool mIsLayoutDirty=true;
    View* mChildToScrollTo = nullptr;
    bool mIsBeingDragged = false;
    bool mFillViewport;
    bool mSmoothScrollingEnabled = true;
    int mTouchSlop;
    int mScrollDuration;
    int mMinimumVelocity;
    int mMaximumVelocity;

    int mOverscrollDistance;
    int mOverflingDistance;

    OverScroller* mScroller;
    VelocityTracker*mVelocityTracker;
    EdgeEffect* mEdgeGlowTop;
    EdgeEffect* mEdgeGlowBottom;
    float mVerticalScrollFactor;
    int mActivePointerId = INVALID_POINTER;
    int mScrollOffset[2] ;
    int mScrollConsumed[2];
    int mNestedYOffset;
    HapticScrollFeedbackProvider* mHapticScrollFeedbackProvider;

    void initScrollView();
    bool canScroll();
    bool inChild(int x, int y);
    void onSecondaryPointerUp(MotionEvent& ev);
    int getScrollRange();
    View* findFocusableViewInBounds(bool topFocus, int top, int bottom);
    bool scrollAndFocus(int direction, int top, int bottom);
    bool isOffScreen(const View* descendant);
    bool isWithinDeltaOfScreen(const View* descendant, int delta, int height);
    void doScrollY(int delta);
    void smoothScrollBy(int dx, int dy);
    int consumeFlingInStretch(int unconsumed);
    bool scrollToChildRect(Rect& rect, bool immediate);
    bool shouldDisplayEdgeEffects()const;
    static bool isViewDescendantOf(View* child, View* parent);
    static int clamp(int n, int my, int child);
    void flingWithNestedDispatch(int velocityY);
    bool shouldAbsorb(EdgeEffect* edgeEffect, int velocity);
    void endDrag();
protected:
    static constexpr float MAX_SCROLL_FACTOR = 0.5f;
    static constexpr int ANIMATED_SCROLL_GAP = 250;
    static constexpr float FLING_DESTRETCH_FACTOR = 4.f;
    float getTopFadingEdgeStrength()override;
    float getBottomFadingEdgeStrength()override;
    void setEdgeEffectColor(int color);
    void setBottomEdgeEffectColor(int color);
    void setTopEdgeEffectColor(int color);
    int  getTopEdgeEffectColor()const;
    int getBottomEdgeEffectColor()const;
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec)override;
    void onOverScrolled(int scrollX, int scrollY, bool clampedX, bool clampedY)override;
    int computeVerticalScrollRange()override;
    int computeVerticalScrollOffset()override;
    void initOrResetVelocityTracker();
    void initVelocityTrackerIfNotExists();
    void initHapticScrollFeedbackProviderIfNotExists();
    void recycleVelocityTracker();
    void measureChild(View* child, int parentWidthMeasureSpec,int parentHeightMeasureSpec)override;
    void measureChildWithMargins(View* child, int parentWidthMeasureSpec, int widthUsed,
            int parentHeightMeasureSpec, int heightUsed)override;
    int computeScrollDeltaToGetChildRectOnScreen(Rect& rect);
    bool onRequestFocusInDescendants(int direction,Rect* previouslyFocusedRect)override;
    bool requestChildRectangleOnScreen(View* child,Rect& rectangle, bool immediate)override;
    void onLayout(bool changed, int l, int t, int w, int h)override;
    void draw(Canvas& canvas)override;
public:
    ScrollView(int w,int h);
    ScrollView(Context*ctx,const AttributeSet&atts);
    ~ScrollView()override;
    int getMaxScrollAmount();
    std::string getAccessibilityClassName() const override;
    bool shouldDelayChildPressedState()override;
    void addView(View* child)override;
    void addView(View* child, int index)override;
    void addView(View* child, ViewGroup::LayoutParams* params)override;
    void addView(View* child, int index, ViewGroup::LayoutParams* params)override;
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
    bool performAccessibilityActionInternal(int action, Bundle* arguments)override;
    void onInitializeAccessibilityNodeInfoInternal(AccessibilityNodeInfo& info)override;
    void onInitializeAccessibilityEventInternal(AccessibilityEvent& event)override;
    bool pageScroll(int direction);
    bool fullScroll(int direction);
    bool arrowScroll(int direction);
    void smoothScrollTo(int x, int y);
    void computeScroll()override;
    void requestChildFocus(View* child, View* focused)override;
    void requestLayout()override;
    void scrollTo(int x, int y)override;
    void scrollToDescendant(View* child);
    void setOverScrollMode(int mode)override;
    bool onStartNestedScroll(View* child, View* target, int nestedScrollAxes)override;
    void onNestedScrollAccepted(View* child, View* target, int axes)override;
    void onStopNestedScroll(View* target)override;
    void onNestedScroll(View* target, int dxConsumed, int dyConsumed,
            int dxUnconsumed, int dyUnconsumed)override;
    bool onNestedFling(View* target, float velocityX, float velocityY, bool consumed)override;
    void fling(int velocityY);
};

}
#endif//__SCROLL_VIEW__
