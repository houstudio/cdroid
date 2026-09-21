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
#ifndef __WEARABLE_DRAWER_LAYOUT_H__
#define __WEARABLE_DRAWER_LAYOUT_H__
#include <widget/framelayout.h>
#include <widget/viewdraghelper.h>
#include <view/viewtreeobserver.h>
#include <widgetEx/wear/flingwatcherfactory.h>
#include <vector>

namespace cdroid{

class WearableDrawerView;
class WearableDrawerController;
class NestedScrollingParentHelper;

/** Top-level container that allows interactive drawers to be pulled from the top and bottom edge
 *  of the window. For WearableDrawerLayout to work properly, scrolling children must send nested
 *  scrolling events: views implementing NestedScrollingChild do this by default, framework views
 *  like ListView need android:nestedScrollingEnabled="true". This includes the main content in a
 *  WearableDrawerLayout, as well as the content inside of the drawers.
 *  androidx.wear.widget.drawer.WearableDrawerLayout.java (lines 102-1212). */
class WearableDrawerLayout: public FrameLayout, public FlingWatcherFactory::FlingListener {
public:
    /** Listener for monitoring events about drawers. (lines 919-941) */
    class DrawerStateCallback {
    public:
        virtual ~DrawerStateCallback() = default;

        /** Called when a drawer has settled in a completely open state. The drawer is interactive
            at this point. */
        virtual void onDrawerOpened(WearableDrawerLayout& layout, WearableDrawerView& drawerView);

        /** Called when a drawer has settled in a completely closed state. */
        virtual void onDrawerClosed(WearableDrawerLayout& layout, WearableDrawerView& drawerView);

        /** Called when the drawer motion state changes. The new state will be one of
            WearableDrawerView STATE_IDLE, STATE_DRAGGING or STATE_SETTLING. */
        virtual void onDrawerStateChanged(WearableDrawerLayout& layout, int newState);
    };

    /** Base class for top and bottom drawer dragger callbacks. (lines 970-1042) */
    class DrawerDraggerCallback:public ViewDragHelper::Callback {
    protected:
        /** The enclosing WearableDrawerLayout (Java inner classes get this implicitly). */
        WearableDrawerLayout* mDrawerLayout;
    public:
        explicit DrawerDraggerCallback(WearableDrawerLayout* drawerLayout);

        virtual WearableDrawerView* getDrawerView() = 0;

        bool tryCaptureView(View& child, int pointerId) override;

        /** Defines the vertical drag range of the drawer. */
        int getViewVerticalDragRange(View& child) override;

        void onViewCaptured(View& capturedChild, int activePointerId) override;

        void onViewDragStateChanged(int state) override;
    };

    /** For communicating with top drawer view dragger. (lines 1047-1118) */
    class TopDrawerDraggerCallback:public DrawerDraggerCallback {
    public:
        explicit TopDrawerDraggerCallback(WearableDrawerLayout* drawerLayout);

        int clampViewPositionVertical(View& child, int top, int dy) override;

        void onEdgeDragStarted(int edgeFlags, int pointerId) override;

        void onViewReleased(View& releasedChild, float xvel, float yvel) override;

        void onViewPositionChanged(View& changedView, int left, int top, int dx, int dy) override;

        WearableDrawerView* getDrawerView() override;
    };

    /** For communicating with bottom drawer view dragger. (lines 1123-1189) */
    class BottomDrawerDraggerCallback:public DrawerDraggerCallback {
    public:
        explicit BottomDrawerDraggerCallback(WearableDrawerLayout* drawerLayout);

        int clampViewPositionVertical(View& child, int top, int dy) override;

        void onEdgeDragStarted(int edgeFlags, int pointerId) override;

        void onViewReleased(View& releasedChild, float xvel, float yvel) override;

        void onViewPositionChanged(View& changedView, int left, int top, int dx, int dy) override;

        WearableDrawerView* getDrawerView() override;
    };

    WearableDrawerLayout(Context* context);
    WearableDrawerLayout(Context* context, const AttributeSet* attrs);
    WearableDrawerLayout(Context* context, const AttributeSet* attrs, int defStyleAttr);
    WearableDrawerLayout(Context* context, const AttributeSet* attrs, int defStyleAttr, int defStyleRes);
    ~WearableDrawerLayout() override;

    /** Records the bottom system window inset and mirrors it into this layout's bottom margin.
     *  (lines 322-334) */
    WindowInsets onApplyWindowInsets(const WindowInsets& insets) override;

    // The methods below are package-private upstream; the WearableDrawerController handles and
    // drawer subclasses reach them through this class (C++ has no package visibility).

    /** Closes drawer after {@code delayMs} milliseconds. (lines 339-352) */
    void closeDrawerDelayed(int gravity, long delayMs);

    /** Close the specified drawer by animating it out of view.
        @param gravity Gravity.TOP to move the top drawer or Gravity.BOTTOM for the bottom. */
    void closeDrawer(int gravity);

