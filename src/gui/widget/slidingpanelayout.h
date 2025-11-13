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
#ifndef __SLIDING_PANEL_LAYOUT_H__
#define __SLIDING_PANEL_LAYOUT_H__
#include <widget/framelayout.h>
#include <widget/viewdraghelper.h>

namespace cdroid{
class SlidingPaneLayout:public ViewGroup {
public:
     /* User can freely swipe between list and detail panes.*/
    static constexpr int LOCK_MODE_UNLOCKED = 0;

    /* The detail pane is locked in an open position. The user cannot swipe to close the detail
     * pane, but the app can close the detail pane programmatically. */
    static constexpr int LOCK_MODE_LOCKED_OPEN = 1;

    /* The detail pane is locked in a closed position. The user cannot swipe to open the detail
     * pane, but the app can open the detail pane programmatically. */
    static constexpr int LOCK_MODE_LOCKED_CLOSED = 2;

    /* The user cannot swipe between list and detail panes, though the app can open or close the
     * detail pane programmatically. */
    static constexpr int LOCK_MODE_LOCKED = 3;
    class LayoutParams:public ViewGroup::MarginLayoutParams {
    public:
        float weight = 0;
        bool slideable=false;
        bool dimWhenOffset=false;
        //Paint dimPaint;
        LayoutParams();
        LayoutParams(int width, int height);
        LayoutParams(const ViewGroup::LayoutParams& source);
        LayoutParams(const ViewGroup::MarginLayoutParams& source);
        LayoutParams(const LayoutParams& source);
        LayoutParams(Context* c, const AttributeSet& attrs);
    };

    struct PanelSlideListener:public EventSet{
        std::function<void(View&/*panel*/,float/*slideOffset*/)>onPanelSlide;
        std::function<void(View&/*panel*/)>onPanelOpened;
        std::function<void(View&/*panel*/)>onPanelClosed;
    };
private:
    static constexpr int DEFAULT_OVERHANG_SIZE = 32; // dp;
    static constexpr int DEFAULT_FADE_COLOR = 0xcccccccc;
    static constexpr int MIN_FLING_VELOCITY = 400; // dips per second
private:
    class DragHelperCallback;
    class SavedState;
    class AccessibilityDelegate;
    class TouchBlocker;
    class DisableLayerRunnable;
    int mSliderFadeColor = DEFAULT_FADE_COLOR;
    int mCoveredFadeColor;
    int mOverhangSize;
    int mParallaxBy;
    Drawable* mShadowDrawableLeft;
    Drawable* mShadowDrawableRight;
    bool mCanSlide;
    bool mFirstLayout = true;
    int mLockMode;
    float mParallaxOffset;
    float mInitialMotionX;
    float mInitialMotionY;
    std::vector<PanelSlideListener>mPanelSlideListeners;
    //Method mGetDisplayList;
    //Field mRecreateDisplayList;
    //bool mDisplayListReflectionLoaded;
protected:
    int mSlideRange;
    float mSlideOffset;
    bool mIsUnableToDrag;
    bool mPreservedOpenState;
    View* mSlideableView;
    ViewDragHelper* mDragHelper;
    std::vector<DisableLayerRunnable> mPostedRunnables;
private:
    void initView();
    static bool viewIsOpaque(View* v);
    bool closePane(int initialVelocity);
    bool openPane(int initialVelocity);
    Insets getSystemGestureInsets();
    void parallaxOtherViews(float slideOffset);
    static int getMinimumWidth(View* child);
    static int measureChildHeight(View* child,int spec, int padding);
protected:
    void dispatchOnPanelSlide(View* panel);
    void dispatchOnPanelOpened(View* panel);
    void dispatchOnPanelClosed(View* panel);
    void updateObscuredViewsVisibility(View* panel);
    void setAllChildrenVisible();
    
    void onAttachedToWindow()override;
    void onDetachedFromWindow()override;
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec)override;
    void onLayout(bool changed, int l, int t, int w, int h)override;
    void onSizeChanged(int w, int h, int oldw, int oldh)override;

    void onPanelDragged(int newLeft);
    bool drawChild(Canvas& canvas, View* child,int64_t drawingTime)override;
    void invalidateChildRegion(View* v);
    bool smoothSlideTo(float slideOffset, int velocity);
    
    bool canScroll(View* v, bool checkV, int dx, int x, int y);
    bool isDimmed(View* child)const;

