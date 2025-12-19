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
#include <widget/R.h>
#include <widgetEx/viewpager2.h>
#include <widgetEx/recyclerview/pagersnaphelper.h>
#include <widgetEx/scrolleventadapter.h>
#include <widgetEx/fakedrag.h>
//#include <widgetEx/compositeonpagechangecallback.h>

namespace cdroid{

DECLARE_WIDGET(ViewPager2)

class PageTransformerAdapter:public ViewPager2::OnPageChangeCallback {
private:
    LinearLayoutManager* mLayoutManager;
    ViewPager2::PageTransformer* mPageTransformer;
public:
    PageTransformerAdapter(LinearLayoutManager* layoutManager) {
        mLayoutManager = layoutManager;
        mPageTransformer= nullptr;
	    onPageScrolled = [this](int position, float positionOffset, int positionOffsetPixels){
            doPageScrolled(position,positionOffset,positionOffsetPixels);
        };
    }
    ViewPager2::PageTransformer* getPageTransformer() {
        return mPageTransformer;
    }

    void setPageTransformer(ViewPager2::PageTransformer* transformer) {
        mPageTransformer = transformer;
    }

    void doPageScrolled(int position, float positionOffset, int positionOffsetPixels) {
        if (mPageTransformer != nullptr) {
            const float transformOffset = -positionOffset;
            for(int i = 0; i < mLayoutManager->getChildCount(); ++i) {
                View* view = mLayoutManager->getChildAt(i);
                FATAL_IF(view==nullptr,"LayoutManager returned a null child at pos %d/%d while transforming pages",
				i, mLayoutManager->getChildCount());
                const int currPos = mLayoutManager->getPosition(view);
                const float viewOffset = transformOffset + (float)(currPos - position);
                mPageTransformer->transformPage(*view, viewOffset);
            }

        }
    }
};

ViewPager2::ViewPager2(int w,int h):ViewGroup(w,h){
    initialize(mContext, AttributeSet(mContext,""));
}

ViewPager2::ViewPager2(Context* context,const AttributeSet& attrs)
    :ViewGroup(context, attrs){
    initialize(context, attrs);
}

ViewPager2::~ViewPager2(){
    delete mScrollEventAdapter;
    delete mFakeDragger;
    delete mPagerSnapHelper;
    //delete mPageChangeEventDispatcher;
    //delete mExternalPageChangeCallbacks;
    delete mPageTransformerAdapter;
}

void ViewPager2::initialize(Context* context,const AttributeSet& attrs) {
    if(sFeatureEnhancedA11yEnabled)
        mAccessibilityProvider = new PageAwareAccessibilityProvider(this);
    else 
       mAccessibilityProvider  = new BasicAccessibilityProvider(this);
    mCurrentItemDataSetChangeObserver = nullptr;
    mRecyclerView = new RecyclerViewImpl(context,attrs,this);
    mRecyclerView->setId(View::generateViewId());
    mRecyclerView->setDescendantFocusability(FOCUS_BEFORE_DESCENDANTS);

    mLayoutManager = new LinearLayoutManagerImpl(context,this);
    mRecyclerView->setLayoutManager(mLayoutManager);
    mRecyclerView->setScrollingTouchSlop(RecyclerView::TOUCH_SLOP_PAGING);
    setOrientation(context, attrs);

    mRecyclerView->setLayoutParams(new ViewGroup::LayoutParams(LayoutParams::MATCH_PARENT, LayoutParams::MATCH_PARENT));
    attachViewToParent(mRecyclerView, 0, mRecyclerView->getLayoutParams());

    RecyclerView::OnChildAttachStateChangeListener ls;
    ls.onChildViewAttachedToWindow=[](View&view){
        RecyclerView::LayoutParams* layoutParams = (RecyclerView::LayoutParams*) view.getLayoutParams();
        if (layoutParams->width != LayoutParams::MATCH_PARENT
                || layoutParams->height != LayoutParams::MATCH_PARENT) {
            FATAL("Pages must fill the whole ViewPager2 (use match_parent)");
        }
    };
    ls.onChildViewDetachedFromWindow = [](View& view){
        LOGV("onChildViewDetachedFromWindow %p:%d",&view,view.getId());
    };
    mRecyclerView->addOnChildAttachStateChangeListener(ls);//enforceChildFillListener();

    // Create ScrollEventAdapter before attaching PagerSnapHelper to RecyclerView, because the
    // attach process calls PagerSnapHelperImpl.findSnapView, which uses the mScrollEventAdapter
    mScrollEventAdapter = new ScrollEventAdapter(this);
    // Create FakeDrag before attaching PagerSnapHelper, same reason as above
    mFakeDragger = new FakeDrag(this, mScrollEventAdapter, mRecyclerView);
    mPagerSnapHelper = new PagerSnapHelperImpl();
    mPagerSnapHelper->attachToRecyclerView(mRecyclerView);
    // Add mScrollEventAdapter after attaching mPagerSnapHelper to mRecyclerView, because we
    // don't want to respond on the events sent out during the attach process
    RecyclerView::OnScrollListener scrollCBK;
    scrollCBK.onScrolled = [this](RecyclerView& rv, int dx, int dy){
        mScrollEventAdapter->onScrolled(rv,dx,dy);
    };
    scrollCBK.onScrollStateChanged = [this](RecyclerView& rv, int newState){
        mScrollEventAdapter->onScrollStateChanged(rv,newState);
    };
    mRecyclerView->addOnScrollListener(scrollCBK);

    //mPageChangeEventDispatcher.addOnPageChangeCallback=[](){};
    //mPageChangeEventDispatcher.removeOnPageChangeCallback=[](){};
    mPageChangeEventDispatcher.onPageScrolled=[this](int position, float positionOffset,int positionOffsetPixels){
        for (OnPageChangeCallback& callback : mPageChangeCallbacks) {
            if(callback.onPageScrolled){
                callback.onPageScrolled(position, positionOffset, positionOffsetPixels);
            }
        }
    };
    mPageChangeEventDispatcher.onPageSelected=[this](int position){
        for (OnPageChangeCallback& callback : mPageChangeCallbacks) {
             if(callback.onPageSelected){
                 callback.onPageSelected(position);
             }
        }
    };
    mPageChangeEventDispatcher.onPageScrollStateChanged=[this](int state){
        for (OnPageChangeCallback& callback : mPageChangeCallbacks) {
             if(callback.onPageScrollStateChanged){
                 callback.onPageScrollStateChanged(state);
             }
        }
    };
    mScrollEventAdapter->setOnPageChangeCallback(mPageChangeEventDispatcher);

    // Callback that updates mCurrentItem after swipes. Also triggered in other cases, but in
    // all those cases mCurrentItem will only be overwritten with the same value.
    ViewPager2::OnPageChangeCallback currentItemUpdater;// = new OnPageChangeCallback() {
    currentItemUpdater.onPageSelected=[this](int position) {
            if (mCurrentItem != position) {
                mCurrentItem = position;
                mAccessibilityProvider->onSetNewCurrentItem();
            }
        };

    currentItemUpdater.onPageScrollStateChanged=[this](int newState) {
            if (newState == SCROLL_STATE_IDLE) {
                updateCurrentItem();
            }
        };

    // Prevents focus from remaining on a no-longer visible page
    ViewPager2::OnPageChangeCallback focusClearer;// = new OnPageChangeCallback() {
    focusClearer.onPageSelected=[this](int position) {
            clearFocus();
            if (hasFocus()) { // if clear focus did not succeed
                mRecyclerView->requestFocus(View::FOCUS_FORWARD);
            }
        };

    // Add currentItemUpdater before mExternalPageChangeCallbacks, because we need to update
    // internal state first
    mPageChangeCallbacks.push_back(currentItemUpdater);//mPageChangeEventDispatcher->addOnPageChangeCallback(currentItemUpdater);
    mPageChangeCallbacks.push_back(focusClearer);//mPageChangeEventDispatcher->addOnPageChangeCallback(focusClearer);
    // Allow a11y to register its listeners after currentItemUpdater (so it has the
    // right data). TODO: replace ordering comments with a test.
    mAccessibilityProvider->onInitialize(mPageChangeEventDispatcher, mRecyclerView);

    //mExternalPageChangeCallbacks = new CompositeOnPageChangeCallback(0);
    mExternalPageChangeCallbacks.onPageScrolled=[this](int position, float positionOffset,int positionOffsetPixels){
        for (OnPageChangeCallback& callback : mPageChangeCallbacksExternal) {
            if(callback.onPageScrolled){
                callback.onPageScrolled(position, positionOffset, positionOffsetPixels);
            }
        }
    };
    mExternalPageChangeCallbacks.onPageSelected=[this](int position){
        for (OnPageChangeCallback& callback : mPageChangeCallbacksExternal) {
             if(callback.onPageSelected){
                 callback.onPageSelected(position);
             }
        }
    };
    mExternalPageChangeCallbacks.onPageScrollStateChanged=[this](int state){
        for (OnPageChangeCallback& callback : mPageChangeCallbacksExternal) {
             if(callback.onPageScrollStateChanged){
                 callback.onPageScrollStateChanged(state);
             }
        }
    };

    mPageChangeCallbacks.push_back(mExternalPageChangeCallbacks);//mPageChangeEventDispatcher->addOnPageChangeCallback(*mExternalPageChangeCallbacks);

    // Add mPageTransformerAdapter after mExternalPageChangeCallbacks, because page transform
    // events must be fired after scroll events
    mPageTransformerAdapter = new PageTransformerAdapter(mLayoutManager);
    mPageChangeCallbacks.push_back(*mPageTransformerAdapter);//mPageChangeEventDispatcher->addOnPageChangeCallback(*mPageTransformerAdapter);
}

void ViewPager2::setOrientation(Context* context,const AttributeSet& attrs) {
    setOrientation(ORIENTATION_HORIZONTAL);
    //a.getInt(R.styleable.ViewPager2_android_orientation, ORIENTATION_HORIZONTAL));
}

Parcelable* ViewPager2::onSaveInstanceState() {
    Parcelable* superState = ViewGroup::onSaveInstanceState();
    SavedState* ss = new SavedState(*superState);

    ss->mRecyclerViewId = mRecyclerView->getId();
    ss->mCurrentItem = mPendingCurrentItem == RecyclerView::NO_POSITION ? mCurrentItem : mPendingCurrentItem;

    if (mPendingAdapterState != nullptr) {
        ss->mAdapterState = mPendingAdapterState;
    } else {
        /*RecyclerView::Adapter*adapter = mRecyclerView->getAdapter();
        if (adapter instanceof StatefulAdapter) {
            ss->mAdapterState = ((StatefulAdapter) adapter).saveState();
        }*/
    }

    return ss;
}

void ViewPager2::onRestoreInstanceState(Parcelable& state) {
    if ((dynamic_cast<SavedState*>(&state))==nullptr) {
        ViewGroup::onRestoreInstanceState(state);
        return;
    }

    SavedState& ss = (SavedState&) state;
    //ViewGroup::onRestoreInstanceState(ss.getSuperState());
    mPendingCurrentItem = ss.mCurrentItem;
    mPendingAdapterState = ss.mAdapterState;
}

void ViewPager2::restorePendingState() {
    if (mPendingCurrentItem == RecyclerView::NO_POSITION) {
        // No state to restore, or state is already restored
        return;
    }
    RecyclerView::Adapter*adapter = getAdapter();
    if (adapter == nullptr) {
        return;
    }
    if (mPendingAdapterState != nullptr) {
        /*if (adapter instanceof StatefulAdapter) {
            ((StatefulAdapter) adapter).restoreState(mPendingAdapterState);
        }*/
        mPendingAdapterState = nullptr;
    }
    // Now we have an adapter, we can clamp the pending current item and set it
    mCurrentItem = std::max(0, std::min(mPendingCurrentItem, adapter->getItemCount() - 1));
    mPendingCurrentItem = RecyclerView::NO_POSITION;
    mRecyclerView->scrollToPosition(mCurrentItem);
    mAccessibilityProvider->onRestorePendingState();
}

void ViewPager2::dispatchRestoreInstanceState(SparseArray<Parcelable*>& container) {
    // RecyclerView changed an id, so we need to reflect that in the saved state
    Parcelable* state = container.get(getId());
    if (dynamic_cast<SavedState*>(state)) {
        const int previousRvId = ((SavedState*) state)->mRecyclerViewId;
        const int currentRvId = mRecyclerView->getId();
        container.put(currentRvId, container.get(previousRvId));
        container.remove(previousRvId);
    }

    ViewGroup::dispatchRestoreInstanceState(container);

    // State of ViewPager2 and its child (RecyclerView) has been restored now
    restorePendingState();
}

void ViewPager2::setAdapter(RecyclerView::Adapter* adapter) {
    RecyclerView::Adapter*currentAdapter = mRecyclerView->getAdapter();
    mAccessibilityProvider->onDetachAdapter(currentAdapter);
    unregisterCurrentItemDataSetTracker(currentAdapter);
    mRecyclerView->setAdapter(adapter);
    mCurrentItem = 0;
    restorePendingState();
    mAccessibilityProvider->onAttachAdapter(adapter);
    registerCurrentItemDataSetTracker(adapter);
}

void ViewPager2::registerCurrentItemDataSetTracker(RecyclerView::Adapter*adapter) {
    if (adapter && mCurrentItemDataSetChangeObserver) {
        adapter->registerAdapterDataObserver(mCurrentItemDataSetChangeObserver);
    }
}

void ViewPager2::unregisterCurrentItemDataSetTracker(RecyclerView::Adapter*adapter) {
    if (adapter != nullptr) {
        adapter->unregisterAdapterDataObserver(mCurrentItemDataSetChangeObserver);
    }
}

RecyclerView::Adapter* ViewPager2::getAdapter() {
    return mRecyclerView->getAdapter();
}

void ViewPager2::onViewAdded(View* child) {
    // TODO(b/70666620): consider adding a support for Decor views
    FATAL(" does not support direct child views");
}

void ViewPager2::onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
    measureChild(mRecyclerView, widthMeasureSpec, heightMeasureSpec);
    int width = mRecyclerView->getMeasuredWidth();
    int height = mRecyclerView->getMeasuredHeight();
    int childState = mRecyclerView->getMeasuredState();

