#ifndef __HORIZONTAL_SCROLLVIEW_H__
#define __HORIZONTAL_SCROLLVIEW_H__
#include <widget/framelayout.h>
#include <widget/overscroller.h>
#include <widget/edgeeffect.h>

namespace cdroid{

class HorizontalScrollView:public FrameLayout{
private:
    long mLastScroll;
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

    int mTouchSlop;
    int mMinimumVelocity;
    int mMaximumVelocity;

    int mOverscrollDistance;
    int mOverflingDistance;

    float mHorizontalScrollFactor;
    int mActivePointerId;
    Rect mTempRect;
private:
    static constexpr float MAX_SCROLL_FACTOR =0.5f;
    static constexpr int ANIMATED_SCROLL_GAP = 250;
    void initScrollView();
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
    void scrollToChild(View* child);
    bool scrollToChildRect(Rect rect, bool immediate);
    bool isViewDescendantOf(View* child, View* parent);
protected:
    float getLeftFadingEdgeStrength();
    float getRightFadingEdgeStrength();
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec)override;
    void onOverScrolled(int scrollX, int scrollY,bool clampedX, bool clampedY)override;
    int computeHorizontalScrollRange()override;
    int computeHorizontalScrollOffset()override;
    void measureChild(View* child, int parentWidthMeasureSpec,int parentHeightMeasureSpec);
    void measureChildWithMargins(View* child, int parentWidthMeasureSpec, int widthUsed,
            int parentHeightMeasureSpec, int heightUsed);
    int computeScrollDeltaToGetChildRectOnScreen(Rect& rect);
    bool onRequestFocusInDescendants(int direction,Rect* previouslyFocusedRect)override;
    void onLayout(bool changed, int l, int t, int w, int h)override;
    void onSizeChanged(int w, int h, int oldw, int oldh);
    void draw(Canvas& canvas);
public:
    HorizontalScrollView(int w,int h);
    HorizontalScrollView(Context*ctx,const AttributeSet&atts);
    ~HorizontalScrollView();
    int getMaxScrollAmount();
    View& addView(View* child);
    View& addView(View* child, int index);
    View& addView(View* child, ViewGroup::LayoutParams* params);
    View& addView(View* child, int index,ViewGroup::LayoutParams* params);
    bool isFillViewport()const;
    void setFillViewport(bool fillViewport);
    bool isSmoothScrollingEnabled()const;
    void setSmoothScrollingEnabled(bool smoothScrollingEnabled);
    bool dispatchKeyEvent(KeyEvent& event)override;
    bool executeKeyEvent(KeyEvent& event);
    void requestDisallowInterceptTouchEvent(bool disallowIntercept);
    bool onInterceptTouchEvent(MotionEvent& ev)override;
    bool onTouchEvent(MotionEvent& ev)override;
    bool onGenericMotionEvent(MotionEvent& event);
    bool shouldDelayChildPressedState()override;
    bool pageScroll(int direction);
    bool fullScroll(int direction);
    bool arrowScroll(int direction);
    void smoothScrollBy(int dx, int dy);
    void smoothScrollTo(int x, int y);
    void computeScroll();
    void requestChildFocus(View* child, View* focused)override;
    bool requestChildRectangleOnScreen(View* child, Rect rectangle,bool immediate);
    void requestLayout();
    void fling(int velocityX);
    void scrollTo(int x, int y);
    void setOverScrollMode(int mode);
};

}
#endif
