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
#include <widgetEx/wear/wearabledrawerlayout.h>
#include <widgetEx/wear/wearabledrawerview.h>
#include <widgetEx/wear/wearabledrawercontroller.h>
#include <widget/nestedscrollinghelper.h>
#include <view/gravity.h>
#include <view/layoutinflater.h>
#include <view/accessibility/accessibilitymanager.h>
#include <porting/cdlog.h>
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace cdroid{

// androidx.wear.widget.drawer.WearableDrawerLayout.java (lines 102-1212)
DECLARE_WIDGET2(WearableDrawerLayout, "androidx.wear.widget.drawer.WearableDrawerLayout");

WearableDrawerLayout::WearableDrawerLayout(Context* context)
    : WearableDrawerLayout(context, nullptr) {
}

WearableDrawerLayout::WearableDrawerLayout(Context* context, const AttributeSet* attrs)
    : WearableDrawerLayout(context, attrs, 0) {
}

WearableDrawerLayout::WearableDrawerLayout(Context* context, const AttributeSet* attrs,
        int defStyleAttr)
    : WearableDrawerLayout(context, attrs, defStyleAttr, 0) {
}

WearableDrawerLayout::WearableDrawerLayout(Context* context, const AttributeSet* attrs,
        int defStyleAttr, int defStyleRes)
    : FrameLayout(context, attrs, defStyleAttr, defStyleRes),
      // Math.round(density * NESTED_SCROLL_SLOP_DP); upstream reads the DisplayMetrics off the
      // WindowManager's default display (lines 249-253), CDROID carries them on Context.
      mNestedScrollSlopPx((int) std::floor(
              context->getDisplayMetrics().density * NESTED_SCROLL_SLOP_DP + 0.5f)),
      mIsAccessibilityEnabled(AccessibilityManager::getInstance(context).isEnabled()),
      mFlingWatcher(*this) {

    mTopDrawerDraggerCallback = new TopDrawerDraggerCallback(this);
    mTopDrawerDragger = ViewDragHelper::create(this, 1.f /* sensitivity */, mTopDrawerDraggerCallback);
    mTopDrawerDragger->setEdgeTrackingEnabled(ViewDragHelper::EDGE_TOP);

    mBottomDrawerDraggerCallback = new BottomDrawerDraggerCallback(this);
    mBottomDrawerDragger = ViewDragHelper::create(this, 1.f /* sensitivity */, mBottomDrawerDraggerCallback);
    mBottomDrawerDragger->setEdgeTrackingEnabled(ViewDragHelper::EDGE_BOTTOM);

    mNestedScrollingParentHelper = new NestedScrollingParentHelper(this);

    // Upstream keeps Handler-posted ClosePeekRunnable instances (lines 1194-1211); the same
    // runnables are posted through View::postDelayed/removeCallbacks here.
    mCloseTopPeekRunnable = [this]() { closePeek(Gravity::TOP); };
    mCloseBottomPeekRunnable = [this]() { closePeek(Gravity::BOTTOM); };

    // Upstream implements View.OnLayoutChangeListener on the layout itself (lines 572-596);
    // CDROID listeners are functors, so this member forwards to the same method.
    mOnLayoutChangeListener = [this](View& v, int left, int top, int right, int bottom,
            int oldLeft, int oldTop, int oldRight, int oldBottom) {
        onLayoutChange(v, left, top, right, bottom, oldLeft, oldTop, oldRight, oldBottom);
    };

    // The upstream one-shot anonymous OnGlobalLayoutListener (lines 612-634).
    mGlobalLayoutListener = [this]() {
        getViewTreeObserver()->removeOnGlobalLayoutListener(mGlobalLayoutListener);
        if (mShouldOpenBottomDrawerAfterLayout) {
            openDrawerWithoutAnimation(mBottomDrawerView);
            mShouldOpenBottomDrawerAfterLayout = false;
        } else if (mShouldPeekBottomDrawerAfterLayout) {
            peekDrawer(Gravity::BOTTOM);
            mShouldPeekBottomDrawerAfterLayout = false;
        }

        if (mShouldOpenTopDrawerAfterLayout) {
            openDrawerWithoutAnimation(mTopDrawerView);
            mShouldOpenTopDrawerAfterLayout = false;
        } else if (mShouldPeekTopDrawerAfterLayout) {
            peekDrawer(Gravity::TOP);
            mShouldPeekTopDrawerAfterLayout = false;
        }
    };
}

WearableDrawerLayout::~WearableDrawerLayout() {
    delete mDrawerOpenLastInterceptedTouchEvent;
    delete mTopDrawerDragger;    // owns and deletes mTopDrawerDraggerCallback
    delete mBottomDrawerDragger; // owns and deletes mBottomDrawerDraggerCallback
    mTopDrawerDraggerCallback = nullptr;
    mBottomDrawerDraggerCallback = nullptr;
    delete mNestedScrollingParentHelper;
    for (WearableDrawerController* controller : mDrawerControllers) {
        delete controller;
    }
}

void WearableDrawerLayout::animatePeekVisibleAfterBeingClosed(WearableDrawerView& drawer) {
    View* content = drawer.getDrawerContent();
    if (content != nullptr) {
        content->animate()
                .setDuration(PEEK_FADE_DURATION_MS)
                .alpha(0)
                .withEndAction([content]() { content->setVisibility(View::GONE); })
                .start();
    }

    ViewGroup* peek = drawer.getPeekContainer();
    peek->setVisibility(View::VISIBLE);
    peek->animate()
            .setStartDelay(PEEK_FADE_DURATION_MS)
            .setDuration(PEEK_FADE_DURATION_MS)
            .alpha(1)
            .scaleX(1)
            .scaleY(1)
            .start();

    drawer.setIsPeeking(true);
}

void WearableDrawerLayout::showDrawerContentMaybeAnimate(WearableDrawerView& drawerView) {
    drawerView.bringToFront();
    View* contentView = drawerView.getDrawerContent();
    if (contentView != nullptr) {
        contentView->setVisibility(View::VISIBLE);
    }

    if (drawerView.isPeeking()) {
        View* peekView = drawerView.getPeekContainer();
        peekView->animate().alpha(0).scaleX(0).scaleY(0)
                .setDuration(PEEK_FADE_DURATION_MS).start();

        if (contentView != nullptr) {
            contentView->setAlpha(0);
            contentView->animate()
                    .setStartDelay(PEEK_FADE_DURATION_MS)
                    .alpha(1)
                    .setDuration(PEEK_FADE_DURATION_MS)
                    .start();
        }
    } else {
        drawerView.getPeekContainer()->setAlpha(0);
        if (contentView != nullptr) {
            contentView->setAlpha(1);
        }
    }
}

WindowInsets WearableDrawerLayout::onApplyWindowInsets(const WindowInsets& insets) {
    mSystemWindowInsetBottom = insets.getSystemWindowInsetBottom();

    if (mSystemWindowInsetBottom != 0) {
        MarginLayoutParams* layoutParams = static_cast<MarginLayoutParams*>(getLayoutParams());
        layoutParams->bottomMargin = mSystemWindowInsetBottom;
        setLayoutParams(layoutParams);
    }

    return FrameLayout::onApplyWindowInsets(insets);
}

void WearableDrawerLayout::closeDrawerDelayed(int gravity, long delayMs) {
    switch (gravity) {
    case Gravity::TOP:
        removeCallbacks(mCloseTopPeekRunnable);
        postDelayed(mCloseTopPeekRunnable, delayMs);
        break;
    case Gravity::BOTTOM:
        removeCallbacks(mCloseBottomPeekRunnable);
        postDelayed(mCloseBottomPeekRunnable, delayMs);
        break;
    default:
        LOGW("Invoked a delayed drawer close with an invalid gravity: %d", gravity);
    }
}

void WearableDrawerLayout::closeDrawer(int gravity) {
    closeDrawer(findDrawerWithGravity(gravity));
}

void WearableDrawerLayout::closeDrawer(WearableDrawerView* drawer) {
    if (drawer == nullptr) {
        return;
    }
    if (drawer == mTopDrawerView) {
        mTopDrawerDragger->smoothSlideViewTo(
                mTopDrawerView, 0 /* finalLeft */, -mTopDrawerView->getHeight());
        invalidate();
    } else if (drawer == mBottomDrawerView) {
        mBottomDrawerDragger->smoothSlideViewTo(
                mBottomDrawerView, 0 /* finalLeft */, getHeight());
        invalidate();
    } else {
        LOGW("closeDrawer(View) should be passed in the top or bottom drawer");
    }
}

void WearableDrawerLayout::openDrawer(int gravity) {
    if (!isLaidOut()) {
        switch (gravity) {
        case Gravity::TOP:
            mShouldOpenTopDrawerAfterLayout = true;
            break;
        case Gravity::BOTTOM:
            mShouldOpenBottomDrawerAfterLayout = true;
            break;
        default: // fall out
            ;
        }
        return;
    }
    openDrawer(findDrawerWithGravity(gravity));
}

void WearableDrawerLayout::openDrawer(WearableDrawerView* drawer) {
    if (drawer == nullptr) {
        return;
    }
    if (!isLaidOut()) {
        if (drawer == mTopDrawerView) {
            mShouldOpenTopDrawerAfterLayout = true;
        } else if (drawer == mBottomDrawerView) {
            mShouldOpenBottomDrawerAfterLayout = true;
        }
        return;
    }

    if (drawer == mTopDrawerView) {
        mTopDrawerDragger->smoothSlideViewTo(
                mTopDrawerView, 0 /* finalLeft */, 0 /* finalTop */);
        showDrawerContentMaybeAnimate(*mTopDrawerView);
        invalidate();
    } else if (drawer == mBottomDrawerView) {
        mBottomDrawerDragger->smoothSlideViewTo(mBottomDrawerView, 0 /* finalLeft */,
                getHeight() - mBottomDrawerView->getHeight());
        showDrawerContentMaybeAnimate(*mBottomDrawerView);
        invalidate();
    } else {
        LOGW("openDrawer(View) should be passed in the top or bottom drawer");
    }
}

void WearableDrawerLayout::peekDrawer(int gravity) {
    if (!isLaidOut()) {
        // If this view is not laid out yet, postpone the peek until onLayout is called.
        LOGD("WearableDrawerLayout not laid out yet. Postponing peek.");
        switch (gravity) {
        case Gravity::TOP:
            mShouldPeekTopDrawerAfterLayout = true;
            break;
        case Gravity::BOTTOM:
            mShouldPeekBottomDrawerAfterLayout = true;
            break;
        default: // fall out
            ;
        }
        return;
    }

    WearableDrawerView* drawerView = findDrawerWithGravity(gravity);
    maybePeekDrawer(drawerView);
}

void WearableDrawerLayout::peekDrawer(WearableDrawerView* drawer) {
    if (drawer == nullptr) {
        throw std::invalid_argument("peekDrawer(WearableDrawerView) received a null drawer.");
    } else if (drawer != mTopDrawerView && drawer != mBottomDrawerView) {
        throw std::invalid_argument(
                "peekDrawer(WearableDrawerView) received a drawer that isn't a child.");
    }

    if (!isLaidOut()) {
        // If this view is not laid out yet, postpone the peek until onLayout is called.
        LOGD("WearableDrawerLayout not laid out yet. Postponing peek.");
        if (drawer == mTopDrawerView) {
            mShouldPeekTopDrawerAfterLayout = true;
        } else if (drawer == mBottomDrawerView) {
            mShouldPeekBottomDrawerAfterLayout = true;
        }
        return;
    }

    maybePeekDrawer(drawer);
}

bool WearableDrawerLayout::onInterceptTouchEvent(MotionEvent& ev) {
    // Do not intercept touch events if a drawer is open. If the content in a drawer scrolls,
    // then the touch event can be intercepted if the content in the drawer is scrolled to
    // the maximum opposite of the drawer's gravity (ex: the touch event can be intercepted
    // if the top drawer is open and scrolling content is at the bottom.
    if ((mBottomDrawerView != nullptr && mBottomDrawerView->isOpened() && !mCanBottomDrawerBeClosed)
            || (mTopDrawerView != nullptr && mTopDrawerView->isOpened()
            && !mCanTopDrawerBeClosed)) {
        // Upstream keeps the borrowed event (line 506); CDROID touch events are pooled, so
        // keep a private clone (deleted on replace and in the destructor).
        delete mDrawerOpenLastInterceptedTouchEvent;
        mDrawerOpenLastInterceptedTouchEvent = MotionEvent::obtain(ev);
        return false;
    }

    // Delegate event to drawer draggers.
    const bool shouldInterceptTop = mTopDrawerDragger->shouldInterceptTouchEvent(ev);
    const bool shouldInterceptBottom = mBottomDrawerDragger->shouldInterceptTouchEvent(ev);
    return shouldInterceptTop || shouldInterceptBottom;
}

bool WearableDrawerLayout::onTouchEvent(MotionEvent& ev) {
    // The upstream null-event check (lines 518-521) is unportable: CDROID hands the event by
    // reference. Callers re-feeding mDrawerOpenLastInterceptedTouchEvent null-check it instead.
    // Delegate event to drawer draggers.
    mTopDrawerDragger->processTouchEvent(ev);
    mBottomDrawerDragger->processTouchEvent(ev);
    return true;
}

void WearableDrawerLayout::computeScroll() {
    // For scrolling the drawers.
    const bool topSettling = mTopDrawerDragger->continueSettling(true /* deferCallbacks */);
    const bool bottomSettling = mBottomDrawerDragger->continueSettling(true /* deferCallbacks */);
    if (topSettling || bottomSettling) {
        postInvalidateOnAnimation();
    }
}

void WearableDrawerLayout::addView(View* child, int index, ViewGroup::LayoutParams* params) {
    FrameLayout::addView(child, index, params);

    WearableDrawerView* drawerChild = dynamic_cast<WearableDrawerView*>(child);
    if (drawerChild == nullptr) {
        return;
    }

    WearableDrawerController* controller = new WearableDrawerController(this, drawerChild);
    mDrawerControllers.push_back(controller);
    drawerChild->setDrawerController(controller);

    FrameLayout::LayoutParams* drawerParams = static_cast<FrameLayout::LayoutParams*>(params);
    int childGravity = drawerParams->gravity;
    // Check for preferential gravity if no gravity is set in the layout.
    if (childGravity == Gravity::NO_GRAVITY || childGravity == GRAVITY_UNDEFINED) {
        drawerParams->gravity = drawerChild->preferGravity();
        childGravity = drawerChild->preferGravity();
        drawerChild->setLayoutParams(params);
    }

    WearableDrawerView* drawerView = nullptr;
    if (childGravity == Gravity::TOP) {
        mTopDrawerView = drawerChild;
        drawerView = mTopDrawerView;
    } else if (childGravity == Gravity::BOTTOM) {
        mBottomDrawerView = drawerChild;
        drawerView = mBottomDrawerView;
    } else {
        drawerView = nullptr;
    }

    if (drawerView != nullptr) {
        drawerView->addOnLayoutChangeListener(mOnLayoutChangeListener);
    }
}

void WearableDrawerLayout::onLayoutChange(View& v, int left, int top, int right, int bottom,
        int oldLeft, int oldTop, int oldRight, int oldBottom) {
    if (&v == mTopDrawerView) {
        // Layout the top drawer base on the openedPercent. It is initially hidden.
        const float openedPercent = mTopDrawerView->getOpenedPercent();
        const int height = v.getHeight();
        const int childTop = -height + (int) (height * openedPercent);
        // CDROID View::layout takes (left, top, width, height); upstream passes
        // (left, childTop, right, childTop + height) (line 588). The same-frame
        // guard: upstream converges because View::layout skips the whole
        // changed||LAYOUT_REQUIRED block on an unchanged frame; CDROID's flag
        // can be re-armed mid-pass, so the same-value relay must not re-enter
        // layout or the listener recurses to stack overflow.
        if (v.getTop() != childTop) {
            v.layout(v.getLeft(), childTop, v.getRight() - v.getLeft(), height);
        }
    } else if (&v == mBottomDrawerView) {
        // Layout the bottom drawer base on the openedPercent. It is initially hidden.
        const float openedPercent = mBottomDrawerView->getOpenedPercent();
        const int height = v.getHeight();
        const int childTop = (int) (getHeight() - height * openedPercent);
        if (v.getTop() != childTop) {
            v.layout(v.getLeft(), childTop, v.getRight() - v.getLeft(), height);
        }
    }
}

void WearableDrawerLayout::setDrawerStateCallback(DrawerStateCallback* callback) {
    mDrawerStateCallback = callback;
}

void WearableDrawerLayout::onLayout(bool changed, int left, int top, int width, int height) {
    FrameLayout::onLayout(changed, left, top, width, height);
    if (mShouldPeekBottomDrawerAfterLayout
            || mShouldPeekTopDrawerAfterLayout
            || mShouldOpenTopDrawerAfterLayout
            || mShouldOpenBottomDrawerAfterLayout) {
        // Upstream adds a fresh one-shot OnGlobalLayoutListener per pass (lines 612-634);
        // this member listener removes itself when it fires, so re-registering it here
        // cannot queue duplicates.
        getViewTreeObserver()->removeOnGlobalLayoutListener(mGlobalLayoutListener);
        getViewTreeObserver()->addOnGlobalLayoutListener(mGlobalLayoutListener);
    }
}

void WearableDrawerLayout::onFlingComplete(View& view) {
    const bool canTopPeek = mTopDrawerView != nullptr && mTopDrawerView->isAutoPeekEnabled();
    const bool canBottomPeek = mBottomDrawerView != nullptr && mBottomDrawerView->isAutoPeekEnabled();
    const bool canScrollUp = view.canScrollVertically(UP);
    const bool canScrollDown = view.canScrollVertically(DOWN);

    if (!canScrollUp && !canScrollDown) {
        // The inner view isn't vertically scrollable, so this fling completion cannot have been
        // fired from a vertical scroll. To prevent the peeks being shown after a horizontal
        // scroll, bail out here.
        return;
    }

    if (canTopPeek && !canScrollUp && !mTopDrawerView->isPeeking()) {
        peekDrawer(Gravity::TOP);
    }
    if (canBottomPeek && (!canScrollUp || !canScrollDown) && !mBottomDrawerView->isPeeking()) {
        peekDrawer(Gravity::BOTTOM);
    }
}

int WearableDrawerLayout::getNestedScrollAxes() {
    return mNestedScrollingParentHelper->getNestedScrollAxes();
}

bool WearableDrawerLayout::onNestedFling(View* target, float velocityX, float velocityY,
        bool consumed) {
    return false;
}

bool WearableDrawerLayout::onNestedPreFling(View* target, float velocityX, float velocityY) {
    maybeUpdateScrollingContentView(target);
    mLastScrollWasFling = true;

    if (target == mScrollingContentView) {
        FlingWatcherFactory::FlingWatcher* flingWatcher = mFlingWatcher.getFor(*mScrollingContentView);
        if (flingWatcher != nullptr) {
            flingWatcher->watch();
        }
    }
    // We do not want to intercept the child from receiving the fling, so return false.
    return false;
}

void WearableDrawerLayout::onNestedPreScroll(View* target, int dx, int dy, int* consumed) {
    maybeUpdateScrollingContentView(target);
}

void WearableDrawerLayout::onNestedScroll(View* target, int dxConsumed, int dyConsumed,
        int dxUnconsumed, int dyUnconsumed) {

    const bool scrolledUp = dyConsumed < 0;
    const bool scrolledDown = dyConsumed > 0;
    const bool overScrolledUp = dyUnconsumed < 0;
    const bool overScrolledDown = dyUnconsumed > 0;

    // When the top drawer is open, we need to track whether it can be closed.
    if (mTopDrawerView != nullptr && mTopDrawerView->isOpened()) {
        // When the top drawer is overscrolled down or cannot scroll down, we consider it to be
        // at the bottom of its content, so it can be closed.
        mCanTopDrawerBeClosed =
                overScrolledDown || !mTopDrawerView->getDrawerContent()
                        ->canScrollVertically(DOWN);
        // If the last scroll was a fling and the drawer can be closed, pass along the last
        // touch event to start closing the drawer. See the docs on mLastScrollWasFling.
        // (The possibly-null stash is null-checked here; upstream's onTouchEvent(null)
        // no-ops at line 518.)
        if (mCanTopDrawerBeClosed && mLastScrollWasFling
                && mDrawerOpenLastInterceptedTouchEvent != nullptr) {
            onTouchEvent(*mDrawerOpenLastInterceptedTouchEvent);
        }
        mLastScrollWasFling = false;
        return;
    }

    // When the bottom drawer is open, we need to track whether it can be closed.
    if (mBottomDrawerView != nullptr && mBottomDrawerView->isOpened()) {
        // When the bottom drawer is scrolled to the top of its content, it can be closed.
        mCanBottomDrawerBeClosed = overScrolledUp;
        // If the last scroll was a fling and the drawer can be closed, pass along the last
        // touch event to start closing the drawer. See the docs on mLastScrollWasFling.
        if (mCanBottomDrawerBeClosed && mLastScrollWasFling
                && mDrawerOpenLastInterceptedTouchEvent != nullptr) {
            onTouchEvent(*mDrawerOpenLastInterceptedTouchEvent);
        }
        mLastScrollWasFling = false;
        return;
    }

    mLastScrollWasFling = false;

    // The following code assumes that neither drawer is open.

    // The bottom and top drawer are not open. Look at the scroll events to figure out whether
    // a drawer should peek, close it's peek, or do nothing.
    const bool canTopAutoPeek = mTopDrawerView != nullptr && mTopDrawerView->isAutoPeekEnabled();
    const bool canBottomAutoPeek =
            mBottomDrawerView != nullptr && mBottomDrawerView->isAutoPeekEnabled();
    const bool isTopDrawerPeeking = mTopDrawerView != nullptr && mTopDrawerView->isPeeking();
    const bool isBottomDrawerPeeking = mBottomDrawerView != nullptr && mBottomDrawerView->isPeeking();
    bool scrolledDownPastSlop = false;
    const bool shouldPeekOnScrollDown =
            mBottomDrawerView != nullptr && mBottomDrawerView->isPeekOnScrollDownEnabled();
    if (scrolledDown) {
        mCurrentNestedScrollSlopTracker += dyConsumed;
        scrolledDownPastSlop = mCurrentNestedScrollSlopTracker > mNestedScrollSlopPx;
    }

    if (canTopAutoPeek) {
        if (overScrolledUp && !isTopDrawerPeeking) {
            peekDrawer(Gravity::TOP);
        } else if (scrolledDown && isTopDrawerPeeking && !isClosingPeek(mTopDrawerView)) {
            closeDrawer(Gravity::TOP);
        }
    }

    if (canBottomAutoPeek) {
        if ((overScrolledDown || overScrolledUp) && !isBottomDrawerPeeking) {
            peekDrawer(Gravity::BOTTOM);
        } else if (shouldPeekOnScrollDown && scrolledDownPastSlop && !isBottomDrawerPeeking) {
            peekDrawer(Gravity::BOTTOM);
        } else if ((scrolledUp || (!shouldPeekOnScrollDown && scrolledDown))
                && isBottomDrawerPeeking
                && !isClosingPeek(mBottomDrawerView)) {
            closeDrawer(mBottomDrawerView);
        }
    }
}

void WearableDrawerLayout::maybePeekDrawer(WearableDrawerView* drawerView) {
    if (drawerView == nullptr) {
        return;
    }
    View* peekView = drawerView->getPeekContainer();
    if (peekView == nullptr) {
        return;
    }

    View* drawerContent = drawerView->getDrawerContent();
    const int layoutGravity =
            static_cast<FrameLayout::LayoutParams*>(drawerView->getLayoutParams())->gravity;
    const int gravity =
            layoutGravity == Gravity::NO_GRAVITY ? drawerView->preferGravity() : layoutGravity;

    drawerView->setIsPeeking(true);
    peekView->setAlpha(1);
    peekView->setScaleX(1);
    peekView->setScaleY(1);
    peekView->setVisibility(View::VISIBLE);
    if (drawerContent != nullptr) {
        drawerContent->setAlpha(0);
        drawerContent->setVisibility(View::GONE);
    }

    if (gravity == Gravity::BOTTOM) {
        mBottomDrawerDragger->smoothSlideViewTo(
                drawerView, 0 /* finalLeft */, getHeight() - peekView->getHeight());
    } else if (gravity == Gravity::TOP) {
        mTopDrawerDragger->smoothSlideViewTo(drawerView, 0 /* finalLeft */,
                -(drawerView->getHeight() - peekView->getHeight()));
        if (!mIsAccessibilityEnabled) {
            // Don't automatically close the top drawer when in accessibility mode.
            closeDrawerDelayed(gravity, PEEK_AUTO_CLOSE_DELAY_MS);
        }
    }

    invalidate();
}

void WearableDrawerLayout::openDrawerWithoutAnimation(WearableDrawerView* drawer) {
    if (drawer == nullptr) {
        return;
    }

    int offset;
    if (drawer == mTopDrawerView) {
        offset = mTopDrawerView->getHeight();
    } else if (drawer == mBottomDrawerView) {
        offset = -mBottomDrawerView->getHeight();
    } else {
        LOGW("openDrawer(View) should be passed in the top or bottom drawer");
        return;
    }

    drawer->offsetTopAndBottom(offset);
    drawer->setOpenedPercent(1.f);
    drawer->onDrawerOpened();
    if (mDrawerStateCallback != nullptr) {
        mDrawerStateCallback->onDrawerOpened(*this, *drawer);
    }
    showDrawerContentMaybeAnimate(*drawer);
    invalidate();
}

WearableDrawerView* WearableDrawerLayout::findDrawerWithGravity(int gravity) const {
    switch (gravity) {
    case Gravity::TOP:
        return mTopDrawerView;
    case Gravity::BOTTOM:
        return mBottomDrawerView;
    default:
        LOGW("Invalid drawer gravity: %d", gravity);
        return nullptr;
    }
}

void WearableDrawerLayout::maybeUpdateScrollingContentView(View* view) {
    if (view != mScrollingContentView && !isDrawerOrChildOfDrawer(view)) {
        mScrollingContentView = view;
    }
}

bool WearableDrawerLayout::isDrawerOrChildOfDrawer(View* view) const {
    while (view != nullptr && view != (View*) this) {
        if (dynamic_cast<WearableDrawerView*>(view) != nullptr) {
            return true;
        }

        view = view->getParent();
    }

    return false;
}

bool WearableDrawerLayout::isClosingPeek(WearableDrawerView* drawerView) const {
    return drawerView != nullptr && drawerView->getDrawerState() == WearableDrawerView::STATE_SETTLING;
}

void WearableDrawerLayout::onNestedScrollAccepted(View* child, View* target, int axes) {
    mNestedScrollingParentHelper->onNestedScrollAccepted(child, target, axes);
}

bool WearableDrawerLayout::onStartNestedScroll(View* child, View* target, int axes) {
    mCurrentNestedScrollSlopTracker = 0;
    return true;
}

void WearableDrawerLayout::onStopNestedScroll(View* target) {
    mNestedScrollingParentHelper->onStopNestedScroll(target);
}

bool WearableDrawerLayout::canDrawerContentScrollVertically(
        WearableDrawerView* drawerView, int direction) const {
    if (drawerView == nullptr) {
        return false;
    }

    View* drawerContent = drawerView->getDrawerContent();
    if (drawerContent == nullptr) {
        return false;
    }

    return drawerContent->canScrollVertically(direction);
}

void WearableDrawerLayout::DrawerStateCallback::onDrawerOpened(
        WearableDrawerLayout& layout, WearableDrawerView& drawerView) {
}

void WearableDrawerLayout::DrawerStateCallback::onDrawerClosed(
        WearableDrawerLayout& layout, WearableDrawerView& drawerView) {
}

void WearableDrawerLayout::DrawerStateCallback::onDrawerStateChanged(
        WearableDrawerLayout& layout, int newState) {
}

void WearableDrawerLayout::allowAccessibilityFocusOnAllChildren() {
    if (!mIsAccessibilityEnabled) {
        return;
    }

    for (int i = 0; i < getChildCount(); i++) {
        getChildAt(i)->setImportantForAccessibility(View::IMPORTANT_FOR_ACCESSIBILITY_YES);
    }
}

void WearableDrawerLayout::allowAccessibilityFocusOnOnly(WearableDrawerView* drawer) {
    if (!mIsAccessibilityEnabled) {
        return;
    }

    for (int i = 0; i < getChildCount(); i++) {
        View* child = getChildAt(i);
        if (child != drawer) {
            child->setImportantForAccessibility(
                    View::IMPORTANT_FOR_ACCESSIBILITY_NO_HIDE_DESCENDANTS);
        }
    }
}

WearableDrawerLayout::DrawerDraggerCallback::DrawerDraggerCallback(
        WearableDrawerLayout* drawerLayout)
    : mDrawerLayout(drawerLayout) {
}

bool WearableDrawerLayout::DrawerDraggerCallback::tryCaptureView(View& child, int pointerId) {
    WearableDrawerView* drawerView = getDrawerView();
    // Returns true if the dragger is dragging the drawer.
    return &child == drawerView && !drawerView->isLocked()
            && drawerView->getDrawerContent() != nullptr;
}

int WearableDrawerLayout::DrawerDraggerCallback::getViewVerticalDragRange(View& child) {
    // Defines the vertical drag range of the drawer.
    return &child == getDrawerView() ? child.getHeight() : 0;
}

void WearableDrawerLayout::DrawerDraggerCallback::onViewCaptured(
        View& capturedChild, int activePointerId) {
    // tryCaptureView only ever accepts the drawer view (lines 975-980), so this cast is safe.
    showDrawerContentMaybeAnimate(static_cast<WearableDrawerView&>(capturedChild));
}

void WearableDrawerLayout::DrawerDraggerCallback::onViewDragStateChanged(int state) {
    WearableDrawerView* drawerView = getDrawerView();
    switch (state) {
    case ViewDragHelper::STATE_IDLE: {
        bool openedOrClosed = false;
        if (drawerView->isOpened()) {
            openedOrClosed = true;
            drawerView->onDrawerOpened();
            mDrawerLayout->allowAccessibilityFocusOnOnly(drawerView);
            if (mDrawerLayout->mDrawerStateCallback != nullptr) {
                mDrawerLayout->mDrawerStateCallback->onDrawerOpened(*mDrawerLayout, *drawerView);
            }

            // Drawers can be closed if a drag to close them will not cause a scroll.
            mDrawerLayout->mCanTopDrawerBeClosed = !mDrawerLayout->canDrawerContentScrollVertically(
                    mDrawerLayout->mTopDrawerView, DOWN);
            mDrawerLayout->mCanBottomDrawerBeClosed =
                    !mDrawerLayout->canDrawerContentScrollVertically(
                            mDrawerLayout->mBottomDrawerView, UP);
        } else if (drawerView->isClosed()) {
            openedOrClosed = true;
            drawerView->onDrawerClosed();
            mDrawerLayout->allowAccessibilityFocusOnAllChildren();
            if (mDrawerLayout->mDrawerStateCallback != nullptr) {
                mDrawerLayout->mDrawerStateCallback->onDrawerClosed(*mDrawerLayout, *drawerView);
            }
        } else { // drawerView is peeking
            mDrawerLayout->allowAccessibilityFocusOnAllChildren();
        }

        // If the drawer is fully opened or closed, change it to non-peeking mode.
        if (openedOrClosed && drawerView->isPeeking()) {
            drawerView->setIsPeeking(false);
            drawerView->getPeekContainer()->setVisibility(View::INVISIBLE);
        }
        break;
    }
    default: // fall out
        ;
    }

    if (drawerView->getDrawerState() != state) {
        drawerView->setDrawerState(state);
        drawerView->onDrawerStateChanged(state);
        if (mDrawerLayout->mDrawerStateCallback != nullptr) {
            mDrawerLayout->mDrawerStateCallback->onDrawerStateChanged(*mDrawerLayout, state);
        }
    }
}

WearableDrawerLayout::TopDrawerDraggerCallback::TopDrawerDraggerCallback(
        WearableDrawerLayout* drawerLayout)
    : DrawerDraggerCallback(drawerLayout) {
}

int WearableDrawerLayout::TopDrawerDraggerCallback::clampViewPositionVertical(
        View& child, int top, int dy) {
    if (mDrawerLayout->mTopDrawerView == &child) {
        const int peekHeight = mDrawerLayout->mTopDrawerView->getPeekContainer()->getHeight();
        // The top drawer can be dragged vertically from peekHeight - height to 0.
        return std::max(peekHeight - child.getHeight(), std::min(top, 0));
    }
    return 0;
}

void WearableDrawerLayout::TopDrawerDraggerCallback::onEdgeDragStarted(
        int edgeFlags, int pointerId) {
    if (mDrawerLayout->mTopDrawerView != nullptr
            && edgeFlags == ViewDragHelper::EDGE_TOP
            && !mDrawerLayout->mTopDrawerView->isLocked()
            && (mDrawerLayout->mBottomDrawerView == nullptr
                    || !mDrawerLayout->mBottomDrawerView->isOpened())
            && mDrawerLayout->mTopDrawerView->getDrawerContent() != nullptr) {

        const bool atTop = mDrawerLayout->mScrollingContentView == nullptr
                || !mDrawerLayout->mScrollingContentView->canScrollVertically(UP);
        if (!mDrawerLayout->mTopDrawerView->isOpenOnlyAtTopEnabled() || atTop) {
            mDrawerLayout->mTopDrawerDragger->captureChildView(
                    mDrawerLayout->mTopDrawerView, pointerId);
        }
    }
}

void WearableDrawerLayout::TopDrawerDraggerCallback::onViewReleased(
        View& releasedChild, float xvel, float yvel) {
    if (&releasedChild == mDrawerLayout->mTopDrawerView) {
        // Settle to final position. Either swipe open or close.
        const float openedPercent = mDrawerLayout->mTopDrawerView->getOpenedPercent();

        int finalTop;
        if (yvel > 0 || (yvel == 0 && openedPercent > OPENED_PERCENT_THRESHOLD)) {
            // Drawer was being flung open or drawer is mostly open, so finish opening.
            finalTop = 0;
        } else {
            // Drawer animates to its peek state and fully closes after a delay.
            animatePeekVisibleAfterBeingClosed(*mDrawerLayout->mTopDrawerView);
            finalTop = mDrawerLayout->mTopDrawerView->getPeekContainer()->getHeight()
                    - releasedChild.getHeight();
            if (mDrawerLayout->mTopDrawerView->isAutoPeekEnabled()) {
                mDrawerLayout->closeDrawerDelayed(Gravity::TOP, PEEK_AUTO_CLOSE_DELAY_MS);
            }
        }

        mDrawerLayout->mTopDrawerDragger->settleCapturedViewAt(0 /* finalLeft */, finalTop);
        mDrawerLayout->invalidate();
    }
}

void WearableDrawerLayout::TopDrawerDraggerCallback::onViewPositionChanged(
        View& changedView, int left, int top, int dx, int dy) {
    if (&changedView == mDrawerLayout->mTopDrawerView) {
        // Compute the offset and invalidate will move the drawer during layout.
        const int height = changedView.getHeight();
        mDrawerLayout->mTopDrawerView->setOpenedPercent((float) (top + height) / height);
        mDrawerLayout->invalidate();
    }
}

WearableDrawerView* WearableDrawerLayout::TopDrawerDraggerCallback::getDrawerView() {
    return mDrawerLayout->mTopDrawerView;
}

WearableDrawerLayout::BottomDrawerDraggerCallback::BottomDrawerDraggerCallback(
        WearableDrawerLayout* drawerLayout)
    : DrawerDraggerCallback(drawerLayout) {
}

int WearableDrawerLayout::BottomDrawerDraggerCallback::clampViewPositionVertical(
        View& child, int top, int dy) {
    if (mDrawerLayout->mBottomDrawerView == &child) {
        // The bottom drawer can be dragged vertically from (parentHeight - height) to
        // (parentHeight - peekHeight).
        const int parentHeight = mDrawerLayout->getHeight();
        const int peekHeight = mDrawerLayout->mBottomDrawerView->getPeekContainer()->getHeight();
        return std::max(parentHeight - child.getHeight(),
                std::min(top, parentHeight - peekHeight));
    }
    return 0;
}

void WearableDrawerLayout::BottomDrawerDraggerCallback::onEdgeDragStarted(
        int edgeFlags, int pointerId) {
    if (mDrawerLayout->mBottomDrawerView != nullptr
            && edgeFlags == ViewDragHelper::EDGE_BOTTOM
            && !mDrawerLayout->mBottomDrawerView->isLocked()
            && (mDrawerLayout->mTopDrawerView == nullptr
                    || !mDrawerLayout->mTopDrawerView->isOpened())
            && mDrawerLayout->mBottomDrawerView->getDrawerContent() != nullptr) {
        // Tells the dragger which view to start dragging.
        mDrawerLayout->mBottomDrawerDragger->captureChildView(
                mDrawerLayout->mBottomDrawerView, pointerId);
    }
}

void WearableDrawerLayout::BottomDrawerDraggerCallback::onViewReleased(
        View& releasedChild, float xvel, float yvel) {
    if (&releasedChild == mDrawerLayout->mBottomDrawerView) {
        // Settle to final position. Either swipe open or close.
        const int parentHeight = mDrawerLayout->getHeight();
        const float openedPercent = mDrawerLayout->mBottomDrawerView->getOpenedPercent();
        int finalTop;
        if (yvel < 0 || (yvel == 0 && openedPercent > OPENED_PERCENT_THRESHOLD)) {
            // Drawer was being flung open or drawer is mostly open, so finish opening it.
            finalTop = parentHeight - releasedChild.getHeight();
        } else {
            // Drawer should be closed to its peek state.
            animatePeekVisibleAfterBeingClosed(*mDrawerLayout->mBottomDrawerView);
            finalTop = mDrawerLayout->getHeight()
                    - mDrawerLayout->mBottomDrawerView->getPeekContainer()->getHeight();
        }
        mDrawerLayout->mBottomDrawerDragger->settleCapturedViewAt(0 /* finalLeft */, finalTop);
        mDrawerLayout->invalidate();
    }
}

void WearableDrawerLayout::BottomDrawerDraggerCallback::onViewPositionChanged(
        View& changedView, int left, int top, int dx, int dy) {
    if (&changedView == mDrawerLayout->mBottomDrawerView) {
        // Compute the offset and invalidate will move the drawer during layout.
        const int height = changedView.getHeight();
        const int parentHeight = mDrawerLayout->getHeight();

        mDrawerLayout->mBottomDrawerView->setOpenedPercent((float) (parentHeight - top) / height);
        mDrawerLayout->invalidate();
    }
}

WearableDrawerView* WearableDrawerLayout::BottomDrawerDraggerCallback::getDrawerView() {
    return mDrawerLayout->mBottomDrawerView;
}

void WearableDrawerLayout::closePeek(int gravity) {
    // Upstream ClosePeekRunnable::run (lines 1203-1210).
    WearableDrawerView* drawer = findDrawerWithGravity(gravity);
    if (drawer != nullptr
            && !drawer->isOpened()
            && drawer->getDrawerState() == WearableDrawerView::STATE_IDLE) {
        closeDrawer(gravity);
    }
}

}/*endof namespace*/