    width += getPaddingLeft() + getPaddingRight();
    height += getPaddingTop() + getPaddingBottom();

    width = std::max(width, getSuggestedMinimumWidth());
    height = std::max(height, getSuggestedMinimumHeight());

    setMeasuredDimension(resolveSizeAndState(width, widthMeasureSpec, childState),
            resolveSizeAndState(height, heightMeasureSpec,
                    childState << MEASURED_HEIGHT_STATE_SHIFT));
}

void ViewPager2::onLayout(bool changed, int l, int t, int w, int h) {
    int width = mRecyclerView->getMeasuredWidth();
    int height = mRecyclerView->getMeasuredHeight();
    Rect mTmpContainerRect,mTmpChildRect;

    // TODO(b/70666626): consider delegating padding handling to the RecyclerView to avoid
    // an unnatural page transition effect: http://shortn/_Vnug3yZpQT
    mTmpContainerRect.left = getPaddingLeft();
    mTmpContainerRect.width =w - getPaddingLeft() - getPaddingRight();
    mTmpContainerRect.top = getPaddingTop();
    mTmpContainerRect.height = h - getPaddingTop() - getPaddingBottom();

    Gravity::apply(Gravity::TOP | Gravity::START, width, height, mTmpContainerRect, mTmpChildRect);
    mRecyclerView->layout(mTmpChildRect.left, mTmpChildRect.top, mTmpChildRect.width,
            mTmpChildRect.height);

    if (mCurrentItemDirty) {
        updateCurrentItem();
    }
}

