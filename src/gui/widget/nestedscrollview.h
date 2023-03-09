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
private:
    long mLastScroll;
 
    Rect mTempRect;
    OverScroller* mScroller;
    EdgeEffect* mEdgeGlowTop;
    EdgeEffect* mEdgeGlowBottom;
	
    int mLastMotionY;
	
    bool mIsLayoutDirty = true;
    bool mIsLaidOut = false;
    View* mChildToScrollTo = nullptr;
    bool mIsBeingDragged = false;
    VelocityTracker* mVelocityTracker;
    bool mFillViewport;
    bool mSmoothScrollingEnabled = true;
	
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
    void initScrollView();
    bool canScroll();
    bool inChild(int x, int y);
    void initOrResetVelocityTracker();
    void initVelocityTrackerIfNotExists();
    void recycleVelocityTracker();
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
    float getTopFadingEdgeStrength();
    float getBottomFadingEdgeStrength();
    void onScrollChanged(int l, int t, int oldl, int oldt);
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec);
    void onOverScrolled(int scrollX, int scrollY,bool clampedX, bool clampedY);
    bool overScrollByCompat(int deltaX, int deltaY, int scrollX, int scrollY,
        int scrollRangeX, int scrollRangeY,int maxOverScrollX, int maxOverScrollY,bool isTouchEvent);
    int getScrollRange();
    void measureChild(View* child, int parentWidthMeasureSpec,int parentHeightMeasureSpec);
    int computeScrollDeltaToGetChildRectOnScreen(Rect rect);
    void onLayout(bool changed, int l, int t, int w, int h);
    void onSizeChanged(int w, int h, int oldw, int oldh);
public:
    NestedScrollView(int w,int h);
    NestedScrollView(Context* context,const AttributeSet&attrs);
    bool startNestedScroll(int axes, int type);
    void stopNestedScroll(int type);
    bool hasNestedScrollingParent(int type);
    bool dispatchNestedScroll(int dxConsumed, int dyConsumed, int dxUnconsumed,
        int dyUnconsumed, int offsetInWindow[], int type);
    bool dispatchNestedPreScroll(int dx, int dy, int consumed[], int offsetInWindow[],int type);
    void setNestedScrollingEnabled(bool enabled);
    bool isNestedScrollingEnabled();
    bool startNestedScroll(int axes);
    void stopNestedScroll();
    bool hasNestedScrollingParent();
    bool dispatchNestedScroll(int dxConsumed, int dyConsumed, int dxUnconsumed,
        int dyUnconsumed, int offsetInWindow[]);
    bool dispatchNestedPreScroll(int dx, int dy, int consumed[], int offsetInWindow[]);
    bool dispatchNestedFling(float velocityX, float velocityY, bool consumed);
    bool dispatchNestedPreFling(float velocityX, float velocityY);
    bool onStartNestedScroll(View* child,View* target, int axes,int type);
    void onNestedScrollAccepted(View* child,View* target, int axes,int type);
    void onStopNestedScroll(View* target, int type);
    void onNestedScroll(View* target, int dxConsumed, int dyConsumed, int dxUnconsumed, int dyUnconsumed, int type);
    void onNestedPreScroll(View* target, int dx, int dy,int consumed[], int type);
    bool onStartNestedScroll(View* child, View* target, int nestedScrollAxes);
    void onNestedScrollAccepted(View* child, View* target, int nestedScrollAxes);
    void onStopNestedScroll(View* target);
    void onNestedScroll(View* target, int dxConsumed, int dyConsumed, int dxUnconsumed,int dyUnconsumed);
    void onNestedPreScroll(View* target, int dx, int dy, int consumed[]);
    bool onNestedFling(View* target, float velocityX, float velocityY, bool consumed);
    bool onNestedPreFling(View* target, float velocityX, float velocityY);
    int getNestedScrollAxes();
    bool shouldDelayChildPressedState();
    int getMaxScrollAmount();
    View& addView(View* child);
    View& addView(View* child, int index);
    View& addView(View* child, ViewGroup::LayoutParams* params);
    View& addView(View* child, int index, ViewGroup::LayoutParams* params);
    void setOnScrollChangeListener(OnScrollChangeListener l);
    bool isFillViewport();
    void setFillViewport(bool fillViewport);
    bool isSmoothScrollingEnabled();
    void setSmoothScrollingEnabled(bool smoothScrollingEnabled);
    bool dispatchKeyEvent(KeyEvent& event);
    bool executeKeyEvent(KeyEvent& event);
    void requestDisallowInterceptTouchEvent(bool disallowIntercept);
    bool onInterceptTouchEvent(MotionEvent& ev)override;
    bool onTouchEvent(MotionEvent& ev)override;
    bool onGenericMotionEvent(MotionEvent& event)override;
    bool pageScroll(int direction);
    bool fullScroll(int direction);
    bool arrowScroll(int direction);
    void smoothScrollBy(int dx, int dy);
    void smoothScrollTo(int x, int y);
    int computeVerticalScrollRange();

    int computeVerticalScrollOffset();
    int computeVerticalScrollExtent();
    int computeHorizontalScrollRange();
    int computeHorizontalScrollOffset();
    int computeHorizontalScrollExtent();

    void computeScroll();
    void requestChildFocus(View* child, View* focused);
    bool requestChildRectangleOnScreen(View* child, Rect rectangle, bool immediate);
    void requestLayout();
    void onAttachedToWindow();
    void fling(int velocityY);
    void scrollTo(int x, int y);
    void draw(Canvas& canvas);
};
}/*endof namespace*/
#endif//__NESTED_SCROLLVIEW_H__
