#ifndef __EXPANDABLE_LISTVIEW_H__
#define __EXPANDABLE_LISTVIEW_H__
#include <widget/listview.h>
#include <widget/expandablelistconnector.h>
#include <widget/expandablelistposition.h>
#include <widget/baseexpandablelistadapter.h>

namespace cdroid{

class ExpandableListView:public ListView{
private:
    static constexpr long PACKED_POSITION_MASK_CHILD = 0x00000000FFFFFFFFL;

    /** The mask (in packed position representation) for the group */
    static constexpr long PACKED_POSITION_MASK_GROUP = 0x7FFFFFFF00000000L;

    /** The mask (in packed position representation) for the type */
    static constexpr long PACKED_POSITION_MASK_TYPE  = 0x8000000000000000L;

    /** The shift amount (in packed position representation) for the group */
    static constexpr long PACKED_POSITION_SHIFT_GROUP = 32;

    /** The shift amount (in packed position representation) for the type */
    static constexpr long PACKED_POSITION_SHIFT_TYPE  = 63;

    /** The mask (in integer child position representation) for the child */
    static constexpr long PACKED_POSITION_INT_MASK_CHILD = 0xFFFFFFFF;

    /** The mask (in integer group position representation) for the group */
    static constexpr long PACKED_POSITION_INT_MASK_GROUP = 0x7FFFFFFF;
    /* Denotes an undefined value for an indicator */
    static constexpr int INDICATOR_UNDEFINED = -2;    
public:
    /* The packed position represents a group. */
    static constexpr int PACKED_POSITION_TYPE_GROUP = 0;

    /* The packed position represents a child. */
    static constexpr int PACKED_POSITION_TYPE_CHILD = 1;

    /* The packed position represents a neither/null/no preference. */
    static constexpr int PACKED_POSITION_TYPE_NULL = 2;

    /* The value for a packed position that represents neither/null/no
     * preference. This value is not otherwise possible since a group type
     * (first bit 0) should not have a child position filled. */
    static constexpr long PACKED_POSITION_VALUE_NULL = 0x00000000FFFFFFFFL;
    /* Denotes when a child indicator should inherit this bound from the generic
     * indicator bounds */
    static constexpr int CHILD_INDICATOR_INHERIT = -1;
    DECLARE_UIEVENT(void,OnGroupExpandListener,int);
    DECLARE_UIEVENT(void,OnGroupCollapseListener,int);
    DECLARE_UIEVENT(bool,OnGroupClickListener,ExpandableListView& parent, View& v, int groupPosition, long id);
    DECLARE_UIEVENT(bool,OnChildClickListener,ExpandableListView& parent, View& v, int groupPosition, int childPosition, long id);
private:
    int mIndicatorLeft;

    /** Right bound for drawing the indicator. */
    int mIndicatorRight;

    /** Start bound for drawing the indicator. */
    int mIndicatorStart;

    /** End bound for drawing the indicator. */
    int mIndicatorEnd;

    /* Left bound for drawing the indicator of a child. Value of
     * {@link #CHILD_INDICATOR_INHERIT} means use mIndicatorLeft.*/
    int mChildIndicatorLeft;

    /* Right bound for drawing the indicator of a child. Value of
     * {@link #CHILD_INDICATOR_INHERIT} means use mIndicatorRight. */
    int mChildIndicatorRight;

    /* Start bound for drawing the indicator of a child. Value of
     * {@link #CHILD_INDICATOR_INHERIT} means use mIndicatorStart.*/
    int mChildIndicatorStart;

    /* End bound for drawing the indicator of a child. Value of
     * {@link #CHILD_INDICATOR_INHERIT} means use mIndicatorEnd. */
    int mChildIndicatorEnd;    

    ExpandableListConnector* mConnector;
    ExpandableListAdapter* mAdapter;
    /** The indicator drawn next to a group. */
    Drawable* mGroupIndicator;

    /** The indicator drawn next to a child. */
    Drawable* mChildIndicator;
    /** Drawable to be used as a divider when it is adjacent to any children */
    Drawable* mChildDivider;
    OnGroupExpandListener mOnGroupExpandListener;
    OnGroupCollapseListener mOnGroupCollapseListener;
    OnGroupClickListener mOnGroupClickListener;
    OnChildClickListener mOnChildClickListener;
private:
    void initView();
    bool isRtlCompatibilityMode()const;
    void resolveIndicator();
    void resolveChildIndicator();
    Drawable* getIndicator(ExpandableListConnector::PositionMetadata& pos);
    bool isHeaderOrFooterPosition(int position);
    int getFlatPositionForConnector(int flatListPosition);
    int getAbsoluteFlatPosition(int flatListPosition);
    long getChildOrGroupId(const ExpandableListPosition& position);
protected:
    void dispatchDraw(Canvas& canvas);
    void drawDivider(Canvas& canvas, Rect bounds, int childIndex);
    bool handleItemClick(View& v, int position, long id);
public:
    ExpandableListView(int,int);
    ExpandableListView(Context* context,const AttributeSet& attrs);
    ~ExpandableListView();
    void setAdapter(ListAdapter* adapter);
    void setAdapter(ExpandableListAdapter* adapter);
    ExpandableListAdapter* getExpandableListAdapter();
    void onRtlPropertiesChanged(int layoutDirection);
    void setChildDivider(Drawable* childDivider);
    bool performItemClick(View& v, int position, long id)override;
    bool expandGroup(int groupPos);
    bool expandGroup(int groupPos, bool animate);
    bool collapseGroup(int groupPos);
    void setOnGroupCollapseListener(OnGroupCollapseListener onGroupCollapseListener);
    void setOnGroupExpandListener( OnGroupExpandListener onGroupExpandListener);
    void setOnGroupClickListener(OnGroupClickListener onGroupClickListener);
    void setOnChildClickListener(OnChildClickListener onChildClickListener);
    long getExpandableListPosition(int flatListPosition);
    int getFlatListPosition(long packedPosition);
    long getSelectedPosition();
    long getSelectedId();
    void setSelectedGroup(int groupPosition);
    bool setSelectedChild(int groupPosition, int childPosition, bool shouldExpandGroup);
    bool isGroupExpanded(int groupPosition);

    static int getPackedPositionType(long packedPosition);
    static int getPackedPositionGroup(long packedPosition);
    static int getPackedPositionChild(long packedPosition);
    static long getPackedPositionForChild(int groupPosition, int childPosition);
    static long getPackedPositionForGroup(int groupPosition);

    void setChildIndicator(Drawable* childIndicator);
    void setChildIndicatorBounds(int left, int right);
    void setChildIndicatorBoundsRelative(int start, int end);
    void setGroupIndicator(Drawable* groupIndicator);
    void setIndicatorBounds(int left, int right);
    void setIndicatorBoundsRelative(int start, int end);
};
}/*endof namespace*/
#endif