void ViewPager2::updateCurrentItem() {
    if (mPagerSnapHelper == nullptr) {
        FATAL("Design assumption violated.");
    }

    View* snapView = mPagerSnapHelper->findSnapView(*mLayoutManager);
    if (snapView == nullptr) {
        return; // nothing we can do
    }
    const int snapPosition = mLayoutManager->getPosition(snapView);

    if (snapPosition != mCurrentItem && getScrollState() == SCROLL_STATE_IDLE) {
        /** TODO: revisit if push to {@link ScrollEventAdapter} / separate component */
        mPageChangeEventDispatcher.onPageSelected(snapPosition);
    }

    mCurrentItemDirty = false;
}

int ViewPager2::getPageSize()const{
    RecyclerView* rv = mRecyclerView;
    return getOrientation() == ORIENTATION_HORIZONTAL
            ? rv->getWidth() - rv->getPaddingLeft() - rv->getPaddingRight()
            : rv->getHeight() - rv->getPaddingTop() - rv->getPaddingBottom();
}

void ViewPager2::setOrientation(int orientation) {
    mLayoutManager->setOrientation(orientation);
    mAccessibilityProvider->onSetOrientation();
}

int ViewPager2::getOrientation()const{
    return mLayoutManager->getOrientation();
}

bool ViewPager2::isRtl()const{
    return mLayoutManager->getLayoutDirection() == View::LAYOUT_DIRECTION_RTL;
}


