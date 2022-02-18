#include <widget/gallery.h>
#include <core/soundeffect.h>
namespace cdroid{

DECLARE_WIDGET2(Gallery,"cdroid:attr/galleryStyle")

Gallery::Gallery(Context* context,const AttributeSet& attrs):AbsSpinner(context,attrs){
    int index = attrs.getGravity("gravity",-1);//com.android.internal.R.styleable.Gallery_gravity, -1);
    if (index >= 0) {
        setGravity(index);
    }

    const int animationDuration = attrs.getInt("animationDuration", -1);
    if (animationDuration > 0) {
        setAnimationDuration(animationDuration);
    }

    const int spacing = attrs.getDimensionPixelOffset("spacing", 0);
    setSpacing(spacing);

    const float unselectedAlpha = attrs.getFloat("unselectedAlpha", 0.5f);
    setUnselectedAlpha(unselectedAlpha);

    mFlingRunnable =new FlingRunnable(this);
    // We draw the selected item last (because otherwise the item to the
    // right overlaps it)
    mGroupFlags |= FLAG_USE_CHILD_DRAWING_ORDER;
    mGroupFlags |= FLAG_SUPPORT_STATIC_TRANSFORMATIONS;
}

void Gallery::onAttachedToWindow(){
   AbsSpinner::onAttachedToWindow();
}

void Gallery::setCallbackDuringFling(bool shouldCallback){
    mShouldCallbackDuringFling = shouldCallback;
}

void Gallery::setCallbackOnUnselectedItemClick(bool shouldCallback){
    mShouldCallbackOnUnselectedItemClick = shouldCallback;
}

void Gallery::setAnimationDuration(int animationDurationMillis){
    mAnimationDuration = animationDurationMillis;
}

void Gallery::setSpacing(int spacing) {
    mSpacing = spacing;
}

void Gallery::setUnselectedAlpha(float unselectedAlpha){
     mUnselectedAlpha = unselectedAlpha;
}

bool Gallery::getChildStaticTransformation(View* child, Transformation& t){
    t.clear();
    t.setAlpha(child == mSelectedChild ? 1.0f : mUnselectedAlpha);
}

int Gallery::computeHorizontalScrollExtent() {
    // Only 1 item is considered to be selected
    return 1;
}

int Gallery::computeHorizontalScrollOffset() {
    // Current scroll position is the same as the selected position
    return mSelectedPosition;
}

int Gallery::computeHorizontalScrollRange() {
    // Scroll range is the same as the item count
    return mItemCount;
}

bool Gallery::checkLayoutParams(const ViewGroup::LayoutParams* p)const {
    return dynamic_cast<const LayoutParams*>(p);
}

ViewGroup::LayoutParams* Gallery::generateLayoutParams(const ViewGroup::LayoutParams*p)const{
    return new LayoutParams(*p);
}

ViewGroup::LayoutParams* Gallery::generateLayoutParams(const AttributeSet& attrs)const {
    return new LayoutParams(getContext(), attrs);
}

ViewGroup::LayoutParams* Gallery::generateDefaultLayoutParams()const{
    /* Gallery expects Gallery.LayoutParams. */
    return new Gallery::LayoutParams(LayoutParams::WRAP_CONTENT,LayoutParams::WRAP_CONTENT);
}

void Gallery::onLayout(bool changed, int l, int t, int r, int b) {
    AbsSpinner::onLayout(changed, l, t, r, b);

    /* Remember that we are in layout to prevent more layout request from
     * being generated. */
    mInLayout = true;
    layout(0, false);
    mInLayout = false;
}

int Gallery::getChildHeight(View* child) {
    return child->getMeasuredHeight();
}

/**
 * Tracks a motion scroll. In reality, this is used to do just about any
 * movement to items (touch scroll, arrow-key scroll, set an item as selected).
 *
 * @param deltaX Change in X from the previous event.
 */
void Gallery::trackMotionScroll(int deltaX) {

    if (getChildCount() == 0) {
        return;
    }

    bool toLeft = deltaX < 0;

    int limitedDeltaX = getLimitedMotionScrollAmount(toLeft, deltaX);
    if (limitedDeltaX != deltaX) {
        // The above call returned a limited amount, so stop any scrolls/flings
        mFlingRunnable->endFling(false);
        onFinishedMovement();
    }

    offsetChildrenLeftAndRight(limitedDeltaX);

    detachOffScreenChildren(toLeft);

    if (toLeft) {
        // If moved left, there will be empty space on the right
        fillToGalleryRight();
    } else {
        // Similarly, empty space on the left
        fillToGalleryLeft();
    }

    // Clear unused views
    mRecycler->clear();

    setSelectionToCenterChild();

    View* selChild = mSelectedChild;
    if (selChild ) {
        int childLeft = selChild->getLeft();
        int childCenter = selChild->getWidth() / 2;
        int galleryCenter = getWidth() / 2;
        mSelectedCenterOffset = childLeft + childCenter - galleryCenter;
    }

    onScrollChanged(0, 0, 0, 0); // dummy values, View's implementation does not use these.

    invalidate();
}

int Gallery::getLimitedMotionScrollAmount(bool motionToLeft, int deltaX) {
    int extremeItemPosition = motionToLeft != mIsRtl ? mItemCount - 1 : 0;
    View* extremeChild = getChildAt(extremeItemPosition - mFirstPosition);

    if (extremeChild == nullptr) {
        return deltaX;
    }

    int extremeChildCenter = getCenterOfView(extremeChild);
    int galleryCenter = getCenterOfGallery();

    if (motionToLeft) {
        if (extremeChildCenter <= galleryCenter) {
            // The extreme child is past his boundary point!
            return 0;
        }
    } else {
        if (extremeChildCenter >= galleryCenter) {
            // The extreme child is past his boundary point!
            return 0;
        }
    }

    const int centerDifference = galleryCenter - extremeChildCenter;
    return motionToLeft ? std::max(centerDifference, deltaX):std::min(centerDifference, deltaX);
}

/**
 * Offset the horizontal location of all children of this view by the
 * specified number of pixels.
 *
 * @param offset the number of pixels to offset
 */
void Gallery::offsetChildrenLeftAndRight(int offset) {
    for (int i = getChildCount() - 1; i >= 0; i--) {
        getChildAt(i)->offsetLeftAndRight(offset);
    }
}

/**
 * @return The center of this Gallery.
 */
int Gallery::getCenterOfGallery() {
    return (getWidth() - mPaddingLeft - mPaddingRight) / 2 + mPaddingLeft;
}

/**
 * @return The center of the given view.
 */
int Gallery::getCenterOfView(View* view) {
    return view->getLeft() + view->getWidth() / 2;
}

/**
 * Detaches children that are off the screen (i.e.: Gallery bounds).
 *
 * @param toLeft Whether to detach children to the left of the Gallery, or
 *            to the right.
 */
void Gallery::detachOffScreenChildren(bool toLeft) {
    int numChildren = getChildCount();
    int firstPosition = mFirstPosition;
    int start = 0;
    int count = 0;

    if (toLeft) {
        int galleryLeft = mPaddingLeft;
        for (int i = 0; i < numChildren; i++) {
            int n = mIsRtl ? (numChildren - 1 - i) : i;
            View* child = getChildAt(n);
            if (child->getRight() >= galleryLeft) {
                break;
            } else {
                start = n;
                count++;
                mRecycler->put(firstPosition + n, child);
            }
        }
        if (!mIsRtl) {
            start = 0;
        }
    } else {
        int galleryRight = getWidth() - mPaddingRight;
        for (int i = numChildren - 1; i >= 0; i--) {
            int n = mIsRtl ? numChildren - 1 - i : i;
            View* child = getChildAt(n);
            if (child->getLeft() <= galleryRight) {
                break;
            } else {
                start = n;
                count++;
                mRecycler->put(firstPosition + n, child);
            }
        }
        if (mIsRtl) {
            start = 0;
        }
    }

    detachViewsFromParent(start, count);

    if (toLeft != mIsRtl) {
        mFirstPosition += count;
    }
}

/**
 * Scrolls the items so that the selected item is in its 'slot' (its center
 * is the gallery's center).
 */
void Gallery::scrollIntoSlots() {

    if (getChildCount() == 0 || mSelectedChild == nullptr) return;

    int selectedCenter = getCenterOfView(mSelectedChild);
    int targetCenter = getCenterOfGallery();

    int scrollAmount = targetCenter - selectedCenter;
    if (scrollAmount != 0) {
        mFlingRunnable->startUsingDistance(scrollAmount);
    } else {
        onFinishedMovement();
    }
}

void Gallery::onFinishedMovement() {
    if (mSuppressSelectionChanged) {
        mSuppressSelectionChanged = false;

        // We haven't been callbacking during the fling, so do it now
        AbsSpinner::selectionChanged();
    }
    mSelectedCenterOffset = 0;
    invalidate();
}


void Gallery::selectionChanged() {
    if (!mSuppressSelectionChanged) {
        AbsSpinner::selectionChanged();
    }
}

/**
 * Looks for the child that is closest to the center and sets it as the
 * selected child.
 */
void Gallery::setSelectionToCenterChild() {

    View* selView = mSelectedChild;
    if (mSelectedChild == nullptr) return;

    int galleryCenter = getCenterOfGallery();

    // Common case where the current selected position is correct
    if (selView->getLeft() <= galleryCenter && selView->getRight() >= galleryCenter) {
        return;
    }

    // TODO better search
    int closestEdgeDistance = INT_MAX;//Integer.MAX_VALUE;
    int newSelectedChildIndex = 0;
    for (int i = getChildCount() - 1; i >= 0; i--) {

        View* child = getChildAt(i);

        if (child->getLeft() <= galleryCenter && child->getRight() >=  galleryCenter) {
            // This child is in the center
            newSelectedChildIndex = i;
            break;
        }

        int childClosestEdgeDistance = std::min(abs(child->getLeft() - galleryCenter),
                abs(child->getRight() - galleryCenter));
        if (childClosestEdgeDistance < closestEdgeDistance) {
            closestEdgeDistance = childClosestEdgeDistance;
            newSelectedChildIndex = i;
        }
    }

    int newPos = mFirstPosition + newSelectedChildIndex;

    if (newPos != mSelectedPosition) {
        setSelectedPositionInt(newPos);
        setNextSelectedPositionInt(newPos);
        checkSelectionChanged();
    }
}

/**
 * Creates and positions all views for this Gallery.
 * <p>
 * We layout rarely, most of the time {@link #trackMotionScroll(int)} takes
 * care of repositioning, adding, and removing children.
 *
 * @param delta Change in the selected position. +1 means the selection is
 *            moving to the right, so views are scrolling to the left. -1
 *            means the selection is moving to the left.
 */

void Gallery::layout(int delta, bool animate) {

    mIsRtl = isLayoutRtl();

    int childrenLeft = mSpinnerPadding.left;
    int childrenWidth = mRight - mLeft - mSpinnerPadding.left - mSpinnerPadding.width;

    if (mDataChanged) {
        handleDataChanged();
    }

    // Handle an empty gallery by removing all views.
    if (mItemCount == 0) {
        resetList();
        return;
    }

    // Update to the new selected position.
    if (mNextSelectedPosition >= 0) {
        setSelectedPositionInt(mNextSelectedPosition);
    }

    // All views go in recycler while we are in layout
    recycleAllViews();

    // Clear out old views
    //removeAllViewsInLayout();
    detachAllViewsFromParent();

    /*
     * These will be used to give initial positions to views entering the
     * gallery as we scroll
     */
    mRightMost = 0;
    mLeftMost = 0;

    // Make selected view and center it

    /*
     * mFirstPosition will be decreased as we add views to the left later
     * on. The 0 for x will be offset in a couple lines down.
     */
    mFirstPosition = mSelectedPosition;
    View* sel = makeAndAddView(mSelectedPosition, 0, 0, true);

    // Put the selected child in the center
    int selectedOffset = childrenLeft + (childrenWidth / 2) - (sel->getWidth() / 2) +
            mSelectedCenterOffset;
    sel->offsetLeftAndRight(selectedOffset);

    fillToGalleryRight();
    fillToGalleryLeft();

    // Flush any cached views that did not get reused above
    mRecycler->clear();

    invalidate();
    checkSelectionChanged();

    mDataChanged = false;
    mNeedSync = false;
    setNextSelectedPositionInt(mSelectedPosition);

    updateSelectedItemMetadata();
}

void Gallery::fillToGalleryLeft() {
    if (mIsRtl) {
        fillToGalleryLeftRtl();
    } else {
        fillToGalleryLeftLtr();
    }
}

void Gallery::fillToGalleryLeftRtl() {
    int itemSpacing = mSpacing;
    int galleryLeft = mPaddingLeft;
    int numChildren = getChildCount();
    int numItems = mItemCount;

    // Set state for initial iteration
    View* prevIterationView = getChildAt(numChildren - 1);
    int curPosition;
    int curRightEdge;

    if (prevIterationView) {
        curPosition = mFirstPosition + numChildren;
        curRightEdge = prevIterationView->getLeft() - itemSpacing;
    } else {
        // No children available!
        mFirstPosition = curPosition = mItemCount - 1;
        curRightEdge = mRight - mLeft - mPaddingRight;
        mShouldStopFling = true;
    }

    while (curRightEdge > galleryLeft && curPosition < mItemCount) {
        prevIterationView = makeAndAddView(curPosition, curPosition - mSelectedPosition,
                curRightEdge, false);

        // Set state for next iteration
        curRightEdge = prevIterationView->getLeft() - itemSpacing;
        curPosition++;
    }
}

void Gallery::fillToGalleryLeftLtr() {
    int itemSpacing = mSpacing;
    int galleryLeft = mPaddingLeft;

    // Set state for initial iteration
    View* prevIterationView = getChildAt(0);
    int curPosition;
    int curRightEdge;

    if (prevIterationView) {
        curPosition = mFirstPosition - 1;
        curRightEdge = prevIterationView->getLeft() - itemSpacing;
    } else {
        // No children available!
        curPosition = 0;
        curRightEdge = mRight - mLeft - mPaddingRight;
        mShouldStopFling = true;
    }

    while (curRightEdge > galleryLeft && curPosition >= 0) {
        prevIterationView = makeAndAddView(curPosition, curPosition - mSelectedPosition,
                curRightEdge, false);

        // Remember some state
        mFirstPosition = curPosition;

        // Set state for next iteration
        curRightEdge = prevIterationView->getLeft() - itemSpacing;
        curPosition--;
    }
}

void Gallery::fillToGalleryRight() {
    if (mIsRtl) {
        fillToGalleryRightRtl();
    } else {
        fillToGalleryRightLtr();
    }
}

void Gallery::fillToGalleryRightRtl() {
    int itemSpacing = mSpacing;
    int galleryRight = mRight - mLeft - mPaddingRight;

    // Set state for initial iteration
    View* prevIterationView = getChildAt(0);
    int curPosition;
    int curLeftEdge;

    if (prevIterationView) {
        curPosition = mFirstPosition -1;
        curLeftEdge = prevIterationView->getRight() + itemSpacing;
    } else {
        curPosition = 0;
        curLeftEdge = mPaddingLeft;
        mShouldStopFling = true;
    }

    while (curLeftEdge < galleryRight && curPosition >= 0) {
        prevIterationView = makeAndAddView(curPosition, curPosition - mSelectedPosition,
                curLeftEdge, true);

        // Remember some state
        mFirstPosition = curPosition;

        // Set state for next iteration
        curLeftEdge = prevIterationView->getRight() + itemSpacing;
        curPosition--;
    }
}

void Gallery::fillToGalleryRightLtr() {
    int itemSpacing = mSpacing;
    int galleryRight = mRight - mLeft - mPaddingRight;
    int numChildren = getChildCount();
    int numItems = mItemCount;

    // Set state for initial iteration
    View* prevIterationView = getChildAt(numChildren - 1);
    int curPosition;
    int curLeftEdge;

    if (prevIterationView) {
        curPosition = mFirstPosition + numChildren;
        curLeftEdge = prevIterationView->getRight() + itemSpacing;
    } else {
        mFirstPosition = curPosition = mItemCount - 1;
        curLeftEdge = mPaddingLeft;
        mShouldStopFling = true;
    }

    while (curLeftEdge < galleryRight && curPosition < numItems) {
        prevIterationView = makeAndAddView(curPosition, curPosition - mSelectedPosition,
                curLeftEdge, true);

        // Set state for next iteration
        curLeftEdge = prevIterationView->getRight() + itemSpacing;
        curPosition++;
    }
}

/**
 * Obtain a view, either by pulling an existing view from the recycler or by
 * getting a new one from the adapter. If we are animating, make sure there
 * is enough information in the view's layout parameters to animate from the
 * old to new positions.
 *
 * @param position Position in the gallery for the view to obtain
 * @param offset Offset from the selected position
 * @param x X-coordinate indicating where this view should be placed. This
 *        will either be the left or right edge of the view, depending on
 *        the fromLeft parameter
 * @param fromLeft Are we positioning views based on the left edge? (i.e.,
 *        building from left to right)?
 * @return A view that has been added to the gallery
 */
View* Gallery::makeAndAddView(int position, int offset, int x, bool fromLeft) {

    View* child;
    if (!mDataChanged) {
        child = mRecycler->get(position);
        if (child) {
            // Can reuse an existing view
            int childLeft = child->getLeft();

            // Remember left and right edges of where views have been placed
            mRightMost= std::max(mRightMost, childLeft + child->getMeasuredWidth());
            mLeftMost = std::min(mLeftMost, childLeft);

            // Position the view
            setUpChild(child, offset, x, fromLeft);

            return child;
        }
    }

    // Nothing found in the recycler -- ask the adapter for a view
    child = mAdapter->getView(position, nullptr, this);

    // Position the view
    setUpChild(child, offset, x, fromLeft);

    return child;
}

/**
 * Helper for makeAndAddView to set the position of a view and fill out its
 * layout parameters.
 *
 * @param child The view to position
 * @param offset Offset from the selected position
 * @param x X-coordinate indicating where this view should be placed. This
 *        will either be the left or right edge of the view, depending on
 *        the fromLeft parameter
 * @param fromLeft Are we positioning views based on the left edge? (i.e.,
 *        building from left to right)?
 */
void Gallery::setUpChild(View* child, int offset, int x, bool fromLeft) {

    // Respect layout params that are already in the view. Otherwise
    // make some up...
    Gallery::LayoutParams* lp = (Gallery::LayoutParams*) child->getLayoutParams();
    if (lp == nullptr) {
        lp = (Gallery::LayoutParams*) generateDefaultLayoutParams();
    }

    addViewInLayout(child, fromLeft != mIsRtl ? -1 : 0, lp, true);

    child->setSelected(offset == 0);

    // Get measure specs
    int childHeightSpec = ViewGroup::getChildMeasureSpec(mHeightMeasureSpec,
            mSpinnerPadding.top + mSpinnerPadding.height, lp->height);
    int childWidthSpec = ViewGroup::getChildMeasureSpec(mWidthMeasureSpec,
            mSpinnerPadding.left + mSpinnerPadding.width, lp->width);

    // Measure child
    child->measure(childWidthSpec, childHeightSpec);

    int childLeft;
    int childRight;

    // Position vertically based on gravity setting
    int childTop = calculateTop(child, true);
    int childBottom = childTop + child->getMeasuredHeight();

    int width = child->getMeasuredWidth();
    if (fromLeft) {
        childLeft = x;
        childRight = childLeft + width;
    } else {
        childLeft = x - width;
        childRight = x;
    }

    child->layout(childLeft, childTop, childRight, childBottom);
}

/**
 * Figure out vertical placement based on mGravity
 *
 * @param child Child to place
 * @return Where the top of the child should be
 */
int Gallery::calculateTop(View* child, bool duringLayout) {
    int myHeight = duringLayout ? getMeasuredHeight() : getHeight();
    int childHeight = duringLayout ? child->getMeasuredHeight() : child->getHeight();

    int childTop = 0;

    switch (mGravity) {
    case Gravity::TOP:
        childTop = mSpinnerPadding.top;
        break;
    case Gravity::CENTER_VERTICAL:
        childTop = mSpinnerPadding.top + (myHeight - mSpinnerPadding.height-mSpinnerPadding.top - childHeight) / 2;
        break;
    case Gravity::BOTTOM:
        childTop = myHeight - mSpinnerPadding.height - childHeight;
        break;
    }
    return childTop;
}

bool Gallery::onTouchEvent(MotionEvent& event) {
    // Give everything to the gesture detector
    bool retValue =false;// mGestureDetector.onTouchEvent(event);
    int action = event.getAction();
    if (action == MotionEvent::ACTION_UP) {
        // Helper method for lifted finger
        onUp();
    } else if (action == MotionEvent::ACTION_CANCEL) {
        onCancel();
    }

    return retValue;
}

bool Gallery::onSingleTapUp(MotionEvent& e) {
    if (mDownTouchPosition >= 0) {
        // An item tap should make it selected, so scroll to this child.
        scrollToChild(mDownTouchPosition - mFirstPosition);

        // Also pass the click so the client knows, if it wants to.
        if (mShouldCallbackOnUnselectedItemClick || mDownTouchPosition == mSelectedPosition) {
            performItemClick(mDownTouchView, mDownTouchPosition, mAdapter->getItemId(mDownTouchPosition));
        }

        return true;
    }
    return false;
}

bool Gallery::onFling(MotionEvent& e1, MotionEvent& e2, float velocityX, float velocityY) {

    if (!mShouldCallbackDuringFling) {
        // We want to suppress selection changes
        // Remove any future code to set mSuppressSelectionChanged = false
        removeCallbacks(mDisableSuppressSelectionChangedRunnable);
        // This will get reset once we scroll into slots
        if (!mSuppressSelectionChanged) mSuppressSelectionChanged = true;
    }

    // Fling the gallery!
    mFlingRunnable->startUsingVelocity((int) -velocityX);

    return true;
}


bool Gallery::onScroll(MotionEvent& e1, MotionEvent& e2, float distanceX, float distanceY) {

    //if (localLOGV) Log.v(TAG, String.valueOf(e2.getX() - e1.getX()));

    /* Now's a good time to tell our parent to stop intercepting our events!
     * The user has moved more than the slop amount, since GestureDetector
     * ensures this before calling this method. Also, if a parent is more
     * interested in this touch's events than we are, it would have
     * intercepted them by now (for example, we can assume when a Gallery is
     * in the ListView, a vertical scroll would not end up in this method
     * since a ListView would have intercepted it by now).*/
    mParent->requestDisallowInterceptTouchEvent(true);

    // As the user scrolls, we want to callback selection changes so related-
    // info on the screen is up-to-date with the gallery's selection
    if (!mShouldCallbackDuringFling) {
        if (mIsFirstScroll) {
            /* We're not notifying the client of selection changes during
             * the fling, and this scroll could possibly be a fling. Don't
             * do selection changes until we're sure it is not a fling.*/
            if (!mSuppressSelectionChanged) mSuppressSelectionChanged = true;
            postDelayed(mDisableSuppressSelectionChangedRunnable, SCROLL_TO_FLING_UNCERTAINTY_TIMEOUT);
        }
    } else {
        if (mSuppressSelectionChanged) mSuppressSelectionChanged = false;
    }

    // Track the motion
    trackMotionScroll(-1 * (int) distanceX);
    mIsFirstScroll = false;
    return true;
}


bool Gallery::onDown(MotionEvent& e) {
    // Kill any existing fling/scroll
    mFlingRunnable->stop(false);

    // Get the item's view that was touched
    mDownTouchPosition = pointToPosition((int) e.getX(), (int) e.getY());
    if (mDownTouchPosition >= 0) {
        mDownTouchView = getChildAt(mDownTouchPosition - mFirstPosition);
        mDownTouchView->setPressed(true);
    }
    // Reset the multiple-scroll tracking state
    mIsFirstScroll = true;
    // Must return true to get matching events for this down event.
    return true;
}

/**
 * Called when a touch event's action is MotionEvent.ACTION_UP.
 */
void Gallery::onUp() {

    if (mFlingRunnable->mScroller->isFinished()) {
        scrollIntoSlots();
    }

    dispatchUnpress();
}

/**Called when a touch event's action is MotionEvent.ACTION_CANCEL.*/
void Gallery::onCancel() {
    onUp();
}


void Gallery::onLongPress(MotionEvent& e) {
    if (mDownTouchPosition < 0) {
        return;
    }
    //performHapticFeedback(HapticFeedbackConstants.LONG_PRESS);

    long id = getItemIdAtPosition(mDownTouchPosition);
    dispatchLongPress(*mDownTouchView, mDownTouchPosition, id, e.getX(), e.getY(), true);
}

// Unused methods from GestureDetector.OnGestureListener below


void Gallery::onShowPress(MotionEvent& e) {
}

// Unused methods from GestureDetector.OnGestureListener above

void Gallery::dispatchPress(View* child) {
    if (child)  child->setPressed(true);
    setPressed(true);
}

void Gallery::dispatchUnpress() {
    for (int i = getChildCount() - 1; i >= 0; i--) {
        getChildAt(i)->setPressed(false);
    }
    setPressed(false);
}


void Gallery::dispatchSetSelected(bool selected) {
    /* We don't want to pass the selected state given from its parent to its
     * children since this widget itself has a selected state to give to its
     * children.*/
}


void Gallery::dispatchSetPressed(bool pressed) {
    // Show the pressed state on the selected child
    if (mSelectedChild ) {
        mSelectedChild->setPressed(pressed);
    }
}

bool Gallery::dispatchLongPress(View& view, int position, long id, float x, float y,bool useOffsets) {
    bool handled = false;
    if (mOnItemLongClickListener) {
        handled = mOnItemLongClickListener(*this, *mDownTouchView, mDownTouchPosition, id);
    }

    if (!handled) {
        //mContextMenuInfo = new AdapterContextMenuInfo(view, position, id);
        if (useOffsets) {
            handled = AbsSpinner::showContextMenuForChild(&view, x, y);
        } else {
            handled = AbsSpinner::showContextMenuForChild(this);
        }
    }
    if (handled) {
        //performHapticFeedback(HapticFeedbackConstants.LONG_PRESS);
    }
    return handled;
}

bool Gallery::dispatchKeyEvent(KeyEvent& event) {
    // Gallery steals all key events
    return event.dispatch(this, nullptr, nullptr);
}

bool Gallery::onKeyDown(int keyCode, KeyEvent& event){
    switch (keyCode) {
    case KEY_DPAD_LEFT:
        if (moveDirection(-1)) {
            playSoundEffect(SoundEffectConstants::NAVIGATION_LEFT);
            return true;
        }
        break;
    case KEY_DPAD_RIGHT:
        if (moveDirection(1)) {
            playSoundEffect(SoundEffectConstants::NAVIGATION_RIGHT);
            return true;
        }
        break;
    case KEY_DPAD_CENTER:
    case KEY_ENTER:
        mReceivedInvokeKeyDown = true;
        // fallthrough to default handling
    }

    return AbsSpinner::onKeyDown(keyCode, event);
}

bool Gallery::onKeyUp(int keyCode, KeyEvent& event) {
    if (KeyEvent::isConfirmKey(keyCode)) {
        if (mReceivedInvokeKeyDown) {
            if (mItemCount > 0) {
                dispatchPress(mSelectedChild);
                /*postDelayed(new Runnable() {
                    public void run() {
                        dispatchUnpress();
                    }
                }, ViewConfiguration.getPressedStateDuration());*/

                int selectedIndex = mSelectedPosition - mFirstPosition;
                performItemClick(getChildAt(selectedIndex), mSelectedPosition, mAdapter->getItemId(mSelectedPosition));
            }
        }

        // Clear the flag
        mReceivedInvokeKeyDown = false;
        return true;
    }
    return AbsSpinner::onKeyUp(keyCode, event);
}

bool Gallery::moveDirection(int direction) {
    direction = isLayoutRtl() ? -direction : direction;
    int targetPosition = mSelectedPosition + direction;

    if (mItemCount > 0 && targetPosition >= 0 && targetPosition < mItemCount) {
        scrollToChild(targetPosition - mFirstPosition);
        return true;
    } else {
        return false;
    }
}

bool Gallery::scrollToChild(int childPosition) {
    View* child = getChildAt(childPosition);

    if (child != nullptr) {
        int distance = getCenterOfGallery() - getCenterOfView(child);
        mFlingRunnable->startUsingDistance(distance);
        return true;
    }

    return false;
}

void Gallery::setSelectedPositionInt(int position) {
    AbsSpinner::setSelectedPositionInt(position);

    // Updates any metadata we keep about the selected item.
    updateSelectedItemMetadata();
}

void Gallery::updateSelectedItemMetadata() {

    View* oldSelectedChild = mSelectedChild;
    View* child = mSelectedChild = getChildAt(mSelectedPosition - mFirstPosition);
    if (child == nullptr) {
        return;
    }

    child->setSelected(true);
    child->setFocusable(true);

    if (hasFocus()) {
        child->requestFocus();
    }

    // We unfocus the old child down here so the above hasFocus check
    // returns true
    if (oldSelectedChild && oldSelectedChild != child) {
        // Make sure its drawable state doesn't contain 'selected'
        oldSelectedChild->setSelected(false);

        // Make sure it is not focusable anymore, since otherwise arrow keys
        // can make this one be focused
        oldSelectedChild->setFocusable(false);
    }

}

void Gallery::setGravity(int gravity){
    if (mGravity != gravity) {
        mGravity = gravity;
        requestLayout();
    }
}

int Gallery::getChildDrawingOrder(int childCount, int i) {
    int selectedIndex = mSelectedPosition - mFirstPosition;

    // Just to be safe
    if (selectedIndex < 0) return i;
    if (i == childCount - 1) {
        // Draw the selected child last
        return selectedIndex;
    } else if (i >= selectedIndex) {
        // Move the children after the selected child earlier one
        return i + 1;
    } else {
        // Keep the children before the selected child the same
        return i;
    }
}

//////////////////////////////////////////////////////////////////////////
Gallery::FlingRunnable::FlingRunnable(Gallery*g){
     mScroller = new Scroller(g->getContext()); 
     mGallery =g;
}

void Gallery::FlingRunnable::startCommon() {
    // Remove any pending flings
    mGallery->removeCallbacks(*this);
}

void Gallery::FlingRunnable::startUsingVelocity(int initialVelocity) {
    if (initialVelocity == 0) return;
    startCommon();

    int initialX = initialVelocity < 0 ? INT_MAX : 0;
    mLastFlingX = initialX;
    mScroller->fling(initialX, 0, initialVelocity, 0, 0, INT_MAX, 0, INT_MAX);
    mGallery->post(*this);
}

void Gallery::FlingRunnable::startUsingDistance(int distance) {
    if (distance == 0) return;
    startCommon();
    mLastFlingX = 0;
    mScroller->startScroll(0, 0, -distance, 0, mGallery->mAnimationDuration);
    mGallery->post(*this);
}

void Gallery::FlingRunnable::stop(bool scrollIntoSlots) {
    mGallery->removeCallbacks(*this);
    endFling(scrollIntoSlots);
}

void Gallery::FlingRunnable::endFling(bool scrollInto) {
    /* Force the scroller's status to finished (without setting its
     * position to the end)*/
    mScroller->forceFinished(true);
    if (scrollInto) mGallery->scrollIntoSlots();
}

void Gallery::FlingRunnable::operator()() {
    if (mGallery->mItemCount == 0) {
        endFling(true);
        return;
    }
    mGallery->mShouldStopFling = false;

    bool more = mScroller->computeScrollOffset();
    int x = mScroller->getCurrX();

    // Flip sign to convert finger direction to list items direction
    // (e.g. finger moving down means list is moving towards the top)
    int delta = mLastFlingX - x;

    // Pretend that each frame of a fling scroll is a touch scroll
    if (delta > 0) {
        // Moving towards the left. Use leftmost view as mDownTouchPosition
        mGallery->mDownTouchPosition = mGallery->mIsRtl ? (mGallery->mFirstPosition + mGallery->getChildCount() - 1) :
            mGallery->mFirstPosition;

        // Don't fling more than 1 screen
        delta = std::min(mGallery->getWidth() - mGallery->mPaddingLeft - mGallery->mPaddingRight - 1, delta);
    } else {
        // Moving towards the right. Use rightmost view as mDownTouchPosition
        const int offsetToLast = mGallery->getChildCount() - 1;
        mGallery->mDownTouchPosition = mGallery->mIsRtl ? mGallery->mFirstPosition :
            (mGallery->mFirstPosition + mGallery->getChildCount() - 1);

        // Don't fling more than 1 screen
        delta = std::max(-(mGallery->getWidth() - mGallery->mPaddingRight - mGallery->mPaddingLeft - 1), delta);
    }

    mGallery->trackMotionScroll(delta);

    if (more && !mGallery->mShouldStopFling) {
        mLastFlingX = x;
        mGallery->post(*this);
    } else {
       endFling(true);
    }
}

}//endof namespace
