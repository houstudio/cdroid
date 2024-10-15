#include <widget/recyclebin.h>
#include <widget/abslistview.h>
#include <cdtypes.h>
#include <cdlog.h>


namespace cdroid {

RecycleBin::RecycleBin(AbsListView*lv){
    LV=lv;
    mFirstActivePosition=0;
    mViewTypeCount=0;
}

Adapter*RecycleBin::getAdapter(){
    return LV->mAdapter;
}

void RecycleBin::setViewTypeCount(int viewTypeCount) {
    LOGE_IF(viewTypeCount < 1,"Can't have a viewTypeCount(%d) < 1",viewTypeCount);
    //noinspection unchecked
    for (int i = 0; i < viewTypeCount; i++) {
        mScrapViews.push_back(std::vector<View*>());
    }
    mCurrentScrap=&mScrapViews.at(0);
    mViewTypeCount = viewTypeCount;
}

void RecycleBin::markChildrenDirty() {
    const int typeCount = mScrapViews.size();//mViewTypeCount;
    for (int i = 0; i < typeCount; i++) {
 	    std::vector<View*>& scrap = mScrapViews[i];
        int scrapCount = scrap.size();
        for (int j = 0; j < scrapCount; j++) {
            scrap[j]->forceLayout();
        }
    }

    if (mTransientStateViews.size()) {
        int count = mTransientStateViews.size();
        for (int i = 0; i < count; i++) {
            mTransientStateViews.valueAt(i)->forceLayout();
        }
    }
    if (mTransientStateViewsById.size()) {
        int count = mTransientStateViewsById.size();
        for (int i = 0; i < count; i++) {
            mTransientStateViewsById.valueAt(i)->forceLayout();
        }
    }
}

void RecycleBin::clear() {
    for (int i = 0; i < mScrapViews.size(); i++) {
        std::vector<View*>& scrap = mScrapViews[i];
        clearScrap(scrap);
    }
    clearTransientStateViews();
}

void RecycleBin::fillActiveViews(int childCount, int firstActivePosition) {
    if (mActiveViews.size() < childCount) {
        mActiveViews.resize(childCount);
    }
    mFirstActivePosition = firstActivePosition;
    //noinspection MismatchedReadAndWriteOfArray
    for (int i = 0; i < childCount; i++) {
        View* child = LV->getChildAt(i);
        AbsListView::LayoutParams* lp = (AbsListView::LayoutParams*) child->getLayoutParams();
        // Don't put header or footer views into the scrap heap
        if (lp != nullptr && lp->viewType != AdapterView::ITEM_VIEW_TYPE_HEADER_OR_FOOTER) {
            // Note:  We do place AdapterView.ITEM_VIEW_TYPE_IGNORE in active views.
            //        However, we will NOT place them into scrap views.
            mActiveViews[i] = child;
            // Remember the position so that setupChild() doesn't reset state.
            lp->scrappedFromPosition = firstActivePosition + i;
        }
    }
}

View* RecycleBin::getActiveView(int position) {
    int index = position - mFirstActivePosition;
    if (index >=0 && index < mActiveViews.size()) {
        View* match = mActiveViews[index];
        mActiveViews[index] = nullptr;
        LOGV("position=%d view=%p",position,match);
        return match;
    }
    return nullptr;
}

View* RecycleBin::getTransientStateView(int position) {
    if (getAdapter() != nullptr && LV->mAdapterHasStableIds && mTransientStateViewsById.size()) {
        long id = getAdapter()->getItemId(position);
        View* result = mTransientStateViewsById.get(id);
        mTransientStateViewsById.remove(id);
        return result;
    }
    if (mTransientStateViews.size()) {
        int index = mTransientStateViews.indexOfKey(position);
        if (index >= 0) {
            View* result = mTransientStateViews.valueAt(index);
            mTransientStateViews.removeAt(index);
            return result;
        }
    }
    return nullptr;
}

void RecycleBin::clearTransientStateViews() {
    SparseArray<View*>& viewsByPos = mTransientStateViews;
    if (viewsByPos.size()) {
        const int N = (int)viewsByPos.size();
        for (int i = 0; i < N; i++) {
            removeDetachedView(viewsByPos.valueAt(i), false);
        }
        viewsByPos.clear();
    }

    SparseArray<View*>& viewsById = mTransientStateViewsById;
    if (viewsById.size()) {
        const int N = (int)viewsById.size();
        for (int i = 0; i < N; i++) {
            removeDetachedView(viewsById.valueAt(i), false);
        }
        viewsById.clear();
    }
}

View* RecycleBin::getScrapView(int position) {
    int whichScrap = getAdapter()->getItemViewType(position);
    if (whichScrap < 0) {
        return nullptr;
    }
    if (mViewTypeCount == 1) {
        return retrieveFromScrap(*mCurrentScrap, position);
    } else if (whichScrap < mScrapViews.size()) {
        return retrieveFromScrap(mScrapViews[whichScrap], position);
    }
    return nullptr;
}

void RecycleBin::addScrapView(View* scrap, int position) {
    AbsListView::LayoutParams* lp = (AbsListView::LayoutParams*) scrap->getLayoutParams();
    if (lp == nullptr) {
        // Can't recycle, but we don't know anything about the view.
        // Ignore it completely.
        return;
    }

    lp->scrappedFromPosition = position;

    // Remove but don't scrap header or footer views, or views that
    // should otherwise not be recycled.
    int viewType = lp->viewType;
    if (!shouldRecycleViewType(viewType)) {
        // Can't recycle. If it's not a header or footer, which have
        // special handling and should be ignored, then skip the scrap
        // heap and we'll fully detach the view later.
        if (viewType != AdapterView::ITEM_VIEW_TYPE_HEADER_OR_FOOTER) {
            getSkippedScrap().push_back(scrap);
        }
        return;
    }

    scrap->dispatchStartTemporaryDetach();

    // The the accessibility state of the view may change while temporary
    // detached and we do not allow detached views to fire accessibility
    // events. So we are announcing that the subtree changed giving a chance
    // to clients holding on to a view in this subtree to refresh it.
    //notifyViewAccessibilityStateChangedIfNeeded(AccessibilityEvent.CONTENT_CHANGE_TYPE_SUBTREE);

    // Don't scrap views that have transient state.
    bool scrapHasTransientState = scrap->hasTransientState();
    if (scrapHasTransientState) {
        if (getAdapter() && LV->mAdapterHasStableIds) {
            // If the adapter has stable IDs, we can reuse the view for
            // the same data.
            mTransientStateViewsById.put(lp->itemId, scrap);
        } else if (!LV->mDataChanged) {
            // If the data hasn't changed, we can reuse the views at
            // their old positions.
            mTransientStateViews.put(position, scrap);
        } else {
            // Otherwise, we'll have to remove the view and start over.
            clearScrapForRebind(scrap);
            getSkippedScrap().push_back(scrap);
        }
    } else {
        clearScrapForRebind(scrap);
        if (mViewTypeCount == 1) {
            mCurrentScrap->push_back(scrap);//mScrapViews[0].push_back(scrap);
        } else {
            mScrapViews[viewType].push_back(scrap);
        }
        if (mRecyclerListener)mRecyclerListener(*scrap);
    }
}

std::vector<View*>RecycleBin::getSkippedScrap() {
    LOGV("mSkippedScrap.size=%d",mSkippedScrap.size());
    return mSkippedScrap;
}

void RecycleBin::removeSkippedScrap() {
    int count = mSkippedScrap.size();
    for (int i = 0; i < count; i++) {
        removeDetachedView(mSkippedScrap[i], false);
    }
    mSkippedScrap.clear();
}

void RecycleBin::scrapActiveViews() {
    std::vector<View*>& activeViews = mActiveViews;
    bool multipleScraps = mViewTypeCount > 1;

    std::vector<View*>& scrapViews =*mCurrentScrap;
    const int count = (int)activeViews.size();
    for (int i = count - 1; i >= 0; i--) {
        View* victim = activeViews[i];
        if (victim == nullptr)continue;
        AbsListView::LayoutParams *lp= (AbsListView::LayoutParams*)victim->getLayoutParams();
        int whichScrap = lp->viewType;

        activeViews[i] = nullptr;

        if (victim->hasTransientState()) {
            // Store views with transient state for later use.
            victim->dispatchStartTemporaryDetach();

            if (getAdapter()!= nullptr && LV->mAdapterHasStableIds) {
                long id = getAdapter()->getItemId(mFirstActivePosition + i);
                mTransientStateViewsById.put(id, victim);
            } else if (!LV->mDataChanged) {
                mTransientStateViews.put(mFirstActivePosition + i, victim);
            } else if (whichScrap != AdapterView::ITEM_VIEW_TYPE_HEADER_OR_FOOTER) {
                // The data has changed, we can't keep this view.
                removeDetachedView(victim, false);
            }
        } else if (!shouldRecycleViewType(whichScrap)) {
            // Discard non-recyclable views except headers/footers.
            if (whichScrap != AdapterView::ITEM_VIEW_TYPE_HEADER_OR_FOOTER) {
                removeDetachedView(victim, false);
            }
        } else {
            // Store everything else on the appropriate scrap heap.
            if (multipleScraps) {
                scrapViews = mScrapViews[whichScrap];
            }

            lp->scrappedFromPosition = mFirstActivePosition + i;
            removeDetachedView(victim, false);
            scrapViews.push_back(victim);//add(victim);

            if (mRecyclerListener) mRecyclerListener(*victim);
        }
    }
    pruneScrapViews();
}


void RecycleBin::fullyDetachScrapViews() {
    int viewTypeCount = mViewTypeCount;
    for (int i = 0; i < viewTypeCount; ++i) {
        std::vector<View*>& scrapPile = mScrapViews[i];
        for (int j = scrapPile.size() - 1; j >= 0; j--) {
            View* view = scrapPile[j];
            if (view->isTemporarilyDetached()) {
                removeDetachedView(view, false);
            }
        }
    }
}


void RecycleBin::pruneScrapViews() {
    int maxViews = mActiveViews.size();
    int viewTypeCount = mViewTypeCount;
    for (int i = 0; i < viewTypeCount; ++i) {
        std::vector<View*>& scrapPile = mScrapViews[i];
        int size = scrapPile.size();
        while (size > maxViews) {
            scrapPile.erase(scrapPile.begin()+(--size));//remove(--size);
        }
    }

    SparseArray<View*>& transViewsByPos = mTransientStateViews;
    if (transViewsByPos.size()) {
        for (int i = 0; i < transViewsByPos.size(); i++) {
            View* v = transViewsByPos.valueAt(i);
            if (!v->hasTransientState()) {
                removeDetachedView(v, false);
                transViewsByPos.removeAt(i);
                i--;
            }
        }
    }

    SparseArray<View*>& transViewsById = mTransientStateViewsById;
    if (transViewsById.size()) {
        for (int i = 0; i < transViewsById.size(); i++) {
            View* v = transViewsById.valueAt(i);
            if (!v->hasTransientState()) {
                removeDetachedView(v, false);
                transViewsById.removeAt(i);
                i--;
            }
        }
    }
}

void RecycleBin::reclaimScrapViews(std::vector<View*>& views) {
    if (mViewTypeCount == 1) {
        views=mScrapViews[0];
    } else {
        for (int i = 0; i < mViewTypeCount; ++i) {
            std::vector<View*>& scrapPile = mScrapViews[i];
            //views.addAll(scrapPile);
            views.insert(views.begin(),scrapPile.begin(),scrapPile.end());
        }
    }
}

void RecycleBin::setCacheColorHint(int color) {
    const size_t typeCount = mScrapViews.size();//mViewTypeCount;
    for (size_t i = 0; i < typeCount; i++) {
        std::vector<View*>& scrap = mScrapViews[i];
        int scrapCount = scrap.size();
        for (int j = 0; j < scrapCount; j++) {
            scrap[j]->setDrawingCacheBackgroundColor(color);
        }
    }

    // Just in case this is called during a layout pass
    size_t count = mActiveViews.size();
    for (size_t i = 0; i < count; ++i) {
        View* victim = mActiveViews[i];
        if (victim != nullptr) {
            victim->setDrawingCacheBackgroundColor(color);
        }
    }
}

View* RecycleBin::retrieveFromScrap(std::vector<View*>& scrapViews, int position) {
    const size_t size = scrapViews.size();
    if (size > 0) {
        // See if we still have a view for this position or ID.
        // Traverse backwards to find the most recently used scrap view
        for (size_t i = size - 1; i >= 0; i--) {
            View* view = scrapViews[i];
            AbsListView::LayoutParams* params =(AbsListView::LayoutParams*) view->getLayoutParams();

            if (LV->mAdapterHasStableIds) {
                long id = getAdapter()->getItemId(position);
                if (id == params->itemId) {
		            View*scrap=scrapViews[i];
                    scrapViews.erase(scrapViews.begin()+i);//remove(i);
                    return scrap;
                }
            } else if (params->scrappedFromPosition == position) {
                View* scrap = scrapViews[i];
                scrapViews.erase(scrapViews.begin()+i);
                clearScrapForRebind(scrap);
                return scrap;
            }
        }
        View* scrap =scrapViews[size-1];
       	scrapViews.erase(scrapViews.begin()+size-1);
        clearScrapForRebind(scrap);
        return scrap;
    } else {
        return nullptr;
    }
}

void RecycleBin::clearScrap(std::vector<View*>& scrap) {
    const size_t scrapCount = scrap.size();
    for (size_t j = 0; j < scrapCount; j++) {
        View*v=scrap[scrapCount - 1 - j];
        scrap.erase(scrap.begin()+scrapCount - 1 - j);
        removeDetachedView(v, false);
        delete v;//when we destroy the listview'spage,if listview is fling,delete v will caused crash
    }
}

void RecycleBin::clearScrapForRebind(View* view) {
    LOGV("view=%p %d",view,mScrapViews[0].size());
    view->clearAccessibilityFocus();
    //view->setAccessibilityDelegate(nullptr);
}

void RecycleBin::removeDetachedView(View* child, bool animate) {
    //child->setAccessibilityDelegate(nullptr);
    LV->removeDetachedView(child, animate);
}

}//namespace