void ViewPager2::setCurrentItem(int item) {
    setCurrentItem(item, true);
}

void ViewPager2::setCurrentItem(int item, bool smoothScroll) {
    if (isFakeDragging()) {
        FATAL("Cannot change current item when ViewPager2 is fake dragging");
    }
    setCurrentItemInternal(item, smoothScroll);
}

void ViewPager2::setCurrentItemInternal(int item, bool smoothScroll) {

    // 1. Preprocessing (check state, validate item, decide if update is necessary, etc)

    RecyclerView::Adapter* adapter = getAdapter();
    if (adapter == nullptr) {
        // Update the pending current item if we're still waiting for the adapter
        if (mPendingCurrentItem != RecyclerView::NO_POSITION) {
            mPendingCurrentItem = std::max(item, 0);
        }
        return;
    }
    if (adapter->getItemCount() <= 0) {
        // Adapter is empty
        return;
    }
    item = std::max(item, 0);
    item = std::min(item, adapter->getItemCount() - 1);

    if (item == mCurrentItem && mScrollEventAdapter->isIdle()) {
        // Already at the correct page
        return;
    }
    if (item == mCurrentItem && smoothScroll) {
        // Already scrolling to the correct page, but not yet there. Only handle instant scrolls
        // because then we need to interrupt the current smooth scroll.
        return;
    }

    // 2. Update the item internally

    double previousItem = mCurrentItem;
    mCurrentItem = item;
    mAccessibilityProvider->onSetNewCurrentItem();

    if (!mScrollEventAdapter->isIdle()) {
        // Scroll in progress, overwrite previousItem with actual current position
        previousItem = mScrollEventAdapter->getRelativeScrollPosition();
    }

    // 3. Perform the necessary scroll actions on RecyclerView

    mScrollEventAdapter->notifyProgrammaticScroll(item, smoothScroll);
    if (!smoothScroll) {
        mRecyclerView->scrollToPosition(item);
        return;
    }

    // For smooth scroll, pre-jump to nearby item for long jumps.
    if (std::abs(item - previousItem) > 3) {
        mRecyclerView->scrollToPosition(item > previousItem ? item - 3 : item + 3);
        // TODO(b/114361680): call smoothScrollToPosition synchronously (blocked by b/114019007)
        Runnable run([this,item](){
            mRecyclerView->smoothScrollToPosition(item);
        });
        mRecyclerView->post(run);//new SmoothScrollToPosition(item, mRecyclerView));
    } else {
        mRecyclerView->smoothScrollToPosition(item);
    }
}

int ViewPager2::getCurrentItem() const{
    return mCurrentItem;
}

int ViewPager2::getScrollState() const{
    return mScrollEventAdapter->getScrollState();
}

bool ViewPager2::beginFakeDrag() {
    return mFakeDragger->beginFakeDrag();
}


bool ViewPager2::fakeDragBy(float offsetPxFloat) {
    return mFakeDragger->fakeDragBy(offsetPxFloat);
}

bool ViewPager2::endFakeDrag() {
    return mFakeDragger->endFakeDrag();
}

bool ViewPager2::isFakeDragging() {
    return mFakeDragger->isFakeDragging();
}

void ViewPager2::snapToPage() {
    // Method copied from PagerSnapHelper#snapToTargetExistingView
    // When fixing something here, make sure to update that method as well
    View* view = mPagerSnapHelper->findSnapView(*mLayoutManager);
    if (view == nullptr) {
        return;
    }
    int snapDistance[2];
    mPagerSnapHelper->calculateDistanceToFinalSnap(*mLayoutManager,*view,snapDistance);
    //noinspection ConstantConditions
    if ((snapDistance[0] != 0) || (snapDistance[1] != 0)){
        mRecyclerView->smoothScrollBy(snapDistance[0], snapDistance[1]);
    }
}

void ViewPager2::setUserInputEnabled(bool enabled) {
    mUserInputEnabled = enabled;
    mAccessibilityProvider->onSetUserInputEnabled();
}

bool ViewPager2::isUserInputEnabled()const{
    return mUserInputEnabled;
}

void ViewPager2::setOffscreenPageLimit(int limit) {
    if (limit < 1 && limit != OFFSCREEN_PAGE_LIMIT_DEFAULT) {
        FATAL("Offscreen page limit must be OFFSCREEN_PAGE_LIMIT_DEFAULT or a number > 0");
    }
    mOffscreenPageLimit = limit;
    // Trigger layout so prefetch happens through getExtraLayoutSize()
    mRecyclerView->requestLayout();
}

int ViewPager2::getOffscreenPageLimit()const{
    return mOffscreenPageLimit;
}

bool ViewPager2::canScrollHorizontally(int direction)const{
    return mRecyclerView->canScrollHorizontally(direction);
}

bool ViewPager2::canScrollVertically(int direction)const{
    return mRecyclerView->canScrollVertically(direction);
}

void ViewPager2::registerOnPageChangeCallback(const ViewPager2::OnPageChangeCallback& callback) {
    //mExternalPageChangeCallbacks->addOnPageChangeCallback(callback);
    mPageChangeCallbacksExternal.push_back(callback);
}

void ViewPager2::unregisterOnPageChangeCallback(const ViewPager2::OnPageChangeCallback& callback) {
    //mExternalPageChangeCallbacks->removeOnPageChangeCallback(callback);
    auto it = std::find(mPageChangeCallbacksExternal.begin(),mPageChangeCallbacksExternal.end(),callback);
    if(it!=mPageChangeCallbacksExternal.end()){
        mPageChangeCallbacksExternal.erase(it);
    }
}

