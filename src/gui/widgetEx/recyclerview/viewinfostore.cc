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
#include <widgetEx/recyclerview/recyclerview.h>
#include <widgetEx/recyclerview/viewinfostore.h>

namespace cdroid{
static constexpr int _Debug=1;

ViewInfoStore::ViewInfoStore():mPool(20){
}

ViewInfoStore::~ViewInfoStore(){
}

void ViewInfoStore::clear() {
    mLayoutHolderMap.clear();
    mOldChangedHolders.clear();
}

void ViewInfoStore::addToPreLayout(RecyclerView::ViewHolder* holder, RecyclerView::ItemAnimator::ItemHolderInfo* info) {
    InfoRecord* record = nullptr;
    auto it = mLayoutHolderMap.find(holder);
    if(it !=mLayoutHolderMap.end())record = it->second;
    if (record == nullptr) {
        record = obtainInfoRecord();
        mLayoutHolderMap.insert({holder,record});
    }
    record->preInfo = info;
    record->flags |= InfoRecord::FLAG_PRE;
}

bool ViewInfoStore::isDisappearing(RecyclerView::ViewHolder* holder) {
    InfoRecord* record = nullptr;
    auto it = mLayoutHolderMap.find(holder);
    if(it!=mLayoutHolderMap.end())record = it->second;
    return record != nullptr && ((record->flags & InfoRecord::FLAG_DISAPPEARED) != 0);
}

RecyclerView::ItemAnimator::ItemHolderInfo* ViewInfoStore::popFromPreLayout(RecyclerView::ViewHolder* vh) {
    return popFromLayoutStep(vh, InfoRecord::FLAG_PRE);
}

RecyclerView::ItemAnimator::ItemHolderInfo* ViewInfoStore::popFromPostLayout(RecyclerView::ViewHolder* vh) {
    return popFromLayoutStep(vh, InfoRecord::FLAG_POST);
}

RecyclerView::ItemAnimator::ItemHolderInfo* ViewInfoStore::popFromLayoutStep(RecyclerView::ViewHolder* vh, int flag) {
    auto it = mLayoutHolderMap.find(vh);
    if (it ==mLayoutHolderMap.end()) {
        return nullptr;
    }
    InfoRecord* record = it->second;
    if ((record != nullptr) && (record->flags & flag) != 0) {
        record->flags &= ~flag;
        RecyclerView::ItemAnimator::ItemHolderInfo* info;
        if (flag == InfoRecord::FLAG_PRE) {
            info = record->preInfo;
        } else if (flag == InfoRecord::FLAG_POST) {
            info = record->postInfo;
        } else {
            LOGE("Must provide flag PRE or POST");
        }
        // if not pre-post flag is left, clear.
        if ((record->flags & (InfoRecord::FLAG_PRE | InfoRecord::FLAG_POST)) == 0) {
            mLayoutHolderMap.erase(it);
            recycleInfoRecord(record);
        }
        return info;
    }
    return nullptr;
}

void ViewInfoStore::addToOldChangeHolders(long key, RecyclerView::ViewHolder* holder) {
    mOldChangedHolders.put(key, holder);
}

void ViewInfoStore::addToAppearedInPreLayoutHolders(RecyclerView::ViewHolder* holder, RecyclerView::ItemAnimator::ItemHolderInfo* info) {
    InfoRecord* record = nullptr;
    auto it = mLayoutHolderMap.find(holder);
    if(it!=mLayoutHolderMap.end())record = it->second;
    if (record == nullptr) {
        record = obtainInfoRecord();
        mLayoutHolderMap.insert({holder,record});
    }
    record->flags |= InfoRecord::FLAG_APPEAR;
    record->preInfo = info;
}

bool ViewInfoStore::isInPreLayout(RecyclerView::ViewHolder* viewHolder) {
    InfoRecord* record = nullptr;
    auto it = mLayoutHolderMap.find(viewHolder);
    if(it!=mLayoutHolderMap.end())record = it->second;
    return record && (record->flags & InfoRecord::FLAG_PRE) != 0;
}

RecyclerView::ViewHolder* ViewInfoStore::getFromOldChangeHolders(long key) {
    return  mOldChangedHolders.get(key);
}

void ViewInfoStore::addToPostLayout(RecyclerView::ViewHolder* holder, RecyclerView::ItemAnimator::ItemHolderInfo* info) {
    InfoRecord* record = nullptr;
    auto it = mLayoutHolderMap.find(holder);
    if(it!=mLayoutHolderMap.end())record = it->second;
    if (record == nullptr) {
        record = obtainInfoRecord();
        mLayoutHolderMap.insert({holder,record});
    }
    record->postInfo = info;
    record->flags |= InfoRecord::FLAG_POST;
}

void ViewInfoStore::addToDisappearedInLayout(RecyclerView::ViewHolder* holder) {
    InfoRecord* record = nullptr;
    auto it = mLayoutHolderMap.find(holder);
    if(it!=mLayoutHolderMap.end())record = it->second;
    if (record == nullptr) {
        record = /*InfoRecord::*/obtainInfoRecord();
        mLayoutHolderMap.insert({holder,record});
    }
    record->flags |= InfoRecord::FLAG_DISAPPEARED;
}

void ViewInfoStore::removeFromDisappearedInLayout(RecyclerView::ViewHolder* holder) {
    InfoRecord* record = nullptr;
    auto it = mLayoutHolderMap.find(holder);
    if(it!=mLayoutHolderMap.end())record = it->second;
    if (record == nullptr) {
        return;
    }
    record->flags &= ~InfoRecord::FLAG_DISAPPEARED;
}

void ViewInfoStore::process(ProcessCallback callback) {
    for(auto it = mLayoutHolderMap.begin();it!=mLayoutHolderMap.end();){
        RecyclerView::ViewHolder* viewHolder = it->first;
        InfoRecord* record = it->second;
        it = mLayoutHolderMap.erase(it);
        if ((record->flags & InfoRecord::FLAG_APPEAR_AND_DISAPPEAR) == InfoRecord::FLAG_APPEAR_AND_DISAPPEAR) {
            // Appeared then disappeared. Not useful for animations.
            callback.unused(viewHolder);
        } else if ((record->flags & InfoRecord::FLAG_DISAPPEARED) != 0) {
            // Set as "disappeared" by the LayoutManager (addDisappearingView)
            if (record->preInfo == nullptr) {
                // similar to appear disappear but happened between different layout passes.
                // this can happen when the layout manager is using auto-measure
                callback.unused(viewHolder);
            } else {
                callback.processDisappeared(viewHolder, record->preInfo, record->postInfo);
            }
        } else if ((record->flags & InfoRecord::FLAG_APPEAR_PRE_AND_POST) == InfoRecord::FLAG_APPEAR_PRE_AND_POST) {
            // Appeared in the layout but not in the adapter (e.g. entered the viewport)
            callback.processAppeared(viewHolder, record->preInfo, record->postInfo);
        } else if ((record->flags & InfoRecord::FLAG_PRE_AND_POST) == InfoRecord::FLAG_PRE_AND_POST) {
            // Persistent in both passes. Animate persistence
            callback.processPersistent(viewHolder, record->preInfo, record->postInfo);
        } else if ((record->flags & InfoRecord::FLAG_PRE) != 0) {
            // Was in pre-layout, never been added to post layout
            callback.processDisappeared(viewHolder, record->preInfo, nullptr);
        } else if ((record->flags & InfoRecord::FLAG_POST) != 0) {
            // Was not in pre-layout, been added to post layout
            callback.processAppeared(viewHolder, record->preInfo, record->postInfo);
        } else if ((record->flags & InfoRecord::FLAG_APPEAR) != 0) {
            // Scrap view. RecyclerView will handle removing/recycling this.
        } else if (_Debug) {
            LOGE("record without any reasonable flag combination:/");
        }
        delete record->preInfo;
        delete record->postInfo;
        recycleInfoRecord(record);
    }
}

void ViewInfoStore::removeViewHolder(RecyclerView::ViewHolder* holder) {
    for (int i = mOldChangedHolders.size()-1;i>=0;i--){
        if (holder == mOldChangedHolders.valueAt(i)) {
            mOldChangedHolders.removeAt(i);
            break;
        }
    }
    auto it = mLayoutHolderMap.find(holder);
    if( it!= mLayoutHolderMap.end() ){
        InfoRecord*info = it->second;
        if(info)
            recycleInfoRecord(info);
        mLayoutHolderMap.erase(it);
    }
}

void ViewInfoStore::onDetach() {
    drainInfoRecordCache();
}

void ViewInfoStore::onViewDetached(RecyclerView::ViewHolder* viewHolder) {
    removeFromDisappearedInLayout(viewHolder);
}

ViewInfoStore::InfoRecord::InfoRecord() {
    preInfo = nullptr;
    postInfo = nullptr;
    flags = 0;
}

ViewInfoStore::InfoRecord* ViewInfoStore::obtainInfoRecord() {
    InfoRecord* record = mPool.acquire();
    if(record==nullptr)record = new InfoRecord();
    record->preInfo = nullptr;
    record->postInfo = nullptr;
    record->flags = 0;
    return record;
}

void ViewInfoStore::recycleInfoRecord(InfoRecord* record) {
    record->flags = 0;
    record->preInfo = nullptr;
    record->postInfo= nullptr;
    mPool.release(record);
}

void ViewInfoStore::drainInfoRecordCache() {
    //noinspection StatementWithEmptyBody
    InfoRecord*info = nullptr;
    while ((info=mPool.acquire())!=nullptr) { 
        delete info;
    }
}

}/*endof namespace*/