    ViewGroup::LayoutParams* generateDefaultLayoutParams()const override;
    ViewGroup::LayoutParams* generateLayoutParams(const ViewGroup::LayoutParams* p)const override;
    bool checkLayoutParams(const ViewGroup::LayoutParams* p)const override;
    Parcelable* onSaveInstanceState() override;
    void onRestoreInstanceState(Parcelable& state)override;
public:
    SlidingPaneLayout(int w,int h);
    SlidingPaneLayout(Context* context, const AttributeSet& attrs);
    ~SlidingPaneLayout()override;

    void setLockMode(int);
    int getLockMode()const;
    void setParallaxDistance(int parallaxBy);
    int getParallaxDistance() const;

    void setSliderFadeColor(int color);
    int getSliderFadeColor() const;

    void setCoveredFadeColor(int color);
    int getCoveredFadeColor() const;

    void setPanelSlideListener(const PanelSlideListener& listener);
    void addPanelSlideListener(const PanelSlideListener& listener);
    void removePanelSlideListener(const PanelSlideListener& listener);

    void addView(View* child, int index, ViewGroup::LayoutParams* params)override;
    void removeView(View*)override;
    void requestChildFocus(View* child, View* focused) override;
    bool onInterceptTouchEvent(MotionEvent& ev) override;
    bool onTouchEvent(MotionEvent& ev) override;

    bool openPane();
    bool closePane();
    bool isOpen() const;
    bool isSlideable() const;

    void computeScroll() override;

    void setShadowDrawableLeft(Drawable* d);
    void setShadowDrawableRight(Drawable* d);

    void setShadowResourceLeft(const std::string& resId);
    void setShadowResourceRight(const std::string& resId);
    void draw(Canvas& c)override;

    ViewGroup::LayoutParams* generateLayoutParams(const AttributeSet& attrs)const override;

    bool isLayoutRtlSupport() const;
};/*endof SlidingPaneLayout*/

class SlidingPaneLayout::DragHelperCallback:public ViewDragHelper::Callback {
private:
    SlidingPaneLayout*mSPL;
    bool isDraggable()const;
public:
    DragHelperCallback(SlidingPaneLayout*spl);
    bool tryCaptureView(View& child, int pointerId)override;
    void onViewDragStateChanged(int state) override;

    void onViewCaptured(View& capturedChild, int activePointerId) override;
    void onViewPositionChanged(View& changedView, int left, int top, int dx, int dy) override;
    void onViewReleased(View& releasedChild, float xvel, float yvel)override;

    int getViewHorizontalDragRange(View& child) override;
    int clampViewPositionHorizontal(View& child, int left, int dx)override;

    int clampViewPositionVertical(View& child, int top, int dy) override;
    void onEdgeTouched(int edgeFlags, int pointerId)override;
    void onEdgeDragStarted(int edgeFlags, int pointerId) override;
};

class SlidingPaneLayout::SavedState:public AbsSavedState {
    bool isOpen;
    int mLockMode;
public:
    SavedState(Parcelable superState);
    //SavedState(Parcel in, ClassLoader loader);
    void writeToParcel(Parcel& out, int flags)override;
};

class SlidingPaneLayout::AccessibilityDelegate:public View::AccessibilityDelegate{
private:
    void copyNodeInfoNoChildren(AccessibilityNodeInfo& dest, AccessibilityNodeInfo& src);
public:
    void onInitializeAccessibilityNodeInfo(View& host, AccessibilityNodeInfo& info)override;
    void onInitializeAccessibilityEvent(View& host, AccessibilityEvent& event) override;
    bool onRequestSendAccessibilityEvent(ViewGroup& host, View& child, AccessibilityEvent& event) override;

    bool filter(View child);
};

class SlidingPaneLayout::TouchBlocker:public FrameLayout {
public:
    TouchBlocker(View* view):FrameLayout(1,1){
        addView(view);
    }
    bool onTouchEvent(MotionEvent& event) override{
        return true;
    }
    bool onGenericMotionEvent(MotionEvent& event) override{
        return true;
    }
};

class SlidingPaneLayout::DisableLayerRunnable:public ViewRunnable {
    View* mChildView;
public:
    DisableLayerRunnable(View*v,View* childView);
    void run() override;
};
}/*endof namespace*/
#endif/*__SLIDING_PANEL_LAYOUT_H__*/
