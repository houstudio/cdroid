#ifndef __BASEEXPANDABLE_LISTADAPTER_H__
#define __BASEEXPANDABLE_LISTADAPTER_H__
#include <widget/adapter.h>
#include <widget/expandablelistadapter.h>

namespace cdroid{

class BaseExpandableListAdapter:virtual public ExpandableListAdapter{//virtual public HeterogeneousExpandableList{
private:
    //DataSetObservable* mDataSetObservable;
    std::vector<DataSetObserver*>mDataObservers;
public:
    void registerDataSetObserver(DataSetObserver* observer)override;
    void unregisterDataSetObserver(DataSetObserver* observer)override;
    virtual void notifyDataSetInvalidated();
    virtual void notifyDataSetChanged();
    bool areAllItemsEnabled(){
        return true;
    }

    void onGroupCollapsed(int groupPosition)override{
    }

    void onGroupExpanded(int groupPosition)override{
    }

    int64_t getCombinedChildId(long groupId, long childId)override{
        return 0x8000000000000000L | (int64_t((groupId & 0x7FFFFFFF)) << 32) | (childId & 0xFFFFFFFF);
    }


    int64_t getCombinedGroupId(long groupId)override{
        return int64_t((groupId & 0x7FFFFFFF)) << 32;
    }

    bool isEmpty()override{
        return getGroupCount() == 0;
    }


    int getChildType(int groupPosition, int childPosition) override{
        return 0;
    }

    int getChildTypeCount()override {
        return 1;
    }


    int getGroupType(int groupPosition)override {
        return 0;
    }

    int getGroupTypeCount()override{
        return 1;
    }    
};

}/*endof namespace*/
#endif/*__BASEEXPANDABLE_LISTADAPTER_H__*/
