#ifndef __EXPANDABLE_LIST_POSITION_H__
#define __EXPANDABLE_LIST_POSITION_H__
#include <queue>
namespace cdroid{

class ExpandableListPosition {
private:
    static constexpr int MAX_POOL_SIZE = 5;
    static std::queue<ExpandableListPosition*> sPool;
private:
    void resetState();
    ExpandableListPosition();
    static ExpandableListPosition* getRecycledOrCreate();
public:
    /* This data type represents a child position */
    static constexpr int CHILD = 1;

    /* This data type represents a group position */
    static constexpr int GROUP = 2;

    /* The position of either the group being referred to, or the parent
     * group of the child being referred to */
    int groupPos;

    /* The position of the child within its parent group */
    int childPos;

    /* The position of the item in the flat list (optional, used internally when
     * the corresponding flat list position for the group or child is known) */
    int flatListPos;

    /* What type of position this ExpandableListPosition represents */
    int type;
public:
    long getPackedPosition();
    static ExpandableListPosition* obtainGroupPosition(int groupPosition);
    static ExpandableListPosition* obtainChildPosition(int groupPosition, int childPosition);
    static ExpandableListPosition* obtainPosition(long packedPosition);
    static ExpandableListPosition* obtain(int type, int groupPos, int childPos, int flatListPos);
    void recycle()const;
};

}/*endof namespace*/
#endif/*__EXPANDABLE_LIST_POSITION_H__*/
