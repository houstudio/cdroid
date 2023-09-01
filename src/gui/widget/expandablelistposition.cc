#include <widget/expandablelistposition.h>
#include <widget/expandablelistview.h>
namespace cdroid{

std::queue<ExpandableListPosition*> ExpandableListPosition::sPool;

void ExpandableListPosition::resetState() {
    groupPos = 0;
    childPos = 0;
    flatListPos = 0;
    type = 0;
}

ExpandableListPosition::ExpandableListPosition() {
}

long ExpandableListPosition::getPackedPosition() {
    if (type == CHILD) return ExpandableListView::getPackedPositionForChild(groupPos, childPos);
    else return ExpandableListView::getPackedPositionForGroup(groupPos);
}

ExpandableListPosition* ExpandableListPosition::obtainGroupPosition(int groupPosition) {
    return obtain(GROUP, groupPosition, 0, 0);
}

ExpandableListPosition* ExpandableListPosition::obtainChildPosition(int groupPosition, int childPosition) {
    return obtain(CHILD, groupPosition, childPosition, 0);
}

ExpandableListPosition* ExpandableListPosition::obtainPosition(long packedPosition) {
    if (packedPosition == ExpandableListView::PACKED_POSITION_VALUE_NULL) {
        return nullptr;
    }

    ExpandableListPosition* elp = getRecycledOrCreate();
    elp->groupPos = ExpandableListView::getPackedPositionGroup(packedPosition);
    if (ExpandableListView::getPackedPositionType(packedPosition) ==
            ExpandableListView::PACKED_POSITION_TYPE_CHILD) {
        elp->type = CHILD;
        elp->childPos = ExpandableListView::getPackedPositionChild(packedPosition);
    } else {
        elp->type = GROUP;
    }
    return elp;
}

ExpandableListPosition* ExpandableListPosition::obtain(int type, int groupPos, int childPos, int flatListPos) {
    ExpandableListPosition* elp = getRecycledOrCreate();
    elp->type = type;
    elp->groupPos = groupPos;
    elp->childPos = childPos;
    elp->flatListPos = flatListPos;
    return elp;
}

ExpandableListPosition* ExpandableListPosition::getRecycledOrCreate() {
    ExpandableListPosition* elp;
    if (sPool.size() > 0) {
        elp = sPool.front();
	sPool.pop();
    } else {
        return new ExpandableListPosition();
    }
    elp->resetState();
    return elp;
}

/**
 * Do not call this unless you obtained this via ExpandableListPosition.obtain().
 * PositionMetadata will handle recycling its own children.
 */
void ExpandableListPosition::recycle()const{
    if (sPool.size() < MAX_POOL_SIZE) {
        sPool.push((ExpandableListPosition*)this);
    }
}
}
