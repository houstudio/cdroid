#include <widgetEx/wear/wearablerecyclerview.h>

namespace cdroid{
WearableRecyclerView::WearableRecyclerView(Context* context, const AttributeSet& attrs)
    :RecyclerView(context, attrs){

    setHasFixedSize(true);
    // Padding is used to center the top and bottom items in the list, don't clip to padding to
    // allows the items to draw in that space.
    setClipToPadding(false);

    setCircularScrollingGestureEnabled(attrs.getBoolean("circularScrollingGestureEnabled",mCircularScrollingEnabled));
    setBezelFraction(attrs.getFloat("bezelWidth",mScrollManager->getBezelWidth()));
    setScrollDegreesPerScreen(attrs.getFloat("scrollDegreesPerScreen",mScrollManager->getScrollDegreesPerScreen()));
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
        int focusedPosition = (focusedChild != nullptr) ? getLayoutManager()->getPosition(
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
};