    /** Close the specified drawer by animating it out of view.
        @param drawer The drawer view to close. */
    void closeDrawer(WearableDrawerView* drawer);

    /** Open the specified drawer by animating it into view.
        @param gravity Gravity.TOP to move the top drawer or Gravity.BOTTOM for the bottom. */
    void openDrawer(int gravity);

    /** Open the specified drawer by animating it into view.
        @param drawer The drawer view to open. */
    void openDrawer(WearableDrawerView* drawer);

    /** Peek the drawer.
        @param gravity Gravity.TOP to peek the top drawer or Gravity.BOTTOM to peek the bottom. */
    void peekDrawer(int gravity);

    /** Peek the given WearableDrawerView, which may either be the top drawer or bottom drawer.
        This should only be used after the drawer has been added as a child of this layout.
        @throws std::invalid_argument if the drawer is not a child of this layout. */
    void peekDrawer(WearableDrawerView* drawer);

    bool onInterceptTouchEvent(MotionEvent& ev) override;

    bool onTouchEvent(MotionEvent& ev) override;

    void computeScroll() override;

    void addView(View* child, int index, ViewGroup::LayoutParams* params) override;
    // C++ name hiding: re-expose the remaining ViewGroup::addView overloads as one
    // flat overload set (Java's); they dispatch into the override above.
    using ViewGroup::addView;

    /** Lays a child drawer out based on its openedPercent. Implements the
        View.OnLayoutChangeListener registered on the drawers in addView. (lines 572-596) */
    void onLayoutChange(View& v, int left, int top, int right, int bottom,
            int oldLeft, int oldTop, int oldRight, int oldBottom);

    /** Sets a listener to be notified of drawer events. Borrowed pointer. (lines 601-603) */
    void setDrawerStateCallback(DrawerStateCallback* callback);

    /** FlingWatcherFactory.FlingListener: called when a watched scrolling view settles. */
    void onFlingComplete(View& view) override;

    // NestedScrollingParent (the 1-type base virtuals; TYPE_TOUCH dispatch lands here)
    int getNestedScrollAxes() override;
    bool onNestedFling(View* target, float velocityX, float velocityY, bool consumed) override;
    bool onNestedPreFling(View* target, float velocityX, float velocityY) override;
    void onNestedPreScroll(View* target, int dx, int dy, int* consumed) override;
    void onNestedScroll(View* target, int dxConsumed, int dyConsumed,
            int dxUnconsumed, int dyUnconsumed) override;
    void onNestedScrollAccepted(View* child, View* target, int axes) override;
    bool onStartNestedScroll(View* child, View* target, int axes) override;
    void onStopNestedScroll(View* target) override;

    /** (lines 902-914; the drawer is @Nullable upstream, so a null drawer reports "cannot
        scroll" the same way a null/empty content view does) */
    bool canDrawerContentScrollVertically(WearableDrawerView* drawerView, int direction) const;

    /** (lines 943-951) Accessibility focus is limited to all children only while enabled. */
    void allowAccessibilityFocusOnAllChildren();

    /** (lines 953-965) Restricts accessibility focus to the given drawer while enabled. */
    void allowAccessibilityFocusOnOnly(WearableDrawerView* drawer);

protected:
    void onLayout(bool changed, int left, int top, int width, int height) override;

private:
    /** Undefined layout_gravity. This is different from Gravity#NO_GRAVITY (b/27576632). */
    static constexpr int GRAVITY_UNDEFINED = -1;
    static constexpr int PEEK_FADE_DURATION_MS = 150;
    static constexpr int PEEK_AUTO_CLOSE_DELAY_MS = 1000;
    /** The downward scroll direction for use as a parameter to canScrollVertically. */
    static constexpr int DOWN = 1;
    /** The upward scroll direction for use as a parameter to canScrollVertically. */
    static constexpr int UP = -1;
    /** The percent at which the drawer will be opened when the drawer is released mid-drag. */
    static constexpr float OPENED_PERCENT_THRESHOLD = 0.5f;
    /** When a user lifts their finger off the screen, this may trigger a couple of small scroll
        events. If the user is scrolling down and the final events are up, this would peek the
        bottom drawer; the bottom drawer may not peek until this amount of scroll is exceeded. */
    static constexpr int NESTED_SCROLL_SLOP_DP = 5;

