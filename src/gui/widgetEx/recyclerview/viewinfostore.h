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
#ifndef __VIEW_INFO_STORE_H__
#define __VIEW_INFO_STORE_H__
#include <core/pools.h>
#include <map>

namespace cdroid{
class ViewInfoStore {
protected:
    class InfoRecord {
        static constexpr int FLAG_DISAPPEARED = 1;
        static constexpr int FLAG_APPEAR = 1 << 1;
        static constexpr int FLAG_PRE  = 1 << 2;
        static constexpr int FLAG_POST = 1 << 3;
        static constexpr int FLAG_APPEAR_AND_DISAPPEAR = FLAG_APPEAR | FLAG_DISAPPEARED;
        static constexpr int FLAG_PRE_AND_POST = FLAG_PRE | FLAG_POST;
        static constexpr int FLAG_APPEAR_PRE_AND_POST = FLAG_APPEAR | FLAG_PRE | FLAG_POST;
        int flags;
        RecyclerView::ItemAnimator::ItemHolderInfo* preInfo;
        RecyclerView::ItemAnimator::ItemHolderInfo* postInfo;
    private:
        friend ViewInfoStore;
    public:
        InfoRecord();
    };
public:
    struct ProcessCallback {
        std::function<void(RecyclerView::ViewHolder*,RecyclerView::ItemAnimator::ItemHolderInfo*,
			RecyclerView::ItemAnimator::ItemHolderInfo*)> processDisappeared;
	    //(RecyclerView::ViewHolder* viewHolder,RecyclerView::ItemAnimator::ItemHolderInfo* preInfo, RecyclerView::ItemAnimator::ItemHolderInfo* postInfo);
	    std::function<void(RecyclerView::ViewHolder*,RecyclerView::ItemAnimator::ItemHolderInfo*,
			RecyclerView::ItemAnimator::ItemHolderInfo*)> processAppeared;
	    //(RecyclerView::ViewHolder* viewHolder, RecyclerView::ItemAnimator::ItemHolderInfo* preInfo,RecyclerView::ItemAnimator::ItemHolderInfo* postInfo);
	    std::function<void(RecyclerView::ViewHolder*,RecyclerView::ItemAnimator::ItemHolderInfo*,RecyclerView::ItemAnimator::ItemHolderInfo*)> processPersistent;
	    //(RecyclerView::ViewHolder* viewHolder,RecyclerView::ItemAnimator::ItemHolderInfo* preInfo, RecyclerView::ItemAnimator::ItemHolderInfo* postInfo);
	    std::function<void(RecyclerView::ViewHolder*)>unused;//(RecyclerView::ViewHolder* holder);
    };
private:
    Pools::SimplePool<InfoRecord> mPool;
protected:
    std::unordered_map<RecyclerView::ViewHolder*, InfoRecord*> mLayoutHolderMap;
    LongSparseArray<RecyclerView::ViewHolder*> mOldChangedHolders;

    InfoRecord* obtainInfoRecord();
    void recycleInfoRecord(InfoRecord* record);
    void drainInfoRecordCache();
private:
    friend RecyclerView;
    RecyclerView::ItemAnimator::ItemHolderInfo* popFromLayoutStep(RecyclerView::ViewHolder* vh, int flag);
protected:
    void clear();
    void addToPreLayout(RecyclerView::ViewHolder* holder, RecyclerView::ItemAnimator::ItemHolderInfo* info);
    bool isDisappearing(RecyclerView::ViewHolder* holder);
    RecyclerView::ItemAnimator::ItemHolderInfo* popFromPreLayout(RecyclerView::ViewHolder* vh);
    RecyclerView::ItemAnimator::ItemHolderInfo* popFromPostLayout(RecyclerView::ViewHolder* vh);
    void addToOldChangeHolders(long key, RecyclerView::ViewHolder* holder);
    void addToAppearedInPreLayoutHolders(RecyclerView::ViewHolder* holder, RecyclerView::ItemAnimator::ItemHolderInfo* info);
    bool isInPreLayout(RecyclerView::ViewHolder* viewHolder);
    RecyclerView::ViewHolder* getFromOldChangeHolders(long key);
    void addToPostLayout(RecyclerView::ViewHolder* holder, RecyclerView::ItemAnimator::ItemHolderInfo* info);
    void addToDisappearedInLayout(RecyclerView::ViewHolder* holder);
    void removeFromDisappearedInLayout(RecyclerView::ViewHolder* holder);
    void process(ProcessCallback callback);
    void removeViewHolder(RecyclerView::ViewHolder* holder);
    void onDetach();
public:
    ViewInfoStore();
    virtual ~ViewInfoStore();
    void onViewDetached(RecyclerView::ViewHolder* viewHolder);
};
}/*endof namespace*/
#endif

