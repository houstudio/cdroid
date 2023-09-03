#ifndef __EXPANDABLE_LIST_CONNECTIOR_H__
#define __EXPANDABLE_LIST_CONNECTIOR_H__
#include <queue>
#include <widget/adapter.h>
#include <widget/filterable.h>
#include <widget/expandablelistposition.h>
#include <widget/expandablelistadapter.h>

namespace cdroid{
class ExpandableListConnector:public BaseAdapter,public Filterable{
protected:
    class MyDataSetObserver:public DataSetObserver {
	ExpandableListConnector*mConnector;
    public:
	MyDataSetObserver(ExpandableListConnector*connector){
	    mConnector = connector;
	}
	void onChanged() {
            mConnector->refreshExpGroupMetadataList(true, true);
            mConnector->notifyDataSetChanged();
        }
        void onInvalidated() {
            mConnector->refreshExpGroupMetadataList(true, true);
            mConnector->notifyDataSetInvalidated();
        }
	void clearSavedState()override{}
    };
public:
    class GroupMetadata{
    public:
	static constexpr int REFRESH = -1;
	/** This group's flat list position */
        int flPos;
        /* firstChildFlPos isn't needed since it's (flPos + 1) */
        /* This group's last child's flat list position, so basically
         * the range of this group in the flat list */
        int lastChildFlPos;
        /* This group's group position */
        int gPos;
        /* This group's id */
        long gId;
    public:
	static GroupMetadata* obtain(int flPos, int lastChildFlPos, int gPos, long gId);
    };
    class PositionMetadata {
    private:
	static constexpr int MAX_POOL_SIZE = 5;
        static std::queue<PositionMetadata*> sPool;
        /** Data type to hold the position and its type (child/group) */
	void resetState();
	PositionMetadata();
    public:
	ExpandableListPosition* position;
        /* Link back to the expanded GroupMetadata for this group. Useful for
         * removing the group from the list of expanded groups inside the
         * connector when we collapse the group, and also as a check to see if
         * the group was expanded or collapsed (this will be null if the group
         * is collapsed since we don't keep that group's metadata)*/
        GroupMetadata* groupMetadata;

        /* For groups that are collapsed, we use this as the index (in
         * mExpGroupMetadataList) to insert this group when we are expanding
         * this group.*/
        int groupInsertIndex;
	static PositionMetadata* obtain(int flatListPos, int type, int groupPos,
                int childPos, GroupMetadata* groupMetadata, int groupInsertIndex);
	static PositionMetadata* getRecycledOrCreate() ;
	void recycle();
	bool isExpanded()const;
    };
private:
    ExpandableListAdapter* mExpandableListAdapter;
    /**
     * List of metadata for the currently expanded groups. The metadata consists
     * of data essential for efficiently translating between flat list positions
     * and group/child positions. See {@link GroupMetadata}.
     */
    std::vector<GroupMetadata*> mExpGroupMetadataList;

    /** The number of children from all currently expanded groups */
    int mTotalExpChildrenCount;

    /** The maximum number of allowable expanded groups. Defaults to 'no limit' */
    int mMaxExpGroupCount = INT_MAX;

    /** Change observer used to have ExpandableListAdapter changes pushed to us */
    DataSetObserver* mDataSetObserver;
private:
    void refreshExpGroupMetadataList(bool forceChildrenCountRefresh, bool syncGroupPositions);
public:
    ExpandableListConnector(ExpandableListAdapter* expandableListAdapter);
    ~ExpandableListConnector();
    void setExpandableListAdapter(ExpandableListAdapter* expandableListAdapter);
    PositionMetadata* getUnflattenedPos( int flPos)const;/*protected*/
    PositionMetadata* getFlattenedPos(const ExpandableListPosition& pos);/*proteced*/
    bool areAllItemsEnabled();
    bool isEnabled(int flatListPos);
    int getCount()const;
    void*getItem(int flatListPos)const;
    long getItemId(int flatListPos);
    View* getView(int flatListPos, View* convertView, ViewGroup* parent);
    int getItemViewType(int flatListPos)const override;
    int getViewTypeCount()const override;
    bool hasStableIds()const override;

    bool collapseGroup(int groupPos);
    bool collapseGroup(PositionMetadata& posMetadata);
    bool expandGroup(int groupPos);
    bool expandGroup(PositionMetadata& posMetadata);
    bool isGroupExpanded(int groupPosition);

    void setMaxExpGroupCount(int maxExpGroupCount);
    ExpandableListAdapter* getAdapter();
    Filter* getFilter();

    std::vector<GroupMetadata*>& getExpandedGroupMetadataList();
    void setExpandedGroupMetadataList(std::vector<GroupMetadata*>& expandedGroupMetadataList);
    bool isEmpty();
    int findGroupPosition(long groupIdToMatch, int seedGroupPosition);
};

}/*endof namespace*/
#endif