void ViewPager2::setPageTransformer(PageTransformer* transformer) {
    if (transformer != nullptr) {
        if (!mSavedItemAnimatorPresent) {
            mSavedItemAnimator = mRecyclerView->getItemAnimator();
            mSavedItemAnimatorPresent = true;
        }
        mRecyclerView->setItemAnimator(nullptr);
    } else {
        if (mSavedItemAnimatorPresent) {
            mRecyclerView->setItemAnimator(mSavedItemAnimator);
            mSavedItemAnimator = nullptr;
            mSavedItemAnimatorPresent = false;
        }
    }

    // TODO: add support for reverseDrawingOrder: b/112892792
    // TODO: add support for pageLayerType: b/112893074
    if (transformer == mPageTransformerAdapter->getPageTransformer()) {
        return;
    }
    mPageTransformerAdapter->setPageTransformer(transformer);
    requestTransform();
}

void ViewPager2::requestTransform() {
    if (mPageTransformerAdapter->getPageTransformer() == nullptr) {
        return;
    }
    double relativePosition = mScrollEventAdapter->getRelativeScrollPosition();
    int position = (int) relativePosition;
    float positionOffset = (float) (relativePosition - position);
    int positionOffsetPx = std::round(getPageSize() * positionOffset);
    mPageTransformerAdapter->onPageScrolled(position, positionOffset, positionOffsetPx);
}

void ViewPager2::setLayoutDirection(int layoutDirection) {
    ViewGroup::setLayoutDirection(layoutDirection);
    mAccessibilityProvider->onSetLayoutDirection();
}

void ViewPager2::onInitializeAccessibilityNodeInfo(AccessibilityNodeInfo& info) {
    ViewGroup::onInitializeAccessibilityNodeInfo(info);
    mAccessibilityProvider->onInitializeAccessibilityNodeInfo(info);
}

bool ViewPager2::performAccessibilityAction(int action, Bundle* arguments) {
    if (mAccessibilityProvider->handlesPerformAccessibilityAction(action, arguments)) {
        return mAccessibilityProvider->onPerformAccessibilityAction(action, arguments);
    }
    return ViewGroup::performAccessibilityAction(action, arguments);
}

ViewPager2::RecyclerViewImpl::RecyclerViewImpl(Context* context,const AttributeSet&attr,ViewPager2*vp)
    :RecyclerView(context,attr),mVP(vp){
}

std::string ViewPager2::RecyclerViewImpl::getAccessibilityClassName() const{
    if (mVP->mAccessibilityProvider->handlesRvGetAccessibilityClassName()) {
        return mVP->mAccessibilityProvider->onRvGetAccessibilityClassName();
    }
    return ViewGroup::getAccessibilityClassName();
}

void ViewPager2::RecyclerViewImpl::onInitializeAccessibilityEvent(AccessibilityEvent& event) {
    ViewGroup::onInitializeAccessibilityEvent(event);
    event.setFromIndex(mVP->mCurrentItem);
    event.setToIndex(mVP->mCurrentItem);
    mVP->mAccessibilityProvider->onRvInitializeAccessibilityEvent(event);
}

bool ViewPager2::RecyclerViewImpl::onTouchEvent(MotionEvent& event) {
    return mVP->isUserInputEnabled() && RecyclerView::onTouchEvent(event);
}

bool ViewPager2::RecyclerViewImpl::onInterceptTouchEvent(MotionEvent& ev) {
    return mVP->isUserInputEnabled() && RecyclerView::onInterceptTouchEvent(ev);
}

/////////////////////////////////////////////////LinearLayoutManagerImpl/////////////////////////////////////////////////////

ViewPager2::LinearLayoutManagerImpl::LinearLayoutManagerImpl(Context* context,ViewPager2*vp)
    :LinearLayoutManager(context){
    mVP = vp;
}

bool ViewPager2::LinearLayoutManagerImpl::performAccessibilityAction(RecyclerView::Recycler& recycler,
        RecyclerView::State& state, int action, Bundle* args) {
    if (mVP->mAccessibilityProvider->handlesLmPerformAccessibilityAction(action)) {
        return mVP->mAccessibilityProvider->onLmPerformAccessibilityAction(action);
    }
    return /*Linear*/LayoutManager::performAccessibilityAction(recycler, state, action, args);
}

void ViewPager2::LinearLayoutManagerImpl::onInitializeAccessibilityNodeInfo(RecyclerView::Recycler& recycler,
        RecyclerView::State& state, AccessibilityNodeInfo&info) {
    LinearLayoutManager::onInitializeAccessibilityNodeInfo(recycler, state, info);
    mVP->mAccessibilityProvider->onLmInitializeAccessibilityNodeInfo(info);
}

void ViewPager2::LinearLayoutManagerImpl::calculateExtraLayoutSpace(RecyclerView::State& state,
        int extraLayoutSpace[2]) {
    const int pageLimit = mVP->getOffscreenPageLimit();
    if (pageLimit == OFFSCREEN_PAGE_LIMIT_DEFAULT) {
        // Only do custom prefetching of offscreen pages if requested
        LinearLayoutManager::calculateExtraLayoutSpace(state, extraLayoutSpace);
        return;
    }
    const int offscreenSpace = mVP->getPageSize() * pageLimit;
    extraLayoutSpace[0] = offscreenSpace;
    extraLayoutSpace[1] = offscreenSpace;
}

