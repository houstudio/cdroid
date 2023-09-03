#include <widget/expandablelistview.h>
#include <core/soundeffect.h>
namespace cdroid{

DECLARE_WIDGET(ExpandableListView)

ExpandableListView::ExpandableListView(int w,int h):ListView(w,h){
    initView();
}

ExpandableListView::ExpandableListView(Context* context,const AttributeSet& attrs)
  :ListView(context,attrs){
    initView();
    mGroupIndicator = attrs.getDrawable("groupIndicator");
    mChildIndicator = attrs.getDrawable("childIndicator");
    mIndicatorLeft = attrs.getDimensionPixelSize("indicatorLeft", 0);
    mIndicatorRight = attrs.getDimensionPixelSize("indicatorRight", 0);
    if (mIndicatorRight == 0 && mGroupIndicator ) {
        mIndicatorRight = mIndicatorLeft + mGroupIndicator->getIntrinsicWidth();
    }
    mChildIndicatorLeft = attrs.getDimensionPixelSize("childIndicatorLeft", CHILD_INDICATOR_INHERIT);
    mChildIndicatorRight = attrs.getDimensionPixelSize("childIndicatorRight", CHILD_INDICATOR_INHERIT);
    mChildDivider = attrs.getDrawable("childDivider");

    if (!isRtlCompatibilityMode()) {
        mIndicatorStart = attrs.getDimensionPixelSize("indicatorStart", INDICATOR_UNDEFINED);
        mIndicatorEnd = attrs.getDimensionPixelSize("indicatorEnd",INDICATOR_UNDEFINED);

        mChildIndicatorStart = attrs.getDimensionPixelSize("childIndicatorStart", CHILD_INDICATOR_INHERIT);
        mChildIndicatorEnd = attrs.getDimensionPixelSize("childIndicatorEnd", CHILD_INDICATOR_INHERIT);
    }
}

ExpandableListView::~ExpandableListView(){
    delete mConnector;
    delete mGroupIndicator;
    delete mChildIndicator;
    delete mChildDivider;
}

void ExpandableListView::initView(){
    mGroupIndicator = nullptr;
    mChildIndicator = nullptr;
    mChildDivider  = nullptr;
    mIndicatorLeft = 0;
    mIndicatorRight= 0;
    mConnector = nullptr;
    mAdapter = nullptr;
    mChildIndicatorLeft = CHILD_INDICATOR_INHERIT;
    mChildIndicatorRight= CHILD_INDICATOR_INHERIT;
    mIndicatorStart = INDICATOR_UNDEFINED;
    mIndicatorEnd   = INDICATOR_UNDEFINED;
    mChildIndicatorStart = CHILD_INDICATOR_INHERIT;
    mChildIndicatorEnd   = CHILD_INDICATOR_INHERIT;
}

bool ExpandableListView::isRtlCompatibilityMode()const{
    //final int targetSdkVersion = mContext.getApplicationInfo().targetSdkVersion;
    return /*targetSdkVersion < JELLY_BEAN_MR1 ||*/ !hasRtlSupport();
}

void ExpandableListView::onRtlPropertiesChanged(int layoutDirection){
    resolveIndicator();
    resolveChildIndicator();
}

void ExpandableListView::resolveIndicator() {
    const bool bIsLayoutRtl = isLayoutRtl();
    if (bIsLayoutRtl) {
        if (mIndicatorStart >= 0) {
            mIndicatorRight = mIndicatorStart;
        }
        if (mIndicatorEnd >= 0) {
            mIndicatorLeft = mIndicatorEnd;
        }
    } else {
        if (mIndicatorStart >= 0) {
            mIndicatorLeft = mIndicatorStart;
        }
        if (mIndicatorEnd >= 0) {
            mIndicatorRight = mIndicatorEnd;
        }
    }
    if (mIndicatorRight == 0 && mGroupIndicator) {
        mIndicatorRight = mIndicatorLeft + mGroupIndicator->getIntrinsicWidth();
    }
}

void ExpandableListView::resolveChildIndicator() {
    const bool bIsLayoutRtl = isLayoutRtl();
    if (bIsLayoutRtl) {
        if (mChildIndicatorStart >= CHILD_INDICATOR_INHERIT) {
            mChildIndicatorRight = mChildIndicatorStart;
        }
        if (mChildIndicatorEnd >= CHILD_INDICATOR_INHERIT) {
            mChildIndicatorLeft = mChildIndicatorEnd;
        }
    } else {
        if (mChildIndicatorStart >= CHILD_INDICATOR_INHERIT) {
            mChildIndicatorLeft = mChildIndicatorStart;
        }
        if (mChildIndicatorEnd >= CHILD_INDICATOR_INHERIT) {
            mChildIndicatorRight = mChildIndicatorEnd;
        }
    }
}

void ExpandableListView::dispatchDraw(Canvas& canvas) {
    // Draw children, etc.
    ListView::dispatchDraw(canvas);

    // If we have any indicators to draw, we do it here
    if ((mChildIndicator == nullptr) && (mGroupIndicator == nullptr)) {
        return;
    }

    const bool clipToPadding = (mGroupFlags & CLIP_TO_PADDING_MASK) == CLIP_TO_PADDING_MASK;
    if (clipToPadding) {
        canvas.save();
        canvas.rectangle(mScrollX + mPaddingLeft, mScrollY + mPaddingTop,
                mScrollX + mRight - mLeft - mPaddingRight,
                mScrollY + mBottom - mTop - mPaddingBottom);
	canvas.clip();
    }

    int headerViewsCount = getHeaderViewsCount();

    int lastChildFlPos = mItemCount - getFooterViewsCount() - headerViewsCount - 1;

    int myB = mBottom;

    ExpandableListConnector::PositionMetadata* pos;
    View* item;
    Drawable* indicator;
    int t, b;

    // Start at a value that is neither child nor group
    int lastItemType = ~(ExpandableListPosition::CHILD | ExpandableListPosition::GROUP);

    Rect indicatorRect;// = mIndicatorRect;

    // The "child" mentioned in the following two lines is this
    // View's child, not referring to an expandable list's
    // notion of a child (as opposed to a group)
    const int childCount = getChildCount();
    for (int i = 0, childFlPos = mFirstPosition - headerViewsCount; i < childCount;
         i++, childFlPos++) {

        if (childFlPos < 0) {
            // This child is header
            continue;
        } else if (childFlPos > lastChildFlPos) {
            // This child is footer, so are all subsequent children
            break;
        }

        item = getChildAt(i);
        t = item->getTop();
        b = item->getBottom();

        // This item isn't on the screen
        if ((b < 0) || (t > myB)) continue;

        // Get more expandable list-related info for this item
        pos = mConnector->getUnflattenedPos(childFlPos);

        const bool bIsLayoutRtl = isLayoutRtl();
        const int width = getWidth();

        // If this item type and the previous item type are different, then we need to change
        // the left & right bounds
        if (pos->position->type != lastItemType) {
            if (pos->position->type == ExpandableListPosition::CHILD) {
                indicatorRect.left = (mChildIndicatorLeft == CHILD_INDICATOR_INHERIT) ?
                        mIndicatorLeft : mChildIndicatorLeft;
                indicatorRect.width = ((mChildIndicatorRight == CHILD_INDICATOR_INHERIT) ?
                        mIndicatorRight : mChildIndicatorRight) - indicatorRect.left;
            } else {
                indicatorRect.left = mIndicatorLeft;
                indicatorRect.width = mIndicatorRight - mIndicatorLeft;
            }

            if (bIsLayoutRtl) {
                const int temp = indicatorRect.left;
                indicatorRect.left = width - indicatorRect.right();
                indicatorRect.width = indicatorRect.right() - temp;

                indicatorRect.left -= mPaddingRight;
                //indicatorRect.right -= mPaddingRight;
            } else {
                indicatorRect.left += mPaddingLeft;
                //indicatorRect.right += mPaddingLeft;
            }

            lastItemType = pos->position->type;
        }

        if (indicatorRect.width > 0) {
            // Use item's full height + the divider height
            if (mStackFromBottom) {
                // See ListView#dispatchDraw
                indicatorRect.top = t;// - mDividerHeight;
                indicatorRect.height = b - t;
            } else {
                indicatorRect.top = t;
                indicatorRect.height = b - t;// + mDividerHeight;
            }

            // Get the indicator (with its state set to the item's state)
            indicator = getIndicator(*pos);
            if (indicator) {
                // Draw the indicator
                indicator->setBounds(indicatorRect);
                indicator->draw(canvas);
            }
        }
        pos->recycle();
    }

    if (clipToPadding) {
        canvas.restore();
    }
}

Drawable* ExpandableListView::getIndicator(ExpandableListConnector::PositionMetadata& pos) {
    Drawable* indicator;

    if (pos.position->type == ExpandableListPosition::GROUP) {
        indicator = mGroupIndicator;

        if (indicator && indicator->isStateful()) {
            // Empty check based on availability of data.  If the groupMetadata isn't null,
            // we do a check on it. Otherwise, the group is collapsed so we consider it
            // empty for performance reasons.
            const bool isEmpty = (pos.groupMetadata == nullptr) ||
                    (pos.groupMetadata->lastChildFlPos == pos.groupMetadata->flPos);

            const int stateSetIndex = (pos.isExpanded() ? 1 : 0) | // Expanded?
                (isEmpty ? 2 : 0); // Empty?
            //indicator->setState(GROUP_STATE_SETS[stateSetIndex]);
        }
    } else {
        indicator = mChildIndicator;

        if (indicator && indicator->isStateful()) {
            // No need for a state sets array for the child since it only has two states
            /*int stateSet[] = pos.position.flatListPos == pos.groupMetadata.lastChildFlPos
                    ? CHILD_LAST_STATE_SET : EMPTY_STATE_SET;
            indicator->setState(stateSet);*/
        }
    }

    return indicator;
}

void ExpandableListView::setChildDivider(Drawable* childDivider) {
    mChildDivider = childDivider;
}

void ExpandableListView::drawDivider(Canvas& canvas, Rect bounds, int childIndex) {
        int flatListPosition = childIndex + mFirstPosition;

        // Only proceed as possible child if the divider isn't above all items (if it is above
        // all items, then the item below it has to be a group)
        if (flatListPosition >= 0) {
            const int adjustedPosition = getFlatPositionForConnector(flatListPosition);
	    ExpandableListConnector::PositionMetadata* pos = mConnector->getUnflattenedPos(adjustedPosition);
            // If this item is a child, or it is a non-empty group that is expanded
            if ((pos->position->type == ExpandableListPosition::CHILD) || (pos->isExpanded() &&
                    pos->groupMetadata->lastChildFlPos != pos->groupMetadata->flPos)) {
                // These are the cases where we draw the child divider
                Drawable* divider = mChildDivider;
                divider->setBounds(bounds);
                divider->draw(canvas);
                pos->recycle();
                return;
            }
            pos->recycle();
        }

        // Otherwise draw the default divider
	ListView::drawDivider(canvas, bounds, flatListPosition);
}

/*void ExpandableListView::setOnItemClickListener(OnItemClickListener l) {
    ListView::setOnItemClickListener(l);
}*/
void ExpandableListView::setAdapter(ExpandableListAdapter* adapter){
    mAdapter = adapter;
    if (adapter != nullptr) {
        // Create the connector
        mConnector = new ExpandableListConnector(adapter);
    } else {
        mConnector = nullptr;
    }

    // Link the ListView (superclass) to the expandable list data through the connector
    ListView::setAdapter(mConnector);   
}

ExpandableListAdapter* ExpandableListView::getExpandableListAdapter(){
    return mAdapter;
}

void ExpandableListView::setAdapter(ListAdapter* adapter) {
    FATAL("For ExpandableListView, use setAdapter(ExpandableListAdapter) instead of setAdapter(ListAdapter)");
}

bool ExpandableListView::isHeaderOrFooterPosition(int position) {
    const int footerViewsStart = mItemCount - getFooterViewsCount();
    return (position < getHeaderViewsCount() || position >= footerViewsStart);
}

int ExpandableListView::getFlatPositionForConnector(int flatListPosition) {
    return flatListPosition - getHeaderViewsCount();
}


int ExpandableListView::getAbsoluteFlatPosition(int flatListPosition) {
    return flatListPosition + getHeaderViewsCount();
}

bool ExpandableListView::performItemClick(View& v, int position, long id) {
    // Ignore clicks in header/footers
    if (isHeaderOrFooterPosition(position)) {
        // Clicked on a header/footer, so ignore pass it on to super
        return ListView::performItemClick(v, position, id);
    }

    // Internally handle the item click
    const int adjustedPosition = getFlatPositionForConnector(position);
    return handleItemClick(v, adjustedPosition, id);
}

bool ExpandableListView::handleItemClick(View& v, int position, long id) {
    ExpandableListConnector::PositionMetadata* posMetadata = mConnector->getUnflattenedPos(position);

    id = getChildOrGroupId(*posMetadata->position);

    bool returnValue;
    if (posMetadata->position->type == ExpandableListPosition::GROUP) {
        /* It's a group, so handle collapsing/expanding */

        /* It's a group click, so pass on event */
        if (mOnGroupClickListener) {
            if (mOnGroupClickListener(*this, v, posMetadata->position->groupPos, id)) {
                posMetadata->recycle();
                return true;
            }
        }

        if (posMetadata->isExpanded()) {
            /* Collapse it */
            mConnector->collapseGroup(*posMetadata);

            playSoundEffect(SoundEffectConstants::CLICK);

            if (mOnGroupCollapseListener) {
                mOnGroupCollapseListener(posMetadata->position->groupPos);
            }
        } else {
            /* Expand it */
            mConnector->expandGroup(*posMetadata);

            playSoundEffect(SoundEffectConstants::CLICK);

            if (mOnGroupExpandListener) {
                mOnGroupExpandListener(posMetadata->position->groupPos);
            }

            const int groupPos = posMetadata->position->groupPos;
            const int groupFlatPos = posMetadata->position->flatListPos;

            const int shiftedGroupPosition = groupFlatPos + getHeaderViewsCount();
            smoothScrollToPosition(shiftedGroupPosition + mAdapter->getChildrenCount(groupPos),
                    shiftedGroupPosition);
        }

        returnValue = true;
    } else {
        /* It's a child, so pass on event */
        if (mOnChildClickListener) {
            playSoundEffect(SoundEffectConstants::CLICK);
            return mOnChildClickListener(*this, v, posMetadata->position->groupPos,
                    posMetadata->position->childPos, id);
        }

        returnValue = false;
    }

    posMetadata->recycle();

    return returnValue;
}

bool ExpandableListView::expandGroup(int groupPos) {
    return expandGroup(groupPos, false);
}

bool ExpandableListView::expandGroup(int groupPos, bool animate) {
    ExpandableListPosition* elGroupPos = ExpandableListPosition::obtain(
            ExpandableListPosition::GROUP, groupPos, -1, -1);
    ExpandableListConnector::PositionMetadata* pm = mConnector->getFlattenedPos(*elGroupPos);
    elGroupPos->recycle();
    const bool retValue = mConnector->expandGroup(*pm);

    if (mOnGroupExpandListener) {
        mOnGroupExpandListener(groupPos);
    }

    if (animate) {
        const int groupFlatPos = pm->position->flatListPos;

        const int shiftedGroupPosition = groupFlatPos + getHeaderViewsCount();
        smoothScrollToPosition(shiftedGroupPosition + mAdapter->getChildrenCount(groupPos),
                shiftedGroupPosition);
    }
    pm->recycle();

    return retValue;
}

bool ExpandableListView::collapseGroup(int groupPos) {
    const bool retValue = mConnector->collapseGroup(groupPos);

    if (mOnGroupCollapseListener ) {
        mOnGroupCollapseListener(groupPos);
    }

    return retValue;
}

void ExpandableListView::setOnGroupCollapseListener(OnGroupCollapseListener onGroupCollapseListener){
    mOnGroupCollapseListener = onGroupCollapseListener;
}

void ExpandableListView::setOnGroupExpandListener( OnGroupExpandListener onGroupExpandListener) {
    mOnGroupExpandListener = onGroupExpandListener;
}

void ExpandableListView::setOnGroupClickListener(OnGroupClickListener onGroupClickListener) {
    mOnGroupClickListener = onGroupClickListener;
}

void ExpandableListView::setOnChildClickListener(OnChildClickListener onChildClickListener) {
    mOnChildClickListener = onChildClickListener;
}

long ExpandableListView::getExpandableListPosition(int flatListPosition) {
    if (isHeaderOrFooterPosition(flatListPosition)) {
        return PACKED_POSITION_VALUE_NULL;
    }
    const int adjustedPosition = getFlatPositionForConnector(flatListPosition);
    ExpandableListConnector::PositionMetadata* pm = mConnector->getUnflattenedPos(adjustedPosition);
    const long packedPos = pm->position->getPackedPosition();
    pm->recycle();
    return packedPos;
}

int ExpandableListView::getFlatListPosition(long packedPosition) {
    ExpandableListPosition* elPackedPos = ExpandableListPosition::obtainPosition(packedPosition);
    ExpandableListConnector::PositionMetadata* pm = mConnector->getFlattenedPos(*elPackedPos);
    elPackedPos->recycle();
    const int flatListPosition = pm->position->flatListPos;
    pm->recycle();
    return getAbsoluteFlatPosition(flatListPosition);
}

long ExpandableListView::getSelectedPosition() {
    const int selectedPos = getSelectedItemPosition();

    // The case where there is no selection (selectedPos == -1) is also handled here.
    return getExpandableListPosition(selectedPos);
}

long ExpandableListView::getSelectedId() {
    long packedPos = getSelectedPosition();
    if (packedPos == PACKED_POSITION_VALUE_NULL) return -1;

    int groupPos = getPackedPositionGroup(packedPos);

    if (getPackedPositionType(packedPos) == PACKED_POSITION_TYPE_GROUP) {
        // It's a group
        return mAdapter->getGroupId(groupPos);
    } else {
        // It's a child
        return mAdapter->getChildId(groupPos, getPackedPositionChild(packedPos));
    }
}

void ExpandableListView::setSelectedGroup(int groupPosition) {
    ExpandableListPosition* elGroupPos = ExpandableListPosition::obtainGroupPosition(groupPosition);
    ExpandableListConnector::PositionMetadata* pm = mConnector->getFlattenedPos(*elGroupPos);
    elGroupPos->recycle();
    const int absoluteFlatPosition = getAbsoluteFlatPosition(pm->position->flatListPos);
    ListView::setSelection(absoluteFlatPosition);
    pm->recycle();
}

bool ExpandableListView::setSelectedChild(int groupPosition, int childPosition, bool shouldExpandGroup) {
   ExpandableListPosition* elChildPos = ExpandableListPosition::obtainChildPosition(groupPosition, childPosition);
   ExpandableListConnector::PositionMetadata* flatChildPos = mConnector->getFlattenedPos(*elChildPos);

    if (flatChildPos == nullptr) {
        // The child's group isn't expanded

        // Shouldn't expand the group, so return false for we didn't set the selection
        if (!shouldExpandGroup) return false;

        expandGroup(groupPosition);

        flatChildPos = mConnector->getFlattenedPos(*elChildPos);

        // Validity check
        if (flatChildPos == nullptr) {
            FATAL("Could not find child");
        }
    }

    const int absoluteFlatPosition = getAbsoluteFlatPosition(flatChildPos->position->flatListPos);
    ListView::setSelection(absoluteFlatPosition);

    elChildPos->recycle();
    flatChildPos->recycle();

    return true;
}

bool ExpandableListView::isGroupExpanded(int groupPosition) {
    return mConnector->isGroupExpanded(groupPosition);
}

int ExpandableListView::getPackedPositionType(long packedPosition) {
    if (packedPosition == PACKED_POSITION_VALUE_NULL) {
        return PACKED_POSITION_TYPE_NULL;
    }
    return (packedPosition & PACKED_POSITION_MASK_TYPE) == PACKED_POSITION_MASK_TYPE
            ? PACKED_POSITION_TYPE_CHILD  : PACKED_POSITION_TYPE_GROUP;
}

int ExpandableListView::getPackedPositionGroup(long packedPosition) {
    // Null
    if (packedPosition == PACKED_POSITION_VALUE_NULL) return -1;

    return (int) ((packedPosition & PACKED_POSITION_MASK_GROUP) >> PACKED_POSITION_SHIFT_GROUP);
}

int ExpandableListView::getPackedPositionChild(long packedPosition) {
    // Null
    if (packedPosition == PACKED_POSITION_VALUE_NULL) return -1;

    // Group since a group type clears this bit
    if ((packedPosition & PACKED_POSITION_MASK_TYPE) != PACKED_POSITION_MASK_TYPE) return -1;

    return (int) (packedPosition & PACKED_POSITION_MASK_CHILD);
}

long ExpandableListView::getPackedPositionForChild(int groupPosition, int childPosition) {
    return (((long)PACKED_POSITION_TYPE_CHILD) << PACKED_POSITION_SHIFT_TYPE)
            | ((((long)groupPosition) & PACKED_POSITION_INT_MASK_GROUP)
                    << PACKED_POSITION_SHIFT_GROUP)
            | (childPosition & PACKED_POSITION_INT_MASK_CHILD);
}

long ExpandableListView::getPackedPositionForGroup(int groupPosition) {
     // No need to OR a type in because PACKED_POSITION_GROUP == 0
     return ((((long)groupPosition) & PACKED_POSITION_INT_MASK_GROUP)
                     << PACKED_POSITION_SHIFT_GROUP);
}

long ExpandableListView::getChildOrGroupId(const ExpandableListPosition& position) {
    if (position.type == ExpandableListPosition::CHILD) {
        return mAdapter->getChildId(position.groupPos, position.childPos);
    } else {
        return mAdapter->getGroupId(position.groupPos);
    }
}

void ExpandableListView::setChildIndicator(Drawable* childIndicator) {
    mChildIndicator = childIndicator;
}

void ExpandableListView::setChildIndicatorBounds(int left, int right) {
    mChildIndicatorLeft = left;
    mChildIndicatorRight = right;
    resolveChildIndicator();
}

void ExpandableListView::setChildIndicatorBoundsRelative(int start, int end) {
    mChildIndicatorStart = start;
    mChildIndicatorEnd = end;
    resolveChildIndicator();
}

void ExpandableListView::setGroupIndicator(Drawable* groupIndicator) {
    mGroupIndicator = groupIndicator;
    if (mIndicatorRight == 0 && mGroupIndicator ) {
        mIndicatorRight = mIndicatorLeft + mGroupIndicator->getIntrinsicWidth();
    }
}

void ExpandableListView::setIndicatorBounds(int left, int right) {
    mIndicatorLeft = left;
    mIndicatorRight = right;
    resolveIndicator();
}

void ExpandableListView::setIndicatorBoundsRelative(int start, int end) {
    mIndicatorStart = start;
    mIndicatorEnd = end;
    resolveIndicator();
}

}/*endof namespace*/

