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
*/
#include <widgetEx/wear/wearablenavigationdrawerview.h>
#include <widgetEx/wear/wearablenavigationdraweradapter.h>
#include <widgetEx/wear/singlepagepresenter.h>
#include <widgetEx/wear/singlepageui.h>
#include <widgetEx/wear/multipagepresenter.h>
#include <widgetEx/wear/multipageui.h>
#include <widgetEx/wear/wearabledrawercontroller.h>
#include <widgetEx/widgetex_styleable.h>
#include <view/accessibility/accessibilitymanager.h>
#include <view/gravity.h>

namespace cdroid{
using namespace cdroid::internal;

// androidx.wear.widget.drawer.WearableNavigationDrawerView.java (lines 62-292)
DECLARE_WIDGET2(WearableNavigationDrawerView, "androidx.wear.widget.drawer.WearableNavigationDrawerView");

WearableNavigationDrawerView::WearableNavigationDrawerView(Context* context)
    : WearableNavigationDrawerView(context, (AttributeSet*) nullptr) {
}

WearableNavigationDrawerView::WearableNavigationDrawerView(Context* context,
        const AttributeSet* attrs)
    : WearableNavigationDrawerView(context, attrs, 0) {
}

WearableNavigationDrawerView::WearableNavigationDrawerView(Context* context,
        const AttributeSet* attrs, int defStyleAttr)
    : WearableNavigationDrawerView(context, attrs, defStyleAttr, 0) {
}

WearableNavigationDrawerView::WearableNavigationDrawerView(Context* context,
        const AttributeSet* attrs, int defStyleAttr, int defStyleRes)
    : WearableDrawerView(context, attrs, defStyleAttr, defStyleRes),
      mIsAccessibilityEnabled(AccessibilityManager::getInstance(context).isEnabled()) {

    // Upstream's anonymous Runnable (lines 102-108); a posted close can fire
    // after the drawer was detached (no controller — Java NPEs), so it stays
    // inert then.
    mCloseDrawerRunnable = [this]() {
        WearableDrawerController* controller = getController();
        if (controller != nullptr) {
            controller->closeDrawer();
        }
    };

    // Upstream's SimpleOnGestureListener (lines 115-121); a functor member
    // replaces the anonymous subclass.
    mOnGestureListener.onSingleTapUp = [this](MotionEvent& e) {
        return mPresenter->onDrawerTapped();
    };
    mGestureDetector = new GestureDetector(getContext(), mOnGestureListener);

    int navStyle = DEFAULT_STYLE;
    if (attrs != nullptr) {
        // navigationStyle is an enum attr (singlePage=0, multiPage=1); aapt2
        // compiles the symbols to their int values in binary AXML, so the plain
        // two-arg getInt reads it (same as the TabLayout_tabMode precedent).
        auto typedArray = context->obtainStyledAttributes(attrs,
                R::styleable::WearableNavigationDrawerView, defStyleAttr, 0 /* defStyleRes */);

        navStyle = typedArray->getInt(
                R::styleable::WearableNavigationDrawerView_navigationStyle, DEFAULT_STYLE);
    }

    mNavigationStyle = navStyle;

    mPresenter = mNavigationStyle == SINGLE_PAGE
            ? (WearableNavigationDrawerPresenter*) new SinglePagePresenter(
                    new SinglePageUi(this), mIsAccessibilityEnabled)
            : new MultiPagePresenter(this, new MultiPageUi(), mIsAccessibilityEnabled);

    getPeekContainer()->setContentDescription(
            context->getString((int) R::string::ws_navigation_drawer_content_description));

    setOpenOnlyAtTopEnabled(true);
}

WearableNavigationDrawerView::~WearableNavigationDrawerView() {
    removeCallbacks(mCloseDrawerRunnable); // a pending auto-close captures this
    delete mGestureDetector;
    delete mPresenter;
}

void WearableNavigationDrawerView::setAdapter(WearableNavigationDrawerAdapter* adapter) {
    mPresenter->onNewAdapter(adapter);
}

void WearableNavigationDrawerView::addOnItemSelectedListener(const OnItemSelectedListener& listener) {
    mPresenter->onItemSelectedListenerAdded(listener);
}

void WearableNavigationDrawerView::removeOnItemSelectedListener(
        const OnItemSelectedListener& listener) {
    mPresenter->onItemSelectedListenerRemoved(listener);
}

void WearableNavigationDrawerView::setCurrentItem(int index, bool smoothScrollTo) {
    mPresenter->onSetCurrentItemRequested(index, smoothScrollTo);
}

int WearableNavigationDrawerView::getNavigationStyle() const {
    return mNavigationStyle;
}

bool WearableNavigationDrawerView::onInterceptTouchEvent(MotionEvent& ev) {
    autoCloseDrawerAfterDelay();
    // mGestureDetector is always constructed (upstream's @Nullable is vestigial).
    return mGestureDetector->onTouchEvent(ev);
}

bool WearableNavigationDrawerView::canScrollHorizontally(int direction) {
    // Prevent the window from being swiped closed while it is open by saying that it can scroll
    // horizontally.
    return isOpened();
}

void WearableNavigationDrawerView::onDrawerOpened() {
    autoCloseDrawerAfterDelay();
}

void WearableNavigationDrawerView::onDrawerClosed() {
    removeCallbacks(mCloseDrawerRunnable);
}

void WearableNavigationDrawerView::autoCloseDrawerAfterDelay() {
    if (!mIsAccessibilityEnabled) {
        removeCallbacks(mCloseDrawerRunnable);
        postDelayed(mCloseDrawerRunnable, AUTO_CLOSE_DRAWER_DELAY_MS);
    }
}

int WearableNavigationDrawerView::preferGravity() const {
    return Gravity::TOP;
}

}/*endof namespace*/
