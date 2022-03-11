#ifndef __SCROLL_VIEW__
#define __SCROLL_VIEW__
#include <widget/framelayout.h>
#include <widget/overscroller.h>
#include <widget/edgeeffect.h>

namespace cdroid{

class ScrollView:public FrameLayout{
private:
    long mLastScroll;
    int mLastMotionY;
    bool mIsLayoutDirty=true;
    View* mChildToScrollTo = nullptr;
    bool mIsBeingDragged = false;
    bool mFillViewport;
    bool mSmoothScrollingEnabled = true;
    int mTouchSlop;
    int mMinimumVelocity;
    int mMaximumVelocity;

    int mOverscrollDistance;
    int mOverflingDistance;

    OverScroller* mScroller;
    VelocityTracker*mVelocityTracker;
    EdgeEffect* mEdgeGlowTop;
    EdgeEffect* mEdgeGlowBottom;
    float mVerticalScrollFactor;
    int mActivePointerId =-1/*INVALID_POINTER */;
    int mScrollOffset[2] ;
    int mScrollConsumed[2];
    int mNestedYOffset;
    Rect mTempRect;

    void initScrollView();
    bool canScroll();
    bool inChild(int x, int y);
    void onSecondaryPointerUp(MotionEvent& ev);
    int getScrollRange();
    View* findFocusableViewInBounds(bool topFocus, int top, int bottom);
    bool scrollAndFocus(int direction, int top, int bottom);
    bool isOffScreen(View* descendant);
    bool isWithinDeltaOfScreen(View* descendant, int delta, int height);
    void doScrollY(int delta);
    void smoothScrollBy(int dx, int dy);
    void scrollToChild(View* child);
    bool scrollToChildRect(Rect& rect, bool immediate);
    static bool isViewDescendantOf(View* child, View* parent);
    static int clamp(int n, int my, int child);
    void flingWithNestedDispatch(int velocityY);
    void endDrag();
protected:
    static constexpr float MAX_SCROLL_FACTOR =0.5f;
    static constexpr int INVALID_POINTER=-1;
    static constexpr int ANIMATED_SCROLL_GAP = 250;
    float getTopFadingEdgeStrength();
    float getBottomFadingEdgeStrength();
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec);
    void onOverScrolled(int scrollX, int scrollY, bool clampedX, bool clampedY)override;
    int computeVerticalScrollRange();
    int computeVerticalScrollOffset();
    void initOrResetVelocityTracker();
    void initVelocityTrackerIfNotExists();
    void recycleVelocityTracker();
    void measureChild(View* child, int parentWidthMeasureSpec,int parentHeightMeasureSpec);
    void measureChildWithMargins(View* child, int parentWidthMeasureSpec, int widthUsed,
            int parentHeightMeasureSpec, int heightUsed);
    int computeScrollDeltaToGetChildRectOnScreen(Rect& rect);
    bool onRequestFocusInDescendants(int direction,Rect* previouslyFocusedRect)override;
    bool requestChildRectangleOnScreen(View* child,Rect& rectangle, bool immediate)override;
    void onLayout(bool changed, int l, int t, int w, int h)override;
    void draw(Canvas& canvas)override;
public:
    ScrollView(int w,int h);
    ScrollView(Context*ctx,const AttributeSet&atts);
    int getMaxScrollAmount();
    View& addView(View* child)override;
    View& addView(View* child, int index)override;
    View& addView(View* child, ViewGroup::LayoutParams* params)override;
    View& addView(View* child, int index, ViewGroup::LayoutParams* params)override;
    bool isFillViewport()const;
    void setFillViewport(bool fillViewport);
    bool isSmoothScrollingEnabled()const;
    void setSmoothScrollingEnabled(bool smoothScrollingEnabled);
    bool executeKeyEvent(KeyEvent& event);
    bool onInterceptTouchEvent(MotionEvent& ev)override;
    bool onTouchEvent(MotionEvent& ev)override;
    bool onGenericMotionEvent(MotionEvent& event);
    bool pageScroll(int direction);
    bool fullScroll(int direction);
    bool arrowScroll(int direction);
    void smoothScrollTo(int x, int y);
    void computeScroll();
    void requestChildFocus(View* child, View* focused)override;
    void requestLayout()override;
    void scrollTo(int x, int y)override;
    void setOverScrollMode(int mode);
    bool onStartNestedScroll(View* child, View* target, int nestedScrollAxes)override;
    void onNestedScrollAccepted(View* child, View* target, int axes)override;
    void onStopNestedScroll(View* target)override;
    void onNestedScroll(View* target, int dxConsumed, int dyConsumed,
            int dxUnconsumed, int dyUnconsumed);
    bool onNestedFling(View* target, float velocityX, float velocityY, bool consumed);
    void fling(int velocityY);
};

}
#endif//__SCROLL_VIEW__