    // @VisibleForTesting upstream. CDROID's ViewDragHelper owns (deletes) its Callback, so these
    // are borrowed handles kept for parity with the upstream fields.
    DrawerDraggerCallback* mTopDrawerDraggerCallback = nullptr;
    DrawerDraggerCallback* mBottomDrawerDraggerCallback = nullptr;
    const int mNestedScrollSlopPx;
    NestedScrollingParentHelper* mNestedScrollingParentHelper = nullptr; // owned
    /** Helper for dragging the top drawer. */
    ViewDragHelper* mTopDrawerDragger = nullptr;
    /** Helper for dragging the bottom drawer. */
    ViewDragHelper* mBottomDrawerDragger = nullptr;
    const bool mIsAccessibilityEnabled;
    FlingWatcherFactory mFlingWatcher;
    /** Upstream posts ClosePeekRunnables on a main-looper Handler; CDROID posts the equivalent
        runnables through View::postDelayed/removeCallbacks (also the main looper). */
    Runnable mCloseTopPeekRunnable;    // upstream ClosePeekRunnable(Gravity.TOP), lines 1194-1211
    Runnable mCloseBottomPeekRunnable; // upstream ClosePeekRunnable(Gravity.BOTTOM)
    /** Controllers handed to drawer children by addView; owned here (GC'd upstream, lines 548). */
    std::vector<WearableDrawerController*> mDrawerControllers;
    /** Top drawer view. */
    WearableDrawerView* mTopDrawerView = nullptr;
    /** Bottom drawer view. */
    WearableDrawerView* mBottomDrawerView = nullptr;
    /** What we have inferred the scrolling content view to be, should one exist. */
    View* mScrollingContentView = nullptr;
    /** Listens to drawer events; borrowed. */
    DrawerStateCallback* mDrawerStateCallback = nullptr;
    int mSystemWindowInsetBottom = 0;
    /** Tracks the amount of nested scroll in the up direction; used with NESTED_SCROLL_SLOP_DP
        to prevent false drawer peeks. */
    int mCurrentNestedScrollSlopTracker = 0;
    /** Tracks whether the top drawer should be opened after layout. */
    bool mShouldOpenTopDrawerAfterLayout = false;
    /** Tracks whether the bottom drawer should be opened after layout. */
    bool mShouldOpenBottomDrawerAfterLayout = false;
    /** Tracks whether the top drawer should be peeked after layout. */
    bool mShouldPeekTopDrawerAfterLayout = false;
    /** Tracks whether the bottom drawer should be peeked after layout. */
    bool mShouldPeekBottomDrawerAfterLayout = false;
    /** True while the open top drawer's content can still scroll; the dragger should not
        intercept until the content is scrolled to its bottom. */
    bool mCanTopDrawerBeClosed = false;
    /** True while the open bottom drawer's content can still scroll; the dragger should not
        intercept until the content is scrolled to its top. */
    bool mCanBottomDrawerBeClosed = false;
    /** Tracks whether the last scroll resulted in a fling. Fling events do not contain the
        amount scrolled, so if the last scroll was a fling and the next scroll unlocks the
        drawer, mDrawerOpenLastInterceptedTouchEvent is re-fed into onTouchEvent. */
    bool mLastScrollWasFling = false;
    /** Clone of the last intercepted touch event; see mLastScrollWasFling. Upstream keeps the
        borrowed event (line 506); CDROID input events are pooled, so a private clone is kept,
        deleted on replace and in the destructor. */
    MotionEvent* mDrawerOpenLastInterceptedTouchEvent = nullptr;
    /** This layout as the drawers' OnLayoutChangeListener; forwards to onLayoutChange. */
    View::OnLayoutChangeListener mOnLayoutChangeListener;
    /** One-shot post-layout listener (the anonymous OnGlobalLayoutListener, lines 612-634). */
    ViewTreeObserver::OnGlobalLayoutListener mGlobalLayoutListener;

    /** Fades the drawer content out and the peek back in after the drawer was closed.
        (lines 260-287) */
    static void animatePeekVisibleAfterBeingClosed(WearableDrawerView& drawer);

    /** Shows the drawer's contents. If the drawer is peeking, an animation is used to fade out
        the peek view and fade in the drawer content. (lines 293-320) */
    static void showDrawerContentMaybeAnimate(WearableDrawerView& drawerView);

    /** Peeks the given drawer if it is not null and has a peek view. (lines 774-812) */
    void maybePeekDrawer(WearableDrawerView* drawerView);

    /** (lines 814-837) */
    void openDrawerWithoutAnimation(WearableDrawerView* drawer);

    /** @param gravity the gravity of the child to return.
        @return the drawer with the specified gravity, or null. (lines 843-853) */
    WearableDrawerView* findDrawerWithGravity(int gravity) const;

    /** Updates mScrollingContentView if view is not a descendant of a WearableDrawerView.
        (lines 859-863) */
    void maybeUpdateScrollingContentView(View* view);

    /** Returns true if view is a descendant of a WearableDrawerView. (lines 868-878) */
    bool isDrawerOrChildOfDrawer(View* view) const;

    /** (lines 880-882) */
    bool isClosingPeek(WearableDrawerView* drawerView) const;

    /** Body of the ClosePeekRunnables: closes the drawer if it is just peeking. */
    void closePeek(int gravity);
};

}/*endof namespace*/
#endif/*__WEARABLE_DRAWER_LAYOUT_H__*/
