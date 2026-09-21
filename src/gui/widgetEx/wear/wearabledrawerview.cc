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
#include <widgetEx/wear/wearabledrawerview.h>
#include <view/motionevent.h>
#include <widgetEx/wear/wearabledrawercontroller.h>
#include <widgetEx/widgetex_styleable.h>
#include <view/layoutinflater.h>
#include <widget/imageview.h>

namespace cdroid{
using namespace cdroid::internal;

// androidx.wear.widget.drawer.WearableDrawerView.java (lines 91-511)
DECLARE_WIDGET2(WearableDrawerView, "androidx.wear.widget.drawer.WearableDrawerView");

WearableDrawerView::WearableDrawerView(Context* context)
    : WearableDrawerView(context, nullptr) {
}

WearableDrawerView::WearableDrawerView(Context* context, const AttributeSet* attrs)
    : WearableDrawerView(context, attrs, 0) {
}

WearableDrawerView::WearableDrawerView(Context* context, const AttributeSet* attrs, int defStyleAttr)
    : WearableDrawerView(context, attrs, defStyleAttr, 0) {
}

WearableDrawerView::WearableDrawerView(Context* context, const AttributeSet* attrs,
        int defStyleAttr, int defStyleRes)
    : FrameLayout(context, attrs, defStyleAttr, defStyleRes) {
    LayoutInflater::from(context)->inflate(
            (int)internal::R::layout::ws_wearable_drawer_view, this, true);

    setClickable(true);
    setElevation(context->getDimension((int)internal::R::dimen::ws_wearable_drawer_view_elevation));

    mPeekContainer = (ViewGroup*) findViewById((int)internal::R::id::ws_drawer_view_peek_container);
    mPeekIcon = (ImageView*) findViewById((int)internal::R::id::ws_drawer_view_peek_icon);

    mPeekContainer->setOnClickListener([this](View& v) {
        onPeekContainerClicked(v);
    });

    parseAttributes(*context, attrs, defStyleAttr);
}

Drawable* WearableDrawerView::getDrawable(Context& context, const TypedArray& typedArray,
        size_t index) {
    Drawable* background;
    const uint32_t backgroundResId = typedArray.getResourceId(index, 0);
    if (backgroundResId == 0) {
        background = typedArray.getDrawable(index);
    } else {
        background = context.getDrawable((int) backgroundResId);
    }
    return background;
}

void WearableDrawerView::onFinishInflate() {
    FrameLayout::onFinishInflate();

    // Drawer content is added after the peek view, so we need to bring the peek view
    // to the front so it shows on top of the content.
    mPeekContainer->bringToFront();
}

void WearableDrawerView::onPeekContainerClicked(View& v) {
    if (mController != nullptr) { // Java throws NPE when detached; keep unattached clicks inert
        mController->openDrawer();
    }
}

void WearableDrawerView::onAttachedToWindow() {
    FrameLayout::onAttachedToWindow();

    // The peek view has a layout gravity of bottom for the top drawer, and a layout gravity
    // of top for the bottom drawer. This is required so that the peek view shows. On the top
    // drawer, the bottom peeks from the top, and on the bottom drawer, the top peeks.
    // LayoutParams are not guaranteed to return a non-null value until a child is attached to
    // the window.
    FrameLayout::LayoutParams* peekParams =
            (FrameLayout::LayoutParams*) mPeekContainer->getLayoutParams();
    if (!Gravity::isVertical(peekParams->gravity)) {
        const bool isTopDrawer = ((((FrameLayout::LayoutParams*) getLayoutParams())->gravity)
                & Gravity::VERTICAL_GRAVITY_MASK) == Gravity::TOP;
        if (isTopDrawer) {
            peekParams->gravity = Gravity::BOTTOM;
            mPeekIcon->setImageResource((int)internal::R::drawable::ws_ic_more_horiz_24dp_wht);
        } else {
            peekParams->gravity = Gravity::TOP;
            mPeekIcon->setImageResource((int)internal::R::drawable::ws_ic_more_vert_24dp_wht);
        }
        mPeekContainer->setLayoutParams(peekParams);
    }
}

void WearableDrawerView::addView(View* child, int index, ViewGroup::LayoutParams* params) {
    const int childId = child->getId();
    if (childId != 0) {
        if (childId == mPeekResId) {
            setPeekContent(child, index, params);
            return;
        }
        if (childId == mContentResId && !setDrawerContentWithoutAdding(child)) {
            return;
        }
    }

    FrameLayout::addView(child, index, params);
}

int WearableDrawerView::preferGravity() const {
    return Gravity::NO_GRAVITY;
}

ViewGroup* WearableDrawerView::getPeekContainer() {
    return mPeekContainer;
}

void WearableDrawerView::setDrawerController(WearableDrawerController* controller) {
    mController = controller;
}

View* WearableDrawerView::getDrawerContent() {
    return mContent;
}

void WearableDrawerView::setDrawerContent(View* content) {
    if (setDrawerContentWithoutAdding(content)) {
        addView(content);
    }
}

void WearableDrawerView::setPeekContent(View* content) {
    ViewGroup::LayoutParams* layoutParams = content->getLayoutParams();
    setPeekContent(content, -1 /* index */,
            layoutParams != nullptr ? layoutParams : generateDefaultLayoutParams());
}

void WearableDrawerView::onDrawerOpened() {
}

void WearableDrawerView::onDrawerClosed() {
}

void WearableDrawerView::onDrawerStateChanged(int state) {
}

void WearableDrawerView::setOpenOnlyAtTopEnabled(bool openOnlyAtTop) {
    mOpenOnlyAtTop = openOnlyAtTop;
}

bool WearableDrawerView::isOpenOnlyAtTopEnabled() const {
    return mOpenOnlyAtTop;
}

void WearableDrawerView::setPeekOnScrollDownEnabled(bool peekOnScrollDown) {
    mPeekOnScrollDown = peekOnScrollDown;
}

bool WearableDrawerView::isPeekOnScrollDownEnabled() const {
    return mPeekOnScrollDown;
}

void WearableDrawerView::setLockedWhenClosed(bool locked) {
    mLockWhenClosed = locked;
}

bool WearableDrawerView::isLockedWhenClosed() const {
    return mLockWhenClosed;
}

int WearableDrawerView::getDrawerState() const {
    return mDrawerState;
}

void WearableDrawerView::setDrawerState(int drawerState) {
    mDrawerState = drawerState;
}

bool WearableDrawerView::isPeeking() const {
    return mIsPeeking;
}

bool WearableDrawerView::isAutoPeekEnabled() const {
    return mCanAutoPeek && !mIsLocked;
}

void WearableDrawerView::setIsAutoPeekEnabled(bool canAutoPeek) {
    mCanAutoPeek = canAutoPeek;
}

bool WearableDrawerView::isLocked() const {
    return mIsLocked || (isLockedWhenClosed() && mOpenedPercent <= 0);
}

void WearableDrawerView::setIsLocked(bool locked) {
    mIsLocked = locked;
}

bool WearableDrawerView::isOpened() const {
    return mOpenedPercent == 1;
}

bool WearableDrawerView::isClosed() const {
    return mOpenedPercent == 0;
}

WearableDrawerController* WearableDrawerView::getController() {
    return mController;
}

void WearableDrawerView::setIsPeeking(bool isPeeking) {
    mIsPeeking = isPeeking;
}

float WearableDrawerView::getOpenedPercent() const {
    return mOpenedPercent;
}

void WearableDrawerView::setOpenedPercent(float openedPercent) {
    mOpenedPercent = openedPercent;
}

void WearableDrawerView::parseAttributes(Context& context, const AttributeSet* attrs,
        int defStyleAttr) {
    if (attrs == nullptr) {
        return;
    }

    auto typedArray = context.obtainStyledAttributes(attrs, internal::R::styleable::WearableDrawerView,
            defStyleAttr, (int) internal::R::style::Widget_Wear_WearableDrawerView);

    Drawable* background = getDrawable(context, *typedArray,
            internal::R::styleable::WearableDrawerView_background);
    const int elevation = typedArray->getDimensionPixelSize(
            internal::R::styleable::WearableDrawerView_elevation, 0);
    setBackground(background);
    setElevation(elevation);

    mContentResId = (int) typedArray->getResourceId(
            internal::R::styleable::WearableDrawerView_drawerContent, 0);
    mPeekResId = (int) typedArray->getResourceId(
            internal::R::styleable::WearableDrawerView_peekView, 0);
    mCanAutoPeek = typedArray->getBoolean(
            internal::R::styleable::WearableDrawerView_enableAutoPeek, mCanAutoPeek);
}

void WearableDrawerView::setPeekContent(View* content, int index, ViewGroup::LayoutParams* params) {
    if (content == nullptr) {
        return;
    }
    if (mPeekContainer->getChildCount() > 0) {
        mPeekContainer->removeAllViews();
    }
    mPeekContainer->addView(content, index, params);
}

bool WearableDrawerView::setDrawerContentWithoutAdding(View* content) {
    if (content == mContent) {
        return false;
    }
    if (mContent != nullptr) {
        removeView(mContent);
    }

    mContent = content;
    return mContent != nullptr;
}

}/*endof namespace*/
