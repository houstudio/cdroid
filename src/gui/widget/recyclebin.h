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
    size_t mViewTypeCount;
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
}/*endof namspace*/
#endif
