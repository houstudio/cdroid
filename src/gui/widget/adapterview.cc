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
#include <widget/adapterview.h>
#include <view/accessibility/accessibilitymanager.h>
#include <cdtypes.h>
#include <cdlog.h>
#include <systemclock.h>

namespace cdroid{
AdapterView::AdapterView(int w,int h):ViewGroup(w,h){
    initAdapterView();
}

void AdapterView::initAdapterView(){
    mFirstPosition=0;
    mIsVertical = true;
    mOldItemCount= mItemCount =0;
    mSelectedPosition = INVALID_POSITION;
    mNextSelectedPosition = INVALID_POSITION;
    mOldSelectedPosition = INVALID_POSITION;
    mOldSelectedRowId = INVALID_ROW_ID;
    mSelectedRowId=INVALID_ROW_ID;
    mDesiredFocusableState = FOCUSABLE_AUTO;
    mOnItemSelectedListener ={};
    mNeedSync = false;
    mDataChanged= false;
    mInLayout = false;
    mAdapter = nullptr;
    mEmptyView = nullptr;
    mSelectionNotifier =nullptr;
    mOnItemClickListener = nullptr;
    mOnItemLongClickListener = nullptr;
    mPendingSelectionNotifier =nullptr;
    setClickable(true);
    mBlockLayoutRequests = false;
    mDesiredFocusableInTouchModeState = false;

    // If not explicitly specified this view is important for accessibility.
    if (getImportantForAccessibility() == IMPORTANT_FOR_ACCESSIBILITY_AUTO) {
        setImportantForAccessibility(IMPORTANT_FOR_ACCESSIBILITY_YES);
    }

    mDesiredFocusableState = getFocusable();
    if (mDesiredFocusableState == FOCUSABLE_AUTO) {
        // Starts off without an adapter, so NOT_FOCUSABLE by default.
        ViewGroup::setFocusable((int)NOT_FOCUSABLE);
    }
}

AdapterView::AdapterView(Context*ctx,const AttributeSet&atts)
  :ViewGroup(ctx,atts){
    initAdapterView();
}

AdapterView::~AdapterView(){
    //delete mAdapter;
    //adapter maybe shared by morethan one adapterview,we consider 
    //it's good idea to freed by the owner who created it
}

void AdapterView::onDetachedFromWindow(){
    ViewGroup::onDetachedFromWindow();
    removeCallbacks(mSelectionNotifier);
}

Adapter*AdapterView::getAdapter(){
    return mAdapter;
}

void AdapterView::addView(View* child, int index, ViewGroup::LayoutParams* params){
    LOGI("addView(View*,...)is not supported in AdapterView");
}

void AdapterView::removeView(View* child){
    LOGI("is not support in AdapterView");
}

void AdapterView::removeViewAt(int index){
    LOGI("is not support in AdapterView");
}

void AdapterView::removeAllViews(){
    LOGI("is not support in AdapterView");
}

void AdapterView::onLayout(bool changed, int left, int top, int w, int h) {
    mLayoutHeight = getHeight();
}

void AdapterView::rememberSyncState() {
    if (getChildCount() > 0) {
        mNeedSync = true;
        mSyncHeight = mLayoutHeight;
        if (mSelectedPosition >= 0) {
            // Sync the selection state
            View* v = getChildAt(mSelectedPosition - mFirstPosition);
            mSyncRowId = mNextSelectedRowId;
            mSyncPosition = mNextSelectedPosition;
            if (v) mSpecificTop = v->getTop();
            mSyncMode = SYNC_SELECTED_POSITION;
        } else {
            // Sync the based on the offset of the first view
            View* v = getChildAt(0);
            Adapter* adapter = getAdapter();
            if (mFirstPosition >= 0 && mFirstPosition < adapter->getCount()) {
                mSyncRowId = adapter->getItemId(mFirstPosition);
            } else {
                mSyncRowId = NO_ID;
            }
            mSyncPosition = mFirstPosition;
            if (v) mSpecificTop = v->getTop();
            mSyncMode = SYNC_FIRST_POSITION;
        }
    }
}

void AdapterView::handleDataChanged(){
    int count = mItemCount;
    bool found = false;

    if (count > 0) {
        int newPos;
        // Find the row we are supposed to sync to
        if (mNeedSync) {
            // Update this first, since setNextSelectedPositionInt inspects
            // it
            mNeedSync = false;

            // See if we can find a position in the new data with the same
            // id as the old selection
            newPos = findSyncPosition();
            if (newPos >= 0) {
                // Verify that new selection is selectable
                int selectablePos = lookForSelectablePosition(newPos, true);
                if (selectablePos == newPos) {
                    // Same row id is selected
                    setNextSelectedPositionInt(newPos);
                    found = true;
                }
            }
        }
        if (!found) {
            // Try to use the same position if we can't find matching data
            newPos = getSelectedItemPosition();

            // Pin position to the available range
            if (newPos >= count) {
                newPos = count - 1;
            }
            if (newPos < 0) {
                newPos = 0;
            }

            // Make sure we select something selectable -- first look down
            int selectablePos = lookForSelectablePosition(newPos, true);
            if (selectablePos < 0) {
                // Looking down didn't work -- try looking up
                selectablePos = lookForSelectablePosition(newPos, false);
            }
            if (selectablePos >= 0) {
                setNextSelectedPositionInt(selectablePos);
                checkSelectionChanged();
                found = true;
            }
        }
    }
    if (!found) {
        // Nothing is selected
        mSelectedPosition = INVALID_POSITION;
        mSelectedRowId = INVALID_ROW_ID;
        mNextSelectedPosition = INVALID_POSITION;
        mNextSelectedRowId = INVALID_ROW_ID;
        mNeedSync = false;
        checkSelectionChanged();
    }
    notifySubtreeAccessibilityStateChangedIfNeeded();
}

int AdapterView::findSyncPosition(){
    const int count = mItemCount;

    if (count == 0) {
        return INVALID_POSITION;
    }

    const long idToMatch = mSyncRowId;
    int seed = mSyncPosition;

    // If there isn't a selection don't hunt for it
    if (idToMatch == INVALID_ROW_ID) {
        return INVALID_POSITION;
    }

    // Pin seed to reasonable values
    seed = std::max(0, seed);
    seed = std::min(count - 1, seed);

    int64_t endTime = SystemClock::uptimeMillis() + SYNC_MAX_DURATION_MILLIS;

    // first position scanned so far
    int first = seed;

    // last position scanned so far
    int last = seed;

    // True if we should move down on the next iteration
    bool next = false;

    // Get the item ID locally (instead of getItemIdAtPosition), so
    // we need the adapter
    Adapter*adapter=getAdapter();
    if (adapter == nullptr) {
        return INVALID_POSITION;
    }

    while (SystemClock::uptimeMillis() <= endTime) {
        long rowId = adapter->getItemId(seed);
        if (rowId == idToMatch) {
            // Found it!
            return seed;
        }

        const bool hitLast = last == count - 1;
        const bool hitFirst = first == 0;

        if (hitLast && hitFirst) {
            // Looked at everything
            break;
        }

        if (hitFirst || (next && !hitLast)) {
            // Either we hit the top, or we are trying to move down
            last++;
            seed = last;
            // Try going up next time
            next = false;
        } else if (hitLast || (!next && !hitFirst)) {
            // Either we hit the bottom, or we are trying to move up
            first--;
            seed = first;
            // Try going down next time
            next = true;
        }
    }
    return INVALID_POSITION;
}

int AdapterView::lookForSelectablePosition(int position, bool lookDown){
    return position;
}

int AdapterView::getPositionForView(View* view){
    const int childCount = getChildCount();
    View* listItem = view;
    View* v;
    while ((v = (View*) listItem->getParent()) && (v!=this)) {
        listItem = v;
    }
    for (int i = 0; view && (i < childCount); i++) {
        if (getChildAt(i)==listItem)
            return mFirstPosition + i;
    }
    return INVALID_POSITION;
}

int AdapterView::getFirstVisiblePosition() const{
    return mFirstPosition;
}

int AdapterView::getLastVisiblePosition() const{
    return mFirstPosition + getChildCount() - 1;
}

void AdapterView::setEmptyView(View* emptyView){
    mEmptyView = emptyView;
    if(emptyView&&emptyView->getImportantForAccessibility() == IMPORTANT_FOR_ACCESSIBILITY_AUTO) {
        emptyView->setImportantForAccessibility(IMPORTANT_FOR_ACCESSIBILITY_YES);
    }
    const bool empty = (mAdapter==nullptr) || mAdapter->isEmpty();
    updateEmptyStatus(empty);
}

View* AdapterView::getEmptyView(){
    return mEmptyView;
}

void* AdapterView::getItemAtPosition(int position){
    Adapter*adapter=getAdapter();
    return (adapter==nullptr||(position<0))?nullptr:adapter->getItem(position);
}

long AdapterView::getItemIdAtPosition(int position){
    Adapter*adapter = getAdapter();
    return (adapter==nullptr||(position<0))?INVALID_ROW_ID:adapter->getItemId(position);
}

int AdapterView::getSelectedItemPosition() const{
    return mNextSelectedPosition;
}

long AdapterView::getSelectedItemId() const{
    return mNextSelectedRowId;
}

void* AdapterView::getSelectedItem() {
    Adapter*adapter=getAdapter();
    int selection = getSelectedItemPosition();
    if (adapter != nullptr && adapter->getCount() > 0 && selection >= 0)
        return adapter->getItem(selection);
    return nullptr;
}

int  AdapterView::getCount(){
    return mItemCount;
}

bool AdapterView::isInFilterMode(){
    return false;
}

void AdapterView::setFocusable(int focusable) {
    Adapter*adapter = getAdapter();
    bool empty = adapter == nullptr || adapter->getCount() == 0;

    mDesiredFocusableState = focusable;
    if ((focusable & (FOCUSABLE_AUTO | FOCUSABLE)) == 0) {
        mDesiredFocusableInTouchModeState = false;
    }
    focusable=(!empty || isInFilterMode()) ? focusable : NOT_FOCUSABLE;
    ViewGroup::setFocusable(focusable);
}

void AdapterView::setFocusableInTouchMode(bool focusable) {
    Adapter*adapter = getAdapter();
    bool empty = adapter == nullptr || adapter->getCount() == 0;

    mDesiredFocusableInTouchModeState = focusable;
    if (focusable) {
        mDesiredFocusableState = FOCUSABLE;
    }

    ViewGroup::setFocusableInTouchMode(focusable && (!empty || isInFilterMode()));
}

void AdapterView::setNextSelectedPositionInt(int position){
     LOGV("%d->%d",mNextSelectedPosition,position);
     mNextSelectedPosition = position;
     mNextSelectedRowId = getItemIdAtPosition(position);
     // If we are trying to sync to the selection, update that too
     if (mNeedSync && mSyncMode == SYNC_SELECTED_POSITION && position >= 0) {
         mSyncPosition = position;
         mSyncRowId = mNextSelectedRowId;
     }
}

void AdapterView::setSelectedPositionInt(int position) {
    mSelectedPosition = position;
    mSelectedRowId = getItemIdAtPosition(position);
}

void AdapterView::checkSelectionChanged() {
    if ((mSelectedPosition != mOldSelectedPosition) || (mSelectedRowId != mOldSelectedRowId)) {
        selectionChanged();
        mOldSelectedPosition = mSelectedPosition;
        mOldSelectedRowId = mSelectedRowId;
    }

    // If we have a pending selection notification -- and we won't if we
    // just fired one in selectionChanged() -- run it now.
    if (mPendingSelectionNotifier != nullptr) {
        mPendingSelectionNotifier();
    }
}

void AdapterView::checkFocus() {
    Adapter*adapter=getAdapter();
    const bool empty = (adapter==nullptr)||(adapter->getCount() == 0);
    const bool focusable = !empty || isInFilterMode();
    // The order in which we set focusable in touch mode/focusable may matter
    // for the client, see View.setFocusableInTouchMode() comments for more
    // details
    ViewGroup::setFocusableInTouchMode(focusable && mDesiredFocusableInTouchModeState);
    ViewGroup::setFocusable((int)(focusable ? mDesiredFocusableState : NOT_FOCUSABLE));
    if (mEmptyView != nullptr) {
        updateEmptyStatus((adapter == nullptr) || adapter->isEmpty());
    }
}

void AdapterView::updateEmptyStatus(bool empty) {
    if (isInFilterMode()) {
        empty = false;
    }

    if (empty) {
        if (mEmptyView ) {
            mEmptyView->setVisibility(View::VISIBLE);
            setVisibility(View::GONE);
        } else {
            // If the caller just removed our empty view, make sure the list view is visible
            setVisibility(View::VISIBLE);
        }

        // We are now GONE, so pending layouts will not be dispatched.
        // Force one here to make sure that the state of the list matches
        // the state of the adapter.
        if (mDataChanged) {
            onLayout(false, getX(), getY(), getWidth(), getHeight());
        }
    } else {
        if (mEmptyView ) mEmptyView->setVisibility(View::GONE);
        setVisibility(View::VISIBLE);
    }
}

void AdapterView::doSectionNotify(){
    mPendingSelectionNotifier = nullptr;
    ViewGroup*root=getRootView();
    if(mDataChanged && root && root->isLayoutRequested()){
        if(getAdapter())
            mPendingSelectionNotifier = std::bind(&AdapterView::doSectionNotify,this);
    }else{
        dispatchOnItemSelected();
    }
}

void AdapterView::selectionChanged(){
    mPendingSelectionNotifier = nullptr;
    if (mOnItemSelectedListener.onItemSelected
            ||AccessibilityManager::getInstance(mContext).isEnabled()){
        if (mInLayout || mBlockLayoutRequests) {
            // If we are in a layout traversal, defer notification
            // by posting. This ensures that the view tree is
            // in a consistent state and is able to accommodate
            // new layout or invalidate requests.
            if (mSelectionNotifier==nullptr) {
                mSelectionNotifier = std::bind(&AdapterView::doSectionNotify,this);
            } else {
                removeCallbacks(mSelectionNotifier);
            }
            post(mSelectionNotifier);
         } else {
            dispatchOnItemSelected();
        }
    }
    // Always notify AutoFillManager - it will return right away if autofill is disabled.
    /*final AutofillManager afm = mContext.getSystemService(AutofillManager.class);
    if (afm != null) {
        afm.notifyValueChanged(this);
    }*/
}

void AdapterView::dispatchOnItemSelected() {
    fireOnSelected();
    performAccessibilityActionsOnSelected();
}

void AdapterView::fireOnSelected() {
    int selection = getSelectedItemPosition();
    if (selection >= 0) {
        View* v = getSelectedView();
        if(mOnItemSelectedListener.onItemSelected)
            mOnItemSelectedListener.onItemSelected(*this, *v, selection,getAdapter()->getItemId(selection));
    } else if(mOnItemSelectedListener.onNothingSelected){
        mOnItemSelectedListener.onNothingSelected(*this);
    }
}

void AdapterView::performAccessibilityActionsOnSelected() {
    if (!AccessibilityManager::getInstance(mContext).isEnabled()) {
        return;
    }
    const int position = getSelectedItemPosition();
    if (position >= 0) {
        // we fire selection events here not in View
        sendAccessibilityEvent(AccessibilityEvent::TYPE_VIEW_SELECTED);
    }
}

std::string AdapterView::getAccessibilityClassName()const{
    return "AdapterView";
}

bool AdapterView::dispatchPopulateAccessibilityEventInternal(AccessibilityEvent& event){
    View* selectedView = getSelectedView();
    if (selectedView && selectedView->getVisibility() == VISIBLE
            && selectedView->dispatchPopulateAccessibilityEvent(event)) {
        return true;
    }
    return false;
}

bool AdapterView::onRequestSendAccessibilityEventInternal(View* child, AccessibilityEvent& event){
    if (ViewGroup::onRequestSendAccessibilityEventInternal(child, event)) {
        // Add a record for ourselves as well.
        AccessibilityEvent* record = AccessibilityEvent::obtain();
        onInitializeAccessibilityEvent(*record);
        // Populate with the text of the requesting child.
        child->dispatchPopulateAccessibilityEvent(*record);
        event.appendRecord(record);
        return true;
    }
    return false;
}

void AdapterView::onInitializeAccessibilityNodeInfoInternal(AccessibilityNodeInfo& info){
   ViewGroup::onInitializeAccessibilityNodeInfoInternal(info);
   info.setScrollable(isScrollableForAccessibility());
   View* selectedView = getSelectedView();
   if (selectedView != nullptr) {
        info.setEnabled(selectedView->isEnabled());
    }
}

void AdapterView::onInitializeAccessibilityEventInternal(AccessibilityEvent& event){
    ViewGroup::onInitializeAccessibilityEventInternal(event);
    event.setScrollable(isScrollableForAccessibility());
    View* selectedView = getSelectedView();
    if (selectedView != nullptr) {
        event.setEnabled(selectedView->isEnabled());
    }
    event.setCurrentItemIndex(getSelectedItemPosition());
    event.setFromIndex(getFirstVisiblePosition());
    event.setToIndex(getLastVisiblePosition());
    event.setItemCount(getCount());
}

bool AdapterView::isScrollableForAccessibility() {
    Adapter* adapter = getAdapter();
    if (adapter != nullptr) {
        const int itemCount = adapter->getCount();
        return itemCount > 0
            && (getFirstVisiblePosition() > 0 || getLastVisiblePosition() < itemCount - 1);
    }
    return false;
}

bool AdapterView::performItemClick(View& view, int position, long id){
    view.sendAccessibilityEvent(AccessibilityEvent::TYPE_VIEW_CLICKED);
    if (mOnItemClickListener != nullptr) {
        playSoundEffect(SoundEffectConstants::CLICK);
        mOnItemClickListener(*this,view, position, id);
        return true;
    }
    return false;
}


void AdapterView::setOnItemClickListener(const OnItemClickListener& listener) {
    mOnItemClickListener = listener;
}

AdapterView::OnItemClickListener AdapterView::getOnItemClickListener() const{
    return mOnItemClickListener;
}

void AdapterView::setOnItemSelectedListener(const OnItemSelectedListener& listener) {
    mOnItemSelectedListener = listener;
}

AdapterView::OnItemSelectedListener AdapterView::getOnItemSelectedListener()const {
    return mOnItemSelectedListener;
}

void AdapterView::setOnItemLongClickListener(const OnItemLongClickListener& listener){
    if (!isLongClickable()) setLongClickable(true);
    mOnItemLongClickListener = listener;
}

AdapterView::OnItemLongClickListener AdapterView::getOnItemLongClickListener() const{
    return mOnItemLongClickListener;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
AdapterView::AdapterContextMenuInfo::AdapterContextMenuInfo(View* view, int pos, long _id):
    targetView(view),position(pos),id(_id){
}

AdapterDataSetObserver::AdapterDataSetObserver(AdapterView*lv){
    mAdapterView = lv;
}

AdapterView* AdapterDataSetObserver::getAdapterView()const{
    return mAdapterView;
}

void AdapterDataSetObserver::onChanged() {
    AdapterView* adv = mAdapterView;
    adv->mDataChanged = true;
    adv->mOldItemCount= adv->mItemCount;
    adv->mItemCount   = adv->getAdapter()->getCount();
    // Detect the case where a cursor that was previously invalidated has
    // been repopulated with new data.
    if (adv->getAdapter()->hasStableIds() //&& mInstanceState != null
          && adv->mOldItemCount == 0 && adv->mItemCount > 0) {
         adv->onRestoreInstanceState(adv->mInstanceState);
         //mInstanceState = null;
    } else {
         adv->rememberSyncState();
    }
    adv->checkFocus();
    adv->requestLayout();
}

void AdapterDataSetObserver::onInvalidated() {
    AdapterView* adv = mAdapterView;
    adv->mDataChanged = true;

    if (adv->getAdapter()->hasStableIds()) {
        // Remember the current state for the case where our hosting activity is being
        // stopped and later restarted
        //mInstanceState = AdapterView.this.onSaveInstanceState();
        //adv->mInstanceState = adv->onSaveInstanceState();
    }
    // Data is invalid so we should reset our state
    adv->mOldItemCount = adv->mItemCount;
    adv->mItemCount = 0;
    adv->mSelectedPosition = AdapterView::INVALID_POSITION;
    adv->mSelectedRowId    = AdapterView::INVALID_ROW_ID;
    adv->mNextSelectedPosition = AdapterView::INVALID_POSITION;
    adv->mNextSelectedRowId    = AdapterView::INVALID_ROW_ID;
    adv->mNeedSync = false;

    adv->checkFocus();
    adv->requestLayout();
}

void AdapterDataSetObserver::clearSavedState() {
     //mInstanceState = null;
}

}//namespace