bool ViewPager2::LinearLayoutManagerImpl::requestChildRectangleOnScreen(RecyclerView& parent,
        View& child,const Rect& rect, bool immediate, bool focusedChildVisible) {
    return false; // users should use setCurrentItem instead
}

ViewPager2::PagerSnapHelperImpl::PagerSnapHelperImpl():PagerSnapHelper(){
}

View* ViewPager2::PagerSnapHelperImpl::findSnapView(RecyclerView::LayoutManager& layoutManager) {
    // When interrupting a smooth scroll with a fake drag, we stop RecyclerView's scroll
    // animation, which fires a scroll state change to IDLE. PagerSnapHelper then kicks in
    // to snap to a page, which we need to prevent here.
    // Simplifying that case: during a fake drag, no snapping should occur.
    ViewPager2*vp = dynamic_cast<ViewPager2*>(mRecyclerView->getParent());
    return vp->isFakeDragging() ? nullptr : PagerSnapHelper::findSnapView(layoutManager);
}


void ViewPager2::addItemDecoration(RecyclerView::ItemDecoration* decor) {
    mRecyclerView->addItemDecoration(decor);
}

void ViewPager2::addItemDecoration(RecyclerView::ItemDecoration* decor, int index) {
    mRecyclerView->addItemDecoration(decor, index);
}

RecyclerView::ItemDecoration* ViewPager2::getItemDecorationAt(int index) {
    return mRecyclerView->getItemDecorationAt(index);
}

int ViewPager2::getItemDecorationCount() const{
    return mRecyclerView->getItemDecorationCount();
}

void ViewPager2::invalidateItemDecorations() {
    mRecyclerView->invalidateItemDecorations();
}

void ViewPager2::removeItemDecorationAt(int index) {
    mRecyclerView->removeItemDecorationAt(index);
}

void ViewPager2::removeItemDecoration(RecyclerView::ItemDecoration* decor) {
    mRecyclerView->removeItemDecoration(decor);
}

ViewPager2::SavedState::SavedState(Parcel& source):BaseSavedState(source){
    //readValues(source, nullptr);
}

ViewPager2::SavedState::SavedState(Parcelable& superState):BaseSavedState(&superState){
}

/*void ViewPager2::SavedState::readValues(Parcel& source, ClassLoader loader) {
    mRecyclerViewId = source.readInt();
    mCurrentItem = source.readInt();
    mAdapterState = source.readParcelable(loader);
}*/

