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
#include <widget/textview.h>
#include <widget/dropdownlistview.h>

namespace cdroid{

DropDownListView::DropDownListView(Context*context,bool hijackfocus)
    :ListView(context,context->obtainStyledAttributes("cdroid:attr/dropDownListViewStyle")){
    mScrollHelper = nullptr;
    mHijackFocus = hijackfocus;
    mDrawsInPressedState = false;
    mResolveHoverRunnable = nullptr;
}

DropDownListView::~DropDownListView(){
    LOGD("destroy %p",this);
    delete mScrollHelper;
    removeCallbacks(mResolveHoverRunnable);
}

bool DropDownListView::shouldShowSelector()const{
    return isHovered() || ListView::shouldShowSelector();
}

bool DropDownListView::onTouchEvent(MotionEvent& ev){
    if(mResolveHoverRunnable){
        removeCallbacks(mResolveHoverRunnable);
        mResolveHoverRunnable = nullptr;
    }
    return ListView::onTouchEvent(ev);
}

bool DropDownListView::onHoverEvent(MotionEvent& ev){
    const int action = ev.getActionMasked();
    if ((action == MotionEvent::ACTION_HOVER_EXIT) && (mResolveHoverRunnable == nullptr)) {
        // This may be transitioning to TOUCH_DOWN. Postpone drawable state
        // updates until either the next frame or the next touch event.
        mResolveHoverRunnable = [this](){
            mResolveHoverRunnable = nullptr;
            drawableStateChanged();
        };
        post(mResolveHoverRunnable);
    }

    // Allow the super class to handle hover state management first.
    bool handled = ListView::onHoverEvent(ev);

    if ((action == MotionEvent::ACTION_HOVER_ENTER) || (action == MotionEvent::ACTION_HOVER_MOVE)) {
        const int position = pointToPosition((int) ev.getX(), (int) ev.getY());
        if ((position != INVALID_POSITION) && (position != mSelectedPosition)) {
            View* hoveredItem = getChildAt(position - getFirstVisiblePosition());
            if (hoveredItem->isEnabled()) {
                // Force a focus so that the proper selector state gets
                // used when we update.
                requestFocus();

                positionSelector(position, hoveredItem);
                setSelectedPositionInt(position);
                setNextSelectedPositionInt(position);
            }
            updateSelectorState();
        }
    } else {
        // Do not cancel the selected position if the selection is visible
        // by other means.
        if (!ListView::shouldShowSelector()) {
            setSelectedPositionInt(INVALID_POSITION);
            setNextSelectedPositionInt(INVALID_POSITION);
        }
    }

    return handled;
}

void DropDownListView::drawableStateChanged() {
    if (mResolveHoverRunnable == nullptr) {
        ListView::drawableStateChanged();
    }
}

bool DropDownListView::onForwardedEvent(MotionEvent& event, int activePointerId) {
    bool handledEvent = true;
    bool bClearPressedItem = false;
    int x,y,activeIndex,position;
    const int actionMasked = event.getActionMasked();
    switch (actionMasked) {
        case MotionEvent::ACTION_CANCEL:
            handledEvent = false;
            break;
        case MotionEvent::ACTION_UP:
            handledEvent = false;
            // $FALL-THROUGH$
        case MotionEvent::ACTION_MOVE:
            activeIndex = event.findPointerIndex(activePointerId);
            if (activeIndex < 0) {
                handledEvent = false;
                break;
            }

            x = (int) event.getX(activeIndex);
            y = (int) event.getY(activeIndex);
            position = pointToPosition(x, y);
            if (position == INVALID_POSITION) {
                bClearPressedItem = true;
                break;
            }

            View* child = getChildAt(position - getFirstVisiblePosition());
            setPressedItem(child, position, x, y);
            handledEvent = true;

            if (actionMasked == MotionEvent::ACTION_UP) {
                long id = getItemIdAtPosition(position);
                performItemClick(*child, position, id);
            }
            break;
    }

    // Failure to handle the event cancels forwarding.
    if (!handledEvent || bClearPressedItem) {
        clearPressedItem();
    }

    // Manage automatic scrolling.
    if (handledEvent) {
        if (mScrollHelper == nullptr) {
            mScrollHelper = new AbsListViewAutoScroller(this);
        }
        mScrollHelper->setEnabled(true);
        mScrollHelper->onTouch(*this, event);
    } else if (mScrollHelper != nullptr) {
        mScrollHelper->setEnabled(false);
    }

    return handledEvent;
}

void DropDownListView::setListSelectionHidden(bool hideListSelection) {
    mListSelectionHidden = hideListSelection;
}

void DropDownListView::clearPressedItem() {
    mDrawsInPressedState = false;
    setPressed(false);
    updateSelectorState();

    View* motionView = getChildAt(mMotionPosition - mFirstPosition);
    if (motionView) motionView->setPressed(false);
}

void DropDownListView::setPressedItem(View* child, int position, float x, float y) {
    mDrawsInPressedState = true;

    // Ordering is essential. First, update the container's pressed state.
    drawableHotspotChanged(x, y);
    if (!isPressed()) {
        setPressed(true);
    }

    // Next, run layout if we need to stabilize child positions.
    if (mDataChanged) {
        layoutChildren();
    }

    // Manage the pressed view based on motion position. This allows us to
    // play nicely with actual touch and scroll events.
    View* motionView = getChildAt(mMotionPosition - mFirstPosition);
    if (motionView && motionView != child && motionView->isPressed()) {
        motionView->setPressed(false);
    }
    mMotionPosition = position;

    // Offset for child coordinates.
    const float childX = x - child->getLeft();
    const float childY = y - child->getTop();
    child->drawableHotspotChanged(childX, childY);
    if (!child->isPressed()) {
        child->setPressed(true);
    }

    // Ensure that keyboard focus starts from the last touched position.
    setSelectedPositionInt(position);
    positionSelectorLikeTouch(position, child, x, y);

    // Refresh the drawable state to reflect the new pressed state,
    // which will also update the selector state.
    refreshDrawableState();
}

bool DropDownListView::touchModeDrawsInPressedState() const{
    return mDrawsInPressedState || ListView::touchModeDrawsInPressedState();
}

View* DropDownListView::obtainView(int position, bool* isScrap) {
    View* view = ListView::obtainView(position, isScrap);
    if (dynamic_cast<TextView*>(view)) {
        ((TextView*) view)->setHorizontallyScrolling(true);
    }
    return view;
}

bool DropDownListView::isInTouchMode()const{
    // WARNING: Please read the comment where mListSelectionHidden is declared
    return (mHijackFocus && mListSelectionHidden) || ListView::isInTouchMode();
}

bool DropDownListView::hasWindowFocus()const{
    return mHijackFocus || ListView::hasWindowFocus();
}

bool DropDownListView::isFocused()const{
    return mHijackFocus || ListView::isFocused();
}

bool DropDownListView::hasFocus()const{
    return mHijackFocus || ListView::hasFocus();
}

}/*endof namespace*/
