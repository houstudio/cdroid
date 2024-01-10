#include <widgetEx/recyclerview/recyclerview.h>
#include <widgetEx/recyclerview/viewinfostore.h>

namespace cdroid{
static constexpr int _DEBUG=1;

void ViewInfoStore::clear() {
    mLayoutHolderMap.clear();
    mOldChangedHolders.clear();
}

void ViewInfoStore::addToPreLayout(RecyclerView::ViewHolder* holder, RecyclerView::ItemAnimator::ItemHolderInfo* info) {
    InfoRecord* record = nullptr;//mLayoutHolderMap.get(holder);
    auto it = mLayoutHolderMap.find(holder);
    if(it ==mLayoutHolderMap.end())record = it->second;
    if (record == nullptr) {
        record = InfoRecord::obtain();
        mLayoutHolderMap.insert({holder,record});//put(holder, record);
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
    //int index = mLayoutHolderMap.indexOfKey(vh);
    auto it = mLayoutHolderMap.find(vh);
    if (it ==mLayoutHolderMap.end()){//index < 0) {
        return nullptr;
    }
    InfoRecord* record = it->second;//mLayoutHolderMap.valueAt(index);
    if (record != nullptr && (record->flags & flag) != 0) {
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
            mLayoutHolderMap.erase(it);//removeAt(index);
	    InfoRecord::recycle(record);
        }
        return info;
    }
    return nullptr;
}

void ViewInfoStore::addToOldChangeHolders(long key, RecyclerView::ViewHolder* holder) {
    mOldChangedHolders.insert({key,holder});//put(key, holder);
}

void ViewInfoStore::addToAppearedInPreLayoutHolders(RecyclerView::ViewHolder* holder, RecyclerView::ItemAnimator::ItemHolderInfo* info) {
    InfoRecord* record = nullptr;//mLayoutHolderMap.get(holder);
    auto it = mLayoutHolderMap.find(holder);
    if(it!=mLayoutHolderMap.end())record = it->second;
    if (record == nullptr) {
        record = InfoRecord::obtain();
        mLayoutHolderMap.insert({holder,record});//put(holder, record);
    }
    record->flags |= InfoRecord::FLAG_APPEAR;
    record->preInfo = info;
}

bool ViewInfoStore::isInPreLayout(RecyclerView::ViewHolder* viewHolder) {
    InfoRecord* record = nullptr;//mLayoutHolderMap.get(viewHolder);
    auto it = mLayoutHolderMap.find(viewHolder);
    if(it!=mLayoutHolderMap.end())record = it->second;
    return record && (record->flags & InfoRecord::FLAG_PRE) != 0;
}

RecyclerView::ViewHolder* ViewInfoStore::getFromOldChangeHolders(long key) {
    auto it = mOldChangedHolders.find(key);
    return (it!=mOldChangedHolders.end())?it->second:nullptr;
}

void ViewInfoStore::addToPostLayout(RecyclerView::ViewHolder* holder, RecyclerView::ItemAnimator::ItemHolderInfo* info) {
    InfoRecord* record = nullptr;//mLayoutHolderMap.get(holder);
    auto it = mLayoutHolderMap.find(holder);
    if(it!=mLayoutHolderMap.end())record = it->second;
    if (record == nullptr) {
        record = InfoRecord::obtain();
        mLayoutHolderMap.insert({holder,record});//put(holder, record);
    }
    record->postInfo = info;
    record->flags |= InfoRecord::FLAG_POST;
}

void ViewInfoStore::addToDisappearedInLayout(RecyclerView::ViewHolder* holder) {
    InfoRecord* record = nullptr;//mLayoutHolderMap.get(holder);
    auto it = mLayoutHolderMap.find(holder);
    if(it!=mLayoutHolderMap.end())record = it->second;
    if (record == nullptr) {
        record = InfoRecord::obtain();
        mLayoutHolderMap.insert({holder,record});//put(holder, record);
    }
    record->flags |= InfoRecord::FLAG_DISAPPEARED;
}

void ViewInfoStore::removeFromDisappearedInLayout(RecyclerView::ViewHolder* holder) {
    InfoRecord* record = nullptr;//mLayoutHolderMap.get(holder);
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
        InfoRecord* record = it->second;//mLayoutHolderMap.removeAt(index);
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
        } else if (_DEBUG) {
            LOGE("record without any reasonable flag combination:/");
        }
        InfoRecord::recycle(record);
    }
}

void ViewInfoStore::removeViewHolder(RecyclerView::ViewHolder* holder) {
    for (auto it =mOldChangedHolders.begin();it!=mOldChangedHolders.end();it++){
        if (holder == it->second){//mOldChangedHolders.valueAt(i)) {
            mOldChangedHolders.erase(it);//removeAt(i);
            break;
        }
    }
    InfoRecord* info = nullptr;//mLayoutHolderMap.remove(holder);
    auto it = mLayoutHolderMap.find(holder);
    if( it!= mLayoutHolderMap.end() )info = it->second;
    if (info != nullptr) {
        InfoRecord::recycle(info);
    }
}

void ViewInfoStore::onDetach() {
    InfoRecord::drainCache();
}

void ViewInfoStore::onViewDetached(RecyclerView::ViewHolder* viewHolder) {
    removeFromDisappearedInLayout(viewHolder);
}

Pools::SimplePool<ViewInfoStore::InfoRecord*> ViewInfoStore::InfoRecord::sPool(20);//=new Pools.SimplePool<>(20);

ViewInfoStore::InfoRecord::InfoRecord() {
}

ViewInfoStore::InfoRecord* ViewInfoStore::InfoRecord::obtain() {
    InfoRecord* record = sPool.acquire();
    return record == nullptr ? new InfoRecord() : record;
}

void ViewInfoStore::InfoRecord::recycle(InfoRecord* record) {
    record->flags = 0;
    record->preInfo = nullptr;
    record->postInfo = nullptr;
    sPool.release(record);
}

void ViewInfoStore::InfoRecord::drainCache() {
    //noinspection StatementWithEmptyBody
    while (sPool.acquire() != nullptr);
}

}/*endof namespace*/