void ViewPager2::SavedState::writeToParcel(Parcel& out, int flags) {
    //ViewGroup::writeToParcel(out, flags);
    out.writeInt(mRecyclerViewId);
    out.writeInt(mCurrentItem);
    //out.writeParcelable(mAdapterState, flags);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////ViewPager2::AccessibilityProvider///////////////////////////////////////
ViewPager2::AccessibilityProvider::AccessibilityProvider(ViewPager2*v)
    :mVP(v){
}

void ViewPager2::AccessibilityProvider::onInitialize(OnPageChangeCallback pageChangeEventDispatcher,RecyclerView* recyclerView){
}

bool ViewPager2::AccessibilityProvider::handlesGetAccessibilityClassName(){
    return "";
}

std::string ViewPager2::AccessibilityProvider::onGetAccessibilityClassName() {
    throw std::runtime_error("Not implemented.");
    return "";
}

void ViewPager2::AccessibilityProvider::onRestorePendingState(){
}

void ViewPager2::AccessibilityProvider::onAttachAdapter(RecyclerView::Adapter*newAdapter){
}

void ViewPager2::AccessibilityProvider::onDetachAdapter(RecyclerView::Adapter*oldAdapter){
}

void ViewPager2::AccessibilityProvider::onSetOrientation(){
}

void ViewPager2::AccessibilityProvider::onSetNewCurrentItem(){
}

void ViewPager2::AccessibilityProvider::onSetUserInputEnabled(){
}

void ViewPager2::AccessibilityProvider::onSetLayoutDirection(){
}

void ViewPager2::AccessibilityProvider::onInitializeAccessibilityNodeInfo(AccessibilityNodeInfo& info) {
}

bool ViewPager2::AccessibilityProvider::handlesPerformAccessibilityAction(int action, Bundle* arguments) {
    return false;
}

bool ViewPager2::AccessibilityProvider::onPerformAccessibilityAction(int action, Bundle* arguments) {
    throw std::runtime_error("Not implemented.");
}

void ViewPager2::AccessibilityProvider::onRvInitializeAccessibilityEvent(AccessibilityEvent& event){
}

bool ViewPager2::AccessibilityProvider::handlesLmPerformAccessibilityAction(int action) {
    return false;
}

bool ViewPager2::AccessibilityProvider::onLmPerformAccessibilityAction(int action) {
    throw std::runtime_error("Not implemented.");
}

void ViewPager2::AccessibilityProvider::onLmInitializeAccessibilityNodeInfo(AccessibilityNodeInfo&info) {
}

bool ViewPager2::AccessibilityProvider::handlesRvGetAccessibilityClassName() {
    return false;
}

std::string ViewPager2::AccessibilityProvider::onRvGetAccessibilityClassName() {
    throw std::runtime_error("Not implemented.");
    return "";
}


///////////////////class BasicAccessibilityProvider extends AccessibilityProvider///////////////////
ViewPager2::BasicAccessibilityProvider::BasicAccessibilityProvider(ViewPager2*vp):AccessibilityProvider(vp){
}
bool ViewPager2::BasicAccessibilityProvider::handlesLmPerformAccessibilityAction(int action) {
    return (action == AccessibilityNodeInfo::ACTION_SCROLL_BACKWARD
            || action == AccessibilityNodeInfo::ACTION_SCROLL_FORWARD)
            && !mVP->isUserInputEnabled();
}

bool ViewPager2::BasicAccessibilityProvider::onLmPerformAccessibilityAction(int action) {
    if (!handlesLmPerformAccessibilityAction(action)) {
        throw std::runtime_error("IllegalStateException");
    }
    return false;
}

void ViewPager2::BasicAccessibilityProvider::onLmInitializeAccessibilityNodeInfo(AccessibilityNodeInfo& info) {
    if (mVP->isUserInputEnabled()) {
        info.removeAction(AccessibilityNodeInfo::ACTION_SCROLL_BACKWARD);
        info.removeAction(AccessibilityNodeInfo::ACTION_SCROLL_FORWARD);
        info.setScrollable(false);
    }
}

bool ViewPager2::BasicAccessibilityProvider::handlesRvGetAccessibilityClassName() {
    return true;
}

std::string ViewPager2::BasicAccessibilityProvider::onRvGetAccessibilityClassName() {
    if (!handlesRvGetAccessibilityClassName()) {
        throw std::runtime_error("IllegalStateException");
    }
    return "androidx.viewpager.widget.ViewPager";
}

////////////////////class PageAwareAccessibilityProvider extends AccessibilityProvider//////////////////////////////
/*private final AccessibilityViewCommand mActionPageForward =
    new AccessibilityViewCommand() {
        @Override
        public bool perform(@NonNull View view,
                @Nullable CommandArguments arguments) {
            ViewPager2 viewPager = (ViewPager2) view;
            setCurrentItemFromAccessibilityCommand(viewPager.getCurrentItem() + 1);
            return true;
        }
    };

private final AccessibilityViewCommand mActionPageBackward =
    new AccessibilityViewCommand() {
        @Override
        public bool perform(@NonNull View view,
                @Nullable CommandArguments arguments) {
            ViewPager2 viewPager = (ViewPager2) view;
            setCurrentItemFromAccessibilityCommand(viewPager.getCurrentItem() - 1);
            return true;
        }
    };*/

ViewPager2::PageAwareAccessibilityProvider::PageAwareAccessibilityProvider(ViewPager2*v)
    :AccessibilityProvider(v){
    mAdapterDataObserver = nullptr;
}

void ViewPager2::PageAwareAccessibilityProvider::onInitialize(OnPageChangeCallback pageChangeEventDispatcher,RecyclerView* recyclerView) {

    class MyDataSetChangeObserver:public ViewPager2::DataSetChangeObserver{
        PageAwareAccessibilityProvider*mPP;
    public:
        MyDataSetChangeObserver(ViewPager2*v,PageAwareAccessibilityProvider*p)
            :DataSetChangeObserver(v),mPP(p){
        }
        void onChanged()override{
            mPP->updatePageAccessibilityActions();
        }
    };

    recyclerView->setImportantForAccessibility(View::IMPORTANT_FOR_ACCESSIBILITY_NO);
    mAdapterDataObserver = new MyDataSetChangeObserver(mVP,this);

    if (mVP->getImportantForAccessibility() == View::IMPORTANT_FOR_ACCESSIBILITY_AUTO) {
        mVP->setImportantForAccessibility(View::IMPORTANT_FOR_ACCESSIBILITY_YES);
    }
}

bool ViewPager2::PageAwareAccessibilityProvider::handlesGetAccessibilityClassName() {
    return true;
}

std::string ViewPager2::PageAwareAccessibilityProvider::onGetAccessibilityClassName() {
    if (!handlesGetAccessibilityClassName()) {
        throw std::runtime_error("IllegalStateException");
    }
    return "androidx.viewpager.widget.ViewPager";
}

void ViewPager2::PageAwareAccessibilityProvider::onRestorePendingState() {
    updatePageAccessibilityActions();
}

void ViewPager2::PageAwareAccessibilityProvider::onAttachAdapter(RecyclerView::Adapter*newAdapter) {
    updatePageAccessibilityActions();
    if (newAdapter != nullptr) {
        newAdapter->registerAdapterDataObserver(mAdapterDataObserver);
    }
}

void ViewPager2::PageAwareAccessibilityProvider::onDetachAdapter(RecyclerView::Adapter*oldAdapter) {
    if (oldAdapter != nullptr) {
        oldAdapter->unregisterAdapterDataObserver(mAdapterDataObserver);
    }
}

void ViewPager2::PageAwareAccessibilityProvider::onSetOrientation() {
    updatePageAccessibilityActions();
}

void ViewPager2::PageAwareAccessibilityProvider::onSetNewCurrentItem() {
    updatePageAccessibilityActions();
}

void ViewPager2::PageAwareAccessibilityProvider::onSetUserInputEnabled() {
    updatePageAccessibilityActions();
    if (false){//Build.VERSION.SDK_INT < 21) {
        mVP->sendAccessibilityEvent(AccessibilityEvent::TYPE_WINDOW_CONTENT_CHANGED);
    }
}

void ViewPager2::PageAwareAccessibilityProvider::onSetLayoutDirection() {
    updatePageAccessibilityActions();
}

void ViewPager2::PageAwareAccessibilityProvider::onInitializeAccessibilityNodeInfo(AccessibilityNodeInfo& info) {
    addCollectionInfo(info);
    if (true){//Build.VERSION.SDK_INT >= 16) {
        addScrollActions(info);
    }
}

bool ViewPager2::PageAwareAccessibilityProvider::handlesPerformAccessibilityAction(int action, Bundle* arguments) {
    return (action == AccessibilityNodeInfo::ACTION_SCROLL_BACKWARD)
            || (action == AccessibilityNodeInfo::ACTION_SCROLL_FORWARD);
}

bool ViewPager2::PageAwareAccessibilityProvider::onPerformAccessibilityAction(int action, Bundle* arguments) {
    if (!handlesPerformAccessibilityAction(action, arguments)) {
        throw std::runtime_error(" IllegalStateException");
    }

    const int nextItem = (action == AccessibilityNodeInfo::ACTION_SCROLL_BACKWARD)
            ? (mVP->getCurrentItem() - 1) : (mVP->getCurrentItem() + 1);
    setCurrentItemFromAccessibilityCommand(nextItem);
    return true;
}

void ViewPager2::PageAwareAccessibilityProvider::onRvInitializeAccessibilityEvent(AccessibilityEvent& event) {
    event.setSource(mVP);
    event.setClassName(onGetAccessibilityClassName());
}

void ViewPager2::PageAwareAccessibilityProvider::setCurrentItemFromAccessibilityCommand(int item) {
    if (mVP->isUserInputEnabled()) {
        mVP->setCurrentItemInternal(item, true);
    }
}

void ViewPager2::PageAwareAccessibilityProvider::updatePageAccessibilityActions() {
    ViewPager2* viewPager = mVP;
#if 0
    constexpr int actionIdPageLeft = R::id::accessibilityActionPageLeft;
    constexpr int actionIdPageRight = R::id::accessibilityActionPageRight;
    constexpr int actionIdPageUp = R::id::accessibilityActionPageUp;
    constexpr int actionIdPageDown = R::id::accessibilityActionPageDown;

    viewPager->removeAccessibilityAction(actionIdPageLeft);
    viewPager->removeAccessibilityAction(actionIdPageRight);
    viewPager->removeAccessibilityAction(actionIdPageUp);
    viewPager->removeAccessibilityAction(actionIdPageDown);

    if (mVP->getAdapter() == nullptr) {
        return;
    }

    int itemCount = mVP->getAdapter()->getItemCount();
    if (itemCount == 0) {
        return;
    }

    if (!mVP->isUserInputEnabled()) {
        return;
    }

    if (mVP->getOrientation() == ORIENTATION_HORIZONTAL) {
        bool isLayoutRtl = mVP->isRtl();
        int actionIdPageForward = isLayoutRtl ? actionIdPageLeft : actionIdPageRight;
        int actionIdPageBackward = isLayoutRtl ? actionIdPageRight : actionIdPageLeft;

        if (mVP->mCurrentItem < itemCount - 1) {
            viewPager->replaceAccessibilityAction(
                    new AccessibilityNodeInfo::AccessibilityAction(actionIdPageForward, nullptr), nullptr,
                    mActionPageForward);
        }
        if (mVP->mCurrentItem > 0) {
            viewPager->replaceAccessibilityAction(
                    new AccessibilityNodeInfo::AccessibilityAction(actionIdPageBackward, nullptr), nullptr,
                    mActionPageBackward);
        }
    } else {
        if (mVP->mCurrentItem < itemCount - 1) {
            viewPager->replaceAccessibilityAction(
                    new AccessibilityNodeInfo::AccessibilityAction(actionIdPageDown, nullptr), nullptr,
                    mActionPageForward);
        }
        if (mVP->mCurrentItem > 0) {
            viewPager->replaceAccessibilityAction(
                    new AccessibilityNodeInfo::AccessibilityAction(actionIdPageUp, nullptr), nullptr,
                    mActionPageBackward);
        }
    }
#endif
}

void ViewPager2::PageAwareAccessibilityProvider::addCollectionInfo(AccessibilityNodeInfo& info) {
    int rowCount = 0;
    int colCount = 0;
    if (mVP->getAdapter() != nullptr) {
        if (mVP->getOrientation() == ORIENTATION_VERTICAL) {
            rowCount = mVP->getAdapter()->getItemCount();
        } else {
            colCount = mVP->getAdapter()->getItemCount();
        }
    }
    AccessibilityNodeInfo::CollectionInfo* collectionInfo =
            AccessibilityNodeInfo::CollectionInfo::obtain(rowCount, colCount,/* hierarchical= */false,
                    AccessibilityNodeInfo::CollectionInfo::SELECTION_MODE_NONE);
    info.setCollectionInfo(collectionInfo);
}

void ViewPager2::PageAwareAccessibilityProvider::addScrollActions(AccessibilityNodeInfo& info) {
    RecyclerView::Adapter*adapter = mVP->getAdapter();
    if (adapter == nullptr) {
        return;
    }
    int itemCount = adapter->getItemCount();
    if (itemCount == 0 || !mVP->isUserInputEnabled()) {
        return;
    }
    if (mVP->mCurrentItem > 0) {
        info.addAction(AccessibilityNodeInfo::ACTION_SCROLL_BACKWARD);
    }
    if (mVP->mCurrentItem < itemCount - 1) {
        info.addAction(AccessibilityNodeInfo::ACTION_SCROLL_FORWARD);
    }
    info.setScrollable(true);
}

ViewPager2::DataSetChangeObserver::DataSetChangeObserver(ViewPager2*v)
    :mVP(v){
}

void ViewPager2::DataSetChangeObserver::onChanged(){
}

void ViewPager2::DataSetChangeObserver::onItemRangeChanged(int positionStart, int itemCount){
    onChanged();
}

void ViewPager2::DataSetChangeObserver::onItemRangeChanged(int positionStart, int itemCount,
        Object* payload){
    onChanged();
}

void ViewPager2::DataSetChangeObserver::onItemRangeInserted(int positionStart, int itemCount) {
    onChanged();
}

void ViewPager2::DataSetChangeObserver::onItemRangeRemoved(int positionStart, int itemCount){
    onChanged();
}

void ViewPager2::DataSetChangeObserver::onItemRangeMoved(int fromPosition, int toPosition, int itemCount){
    onChanged();
}


}/*endof namespace*/

