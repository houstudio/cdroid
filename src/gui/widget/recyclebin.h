/*************************************************************************
	> File Name: recyclebin.h
	> Author: 
	> Mail: 
	> Created Time: Thu 18 Mar 2021 01:46:27 PM UTC
 ************************************************************************/

#ifndef __RECYCLEBIN_H__
#define __RECYCLEBIN_H__
#include <view/view.h>
#include <core/sparsearray.h>
namespace cdroid{

class RecycleBin{
public:
    DECLARE_UIEVENT(void,RecyclerListener,View&);
    RecyclerListener mRecyclerListener;
private:
    class AbsListView*LV;
    int mFirstActivePosition;
    std::vector<View*>mActiveViews;
    std::vector<std::vector<View*>>mScrapViews;
    std::vector<View*>*mCurrentScrap;
    int mViewTypeCount;
    std::vector<View*>mSkippedScrap;
    SparseArray<View*>mTransientStateViews;
    SparseArray<View*>mTransientStateViewsById;

    class Adapter*getAdapter();//refto abslistview's mAdapter;
    std::vector<View*>getSkippedScrap();
    void pruneScrapViews();
    View* retrieveFromScrap(std::vector<View*>& scrapViews, int position);
    void clearScrap(std::vector<View*>& scrap);
    void clearScrapForRebind(View* view);
    void removeDetachedView(View* child, bool animate) ;
public:
    RecycleBin(AbsListView*);
    void setViewTypeCount(int viewTypeCount);
    void markChildrenDirty();
    bool shouldRecycleViewType(int viewType) {
        return viewType >= 0;
    }
    void clear();
    void fillActiveViews(int childCount, int firstActivePosition) ;
    View* getActiveView(int position);
    View* getTransientStateView(int position);
    void clearTransientStateViews();
    View* getScrapView(int position);
    void addScrapView(View* scrap, int position);
    void removeSkippedScrap();
    void scrapActiveViews();
    void fullyDetachScrapViews();
    void reclaimScrapViews(std::vector<View*>& views) ;
    void setCacheColorHint(int color);
};
}//namspace 
#endif
