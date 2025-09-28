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
#include <widgetEx/wear/wearablerecyclerview.h>

namespace cdroid{

DECLARE_WIDGET(WearableRecyclerView);

WearableRecyclerView::WearableRecyclerView(int w,int h)
    :RecyclerView(w,h){
    mScrollManager = new ScrollManager();
}

WearableRecyclerView::WearableRecyclerView(Context* context, const AttributeSet& attrs)
    :RecyclerView(context, attrs){
    mScrollManager = new ScrollManager();
    setHasFixedSize(true);
    // Padding is used to center the top and bottom items in the list, don't clip to padding to
    // allows the items to draw in that space.
    setClipToPadding(false);

    setCircularScrollingGestureEnabled(attrs.getBoolean("circularScrollingGestureEnabled",mCircularScrollingEnabled));
    setBezelFraction(attrs.getFloat("bezelWidth",mScrollManager->getBezelWidth()));
    setScrollDegreesPerScreen(attrs.getFloat("scrollDegreesPerScreen",mScrollManager->getScrollDegreesPerScreen()));
}

WearableRecyclerView::~WearableRecyclerView(){
}

void WearableRecyclerView::setupCenteredPadding() {
    if ((getChildCount() < 1) || !mEdgeItemsCenteringEnabled) {
        return;
    }
    // All the children in the view are the same size, as we set setHasFixedSize
    // to true, so the height of the first child is the same as all of them.
    View* child = getChildAt(0);
    const int height = child->getHeight();
    // This is enough padding to center the child view in the parent.
    const int desiredPadding = (int) ((getHeight() * 0.5f) - (height * 0.5f));

    if (getPaddingTop() != desiredPadding) {
        mOriginalPaddingTop = getPaddingTop();
        mOriginalPaddingBottom = getPaddingBottom();
        // The view is symmetric along the vertical axis, so the top and bottom
        // can be the same.
        setPadding(getPaddingLeft(), desiredPadding, getPaddingRight(), desiredPadding);

        // The focused child should be in the center, so force a scroll to it.
        View* focusedChild = getFocusedChild();
        const int focusedPosition = (focusedChild != nullptr) ? getLayoutManager()->getPosition(
                        focusedChild) : 0;
        getLayoutManager()->scrollToPosition(focusedPosition);
    }
}

void WearableRecyclerView::setupOriginalPadding() {
    if (mOriginalPaddingTop == NO_VALUE) {
        return;
    } else {
        setPadding(getPaddingLeft(), mOriginalPaddingTop, getPaddingRight(), mOriginalPaddingBottom);
    }
}

bool WearableRecyclerView::onTouchEvent(MotionEvent& event) {
    if (mCircularScrollingEnabled && mScrollManager->onTouchEvent(event)) {
        return true;
    }
    return RecyclerView::onTouchEvent(event);
}

void WearableRecyclerView::onAttachedToWindow() {
    RecyclerView::onAttachedToWindow();
    DisplayMetrics dm = getContext()->getDisplayMetrics();
    mScrollManager->setRecyclerView(this, dm.widthPixels, dm.heightPixels);
    getViewTreeObserver()->addOnPreDrawListener(mPaddingPreDrawListener);
}

void WearableRecyclerView::onDetachedFromWindow() {
    RecyclerView::onDetachedFromWindow();
    mScrollManager->clearRecyclerView();
    getViewTreeObserver()->removeOnPreDrawListener(mPaddingPreDrawListener);
}

void WearableRecyclerView::setCircularScrollingGestureEnabled(bool circularScrollingGestureEnabled) {
    mCircularScrollingEnabled = circularScrollingGestureEnabled;
}

bool WearableRecyclerView::isCircularScrollingGestureEnabled() const{
    return mCircularScrollingEnabled;
}

void WearableRecyclerView::setScrollDegreesPerScreen(float degreesPerScreen) {
    mScrollManager->setScrollDegreesPerScreen(degreesPerScreen);
}

float WearableRecyclerView::getScrollDegreesPerScreen() const{
    return mScrollManager->getScrollDegreesPerScreen();
}

void WearableRecyclerView::setBezelFraction(float fraction) {
    mScrollManager->setBezelWidth(fraction);
}

float WearableRecyclerView::getBezelFraction() const{
    return mScrollManager->getBezelWidth();
}

void WearableRecyclerView::setEdgeItemsCenteringEnabled(bool isEnabled) {
    if (0/*!getResources().getConfiguration().isScreenRound()*/) {
        mEdgeItemsCenteringEnabled = false;
        return;
    }
    mEdgeItemsCenteringEnabled = isEnabled;
    if (mEdgeItemsCenteringEnabled) {
        if (getChildCount() > 0) {
            setupCenteredPadding();
        } else {
            mCenterEdgeItemsWhenThereAreChildren = true;
        }
    } else {
        setupOriginalPadding();
        mCenterEdgeItemsWhenThereAreChildren = false;
    }
}

bool WearableRecyclerView::isEdgeItemsCenteringEnabled() const{
    return mEdgeItemsCenteringEnabled;
}
}/*endof namespace*/
