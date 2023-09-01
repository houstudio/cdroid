#include <widget/expandablelistconnector.h>
#include <widget/adapterview.h>

namespace cdroid{

ExpandableListConnector::ExpandableListConnector(ExpandableListAdapter* expandableListAdapter){
    mExpandableListAdapter = nullptr;
    mDataSetObserver =new MyDataSetObserver(this); 
    setExpandableListAdapter(expandableListAdapter);
}

void ExpandableListConnector::setExpandableListAdapter(ExpandableListAdapter* expandableListAdapter) {
    if (mExpandableListAdapter ) {
        mExpandableListAdapter->unregisterDataSetObserver(mDataSetObserver);
    }
    mExpandableListAdapter = expandableListAdapter;
    expandableListAdapter->registerDataSetObserver(mDataSetObserver);
}

ExpandableListConnector::PositionMetadata* ExpandableListConnector::getUnflattenedPos(int flPos) const{
    /* Keep locally since frequent use */
    const std::vector<GroupMetadata*>& egml = mExpGroupMetadataList;
    const int numExpGroups = egml.size();

    /* Binary search variables */
    int leftExpGroupIndex = 0;
    int rightExpGroupIndex = numExpGroups - 1;
    int midExpGroupIndex = 0;
    GroupMetadata* midExpGm;

    if (numExpGroups == 0) {
        return PositionMetadata::obtain(flPos, ExpandableListPosition::GROUP, flPos, -1, nullptr, 0);
    }

    while (leftExpGroupIndex <= rightExpGroupIndex) {
        midExpGroupIndex = (rightExpGroupIndex - leftExpGroupIndex) / 2 + leftExpGroupIndex;
        midExpGm = egml.at(midExpGroupIndex);

        if (flPos > midExpGm->lastChildFlPos) {
            leftExpGroupIndex = midExpGroupIndex + 1;
        } else if (flPos < midExpGm->flPos) {
            rightExpGroupIndex = midExpGroupIndex - 1;
        } else if (flPos == midExpGm->flPos) {
            return PositionMetadata::obtain(flPos, ExpandableListPosition::GROUP,
                    midExpGm->gPos, -1, midExpGm, midExpGroupIndex);
        } else if (flPos <= midExpGm->lastChildFlPos
                /* && flPos > midGm.flPos as deduced from previous
                 * conditions */) {
            const int childPos = flPos - (midExpGm->flPos + 1);
            return PositionMetadata::obtain(flPos, ExpandableListPosition::CHILD,
                    midExpGm->gPos, childPos, midExpGm, midExpGroupIndex);
        }
    }

    int insertPosition = 0;

    /** What is its group position in the list of all groups? */
    int groupPos = 0;

    if (leftExpGroupIndex > midExpGroupIndex) {
        const GroupMetadata* leftExpGm = egml.at(leftExpGroupIndex-1);
        insertPosition = leftExpGroupIndex;
        groupPos = (flPos - leftExpGm->lastChildFlPos) + leftExpGm->gPos;
    } else if (rightExpGroupIndex < midExpGroupIndex) {
        const GroupMetadata* rightExpGm = egml.at(++rightExpGroupIndex);
        insertPosition = rightExpGroupIndex;

        groupPos = rightExpGm->gPos - (rightExpGm->flPos - flPos);
    } else {
        // TODO: clean exit
        FATAL("Unknown state");
    }
    return PositionMetadata::obtain(flPos, ExpandableListPosition::GROUP, groupPos, -1,
            nullptr, insertPosition);
}

ExpandableListConnector::PositionMetadata* ExpandableListConnector::getFlattenedPos(const ExpandableListPosition& pos) {
    const std::vector<GroupMetadata*>& egml = mExpGroupMetadataList;
    const int numExpGroups = egml.size();

    /* Binary search variables */
    int leftExpGroupIndex = 0;
    int rightExpGroupIndex = numExpGroups - 1;
    int midExpGroupIndex = 0;
    GroupMetadata* midExpGm;

    if (numExpGroups == 0) {
        return PositionMetadata::obtain(pos.groupPos, pos.type, pos.groupPos, pos.childPos, nullptr, 0);
    }

    while (leftExpGroupIndex <= rightExpGroupIndex) {
        midExpGroupIndex = (rightExpGroupIndex - leftExpGroupIndex)/2 + leftExpGroupIndex;
        midExpGm = egml.at(midExpGroupIndex);

        if (pos.groupPos > midExpGm->gPos) {
            leftExpGroupIndex = midExpGroupIndex + 1;
        } else if (pos.groupPos < midExpGm->gPos) {
            rightExpGroupIndex = midExpGroupIndex - 1;
        } else if (pos.groupPos == midExpGm->gPos) {
            if (pos.type == ExpandableListPosition::GROUP) {
                /* If it's a group, give them this matched group's flPos */
                return PositionMetadata::obtain(midExpGm->flPos, pos.type,
                        pos.groupPos, pos.childPos, midExpGm, midExpGroupIndex);
            } else if (pos.type == ExpandableListPosition::CHILD) {
                /* If it's a child, calculate the flat list pos */
                return PositionMetadata::obtain(midExpGm->flPos + pos.childPos
                        + 1, pos.type, pos.groupPos, pos.childPos,
                        midExpGm, midExpGroupIndex);
            } else {
                return nullptr;
            }
        }
    }

    if (pos.type != ExpandableListPosition::GROUP) {
        /* If it isn't a group, return null */
        return nullptr;
    }

    if (leftExpGroupIndex > midExpGroupIndex) {

        const GroupMetadata* leftExpGm = egml.at(leftExpGroupIndex-1);
        const int flPos = leftExpGm->lastChildFlPos + (pos.groupPos - leftExpGm->gPos);

        return PositionMetadata::obtain(flPos, pos.type, pos.groupPos,
                pos.childPos, nullptr, leftExpGroupIndex);
    } else if (rightExpGroupIndex < midExpGroupIndex) {
        const GroupMetadata* rightExpGm = egml.at(++rightExpGroupIndex);
        const int flPos = rightExpGm->flPos - (rightExpGm->gPos - pos.groupPos);
        return PositionMetadata::obtain(flPos, pos.type, pos.groupPos,
                pos.childPos, nullptr, rightExpGroupIndex);
    } else {
        return nullptr;
    }
}

bool ExpandableListConnector::areAllItemsEnabled() {
    return mExpandableListAdapter->areAllItemsEnabled();
}

bool ExpandableListConnector::isEnabled(int flatListPos) {
    PositionMetadata* metadata = getUnflattenedPos(flatListPos);
    ExpandableListPosition* pos = metadata->position;

    bool retValue;
    if (pos->type == ExpandableListPosition::CHILD) {
        retValue = mExpandableListAdapter->isChildSelectable(pos->groupPos, pos->childPos);
    } else {
        // Groups are always selectable
        retValue = true;
    }

    metadata->recycle();
    return retValue;
}

int ExpandableListConnector::getCount()const{
    /* Total count for the list view is the number groups plus the
     * number of children from currently expanded groups (a value we keep
     * cached in this class)*/
    return mExpandableListAdapter->getGroupCount() + mTotalExpChildrenCount;
}

void* ExpandableListConnector::getItem(int flatListPos) const{
    PositionMetadata* posMetadata = getUnflattenedPos(flatListPos);

    void* retValue;/*Object*/
    if (posMetadata->position->type == ExpandableListPosition::GROUP) {
        retValue = mExpandableListAdapter->getGroup(posMetadata->position->groupPos);
    } else if (posMetadata->position->type == ExpandableListPosition::CHILD) {
        retValue = mExpandableListAdapter->getChild(posMetadata->position->groupPos,
                posMetadata->position->childPos);
    } else {
        // TODO: clean exit
        FATAL("Flat list position is of unknown type");
    }

    posMetadata->recycle();

    return retValue;
}

long ExpandableListConnector::getItemId(int flatListPos) {
    PositionMetadata* posMetadata = getUnflattenedPos(flatListPos);
    const long groupId = mExpandableListAdapter->getGroupId(posMetadata->position->groupPos);

    long retValue;
    if (posMetadata->position->type == ExpandableListPosition::GROUP) {
        retValue = mExpandableListAdapter->getCombinedGroupId(groupId);
    } else if (posMetadata->position->type == ExpandableListPosition::CHILD) {
        const long childId = mExpandableListAdapter->getChildId(posMetadata->position->groupPos,
                posMetadata->position->childPos);
        retValue = mExpandableListAdapter->getCombinedChildId(groupId, childId);
    } else {
        // TODO: clean exit
        FATAL("Flat list position is of unknown type");
    }

    posMetadata->recycle();

    return retValue;
}

View* ExpandableListConnector::getView(int flatListPos, View* convertView, ViewGroup* parent){
    View* retValue;
    PositionMetadata* posMetadata = getUnflattenedPos(flatListPos);
    if (posMetadata->position->type == ExpandableListPosition::GROUP) {
        retValue = mExpandableListAdapter->getGroupView(posMetadata->position->groupPos,
                posMetadata->isExpanded(), convertView, parent);
    } else if (posMetadata->position->type == ExpandableListPosition::CHILD) {
        const bool isLastChild = posMetadata->groupMetadata->lastChildFlPos == flatListPos;
        retValue = mExpandableListAdapter->getChildView(posMetadata->position->groupPos,
                    posMetadata->position->childPos, isLastChild, convertView, parent);
    } else {
         // TODO: clean exit
         FATAL("Flat list position is of unknown type");
    }
    posMetadata->recycle();
    return retValue;
}

int ExpandableListConnector::getItemViewType(int flatListPos)const {
    PositionMetadata* metadata = getUnflattenedPos(flatListPos);
    const ExpandableListPosition* pos = metadata->position;

    int retValue;
    auto adapter = mExpandableListAdapter;
    if (pos->type == ExpandableListPosition::GROUP) {
        retValue = adapter->getGroupType(pos->groupPos);
    } else {
        const int childType = adapter->getChildType(pos->groupPos, pos->childPos);
        retValue = adapter->getGroupTypeCount() + childType;
    }
    metadata->recycle();

    return retValue;
}

int ExpandableListConnector::getViewTypeCount()const{
    auto adapter = mExpandableListAdapter;
    return adapter->getGroupTypeCount() + adapter->getChildTypeCount();
}

bool ExpandableListConnector::hasStableIds() const{
    return mExpandableListAdapter->hasStableIds();
}

void ExpandableListConnector::refreshExpGroupMetadataList(bool forceChildrenCountRefresh, bool syncGroupPositions){
    std::vector<GroupMetadata*>& egml = mExpGroupMetadataList;
    int egmlSize = egml.size();
    int curFlPos = 0;
    
    /* Update child count as we go through */
    mTotalExpChildrenCount = 0;
    
    if (syncGroupPositions) {
        // We need to check whether any groups have moved positions
        bool positionsChanged = false;
    
        for (int i = egmlSize - 1; i >= 0; i--) {
            GroupMetadata* curGm = egml.at(i);
            int newGPos = findGroupPosition(curGm->gId, curGm->gPos);
            if (newGPos != curGm->gPos) {
                if (newGPos == AdapterView::INVALID_POSITION) {
                    // Doh, just remove it from the list of expanded groups
                    egml.erase(egml.begin()+i);//remove(i);
                    egmlSize--;
                }
    
                curGm->gPos = newGPos;
                if (!positionsChanged) positionsChanged = true;
            }
        }
    
        if (positionsChanged) {
            // At least one group changed positions, so re-sort
            //Collections.sort(egml);
	    std::sort(egml.begin(),egml.end(), [](GroupMetadata*lhs,GroupMetadata*rhs)->int{
                return lhs->gPos-rhs->gPos;
	    });
        }
    }
    
    int gChildrenCount;
    int lastGPos = 0;
    for (int i = 0; i < egmlSize; i++) {
        /* Store in local variable since we'll access freq */
        GroupMetadata* curGm = egml.at(i);
    
        /* Get the number of children, try to refrain from calling
         * another class's method unless we have to (so do a subtraction) */
        if ((curGm->lastChildFlPos == GroupMetadata::REFRESH) || forceChildrenCountRefresh) {
            gChildrenCount = mExpandableListAdapter->getChildrenCount(curGm->gPos);
        } else {
            /* Num children for this group is its last child's fl pos minus
             * the group's fl pos */
            gChildrenCount = curGm->lastChildFlPos - curGm->flPos;
        }
    
        /* Update */
        mTotalExpChildrenCount += gChildrenCount;
    
        curFlPos += (curGm->gPos - lastGPos);
        lastGPos = curGm->gPos;
    
        /* Update the flat list positions, and the current flat list pos */
        curGm->flPos = curFlPos;
        curFlPos += gChildrenCount;
        curGm->lastChildFlPos = curFlPos;
    }
}

bool ExpandableListConnector::collapseGroup(int groupPos) {
    ExpandableListPosition* elGroupPos = ExpandableListPosition::obtain(
            ExpandableListPosition::GROUP, groupPos, -1, -1);
    PositionMetadata* pm = getFlattenedPos(*elGroupPos);
    elGroupPos->recycle();
    if (pm == nullptr) return false;

    bool retValue = collapseGroup(*pm);
    pm->recycle();
    return retValue;
}

bool ExpandableListConnector::collapseGroup(PositionMetadata& posMetadata) {
    /*
     * Collapsing requires removal from mExpGroupMetadataList
     */

    /*
     * If it is null, it must be already collapsed. This group metadata
     * object should have been set from the search that returned the
     * position metadata object.
     */
    if (posMetadata.groupMetadata == nullptr) return false;

    // Remove the group from the list of expanded groups
    auto it = std::find(mExpGroupMetadataList.begin(),mExpGroupMetadataList.end(),posMetadata.groupMetadata);
    mExpGroupMetadataList.erase(it);//remove(posMetadata.groupMetadata);

    // Refresh the metadata
    refreshExpGroupMetadataList(false, false);

    // Notify of change
    notifyDataSetChanged();

    // Give the callback
    mExpandableListAdapter->onGroupCollapsed(posMetadata.groupMetadata->gPos);

    return true;
}

bool ExpandableListConnector::expandGroup(int groupPos) {
    ExpandableListPosition* elGroupPos = ExpandableListPosition::obtain(
            ExpandableListPosition::GROUP, groupPos, -1, -1);
    PositionMetadata* pm = getFlattenedPos(*elGroupPos);
    elGroupPos->recycle();
    const bool retValue = expandGroup(*pm);
    pm->recycle();
    return retValue;
}

bool ExpandableListConnector::expandGroup(PositionMetadata& posMetadata) {
    /*
     * Expanding requires insertion into the mExpGroupMetadataList
     */

    if (posMetadata.position->groupPos < 0) {
        // TODO clean exit
        FATAL("Need group");
    }

    if (mMaxExpGroupCount == 0) return false;

    // Check to see if it's already expanded
    if (posMetadata.groupMetadata != nullptr) return false;

    /* Restrict number of expanded groups to mMaxExpGroupCount */
    if (mExpGroupMetadataList.size() >= mMaxExpGroupCount) {
        /* Collapse a group */
        // TODO: Collapse something not on the screen instead of the first one?
        // TODO: Could write overloaded function to take GroupMetadata to collapse
        GroupMetadata* collapsedGm = mExpGroupMetadataList.at(0);

	auto it = std::find(mExpGroupMetadataList.begin(),mExpGroupMetadataList.end(),collapsedGm);
        int collapsedIndex = it - mExpGroupMetadataList.begin();//mExpGroupMetadataList.indexOf(collapsedGm);

        collapseGroup(collapsedGm->gPos);

        /* Decrement index if it is after the group we removed */
        if (posMetadata.groupInsertIndex > collapsedIndex) {
            posMetadata.groupInsertIndex--;
        }
    }

    GroupMetadata* expandedGm = GroupMetadata::obtain(GroupMetadata::REFRESH,GroupMetadata::REFRESH,
            posMetadata.position->groupPos, mExpandableListAdapter->getGroupId(posMetadata.position->groupPos));

    mExpGroupMetadataList.insert(mExpGroupMetadataList.begin() + posMetadata.groupInsertIndex, expandedGm);

    // Refresh the metadata
    refreshExpGroupMetadataList(false, false);

    // Notify of change
    notifyDataSetChanged();

    // Give the callback
    mExpandableListAdapter->onGroupExpanded(expandedGm->gPos);

    return true;
}

bool ExpandableListConnector::isGroupExpanded(int groupPosition) {
    GroupMetadata* groupMetadata;
    for (int i = mExpGroupMetadataList.size() - 1; i >= 0; i--) {
        groupMetadata = mExpGroupMetadataList.at(i);
        if (groupMetadata->gPos == groupPosition) {
            return true;
        }
    }
    return false;
}

void ExpandableListConnector::setMaxExpGroupCount(int maxExpGroupCount) {
    mMaxExpGroupCount = maxExpGroupCount;
}

ExpandableListAdapter* ExpandableListConnector::getAdapter() {
    return mExpandableListAdapter;
}

Filter* ExpandableListConnector::getFilter() {
    ExpandableListAdapter* adapter = getAdapter();
    if (dynamic_cast<Filterable*>(adapter)) {
        return ((Filterable*) adapter)->getFilter();
    } else {
        return nullptr;
    }
}

std::vector<ExpandableListConnector::GroupMetadata*>& ExpandableListConnector::getExpandedGroupMetadataList() {
    return mExpGroupMetadataList;
}

void ExpandableListConnector::setExpandedGroupMetadataList(std::vector<GroupMetadata*>& expandedGroupMetadataList) {

    if (mExpandableListAdapter == nullptr) {
        return;
    }

    // Make sure our current data set is big enough for the previously
    // expanded groups, if not, ignore this request
    const int numGroups = mExpandableListAdapter->getGroupCount();
    for (int i = expandedGroupMetadataList.size() - 1; i >= 0; i--) {
        if (expandedGroupMetadataList.at(i)->gPos >= numGroups) {
            // Doh, for some reason the client doesn't have some of the groups
            return;
        }
    }

    mExpGroupMetadataList = expandedGroupMetadataList;
    refreshExpGroupMetadataList(true, false);
}

bool ExpandableListConnector::isEmpty() {
    ExpandableListAdapter* adapter = getAdapter();
    return adapter ? adapter->isEmpty() : true;
}

int ExpandableListConnector::findGroupPosition(long groupIdToMatch, int seedGroupPosition) {
    const int count = mExpandableListAdapter->getGroupCount();

    if (count == 0) {
        return AdapterView::INVALID_POSITION;
    }

    // If there isn't a selection don't hunt for it
    if (groupIdToMatch == AdapterView::INVALID_ROW_ID) {
        return AdapterView::INVALID_POSITION;
    }

    // Pin seed to reasonable values
    seedGroupPosition = std::max(0, seedGroupPosition);
    seedGroupPosition = std::min(count - 1, seedGroupPosition);

    long endTime = SystemClock::uptimeMillis() + AdapterView::SYNC_MAX_DURATION_MILLIS;

    long rowId;

    // first position scanned so far
    int first = seedGroupPosition;

    // last position scanned so far
    int last = seedGroupPosition;

    // True if we should move down on the next iteration
    bool next = false;

    // True when we have looked at the first item in the data
    bool hitFirst;

    // True when we have looked at the last item in the data
    bool hitLast;

    // Get the item ID locally (instead of getItemIdAtPosition), so
    // we need the adapter
    ExpandableListAdapter* adapter = getAdapter();
    if (adapter == nullptr) {
        return AdapterView::INVALID_POSITION;
    }

    while (SystemClock::uptimeMillis() <= endTime) {
        rowId = adapter->getGroupId(seedGroupPosition);
        if (rowId == groupIdToMatch) {
            // Found it!
            return seedGroupPosition;
        }

        hitLast = last == count - 1;
        hitFirst = first == 0;

        if (hitLast && hitFirst) {
            // Looked at everything
            break;
        }

        if (hitFirst || (next && !hitLast)) {
            // Either we hit the top, or we are trying to move down
            last++;
            seedGroupPosition = last;
            // Try going up next time
            next = false;
        } else if (hitLast || (!next && !hitFirst)) {
            // Either we hit the bottom, or we are trying to move up
            first--;
            seedGroupPosition = first;
            // Try going down next time
            next = true;
        }

    }

    return AdapterView::INVALID_POSITION;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

std::queue<ExpandableListConnector::PositionMetadata*> ExpandableListConnector::PositionMetadata::sPool;

ExpandableListConnector::GroupMetadata* 
        ExpandableListConnector::GroupMetadata::obtain(int flPos, int lastChildFlPos, int gPos, long gId) {
    GroupMetadata* gm = new GroupMetadata();
    gm->flPos = flPos;
    gm->lastChildFlPos = lastChildFlPos;
    gm->gPos = gPos;
    gm->gId = gId;
    return gm;
}

void ExpandableListConnector::PositionMetadata::resetState() {
    if (position != nullptr) {
        position->recycle();
        position = nullptr;
    }
    groupMetadata = nullptr;
    groupInsertIndex = 0;
}

ExpandableListConnector::PositionMetadata::PositionMetadata() {
}

ExpandableListConnector::PositionMetadata*
        ExpandableListConnector::PositionMetadata::obtain(int flatListPos, int type, int groupPos,
            int childPos, GroupMetadata* groupMetadata, int groupInsertIndex) {
    PositionMetadata* pm = getRecycledOrCreate();
    pm->position = ExpandableListPosition::obtain(type, groupPos, childPos, flatListPos);
    pm->groupMetadata = groupMetadata;
    pm->groupInsertIndex = groupInsertIndex;
    return pm;
}

ExpandableListConnector::PositionMetadata* ExpandableListConnector::PositionMetadata::getRecycledOrCreate() {
    PositionMetadata* pm;
    if (sPool.size() > 0) {
        pm = sPool.front();
	sPool.pop();
    } else {
        return new PositionMetadata();
    }
    pm->resetState();
    return pm;
}

void ExpandableListConnector::PositionMetadata::recycle(){
    resetState();
    if (sPool.size() < MAX_POOL_SIZE) {
        sPool.push((PositionMetadata*)this);
    }
}

bool ExpandableListConnector::PositionMetadata::isExpanded() const{
    return groupMetadata != nullptr;
}

}/*endof namespace*/
