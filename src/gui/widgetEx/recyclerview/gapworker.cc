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
#include <widgetEx/recyclerview/gapworker.h>
#include <widgetEx/recyclerview/adapterhelper.h>
#include <widgetEx/recyclerview/childhelper.h>
namespace cdroid{

GapWorker*GapWorker::sGapWorker = nullptr;

void GapWorker::LayoutPrefetchRegistryImpl::setPrefetchVector(int dx, int dy) {
    mPrefetchDx = dx;
    mPrefetchDy = dy;
}

void GapWorker::LayoutPrefetchRegistryImpl::collectPrefetchPositionsFromView(RecyclerView* view, bool nested) {
    mCount = 0;
    if (!mPrefetchArray.empty()) {
        std::fill(mPrefetchArray.begin(),mPrefetchArray.end(), -1);
    }

    RecyclerView::LayoutManager* layout = view->mLayout;
    if ((view->mAdapter != nullptr) && (layout != nullptr) && layout->isItemPrefetchEnabled()) {
        if (nested) {
            // nested prefetch, only if no adapter updates pending. Note: we don't query
            // view.hasPendingAdapterUpdates(), as first layout may not have occurred
            if (!view->mAdapterHelper->hasPendingUpdates()) {
                layout->collectInitialPrefetchPositions(view->mAdapter->getItemCount(), *this);
            }
        } else {
            // momentum based prefetch, only if we trust current child/adapter state
            if (!view->hasPendingAdapterUpdates()) {
                layout->collectAdjacentPrefetchPositions(mPrefetchDx, mPrefetchDy, *view->mState, *this);
            }
        }

        if (mCount > layout->mPrefetchMaxCountObserved) {
            layout->mPrefetchMaxCountObserved = mCount;
            layout->mPrefetchMaxObservedInInitialPrefetch = nested;
            view->mRecycler->updateViewCacheSize();
        }
    }
}

void GapWorker::LayoutPrefetchRegistryImpl::addPosition(int layoutPosition, int pixelDistance) {
    if (layoutPosition < 0) {
        throw std::logic_error("Layout positions must be non-negative");
    }

    if (pixelDistance < 0) {
        throw std::logic_error("Pixel distance must be non-negative");
    }

    // allocate or expand array as needed, doubling when needed
    const int storagePosition = mCount * 2;
    if (mPrefetchArray.empty()) {
        mPrefetchArray.resize(4);
        std::fill(mPrefetchArray.begin(),mPrefetchArray.end(), -1);
    } else if (storagePosition >= mPrefetchArray.size()) {
        auto& oldArray = mPrefetchArray;
        mPrefetchArray.reserve(storagePosition * 2);
        //System.arraycopy(oldArray, 0, mPrefetchArray, 0, oldArray.length);
    }

    // add position
    mPrefetchArray[storagePosition] = layoutPosition;
    mPrefetchArray[storagePosition + 1] = pixelDistance;
    //LOGD("addPosition(%d,%d)",layoutPosition,pixelDistance);
    mCount++;
}

bool GapWorker::LayoutPrefetchRegistryImpl::lastPrefetchIncludedPosition(int position) {
    if (!mPrefetchArray.empty()) {
        const int count = mCount * 2;
        for (int i = 0; i < count; i += 2) {
            if (mPrefetchArray[i] == position) return true;
        }
    }
    return false;
}

/**
 * Called when prefetch indices are no longer valid for cache prioritization.
 */
void GapWorker::LayoutPrefetchRegistryImpl::clearPrefetchPositions() {
    if (!mPrefetchArray.empty()) {
        std::fill(mPrefetchArray.begin(),mPrefetchArray.end(), -1);
    }
    mCount = 0;
}

/////////////////////////////////////
GapWorker::GapWorker(){
    mPostTimeNs = 0;
    mFrameIntervalNs =0;
    mRunnable = std::bind(&GapWorker::run,this);
}

void GapWorker::add(RecyclerView* recyclerView) {
    auto it = std::find(mRecyclerViews.begin(),mRecyclerViews.end(),recyclerView);
    if (RecyclerView::sDebugAssertionsEnabled && (it==mRecyclerViews.end()) ) {
        throw std::logic_error("RecyclerView already present in worker list!");
    }
    mRecyclerViews.push_back(recyclerView);
}

void GapWorker::remove(RecyclerView* recyclerView) {
    auto it = std::find(mRecyclerViews.begin(),mRecyclerViews.end(),recyclerView);
    const bool removeSuccess = (it !=mRecyclerViews.end());
    if(removeSuccess)mRecyclerViews.erase(it);
    if (RecyclerView::sDebugAssertionsEnabled && !removeSuccess) {
        throw std::logic_error("RecyclerView removal failed!");
    }
}

/**
 * Schedule a prefetch immediately after the current traversal.
 */
void GapWorker::postFromTraversal(RecyclerView* recyclerView, int prefetchDx, int prefetchDy) {
    if (recyclerView->isAttachedToWindow()) {
        auto it = std::find(mRecyclerViews.begin(),mRecyclerViews.end(),recyclerView);
        if (RecyclerView::sDebugAssertionsEnabled && (it==mRecyclerViews.end())) {
            throw std::logic_error("attempting to post unregistered view!");
        }
        if (mPostTimeNs == 0) {
            mPostTimeNs = recyclerView->getNanoTime();
            recyclerView->post(mRunnable);
        }
    }
    LayoutPrefetchRegistryImpl* prefetchRegistry = (LayoutPrefetchRegistryImpl*)recyclerView->mPrefetchRegistry;
    prefetchRegistry->setPrefetchVector(prefetchDx, prefetchDy);
}

int GapWorker::TaskComparator(GapWorker::Task* lhs, GapWorker::Task* rhs) {
    // first, prioritize non-cleared tasks
    if ((lhs->view == nullptr) != (rhs->view == nullptr)) {
        return lhs->view == nullptr ? 1 : -1;
    }

    // then prioritize those (we think) are needed for next frame
    if (lhs->neededNextFrame != rhs->neededNextFrame) {
        return lhs->neededNextFrame ? -1 : 1;
    }

    // then prioritize _highest_ view velocity
    int deltaViewVelocity = rhs->viewVelocity - lhs->viewVelocity;
    if (deltaViewVelocity != 0) return deltaViewVelocity;

    // then prioritize _lowest_ distance to item
    int deltaDistanceToItem = lhs->distanceToItem - rhs->distanceToItem;
    if (deltaDistanceToItem != 0) return deltaDistanceToItem;

    return 0;
}

void GapWorker::buildTaskList() {
    // Update PrefetchRegistry in each view
    const int viewCount = mRecyclerViews.size();
    int totalTaskCount = 0;
    for (int i = 0; i < viewCount; i++) {
        RecyclerView* view = mRecyclerViews.at(i);
        if (view->getWindowVisibility() == View::VISIBLE) {
            LayoutPrefetchRegistryImpl* prefetchRegistry=(LayoutPrefetchRegistryImpl*)view->mPrefetchRegistry;
            prefetchRegistry->collectPrefetchPositionsFromView(view, false);
            totalTaskCount += prefetchRegistry->mCount;
        }
    }

    // Populate task list from prefetch data...
    mTasks.reserve/*ensureCapacity*/(totalTaskCount);
    int totalTaskIndex = 0;
    for (int i = 0; i < viewCount; i++) {
        RecyclerView* view = mRecyclerViews.at(i);
        if (view->getWindowVisibility() != View::VISIBLE) {
            // Invisible view, don't bother prefetching
            continue;
        }

        LayoutPrefetchRegistryImpl* prefetchRegistry = (LayoutPrefetchRegistryImpl*)view->mPrefetchRegistry;
        const int viewVelocity = std::abs(prefetchRegistry->mPrefetchDx)
                + std::abs(prefetchRegistry->mPrefetchDy);
        for (int j = 0; j < prefetchRegistry->mCount * 2; j += 2) {
            Task* task;
            if (totalTaskIndex >= mTasks.size()) {
                task = new Task();
                mTasks.push_back(task);
            } else {
                task = mTasks.at(totalTaskIndex);
            }
            const int distanceToItem = prefetchRegistry->mPrefetchArray[j + 1];

            task->neededNextFrame = distanceToItem <= viewVelocity;
            task->viewVelocity = viewVelocity;
            task->distanceToItem = distanceToItem;
            task->view = view;
            task->position = prefetchRegistry->mPrefetchArray[j];

            totalTaskIndex++;
        }
    }
    // ... and priority sort
    std::sort(mTasks.begin(),mTasks.end(), TaskComparator);
}

bool GapWorker::isPrefetchPositionAttached(RecyclerView* view, int position) {
    const int childCount = view->mChildHelper->getUnfilteredChildCount();
    for (int i = 0; i < childCount; i++) {
        View* attachedView = view->mChildHelper->getUnfilteredChildAt(i);
        RecyclerView::ViewHolder* holder = RecyclerView::getChildViewHolderInt(attachedView);
        // Note: can use mPosition here because adapter doesn't have pending updates
        if (holder->mPosition == position && !holder->isInvalid()) {
            return true;
        }
    }
    return false;
}

/*RecyclerView::ViewHolder*/
void* GapWorker::prefetchPositionWithDeadline(RecyclerView* view,int position, int64_t deadlineNs) {
    if (isPrefetchPositionAttached(view, position)) {
        // don't attempt to prefetch attached views
        return nullptr;
    }

    RecyclerView::Recycler* recycler = view->mRecycler;
    RecyclerView::ViewHolder* holder;

    // FOREVER_NS is used as a deadline to force the work to occur now,
    // since it's needed next frame, even if it won't fit in gap
    if (deadlineNs == RecyclerView::FOREVER_NS) {
        LOGD("RV Prefetch forced - needed next frame");
    }
    view->onEnterLayoutOrScroll();
    holder = recycler->tryGetViewHolderForPositionByDeadline(position, false, deadlineNs);

    if (holder != nullptr) {
        if (holder->isBound() && !holder->isInvalid()) {
            // Only give the view a chance to go into the cache if binding succeeded
            // Note that we must use public method, since item may need cleanup
            recycler->recycleView(holder->itemView);
        } else {
            // Didn't bind, so we can't cache the view, but it will stay in the pool until
            // next prefetch/traversal. If a View fails to bind, it means we didn't have
            // enough time prior to the deadline (and won't for other instances of this
            // type, during this GapWorker prefetch pass).
            recycler->addViewHolderToRecycledViewPool(*holder, false);
        }
    }
    view->onExitLayoutOrScroll(false);
    return holder;
}

void GapWorker::prefetchInnerRecyclerViewWithDeadline(RecyclerView* innerView,int64_t deadlineNs) {
    if (innerView == nullptr)  {
        return;
    }

    if (innerView->mDataSetHasChangedAfterLayout
            && innerView->mChildHelper->getUnfilteredChildCount() != 0) {
        // RecyclerView has new data, but old attached views. Clear everything, so that
        // we can prefetch without partially stale data.
        innerView->removeAndRecycleViews();
    }

    // do nested prefetch!
    LayoutPrefetchRegistryImpl* innerPrefetchRegistry = (LayoutPrefetchRegistryImpl*)innerView->mPrefetchRegistry;
    innerPrefetchRegistry->collectPrefetchPositionsFromView(innerView, true);

    if (innerPrefetchRegistry->mCount != 0) {
        LOGD(deadlineNs == RecyclerView::FOREVER_NS ? "RV Nested Prefetch" : "RV Nested Prefetch forced - needed next frame");
        innerView->mState->prepareForNestedPrefetch(innerView->mAdapter);
        for (int i = 0; i < innerPrefetchRegistry->mCount * 2; i += 2) {
            // Note that we ignore immediate flag for inner items because
            // we have lower confidence they're needed next frame.
            const int innerPosition = innerPrefetchRegistry->mPrefetchArray[i];
            prefetchPositionWithDeadline(innerView, innerPosition, deadlineNs);
        }
    }
}

void GapWorker::flushTaskWithDeadline(Task* task, int64_t deadlineNs) {
    int64_t taskDeadlineNs = task->neededNextFrame ? RecyclerView::FOREVER_NS : deadlineNs;
    RecyclerView::ViewHolder* holder = (RecyclerView::ViewHolder*)prefetchPositionWithDeadline(task->view,task->position, taskDeadlineNs);
    if ((holder != nullptr) && (holder->mNestedRecyclerView != nullptr)
            && holder->isBound() && !holder->isInvalid()) {
        prefetchInnerRecyclerViewWithDeadline(holder->mNestedRecyclerView, deadlineNs);
    }
}

void GapWorker::flushTasksWithDeadline(int64_t deadlineNs) {
    for (int i = 0; i < mTasks.size(); i++) {
        Task* task = mTasks.at(i);
        if (task->view == nullptr) {
            break; // done with populated tasks
        }
        flushTaskWithDeadline(task, deadlineNs);
        task->clear();
    }
}

void GapWorker::prefetch(int64_t deadlineNs) {
    buildTaskList();
    flushTasksWithDeadline(deadlineNs);
}

void GapWorker::run() {

    // Query most recent vsync so we can predict next one. Note that drawing time not yet
    // valid in animation/input callbacks, so query it here to be safe.
    const size_t size = mRecyclerViews.size();
    int64_t latestFrameVsyncMs = 0;
    for (int i = 0; i < size; i++) {
        RecyclerView* view = mRecyclerViews.at(i);
        if (view->getWindowVisibility() == View::VISIBLE) {
            latestFrameVsyncMs = std::max(view->getDrawingTime(), latestFrameVsyncMs);
        }
    }

    if (latestFrameVsyncMs > 0) {
        const int64_t nextFrameNs = latestFrameVsyncMs*1000000LL + mFrameIntervalNs;
        prefetch(nextFrameNs);
        // TODO: consider rescheduling self, if there's more work to do
        mPostTimeNs = 0;
    }
}

}/*endof namespace*/
