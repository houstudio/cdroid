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
#include <widgetEx/recyclerview/childhelper.h>
#include <widgetEx/recyclerview/viewinfostore.h>
#include <widgetEx/recyclerview/adapterhelper.h>
#include <widgetEx/recyclerview/gridlayoutmanager.h>
#include <widgetEx/recyclerview/defaultitemanimator.h>
#include <widgetEx/recyclerview/linearlayoutmanager.h>
#include <widgetEx/recyclerview/staggeredgridlayoutmanager.h>
#include <widgetEx/recyclerview/recyclerviewaccessibilitydelegate.h>
#include <widgetEx/recyclerview/fastscroller.h>
#include <view/focusfinder.h>
#include <core/neverdestroyed.h>
#include <core/mathutils.h>
#include <core/build.h>
#include <cassert>

namespace cdroid{

//public class RecyclerView extends ViewGroup implements ScrollingView, NestedScrollingChild2 {
class QuinticInterpolator:public Interpolator{
public:
    float getInterpolation(float t)override{
        t -= 1.0f;
        return t * t * t * t * t + 1.0f;
    }
};

bool RecyclerView::sDebugAssertionsEnabled= false;
bool RecyclerView::sVerboseLoggingEnabled = false;
static NeverDestroyed<QuinticInterpolator> sQuinticInterpolator;

DECLARE_WIDGET2(RecyclerView,"cdroid:attr/recyclerviewStyle")

RecyclerView::RecyclerView(int w,int h):ViewGroup(w,h){
    initRecyclerView();
    initAdapterManager();
    initChildrenHelper();
    initAutofill();
    // If not explicitly specified this view is important for accessibility.
    if (getImportantForAccessibility()
            == View::IMPORTANT_FOR_ACCESSIBILITY_AUTO) {
        setImportantForAccessibility(View::IMPORTANT_FOR_ACCESSIBILITY_YES);
    }
    setAccessibilityDelegate(new RecyclerViewAccessibilityDelegate(this));
    
    //Create the layoutManager if specified.
    AttributeSet attrs(getContext(),getContext()->getPackageName());
    std::string layoutManagerName = attrs.getString("layoutManager","LinearLayoutManager");
    const int descendantFocusability = attrs.getInt("descendantFocusability", -1);
    if (descendantFocusability == -1) {
        setDescendantFocusability(ViewGroup::FOCUS_AFTER_DESCENDANTS);
    }
    mEnableFastScroller = attrs.getBoolean("fastScrollEnabled", false);
    if (mEnableFastScroller) {
        StateListDrawable* verticalThumbDrawable = (StateListDrawable*) attrs.getDrawable("fastScrollVerticalThumbDrawable");
        Drawable* verticalTrackDrawable = attrs.getDrawable("fastScrollVerticalTrackDrawable");
        StateListDrawable* horizontalThumbDrawable = (StateListDrawable*) attrs.getDrawable("fastScrollHorizontalThumbDrawable");
        Drawable* horizontalTrackDrawable = attrs.getDrawable("fastScrollHorizontalTrackDrawable");
        initFastScroller(verticalThumbDrawable, verticalTrackDrawable, horizontalThumbDrawable, horizontalTrackDrawable,attrs);
    }
    createLayoutManager(getContext(), layoutManagerName, attrs);//, defStyle, defStyleRes);
    setDescendantFocusability(descendantFocusability==-1?ViewGroup::FOCUS_AFTER_DESCENDANTS:ViewGroup::FOCUS_AFTER_DESCENDANTS);

    // Re-set whether nested scrolling is enabled so that it is set on all API levels
    setNestedScrollingEnabled(attrs.getBoolean("nestedScrollingEnabled", true));
    setWillNotDraw(getOverScrollMode() == View::OVER_SCROLL_NEVER);
}

RecyclerView::RecyclerView(Context* context,const AttributeSet& attrs)
   :ViewGroup(context, attrs){

    initRecyclerView();
    initAdapterManager();
    initChildrenHelper();
    initAutofill();
    mClipToPadding = attrs.getBoolean("clipToPadding", true);
    // If not explicitly specified this view is important for accessibility.
    if (getImportantForAccessibility()
            == View::IMPORTANT_FOR_ACCESSIBILITY_AUTO) {
        setImportantForAccessibility(View::IMPORTANT_FOR_ACCESSIBILITY_YES);
    }
    setAccessibilityDelegate(new RecyclerViewAccessibilityDelegate(this));
    // Create the layoutManager if specified.

    std::string layoutManagerName = attrs.getString("layoutManager");
    const int descendantFocusability = attrs.getInt("descendantFocusability", -1);
    if (descendantFocusability == -1) {
        setDescendantFocusability(ViewGroup::FOCUS_AFTER_DESCENDANTS);
    }
    mEnableFastScroller = attrs.getBoolean("fastScrollEnabled", false);
    if (mEnableFastScroller) {
        StateListDrawable* verticalThumbDrawable = (StateListDrawable*) attrs.getDrawable("fastScrollVerticalThumbDrawable");
        Drawable* verticalTrackDrawable = attrs.getDrawable("fastScrollVerticalTrackDrawable");
        StateListDrawable* horizontalThumbDrawable = (StateListDrawable*) attrs.getDrawable("fastScrollHorizontalThumbDrawable");
        Drawable* horizontalTrackDrawable = attrs.getDrawable("fastScrollHorizontalTrackDrawable");
        initFastScroller(verticalThumbDrawable, verticalTrackDrawable, horizontalThumbDrawable, horizontalTrackDrawable,attrs);
    }
    createLayoutManager(context, layoutManagerName, attrs);//, defStyle, defStyleRes);
    setDescendantFocusability(descendantFocusability==-1?ViewGroup::FOCUS_AFTER_DESCENDANTS:ViewGroup::FOCUS_AFTER_DESCENDANTS);

    // Re-set whether nested scrolling is enabled so that it is set on all API levels
    setNestedScrollingEnabled(attrs.getBoolean("nestedScrollingEnabled", true));
    setWillNotDraw(getOverScrollMode() == View::OVER_SCROLL_NEVER);
}

RecyclerView::~RecyclerView(){
    for(ItemDecoration*id:mItemDecorations){
        delete id;
    }
    mItemDecorations.clear();
    if(mVelocityTracker)
        mVelocityTracker->recycle();
    delete mChildHelper;
    delete mState;
    delete mLayout;
    delete mAdapterHelper;
    delete mViewInfoStore;
    delete mViewFlinger;
    delete mItemAnimator;
    delete mRecycler;
    delete mObserver;
    delete mLeftGlow;
    delete mTopGlow;
    delete mRightGlow;
    delete mBottomGlow;
    delete mEdgeEffectFactory;
    delete mScrollingChildHelper;
    delete mScrollFeedbackProvider;
    delete mPendingSavedState;
    delete mAccessibilityDelegate;
    delete (GapWorker::LayoutPrefetchRegistryImpl*)mPrefetchRegistry;
    delete (ViewInfoStore::ProcessCallback*)mViewInfoProcessCallback;
}

void RecyclerView::initRecyclerView(){
    mIsAttached   = false;
    mHasFixedSize = true;
    mLayoutSuppressed = false;
    mEnableFastScroller = false;
    mClipToPadding = true;
    mIgnoreMotionEventTillDown = false;
    mPostedAnimatorRunner = false;
    mDataSetHasChangedAfterLayout = false;
    mAdapterUpdateDuringMeasure = false;
    mLowResRotaryEncoderFeature = false;
    mEatenAccessibilityChangeFlags =0;
    mInterceptRequestLayoutDepth = 0;
    mState = new State();
    mGapWorker     = nullptr;
    mPrefetchRegistry = new GapWorker::LayoutPrefetchRegistryImpl();
    mAdapterHelper = nullptr;
    mViewInfoStore = new ViewInfoStore();
    mViewFlinger = new ViewFlinger(this);
    mItemAnimator= nullptr;
    mRecycler = new Recycler(this);
    mPendingSavedState = nullptr;
    mObserver = new RecyclerViewDataObserver(this);
    mLeftGlow = mTopGlow = nullptr;
    mRightGlow = mBottomGlow = nullptr;
    mVelocityTracker = nullptr;
    mLayout = nullptr;
    mAdapter= nullptr;
    mAccessibilityDelegate = nullptr;
    mInterceptingOnItemTouchListener = nullptr;
    mInterceptingOnItemTouchListener = nullptr;
    mMinMaxLayoutPositions[0] = mMinMaxLayoutPositions[1] = 0;
    mScrollOffset[0] = mScrollOffset[1] = 0;
    mScrollConsumed[0] = mScrollConsumed[1] = 0;
    mNestedOffsets[0] = mNestedOffsets[1] = 0;
    mScrollStepConsumed[0] = mScrollStepConsumed[1] = 0;
    mScrollingChildHelper = nullptr;
    mScrollFeedbackProvider= nullptr;
    mFirstLayoutComplete = false;
    mEdgeEffectFactory = new EdgeEffectFactory();
    mAccessibilityManager = &AccessibilityManager::getInstance(getContext());
    mItemAnimatorListener = std::bind(&RecyclerView::doAnimatorFinished,this,std::placeholders::_1);
    setItemAnimator(new DefaultItemAnimator());

    setScrollContainer(true);
    setFocusableInTouchMode(true);

    ViewConfiguration&vc = ViewConfiguration::get(getContext());
    mTouchSlop = vc.getScaledTouchSlop();
    mScaledHorizontalScrollFactor = vc.getScaledHorizontalScrollFactor();
    mScaledVerticalScrollFactor = vc.getScaledVerticalScrollFactor();
    mMinFlingVelocity = vc.getScaledMinimumFlingVelocity();
    mMaxFlingVelocity = vc.getScaledMaximumFlingVelocity();
    ViewInfoStore::ProcessCallback* visCBK = new ViewInfoStore::ProcessCallback;

    visCBK->processDisappeared = [this](ViewHolder* viewHolder,ItemAnimator::ItemHolderInfo* info, ItemAnimator::ItemHolderInfo* postInfo){
        mRecycler->unscrapView(*viewHolder);
        animateDisappearance(*viewHolder, *info, postInfo);
        //delete info; delete postInfo;
    };
    visCBK->processAppeared = [this](ViewHolder* viewHolder, ItemAnimator::ItemHolderInfo* preInfo, ItemAnimator::ItemHolderInfo* info){
        animateAppearance(*viewHolder, preInfo, *info);
        //delete preInfo; delete info;
    };

    visCBK->processPersistent = [this](ViewHolder* viewHolder,ItemAnimator::ItemHolderInfo* preInfo,ItemAnimator::ItemHolderInfo* postInfo){
        viewHolder->setIsRecyclable(false);
        if (mDataSetHasChangedAfterLayout) {
            if (mItemAnimator->animateChange(*viewHolder, *viewHolder, *preInfo, *postInfo)) {
                postAnimationRunner();
            }
        } else if (mItemAnimator->animatePersistence(*viewHolder, *preInfo, *postInfo)) {
            postAnimationRunner();
        }
        //delete preInfo; delete postInfo;
    };
    visCBK->unused = [this](ViewHolder* viewHolder){
        mLayout->removeAndRecycleView(viewHolder->itemView, *mRecycler);
    };
    mViewInfoProcessCallback = visCBK;
    mUpdateChildViewsRunnable = std::bind(&RecyclerView::doUpdateChildViews,this);
    mItemAnimatorRunner = std::bind(&RecyclerView::doItemAnimator,this);
}

void RecyclerView::initAutofill() {
    if (getImportantForAutofill()== View::IMPORTANT_FOR_AUTOFILL_AUTO) {
        //setImportantForAutofill(View::IMPORTANT_FOR_AUTOFILL_NO_EXCLUDE_DESCENDANTS);
    }
}

//Runnable mUpdateChildViewsRunnable
void RecyclerView::doUpdateChildViews(){
    if (!mFirstLayoutComplete || isLayoutRequested()) {
        // a layout request will happen, we should not do layout here.
        return;
    }
    if (!mIsAttached) {
        requestLayout();
        // if we are not attached yet, mark us as requiring layout and skip
        return;
    }
    if (mLayoutSuppressed) {
        mLayoutWasDefered = true;
        return; //we'll process updates when ice age ends.
    }
    consumePendingUpdateOperations();
}

//mItemAnimatorRunner
void RecyclerView::doItemAnimator() {
    if (mItemAnimator != nullptr) {
        mItemAnimator->runPendingAnimations();
    }
    mPostedAnimatorRunner = false;
}

void RecyclerView::doAnimatorFinished(ViewHolder& item) {
    item.setIsRecyclable(true);
    if ((item.mShadowedHolder != nullptr) && (item.mShadowingHolder == nullptr)) { // old vh
        item.mShadowedHolder = nullptr;
    }
    // always null this because an OldViewHolder can never become NewViewHolder w/o being
    // recycled.
    item.mShadowingHolder = nullptr;
    if (!item.shouldBeKeptAsChild()) {
        if (!this->removeAnimatingView(item.itemView) && item.isTmpDetached()) {
            this->removeDetachedView(item.itemView, false);
        }
    }
}

RecyclerViewAccessibilityDelegate* RecyclerView::getCompatAccessibilityDelegate() {
    return mAccessibilityDelegate;
}

void RecyclerView::setAccessibilityDelegate(RecyclerViewAccessibilityDelegate* accessibilityDelegate) {
    mAccessibilityDelegate = accessibilityDelegate;
}

void RecyclerView::createLayoutManager(Context* context,const std::string& className,
	const AttributeSet& attrs/*,int defStyleAttr, int defStyleRes*/) {
    if(!className.compare("LinearLayoutManager")){
        setLayoutManager(new LinearLayoutManager(context,attrs));
    }else if(!className.compare("GridLayoutManager")){
        setLayoutManager(new GridLayoutManager(context,attrs));
    }else if(!className.compare("StaggeredGridLayoutManager")){
        setLayoutManager(new StaggeredGridLayoutManager(context,attrs));
    }
}

std::string RecyclerView::getFullClassName(Context* context, const std::string& className) {
    if (className[0] == '.') {
        return context->getPackageName() + className;
    }
    if (className.find(".")!=std::string::npos){//contains(".")) {
        return className;
    }
    return className;//RecyclerView.class.getPackage().getName() + '.' + className;
}

void RecyclerView::initChildrenHelper() {
    ChildHelper::Callback cbk;
    cbk.getChildCount=[this]()->int {
        return getChildCount();
    };

    cbk.addView=[this](View* child, int index) {
        addView(child, index);
        dispatchChildAttached(child);
    };

    cbk.indexOfChild=[this](View* view)->int {
        return indexOfChild(view);
    };

    cbk.removeViewAt=[this](int index) {
        View* child = getChildAt(index);
        if (child != nullptr) {
            dispatchChildDetached(child);
            // Clear any android.view.animation.Animation that may prevent the item from
            // detaching when being removed. If a child is re-added before the
            // lazy detach occurs, it will receive invalid attach/detach sequencing.
            child->clearAnimation();
        }
        removeViewAt(index);
    };

    cbk.getChildAt=[this](int offset)->View*{
        return getChildAt(offset);
    };

    cbk.removeAllViews=[this]() {
        const int count = getChildCount();
        for (int i = 0; i < count; i++) {
            View* child = getChildAt(i);
            dispatchChildDetached(child);
            // Clear any android.view.animation.Animation that may prevent the item from
            // detaching when being removed. If a child is re-added before the
            // lazy detach occurs, it will receive invalid attach/detach sequencing.
            child->clearAnimation();
        }
        removeAllViews();
    };

    cbk.getChildViewHolder=[this](View* view)->ViewHolder*{
        return getChildViewHolderInt(view);
    };

    cbk.attachViewToParent=[this](View* child, int index, ViewGroup::LayoutParams* layoutParams) {
        ViewHolder* vh = getChildViewHolderInt(child);
        if (vh != nullptr) {
            if (!vh->isTmpDetached() && !vh->shouldIgnore()) {
                LOGE("Called attach on a child which is not detached: %p",vh);
            }
            LOGD_IF(sDebugAssertionsEnabled,"reAttach %p %p:%d",vh,vh->itemView,vh->itemView->getId());
            vh->clearTmpDetachFlag();
        }
        attachViewToParent(child, index, layoutParams);
    };

    cbk.detachViewFromParent=[this](int offset) {
        View* view = getChildAt(offset);
        if (view) {
            ViewHolder* vh = getChildViewHolderInt(view);
            if (vh != nullptr) {
                if (vh->isTmpDetached() && !vh->shouldIgnore()) {
                    LOGE("called detach on an already detached child %p",vh);
                }
                LOGD_IF(sDebugAssertionsEnabled,"tmpDetach =%p %p:%d",vh,vh->itemView,vh->itemView->getId());
                vh->addFlags(ViewHolder::FLAG_TMP_DETACHED);
            }
        }
        detachViewFromParent(offset);
    };

    cbk.onEnteredHiddenState=[this](View* child) {
        ViewHolder* vh = getChildViewHolderInt(child);
        if (vh != nullptr) {
            vh->onEnteredHiddenState(*this);
        }
    };

    cbk.onLeftHiddenState=[this](View* child) {
        ViewHolder* vh = getChildViewHolderInt(child);
        if (vh != nullptr) {
            vh->onLeftHiddenState(*this);
        }
    };
    mChildHelper = new ChildHelper(cbk);
}

void RecyclerView::dispatchUpdate(void* /*AdapterHelper::UpdateOp*/ paramsOP) {
    const AdapterHelper::UpdateOp* op=(AdapterHelper::UpdateOp*)paramsOP;
    switch (op->cmd) {
    case AdapterHelper::UpdateOp::ADD:
            mLayout->onItemsAdded(*this, op->positionStart, op->itemCount);
            break;
    case AdapterHelper::UpdateOp::REMOVE:
            mLayout->onItemsRemoved(*this, op->positionStart, op->itemCount);
            break;
    case AdapterHelper::UpdateOp::UPDATE:
            mLayout->onItemsUpdated(*this, op->positionStart, op->itemCount, op->payload);
            break;
    case AdapterHelper::UpdateOp::MOVE:
            mLayout->onItemsMoved(*this, op->positionStart, op->itemCount, 1);
            break;
    }
}
void RecyclerView::initAdapterManager() {
    AdapterHelper::Callback cbk;
    cbk.findViewHolder = [this](int position)->ViewHolder*{
        ViewHolder* vh = findViewHolderForPosition(position, true);
        if (vh == nullptr) {
            return nullptr;
        }
        // ensure it is not hidden because for adapter helper, the only thing matter is that
        // LM thinks view is a child.
        if (mChildHelper->isHidden(vh->itemView)) {
            LOGD("assuming view holder cannot be find because it is hidden");
            return nullptr;
        }
        return vh;
    };

    cbk.offsetPositionsForRemovingInvisible = [this](int start, int count) {
        offsetPositionRecordsForRemove(start, count, true);
        mItemsAddedOrRemoved = true;
        mState->mDeletedInvisibleItemCountSincePreviousLayout += count;
    };

    cbk.offsetPositionsForRemovingLaidOutOrNewView = [this](int positionStart, int itemCount) {
        offsetPositionRecordsForRemove(positionStart, itemCount, false);
        mItemsAddedOrRemoved = true;
    };

    cbk.markViewHoldersUpdated = [this](int positionStart, int itemCount, Object* payload) {
        viewRangeUpdate(positionStart, itemCount, payload);
        mItemsChanged = true;
    };

    cbk.onDispatchFirstPass = [this](AdapterHelper::UpdateOp* op) {
        dispatchUpdate(op);
    };

    cbk.onDispatchSecondPass = [this](AdapterHelper::UpdateOp* op) {
        dispatchUpdate(op);
    };
    cbk.offsetPositionsForAdd = [this](int positionStart, int itemCount) {
        offsetPositionRecordsForInsert(positionStart, itemCount);
        mItemsAddedOrRemoved = true;
    };

    cbk.offsetPositionsForMove = [this](int from, int to) {
        offsetPositionRecordsForMove(from, to);
        // should we create mItemsMoved ?
        mItemsAddedOrRemoved = true;
    };
    mAdapterHelper = new AdapterHelper(cbk);
}

void RecyclerView::setHasFixedSize(bool hasFixedSize) {
    mHasFixedSize = hasFixedSize;
}

bool RecyclerView::hasFixedSize()const {
    return mHasFixedSize;
}

void RecyclerView::setClipToPadding(bool clipToPadding) {
    if (clipToPadding != mClipToPadding) {
        invalidateGlows();
    }
    mClipToPadding = clipToPadding;
    ViewGroup::setClipToPadding(clipToPadding);
    if (mFirstLayoutComplete) {
        requestLayout();
    }
}

bool RecyclerView::getClipToPadding() const{
    return mClipToPadding;
}

void RecyclerView::setScrollingTouchSlop(int slopConstant) {
    ViewConfiguration& vc = ViewConfiguration::get(getContext());
    switch (slopConstant) {
    default:LOGW("setScrollingTouchSlop(): bad argument constant %d; using default value",slopConstant);
            // fall-through
    case TOUCH_SLOP_DEFAULT:
        mTouchSlop = vc.getScaledTouchSlop();
        break;

    case TOUCH_SLOP_PAGING:
        mTouchSlop = vc.getScaledPagingTouchSlop();
        break;
    }
}

void RecyclerView::swapAdapter(Adapter* adapter, bool removeAndRecycleExistingViews) {
    // bail out if layout is frozen
    suppressLayout(false);
    setAdapterInternal(adapter, true, removeAndRecycleExistingViews);
    processDataSetCompletelyChanged(true);
    requestLayout();
}

void RecyclerView::setAdapter(Adapter* adapter) {
    // bail out if layout is frozen
    suppressLayout(false);
    setAdapterInternal(adapter, false, true);
    processDataSetCompletelyChanged(false);
    requestLayout();
}

void RecyclerView::removeAndRecycleViews() {
    // end all running animations
    if (mItemAnimator != nullptr) {
        mItemAnimator->endAnimations();
    }
    // Since animations are ended, mLayout.children should be equal to
    // recyclerView.children. This may not be true if item animator's end does not work as
    // expected. (e.g. not release children instantly). It is safer to use mLayout's child
    // count.
    if (mLayout != nullptr) {
        mLayout->removeAndRecycleAllViews(*mRecycler);
        mLayout->removeAndRecycleScrapInt(*mRecycler);
    }
    // we should clear it here before adapters are swapped to ensure correct callbacks.
    mRecycler->clear();
}

void RecyclerView::setAdapterInternal(Adapter* adapter, bool compatibleWithPrevious,
            bool bRemoveAndRecycleViews) {
    if (mAdapter != nullptr) {
        mAdapter->unregisterAdapterDataObserver(mObserver);
        mAdapter->onDetachedFromRecyclerView(*this);
    }
    if (!compatibleWithPrevious || bRemoveAndRecycleViews) {
        removeAndRecycleViews();
    }
    mAdapterHelper->reset();
    Adapter* oldAdapter = mAdapter;
    mAdapter = adapter;
    if (adapter != nullptr) {
        adapter->registerAdapterDataObserver(mObserver);
        adapter->onAttachedToRecyclerView(*this);
    }
    if (mLayout != nullptr) {
        mLayout->onAdapterChanged(oldAdapter, mAdapter);
    }
    mRecycler->onAdapterChanged(oldAdapter, mAdapter, compatibleWithPrevious);
    mState->mStructureChanged = true;
}

RecyclerView::Adapter* RecyclerView::getAdapter() {
    return mAdapter;
}

void RecyclerView::setRecyclerListener(const RecyclerListener& listener) {
    mRecyclerListener = listener;
}

void RecyclerView::addRecyclerListener(const RecyclerListener& listener) {
    mRecyclerListeners.push_back(listener);
}

/**
 * Removes the provided listener from RecyclerListener list.
 *
 * @param listener Listener to unregister.
 */
void RecyclerView::removeRecyclerListener(const RecyclerListener& listener) {
    auto it=std::find(mRecyclerListeners.begin(),mRecyclerListeners.end(),listener);
    if(it!=mRecyclerListeners.end()){
        mRecyclerListeners.erase(it);
    }
}

int RecyclerView::getBaseline() {
    if (mLayout) {
        return mLayout->getBaseline();
    } else {
        return ViewGroup::getBaseline();
    }
}

void RecyclerView::addOnChildAttachStateChangeListener(const OnChildAttachStateChangeListener& listener) {
    mOnChildAttachStateListeners.push_back(listener);
}

void RecyclerView::removeOnChildAttachStateChangeListener(const OnChildAttachStateChangeListener& listener) {
    if (mOnChildAttachStateListeners.size() == 0) {
        return;
    }
    for(auto it=mOnChildAttachStateListeners.begin();it!=mOnChildAttachStateListeners.end();it++){
        if ((it->onChildViewAttachedToWindow==listener.onChildViewAttachedToWindow)
                && (it->onChildViewDetachedFromWindow==listener.onChildViewDetachedFromWindow))
            mOnChildAttachStateListeners.erase(it);
    }
}

void RecyclerView::clearOnChildAttachStateChangeListeners() {
    mOnChildAttachStateListeners.clear();
}

void RecyclerView::setLayoutManager(LayoutManager* layout) {
    if (layout == mLayout) {
        return;
    }
    stopScroll();
    // TODO We should do this switch a dispatchLayout pass and animate children. There is a good
    // chance that LayoutManagers will re-use views.
    if (mLayout != nullptr) {
        // end all running animations
        if (mItemAnimator != nullptr) {
            mItemAnimator->endAnimations();
        }
        mLayout->removeAndRecycleAllViews(*mRecycler);
        mLayout->removeAndRecycleScrapInt(*mRecycler);
        mRecycler->clear();

        if (mIsAttached) {
            mLayout->dispatchDetachedFromWindow(*this, *mRecycler);
        }
        mLayout->setRecyclerView(nullptr);
        delete mLayout;
    } else {
        mRecycler->clear();
    }
    // this is just a defensive measure for faulty item animators.
    mChildHelper->removeAllViewsUnfiltered();
    mLayout = layout;
    if (layout != nullptr) {
        if (layout->mRecyclerView != nullptr) {
            FATAL("LayoutManager %p is already attached to a RecyclerView:",layout);
        }
        mLayout->setRecyclerView(this);
        if (mIsAttached) {
            mLayout->dispatchAttachedToWindow(*this);
        }
    }
    mRecycler->updateViewCacheSize();
    requestLayout();
}

void RecyclerView::setOnFlingListener(const OnFlingListener& onFlingListener) {
    mOnFlingListener = onFlingListener;
}

RecyclerView::OnFlingListener RecyclerView::getOnFlingListener() {
    return mOnFlingListener;
}

Parcelable* RecyclerView::onSaveInstanceState() {
    SavedState* state = new SavedState(ViewGroup::onSaveInstanceState());
    if (mPendingSavedState != nullptr) {
        state->copyFrom(*mPendingSavedState);
    } else if (mLayout != nullptr) {
        state->mLayoutState = mLayout->onSaveInstanceState();
    } else {
        state->mLayoutState = nullptr;
    }
    return state;
}

void RecyclerView::onRestoreInstanceState(Parcelable& state) {
    if (dynamic_cast<SavedState*>(&state)==nullptr) {
        ViewGroup::onRestoreInstanceState(state);
        return;
    }

    mPendingSavedState = (SavedState*)& state;
    ViewGroup::onRestoreInstanceState(*mPendingSavedState->getSuperState());
    if (mLayout != nullptr && mPendingSavedState->mLayoutState != nullptr) {
        mLayout->onRestoreInstanceState(*mPendingSavedState->mLayoutState);
    }
}

void RecyclerView::dispatchSaveInstanceState(SparseArray<Parcelable*>& container) {
    dispatchFreezeSelfOnly(container);
}

void RecyclerView::dispatchRestoreInstanceState(SparseArray<Parcelable*>& container) {
    dispatchThawSelfOnly(container);
}

void RecyclerView::addAnimatingView(ViewHolder& viewHolder) {
    View* view = viewHolder.itemView;
    const bool alreadyParented = view->getParent() == this;
    mRecycler->unscrapView(*getChildViewHolder(view));
    if (viewHolder.isTmpDetached()) {
        // re-attach
        mChildHelper->attachViewToParent(view, -1, view->getLayoutParams(), true);
    } else if (!alreadyParented) {
        mChildHelper->addView(view, true);
    } else {
        mChildHelper->hide(view);
    }
}

bool RecyclerView::removeAnimatingView(View* view) {
    startInterceptRequestLayout();
    const bool removed = mChildHelper->removeViewIfHidden(view);
    if (removed) {
        ViewHolder* viewHolder = getChildViewHolderInt(view);
        mRecycler->unscrapView(*viewHolder);
        mRecycler->recycleViewHolderInternal(*viewHolder);
        LOGD_IF(sDebugAssertionsEnabled,"after removing animated view: %p",view);
    }
    // only clear request eaten flag if we removed the view.
    stopInterceptRequestLayout(!removed);
    return removed;
}

RecyclerView::LayoutManager* RecyclerView::getLayoutManager() {
    return mLayout;
}

RecyclerView::RecycledViewPool& RecyclerView::getRecycledViewPool() {
    return mRecycler->getRecycledViewPool();
}

void RecyclerView::setRecycledViewPool(RecycledViewPool* pool) {
    mRecycler->setRecycledViewPool(pool);
}

void RecyclerView::setViewCacheExtension(const ViewCacheExtension& extension) {
    mRecycler->setViewCacheExtension(extension);
}

void RecyclerView::setItemViewCacheSize(int size) {
    mRecycler->setViewCacheSize(size);
}

int RecyclerView::getScrollState() const{
    return mScrollState;
}

void RecyclerView::setScrollState(int state) {
    if (state == mScrollState) {
        return;
    }
    LOGD_IF(sDebugAssertionsEnabled,"setting scroll state from %d to %d",mScrollState,state);
    mScrollState = state;
    if (state != SCROLL_STATE_SETTLING) {
        stopScrollersInternal();
    }
    dispatchOnScrollStateChanged(state);
}

void RecyclerView::addItemDecoration(ItemDecoration* decor, int index) {
    if (mLayout != nullptr) {
        mLayout->assertNotInLayoutOrScroll("Cannot add item decoration during a scroll  or layout");
    }
    if (mItemDecorations.empty()){//isEmpty()) {
        setWillNotDraw(false);
    }
    auto it=std::find( mItemDecorations.begin(), mItemDecorations.end(),decor);
    if(it!=mItemDecorations.end()){
        LOGE("ItemDecoration %p has exists",decor);
        return;
    }
    if (index < 0) {
        mItemDecorations.push_back(decor);//add(decor);
    } else {
        mItemDecorations.insert(mItemDecorations.begin()+index,decor);//add(index, decor);
    }
    markItemDecorInsetsDirty();
    requestLayout();
}

void RecyclerView::addItemDecoration(ItemDecoration* decor) {
    addItemDecoration(decor, -1);
}

RecyclerView::ItemDecoration* RecyclerView::getItemDecorationAt(int index) {
    const int size = getItemDecorationCount();
    FATAL_IF((index < 0)||(index >= size),"%d is an invalid index for size %d",index,size);
    return mItemDecorations.at(index);
}

int RecyclerView::getItemDecorationCount() const{
    return mItemDecorations.size();
}

void RecyclerView::removeItemDecorationAt(int index) {
    const int size = getItemDecorationCount();
    if (index < 0 || index >= size) {
        FATAL("%d is an invalid index for size %d",index,size);
    }

    removeItemDecoration(getItemDecorationAt(index));
}

void RecyclerView::removeItemDecoration(ItemDecoration* decor) {
    if (mLayout != nullptr) {
        mLayout->assertNotInLayoutOrScroll("Cannot remove item decoration during a scroll  or layout");
    }
    auto it = std::find(mItemDecorations.begin(),mItemDecorations.end(),decor);
    if(it != mItemDecorations.end())
        mItemDecorations.erase(it);
    delete decor;
    if (mItemDecorations.empty()){
        setWillNotDraw(getOverScrollMode() == View::OVER_SCROLL_NEVER);
    }
    markItemDecorInsetsDirty();
    requestLayout();
}

void RecyclerView::setChildDrawingOrderCallback(ChildDrawingOrderCallback childDrawingOrderCallback) {
    if (childDrawingOrderCallback == mChildDrawingOrderCallback) {
        return;
    }
    mChildDrawingOrderCallback = childDrawingOrderCallback;
    setChildrenDrawingOrderEnabled(mChildDrawingOrderCallback != nullptr);
}

void RecyclerView::setOnScrollListener(const OnScrollListener& listener) {
    mScrollListener = listener;
}

void  RecyclerView::addOnScrollListener(const OnScrollListener& listener) {
    auto it = std::find(mScrollListeners.begin(),mScrollListeners.end(),listener);
    if(it == mScrollListeners.end())mScrollListeners.push_back(listener);
}

void RecyclerView::removeOnScrollListener(const OnScrollListener& listener) {
    auto it = std::find(mScrollListeners.begin(),mScrollListeners.end(),listener);
    if(it != mScrollListeners.end())mScrollListeners.erase(it);
}

void RecyclerView::clearOnScrollListeners() {
    mScrollListeners.clear();
}

void RecyclerView::scrollToPosition(int position) {
    if (mLayoutSuppressed) {
        return;
    }
    stopScroll();
    if (mLayout == nullptr) {
        LOGE("Cannot scroll to position a LayoutManager set. "
             "Call setLayoutManager with a non-null argument.");
        return;
    }
    mLayout->scrollToPosition(position);
    awakenScrollBars();
}

void RecyclerView::jumpToPositionForSmoothScroller(int position) {
    if (mLayout == nullptr) {
        return;
    }
    // If we are jumping to a position, we are in fact scrolling the contents of the RV, so
    // we should be sure that we are in the settling state.
    setScrollState(SCROLL_STATE_SETTLING);
    mLayout->scrollToPosition(position);
    awakenScrollBars();
}

void RecyclerView::smoothScrollToPosition(int position) {
    if (mLayoutSuppressed) {
        return;
    }
    if (mLayout == nullptr) {
        LOGE("Cannot smooth scroll without a LayoutManager set. "
             "Call setLayoutManager with a non-null argument.");
        return;
    }
    mLayout->smoothScrollToPosition(*this, *mState, position);
}

bool RecyclerView::dispatchKeyEvent(KeyEvent& event) {
    // Let child to dispatch first, then handle ours if child didn't do it.
    if (ViewGroup::dispatchKeyEvent(event)) {
        return true;
    }

    LayoutManager* layoutManager = getLayoutManager();
    // If there is no layout manager, then there is nothing to handle key events for.
    if (layoutManager == nullptr) {
        return false;
    }
    bool isReversed;
    int targetOffset,height,width;
    const int keyCode = event.getKeyCode();
    if (layoutManager->canScrollVertically()) {
        switch (keyCode) {
            case KeyEvent::KEYCODE_PAGE_DOWN:
            case KeyEvent::KEYCODE_PAGE_UP:
                height = getMeasuredHeight();
                if (keyCode == KeyEvent::KEYCODE_PAGE_DOWN) {
                    smoothScrollBy(0, height, nullptr, RecyclerView::UNDEFINED_DURATION);
                } else {
                    smoothScrollBy(0, -height, nullptr, RecyclerView::UNDEFINED_DURATION);
                }
                return true;

            case KeyEvent::KEYCODE_MOVE_HOME:
            case KeyEvent::KEYCODE_MOVE_END:
                isReversed = layoutManager->isLayoutReversed();

                if (keyCode == KeyEvent::KEYCODE_MOVE_HOME) {
                    targetOffset = isReversed ? getAdapter()->getItemCount() : 0;
                } else {
                    targetOffset = isReversed ? 0 : getAdapter()->getItemCount();
                }

                smoothScrollToPosition(targetOffset);
                return true;
        }
    } else if (layoutManager->canScrollHorizontally()) {
        switch (keyCode) {
            case KeyEvent::KEYCODE_PAGE_DOWN:
            case KeyEvent::KEYCODE_PAGE_UP:
                width = getMeasuredWidth();
                if (keyCode == KeyEvent::KEYCODE_PAGE_DOWN) {
                    smoothScrollBy(width, 0, nullptr, RecyclerView::UNDEFINED_DURATION);
                } else {
                    smoothScrollBy(-width, 0, nullptr, RecyclerView::UNDEFINED_DURATION);
                }
                return true;

            case KeyEvent::KEYCODE_MOVE_HOME:
            case KeyEvent::KEYCODE_MOVE_END:
                isReversed = layoutManager->isLayoutReversed();

                if (keyCode == KeyEvent::KEYCODE_MOVE_HOME) {
                    targetOffset = isReversed ? getAdapter()->getItemCount() : 0;
                } else {
                    targetOffset = isReversed ? 0 : getAdapter()->getItemCount();
                }

                smoothScrollToPosition(targetOffset);
                return true;
        }
    }

    return false;
}

void RecyclerView::scrollTo(int x, int y) {
    LOGW("RecyclerView does not support scrolling to an absolute position. Use scrollToPosition instead");
}

void RecyclerView::scrollBy(int x, int y) {
    if (mLayout == nullptr) {
        LOGE("Cannot scroll without a LayoutManager set. Call setLayoutManager with a non-null argument.");
        return;
    }
    if (mLayoutSuppressed) {
        return;
    }
    const bool canScrollHorizontal = mLayout->canScrollHorizontally();
    const bool canScrollVertical = mLayout->canScrollVertically();
    if (canScrollHorizontal || canScrollVertical) {
        scrollByInternal(canScrollHorizontal ? x : 0, canScrollVertical ? y : 0,
                /*horizontalAxis*/-1,/*verticalAxis*/-1,/*ev*/nullptr,TYPE_TOUCH);
    }
}

void RecyclerView::nestedScrollBy(int x, int y) {
    nestedScrollByInternal(x, y, -1, -1, nullptr, TYPE_NON_TOUCH);
}

void RecyclerView::nestedScrollByInternal(int x,int y,int horizontalAxis,int verticalAxis,
       MotionEvent* motionEvent,int type) {
    if (mLayout == nullptr) {
        LOGE("Cannot scroll without a LayoutManager set. Call setLayoutManager with a non-null argument.");
        return;
    }
    if (mLayoutSuppressed) {
        return;
    }
    mReusableIntPair[0] = 0;
    mReusableIntPair[1] = 0;
    bool canScrollHorizontal = mLayout->canScrollHorizontally();
    bool canScrollVertical = mLayout->canScrollVertically();

    int nestedScrollAxis = View::SCROLL_AXIS_NONE;
    if (canScrollHorizontal) {
        nestedScrollAxis |= View::SCROLL_AXIS_HORIZONTAL;
    }
    if (canScrollVertical) {
        nestedScrollAxis |= View::SCROLL_AXIS_VERTICAL;
    }

    // If there is no MotionEvent, treat it as center-aligned edge effect:
    float verticalDisplacement = motionEvent == nullptr ? getHeight() / 2.f : motionEvent->getY();
    float horizontalDisplacement = motionEvent == nullptr ? getWidth() / 2.f : motionEvent->getX();
    x -= releaseHorizontalGlow(x, verticalDisplacement);
    y -= releaseVerticalGlow(y, horizontalDisplacement);
    startNestedScroll(nestedScrollAxis, type);
    if (dispatchNestedPreScroll(
            canScrollHorizontal ? x : 0,
            canScrollVertical ? y : 0,
            mReusableIntPair, mScrollOffset, type
    )) {
        x -= mReusableIntPair[0];
        y -= mReusableIntPair[1];
    }

    scrollByInternal(
            canScrollHorizontal ? x : 0,
            canScrollVertical ? y : 0,
            horizontalAxis, verticalAxis,
            motionEvent, type);
    if ((mGapWorker != nullptr) && (x != 0 || y != 0)) {
        mGapWorker->postFromTraversal(this, x, y);
    }
    stopNestedScroll(type);
}

void RecyclerView::scrollStep(int dx, int dy,int* consumed) {
    startInterceptRequestLayout();
    onEnterLayoutOrScroll();

    fillRemainingScrollValues(*mState);

    int consumedX = 0;
    int consumedY = 0;
    if (dx != 0) {
        consumedX = mLayout->scrollHorizontallyBy(dx, *mRecycler, *mState);
    }
    if (dy != 0) {
        consumedY = mLayout->scrollVerticallyBy(dy, *mRecycler, *mState);
    }

    repositionShadowingViews();

    onExitLayoutOrScroll();
    stopInterceptRequestLayout(false);

    if (consumed != nullptr) {
        consumed[0] = consumedX;
        consumed[1] = consumedY;
    }
}

void RecyclerView::consumePendingUpdateOperations() {
    if (!mFirstLayoutComplete || mDataSetHasChangedAfterLayout) {
        dispatchLayout();
        return;
    }
    if (!mAdapterHelper->hasPendingUpdates()) {
        return;
    }

    // if it is only an item change (no add-remove-notifyDataSetChanged) we can check if any
    // of the visible items is affected and if not, just ignore the change.
    if (mAdapterHelper->hasAnyUpdateTypes(AdapterHelper::UpdateOp::UPDATE) && !mAdapterHelper
            ->hasAnyUpdateTypes(AdapterHelper::UpdateOp::ADD | AdapterHelper::UpdateOp::REMOVE
                    | AdapterHelper::UpdateOp::MOVE)) {
        startInterceptRequestLayout();
        onEnterLayoutOrScroll();
        mAdapterHelper->preProcess();
        if (!mLayoutWasDefered) {
            if (hasUpdatedView()) {
                dispatchLayout();
            } else {
                // no need to layout, clean state
                mAdapterHelper->consumePostponedUpdates();
            }
        }
        stopInterceptRequestLayout(true);
        onExitLayoutOrScroll();
    } else if (mAdapterHelper->hasPendingUpdates()) {
        dispatchLayout();
    }
}

/**
 * @return True if an existing view holder needs to be updated
 */
bool RecyclerView::hasUpdatedView() {
    const int childCount = mChildHelper->getChildCount();
    for (int i = 0; i < childCount; i++) {
        ViewHolder* holder = getChildViewHolderInt(mChildHelper->getChildAt(i));
        if (holder == nullptr || holder->shouldIgnore()) {
            continue;
        }
        if (holder->isUpdated()) {
            return true;
        }
    }
    return false;
}

bool RecyclerView::scrollByInternal(int x, int y,int horizontalAxis,int verticalAxis, MotionEvent* ev,int type) {
    int unconsumedX = 0, unconsumedY = 0;
    int consumedX = 0, consumedY = 0;

    consumePendingUpdateOperations();
    if (mAdapter != nullptr) {
        mReusableIntPair[0] = 0;
        mReusableIntPair[1] = 0;
        scrollStep(x, y, mScrollStepConsumed);
        consumedX = mScrollStepConsumed[0];
        consumedY = mScrollStepConsumed[1];
        unconsumedX = x - consumedX;
        unconsumedY = y - consumedY;
    }
    if (!mItemDecorations.empty()){//isEmpty()) {
        invalidate();
    }

    mReusableIntPair[0] = 0;
    mReusableIntPair[1] = 0;

    dispatchNestedScroll(consumedX, consumedY, unconsumedX, unconsumedY, mScrollOffset,type,mReusableIntPair);
    unconsumedX -= mReusableIntPair[0];
    unconsumedY -= mReusableIntPair[1];
    bool consumedNestedScroll = mReusableIntPair[0] != 0 || mReusableIntPair[1] != 0;
    // Update the last touch co-ords, taking any scroll offset into account
    mLastTouchX -= mScrollOffset[0];
    mLastTouchY -= mScrollOffset[1];
    mNestedOffsets[0] += mScrollOffset[0];
    mNestedOffsets[1] += mScrollOffset[1];

    if (ev != nullptr) {
        if (consumedX != 0) {
            getScrollFeedbackProvider()->onScrollProgress(
                ev->getDeviceId(), ev->getSource(), horizontalAxis, consumedX);
        }
        if (consumedY != 0) {
            getScrollFeedbackProvider()->onScrollProgress(
                ev->getDeviceId(), ev->getSource(), verticalAxis, consumedY);
        }
    }
    if (getOverScrollMode() != View::OVER_SCROLL_NEVER) {
        if (ev != nullptr && !ev->isFromSource(InputDevice::SOURCE_MOUSE)) {
            pullGlows(ev,ev->getX(),horizontalAxis, unconsumedX,
                    ev->getY(), verticalAxis,unconsumedY);
            if(ev->isFromSource(InputDevice::SOURCE_ROTARY_ENCODER)){
                releaseGlows();
            }
        }
        considerReleasingGlowsOnScroll(x, y);
    }
    if ((consumedX != 0) || (consumedY != 0)) {
        dispatchOnScrolled(consumedX, consumedY);
    }
    if (!awakenScrollBars()) {
        invalidate();
    }
    return (consumedX != 0) || (consumedY != 0);
}

int RecyclerView::releaseHorizontalGlow(int deltaX, float y) {
    // First allow releasing existing overscroll effect:
    float consumed = 0;
    const float displacement = y / getHeight();
    const float pullDistance = (float) deltaX / getWidth();
    if (mLeftGlow != nullptr && mLeftGlow->getDistance() != 0) {
        if (canScrollHorizontally(-1)) {
            mLeftGlow->onRelease();
        } else {
            consumed = -mLeftGlow->onPullDistance(-pullDistance,1 - displacement);
            if (mLeftGlow->getDistance() == 0) {
                mLeftGlow->onRelease();
            }
        }
        invalidate();
    } else if (mRightGlow != nullptr && mRightGlow->getDistance() != 0) {
        if (canScrollHorizontally(1)) {
            mRightGlow->onRelease();
        } else {
            consumed = mRightGlow->onPullDistance(pullDistance, displacement);
            if (mRightGlow->getDistance() == 0) {
                mRightGlow->onRelease();
            }
        }
        invalidate();
    }
    return std::round(consumed * getWidth());
}

int RecyclerView::releaseVerticalGlow(int deltaY, float x) {
    // First allow releasing existing overscroll effect:
    float consumed = 0;
    const float displacement = x / getWidth();
    const float pullDistance = (float) deltaY / getHeight();
    if (mTopGlow != nullptr && mTopGlow->getDistance() != 0) {
        if (canScrollVertically(-1)) {
            mTopGlow->onRelease();
        } else {
            consumed = -mTopGlow->onPullDistance(-pullDistance, displacement);
            if (mTopGlow->getDistance() == 0) {
                mTopGlow->onRelease();
            }
        }
        invalidate();
    } else if (mBottomGlow != nullptr && mBottomGlow->getDistance() != 0) {
        if (canScrollVertically(1)) {
            mBottomGlow->onRelease();
        } else {
            consumed = mBottomGlow->onPullDistance(pullDistance,1 - displacement);
            if (mBottomGlow->getDistance() == 0) {
                mBottomGlow->onRelease();
            }
        }
        invalidate();
    }
    return std::round(consumed * getHeight());
}

int RecyclerView::computeHorizontalScrollOffset() {
    if (mLayout == nullptr) {
        return 0;
    }
    return mLayout->canScrollHorizontally() ? mLayout->computeHorizontalScrollOffset(*mState) : 0;
}

int RecyclerView::computeHorizontalScrollExtent() {
    if (mLayout == nullptr) {
        return 0;
    }
    return mLayout->canScrollHorizontally() ? mLayout->computeHorizontalScrollExtent(*mState) : 0;
}

int RecyclerView::computeHorizontalScrollRange() {
    if (mLayout == nullptr) {
        return 0;
    }
    return mLayout->canScrollHorizontally() ? mLayout->computeHorizontalScrollRange(*mState) : 0;
}

int RecyclerView::computeVerticalScrollOffset() {
    if (mLayout == nullptr) {
        return 0;
    }
    return mLayout->canScrollVertically() ? mLayout->computeVerticalScrollOffset(*mState) : 0;
}

int RecyclerView::computeVerticalScrollExtent() {
    if (mLayout == nullptr) {
        return 0;
    }
    return mLayout->canScrollVertically() ? mLayout->computeVerticalScrollExtent(*mState) : 0;
}

int RecyclerView::computeVerticalScrollRange() {
    if (mLayout == nullptr) {
        return 0;
    }
    return mLayout->canScrollVertically() ? mLayout->computeVerticalScrollRange(*mState) : 0;
}

void RecyclerView::startInterceptRequestLayout() {
    mInterceptRequestLayoutDepth++;
    if ((mInterceptRequestLayoutDepth == 1) && !mLayoutSuppressed) {
        mLayoutWasDefered = false;
    }
}

void RecyclerView::stopInterceptRequestLayout(bool performLayoutChildren) {
    if (mInterceptRequestLayoutDepth < 1) {
        //noinspection PointlessBooleanExpression
        LOGD_IF(sDebugAssertionsEnabled,"stopInterceptRequestLayout was called more "
            "times than startInterceptRequestLayout.");
        mInterceptRequestLayoutDepth = 1;
    }
    if (!performLayoutChildren && !mLayoutSuppressed) {
        mLayoutWasDefered = false;
    }
    if (mInterceptRequestLayoutDepth == 1) {
        // when layout is frozen we should delay dispatchLayout()
        if (performLayoutChildren && mLayoutWasDefered && !mLayoutSuppressed
                && (mLayout != nullptr) && (mAdapter != nullptr)) {
            dispatchLayout();
        }
        if (!mLayoutSuppressed) {
            mLayoutWasDefered = false;
        }
    }
    mInterceptRequestLayoutDepth--;
}

void RecyclerView::suppressLayout(bool frozen) {
    if (frozen != mLayoutSuppressed) {
        assertNotInLayoutOrScroll("Do not suppressLayout in layout or scroll");
        if (!frozen) {
            mLayoutSuppressed = false;
            if (mLayoutWasDefered && (mLayout != nullptr) && (mAdapter != nullptr)) {
                requestLayout();
            }
            mLayoutWasDefered = false;
        } else {
            const auto now = SystemClock::uptimeMillis();
            MotionEvent* cancelEvent = MotionEvent::obtain(now, now,
                    MotionEvent::ACTION_CANCEL, 0.0f, 0.0f, 0);
            onTouchEvent(*cancelEvent);
            mLayoutSuppressed = true;
            mIgnoreMotionEventTillDown = true;
            stopScroll();
        }
    }
}

bool RecyclerView::isLayoutSuppressed() const{
    return mLayoutSuppressed;
}

void RecyclerView::smoothScrollBy(int dx,int dy) {
    smoothScrollBy(dx, dy, nullptr);
}

void RecyclerView::smoothScrollBy(int dx,int dy,Interpolator* interpolator) {
    smoothScrollBy(dx,dy,interpolator,UNDEFINED_DURATION);
}

void RecyclerView::smoothScrollBy(int dx,int dy,Interpolator* interpolator,int duration){
    smoothScrollBy(dx,dy,interpolator,duration,false);
}

void RecyclerView::smoothScrollBy(int dx,int dy,Interpolator* interpolator,int duration,bool withNestedScrolling){
    if (mLayout == nullptr) {
        LOGE("Cannot smooth scroll without a LayoutManager set. "
               "Call setLayoutManager with a non-null argument.");
        return;
    }
    if (mLayoutSuppressed) {
        return;
    }
    if (!mLayout->canScrollHorizontally()) {
        dx = 0;
    }
    if (!mLayout->canScrollVertically()) {
        dy = 0;
    }
    if ((dx != 0) || (dy != 0)) {
        bool durationSuggestsAnimation = duration == UNDEFINED_DURATION || duration > 0;
        if (durationSuggestsAnimation) {
            if (withNestedScrolling) {
                int nestedScrollAxis = View::SCROLL_AXIS_NONE;
                if (dx != 0) {
                    nestedScrollAxis |= View::SCROLL_AXIS_HORIZONTAL;
                }
                if (dy != 0) {
                    nestedScrollAxis |= View::SCROLL_AXIS_VERTICAL;
                }
                startNestedScroll(nestedScrollAxis, TYPE_NON_TOUCH);
            }
            mViewFlinger->smoothScrollBy(dx, dy, duration, interpolator);
        } else {
            scrollBy(dx, dy);
        }
    }
}

bool RecyclerView::fling(int velocityX, int velocityY) {
    return fling(velocityX,velocityY,mMinFlingVelocity, mMaxFlingVelocity);
}

bool RecyclerView::flingNoThresholdCheck(int velocityX, int velocityY){
    return fling(velocityX, velocityY, 0, INT_MAX);
}

bool RecyclerView::fling(int velocityX, int velocityY, int minFlingVelocity, int maxFlingVelocity){
    if (mLayout == nullptr) {
        LOGE("Cannot fling without a LayoutManager set. Call setLayoutManager with a non-null argument.");
        return false;
    }
    if (mLayoutSuppressed) {
        return false;
    }

    const bool bCanScrollHorizontal = mLayout->canScrollHorizontally();
    const bool bCanScrollVertical = mLayout->canScrollVertically();

    if ((bCanScrollHorizontal==false) || (std::abs(velocityX) < mMinFlingVelocity)) {
        velocityX = 0;
    }
    if ((bCanScrollVertical==false) || (std::abs(velocityY) < mMinFlingVelocity)) {
        velocityY = 0;
    }
    if ((velocityX == 0) && (velocityY == 0)) {
        // If we don't have any velocity, return false
        return false;
    }
    // Flinging while the edge effect is active should affect the edge effect,
    // not scrolling.
    int flingX =0,flingY=0;
    if (velocityX != 0) {
        if (mLeftGlow != nullptr && mLeftGlow->getDistance() != 0) {
            if (shouldAbsorb(mLeftGlow, -velocityX, getWidth())) {
                mLeftGlow->onAbsorb(-velocityX);
            } else {
                flingX = velocityX;
            }
            velocityX = 0;
        } else if (mRightGlow != nullptr && mRightGlow->getDistance() != 0) {
            if (shouldAbsorb(mRightGlow, velocityX, getWidth())) {
                mRightGlow->onAbsorb(velocityX);
            } else {
                flingX = velocityX;
            }
            velocityX = 0;
        }
    }
    if (velocityY != 0) {
        if (mTopGlow != nullptr && mTopGlow->getDistance() != 0) {
            if (shouldAbsorb(mTopGlow, -velocityY, getHeight())) {
                mTopGlow->onAbsorb(-velocityY);
            } else {
                flingY = velocityY;
            }
            velocityY = 0;
        } else if (mBottomGlow != nullptr && mBottomGlow->getDistance() != 0) {
            if (shouldAbsorb(mBottomGlow, velocityY, getHeight())) {
                mBottomGlow->onAbsorb(velocityY);
            } else {
                flingY = velocityY;
            }
            velocityY = 0;
        }
    }
    if (flingX != 0 || flingY != 0) {
        flingX = std::max(-maxFlingVelocity, std::min(flingX, maxFlingVelocity));
        flingY = std::max(-maxFlingVelocity, std::min(flingY, maxFlingVelocity));
        startNestedScrollForType(TYPE_NON_TOUCH);
        mViewFlinger->fling(flingX, flingY);
    }
    if (velocityX == 0 && velocityY == 0) {
        return flingX != 0 || flingY != 0;
    }
    if (!dispatchNestedPreFling(float(velocityX), float(velocityY))) {
        const bool canScroll = bCanScrollHorizontal || bCanScrollVertical;
        dispatchNestedFling(velocityX, velocityY, canScroll);
        if (mOnFlingListener && mOnFlingListener(velocityX, velocityY)){//->onFling(velocityX, velocityY)) {
            return true;
        }
        if (canScroll) {
            int nestedScrollAxis = View::SCROLL_AXIS_NONE;
            if (bCanScrollHorizontal) {
                nestedScrollAxis |= View::SCROLL_AXIS_HORIZONTAL;
            }
            if (bCanScrollVertical) {
                nestedScrollAxis |= View::SCROLL_AXIS_VERTICAL;
            }
            startNestedScroll(nestedScrollAxis, View::TYPE_NON_TOUCH);
            velocityX = std::max(-mMaxFlingVelocity, std::min(velocityX, mMaxFlingVelocity));
            velocityY = std::max(-mMaxFlingVelocity, std::min(velocityY, mMaxFlingVelocity));
            mViewFlinger->fling(velocityX, velocityY);
            return true;
        }
    }
    return false;
}

void RecyclerView::startNestedScrollForType(int type){
    const bool canScrollHorizontal = mLayout->canScrollHorizontally();
    const bool canScrollVertical = mLayout->canScrollVertically();
    int nestedScrollAxis = View::SCROLL_AXIS_NONE;
    if (canScrollHorizontal) {
        nestedScrollAxis |= View::SCROLL_AXIS_HORIZONTAL;
    }
    if (canScrollVertical) {
        nestedScrollAxis |= View::SCROLL_AXIS_VERTICAL;
    }
    startNestedScroll(nestedScrollAxis, type);
}

bool RecyclerView::shouldAbsorb(EdgeEffect* edgeEffect, int velocity, int size){
    if (velocity > 0) {
        return true;
    }
    const float distance = edgeEffect->getDistance() * size;

    // This is flinging without the spring, so let's see if it will fling past the overscroll
    const float flingDistance = getSplineFlingDistance(-velocity);

    return flingDistance < distance;
}

int RecyclerView::consumeFlingInHorizontalStretch(int unconsumedX){
    return consumeFlingInStretch(unconsumedX, mLeftGlow, mRightGlow, getWidth());
}

int RecyclerView::consumeFlingInVerticalStretch(int unconsumedY){
    return consumeFlingInStretch(unconsumedY, mTopGlow, mBottomGlow, getHeight());
}

int RecyclerView::consumeFlingInStretch(int unconsumed, EdgeEffect* startGlow, EdgeEffect* endGlow,int size){
    if (unconsumed > 0 && startGlow != nullptr && startGlow->getDistance() != 0.f) {
            const float deltaDistance = -unconsumed * FLING_DESTRETCH_FACTOR / size;
            const int consumed = std::round(-size / FLING_DESTRETCH_FACTOR
                    * startGlow->onPullDistance(deltaDistance, 0.5f));
            if (consumed != unconsumed) {
                startGlow->finish();
            }
            return unconsumed - consumed;
        }
        if (unconsumed < 0 && endGlow != nullptr && endGlow->getDistance() != 0.f) {
            const float deltaDistance = unconsumed * FLING_DESTRETCH_FACTOR / size;
            const int consumed = std::round(size / FLING_DESTRETCH_FACTOR
                    * endGlow->onPullDistance(deltaDistance, 0.5f));
            if (consumed != unconsumed) {
                endGlow->finish();
            }
            return unconsumed - consumed;
        }
        return unconsumed;
}

void RecyclerView::stopScroll() {
    setScrollState(SCROLL_STATE_IDLE);
    stopScrollersInternal();
}

void RecyclerView::stopScrollersInternal() {
    mViewFlinger->stop();
    if (mLayout != nullptr) {
        mLayout->stopSmoothScroller();
    }
}

int RecyclerView::getMinFlingVelocity() const{
    return mMinFlingVelocity;
}

int RecyclerView::getMaxFlingVelocity() const{
    return mMaxFlingVelocity;
}

void RecyclerView::pullGlows(MotionEvent* ev,
        float x,int horizontalAxis,float overscrollX,
        float y,int verticalAxis,float overscrollY){//float x, float overscrollX, float y, float overscrollY) {
    bool bInvalidate = false;
    if (overscrollX < 0) {
        ensureLeftGlow();
        mLeftGlow->onPull(-overscrollX / getWidth(), 1.f - y  / getHeight());
        if (ev != nullptr) {
            getScrollFeedbackProvider()->onScrollLimit(
                ev->getDeviceId(), ev->getSource(), horizontalAxis, /* isStart= */ true);
        }
        bInvalidate = true;
    } else if (overscrollX > 0) {
        ensureRightGlow();
        mRightGlow->onPull(overscrollX / getWidth(), y / getHeight());
        if (ev != nullptr) {
            getScrollFeedbackProvider()->onScrollLimit(
                ev->getDeviceId(), ev->getSource(), horizontalAxis, /* isStart= */ false);
        }
        bInvalidate = true;
    }

    if (overscrollY < 0) {
        ensureTopGlow();
        mTopGlow->onPull(-overscrollY / getHeight(), x / getWidth());
        if (ev != nullptr) {
            getScrollFeedbackProvider()->onScrollLimit(
                ev->getDeviceId(), ev->getSource(), verticalAxis, /* isStart= */ true);
        }
        bInvalidate = true;
    } else if (overscrollY > 0) {
        ensureBottomGlow();
        mBottomGlow->onPull(overscrollY / getHeight(), 1.f - x / getWidth());
        if (ev != nullptr) {
            getScrollFeedbackProvider()->onScrollLimit(
                ev->getDeviceId(), ev->getSource(), verticalAxis, /* isStart= */ false);
        }
        bInvalidate = true;
    }

    if (bInvalidate || (overscrollX != 0) || (overscrollY != 0)) {
        postInvalidateOnAnimation();
    }
}

void RecyclerView::releaseGlows() {
    bool needsInvalidate = false;
    if (mLeftGlow != nullptr) {
        mLeftGlow->onRelease();
        needsInvalidate = mLeftGlow->isFinished();
    }
    if (mTopGlow != nullptr) {
        mTopGlow->onRelease();
        needsInvalidate |= mTopGlow->isFinished();
    }
    if (mRightGlow != nullptr) {
        mRightGlow->onRelease();
        needsInvalidate |= mRightGlow->isFinished();
    }
    if (mBottomGlow != nullptr) {
        mBottomGlow->onRelease();
        needsInvalidate |= mBottomGlow->isFinished();
    }
    if (needsInvalidate) {
        postInvalidateOnAnimation();
    }
}

void RecyclerView::considerReleasingGlowsOnScroll(int dx, int dy) {
    bool needsInvalidate = false;
    if (mLeftGlow && !mLeftGlow->isFinished() && (dx > 0)) {
        mLeftGlow->onRelease();
        needsInvalidate = mLeftGlow->isFinished();
    }
    if (mRightGlow && !mRightGlow->isFinished() && (dx < 0)) {
        mRightGlow->onRelease();
        needsInvalidate |= mRightGlow->isFinished();
    }
    if (mTopGlow && !mTopGlow->isFinished() && (dy > 0)) {
        mTopGlow->onRelease();
        needsInvalidate |= mTopGlow->isFinished();
    }
    if (mBottomGlow && !mBottomGlow->isFinished() && (dy < 0)) {
        mBottomGlow->onRelease();
        needsInvalidate |= mBottomGlow->isFinished();
    }
    if (needsInvalidate) {
        postInvalidateOnAnimation();
    }
}

void RecyclerView::absorbGlows(int velocityX, int velocityY) {
    if (velocityX < 0) {
        ensureLeftGlow();
        if(mLeftGlow->isFinished())
            mLeftGlow->onAbsorb(-velocityX);
    } else if (velocityX > 0) {
        ensureRightGlow();
        if(mRightGlow->isFinished())
            mRightGlow->onAbsorb(velocityX);
    }

    if (velocityY < 0) {
        ensureTopGlow();
        if(mTopGlow->isFinished())
            mTopGlow->onAbsorb(-velocityY);
    } else if (velocityY > 0) {
        ensureBottomGlow();
        if(mBottomGlow->isFinished())
            mBottomGlow->onAbsorb(velocityY);
    }

    if ((velocityX != 0) || (velocityY != 0)) {
        postInvalidateOnAnimation();
    }
}

void RecyclerView::ensureLeftGlow() {
    if (mLeftGlow != nullptr) {
        return;
    }
    mLeftGlow = mEdgeEffectFactory->createEdgeEffect(*this, EdgeEffectFactory::DIRECTION_LEFT);
    if (mClipToPadding) {
        mLeftGlow->setSize(getMeasuredHeight() - getPaddingTop() - getPaddingBottom(),
                getMeasuredWidth() - getPaddingLeft() - getPaddingRight());
    } else {
        mLeftGlow->setSize(getMeasuredHeight(), getMeasuredWidth());
    }
}

void RecyclerView::ensureRightGlow() {
    if (mRightGlow != nullptr) {
        return;
    }
    mRightGlow = mEdgeEffectFactory->createEdgeEffect(*this, EdgeEffectFactory::DIRECTION_RIGHT);
    if (mClipToPadding) {
        mRightGlow->setSize(getMeasuredHeight() - getPaddingTop() - getPaddingBottom(),
                getMeasuredWidth() - getPaddingLeft() - getPaddingRight());
    } else {
        mRightGlow->setSize(getMeasuredHeight(), getMeasuredWidth());
    }
}

void RecyclerView::ensureTopGlow() {
    if (mTopGlow != nullptr)  {
        return;
    }
    mTopGlow = mEdgeEffectFactory->createEdgeEffect(*this, EdgeEffectFactory::DIRECTION_TOP);
    if (mClipToPadding) {
        mTopGlow->setSize(getMeasuredWidth() - getPaddingLeft() - getPaddingRight(),
                getMeasuredHeight() - getPaddingTop() - getPaddingBottom());
    } else {
        mTopGlow->setSize(getMeasuredWidth(), getMeasuredHeight());
    }

}

void RecyclerView::ensureBottomGlow() {
    if (mBottomGlow != nullptr) {
        return;
    }
    mBottomGlow = mEdgeEffectFactory->createEdgeEffect(*this, EdgeEffectFactory::DIRECTION_BOTTOM);
    if (mClipToPadding) {
        mBottomGlow->setSize(getMeasuredWidth() - getPaddingLeft() - getPaddingRight(),
                getMeasuredHeight() - getPaddingTop() - getPaddingBottom());
    } else {
        mBottomGlow->setSize(getMeasuredWidth(), getMeasuredHeight());
    }
}

void RecyclerView::invalidateGlows() {
    mLeftGlow = mRightGlow = mTopGlow = mBottomGlow = nullptr;
}

void RecyclerView::setEdgeEffectFactory(EdgeEffectFactory* edgeEffectFactory) {
    assert(edgeEffectFactory);//Preconditions.checkNotNull(edgeEffectFactory);
    mEdgeEffectFactory = edgeEffectFactory;
    invalidateGlows();
}

RecyclerView::EdgeEffectFactory* RecyclerView::getEdgeEffectFactory() {
    return mEdgeEffectFactory;
}

View* RecyclerView::focusSearch(View* focused, int direction){
    View* result = mLayout->onInterceptFocusSearch(focused, direction);
    if (result != nullptr) {
        return result;
    }
    const bool canRunFocusFailure = (mAdapter != nullptr) && (mLayout != nullptr)
            && !isComputingLayout() && !mLayoutSuppressed;

    FocusFinder& ff = FocusFinder::getInstance();
    if (canRunFocusFailure && (direction == View::FOCUS_FORWARD || direction == View::FOCUS_BACKWARD)) {
        // convert direction to absolute direction and see if we have a view there and if not
        // tell LayoutManager to add if it can.
        bool needsFocusFailureLayout = false;
        if (mLayout->canScrollVertically()) {
            const int absDir =  (direction == View::FOCUS_FORWARD) ? View::FOCUS_DOWN : View::FOCUS_UP;
            const View* found = ff.findNextFocus(this, focused, absDir);
            needsFocusFailureLayout = (found == nullptr);
        }
        if (!needsFocusFailureLayout && mLayout->canScrollHorizontally()) {
            bool rtl = mLayout->getLayoutDirection() == View::LAYOUT_DIRECTION_RTL;
            const int absDir = (direction == View::FOCUS_FORWARD) ^ rtl
                    ? View::FOCUS_RIGHT : View::FOCUS_LEFT;
            const View* found = ff.findNextFocus(this, focused, absDir);
            needsFocusFailureLayout = (found == nullptr);
        }
        if (needsFocusFailureLayout) {
            consumePendingUpdateOperations();
            const View* focusedItemView = findContainingItemView(focused);
            if (focusedItemView == nullptr) {
                // panic, focused view is not a child anymore, cannot call super.
                return nullptr;
            }
            startInterceptRequestLayout();
            mLayout->onFocusSearchFailed(focused, direction, *mRecycler, *mState);
            stopInterceptRequestLayout(false);
        }
        result = ff.findNextFocus(this, focused, direction);
    } else {
        result = ff.findNextFocus(this, focused, direction);
        if (result == nullptr && canRunFocusFailure) {
            consumePendingUpdateOperations();
            const View* focusedItemView = findContainingItemView(focused);
            if (focusedItemView == nullptr) {
                // panic, focused view is not a child anymore, cannot call super.
                return nullptr;
            }
            startInterceptRequestLayout();
            result = mLayout->onFocusSearchFailed(focused, direction, *mRecycler, *mState);
            stopInterceptRequestLayout(false);
        }
    }
    if (result && !result->hasFocusable()) {
        if (getFocusedChild() == nullptr) {
            // Scrolling to this unfocusable view is not meaningful since there is no currently
            // focused view which RV needs to keep visible.
            return ViewGroup::focusSearch(focused, direction);
        }
        // If the next view returned by onFocusSearchFailed in layout manager has no focusable
        // views, we still scroll to that view in order to make it visible on the screen.
        // If it's focusable, framework already calls RV's requestChildFocus which handles
        // bringing this newly focused item onto the screen.
        requestChildOnScreen(result, nullptr);
        return focused;
    }
    return isPreferredNextFocus(focused, result, direction)
            ? result : ViewGroup::focusSearch(focused, direction);
}

bool RecyclerView::isPreferredNextFocus(View* focused, View* next, int direction) {
    if ((next == nullptr) || (next == this) || (next==focused)) {
        return false;
    }
    // panic, result view is not a child anymore, maybe workaround b/37864393
    if (findContainingItemView(next) == nullptr) {
        return false;
    }
    if (focused == nullptr) {
        return true;
    }
    // panic, focused view is not a child anymore, maybe workaround b/37864393
    if (findContainingItemView(focused) == nullptr) {
        return true;
    }

    mTempRect.set(0, 0, focused->getWidth(), focused->getHeight());
    mTempRect2.set(0, 0, next->getWidth(), next->getHeight());
    offsetDescendantRectToMyCoords(focused, mTempRect);
    offsetDescendantRectToMyCoords(next, mTempRect2);
    const int rtl = (mLayout->getLayoutDirection() == View::LAYOUT_DIRECTION_RTL) ? -1 : 1;
    int rightness = 0;
    if ((mTempRect.left < mTempRect2.left
            || mTempRect.right() <= mTempRect2.left)
            && mTempRect.right() < mTempRect2.right()) {
        rightness = 1;
    } else if ((mTempRect.right() > mTempRect2.right()
            || mTempRect.left >= mTempRect2.right())
            && mTempRect.left > mTempRect2.left) {
        rightness = -1;
    }
    int downness = 0;
    if ((mTempRect.top < mTempRect2.top
            || mTempRect.bottom() <= mTempRect2.top)
            && mTempRect.bottom() < mTempRect2.bottom()) {
        downness = 1;
    } else if ((mTempRect.bottom() > mTempRect2.bottom()
            || mTempRect.top >= mTempRect2.bottom())
            && mTempRect.top > mTempRect2.top) {
        downness = -1;
    }
    switch (direction) {
    case View::FOCUS_LEFT:return rightness < 0;
    case View::FOCUS_RIGHT:return rightness > 0;
    case View::FOCUS_UP :return downness < 0;
    case View::FOCUS_DOWN:return downness > 0;
    case View::FOCUS_FORWARD:return downness > 0 || (downness == 0 && rightness * rtl > 0);
    case View::FOCUS_BACKWARD:return downness < 0 || (downness == 0 && rightness * rtl < 0);
    }
    FATAL("Invalid direction: %d",direction);
    return false;
}

void RecyclerView::requestChildFocus(View* child, View* focused) {
    if (!mLayout->onRequestChildFocus(*this, *mState, *child, focused) && focused != nullptr) {
        requestChildOnScreen(child, focused);
    }
    ViewGroup::requestChildFocus(child, focused);
}

void RecyclerView::requestChildOnScreen(View* child,View* focused) {
    View* rectView = (focused != nullptr) ? focused : child;
    mTempRect.set(0, 0, rectView->getWidth(), rectView->getHeight());

    // get item decor offsets w/o refreshing. If they are invalid, there will be another
    // layout pass to fix them, then it is LayoutManager's responsibility to keep focused
    // View in viewport.
    ViewGroup::LayoutParams* focusedLayoutParams = rectView->getLayoutParams();
    if (dynamic_cast<LayoutParams*>(focusedLayoutParams)) {
        // if focused child has item decors, use them. Otherwise, ignore.
        const LayoutParams* lp = (LayoutParams*) focusedLayoutParams;
        if (!lp->mInsetsDirty) {
            Rect insets = lp->mDecorInsets;
            mTempRect.left -= insets.left;
            mTempRect.width += insets.left+insets.width;
            mTempRect.top -= insets.top;
            mTempRect.height += insets.top+insets.height;
        }
    }

    if (focused != nullptr) {
        offsetDescendantRectToMyCoords(focused, mTempRect);
        offsetRectIntoDescendantCoords(child, mTempRect);
    }
    mLayout->requestChildRectangleOnScreen(*this, *child, mTempRect, !mFirstLayoutComplete,
            (focused == nullptr));
}

bool RecyclerView::requestChildRectangleOnScreen(View* child, Rect& rect, bool immediate) {
    return mLayout->requestChildRectangleOnScreen(*this, *child, rect, immediate);
}

void RecyclerView::addFocusables(std::vector<View*>& views, int direction, int focusableMode) {
    if (mLayout == nullptr || !mLayout->onAddFocusables(*this, views, direction, focusableMode)) {
	    ViewGroup::addFocusables(views, direction, focusableMode);
    }
}

bool RecyclerView::onRequestFocusInDescendants(int direction, Rect* previouslyFocusedRect) {
    if (isComputingLayout()) {
        // if we are in the middle of a layout calculation, don't let any child take focus.
        // RV will handle it after layout calculation is finished.
        return false;
    }
    return ViewGroup::onRequestFocusInDescendants(direction, previouslyFocusedRect);
}

void RecyclerView::onAttachedToWindow() {
    ViewGroup::onAttachedToWindow();
    mLayoutOrScrollCounter = 0;
    mIsAttached = true;
    mFirstLayoutComplete = mFirstLayoutComplete && !isLayoutRequested();

    mRecycler->onAttachedToWindow();

    if (mLayout != nullptr) {
        mLayout->dispatchAttachedToWindow(*this);
    }
    mPostedAnimatorRunner = false;

    if (ALLOW_THREAD_GAP_WORK) {
        // Register with gap worker
        mGapWorker = GapWorker::sGapWorker;//.get();
        if (mGapWorker == nullptr) {
            mGapWorker = new GapWorker();

            // break 60 fps assumption if data from display appears valid
            // NOTE: we only do this query once, statically, because it's very expensive (> 1ms)
            float refreshRate = 60.0f;
            /*Display display = ViewCompat.getDisplay(this);
            if (!isInEditMode() && display != null) {
                float displayRefreshRate = display.getRefreshRate();
                if (displayRefreshRate >= 30.0f) {
                    refreshRate = displayRefreshRate;
                }
            }*/
            mGapWorker->mFrameIntervalNs = (1000000000 / refreshRate);
            GapWorker::sGapWorker = mGapWorker;//.set(mGapWorker);
        }
        mGapWorker->add(this);
    }
}

void RecyclerView::onDetachedFromWindow() {
    ViewGroup::onDetachedFromWindow();
    if (mItemAnimator != nullptr) {
        mItemAnimator->endAnimations();
    }
    stopScroll();
    mIsAttached = false;
    if (mLayout != nullptr) {
        mLayout->dispatchDetachedFromWindow(*this, *mRecycler);
    }
    mPendingAccessibilityImportanceChange.clear();
    removeCallbacks(mItemAnimatorRunner);
    removeCallbacks(mUpdateChildViewsRunnable);
    mViewInfoStore->onDetach();
    mRecycler->onDetachedFromWindow();
    //TODO PoolingContainer::callPoolingContainerOnReleaseForChildren(this);

    if (ALLOW_THREAD_GAP_WORK && (mGapWorker != nullptr)) {
        // Unregister with gap worker
        mGapWorker->remove(this);
        mGapWorker = nullptr;
    }
}

bool RecyclerView::isAttachedToWindow()const {
    return mIsAttached;
}

void RecyclerView::assertInLayoutOrScroll(const std::string& message) {
    if (!isComputingLayout()) {
        if(message.empty())
            throw std::logic_error("Cannot call this method unless RecyclerView is "
            "computing a layout or scrolling");
        //throw new IllegalStateException(message + exceptionLabel());
    }
}

void RecyclerView::assertNotInLayoutOrScroll(const std::string& message) {
    if (isComputingLayout()) {
        LOGE_IF(message.empty(),"Cannot call this method while RecyclerView is "
            "computing a layout or scrolling");
        //throw new IllegalStateException(message);
    }
    if (mDispatchScrollCounter > 0) {
        LOGW("Cannot call this method in a scroll callback. Scroll callbacks might"
                         "be run during a measure & layout pass where you cannot change the"
                         "RecyclerView data. Any method call that might change the structure"
                         "of the RecyclerView or the adapter contents should be postponed to"
                         "the next frame.");
        //new IllegalStateException("" + exceptionLabel()));
    }
}

void RecyclerView::addOnItemTouchListener(OnItemTouchListener listener) {
    auto it = std::find(mOnItemTouchListeners.begin(),mOnItemTouchListeners.end(),listener);
    if(it==mOnItemTouchListeners.end())
    mOnItemTouchListeners.push_back(listener);
}

void RecyclerView::removeOnItemTouchListener(OnItemTouchListener listener) {
    //mOnItemTouchListeners.remove(listener);
    auto it = std::find(mOnItemTouchListeners.begin(),mOnItemTouchListeners.end(),listener);
    if(mOnItemTouchListeners.end()!=it){
        if (mInterceptingOnItemTouchListener == &(*it)) {
            mInterceptingOnItemTouchListener = nullptr;
        }
    }
    mOnItemTouchListeners.erase(it);
}

bool RecyclerView::dispatchToOnItemTouchListeners(MotionEvent& e) {

    // OnItemTouchListeners should receive calls to their methods in the same pattern that
    // ViewGroups do. That pattern is a bit confusing, which in turn makes the below code a
    // bit confusing.  Here are rules for the pattern:
    //
    // 1. A single MotionEvent should not be passed to either OnInterceptTouchEvent or
    // OnTouchEvent twice.
    // 2. ACTION_DOWN MotionEvents may be passed to both onInterceptTouchEvent and
    // onTouchEvent.
    // 3. All other MotionEvents should be passed to either onInterceptTouchEvent or
    // onTouchEvent, not both.

    // Side Note: We don't currently perfectly mimic how MotionEvents work in the view system.
    // If we were to do so, for every MotionEvent, any OnItemTouchListener that is before the
    // intercepting OnItemTouchEvent should still have a chance to intercept, and if it does,
    // the previously intercepting OnItemTouchEvent should get an ACTION_CANCEL event.

    if (mInterceptingOnItemTouchListener == nullptr) {
        if (e.getAction() == MotionEvent::ACTION_DOWN) {
            return false;
        }
        return findInterceptingOnItemTouchListener(e);
    } else {
        mInterceptingOnItemTouchListener->onTouchEvent(*this, e);
        const int action = e.getAction();
        if (action == MotionEvent::ACTION_CANCEL || action == MotionEvent::ACTION_UP) {
            mInterceptingOnItemTouchListener = nullptr;
        }
        return true;
    }
}

bool RecyclerView::findInterceptingOnItemTouchListener(MotionEvent& e) {
    const int action = e.getAction();
    const int listenerCount = mOnItemTouchListeners.size();
    for (int i = 0; i < listenerCount; i++) {
        OnItemTouchListener& listener = mOnItemTouchListeners.at(i);
        if (listener.onInterceptTouchEvent(*this, e) && action != MotionEvent::ACTION_CANCEL) {
            mInterceptingOnItemTouchListener = &listener;
            return true;
        }
    }
    return false;
}

bool RecyclerView::onInterceptTouchEvent(MotionEvent& e) {
    if (mLayoutSuppressed) {
        // When layout is frozen,  RV does not intercept the motion event.
        // A child view e.g. a button may still get the click.
        return false;
    }

    // Clear the active onInterceptTouchListener.  None should be set at this time, and if one
    // is, it's because some other code didn't follow the standard contract.
    mInterceptingOnItemTouchListener = nullptr;
    if (findInterceptingOnItemTouchListener(e)) {
        cancelScroll();
        MotionEvent* cancelEvent = MotionEvent::obtain(e);
        cancelEvent->setAction(MotionEvent::ACTION_CANCEL);
        const int listenerCount = mOnItemTouchListeners.size();
        for (int i = 0; i < listenerCount; i++) {
            OnItemTouchListener& listener = mOnItemTouchListeners.at(i);
            if (/*listener == nullptr ||*/ &listener == mInterceptingOnItemTouchListener) {
                continue;
            } else {
                listener.onInterceptTouchEvent(*this, *cancelEvent);
            }
        }
        cancelEvent->recycle();
        return true;
    }

    if (mLayout == nullptr) {
        return false;
    }

    const bool canScrollHorizontally = mLayout->canScrollHorizontally();
    const bool canScrollVertically = mLayout->canScrollVertically();

    if (mVelocityTracker == nullptr) {
        mVelocityTracker = VelocityTracker::obtain();
    }
    mVelocityTracker->addMovement(e);

    const int action = e.getActionMasked();
    const int actionIndex = e.getActionIndex();
    int nestedScrollAxis;
    switch (action) {
    case MotionEvent::ACTION_DOWN:
         if (mIgnoreMotionEventTillDown) {
             mIgnoreMotionEventTillDown = false;
         }
         mScrollPointerId = e.getPointerId(0);
         mInitialTouchX = mLastTouchX = (int) (e.getX() + 0.5f);
         mInitialTouchY = mLastTouchY = (int) (e.getY() + 0.5f);

         if (stopGlowAnimations(e) ||(mScrollState == SCROLL_STATE_SETTLING)) {
             getParent()->requestDisallowInterceptTouchEvent(true);
             setScrollState(SCROLL_STATE_DRAGGING);
             stopNestedScroll(TYPE_NON_TOUCH);
         }

         // Clear the nested offsets
         mNestedOffsets[0] = mNestedOffsets[1] = 0;
         startNestedScrollForType(TYPE_TOUCH);
         break;

    case MotionEvent::ACTION_POINTER_DOWN:
         mScrollPointerId = e.getPointerId(actionIndex);
         mInitialTouchX = mLastTouchX = (int) (e.getX(actionIndex) + 0.5f);
         mInitialTouchY = mLastTouchY = (int) (e.getY(actionIndex) + 0.5f);
         break;

    case MotionEvent::ACTION_MOVE: {
            const int index = e.findPointerIndex(mScrollPointerId);
            if (index < 0) {
                LOGE("Error processing scroll; pointer index for id %d"
                     " not found. Did any MotionEvents get skipped?",mScrollPointerId);
                return false;
            }

            const int x = (int) (e.getX(index) + 0.5f);
            const int y = (int) (e.getY(index) + 0.5f);
            if (mScrollState != SCROLL_STATE_DRAGGING) {
                const int dx = x - mInitialTouchX;
                const int dy = y - mInitialTouchY;
                bool startScroll = false;
                if (canScrollHorizontally && (std::abs(dx) > mTouchSlop)) {
                    mLastTouchX = x;
                    startScroll = true;
                }
                if (canScrollVertically && (std::abs(dy) > mTouchSlop)) {
                    mLastTouchY = y;
                    startScroll = true;
                }
                if (startScroll) {
                    setScrollState(SCROLL_STATE_DRAGGING);
                }
            }
        } break;

    case MotionEvent::ACTION_POINTER_UP:
         onPointerUp(e);
         break;

    case MotionEvent::ACTION_UP:
         mVelocityTracker->clear();
         stopNestedScroll(View::TYPE_TOUCH);
         break;

    case MotionEvent::ACTION_CANCEL:
         cancelScroll();
    }
    return mScrollState == SCROLL_STATE_DRAGGING;
}

bool RecyclerView::stopGlowAnimations(MotionEvent& e) {
    bool stopped = false;
    if ((mLeftGlow != nullptr) && (mLeftGlow->getDistance() != 0)
            && !canScrollHorizontally(-1)) {
        mLeftGlow->onPullDistance(0, 1 - (e.getY() / getHeight()));
        stopped = true;
    }
    if ((mRightGlow != nullptr) && (mRightGlow->getDistance() != 0)
            && !canScrollHorizontally(1)) {
        mRightGlow->onPullDistance( 0, e.getY() / getHeight());
        stopped = true;
    }
    if ((mTopGlow != nullptr) && (mTopGlow->getDistance() != 0)
            && !canScrollVertically(-1)) {
        mTopGlow->onPullDistance( 0, e.getX() / getWidth());
        stopped = true;
    }
    if ((mBottomGlow != nullptr) && (mBottomGlow->getDistance() != 0)
            && !canScrollVertically(1)) {
        mBottomGlow->onPullDistance(0, 1 - e.getX() / getWidth());
        stopped = true;
    }
    return stopped;
}

void RecyclerView::requestDisallowInterceptTouchEvent(bool disallowIntercept) {
    const int listenerCount = (int)mOnItemTouchListeners.size();
    for (int i = 0; i < listenerCount; i++) {
        OnItemTouchListener listener = mOnItemTouchListeners.at(i);
        listener.onRequestDisallowInterceptTouchEvent(disallowIntercept);
    }
    ViewGroup::requestDisallowInterceptTouchEvent(disallowIntercept);
}

bool RecyclerView::onTouchEvent(MotionEvent& e) {
    if (mLayoutSuppressed || mIgnoreMotionEventTillDown) {
        return false;
    }
    if (dispatchToOnItemTouchListeners(e)) {
        cancelScroll();
        return true;
    }

    if (mLayout == nullptr) {
        return false;
    }

    const bool bCanScrollHorizontally = mLayout->canScrollHorizontally();
    const bool bCanScrollVertically = mLayout->canScrollVertically();

    if (mVelocityTracker == nullptr) {
        mVelocityTracker = VelocityTracker::obtain();
    }
    bool eventAddedToVelocityTracker = false;

    const int action = e.getActionMasked();
    const int actionIndex = e.getActionIndex();

    if (action == MotionEvent::ACTION_DOWN) {
        mNestedOffsets[0] = mNestedOffsets[1] = 0;
    }
    MotionEvent* vtev = MotionEvent::obtain(e);
    vtev->offsetLocation(float(mNestedOffsets[0]), float(mNestedOffsets[1]));

    switch (action) {
    case MotionEvent::ACTION_DOWN:
        mScrollPointerId = e.getPointerId(0);
        mInitialTouchX = mLastTouchX = (int) (e.getX() + 0.5f);
        mInitialTouchY = mLastTouchY = (int) (e.getY() + 0.5f);
        startNestedScrollForType(TYPE_TOUCH);
        break;

    case MotionEvent::ACTION_POINTER_DOWN:
        mScrollPointerId = e.getPointerId(actionIndex);
        mInitialTouchX = mLastTouchX = (int) (e.getX(actionIndex) + 0.5f);
        mInitialTouchY = mLastTouchY = (int) (e.getY(actionIndex) + 0.5f);
        break;

    case MotionEvent::ACTION_MOVE: {
            const int index = e.findPointerIndex(mScrollPointerId);
            if (index < 0) {
                LOGE("Error processing scroll; pointer index for id %d=%d"
                     " not found. Did any MotionEvents get skipped?",mScrollPointerId,index);
                return false;
            }

            const int x = (int) (e.getX(index) + 0.5f);
            const int y = (int) (e.getY(index) + 0.5f);
            int dx = mLastTouchX - x;
            int dy = mLastTouchY - y;

            if (mScrollState != SCROLL_STATE_DRAGGING) {
                bool startScroll = false;
                if (bCanScrollHorizontally) {
                    if (dx > 0) {
                        dx = std::max(0,dx-mTouchSlop);
                    } else {
                        dx = std::min(0,dx+mTouchSlop);
                    }
                    if(dx!=0)startScroll = true;
                }
                if (bCanScrollVertically) {
                    if (dy > 0) {
                        dy = std::max(0,dy-mTouchSlop);
                    } else {
                        dy = std::min(0,dy+mTouchSlop);
                    }
                    if(dy!=0)startScroll = true;
                }
                if (startScroll) {
                    setScrollState(SCROLL_STATE_DRAGGING);
                }
            }

            if (mScrollState == SCROLL_STATE_DRAGGING) {
                mReusableIntPair[0] = 0;
                mReusableIntPair[1] = 0;
               
                if (dispatchNestedPreScroll(
                            (bCanScrollHorizontally ? dx : 0),
                            (bCanScrollVertically ? dy : 0),
                            mReusableIntPair, mScrollOffset, TYPE_TOUCH)) {
                   dx -= mReusableIntPair[0];
                   dy -= mReusableIntPair[1];
                   // Updated the nested offsets
                   mNestedOffsets[0] += mScrollOffset[0];
                   mNestedOffsets[1] += mScrollOffset[1];
                   // Scroll has initiated, prevent parents from intercepting
                   getParent()->requestDisallowInterceptTouchEvent(true);
                }

                mLastTouchX = x - mScrollOffset[0];
                mLastTouchY = y - mScrollOffset[1];

                if (scrollByInternal(
                        bCanScrollHorizontally ? dx : 0,
                        bCanScrollVertically ? dy : 0,
                        MotionEvent::AXIS_X, MotionEvent::AXIS_Y,
                        &e,TYPE_TOUCH)) {
                    getParent()->requestDisallowInterceptTouchEvent(true);
                }
                if ((mGapWorker != nullptr) && (dx != 0 || dy != 0)) {
                    mGapWorker->postFromTraversal(this, dx, dy);
                }
            }
        } break;

    case MotionEvent::ACTION_POINTER_UP:
        onPointerUp(e);
        break;

    case MotionEvent::ACTION_UP: {
            mVelocityTracker->addMovement(*vtev);
            eventAddedToVelocityTracker = true;
            mVelocityTracker->computeCurrentVelocity(1000, float(mMaxFlingVelocity));
            const float xvel = bCanScrollHorizontally ? -mVelocityTracker->getXVelocity(mScrollPointerId) : 0;
            const float yvel = bCanScrollVertically ? -mVelocityTracker->getYVelocity(mScrollPointerId) : 0;
            if (!(((xvel != 0) || (yvel != 0)) && fling((int) xvel, (int) yvel))) {
                setScrollState(SCROLL_STATE_IDLE);
            }
            resetScroll();
        } break;

    case MotionEvent::ACTION_CANCEL:
        cancelScroll();
        break;
    }

    if (!eventAddedToVelocityTracker) {
        mVelocityTracker->addMovement(*vtev);
    }
    vtev->recycle();

    return true;
}

void RecyclerView::resetScroll() {
    if (mVelocityTracker != nullptr)  {
        mVelocityTracker->clear();
    }
    stopNestedScroll(View::TYPE_TOUCH);
    releaseGlows();
}

void RecyclerView::cancelScroll() {
    resetScroll();
    setScrollState(SCROLL_STATE_IDLE);
}

void RecyclerView::onPointerUp(MotionEvent& e) {
    const int actionIndex = e.getActionIndex();
    if (e.getPointerId(actionIndex) == mScrollPointerId) {
        // Pick a new pointer to pick up the slack.
        const int newIndex = (actionIndex == 0) ? 1 : 0;
        mScrollPointerId = e.getPointerId(newIndex);
        mInitialTouchX = mLastTouchX = (int) (e.getX(newIndex) + 0.5f);
        mInitialTouchY = mLastTouchY = (int) (e.getY(newIndex) + 0.5f);
    }
}

bool RecyclerView::onGenericMotionEvent(MotionEvent& event) {
    if (mLayout == nullptr) {
        return false;
    }
    if (mLayoutSuppressed) {
        return false;
    }
    int flingAxis = 0;
    int horizontalAxis =0;
    int verticalAxis = 0;
    bool useSmoothScroll = false;
    if (event.getAction() == MotionEvent::ACTION_SCROLL) {
        float vScroll, hScroll;
        if ((event.getSource() & InputDevice::SOURCE_CLASS_POINTER) != 0) {
            if (mLayout->canScrollVertically()) {
                // Inverse the sign of the vertical scroll to align the scroll orientation
                // with AbsListView.
                vScroll = -event.getAxisValue(MotionEvent::AXIS_VSCROLL);
                verticalAxis = MotionEvent::AXIS_VSCROLL;
            } else {
                vScroll = 0.f;
            }
            if (mLayout->canScrollHorizontally()) {
                hScroll = event.getAxisValue((int)MotionEvent::AXIS_HSCROLL);
                horizontalAxis = MotionEvent::AXIS_HSCROLL;
            } else {
                hScroll = 0.f;
            }
        } else if ((event.getSource() & InputDevice::SOURCE_ROTARY_ENCODER) != 0) {
            const float axisScroll = event.getAxisValue(MotionEvent::AXIS_SCROLL);
            if (mLayout->canScrollVertically()) {
                // Invert the sign of the vertical scroll to align the scroll orientation
                // with AbsListView.
                vScroll = -axisScroll;
                verticalAxis = MotionEvent::AXIS_SCROLL;
                hScroll = 0.f;
            } else if (mLayout->canScrollHorizontally()) {
                vScroll = 0.f;
                hScroll = axisScroll;
                horizontalAxis = MotionEvent::AXIS_SCROLL;
            } else {
                vScroll = 0.f;
                hScroll = 0.f;
            }
            // Use smooth scrolling for low resolution rotary encoders to avoid the visible
            // pixel jumps that would be caused by doing regular scrolling.
            useSmoothScroll = mLowResRotaryEncoderFeature;
            // Support fling for rotary encoders.
            flingAxis = MotionEvent::AXIS_SCROLL;
        } else {
            vScroll = 0.f;
            hScroll = 0.f;
        }

        int scaledVScroll = (int) (vScroll * mScaledVerticalScrollFactor);
        int scaledHScroll = (int) (hScroll * mScaledHorizontalScrollFactor);
        if (useSmoothScroll) {
            OverScroller* overScroller = mViewFlinger->mOverScroller;
            // Account for any remaining scroll from a previous generic motion event.
            scaledVScroll += overScroller->getFinalY() - overScroller->getCurrY();
            scaledHScroll += overScroller->getFinalX() - overScroller->getCurrX();
            smoothScrollBy(scaledHScroll, scaledVScroll, /* interpolator= */ nullptr,
                    UNDEFINED_DURATION, /* withNestedScrolling= */ true);
        } else {
            nestedScrollByInternal(scaledHScroll, scaledVScroll, horizontalAxis,
                    verticalAxis, &event, TYPE_NON_TOUCH);
        }

        if (flingAxis != 0 && !useSmoothScroll) {
            //TODO:mDifferentialMotionFlingController->onMotionEvent(event, flingAxis);
        }
    }
    return false;
}

void RecyclerView::onMeasure(int widthSpec, int heightSpec) {
    if (mLayout == nullptr) {
        defaultOnMeasure(widthSpec, heightSpec);
        return;
    }
    if (mLayout->isAutoMeasureEnabled()) {
        const int widthMode = MeasureSpec::getMode(widthSpec);
        const int heightMode = MeasureSpec::getMode(heightSpec);

        /**
         * This specific call should be considered deprecated and replaced with
         * {@link #defaultOnMeasure(int, int)}. It can't actually be replaced as it could
         * break existing third party code but all documentation directs developers to not
         * override {@link LayoutManager#onMeasure(int, int)} when
         * {@link LayoutManager#isAutoMeasureEnabled()} returns true.
         */
        mLayout->onMeasure(*mRecycler, *mState, widthSpec, heightSpec);
        // Calculate and track whether we should skip measurement here because the MeasureSpec
        // modes in both dimensions are EXACTLY.
        mLastAutoMeasureSkippedDueToExact =(widthMode == MeasureSpec::EXACTLY) && (heightMode == MeasureSpec::EXACTLY);
        if (mLastAutoMeasureSkippedDueToExact || (mAdapter == nullptr)) {
            return;
        }

        if (mState->mLayoutStep == State::STEP_START) {
            dispatchLayoutStep1();
        }
        // set dimensions in 2nd step. Pre-layout should happen with old dimensions for
        // consistency
        mLayout->setMeasureSpecs(widthSpec, heightSpec);
        mState->mIsMeasuring = true;
        dispatchLayoutStep2();

        // now we can get the width and height from the children.
        mLayout->setMeasuredDimensionFromChildren(widthSpec, heightSpec);

        // if RecyclerView has non-exact width and height and if there is at least one child
        // which also has non-exact width & height, we have to re-measure.
        if (mLayout->shouldMeasureTwice()) {
            mLayout->setMeasureSpecs(
                    MeasureSpec::makeMeasureSpec(getMeasuredWidth(), MeasureSpec::EXACTLY),
                    MeasureSpec::makeMeasureSpec(getMeasuredHeight(), MeasureSpec::EXACTLY));
            mState->mIsMeasuring = true;
            dispatchLayoutStep2();
            // now we can get the width and height from the children.
            mLayout->setMeasuredDimensionFromChildren(widthSpec, heightSpec);
        }
        mLastAutoMeasureNonExactMeasuredWidth = getMeasuredWidth();
        mLastAutoMeasureNonExactMeasuredHeight= getMeasuredHeight();
    } else {
        if (mHasFixedSize) {
            mLayout->onMeasure(*mRecycler, *mState, widthSpec, heightSpec);
            return;
        }
        // custom onMeasure
        if (mAdapterUpdateDuringMeasure) {
            startInterceptRequestLayout();
            onEnterLayoutOrScroll();
            processAdapterUpdatesAndSetAnimationFlags();
            onExitLayoutOrScroll();

            if (mState->mRunPredictiveAnimations) {
                mState->mInPreLayout = true;
            } else {
                // consume remaining updates to provide a consistent state with the layout pass.
                mAdapterHelper->consumeUpdatesInOnePass();
                mState->mInPreLayout = false;
            }
            mAdapterUpdateDuringMeasure = false;
            stopInterceptRequestLayout(false);
        } else if (mState->mRunPredictiveAnimations) {
            // If mAdapterUpdateDuringMeasure is false and mRunPredictiveAnimations is true:
            // this means there is already an onMeasure() call performed to handle the pending
            // adapter change, two onMeasure() calls can happen if RV is a child of LinearLayout
            // with layout_width=MATCH_PARENT. RV cannot call LM.onMeasure() second time
            // because getViewForPosition() will crash when LM uses a child to measure.
            setMeasuredDimension(getMeasuredWidth(), getMeasuredHeight());
            return;
        }

        if (mAdapter != nullptr) {
            mState->mItemCount = mAdapter->getItemCount();
        } else {
            mState->mItemCount = 0;
        }
        startInterceptRequestLayout();
        mLayout->onMeasure(*mRecycler, *mState, widthSpec, heightSpec);
        stopInterceptRequestLayout(false);
        mState->mInPreLayout = false; // clear
    }
}

void RecyclerView::defaultOnMeasure(int widthSpec, int heightSpec) {
    // calling LayoutManager here is not pretty but that API is already public and it is better
    // than creating another method since this is internal.
    const int width = LayoutManager::chooseSize(widthSpec,
            getPaddingLeft() + getPaddingRight(),getMinimumWidth());
    const int height = LayoutManager::chooseSize(heightSpec,
            getPaddingTop() + getPaddingBottom(),getMinimumHeight());
    setMeasuredDimension(width, height);
}

void RecyclerView::onSizeChanged(int w, int h, int oldw, int oldh) {
    ViewGroup::onSizeChanged(w, h, oldw, oldh);
    if (w != oldw || h != oldh) {
        invalidateGlows();
        // layout's w/h are updated during measure/layout steps.
    }
}

void RecyclerView::setItemAnimator(ItemAnimator* animator) {
    if (mItemAnimator != nullptr) {
        mItemAnimator->endAnimations();
        mItemAnimator->setListener(nullptr);
        delete mItemAnimator;
    }
    mItemAnimator = animator;
    if (mItemAnimator != nullptr) {
        mItemAnimator->setListener(mItemAnimatorListener);
    }
}

void RecyclerView::onEnterLayoutOrScroll() {
    mLayoutOrScrollCounter++;
}

void RecyclerView::onExitLayoutOrScroll() {
    onExitLayoutOrScroll(true);
}

void RecyclerView::onExitLayoutOrScroll(bool enableChangeEvents) {
    mLayoutOrScrollCounter--;
    if (mLayoutOrScrollCounter < 1) {
        if (mLayoutOrScrollCounter < 0) {
            LOGE("layout or scroll counter cannot go below zero.Some calls are not matching");
        }
        mLayoutOrScrollCounter = 0;
        if (enableChangeEvents) {
            dispatchContentChangedIfNecessary();
            dispatchPendingImportantForAccessibilityChanges();
        }
    }
}

bool RecyclerView::isAccessibilityEnabled() {
    return (mAccessibilityManager != nullptr) && mAccessibilityManager->isEnabled();
}

void RecyclerView::dispatchContentChangedIfNecessary() {
    const int flags = mEatenAccessibilityChangeFlags;
    mEatenAccessibilityChangeFlags = 0;
    if ((flags != 0) && isAccessibilityEnabled()) {
        AccessibilityEvent* event = AccessibilityEvent::obtain();
        event->setEventType(AccessibilityEvent::TYPE_WINDOW_CONTENT_CHANGED);
        event->setContentChangeTypes(flags);
        sendAccessibilityEventUnchecked(*event);
    }
}

bool RecyclerView::isComputingLayout() {
    return mLayoutOrScrollCounter > 0;
}

bool RecyclerView::shouldDeferAccessibilityEvent(AccessibilityEvent& event) {
    if (isComputingLayout()) {
        int type = 0;
        /*if (event != nullptr) {
            type = event.getContentChangeTypes();
        }
        if (type == 0) */{
            type = AccessibilityEvent::CONTENT_CHANGE_TYPE_UNDEFINED;
        }
        mEatenAccessibilityChangeFlags |= type;
        return true;
    }
    return false;
}

void RecyclerView::sendAccessibilityEventUnchecked(AccessibilityEvent& event) {
    if (shouldDeferAccessibilityEvent(event)) {
        return;
    }
    ViewGroup::sendAccessibilityEventUnchecked(event);
}

bool RecyclerView::dispatchPopulateAccessibilityEvent(AccessibilityEvent& event) {
    onPopulateAccessibilityEvent(event);
    return true;
}

RecyclerView::ItemAnimator* RecyclerView::getItemAnimator() {
    return mItemAnimator;
}

void RecyclerView::postAnimationRunner() {
    if (!mPostedAnimatorRunner && mIsAttached) {
        postOnAnimation(mItemAnimatorRunner);
        mPostedAnimatorRunner = true;
    }
}

bool RecyclerView::predictiveItemAnimationsEnabled() {
    return (mItemAnimator != nullptr && mLayout->supportsPredictiveItemAnimations());
}

void RecyclerView::processAdapterUpdatesAndSetAnimationFlags() {
    if (mDataSetHasChangedAfterLayout) {
        // Processing these items have no value since data set changed unexpectedly.
        // Instead, we just reset it.
        mAdapterHelper->reset();
        if (mDispatchItemsChangedEvent) {
            mLayout->onItemsChanged(*this);
        }
    }
    // simple animations are a subset of advanced animations (which will cause a
    // pre-layout step)
    // If layout supports predictive animations, pre-process to decide if we want to run them
    if (predictiveItemAnimationsEnabled()) {
        mAdapterHelper->preProcess();
    } else {
        mAdapterHelper->consumeUpdatesInOnePass();
    }
    const bool animationTypeSupported = mItemsAddedOrRemoved || mItemsChanged;
    mState->mRunSimpleAnimations = mFirstLayoutComplete && (mItemAnimator != nullptr)
            && (mDataSetHasChangedAfterLayout || animationTypeSupported || mLayout->mRequestedSimpleAnimations)
            && (!mDataSetHasChangedAfterLayout|| mAdapter->hasStableIds());
    mState->mRunPredictiveAnimations = mState->mRunSimpleAnimations && animationTypeSupported
            && !mDataSetHasChangedAfterLayout  && predictiveItemAnimationsEnabled();
}

void RecyclerView::dispatchLayout() {
    if (mAdapter == nullptr) {
        LOGE("No adapter attached; skipping layout");
        // leave the state in START
        return;
    }
    if (mLayout == nullptr) {
        LOGE("No layout manager attached; skipping layout");
        // leave the state in START
        return;
    }
    mState->mIsMeasuring = false;
    // If the last time we measured children in onMeasure, we skipped the measurement and layout
    // of RV children because the MeasureSpec in both dimensions was EXACTLY, and current
    // dimensions of the RV are not equal to the last measured dimensions of RV, we need to
    // measure and layout children one last time.
    const bool needsRemeasureDueToExactSkip = mLastAutoMeasureSkippedDueToExact
               && (mLastAutoMeasureNonExactMeasuredWidth != getWidth()
               || mLastAutoMeasureNonExactMeasuredHeight != getHeight());
    mLastAutoMeasureNonExactMeasuredWidth = 0;
    mLastAutoMeasureNonExactMeasuredHeight= 0;
    mLastAutoMeasureSkippedDueToExact = false;

    if (mState->mLayoutStep == State::STEP_START) {
        dispatchLayoutStep1();
        mLayout->setExactMeasureSpecsFrom(this);
        dispatchLayoutStep2();
    } else if (mAdapterHelper->hasUpdates() || needsRemeasureDueToExactSkip
            || (mLayout->getWidth() != getWidth()) || (mLayout->getHeight() != getHeight())) {
        // TODO(shepshapard): Worth a note that I believe
        //  "mLayout.getWidth() != getWidth() || mLayout.getHeight() != getHeight()" above is
        //  not actually correct, causes unnecessary work to be done, and should be
        //  removed. Removing causes many tests to fail and I didn't have the time to
        //  investigate. Just a note for the a future reader or bug fixer.
        mLayout->setExactMeasureSpecsFrom(this);
        dispatchLayoutStep2();
    } else {
        // always make sure we sync them (to ensure mode is exact)
        mLayout->setExactMeasureSpecsFrom(this);
    }
    dispatchLayoutStep3();
}

void RecyclerView::saveFocusInfo() {
    View* child = nullptr;
    if (mPreserveFocusAfterLayout && hasFocus() && mAdapter != nullptr) {
        child = getFocusedChild();
    }

    ViewHolder* focusedVh = (child == nullptr) ? nullptr : findContainingViewHolder(child);
    if (focusedVh == nullptr) {
        resetFocusInfo();
    } else {
        mState->mFocusedItemId = mAdapter->hasStableIds() ? focusedVh->getItemId() : NO_ID;
        // mFocusedItemPosition should hold the current adapter position of the previously
        // focused item. If the item is removed, we store the previous adapter position of the
        // removed item.
        mState->mFocusedItemPosition = mDataSetHasChangedAfterLayout ? NO_POSITION
                : (focusedVh->isRemoved() ? focusedVh->mOldPosition
                        : focusedVh->getAbsoluteAdapterPosition());
        mState->mFocusedSubChildId = getDeepestFocusedViewWithId(focusedVh->itemView);
    }
}

void RecyclerView::resetFocusInfo() {
    mState->mFocusedItemId = NO_ID;
    mState->mFocusedItemPosition = NO_POSITION;
    mState->mFocusedSubChildId = View::NO_ID;
}

View* RecyclerView::findNextViewToFocus() {
    int startFocusSearchIndex = mState->mFocusedItemPosition != -1 ? mState->mFocusedItemPosition: 0;
    ViewHolder* nextFocus;
    const int itemCount = mState->getItemCount();
    for (int i = startFocusSearchIndex; i < itemCount; i++) {
        nextFocus = findViewHolderForAdapterPosition(i);
        if (nextFocus == nullptr) {
            break;
        }
        if (nextFocus->itemView->hasFocusable()) {
            return nextFocus->itemView;
        }
    }
    const int limit = std::min(itemCount, startFocusSearchIndex);
    for (int i = limit - 1; i >= 0; i--) {
        nextFocus = findViewHolderForAdapterPosition(i);
        if (nextFocus == nullptr) {
            return nullptr;
        }
        if (nextFocus->itemView->hasFocusable()) {
            return nextFocus->itemView;
        }
    }
    return nullptr;
}

void RecyclerView::recoverFocusFromState() {
    if (!mPreserveFocusAfterLayout || (mAdapter == nullptr) || !hasFocus()
            || (getDescendantFocusability() == FOCUS_BLOCK_DESCENDANTS)
            || (getDescendantFocusability() == FOCUS_BEFORE_DESCENDANTS && isFocused())) {
        // No-op if either of these cases happens:
        // 1. RV has no focus, or 2. RV blocks focus to its children, or 3. RV takes focus
        // before its children and is focused (i.e. it already stole the focus away from its
        // descendants).
        return;
    }
    // only recover focus if RV itself has the focus or the focused view is hidden
    if (!isFocused()) {
        View* focusedChild = getFocusedChild();
        if (!mChildHelper->isHidden(focusedChild)) {
            // If the currently focused child is hidden, apply the focus recovery logic.
            // Otherwise return, i.e. the currently (unhidden) focused child is good enough :/.
            return;
        }
    }
    ViewHolder* focusTarget = nullptr;
    // RV first attempts to locate the previously focused item to request focus on using
    // mFocusedItemId. If such an item no longer exists, it then makes a best-effort attempt to
    // find the next best candidate to request focus on based on mFocusedItemPosition.
    if ((mState->mFocusedItemId != NO_ID) && mAdapter->hasStableIds()) {
        focusTarget = findViewHolderForItemId(mState->mFocusedItemId);
    }
    View* viewToFocus = nullptr;
    if ((focusTarget == nullptr) || mChildHelper->isHidden(focusTarget->itemView)
            || !focusTarget->itemView->hasFocusable()) {
        if (mChildHelper->getChildCount() > 0) {
            // At this point, RV has focus and either of these conditions are true:
            // 1. There's no previously focused item either because RV received focused before
            // layout, or the previously focused item was removed, or RV doesn't have stable IDs
            // 2. Previous focus child is hidden, or 3. Previous focused child is no longer
            // focusable. In either of these cases, we make sure that RV still passes down the
            // focus to one of its focusable children using a best-effort algorithm.
            viewToFocus = findNextViewToFocus();
        }
    } else {
        // looks like the focused item has been replaced with another view that represents the
        // same item in the adapter. Request focus on that.
        viewToFocus = focusTarget->itemView;
    }

    if (viewToFocus != nullptr) {
        if (mState->mFocusedSubChildId != NO_ID) {
            View* child = viewToFocus->findViewById(mState->mFocusedSubChildId);
            if (child && child->isFocusable()) {
                viewToFocus = child;
            }
        }
        viewToFocus->requestFocus();
    }
}

int RecyclerView::getDeepestFocusedViewWithId(View* view) {
    int lastKnownId = view->getId();
    while (!view->isFocused() && dynamic_cast<ViewGroup*>(view) && view->hasFocus()) {
        view = ((ViewGroup*) view)->getFocusedChild();
        const int id = view->getId();
        if (id != View::NO_ID) {
            lastKnownId = view->getId();
        }
    }
    return lastKnownId;
}

void RecyclerView::fillRemainingScrollValues(State& state) {
    if (getScrollState() == SCROLL_STATE_SETTLING) {
        OverScroller* scroller = mViewFlinger->mOverScroller;
        state.mRemainingScrollHorizontal = scroller->getFinalX() - scroller->getCurrX();
        state.mRemainingScrollVertical = scroller->getFinalY() - scroller->getCurrY();
    } else {
        state.mRemainingScrollHorizontal = 0;
        state.mRemainingScrollVertical = 0;
    }
}

void RecyclerView::dispatchLayoutStep1() {
    mState->assertLayoutStep(State::STEP_START);
    fillRemainingScrollValues(*mState);
    mState->mIsMeasuring = false;
    startInterceptRequestLayout();
    mViewInfoStore->clear();
    onEnterLayoutOrScroll();
    processAdapterUpdatesAndSetAnimationFlags();
    saveFocusInfo();
    mState->mTrackOldChangeHolders = mState->mRunSimpleAnimations && mItemsChanged;
    mItemsAddedOrRemoved = mItemsChanged = false;
    mState->mInPreLayout = mState->mRunPredictiveAnimations;
    mState->mItemCount = mAdapter->getItemCount();
    findMinMaxChildLayoutPositions(mMinMaxLayoutPositions);

    if (mState->mRunSimpleAnimations) {
        // Step 0: Find out where all non-removed items are, pre-layout
        const int count = mChildHelper->getChildCount();
        for (int i = 0; i < count; ++i) {
            ViewHolder* holder = getChildViewHolderInt(mChildHelper->getChildAt(i));
            if (holder->shouldIgnore() || (holder->isInvalid() && !mAdapter->hasStableIds())) {
                continue;
            }
            ItemAnimator::ItemHolderInfo* animationInfo = mItemAnimator->recordPreLayoutInformation(*mState, *holder,
                   ItemAnimator::buildAdapterChangeFlagsForAnimations(holder),*holder->getUnmodifiedPayloads());
            mViewInfoStore->addToPreLayout(holder, animationInfo);
            if (mState->mTrackOldChangeHolders && holder->isUpdated() && !holder->isRemoved()
                    && !holder->shouldIgnore() && !holder->isInvalid()) {
                long key = getChangedHolderKey(*holder);
                // This is NOT the only place where a ViewHolder is added to old change holders
                // list. There is another case where:
                //    * A VH is currently hidden but not deleted
                //    * The hidden item is changed in the adapter
                //    * Layout manager decides to layout the item in the pre-Layout pass (step1)
                // When this case is detected, RV will un-hide that view and add to the old
                // change holders list.
                mViewInfoStore->addToOldChangeHolders(key, holder);
            }
        }
    }
    if (mState->mRunPredictiveAnimations) {
        // Step 1: run prelayout: This will use the old positions of items. The layout manager
        // is expected to layout everything, even removed items (though not to add removed
        // items back to the container). This gives the pre-layout position of APPEARING views
        // which come into existence as part of the real layout.

        // Save old positions so that LayoutManager can run its mapping logic.
        saveOldPositions();
        const bool didStructureChange = mState->mStructureChanged;
        mState->mStructureChanged = false;
        // temporarily disable flag because we are asking for previous layout
        mLayout->onLayoutChildren(*mRecycler, *mState);
        mState->mStructureChanged = didStructureChange;

        for (int i = 0; i < mChildHelper->getChildCount(); ++i) {
            View* child = mChildHelper->getChildAt(i);
            ViewHolder* viewHolder = getChildViewHolderInt(child);
            if (viewHolder->shouldIgnore()) {
                continue;
            }
            if (!mViewInfoStore->isInPreLayout(viewHolder)) {
                int flags = ItemAnimator::buildAdapterChangeFlagsForAnimations(viewHolder);
                bool wasHidden = viewHolder->hasAnyOfTheFlags(ViewHolder::FLAG_BOUNCED_FROM_HIDDEN_LIST);
                if (!wasHidden) {
                    flags |= ItemAnimator::FLAG_APPEARED_IN_PRE_LAYOUT;
                }
                ItemAnimator::ItemHolderInfo* animationInfo = mItemAnimator->recordPreLayoutInformation(
                        *mState, *viewHolder, flags,*viewHolder->getUnmodifiedPayloads());
                if (wasHidden) {
                    recordAnimationInfoIfBouncedHiddenView(viewHolder, animationInfo);
                } else {
                    mViewInfoStore->addToAppearedInPreLayoutHolders(viewHolder, animationInfo);
                }
            }
        }
        // we don't process disappearing list because they may re-appear in post layout pass.
        clearOldPositions();
    } else {
        clearOldPositions();
    }
    onExitLayoutOrScroll();
    stopInterceptRequestLayout(false);
    mState->mLayoutStep = State::STEP_LAYOUT;
}

void RecyclerView::dispatchLayoutStep2() {
    startInterceptRequestLayout();
    onEnterLayoutOrScroll();
    mState->assertLayoutStep(State::STEP_LAYOUT | State::STEP_ANIMATIONS);
    mAdapterHelper->consumeUpdatesInOnePass();
    mState->mItemCount = mAdapter->getItemCount();
    mState->mDeletedInvisibleItemCountSincePreviousLayout = 0;
    if (mPendingSavedState != nullptr && mAdapter->canRestoreState()) {
        if (mPendingSavedState->mLayoutState != nullptr) {
            mLayout->onRestoreInstanceState(*mPendingSavedState->mLayoutState);
        }
        mPendingSavedState = nullptr;
    }
    // Step 2: Run layout
    mState->mInPreLayout = false;
    mLayout->onLayoutChildren(*mRecycler, *mState);

    mState->mStructureChanged = false;

    // onLayoutChildren may have caused client code to disable item animations; re-check
    mState->mRunSimpleAnimations = mState->mRunSimpleAnimations && (mItemAnimator != nullptr);
    mState->mLayoutStep = State::STEP_ANIMATIONS;
    onExitLayoutOrScroll();
    stopInterceptRequestLayout(false);
}

void RecyclerView::dispatchLayoutStep3() {
    mState->assertLayoutStep(State::STEP_ANIMATIONS);
    startInterceptRequestLayout();
    onEnterLayoutOrScroll();
    mState->mLayoutStep = State::STEP_START;
    if (mState->mRunSimpleAnimations) {
        // Step 3: Find out where things are now, and process change animations.
        // traverse list in reverse because we may call animateChange in the loop which may
        // remove the target view holder.
        for (int i = mChildHelper->getChildCount() - 1; i >= 0; i--) {
            ViewHolder* holder = getChildViewHolderInt(mChildHelper->getChildAt(i));
            if (holder->shouldIgnore()) {
                continue;
            }
            long key = getChangedHolderKey(*holder);
            ItemAnimator::ItemHolderInfo* animationInfo = mItemAnimator->recordPostLayoutInformation(*mState, *holder);
            ViewHolder* oldChangeViewHolder = mViewInfoStore->getFromOldChangeHolders(key);
            if (oldChangeViewHolder && !oldChangeViewHolder->shouldIgnore()) {
                // run a change animation

                // If an Item is CHANGED but the updated version is disappearing, it creates
                // a conflicting case.
                // Since a view that is marked as disappearing is likely to be going out of
                // bounds, we run a change animation. Both views will be cleaned automatically
                // once their animations finish.
                // On the other hand, if it is the same view holder instance, we run a
                // disappearing animation instead because we are not going to rebind the updated
                // VH unless it is enforced by the layout manager.
                const bool oldDisappearing = mViewInfoStore->isDisappearing(oldChangeViewHolder);
                const bool newDisappearing = mViewInfoStore->isDisappearing(holder);
                if (oldDisappearing && (oldChangeViewHolder == holder)) {
                    // run disappear animation instead of change
                    mViewInfoStore->addToPostLayout(holder, animationInfo);
                } else {
                    ItemAnimator::ItemHolderInfo* preInfo = mViewInfoStore->popFromPreLayout(oldChangeViewHolder);
                    // we add and remove so that any post info is merged.
                    mViewInfoStore->addToPostLayout(holder, animationInfo);
                    ItemAnimator::ItemHolderInfo* postInfo = mViewInfoStore->popFromPostLayout(holder);
                    if (preInfo == nullptr) {
                        handleMissingPreInfoForChangeError(key, holder, oldChangeViewHolder);
                    } else {
                        animateChange(*oldChangeViewHolder, *holder, *preInfo, *postInfo,
                                oldDisappearing, newDisappearing);
                    }
                    delete preInfo;
                    delete postInfo;
                }
            } else {
                mViewInfoStore->addToPostLayout(holder, animationInfo);
            }
        }

        // Step 4: Process view info lists and trigger animations
        mViewInfoStore->process(*(ViewInfoStore::ProcessCallback*)mViewInfoProcessCallback);
    }

    mLayout->removeAndRecycleScrapInt(*mRecycler);
    mState->mPreviousLayoutItemCount = mState->mItemCount;
    mDataSetHasChangedAfterLayout = false;
    mDispatchItemsChangedEvent = false;
    mState->mRunSimpleAnimations = false;

    mState->mRunPredictiveAnimations = false;
    mLayout->mRequestedSimpleAnimations = false;
    if (mRecycler->mChangedScrap != nullptr) {
        mRecycler->mChangedScrap->clear();
    }
    if (mLayout->mPrefetchMaxObservedInInitialPrefetch) {
        // Initial prefetch has expanded cache, so reset until next prefetch.
        // This prevents initial prefetches from expanding the cache permanently.
        mLayout->mPrefetchMaxCountObserved = 0;
        mLayout->mPrefetchMaxObservedInInitialPrefetch = false;
        mRecycler->updateViewCacheSize();
    }

    mLayout->onLayoutCompleted(*mState);
    onExitLayoutOrScroll();
    stopInterceptRequestLayout(false);
    mViewInfoStore->clear();
    if (didChildRangeChange(mMinMaxLayoutPositions[0], mMinMaxLayoutPositions[1])) {
        dispatchOnScrolled(0, 0);
    }
    recoverFocusFromState();
    resetFocusInfo();
}

void RecyclerView::handleMissingPreInfoForChangeError(long key,
        ViewHolder* holder, ViewHolder* oldChangeViewHolder) {
    // check if two VH have the same key, if so, print that as an error
    const int childCount = mChildHelper->getChildCount();
    for (int i = 0; i < childCount; i++) {
        View* view = mChildHelper->getChildAt(i);
        ViewHolder* other = getChildViewHolderInt(view);
        if (other == holder) {
            continue;
        }
        long otherKey = getChangedHolderKey(*other);
        if (otherKey == key) {
            if (mAdapter && mAdapter->hasStableIds()) {
                LOGE("Two different ViewHolders have the same stable"
                     " ID. Stable IDs in your adapter MUST BE unique and SHOULD NOT"
                     " change.\n ViewHolder 1:%p ViewHolder 2:%p %s",other,holder);
            } else {
                LOGE("Two different ViewHolders have the same change"
                     " ID. This might happen due to inconsistent Adapter update events or"
                     " if the LayoutManager lays out the same View multiple times."
                     "\n ViewHolder 1:%p ViewHolder2:%p",other,holder);
            }
        }
    }
    // Very unlikely to happen but if it does, notify the developer.
    LOGE("Problem while matching changed view holders with the new"
           "ones. The pre-layout information for the change holder %p "
           " cannot be found but it is necessary for %p",oldChangeViewHolder, holder);
}

void RecyclerView::recordAnimationInfoIfBouncedHiddenView(ViewHolder* viewHolder,
        ItemAnimator::ItemHolderInfo* animationInfo) {
    // looks like this view bounced back from hidden list!
    viewHolder->setFlags(0, ViewHolder::FLAG_BOUNCED_FROM_HIDDEN_LIST);
    if (mState->mTrackOldChangeHolders && viewHolder->isUpdated()
            && !viewHolder->isRemoved() && !viewHolder->shouldIgnore()) {
        long key = getChangedHolderKey(*viewHolder);
        mViewInfoStore->addToOldChangeHolders(key, viewHolder);
    }
    mViewInfoStore->addToPreLayout(viewHolder, animationInfo);
}

void RecyclerView::findMinMaxChildLayoutPositions(int* into) {
    const int count = mChildHelper->getChildCount();
    if (count == 0) {
        into[0] = NO_POSITION;
        into[1] = NO_POSITION;
        return;
    }
    int minPositionPreLayout = INT_MAX;//Integer.MAX_VALUE;
    int maxPositionPreLayout = INT_MIN;//Integer.MIN_VALUE;
    for (int i = 0; i < count; ++i) {
        ViewHolder* holder = getChildViewHolderInt(mChildHelper->getChildAt(i));
        if (holder->shouldIgnore()) {
            continue;
        }
        const int pos = holder->getLayoutPosition();
        if (pos < minPositionPreLayout) {
            minPositionPreLayout = pos;
        }
        if (pos > maxPositionPreLayout) {
            maxPositionPreLayout = pos;
        }
    }
    into[0] = minPositionPreLayout;
    into[1] = maxPositionPreLayout;
}

bool RecyclerView::didChildRangeChange(int minPositionPreLayout, int maxPositionPreLayout) {
    findMinMaxChildLayoutPositions(mMinMaxLayoutPositions);
    return mMinMaxLayoutPositions[0] != minPositionPreLayout
            || mMinMaxLayoutPositions[1] != maxPositionPreLayout;
}

void RecyclerView::removeDetachedView(View* child, bool animate) {
    ViewHolder* vh = getChildViewHolderInt(child);
    if (vh != nullptr) {
        if (vh->isTmpDetached()) {
            vh->clearTmpDetachFlag();
        } else if (!vh->shouldIgnore()) {
            LOGE("Called removeDetachedView with a view which"
                 " is not flagged as tmp detached.%p %s",vh);
        }
    }

    // Clear any android.view.animation.Animation that may prevent the item from
    // detaching when being removed. If a child is re-added before the
    // lazy detach occurs, it will receive invalid attach/detach sequencing.
    child->clearAnimation();

    dispatchChildDetached(child);
    ViewGroup::removeDetachedView(child, animate);
}

long RecyclerView::getChangedHolderKey(ViewHolder& holder) {
    return mAdapter->hasStableIds() ? holder.getItemId() : holder.mPosition;
}

void RecyclerView::animateAppearance(ViewHolder& itemHolder,ItemAnimator::ItemHolderInfo* preLayoutInfo,ItemAnimator::ItemHolderInfo& postLayoutInfo) {
    itemHolder.setIsRecyclable(false);
    if (mItemAnimator->animateAppearance(itemHolder, preLayoutInfo, postLayoutInfo)) {
        postAnimationRunner();
    }
}

void RecyclerView::animateDisappearance(ViewHolder& holder, ItemAnimator::ItemHolderInfo& preLayoutInfo, ItemAnimator::ItemHolderInfo* postLayoutInfo) {
    addAnimatingView(holder);
    holder.setIsRecyclable(false);
    if (mItemAnimator->animateDisappearance(holder, preLayoutInfo, postLayoutInfo)) {
        postAnimationRunner();
    }
}

void RecyclerView::animateChange(ViewHolder& oldHolder,ViewHolder& newHolder,
        ItemAnimator::ItemHolderInfo& preInfo,ItemAnimator::ItemHolderInfo& postInfo,
        bool oldHolderDisappearing, bool newHolderDisappearing) {
    oldHolder.setIsRecyclable(false);
    if (oldHolderDisappearing) {
        addAnimatingView(oldHolder);
    }
    if (&oldHolder != &newHolder) {
        if (newHolderDisappearing) {
            addAnimatingView(newHolder);
        }
        oldHolder.mShadowedHolder = &newHolder;
        // old holder should disappear after animation ends
        addAnimatingView(oldHolder);
        mRecycler->unscrapView(oldHolder);
        newHolder.setIsRecyclable(false);
        newHolder.mShadowingHolder = &oldHolder;
    }
    if (mItemAnimator->animateChange(oldHolder, newHolder, preInfo, postInfo)) {
        postAnimationRunner();
    }
}

void RecyclerView::onLayout(bool changed, int l, int t, int w, int h) {
    dispatchLayout();
    mFirstLayoutComplete = true;
}

void RecyclerView::requestLayout() {
    if ((mInterceptRequestLayoutDepth == 0) && !mLayoutSuppressed) {
        ViewGroup::requestLayout();
    } else {
        mLayoutWasDefered = true;
    }
}

void RecyclerView::markItemDecorInsetsDirty() {
    const int childCount = mChildHelper->getUnfilteredChildCount();
    for (int i = 0; i < childCount; i++) {
        View* child = mChildHelper->getUnfilteredChildAt(i);
        ((LayoutParams*) child->getLayoutParams())->mInsetsDirty = true;
    }
    mRecycler->markItemDecorInsetsDirty();
}

void RecyclerView::draw(Canvas& c) {
    ViewGroup::draw(c);

    const int count = (int)mItemDecorations.size();
    for (int i = 0; i < count; i++) {
        mItemDecorations.at(i)->onDrawOver(c, *this, *mState);
    }
    // TODO If padding is not 0 and clipChildrenToPadding is false, to draw glows properly, we
    // need find children closest to edges. Not sure if it is worth the effort.
    bool needsInvalidate = false;
    if (mLeftGlow  && !mLeftGlow->isFinished()) {
        const int padding = mClipToPadding ? getPaddingBottom() : 0;
        c.save();
        c.rotate_degrees(270);
        c.translate(-getHeight() + padding, 0);
        needsInvalidate = mLeftGlow && mLeftGlow->draw(c);
        c.restore();
    }
    if (mTopGlow && !mTopGlow->isFinished()) {
        c.save();
        if (mClipToPadding) {
            c.translate(getPaddingLeft(), getPaddingTop());
        }
        needsInvalidate |= mTopGlow && mTopGlow->draw(c);
        c.restore();
    }
    if (mRightGlow && !mRightGlow->isFinished()) {
        const int width = getWidth();
        const int padding = mClipToPadding ? getPaddingTop() : 0;
        c.save();
        c.rotate_degrees(90);
        c.translate(-padding, -width);
        needsInvalidate |= mRightGlow && mRightGlow->draw(c);
        c.restore();
    }
    if (mBottomGlow && !mBottomGlow->isFinished()) {
        c.save();
        c.rotate_degrees(180);
        if (mClipToPadding) {
            c.translate(-getWidth() + getPaddingRight(), -getHeight() + getPaddingBottom());
        } else {
            c.translate(-getWidth(), -getHeight());
        }
        needsInvalidate |= mBottomGlow && mBottomGlow->draw(c);
        c.restore();
    }

    if (!needsInvalidate && mItemAnimator && mItemDecorations.size() > 0
            && mItemAnimator->isRunning()) {
        needsInvalidate = true;
    }
    if (needsInvalidate) {
        postInvalidateOnAnimation();
    }
}

void RecyclerView::onDraw(Canvas& c) {
    ViewGroup::onDraw(c);

    const int count = (int)mItemDecorations.size();
    for (int i = 0; i < count; i++) {
        mItemDecorations.at(i)->onDraw(c, *this, *mState);
    }
}

bool RecyclerView::checkLayoutParams(const ViewGroup::LayoutParams* p)const {
    return dynamic_cast<const LayoutParams*>(p) && mLayout->checkLayoutParams((const LayoutParams*) p);
}

RecyclerView::LayoutParams* RecyclerView::generateDefaultLayoutParams()const {
    LOGE_IF(mLayout == nullptr,"RecyclerView has no LayoutManager");
    return mLayout->generateDefaultLayoutParams();
}

RecyclerView::LayoutParams* RecyclerView::generateLayoutParams(const AttributeSet& attrs)const {
    LOGE_IF(mLayout == nullptr,"RecyclerView has no LayoutManager");
    return mLayout->generateLayoutParams(getContext(), attrs);
}

RecyclerView::LayoutParams* RecyclerView::generateLayoutParams(const ViewGroup::LayoutParams* p)const {
    LOGE_IF(mLayout == nullptr,"RecyclerView has no LayoutManager");
    return mLayout->generateLayoutParams(*p);
}

bool RecyclerView::isAnimating() {
    return mItemAnimator && mItemAnimator->isRunning();
}

void RecyclerView::saveOldPositions() {
    const int childCount = mChildHelper->getUnfilteredChildCount();
    for (int i = 0; i < childCount; i++) {
        ViewHolder* holder = getChildViewHolderInt(mChildHelper->getUnfilteredChildAt(i));
        if (sDebugAssertionsEnabled && holder->mPosition == -1 && !holder->isRemoved()) {
            LOGE("view holder cannot have position -1 unless it is removed %p");
        }
        if (!holder->shouldIgnore()) {
            holder->saveOldPosition();
        }
    }
}

void RecyclerView::clearOldPositions() {
    const int childCount = mChildHelper->getUnfilteredChildCount();
    for (int i = 0; i < childCount; i++) {
        ViewHolder* holder = getChildViewHolderInt(mChildHelper->getUnfilteredChildAt(i));
        if (!holder->shouldIgnore()) {
            holder->clearOldPosition();
        }
    }
    mRecycler->clearOldPositions();
}

void RecyclerView::offsetPositionRecordsForMove(int from, int to) {
    const int childCount = mChildHelper->getUnfilteredChildCount();
    int start, end, inBetweenOffset;
    if (from < to) {
        start = from;
        end = to;
        inBetweenOffset = -1;
    } else {
        start = to;
        end = from;
        inBetweenOffset = 1;
    }

    for (int i = 0; i < childCount; i++) {
        ViewHolder* holder = getChildViewHolderInt(mChildHelper->getUnfilteredChildAt(i));
        if ( (holder == nullptr) || (holder->mPosition < start) || (holder->mPosition > end) ) {
            continue;
        }
        LOGD_IF(sDebugAssertionsEnabled,"offsetPositionRecordsForMove attached child %d holder %p",i, holder);
        if (holder->mPosition == from) {
            holder->offsetPosition(to - from, false);
        } else {
            holder->offsetPosition(inBetweenOffset, false);
        }

        mState->mStructureChanged = true;
    }
    mRecycler->offsetPositionRecordsForMove(from, to);
    requestLayout();
}

void RecyclerView::offsetPositionRecordsForInsert(int positionStart, int itemCount) {
    const int childCount = mChildHelper->getUnfilteredChildCount();
    for (int i = 0; i < childCount; i++) {
        ViewHolder* holder = getChildViewHolderInt(mChildHelper->getUnfilteredChildAt(i));
        if ( (holder != nullptr) && !holder->shouldIgnore() && (holder->mPosition >= positionStart) ) {
            LOGD_IF(sDebugAssertionsEnabled,"offsetPositionRecordsForInsert attached child %d holder %p  now at position %d"
                       ,i,holder, (holder->mPosition + itemCount));
            holder->offsetPosition(itemCount, false);
            mState->mStructureChanged = true;
        }
    }
    mRecycler->offsetPositionRecordsForInsert(positionStart, itemCount);
    requestLayout();
}

void RecyclerView::offsetPositionRecordsForRemove(int positionStart, int itemCount,bool applyToPreLayout) {
    const int positionEnd = positionStart + itemCount;
    const int childCount = mChildHelper->getUnfilteredChildCount();
    for (int i = 0; i < childCount; i++) {
        ViewHolder* holder = getChildViewHolderInt(mChildHelper->getUnfilteredChildAt(i));
        if (holder && !holder->shouldIgnore()) {
            if (holder->mPosition >= positionEnd) {
                LOGD_IF(sDebugAssertionsEnabled,"offsetPositionRecordsForRemove attached child %d(%p) hold %p now at position %d",
                    i,holder->itemView ,holder, (holder->mPosition - itemCount));
                holder->offsetPosition(-itemCount, applyToPreLayout);
                mState->mStructureChanged = true;
            } else if (holder->mPosition >= positionStart) {
                LOGD_IF(sDebugAssertionsEnabled,"offsetPositionRecordsForRemove attached child %d(%p) holder %p now REMOVED",i,holder->itemView,holder);
                holder->flagRemovedAndOffsetPosition(positionStart - 1, -itemCount,applyToPreLayout);
                mState->mStructureChanged = true;
            }
        }
    }
    mRecycler->offsetPositionRecordsForRemove(positionStart, itemCount, applyToPreLayout);
    requestLayout();
}

void RecyclerView::viewRangeUpdate(int positionStart, int itemCount, Object* payload) {
    const int childCount = mChildHelper->getUnfilteredChildCount();
    const int positionEnd = positionStart + itemCount;

    for (int i = 0; i < childCount; i++) {
        View* child = mChildHelper->getUnfilteredChildAt(i);
        ViewHolder* holder = getChildViewHolderInt(child);
        if ((holder == nullptr) || holder->shouldIgnore()) {
            continue;
        }
        if ( (holder->mPosition >= positionStart) && (holder->mPosition < positionEnd) ) {
            // We re-bind these view holders after pre-processing is complete so that
            // ViewHolders have their final positions assigned.
            holder->addFlags(ViewHolder::FLAG_UPDATE);
            holder->addChangePayload(payload);
            // lp cannot be null since we get ViewHolder from it.
            ((LayoutParams*) child->getLayoutParams())->mInsetsDirty = true;
        }
    }
    mRecycler->viewRangeUpdate(positionStart, itemCount);
}

bool RecyclerView::canReuseUpdatedViewHolder(ViewHolder& viewHolder) {
    return (mItemAnimator == nullptr) || mItemAnimator->canReuseUpdatedViewHolder(viewHolder,
            *viewHolder.getUnmodifiedPayloads());
}

void RecyclerView::processDataSetCompletelyChanged(bool dispatchItemsChanged) {
    mDispatchItemsChangedEvent |= dispatchItemsChanged;
    mDataSetHasChangedAfterLayout = true;
    markKnownViewsInvalid();
}

void RecyclerView::markKnownViewsInvalid() {
    const int childCount = mChildHelper->getUnfilteredChildCount();
    for (int i = 0; i < childCount; i++) {
        ViewHolder* holder = getChildViewHolderInt(mChildHelper->getUnfilteredChildAt(i));
        if (holder && !holder->shouldIgnore()) {
            holder->addFlags(ViewHolder::FLAG_UPDATE | ViewHolder::FLAG_INVALID);
        }
    }
    markItemDecorInsetsDirty();
    mRecycler->markKnownViewsInvalid();
}

void RecyclerView::invalidateItemDecorations() {
    if (mItemDecorations.size() == 0) {
        return;
    }
    if (mLayout != nullptr) {
        mLayout->assertNotInLayoutOrScroll("Cannot invalidate item decorations during a scroll or layout");
    }
    markItemDecorInsetsDirty();
    requestLayout();
}

bool RecyclerView::getPreserveFocusAfterLayout() const{
    return mPreserveFocusAfterLayout;
}

void RecyclerView::setPreserveFocusAfterLayout(bool preserveFocusAfterLayout) {
    mPreserveFocusAfterLayout = preserveFocusAfterLayout;
}

RecyclerView::ViewHolder* RecyclerView::getChildViewHolder(View* child) {
    ViewGroup* parent = child->getParent();
    if ((parent != nullptr) && (parent != this)) {
        LOGE("View %p is not a direct child of %p",child,this);
    }
    return getChildViewHolderInt(child);
}

View* RecyclerView::findContainingItemView(View* view) {
    ViewGroup* parent = view->getParent();
    while ((parent != nullptr) && (parent != this)/* && parent instanceof View*/) {
        view = (View*) parent;
        parent = view->getParent();
    }
    return parent == this ? view : nullptr;
}

RecyclerView::ViewHolder* RecyclerView::findContainingViewHolder(View* view) {
    View* itemView = findContainingItemView(view);
    return (itemView == nullptr) ? nullptr : getChildViewHolder(itemView);
}


RecyclerView::ViewHolder* RecyclerView::getChildViewHolderInt(View* child) {
    if (child == nullptr) {
        return nullptr;
    }
    return ((LayoutParams*) child->getLayoutParams())->mViewHolder;
}

int RecyclerView::getChildPosition(View* child) {
    return getChildAdapterPosition(child);
}

int RecyclerView::getChildAdapterPosition(View* child) {
    ViewHolder* holder = getChildViewHolderInt(child);
    return holder ? holder->getAbsoluteAdapterPosition() : NO_POSITION;
}

int RecyclerView::getChildLayoutPosition(View* child) {
    ViewHolder* holder = getChildViewHolderInt(child);
    return holder ? holder->getLayoutPosition() : NO_POSITION;
}

long RecyclerView::getChildItemId(View* child) {
    if (mAdapter == nullptr || !mAdapter->hasStableIds()) {
        return NO_ID;
    }
    ViewHolder* holder = getChildViewHolderInt(child);
    return holder ? holder->getItemId() : NO_ID;
}

RecyclerView::ViewHolder* RecyclerView::findViewHolderForPosition(int position) {
    return findViewHolderForPosition(position, false);
}

RecyclerView::ViewHolder* RecyclerView::findViewHolderForLayoutPosition(int position) {
    return findViewHolderForPosition(position, false);
}

RecyclerView::ViewHolder* RecyclerView::findViewHolderForAdapterPosition(int position) {
    if (mDataSetHasChangedAfterLayout) {
        return nullptr;
    }
    const int childCount = mChildHelper->getUnfilteredChildCount();
    // hidden VHs are not preferred but if that is the only one we find, we rather return it
    ViewHolder* hidden = nullptr;
    for (int i = 0; i < childCount; i++) {
        ViewHolder* holder = getChildViewHolderInt(mChildHelper->getUnfilteredChildAt(i));
        if (holder && !holder->isRemoved()
                && getAdapterPositionInRecyclerView(holder) == position) {
            if (mChildHelper->isHidden(holder->itemView)) {
                hidden = holder;
            } else {
                return holder;
            }
        }
    }
    return hidden;
}

RecyclerView::ViewHolder* RecyclerView::findViewHolderForPosition(int position, bool checkNewPosition) {
    const int childCount = mChildHelper->getUnfilteredChildCount();
    ViewHolder* hidden = nullptr;
    for (int i = 0; i < childCount; i++) {
        ViewHolder* holder = getChildViewHolderInt(mChildHelper->getUnfilteredChildAt(i));
        if (holder && !holder->isRemoved()) {
            if (checkNewPosition) {
                if (holder->mPosition != position) {
                    continue;
                }
            } else if (holder->getLayoutPosition() != position) {
                continue;
            }
            if (mChildHelper->isHidden(holder->itemView)) {
                hidden = holder;
            } else {
                return holder;
            }
        }
    }
    // This method should not query cached views. It creates a problem during adapter updates
    // when we are dealing with already laid out views. Also, for the public method, it is more
    // reasonable to return null if position is not laid out.
    return hidden;
}

RecyclerView::ViewHolder* RecyclerView::findViewHolderForItemId(long id) {
    if ( (mAdapter == nullptr) || !mAdapter->hasStableIds()) {
        return nullptr;
    }
    const int childCount = mChildHelper->getUnfilteredChildCount();
    ViewHolder* hidden = nullptr;
    for (int i = 0; i < childCount; i++) {
        ViewHolder* holder = getChildViewHolderInt(mChildHelper->getUnfilteredChildAt(i));
        if (holder && !holder->isRemoved() && (holder->getItemId() == id) ) {
            if (mChildHelper->isHidden(holder->itemView)) {
                hidden = holder;
            } else {
                return holder;
            }
        }
    }
    return hidden;
}

View* RecyclerView::findChildViewUnder(float x, float y) {
    const int count = mChildHelper->getChildCount();
    for (int i = count - 1; i >= 0; i--) {
        View* child = mChildHelper->getChildAt(i);
        const float translationX = child->getTranslationX();
        const float translationY = child->getTranslationY();
        if (x >= child->getLeft() + translationX
                && x <= child->getRight() + translationX
                && y >= child->getTop() + translationY
                && y <= child->getBottom() + translationY) {
            return child;
        }
    }
    return nullptr;
}

bool RecyclerView::drawChild(Canvas& canvas, View* child, int64_t drawingTime) {
    return ViewGroup::drawChild(canvas, child, drawingTime);
}

void RecyclerView::offsetChildrenVertical(int dy) {
    const int childCount = mChildHelper->getChildCount();
    for (int i = 0; i < childCount; i++) {
        mChildHelper->getChildAt(i)->offsetTopAndBottom(dy);
    }
}

void RecyclerView::onChildAttachedToWindow(View* child) {
}

void RecyclerView::onChildDetachedFromWindow(View* child) {
}

void RecyclerView::offsetChildrenHorizontal(int dx) {
    const int childCount = mChildHelper->getChildCount();
    for (int i = 0; i < childCount; i++) {
        mChildHelper->getChildAt(i)->offsetLeftAndRight(dx);
    }
}

void RecyclerView::getDecoratedBoundsWithMargins(View*view,Rect& outBounds) const{
    getDecoratedBoundsWithMarginsInt(view, outBounds);
}

void RecyclerView::getDecoratedBoundsWithMarginsInt(View* view, Rect& outBounds){
    const LayoutParams* lp = (LayoutParams*) view->getLayoutParams();
    const Rect insets = lp->mDecorInsets;
    outBounds.set(view->getLeft() - insets.left - lp->leftMargin,
         view->getTop() - insets.top - lp->topMargin,
         view->getWidth() + insets.left+ insets.width + lp->leftMargin + lp->rightMargin,
         view->getHeight() + insets.top + insets.height + lp->topMargin + lp->bottomMargin);
}

Rect RecyclerView::getItemDecorInsetsForChild(View* child) {
    LayoutParams* lp = (LayoutParams*) child->getLayoutParams();
    if (!lp->mInsetsDirty) {
        return lp->mDecorInsets;
    }

    if (mState->isPreLayout() && (lp->isItemChanged() || lp->isViewInvalid())) {
        // changed/invalid items should not be updated until they are rebound.
        return lp->mDecorInsets;
    }
    Rect& insets = lp->mDecorInsets;
    insets.set(0, 0, 0, 0);
    const int decorCount = (int)mItemDecorations.size();
    for (int i = 0; i < decorCount; i++) {
        mTempRect.set(0, 0, 0, 0);
        mItemDecorations.at(i)->getItemOffsets(mTempRect, *child, *this, *mState);
        insets.left += mTempRect.left;
        insets.top += mTempRect.top;
        insets.width += mTempRect.width;//right;
        insets.height += mTempRect.height;//bottom;
    }
    lp->mInsetsDirty = false;
    return insets;
}

void RecyclerView::onScrolled(int dx,int dy) {
    // Do nothing
}

void RecyclerView::dispatchOnScrolled(int hresult, int vresult) {
    mDispatchScrollCounter++;
    // Pass the current scrollX/scrollY values; no actual change in these properties occurred
    // but some general-purpose code may choose to respond to changes this way.
    const int scrollX = getScrollX();
    const int scrollY = getScrollY();
    onScrollChanged(scrollX, scrollY, scrollX - hresult, scrollY-vresult);

    // Pass the real deltas to onScrolled, the RecyclerView-specific method.
    onScrolled(hresult, vresult);

    // Invoke listeners last. Subclassed view methods always handle the event first.
    // All internal state is consistent by the time listeners are invoked.
    if (mScrollListener.onScrolled != nullptr) {
        mScrollListener.onScrolled(*this, hresult, vresult);
    }
    for (int i = mScrollListeners.size() - 1; i >= 0; i--) {
	auto func = mScrollListeners.at(i).onScrolled;
        if(func)func(*this, hresult, vresult);
    }
    mDispatchScrollCounter--;
}

void RecyclerView::onScrollStateChanged(int state) {
    // Do nothing
}

/**
* Copied from OverScroller, this returns the distance that a fling with the given velocity
* will go.
* @param velocity The velocity of the fling
* @return The distance that will be traveled by a fling of the given velocity.
*/
float RecyclerView::getSplineFlingDistance(int velocity) {
    const double l =std::log(INFLEXION * std::abs(velocity) / (SCROLL_FRICTION * mPhysicalCoef));
    const double decelMinusOne = DECELERATION_RATE - 1.0;
    return (float) (SCROLL_FRICTION * mPhysicalCoef * std::exp(DECELERATION_RATE / decelMinusOne * l));
}
void RecyclerView::dispatchOnScrollStateChanged(int state) {
    // Let the LayoutManager go first; this allows it to bring any properties into
    // a consistent state before the RecyclerView subclass responds.
    if (mLayout != nullptr) {
        mLayout->onScrollStateChanged(state);
    }

    // Let the RecyclerView subclass handle this event next; any LayoutManager property
    // changes will be reflected by this time.
    onScrollStateChanged(state);

    // Listeners go last. All other internal state is consistent by this point.
    if (mScrollListener.onScrollStateChanged != nullptr) {
        mScrollListener.onScrollStateChanged(*this, state);
    }
    for (int i = int(mScrollListeners.size() - 1); i >= 0; i--) {
	auto func = mScrollListeners.at(i).onScrollStateChanged;
        if(func)func(*this, state);
    }
}

bool RecyclerView::hasPendingAdapterUpdates() {
    return !mFirstLayoutComplete || mDataSetHasChangedAfterLayout
            || mAdapterHelper->hasPendingUpdates();
}

////////////////////    class ViewFlinger implements Runnabl ////////////////////////////

RecyclerView::ViewFlinger::ViewFlinger(RecyclerView*rv) {
    mEatRunOnAnimationRequest = false;
    mReSchedulePostAnimationCallback = false;
    mRV = rv;
    mInterpolator = nullptr;
    mRunnable = std::bind(&ViewFlinger::run,this);
    mOverScroller = new OverScroller(mRV->getContext(), sQuinticInterpolator.get());
}

RecyclerView::ViewFlinger::~ViewFlinger(){
    delete mOverScroller;
}

void RecyclerView::ViewFlinger::run() {
    if (mRV->mLayout == nullptr) {
        stop();
        return; // no layout, cannot scroll.
    }
    mReSchedulePostAnimationCallback = false;
    mEatRunOnAnimationRequest = true;
    mRV->consumePendingUpdateOperations();

    // TODO(72745539): After reviewing the code, it seems to me we may actually want to
    // update the reference to the OverScroller after onAnimation.  It looks to me like
    // it is possible that a new OverScroller could be created (due to a new Interpolator
    // being used), when the current OverScroller knows it's done after
    // scroller.computeScrollOffset() is called.  If that happens, and we don't update the
    // reference, it seems to me that we could prematurely stop the newly created scroller
    // due to setScrollState(SCROLL_STATE_IDLE) being called below.

    // Keep a local reference so that if it is changed during onAnimation method, it won't
    // cause unexpected behaviors
    OverScroller* scroller = mOverScroller;
    //SmoothScroller* smoothScroller = mRV->mLayout->mSmoothScroller;
    if (scroller->computeScrollOffset()) {
        const int x = scroller->getCurrX();
        const int y = scroller->getCurrY();
        int unconsumedX = x - mLastFlingX;
        int unconsumedY = y - mLastFlingY;

        mLastFlingX = x;
        mLastFlingY = y;

        unconsumedX = mRV->consumeFlingInHorizontalStretch(unconsumedX);
        unconsumedY = mRV->consumeFlingInVerticalStretch(unconsumedY);

        int consumedX = 0;
        int consumedY = 0;

        // Nested Pre Scroll
        mRV->mReusableIntPair[0] = 0;
        mRV->mReusableIntPair[1] = 0;
        if (mRV->dispatchNestedPreScroll(unconsumedX, unconsumedY, mRV->mReusableIntPair, nullptr, View::TYPE_NON_TOUCH)) {
            unconsumedX -= mRV->mReusableIntPair[0];
            unconsumedY -= mRV->mReusableIntPair[1];
        }

        // Based on movement, we may want to trigger the hiding of existing over scroll
        // glows.
        if (mRV->getOverScrollMode() != View::OVER_SCROLL_NEVER) {
            mRV->considerReleasingGlowsOnScroll(unconsumedX, unconsumedY);
        }

        // Local Scroll;
        if (mRV->mAdapter != nullptr) {
            mRV->mReusableIntPair[0] = 0;
            mRV->mReusableIntPair[1] = 0;
            mRV->scrollStep(unconsumedX,unconsumedY, mRV->mReusableIntPair);
            consumedX = mRV->mReusableIntPair[0];
            consumedY = mRV->mReusableIntPair[1];
            unconsumedX -= consumedX;
            unconsumedY -= consumedY;

            // If SmoothScroller exists, this ViewFlinger was started by it, so we must
            // report back to SmoothScroller.
            SmoothScroller* smoothScroller = mRV->mLayout->mSmoothScroller;
            if (smoothScroller && !smoothScroller->isPendingInitialRun()
                   && smoothScroller->isRunning()) {
                const int adapterSize = mRV->mState->getItemCount();
                if (adapterSize == 0) {
                    smoothScroller->stop();
                } else if (smoothScroller->getTargetPosition() >= adapterSize) {
                    smoothScroller->setTargetPosition(adapterSize - 1);
                    smoothScroller->onAnimation(consumedX,consumedY);
                } else {
                    smoothScroller->onAnimation(consumedX,consumedY);
                }
            }
        }

        if (!mRV->mItemDecorations.empty()) {
            mRV->invalidate();
        }

        // Nested Post Scroll
        mRV->mReusableIntPair[0] = 0;
        mRV->mReusableIntPair[1] = 0;
        mRV->dispatchNestedScroll(consumedX, consumedY, unconsumedX, unconsumedY, nullptr,
                TYPE_NON_TOUCH, mRV->mReusableIntPair);
        unconsumedX -= mRV->mReusableIntPair[0];
        unconsumedY -= mRV->mReusableIntPair[1];

        if (consumedX != 0 || consumedY != 0) {
            mRV->dispatchOnScrolled(consumedX, consumedY);
        }

        if (!mRV->awakenScrollBars()) {
            mRV->invalidate();
        }

        // We are done scrolling if scroller is finished, or for both the x and y dimension,
        // we are done scrolling or we can't scroll further (we know we can't scroll further
        // when we have unconsumed scroll distance).  It's possible that we don't need
        // to also check for scroller.isFinished() at all, but no harm in doing so in case
        // of old bugs in Overscroller.

        bool scrollerFinishedX = scroller->getCurrX() == scroller->getFinalX();
        bool scrollerFinishedY = scroller->getCurrY() == scroller->getFinalY();
        const bool doneScrolling = scroller->isFinished()
                        || ((scrollerFinishedX || unconsumedX != 0)
                        && (scrollerFinishedY || unconsumedY != 0));

        // Get the current smoothScroller. It may have changed by this point and we need to
        // make sure we don't stop scrolling if it has changed and it's pending an initial
        // run.
        SmoothScroller* smoothScroller = mRV->mLayout->mSmoothScroller;
        const bool smoothScrollerPending = smoothScroller != nullptr && smoothScroller->isPendingInitialRun();

        if(!smoothScrollerPending && doneScrolling){
            // If we are done scrolling and the layout's SmoothScroller is not pending,
            // do the things we do at the end of a scroll and don't postOnAnimation.

            if (mRV->getOverScrollMode() != View::OVER_SCROLL_NEVER) {
                const int vel = (int) scroller->getCurrVelocity();
                const int velX = unconsumedX < 0 ? -vel : unconsumedX > 0 ? vel : 0;
                const int velY = unconsumedY < 0 ? -vel : unconsumedY > 0 ? vel : 0;
                mRV->absorbGlows(velX,velY);//considerReleasingGlowsOnScroll(dx, dy);
            }
            if (ALLOW_THREAD_GAP_WORK) {
                 ((GapWorker::LayoutPrefetchRegistryImpl*)mRV->mPrefetchRegistry)->clearPrefetchPositions();
            }
        }else{
            // Otherwise continue the scroll.

            postOnAnimation();
            if (mRV->mGapWorker != nullptr) {
                mRV->mGapWorker->postFromTraversal(mRV, consumedX, consumedY);
            }
        }
        /*if (Build::VERSION::SDK_INT >= 35) {
            Api35Impl.setFrameContentVelocity(RecyclerView.this,
                    Math.abs(scroller.getCurrVelocity()));
        }*/
    }

    RecyclerView::SmoothScroller* smoothScroller = mRV->mLayout->mSmoothScroller;
    // call this after the onAnimation is complete not to have inconsistent callbacks etc.
    if (smoothScroller != nullptr && smoothScroller->isPendingInitialRun()) {
        smoothScroller->onAnimation(0, 0);
    }

    mEatRunOnAnimationRequest = false;
    if (mReSchedulePostAnimationCallback) {
        internalPostOnAnimation();
    } else {
        mRV->setScrollState(SCROLL_STATE_IDLE);
        mRV->stopNestedScroll(TYPE_NON_TOUCH);
    }

    //mRV->enableRunOnAnimationRequests();
}

void RecyclerView::ViewFlinger::disableRunOnAnimationRequests() {
    mReSchedulePostAnimationCallback = false;
    mEatRunOnAnimationRequest = true;
}

void RecyclerView::ViewFlinger::postOnAnimation() {
    if (mEatRunOnAnimationRequest) {
        mReSchedulePostAnimationCallback = true;
    } else {
        mRV->removeCallbacks(mRunnable);//this);
        mRV->postOnAnimation(mRunnable);//this);
    }
}

void RecyclerView::ViewFlinger::internalPostOnAnimation() {
    mRV->removeCallbacks(mRunnable);
    mRV->postOnAnimation(mRunnable);
}

void RecyclerView::ViewFlinger::fling(int velocityX, int velocityY) {
    mRV->setScrollState(SCROLL_STATE_SETTLING);
    mLastFlingX = mLastFlingY = 0;
    // Because you can't define a custom interpolator for flinging, we should make sure we
    // reset ourselves back to the teh default interpolator in case a different call
    // changed our interpolator.
    if (mInterpolator != sQuinticInterpolator.get()) {
        mInterpolator = sQuinticInterpolator.get();
        mOverScroller = new OverScroller(mRV->getContext(), sQuinticInterpolator.get());
    }
    mOverScroller->fling(0, 0, velocityX, velocityY, INT_MIN, INT_MAX, INT_MIN, INT_MAX);
    postOnAnimation();
}

void RecyclerView::ViewFlinger::smoothScrollBy(int dx, int dy, int duration, Interpolator* interpolator) {
    if(duration ==UNDEFINED_DURATION){
        duration = computeScrollDuration(dx,dy);
    }
    if (mInterpolator != interpolator) {
        mInterpolator = interpolator;
        mOverScroller->setInterpolator(mInterpolator);
    }

    // Reset the last fling information.
    mLastFlingX = mLastFlingY = 0;

    // Set to settling state and start scrolling.
    mRV->setScrollState(SCROLL_STATE_SETTLING);
    mOverScroller->startScroll(0, 0, dx, dy, duration);
    if (Build::VERSION::SDK_INT < 23) {//Android 6
        // b/64931938 before API 23, startScroll() does not reset getCurX()/getCurY()
        // to start values, which causes fillRemainingScrollValues() put in obsolete values
        // for LayoutManager.onLayoutChildren().
        mOverScroller->computeScrollOffset();
    }
    postOnAnimation();
}

int RecyclerView::ViewFlinger::computeScrollDuration(int dx, int dy) {
    const int absDx = std::abs(dx);
    const int absDy = std::abs(dy);
    const bool horizontal = absDx > absDy;
    const int containerSize = horizontal ? mRV->getWidth() : mRV->getHeight();

    const float absDelta = (float) (horizontal ? absDx : absDy);
    const int duration = (int) (((absDelta / containerSize) + 1) * 300);

    return std::min(duration, (int)MAX_SCROLL_DURATION);
}

void RecyclerView::ViewFlinger::stop() {
    mRV->removeCallbacks(mRunnable);//this);
    mOverScroller->abortAnimation();
}

//////////////////////*endof ViewFlinger*///////////////////////

void RecyclerView::repositionShadowingViews() {
    // Fix up shadow views used by change animations
    const int count = mChildHelper->getChildCount();
    for (int i = 0; i < count; i++) {
        View* view = mChildHelper->getChildAt(i);
        ViewHolder* holder = getChildViewHolder(view);
        if (holder && holder->mShadowingHolder) {
            View* shadowingView = holder->mShadowingHolder->itemView;
            const int left = view->getLeft();
            const int top = view->getTop();
            if (left != shadowingView->getLeft() ||  top != shadowingView->getTop()) {
                shadowingView->layout(left, top,shadowingView->getWidth(),shadowingView->getHeight());
            }
        }
    }
}

RecyclerView::RecyclerViewDataObserver::RecyclerViewDataObserver(RecyclerView*rv) {
    mRV = rv;
}

void RecyclerView::RecyclerViewDataObserver::onChanged() {
    mRV->assertNotInLayoutOrScroll(std::string());
    mRV->mState->mStructureChanged = true;

    mRV->processDataSetCompletelyChanged(true);
    if (!mRV->mAdapterHelper->hasPendingUpdates()) {
        mRV->requestLayout();
    }
}

void RecyclerView::RecyclerViewDataObserver::onItemRangeChanged(int positionStart, int itemCount, Object* payload) {
    mRV->assertNotInLayoutOrScroll(std::string());
    if (mRV->mAdapterHelper->onItemRangeChanged(positionStart, itemCount, payload)) {
        triggerUpdateProcessor();
    }
}

void RecyclerView::RecyclerViewDataObserver::onItemRangeInserted(int positionStart, int itemCount) {
    mRV->assertNotInLayoutOrScroll(std::string());
    if (mRV->mAdapterHelper->onItemRangeInserted(positionStart, itemCount)) {
        triggerUpdateProcessor();
    }
}

void RecyclerView::RecyclerViewDataObserver::onItemRangeRemoved(int positionStart, int itemCount) {
    mRV->assertNotInLayoutOrScroll(std::string());
    if (mRV->mAdapterHelper->onItemRangeRemoved(positionStart, itemCount)) {
        triggerUpdateProcessor();
    }
}

void RecyclerView::RecyclerViewDataObserver::onItemRangeMoved(int fromPosition, int toPosition, int itemCount) {
    mRV->assertNotInLayoutOrScroll(std::string());
    if (mRV->mAdapterHelper->onItemRangeMoved(fromPosition, toPosition, itemCount)) {
        triggerUpdateProcessor();
    }
}

void RecyclerView::RecyclerViewDataObserver::triggerUpdateProcessor() {
    if (POST_UPDATES_ON_ANIMATION && mRV->mHasFixedSize && mRV->mIsAttached) {
        mRV->postOnAnimation(mRV->mUpdateChildViewsRunnable);
    } else {
        mRV->mAdapterUpdateDuringMeasure = true;
        mRV->requestLayout();
    }
}

void RecyclerView::RecyclerViewDataObserver::onStateRestorationPolicyChanged() {
    if (mRV->mPendingSavedState == nullptr) {
        return;
    }
    // If there is a pending saved state and the new mode requires us to restore it,
    // we'll request a layout which will call the adapter to see if it can restore state
    // and trigger state restoration
    RecyclerView::Adapter*adapter = mRV->mAdapter;
    if (adapter != nullptr && adapter->canRestoreState()) {
        mRV->requestLayout();
    }
}

EdgeEffect* RecyclerView::EdgeEffectFactory::createEdgeEffect(RecyclerView& view,int direction) {
    return new EdgeEffect(view.getContext());
}

/////////////RecycledViewPool///////////////////

RecyclerView::RecycledViewPool::RecycledViewPool(){
}

RecyclerView::RecycledViewPool::~RecycledViewPool(){
    clear();
    for (int i = 0; i < mScrap.size(); i++) {
        ScrapData* data = mScrap.valueAt(i);
        delete data;
    }
}

void RecyclerView::RecycledViewPool::clear() {
    for (int i = 0; i < mScrap.size(); i++) {
        ScrapData* data = mScrap.valueAt(i);
        std::vector<ViewHolder*>& vhs = data->mScrapHeap;
        for(int j=0;j<vhs.size();j++)delete vhs.at(j);
        vhs.clear();
    }
}

void RecyclerView::RecycledViewPool::setMaxRecycledViews(int viewType, int max) {
    ScrapData* scrapData = getScrapDataForType(viewType);
    scrapData->mMaxScrap = max;
    std::vector<ViewHolder*>& scrapHeap = scrapData->mScrapHeap;
    while (scrapHeap.size() > max) {
        ViewHolder*holder =  scrapHeap.back();
        scrapHeap.pop_back();
        delete holder;
    }
}

int RecyclerView::RecycledViewPool::getRecycledViewCount(int viewType) {
    return getScrapDataForType(viewType)->mScrapHeap.size();
}

RecyclerView::ViewHolder* RecyclerView::RecycledViewPool::getRecycledView(int viewType) {
    ScrapData* scrapData = mScrap.get(viewType);
    if ( (scrapData != nullptr) && !scrapData->mScrapHeap.empty()) {
        std::vector<ViewHolder*>& scrapHeap = scrapData->mScrapHeap;
#if 1
        for (int i = scrapHeap.size() - 1; i >= 0; i--) {
            if (!scrapHeap.at(i)->isAttachedToTransitionOverlay()) {
                ViewHolder*ret =scrapHeap.at(i);
                scrapHeap.erase(scrapHeap.begin()+i);//remove(i);
                return ret;
            }
        }
#else
        ViewHolder*ret = scrapHeap.back();
        scrapHeap.pop_back();
        return ret;
#endif
    }
    return nullptr;
}

int RecyclerView::RecycledViewPool::size() {
    int count = 0;
    for (int i = 0; i < mScrap.size(); i++) {
        std::vector<ViewHolder*>& viewHolders = mScrap.valueAt(i)->mScrapHeap;
        count += viewHolders.size();
    }
    return count;
}

void RecyclerView::RecycledViewPool::putRecycledView(ViewHolder* scrap) {
    const int viewType = scrap->getItemViewType();
    std::vector<ViewHolder*>& scrapHeap = getScrapDataForType(viewType)->mScrapHeap;
    if (mScrap.get(viewType)->mMaxScrap <= scrapHeap.size()) {
        delete scrap;//chenyang:)
        return;
    }
    if (sDebugAssertionsEnabled){
        auto it =std::find(scrapHeap.begin(),scrapHeap.end(),scrap);
        LOGD_IF(it!=scrapHeap.end(),"this scrap item already exists");
    }
    scrap->resetInternal();
    scrapHeap.push_back(scrap);
}

long RecyclerView::RecycledViewPool::runningAverage(long oldAverage, long newValue) {
    if (oldAverage == 0) {
        return newValue;
    }
    return (oldAverage / 4 * 3) + (newValue / 4);
}

void RecyclerView::RecycledViewPool::factorInCreateTime(int viewType, long createTimeNs) {
    ScrapData* scrapData = getScrapDataForType(viewType);
    scrapData->mCreateRunningAverageNs = runningAverage(
        scrapData->mCreateRunningAverageNs, createTimeNs);
}

void RecyclerView::RecycledViewPool::factorInBindTime(int viewType, long bindTimeNs) {
    ScrapData* scrapData = getScrapDataForType(viewType);
    scrapData->mBindRunningAverageNs = runningAverage(
        scrapData->mBindRunningAverageNs, bindTimeNs);
}

bool RecyclerView::RecycledViewPool::willCreateInTime(int viewType, long approxCurrentNs, long deadlineNs) {
    const long expectedDurationNs = getScrapDataForType(viewType)->mCreateRunningAverageNs;
    return (expectedDurationNs == 0) || (approxCurrentNs + expectedDurationNs < deadlineNs);
}

bool RecyclerView::RecycledViewPool::willBindInTime(int viewType, long approxCurrentNs, long deadlineNs) {
    const long expectedDurationNs = getScrapDataForType(viewType)->mBindRunningAverageNs;
    return (expectedDurationNs == 0) || (approxCurrentNs + expectedDurationNs < deadlineNs);
}

void RecyclerView::RecycledViewPool::attach() {
    mAttachCountForClearing++;
}

void RecyclerView::RecycledViewPool::detach() {
    mAttachCountForClearing--;
}

void RecyclerView::RecycledViewPool::attachForPoolingContainer(Adapter*adapter) {
    mAttachedAdaptersForPoolingContainer.insert(adapter);
}

void RecyclerView::RecycledViewPool::detachForPoolingContainer(Adapter*adapter, bool isBeingReplaced) {
    mAttachedAdaptersForPoolingContainer.erase(adapter);
    if (mAttachedAdaptersForPoolingContainer.size() == 0 && !isBeingReplaced) {
        for (int keyIndex = 0; keyIndex < mScrap.size(); keyIndex++) {
            std::vector<ViewHolder*> scrapHeap = mScrap.get(mScrap.keyAt(keyIndex))->mScrapHeap;
            for (int i = 0; i < scrapHeap.size(); i++) {
                //delete scrapHeap.at(i)->itemView;
                //scrapHeap.at(i)->itemView = nullptr;
                //TODO:PoolingContainer.callPoolingContainerOnRelease(scrapHeap.at(i)->itemView);
            }
        }
    }
}

void RecyclerView::RecycledViewPool::onAdapterChanged(RecyclerView::Adapter* oldAdapter, RecyclerView::Adapter* newAdapter,
        bool compatibleWithPrevious) {
    if (oldAdapter != nullptr) {
        detach();
    }
    if (!compatibleWithPrevious && (mAttachCountForClearing == 0)) {
        clear();
    }
    if (newAdapter != nullptr) {
        attach();
    }
}

RecyclerView::RecycledViewPool::ScrapData* RecyclerView::RecycledViewPool::getScrapDataForType(int viewType) {
    ScrapData* scrapData = mScrap.get(viewType);
    if (scrapData == nullptr) {
        scrapData = new ScrapData();
        mScrap.put(viewType, scrapData);
    }
    return scrapData;
}

//////////////*endofRecycledViewPool*//////////////////
RecyclerView* RecyclerView::findNestedRecyclerView(View* view) {
    if (dynamic_cast<ViewGroup*>(view)==nullptr){// instanceof ViewGroup)) {
        return nullptr;
    }
    if (dynamic_cast<RecyclerView*>(view)){//view instanceof RecyclerView) {
        return (RecyclerView*) view;
    }
    ViewGroup* parent = (ViewGroup*) view;
    const int count = parent->getChildCount();
    for (int i = 0; i < count; i++) {
        View* child = parent->getChildAt(i);
        RecyclerView* descendant = findNestedRecyclerView(child);
        if (descendant != nullptr) {
            return descendant;
        }
    }
    return nullptr;
}

/*Utility method for clearing holder's internal RecyclerView, if present */

void RecyclerView::clearNestedRecyclerViewIfNotNested(ViewHolder& holder) {
    if (holder.mNestedRecyclerView != nullptr) {
        View* item = holder.mNestedRecyclerView;//.get();
        while (item != nullptr) {
            if (item == holder.itemView) {
                return; // match found, don't need to clear
            }

            ViewGroup* parent = item->getParent();
            if (dynamic_cast<View*>(parent)){// instanceof View) {
                item = (View*) parent;
            } else {
                item = nullptr;
            }
        }
        holder.mNestedRecyclerView = nullptr; // not nested
    }
}

int64_t RecyclerView::getNanoTime() {
    if (ALLOW_THREAD_GAP_WORK) {
        return SystemClock::uptimeNanos();//System.nanoTime();
    } else {
        return 0;
    }
}

////////////////////////////////////Recycler/////////////////////////////////////////

RecyclerView::Recycler::Recycler(RecyclerView*rv){
    mRV = rv;
    mViewCacheMax = DEFAULT_CACHE_SIZE;
    mChangedScrap = nullptr;
    mRecyclerPool = nullptr;
}

RecyclerView::Recycler::~Recycler(){
    delete mRecyclerPool;
    delete mChangedScrap;
}

void RecyclerView::Recycler::clear() {
    mAttachedScrap.clear();
    recycleAndClearCachedViews();
}

void RecyclerView::Recycler::setViewCacheSize(int viewCount) {
    mRequestedCacheMax = viewCount;
    updateViewCacheSize();
}

void RecyclerView::Recycler::updateViewCacheSize() {
    const int extraCache = mRV->mLayout ? mRV->mLayout->mPrefetchMaxCountObserved : 0;
    mViewCacheMax = mRequestedCacheMax + extraCache;

    // first, try the views that can be recycled
    for (int i = mCachedViews.size() - 1;i >= 0 && mCachedViews.size() > mViewCacheMax; i--) {
        recycleCachedViewAt(i);
    }
}

std::vector<RecyclerView::ViewHolder*> RecyclerView::Recycler::getScrapList() {
    return mUnmodifiableAttachedScrap;
}

bool RecyclerView::Recycler::validateViewHolderForOffsetPosition(ViewHolder* holder) {
    // if it is a removed holder, nothing to verify since we cannot ask adapter anymore
    // if it is not removed, verify the type and id.
    if (holder->isRemoved()) {
        if (sDebugAssertionsEnabled && !mRV->mState->isPreLayout()) {
            LOGE("should not receive a removed view unless it is pre layout");
        }
        return mRV->mState->isPreLayout();
    }
    if ( (holder->mPosition < 0) || (holder->mPosition >= mRV->mAdapter->getItemCount()) ) {
        LOGE("Inconsistency detected. Invalid view holder %p adapter position",holder);
    }
    if (!mRV->mState->isPreLayout()) {
        // don't check type if it is pre-layout.
        const int type = mRV->mAdapter->getItemViewType(holder->mPosition);
        if (type != holder->getItemViewType()) {
            return false;
        }
    }
    if (mRV->mAdapter->hasStableIds()) {
        return holder->getItemId() == mRV->mAdapter->getItemId(holder->mPosition);
    }
    return true;
}

bool RecyclerView::Recycler::tryBindViewHolderByDeadline(ViewHolder& holder, int offsetPosition,
        int position, long deadlineNs) {
    holder.mBindingAdapter = nullptr;
    holder.mOwnerRecyclerView = mRV;//RecyclerView.this;
    const int viewType = holder.getItemViewType();
    int64_t startBindNs = mRV->getNanoTime();
    if (deadlineNs != FOREVER_NS
            && !mRecyclerPool->willBindInTime(viewType, startBindNs, deadlineNs)) {
        // abort - we have a deadline we can't meet
        return false;
    }
    // Holders being bound should be either fully attached or fully detached.
    // We don't want to bind with views that are temporarily detached, because that
    // creates a situation in which they are unable to reason about their attach state
    // properly.
    // For example, isAttachedToWindow will return true, but the itemView will lack a
    // parent. This breaks, among other possible issues, anything involving traversing
    // the view tree, such as ViewTreeLifecycleOwner.
    // Thus, we temporarily reattach any temp-detached holders for the bind operation.
    // See https://issuetracker.google.com/265347515 for additional details on problems
    // resulting from this
    bool reattachedForBind = false;
    if (holder.isTmpDetached()) {
        mRV->attachViewToParent(holder.itemView, mRV->getChildCount(), holder.itemView->getLayoutParams());
        reattachedForBind = true;
    }
    mRV->mAdapter->bindViewHolder(holder, offsetPosition);
    if(reattachedForBind){
        mRV->detachViewFromParent(holder.itemView);
    }

    int64_t endBindNs = mRV->getNanoTime();
    mRecyclerPool->factorInBindTime(holder.getItemViewType(), endBindNs - startBindNs);
    attachAccessibilityDelegateOnBind(holder);
    if (mRV->mState->isPreLayout()) {
        holder.mPreLayoutPosition = position;
    }
    return true;
}

void RecyclerView::Recycler::bindViewToPosition(View* view, int position) {
    ViewHolder* holder = getChildViewHolderInt(view);
    LOGE_IF(holder == nullptr,"The view does not have a ViewHolder. You cannot pass"
                " arbitrary views to this method, they should be created by the Adapter");
    const int offsetPosition = mRV->mAdapterHelper->findPositionOffset(position);
    if (offsetPosition < 0 || offsetPosition >= mRV->mAdapter->getItemCount()) {
        LOGE("Inconsistency detected. Invalid item position %d (offset:%d).state:%d",
             position,offsetPosition,mRV->mState->getItemCount());
    }
    tryBindViewHolderByDeadline(*holder, offsetPosition, position, FOREVER_NS);

    ViewGroup::LayoutParams* lp = holder->itemView->getLayoutParams();
    LayoutParams* rvLayoutParams;
    if (lp == nullptr) {
        rvLayoutParams = (LayoutParams*) mRV->generateDefaultLayoutParams();
        holder->itemView->setLayoutParams(rvLayoutParams);
    } else if (!mRV->checkLayoutParams(lp)) {
        rvLayoutParams = (LayoutParams*) mRV->generateLayoutParams(lp);
        holder->itemView->setLayoutParams(rvLayoutParams);
    } else {
        rvLayoutParams = (LayoutParams*) lp;
    }
    rvLayoutParams->mInsetsDirty = true;
    rvLayoutParams->mViewHolder = holder;
    rvLayoutParams->mPendingInvalidate = holder->itemView->getParent() == nullptr;
}

int RecyclerView::Recycler::convertPreLayoutPositionToPostLayout(int position) {
    if (position < 0 || position >= mRV->mState->getItemCount()) {
        LOGE("invalid position %d . State item count is %d",position,mRV->mState->getItemCount());
    }
    if (!mRV->mState->isPreLayout()) {
       return position;
    }
    return mRV->mAdapterHelper->findPositionOffset(position);
}

View* RecyclerView::Recycler::getViewForPosition(int position) {
    return getViewForPosition(position, false);
}

View* RecyclerView::Recycler::getViewForPosition(int position, bool dryRun) {
    return tryGetViewHolderForPositionByDeadline(position, dryRun, FOREVER_NS)->itemView;
}

RecyclerView::ViewHolder* RecyclerView::Recycler::tryGetViewHolderForPositionByDeadline(int position,
        bool dryRun, long deadlineNs) {
    if (position < 0 || position >= mRV->mState->getItemCount()) {
        LOGE("Invalid item position %d . Item count:%d",position, mRV->mState->getItemCount());
    }
    bool fromScrapOrHiddenOrCache = false;
    ViewHolder* holder = nullptr;
    // 0) If there is a changed scrap, try to find from there
    if (mRV->mState->isPreLayout()) {
        holder = getChangedScrapViewForPosition(position);
        fromScrapOrHiddenOrCache = holder != nullptr;
    }
    // 1) Find by position from scrap/hidden list/cache
    if (holder == nullptr) {
        holder = getScrapOrHiddenOrCachedHolderForPosition(position, dryRun);
        if (holder != nullptr) {
            if (!validateViewHolderForOffsetPosition(holder)) {
                // recycle holder (and unscrap if relevant) since it can't be used
                if (!dryRun) {
                    // we would like to recycle this but need to make sure it is not used by
                    // animation logic etc.
                    holder->addFlags(ViewHolder::FLAG_INVALID);
                    if (holder->isScrap()) {
                        mRV->removeDetachedView(holder->itemView, false);
                        holder->unScrap();
                    } else if (holder->wasReturnedFromScrap()) {
                        holder->clearReturnedFromScrapFlag();
                    }
                    recycleViewHolderInternal(*holder);
                }
                holder = nullptr;
            } else {
                fromScrapOrHiddenOrCache = true;
            }
        }
    }
    if (holder == nullptr) {
        int offsetPosition = mRV->mAdapterHelper->findPositionOffset(position);
        if (offsetPosition < 0 || offsetPosition >= mRV->mAdapter->getItemCount()) {
            LOGE("Inconsistency detected. Invalid item position %d (offset:%d) state:%d",
                position,offsetPosition,mRV->mState->getItemCount());
        }
        const int type = mRV->mAdapter->getItemViewType(offsetPosition);
        // 2) Find from scrap/cache via stable ids, if exists
        if (mRV->mAdapter->hasStableIds()) {
            holder = getScrapOrCachedViewForId(mRV->mAdapter->getItemId(offsetPosition),type, dryRun);
            if (holder != nullptr) {
                // update position
                holder->mPosition = offsetPosition;
                fromScrapOrHiddenOrCache = true;
            }
        }
        if (holder == nullptr && mViewCacheExtension != nullptr) {
            // We are NOT sending the offsetPosition because LayoutManager does not
            // know it.
            View* view = mViewCacheExtension/*->getViewForPositionAndType*/(*this, position, type);
            if (view != nullptr) {
                holder = mRV->getChildViewHolder(view);
                if (holder == nullptr) {
                    LOGE("getViewForPositionAndType returned a view which does not have a ViewHolder");
                } else if (holder->shouldIgnore()) {
                    LOGE("getViewForPositionAndType returned a view that is ignored."
                         " You must call stopIgnoring before returning this view.");
                }
            }
        }
        if (holder == nullptr) { // fallback to pool
            LOGD_IF(sDebugAssertionsEnabled,"tryGetViewHolderForPositionByDeadline(%d) fetching from shared pool",position);
            holder = getRecycledViewPool().getRecycledView(type);
            if (holder != nullptr) {
                holder->resetInternal();
                if (FORCE_INVALIDATE_DISPLAY_LIST) {
                    invalidateDisplayListInt(*holder);
                }
            }
        }
        if (holder == nullptr) {
            int64_t start = mRV->getNanoTime();
            if (deadlineNs != FOREVER_NS
                    && !mRecyclerPool->willCreateInTime(type, start, deadlineNs)) {
                // abort - we have a deadline we can't meet
                return nullptr;
            }
            holder = mRV->mAdapter->createViewHolder(mRV/*RecyclerView.this*/, type);
            if (ALLOW_THREAD_GAP_WORK) {
                // only bother finding nested RV if prefetching
                RecyclerView* innerView = findNestedRecyclerView(holder->itemView);
                if (innerView != nullptr) {
                    holder->mNestedRecyclerView = innerView;//new WeakReference<>(innerView);
                }
            }
            const int64_t end = mRV->getNanoTime();
            mRecyclerPool->factorInCreateTime(type, end - start);
            LOGD_IF(sDebugAssertionsEnabled,"tryGetViewHolderForPositionByDeadline created new ViewHolder");
        }
    }

    // This is very ugly but the only place we can grab this information
    // before the View is rebound and returned to the LayoutManager for post layout ops.
    // We don't need this in pre-layout since the VH is not updated by the LM.
    if (fromScrapOrHiddenOrCache && !mRV->mState->isPreLayout() && holder
           ->hasAnyOfTheFlags(ViewHolder::FLAG_BOUNCED_FROM_HIDDEN_LIST)) {
        holder->setFlags(0, ViewHolder::FLAG_BOUNCED_FROM_HIDDEN_LIST);
        if (mRV->mState->mRunSimpleAnimations) {
            int changeFlags = ItemAnimator::buildAdapterChangeFlagsForAnimations(holder);
            changeFlags |= ItemAnimator::FLAG_APPEARED_IN_PRE_LAYOUT;
            ItemAnimator::ItemHolderInfo* info = mRV->mItemAnimator->recordPreLayoutInformation(
	            *mRV->mState, *holder, changeFlags, *holder->getUnmodifiedPayloads());
            mRV->recordAnimationInfoIfBouncedHiddenView(holder,info);
        }
    }

    bool bound = false;
    if (mRV->mState->isPreLayout() && holder->isBound()) {
        // do not update unless we absolutely have to.
        holder->mPreLayoutPosition = position;
    } else if (!holder->isBound() || holder->needsUpdate() || holder->isInvalid()) {
        LOGE_IF(sDebugAssertionsEnabled && holder->isRemoved(),"Removed holder should be bound and it should"
                 " come here only in pre-layout. Holder: %p",holder);
        const int offsetPosition = mRV->mAdapterHelper->findPositionOffset(position);
        bound = tryBindViewHolderByDeadline(*holder, offsetPosition, position, deadlineNs);
    }

    ViewGroup::LayoutParams* lp = holder->itemView->getLayoutParams();
    LayoutParams* rvLayoutParams;
    if (lp == nullptr) {
        rvLayoutParams = (LayoutParams*) mRV->generateDefaultLayoutParams();
        holder->itemView->setLayoutParams(rvLayoutParams);
    } else if (!mRV->checkLayoutParams(lp)) {
        rvLayoutParams = (LayoutParams*) mRV->generateLayoutParams(lp);
        holder->itemView->setLayoutParams(rvLayoutParams);
    } else {
        rvLayoutParams = (LayoutParams*) lp;
    }
    rvLayoutParams->mViewHolder = holder;
    rvLayoutParams->mPendingInvalidate = fromScrapOrHiddenOrCache && bound;
    return holder;
}

void RecyclerView::Recycler::attachAccessibilityDelegateOnBind(ViewHolder& holder) {
    if (mRV->isAccessibilityEnabled()) {
        View* itemView = holder.itemView;
        if (itemView->getImportantForAccessibility() == View::IMPORTANT_FOR_ACCESSIBILITY_AUTO) {
            itemView->setImportantForAccessibility(View::IMPORTANT_FOR_ACCESSIBILITY_YES);
        }
        if (mRV->mAccessibilityDelegate == nullptr) {
            return;
        }
        AccessibilityDelegate* itemDelegate = mRV->mAccessibilityDelegate->getItemDelegate();
        if (!itemView->getAccessibilityDelegate()){//hasAccessibilityDelegate()) {
            //holder.addFlags(ViewHolder::FLAG_SET_A11Y_ITEM_DELEGATE);
            //temView->setAccessibilityDelegate( mRV->mAccessibilityDelegate->getItemDelegate());
            // If there was already an a11y delegate set on the itemView, store it in the
            // itemDelegate and then set the itemDelegate as the a11y delegate.
            ((RecyclerViewAccessibilityDelegate::ItemDelegate*) itemDelegate)->saveOriginalDelegate(itemView);
        }
        itemView->setAccessibilityDelegate(itemDelegate);
    }
}

void RecyclerView::Recycler::invalidateDisplayListInt(ViewHolder& holder) {
    if (dynamic_cast<ViewGroup*>(holder.itemView)){// instanceof ViewGroup) {
        invalidateDisplayListInt((ViewGroup&)*holder.itemView, false);
    }
}

void RecyclerView::Recycler::invalidateDisplayListInt(ViewGroup& viewGroup, bool invalidateThis) {
    for (int i = viewGroup.getChildCount() - 1; i >= 0; i--) {
        View* view = viewGroup.getChildAt(i);
        if (dynamic_cast<ViewGroup*>(view)) {
            invalidateDisplayListInt((ViewGroup&)*view, true);
        }
    }
    if (!invalidateThis) {
        return;
    }
    // we need to force it to become invisible
    if (viewGroup.getVisibility() == View::INVISIBLE) {
        viewGroup.setVisibility(View::VISIBLE);
        viewGroup.setVisibility(View::INVISIBLE);
    } else {
        const int visibility = viewGroup.getVisibility();
        viewGroup.setVisibility(View::INVISIBLE);
        viewGroup.setVisibility(visibility);
    }
}

void RecyclerView::Recycler::recycleView(View* view) {
    // This public recycle method tries to make view recycle-able since layout manager
    // intended to recycle this view (e.g. even if it is in scrap or change cache)
    ViewHolder* holder = getChildViewHolderInt(view);
    if (holder->isTmpDetached()) {
        mRV->removeDetachedView(view, false);
    }
    if (holder->isScrap()) {
        holder->unScrap();
    } else if (holder->wasReturnedFromScrap()) {
        holder->clearReturnedFromScrapFlag();
    }
    recycleViewHolderInternal(*holder);
    // If the ViewHolder is running ItemAnimator, we want the recycleView() in scroll pass
    // to stop the ItemAnimator and put ViewHolder back in cache or Pool.
    // There are three situations:
    // 1. If the custom Adapter clears ViewPropertyAnimator in view detach like the
    //    leanback (TV) app does, the ItemAnimator is likely to be stopped and
    //    recycleViewHolderInternal will succeed.
    // 2. If the custom Adapter clears ViewPropertyAnimator, but the ItemAnimator uses
    //    "pending runnable" and ViewPropertyAnimator has not started yet, the ItemAnimator
    //    on the view will not be cleared. See b/73552923.
    // 3. If the custom Adapter does not clear ViewPropertyAnimator in view detach, the
    //    ItemAnimator will not be cleared.
    // Since both 2&3 lead to failure of recycleViewHolderInternal(), we just explicitly end
    // the ItemAnimator, the callback of ItemAnimator.endAnimations() will recycle the View.
    //
    // Note the order: we must call endAnimation() after recycleViewHolderInternal()
    // to avoid recycle twice. If ViewHolder isRecyclable is false,
    // recycleViewHolderInternal() will not recycle it, endAnimation() will reset
    // isRecyclable flag and recycle the view.
    if (mRV->mItemAnimator && !holder->isRecyclable()) {
        mRV->mItemAnimator->endAnimation(*holder);
    }
}

void RecyclerView::Recycler::recycleViewInternal(View* view) {
    recycleViewHolderInternal(*getChildViewHolderInt(view));
}

void RecyclerView::Recycler::recycleAndClearCachedViews() {
    const int count = (int)mCachedViews.size();
    for (int i = count - 1; i >= 0; i--) {
        recycleCachedViewAt(i);
    }
    mCachedViews.clear();
    if (ALLOW_THREAD_GAP_WORK) {
        ((GapWorker::LayoutPrefetchRegistryImpl*)mRV->mPrefetchRegistry)->clearPrefetchPositions();
    }
}

void RecyclerView::Recycler::recycleCachedViewAt(int cachedViewIndex) {
    LOGD_IF(sDebugAssertionsEnabled,"Recycling cached view at index %d" ,cachedViewIndex);
    ViewHolder* viewHolder = mCachedViews.at(cachedViewIndex);
    LOGD_IF(sDebugAssertionsEnabled,"CachedViewHolder to be recycled:%p ", viewHolder);
    addViewHolderToRecycledViewPool(*viewHolder, true);
    mCachedViews.erase(mCachedViews.begin()+cachedViewIndex);//remove(cachedViewIndex);
}

void RecyclerView::Recycler::recycleViewHolderInternal(ViewHolder& holder) {
    if (holder.isScrap() || holder.itemView->getParent() != nullptr) {
        LOGE("Scrapped or attached views may not be recycled. isScrap:%d isAttached:%d",
              holder.isScrap(),(holder.itemView->getParent() != nullptr));
    }

    if (holder.isTmpDetached()) {
        LOGE("Tmp detached view should be removed from RecyclerView "
	   "before it can be recycled: %p",&holder);
    }

    if (holder.shouldIgnore()) {
        LOGE("Trying to recycle an ignored view holder. You should "
	         "first call stopIgnoringView(view) before calling recycle.");
    }
    //noinspection unchecked
    const bool transientStatePreventsRecycling = holder.doesTransientStatePreventRecycling();
    const bool forceRecycle = mRV->mAdapter && transientStatePreventsRecycling
            && mRV->mAdapter->onFailedToRecycleView(holder);
    bool cached = false;
    bool recycled = false;
    auto it = std::find(mCachedViews.begin(),mCachedViews.end(),&holder);
    if (sDebugAssertionsEnabled && it!=mCachedViews.end()){//mCachedViews.contains(holder)) {
        LOGE("cached view received recycle internal? %p",&holder);
    }
    if (forceRecycle || holder.isRecyclable()) {
        if (mViewCacheMax > 0
                && !holder.hasAnyOfTheFlags(ViewHolder::FLAG_INVALID
                | ViewHolder::FLAG_REMOVED | ViewHolder::FLAG_UPDATE
                | ViewHolder::FLAG_ADAPTER_POSITION_UNKNOWN)) {
            // Retire oldest cached view
            int cachedViewSize = mCachedViews.size();
            if (cachedViewSize >= mViewCacheMax && cachedViewSize > 0) {
                recycleCachedViewAt(0);
                cachedViewSize--;
            }
            GapWorker::LayoutPrefetchRegistryImpl*prefetchRegistry=(GapWorker::LayoutPrefetchRegistryImpl*)mRV->mPrefetchRegistry;
            int targetCacheIndex = cachedViewSize;
            if (ALLOW_THREAD_GAP_WORK && (cachedViewSize > 0)
                    && !prefetchRegistry->lastPrefetchIncludedPosition(holder.mPosition)) {
                // when adding the view, skip past most recently prefetched views
                int cacheIndex = cachedViewSize - 1;
                while (cacheIndex >= 0) {
                    const int cachedPos = mCachedViews.at(cacheIndex)->mPosition;
                    if (!prefetchRegistry->lastPrefetchIncludedPosition(cachedPos)) {
                        break;
                    }
                    cacheIndex--;
                }
                targetCacheIndex = cacheIndex + 1;
            }
            mCachedViews.insert(mCachedViews.begin()+targetCacheIndex,&holder);//add(targetCacheIndex, holder);
            cached = true;
        }
        if (!cached) {
            addViewHolderToRecycledViewPool(holder, true);
            recycled = true;
        }
    } else {
        // NOTE: A view can fail to be recycled when it is scrolled off while an animation
        // runs. In this case, the item is eventually recycled by
        // ItemAnimatorRestoreListener#onAnimationFinished.

        // TODO: consider cancelling an animation when an item is removed scrollBy,
        // to return it to the pool faster
        LOGD_IF(sDebugAssertionsEnabled,"trying to recycle a non-recycleable holder. Hopefully, it will "
             "re-visit here. We are still removing it from animation lists");
    }
    // even if the holder is not removed, we still call this method so that it is removed
    // from view holder lists.
    mRV->mViewInfoStore->removeViewHolder(&holder);
    if (!cached && !recycled && transientStatePreventsRecycling) {
        //TODO:PoolingContainer.callPoolingContainerOnRelease(holder.itemView);
        holder.mBindingAdapter = nullptr;
        holder.mOwnerRecyclerView = nullptr;
    }
}

void RecyclerView::Recycler::addViewHolderToRecycledViewPool(ViewHolder& holder, bool dispatchRecycled) {
    clearNestedRecyclerViewIfNotNested(holder);
    View* itemView = holder.itemView;
    if(mRV->mAccessibilityDelegate!=nullptr){
        AccessibilityDelegate* itemDelegate = mRV->mAccessibilityDelegate->getItemDelegate();
        AccessibilityDelegate* originalDelegate = nullptr;
        if (dynamic_cast<RecyclerViewAccessibilityDelegate::ItemDelegate*>(itemDelegate)){
            originalDelegate = ((RecyclerViewAccessibilityDelegate::ItemDelegate*) itemDelegate)
                ->getAndRemoveOriginalDelegateForItem(itemView);
        }
        // Set the a11y delegate back to whatever the original delegate was.
        itemView->setAccessibilityDelegate(originalDelegate);
    }
    if (dispatchRecycled) {
        dispatchViewRecycled(holder);
    }
    holder.mBindingAdapter = nullptr;
    holder.mOwnerRecyclerView = nullptr;
    getRecycledViewPool().putRecycledView(&holder);
}

void RecyclerView::Recycler::quickRecycleScrapView(View* view) {
    ViewHolder* holder = getChildViewHolderInt(view);
    holder->mScrapContainer = nullptr;
    holder->mInChangeScrap = false;
    holder->clearReturnedFromScrapFlag();
    recycleViewHolderInternal(*holder);
}

void RecyclerView::Recycler::scrapView(View* view) {
    ViewHolder* holder = getChildViewHolderInt(view);
    if (holder->hasAnyOfTheFlags(ViewHolder::FLAG_REMOVED | ViewHolder::FLAG_INVALID)
            || !holder->isUpdated() || mRV->canReuseUpdatedViewHolder(*holder)) {
        if (holder->isInvalid() && !holder->isRemoved() && !mRV->mAdapter->hasStableIds()) {
            LOGE("Called scrap view with an invalid view. Invalid views cannot be "
                 "reused from scrap, they should rebound from recycler pool.");
        }
        holder->setScrapContainer(this, false);
        mAttachedScrap.push_back(holder);
    } else {
        if(mChangedScrap == nullptr){
            mChangedScrap = new std::vector<ViewHolder*>;
        }
        holder->setScrapContainer(this, true);
        mChangedScrap->push_back(holder);
    }
}

void RecyclerView::Recycler::unscrapView(ViewHolder& holder) {
    if (holder.mInChangeScrap) {
        auto it = std::find(mChangedScrap->begin(),mChangedScrap->end(),&holder);
        if(it!=mChangedScrap->end())mChangedScrap->erase(it);
    } else {
        auto it = std::find(mAttachedScrap.begin(),mAttachedScrap.end(),&holder);
        if(it!=mAttachedScrap.end())mAttachedScrap.erase(it);
    }
    holder.mScrapContainer = nullptr;
    holder.mInChangeScrap = false;
    holder.clearReturnedFromScrapFlag();
}

int RecyclerView::Recycler::getScrapCount() {
    return mAttachedScrap.size();
}

View* RecyclerView::Recycler::getScrapViewAt(int index) {
    return mAttachedScrap.at(index)->itemView;
}

void RecyclerView::Recycler::clearScrap() {
    mAttachedScrap.clear();
    if (mChangedScrap != nullptr) {
        mChangedScrap->clear();
    }
}

RecyclerView::ViewHolder* RecyclerView::Recycler::getChangedScrapViewForPosition(int position) {
    // If pre-layout, check the changed scrap for an exact match.
    size_t changedScrapSize;
    if ((mChangedScrap == nullptr) || ((changedScrapSize = mChangedScrap->size()) == 0) ) {
        return nullptr;
    }
    // find by position
    for (size_t i = 0; i < changedScrapSize; i++) {
        ViewHolder* holder = mChangedScrap->at(i);
        if (!holder->wasReturnedFromScrap() && (holder->getLayoutPosition() == position) ) {
            holder->addFlags(ViewHolder::FLAG_RETURNED_FROM_SCRAP);
            return holder;
        }
    }
    // find by id
    if (mRV->mAdapter->hasStableIds()) {
        const int offsetPosition = mRV->mAdapterHelper->findPositionOffset(position);
        if ((offsetPosition > 0) && (offsetPosition < mRV->mAdapter->getItemCount())) {
            const long id = mRV->mAdapter->getItemId(offsetPosition);
            for (int i = 0; i < changedScrapSize; i++) {
                ViewHolder* holder = mChangedScrap->at(i);
                if (!holder->wasReturnedFromScrap() && (holder->getItemId() == id)) {
                    holder->addFlags(ViewHolder::FLAG_RETURNED_FROM_SCRAP);
                    return holder;
                }
            }
        }
    }
    return nullptr;
}

RecyclerView::ViewHolder* RecyclerView::Recycler::getScrapOrHiddenOrCachedHolderForPosition(int position, bool dryRun) {
    const int scrapCount = (int)mAttachedScrap.size();

    // Try first for an exact, non-invalid match from scrap.
    for (int i = 0; i < scrapCount; i++) {
        ViewHolder* holder = mAttachedScrap.at(i);
        if (!holder->wasReturnedFromScrap() && (holder->getLayoutPosition() == position)
                && !holder->isInvalid() && (mRV->mState->mInPreLayout || !holder->isRemoved())) {
            holder->addFlags(ViewHolder::FLAG_RETURNED_FROM_SCRAP);
            return holder;
        }
    }

    if (!dryRun) {
        View* view = mRV->mChildHelper->findHiddenNonRemovedView(position);
        if (view != nullptr) {
            // This View is good to be used. We just need to unhide, detach and move to the
            // scrap list.
            ViewHolder* vh = getChildViewHolderInt(view);
            mRV->mChildHelper->unhide(view);
            int layoutIndex = mRV->mChildHelper->indexOfChild(view);
            if (layoutIndex == RecyclerView::NO_POSITION) {
                LOGE("layout index should not be -1 after %p unhiding a view:",vh);
            }
            mRV->mChildHelper->detachViewFromParent(layoutIndex);
            scrapView(view);
            vh->addFlags(ViewHolder::FLAG_RETURNED_FROM_SCRAP
                    | ViewHolder::FLAG_BOUNCED_FROM_HIDDEN_LIST);
            return vh;
        }
    }

    // Search in our first-level recycled view cache.
    const size_t cacheSize = mCachedViews.size();
    for (size_t i = 0; i < cacheSize; i++) {
        ViewHolder* holder = mCachedViews.at(i);
        // invalid view holders may be in cache if adapter has stable ids as they can be
        // retrieved via getScrapOrCachedViewForId
        if (!holder->isInvalid() && (holder->getLayoutPosition() == position)
                && !holder->isAttachedToTransitionOverlay()) {
            if (!dryRun) {
                mCachedViews.erase(mCachedViews.begin()+i);//remove(i);
            }
            LOGD_IF(sDebugAssertionsEnabled,"getScrapOrHiddenOrCachedHolderForPosition(%d) found match in cache:%p",position,holder);
            return holder;
        }
    }
    return nullptr;
}

RecyclerView::ViewHolder* RecyclerView::Recycler::getScrapOrCachedViewForId(long id, int type, bool dryRun) {
    // Look in our attached views first
    const int count = (int)mAttachedScrap.size();
    for (int i = count - 1; i >= 0; i--) {
        ViewHolder* holder = mAttachedScrap.at(i);
        if ((holder->getItemId() == id) && !holder->wasReturnedFromScrap()) {
            if (type == holder->getItemViewType()) {
                holder->addFlags(ViewHolder::FLAG_RETURNED_FROM_SCRAP);
                if (holder->isRemoved()) {
                    // this might be valid in two cases:
                    // > item is removed but we are in pre-layout pass
                    // >> do nothing. return as is. make sure we don't rebind
                    // > item is removed then added to another position and we are in
                    // post layout.
                    // >> remove removed and invalid flags, add update flag to rebind
                    // because item was invisible to us and we don't know what happened in
                    // between.
                    if (!mRV->mState->isPreLayout()) {
                        holder->setFlags(ViewHolder::FLAG_UPDATE, ViewHolder::FLAG_UPDATE
                                | ViewHolder::FLAG_INVALID | ViewHolder::FLAG_REMOVED);
                    }
                }
                return holder;
            } else if (!dryRun) {
                // if we are running animations, it is actually better to keep it in scrap
                // but this would force layout manager to lay it out which would be bad.
                // Recycle this scrap. Type mismatch.
                mAttachedScrap.erase(mAttachedScrap.begin()+i);//remove(i);
                mRV->removeDetachedView(holder->itemView, false);
                quickRecycleScrapView(holder->itemView);
            }
        }
    }

    // Search the first-level cache
    const int cacheSize = (int)mCachedViews.size();
    for (int i = cacheSize - 1; i >= 0; i--) {
        ViewHolder* holder = mCachedViews.at(i);
        if ((holder->getItemId() == id) && !holder->isAttachedToTransitionOverlay()) {
            if (type == holder->getItemViewType()) {
                if (!dryRun) {
                    mCachedViews.erase(mCachedViews.begin()+i);//remove(i);
                }
                return holder;
            } else if (!dryRun) {
                recycleCachedViewAt(i);
                return nullptr;
            }
        }
    }
    return nullptr;
}

void RecyclerView::Recycler::dispatchViewRecycled(ViewHolder& holder) {
    if (mRV->mRecyclerListener != nullptr) {
        mRV->mRecyclerListener(holder);//.onViewRecycled(holder);
    }
    const int listenerCount = mRV->mRecyclerListeners.size();
    for (int i = 0; i < listenerCount; i++) {
        mRV->mRecyclerListeners.at(i)(holder);
    }
    if (mRV->mAdapter != nullptr) {
        mRV->mAdapter->onViewRecycled(holder);
    }
    if (mRV->mState != nullptr) {
        mRV->mViewInfoStore->removeViewHolder(&holder);
    }
    LOGD_IF(sDebugAssertionsEnabled,"dispatchViewRecycled: %p" ,&holder);
}

void RecyclerView::Recycler::onAdapterChanged(Adapter* oldAdapter, Adapter* newAdapter,
        bool compatibleWithPrevious) {
    clear();
    poolingContainerDetach(oldAdapter, true);
    mRV->getRecycledViewPool().onAdapterChanged(oldAdapter, newAdapter, compatibleWithPrevious);
    maybeSendPoolingContainerAttach();
}

void RecyclerView::Recycler::offsetPositionRecordsForMove(int from, int to) {
    int start, end, inBetweenOffset;
    if (from < to) {
        start = from;
        end = to;
        inBetweenOffset = -1;
    } else {
        start = to;
        end = from;
        inBetweenOffset = 1;
    }
    const int cachedCount = (int)mCachedViews.size();
    for (int i = 0; i < cachedCount; i++) {
        ViewHolder* holder = mCachedViews.at(i);
        if ((holder == nullptr) || (holder->mPosition < start) || (holder->mPosition > end)) {
            continue;
        }
        if (holder->mPosition == from) {
            holder->offsetPosition(to - from, false);
        } else {
            holder->offsetPosition(inBetweenOffset, false);
        }
        LOGD_IF(sDebugAssertionsEnabled,"offsetPositionRecordsForMove cached child %d holder %p",i,holder);
    }
}

void RecyclerView::Recycler::offsetPositionRecordsForInsert(int insertedAt, int count) {
    const size_t cachedCount = mCachedViews.size();
    for (size_t i = 0; i < cachedCount; i++) {
        ViewHolder* holder = mCachedViews.at(i);
        if (holder && holder->mPosition >= insertedAt) {
            LOGD_IF(sDebugAssertionsEnabled,"offsetPositionRecordsForInsert cached %d holder %p now at position %d",i,holder,(holder->mPosition + count));
            holder->offsetPosition(count, true);
        }
    }
}

void RecyclerView::Recycler::offsetPositionRecordsForRemove(int removedFrom, int count, bool applyToPreLayout) {
    const int removedEnd = removedFrom + count;
    const int cachedCount = (int)mCachedViews.size();
    for (int i = cachedCount - 1; i >= 0; i--) {
        ViewHolder* holder = mCachedViews.at(i);
        if (holder != nullptr) {
            if (holder->mPosition >= removedEnd) {
                LOGD_IF(sDebugAssertionsEnabled,"offsetPositionRecordsForRemove cached %d holder %p now at positon %d",i,holder,(holder->mPosition - count));
                holder->offsetPosition(-count, applyToPreLayout);
            } else if (holder->mPosition >= removedFrom) {
                // Item for this view was removed. Dump it from the cache.
                holder->addFlags(ViewHolder::FLAG_REMOVED);
                recycleCachedViewAt(i);
            }
        }
    }
}

void RecyclerView::Recycler::setViewCacheExtension(const ViewCacheExtension& extension) {
    mViewCacheExtension = extension;
}

void RecyclerView::Recycler::setRecycledViewPool(RecycledViewPool* pool) {
    poolingContainerDetach(mRV->mAdapter);
    if (mRecyclerPool != nullptr) {
        mRecyclerPool->detach();
    }
    mRecyclerPool = pool;
    if (mRecyclerPool && (mRV->getAdapter() != nullptr)) {
        mRecyclerPool->attach();
    }
    maybeSendPoolingContainerAttach();
}

void RecyclerView::Recycler::maybeSendPoolingContainerAttach() {
    if ((mRecyclerPool != nullptr) && (mRV->mAdapter != nullptr) && mRV->isAttachedToWindow()) {
        mRecyclerPool->attachForPoolingContainer(mRV->mAdapter);
    }
}

void RecyclerView::Recycler::poolingContainerDetach(Adapter*adapter) {
    poolingContainerDetach(adapter, false);
}

void RecyclerView::Recycler::poolingContainerDetach(Adapter* adapter, bool isBeingReplaced) {
    if (mRecyclerPool != nullptr) {
        mRecyclerPool->detachForPoolingContainer(adapter, isBeingReplaced);
    }
}

void RecyclerView::Recycler::onAttachedToWindow() {
    maybeSendPoolingContainerAttach();
}

void RecyclerView::Recycler::onDetachedFromWindow() {
    for (int i = 0; i < mCachedViews.size(); i++) {
        //delete mCachedViews.at(i)->itemView;
        //mCachedViews.at(i)->itemView = nullptr;
        //TODO:PoolingContainer.callPoolingContainerOnRelease(mCachedViews.at(i)->itemView);
    }
    poolingContainerDetach(mRV->mAdapter);
}

RecyclerView::RecycledViewPool& RecyclerView::Recycler::getRecycledViewPool() {
    if (mRecyclerPool == nullptr) {
        mRecyclerPool = new RecycledViewPool();
        maybeSendPoolingContainerAttach();
    }
    return *mRecyclerPool;
}

void RecyclerView::Recycler::viewRangeUpdate(int positionStart, int itemCount) {
    const int positionEnd = positionStart + itemCount;
    const int cachedCount = (int)mCachedViews.size();
    for (int i = cachedCount - 1; i >= 0; i--) {
        ViewHolder* holder = mCachedViews.at(i);
        if (holder == nullptr) {
            continue;
        }

        const int pos = holder->mPosition;
        if ((pos >= positionStart) && (pos < positionEnd) ) {
            holder->addFlags(ViewHolder::FLAG_UPDATE);
            recycleCachedViewAt(i);
            // cached views should not be flagged as changed because this will cause them
            // to animate when they are returned from cache.
        }
    }
}

void RecyclerView::Recycler::markKnownViewsInvalid() {
    const int cachedCount = (int)mCachedViews.size();
    for (int i = 0; i < cachedCount; i++) {
        ViewHolder* holder = mCachedViews.at(i);
        if (holder != nullptr) {
            holder->addFlags(ViewHolder::FLAG_UPDATE | ViewHolder::FLAG_INVALID);
            holder->addChangePayload(nullptr);
        }
    }

    if ((mRV->mAdapter == nullptr) || !mRV->mAdapter->hasStableIds()) {
        // we cannot re-use cached views in this case. Recycle them all
        recycleAndClearCachedViews();
    }
}

void RecyclerView::Recycler::clearOldPositions() {
    const int cachedCount = (int)mCachedViews.size();
    for (int i = 0; i < cachedCount; i++) {
        ViewHolder* holder = mCachedViews.at(i);
        holder->clearOldPosition();
    }
    const int scrapCount = (int)mAttachedScrap.size();
    for (int i = 0; i < scrapCount; i++) {
        mAttachedScrap.at(i)->clearOldPosition();
    }
    if (mChangedScrap != nullptr) {
        const int changedScrapCount = (int)mChangedScrap->size();
        for (int i = 0; i < changedScrapCount; i++) {
            mChangedScrap->at(i)->clearOldPosition();
        }
    }
}

void RecyclerView::Recycler::markItemDecorInsetsDirty() {
    const int cachedCount = (int)mCachedViews.size();
    for (int i = 0; i < cachedCount; i++) {
        ViewHolder* holder = mCachedViews.at(i);
        LayoutParams* layoutParams = (LayoutParams*) holder->itemView->getLayoutParams();
        if (layoutParams != nullptr) {
            layoutParams->mInsetsDirty = true;
        }
    }
}
///////////////////////////////endof Recycler/////////////////////////////

//public abstract static class Adapter<VH extends ViewHolder> {
RecyclerView::Adapter::Adapter(){
    mHasStableIds = false;
    mObservable = new AdapterDataObservable();
}

RecyclerView::Adapter::~Adapter(){
    delete mObservable;
}

void RecyclerView::Adapter::onBindViewHolder(ViewHolder& holder, int position,std::vector<Object*>& payloads) {
    onBindViewHolder(holder, position);
}

int RecyclerView::Adapter::findRelativeAdapterPositionIn(Adapter&adapter,ViewHolder&,int localPosition){
    if(&adapter==this)return localPosition;
    return NO_POSITION;
}

RecyclerView::ViewHolder* RecyclerView::Adapter::createViewHolder(ViewGroup* parent, int viewType) {
    ViewHolder* holder = onCreateViewHolder(parent, viewType);
    if (holder->itemView->getParent() != nullptr) {
        LOGE("ViewHolder views must not be attached when"
             " created. Ensure that you are not passing 'true' to the attachToRoot"
             " parameter of LayoutInflater.inflate(..., bool attachToRoot)");
    }
    holder->mItemViewType = viewType;
    return holder;
}

void RecyclerView::Adapter::bindViewHolder(ViewHolder& holder, int position) {
    const bool rootBind = holder.mBindingAdapter ==nullptr;
    if(rootBind){
        holder.mPosition = position;
        if (hasStableIds()) {
            holder.mItemId = getItemId(position);
        }
        holder.setFlags(ViewHolder::FLAG_BOUND,ViewHolder::FLAG_BOUND | ViewHolder::FLAG_UPDATE
           | ViewHolder::FLAG_INVALID | ViewHolder::FLAG_ADAPTER_POSITION_UNKNOWN);
    }
    holder.mBindingAdapter = nullptr;
    onBindViewHolder(holder, position, *holder.getUnmodifiedPayloads());
    if(rootBind){
        holder.clearPayload();
        ViewGroup::LayoutParams* layoutParams = holder.itemView->getLayoutParams();
        if (dynamic_cast<RecyclerView::LayoutParams*>(layoutParams)) {
            ((LayoutParams*) layoutParams)->mInsetsDirty = true;
        }
    }
}

int RecyclerView::Adapter::getItemViewType(int position) {
    return 0;
}

void RecyclerView::Adapter::setHasStableIds(bool hasStableIds) {
    if (hasObservers()) {
        throw std::runtime_error("Cannot change whether this adapter has "
              "stable IDs while the adapter has registered observers.");
    }
    mHasStableIds = hasStableIds;
}

long RecyclerView::Adapter::getItemId(int position) {
    return NO_ID;
}


bool RecyclerView::Adapter::hasStableIds() {
    return mHasStableIds;
}

void RecyclerView::Adapter::onViewRecycled(ViewHolder& holder) {
}

bool RecyclerView::Adapter::onFailedToRecycleView(ViewHolder& holder) {
    return false;
}

void RecyclerView::Adapter::onViewAttachedToWindow(ViewHolder& holder) {
}

void RecyclerView::Adapter::onViewDetachedFromWindow(ViewHolder& holder) {
}

bool RecyclerView::Adapter::hasObservers()const{
    return mObservable->hasObservers();
}

void RecyclerView::Adapter::registerAdapterDataObserver(RecyclerView::AdapterDataObserver* observer) {
     mObservable->registerObserver(observer);
}

void RecyclerView::Adapter::unregisterAdapterDataObserver(RecyclerView::AdapterDataObserver* observer) {
    mObservable->unregisterObserver(observer);
}

void RecyclerView::Adapter::onAttachedToRecyclerView(RecyclerView& recyclerView) {
}

void RecyclerView::Adapter::onDetachedFromRecyclerView(RecyclerView& recyclerView) {
}

void RecyclerView::Adapter::notifyDataSetChanged() {
    mObservable->notifyChanged();
}

void RecyclerView::Adapter::notifyItemChanged(int position) {
    mObservable->notifyItemRangeChanged(position, 1);
}

void RecyclerView::Adapter::notifyItemChanged(int position,Object* payload) {
    mObservable->notifyItemRangeChanged(position, 1, payload);
}

void RecyclerView::Adapter::notifyItemRangeChanged(int positionStart, int itemCount) {
    mObservable->notifyItemRangeChanged(positionStart, itemCount);
}

void RecyclerView::Adapter::notifyItemRangeChanged(int positionStart, int itemCount,Object* payload) {
    mObservable->notifyItemRangeChanged(positionStart, itemCount, payload);
}

void RecyclerView::Adapter::notifyItemInserted(int position) {
     mObservable->notifyItemRangeInserted(position, 1);
}

void RecyclerView::Adapter::notifyItemMoved(int fromPosition, int toPosition) {
     mObservable->notifyItemMoved(fromPosition, toPosition);
}

void RecyclerView::Adapter::notifyItemRangeInserted(int positionStart, int itemCount) {
     mObservable->notifyItemRangeInserted(positionStart, itemCount);
}

void RecyclerView::Adapter::notifyItemRemoved(int position) {
     mObservable->notifyItemRangeRemoved(position, 1);
}

void RecyclerView::Adapter::notifyItemRangeRemoved(int positionStart, int itemCount) {
     mObservable->notifyItemRangeRemoved(positionStart, itemCount);
}

void RecyclerView::Adapter::setStateRestorationPolicy(StateRestorationPolicy strategy) {
    mStateRestorationPolicy = strategy;
    mObservable->notifyStateRestorationPolicyChanged();
}

RecyclerView::Adapter::StateRestorationPolicy RecyclerView::Adapter::getStateRestorationPolicy() const{
    return mStateRestorationPolicy;
}

bool RecyclerView::Adapter::canRestoreState(){
    switch (mStateRestorationPolicy) {
        case PREVENT:
            return false;
        case PREVENT_WHEN_EMPTY:
            return getItemCount() > 0;
        default:
            return true;
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////

void RecyclerView::dispatchChildDetached(View* child) {
    ViewHolder* viewHolder = getChildViewHolderInt(child);
    onChildDetachedFromWindow(child);
    if (mAdapter != nullptr && viewHolder != nullptr) {
        mAdapter->onViewDetachedFromWindow(*viewHolder);
    }
    const int cnt = (int)mOnChildAttachStateListeners.size();
    for (int i = cnt - 1; i >= 0; i--) {
        auto& ls = mOnChildAttachStateListeners.at(i);
        FATAL_IF(ls.onChildViewDetachedFromWindow==nullptr,"onChildViewDetachedFromWindow cant be nullptr");
        ls.onChildViewDetachedFromWindow(*child);
    }
}

void RecyclerView::dispatchChildAttached(View* child) {
    ViewHolder* viewHolder = getChildViewHolderInt(child);
    onChildAttachedToWindow(child);
    if (mAdapter != nullptr && viewHolder != nullptr) {
        mAdapter->onViewAttachedToWindow(*viewHolder);
    }
    const int cnt = (int)mOnChildAttachStateListeners.size();
    for (int i = cnt - 1; i >= 0; i--) {
        auto& ls = mOnChildAttachStateListeners.at(i);
        FATAL_IF(ls.onChildViewAttachedToWindow==nullptr,"onChildViewAttachedToWindow cant be nullptr");
        ls.onChildViewAttachedToWindow(*child);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

RecyclerView::LayoutManager::LayoutManager(){

    mHorizontalBoundCheckCallback.getChildAt=[this](int index)->View* {
        return this->getChildAt(index);
    };

    mHorizontalBoundCheckCallback.getParentStart=[this]()->int {
        return this->getPaddingLeft();
    };

    mHorizontalBoundCheckCallback.getParentEnd=[this]()->int {
        return this->getWidth() - this->getPaddingRight();
    };

    mHorizontalBoundCheckCallback.getChildStart=[this](View* view)->int {
        RecyclerView::LayoutParams* params = (RecyclerView::LayoutParams*)view->getLayoutParams();
        return this->getDecoratedLeft(view) - params->leftMargin;
    };

    mHorizontalBoundCheckCallback.getChildEnd=[this](View* view)->int {
        RecyclerView::LayoutParams* params = (RecyclerView::LayoutParams*)view->getLayoutParams();
        return this->getDecoratedRight(view) + params->rightMargin;
    };


    mVerticalBoundCheckCallback.getChildAt=[this](int index)->View* {
        return this->getChildAt(index);
    };

    mVerticalBoundCheckCallback.getParentStart=[this]()->int {
        return this->getPaddingTop();
    };

    mVerticalBoundCheckCallback.getParentEnd=[this]()->int {
        return this->getHeight()- this->getPaddingBottom();
    };

    mVerticalBoundCheckCallback.getChildStart=[this](View* view)->int {
        RecyclerView::LayoutParams* params = (RecyclerView::LayoutParams*)view->getLayoutParams();
        return this->getDecoratedTop(view) - params->topMargin;
    };

    mVerticalBoundCheckCallback.getChildEnd=[this](View* view)->int {
        RecyclerView::LayoutParams* params = (RecyclerView::LayoutParams*)view->getLayoutParams();
        return this->getDecoratedBottom(view) + params->bottomMargin;
    };
    mHorizontalBoundCheck = new ViewBoundsCheck(mHorizontalBoundCheckCallback);
    mVerticalBoundCheck = new ViewBoundsCheck(mVerticalBoundCheckCallback);
    mSmoothScroller = nullptr;
    mChildHelper = nullptr;
    mRecyclerView = nullptr;
    mAutoMeasure = false;
    mPrefetchMaxCountObserved = 0;
    mPrefetchMaxObservedInInitialPrefetch = false;
}/*endof RecyclerView::LayoutManager::LayoutManager*/

RecyclerView::LayoutManager::~LayoutManager(){
    delete mHorizontalBoundCheck;
    delete mVerticalBoundCheck;
    delete mSmoothScroller;
}

void RecyclerView::LayoutManager::setRecyclerView(RecyclerView* recyclerView) {
    if (recyclerView == nullptr) {
        mRecyclerView = nullptr;
        mChildHelper = nullptr;
        mWidth = 0;
        mHeight = 0;
    } else {
        mRecyclerView = recyclerView;
        mChildHelper = recyclerView->mChildHelper;
        mWidth = recyclerView->getWidth();
        mHeight = recyclerView->getHeight();
    }
    mWidthMode = MeasureSpec::EXACTLY;
    mHeightMode = MeasureSpec::EXACTLY;
}

void RecyclerView::LayoutManager::setMeasureSpecs(int wSpec, int hSpec) {
    mWidth = MeasureSpec::getSize(wSpec);
    mWidthMode = MeasureSpec::getMode(wSpec);
    if (mWidthMode == MeasureSpec::UNSPECIFIED && !ALLOW_SIZE_IN_UNSPECIFIED_SPEC) {
        mWidth = 0;
    }

    mHeight = MeasureSpec::getSize(hSpec);
    mHeightMode = MeasureSpec::getMode(hSpec);
    if (mHeightMode == MeasureSpec::UNSPECIFIED && !ALLOW_SIZE_IN_UNSPECIFIED_SPEC) {
        mHeight = 0;
    }
}

void RecyclerView::LayoutManager::setMeasuredDimensionFromChildren(int widthSpec, int heightSpec) {
    const int count = getChildCount();
    if (count == 0) {
        mRecyclerView->defaultOnMeasure(widthSpec, heightSpec);
        return;
    }
    int minX = INT_MAX;//Integer.MAX_VALUE;
    int minY = INT_MAX;//Integer.MAX_VALUE;
    int maxX = INT_MIN;//Integer.MIN_VALUE;
    int maxY = INT_MIN;//Integer.MIN_VALUE;

    for (int i = 0; i < count; i++) {
        View* child = getChildAt(i);
        Rect& bounds = mRecyclerView->mTempRect;
        getDecoratedBoundsWithMargins(child, bounds);
        if (bounds.left < minX) {
            minX = bounds.left;
        }
        if (bounds.right() > maxX) {
            maxX = bounds.right();
        }
        if (bounds.top < minY) {
            minY = bounds.top;
        }
        if (bounds.bottom() > maxY) {
            maxY = bounds.bottom();
        }
    }
    mRecyclerView->mTempRect.set(minX, minY, maxX, maxY);
    setMeasuredDimension(mRecyclerView->mTempRect, widthSpec, heightSpec);
}

void RecyclerView::LayoutManager::setMeasuredDimension(Rect& childrenBounds, int wSpec, int hSpec) {
    int usedWidth = childrenBounds.width + getPaddingLeft() + getPaddingRight();
    int usedHeight = childrenBounds.height + getPaddingTop() + getPaddingBottom();
    int width = chooseSize(wSpec, usedWidth, getMinimumWidth());
    int height = chooseSize(hSpec, usedHeight, getMinimumHeight());
    setMeasuredDimension(width, height);
}

void RecyclerView::LayoutManager::requestLayout() {
    if (mRecyclerView != nullptr) {
        mRecyclerView->requestLayout();
    }
}

void RecyclerView::LayoutManager::assertInLayoutOrScroll(const std::string& message) {
    if (mRecyclerView != nullptr) {
        mRecyclerView->assertInLayoutOrScroll(message);
    }
}

int RecyclerView::LayoutManager::chooseSize(int spec, int desired, int min) {
    const int mode = MeasureSpec::getMode(spec);
    const int size = MeasureSpec::getSize(spec);
    switch (mode) {
    case MeasureSpec::EXACTLY:
        return size;
    case MeasureSpec::AT_MOST:
        return std::min(size, std::max(desired, min));
    case MeasureSpec::UNSPECIFIED:
    default:  return std::max(desired, min);
    }
}

void RecyclerView::LayoutManager::assertNotInLayoutOrScroll(const std::string& message) {
    if (mRecyclerView != nullptr) {
        mRecyclerView->assertNotInLayoutOrScroll(message);
    }
}

//@Deprecated
void RecyclerView::LayoutManager::setAutoMeasureEnabled(bool enabled) {
    mAutoMeasure = enabled;
}

bool RecyclerView::LayoutManager::isAutoMeasureEnabled()const {
    return mAutoMeasure;
}

bool RecyclerView::LayoutManager::supportsPredictiveItemAnimations() {
    return false;
}

void RecyclerView::LayoutManager::setItemPrefetchEnabled(bool enabled) {
    if (enabled != mItemPrefetchEnabled) {
        mItemPrefetchEnabled = enabled;
        mPrefetchMaxCountObserved = 0;
        if (mRecyclerView != nullptr) {
            mRecyclerView->mRecycler->updateViewCacheSize();
        }
    }
}

bool RecyclerView::LayoutManager::isItemPrefetchEnabled() {
    return mItemPrefetchEnabled;
}

void  RecyclerView::LayoutManager::collectAdjacentPrefetchPositions(int dx, int dy, State& state,
        LayoutPrefetchRegistry& layoutPrefetchRegistry) {}

void  RecyclerView::LayoutManager::collectInitialPrefetchPositions(int adapterItemCount,
        LayoutPrefetchRegistry& layoutPrefetchRegistry) {}

void RecyclerView::LayoutManager::dispatchAttachedToWindow(RecyclerView& view) {
    mIsAttachedToWindow = true;
    onAttachedToWindow(view);
}

void RecyclerView::LayoutManager::dispatchDetachedFromWindow(RecyclerView& view, Recycler& recycler) {
    mIsAttachedToWindow = false;
    onDetachedFromWindow(view, recycler);
}

bool RecyclerView::LayoutManager::isAttachedToWindow() {
    return mIsAttachedToWindow;
}

void RecyclerView::LayoutManager::postOnAnimation(Runnable& action) {
    if (mRecyclerView != nullptr) {
        mRecyclerView->postOnAnimation(action);
    }
}

bool RecyclerView::LayoutManager::removeCallbacks(Runnable& action) {
    if (mRecyclerView != nullptr) {
        return mRecyclerView->removeCallbacks(action);
    }
    return false;
}

void RecyclerView::LayoutManager::onAttachedToWindow(RecyclerView& view) {
}

void RecyclerView::LayoutManager::onDetachedFromWindow(RecyclerView& view) {

}

void RecyclerView::LayoutManager::onDetachedFromWindow(RecyclerView& view, Recycler& recycler) {
    onDetachedFromWindow(view);
}

bool RecyclerView::LayoutManager::getClipToPadding() {
    return mRecyclerView && mRecyclerView->mClipToPadding;
}

void RecyclerView::LayoutManager::onLayoutChildren(Recycler& recycler, State& state) {
    LOGE("You must override onLayoutChildren(Recycler recycler, State state) ");
}

void RecyclerView::LayoutManager::onLayoutCompleted(State& state) {
}


bool RecyclerView::LayoutManager::checkLayoutParams(const LayoutParams* lp)const{
    return lp != nullptr;
}

RecyclerView::LayoutParams* RecyclerView::LayoutManager::generateLayoutParams(const ViewGroup::LayoutParams& lp)const {
    if (dynamic_cast<const LayoutParams*>(&lp)) {
        return new LayoutParams((const LayoutParams&) lp);
    } else if (dynamic_cast<const ViewGroup::MarginLayoutParams*>(&lp)) {
        return new LayoutParams((const ViewGroup::MarginLayoutParams&) lp);
    }
    return new LayoutParams(lp);
}

RecyclerView::LayoutParams* RecyclerView::LayoutManager::generateLayoutParams(Context *c, const AttributeSet& attrs)const {
    return new LayoutParams(c, attrs);
}

int RecyclerView::LayoutManager::scrollHorizontallyBy(int dx, Recycler& recycler, State& state) {
    return 0;
}

int RecyclerView::LayoutManager::scrollVerticallyBy(int dy, Recycler& recycler, State& state) {
    return 0;
}

bool RecyclerView::LayoutManager::canScrollHorizontally() const{
    return false;
}

bool RecyclerView::LayoutManager::canScrollVertically() const{
    return false;
}

void RecyclerView::LayoutManager::scrollToPosition(int position) {
    LOGE("You MUST implement scrollToPosition. It will soon become abstract");
}

void RecyclerView::LayoutManager::smoothScrollToPosition(RecyclerView& recyclerView, State& state,
        int position) {
    LOGE("You must override smoothScrollToPosition to support smooth scrolling");
}
/**
 * Starts a smooth scroll using the provided {@link SmoothScroller}.
 *
 * <p>Each instance of SmoothScroller is intended to only be used once. Provide a new
 * SmoothScroller instance each time this method is called.
 *
 * <p>Calling this method will cancel any previous smooth scroll request.
 *
 * @param smoothScroller Instance which defines how smooth scroll should be animated
 */
void RecyclerView::LayoutManager::startSmoothScroll(SmoothScroller* smoothScroller) {
    if (mSmoothScroller && (smoothScroller != mSmoothScroller) && mSmoothScroller->isRunning()) {
        mSmoothScroller->stop();
    }
    delete mSmoothScroller;
    mSmoothScroller = smoothScroller;
    mSmoothScroller->start(mRecyclerView, this);
}

bool RecyclerView::LayoutManager::computeScrollVectorForPosition(int targetPosition,PointF& point){
    return false;
}

bool RecyclerView::LayoutManager::isSmoothScrolling() const{
    return mSmoothScroller && mSmoothScroller->isRunning();
}

int RecyclerView::LayoutManager::getLayoutDirection() const{
    return mRecyclerView->getLayoutDirection();
}

bool RecyclerView::LayoutManager::isLayoutReversed() const{
    return false;
}

void RecyclerView::LayoutManager::endAnimation(View* view) {
    if (mRecyclerView->mItemAnimator != nullptr) {
        mRecyclerView->mItemAnimator->endAnimation(*getChildViewHolderInt(view));
    }
}

void RecyclerView::LayoutManager::addDisappearingView(View* child) {
    addDisappearingView(child, -1);
}

void RecyclerView::LayoutManager::addDisappearingView(View* child, int index) {
    addViewInt(child, index, true);
}

void RecyclerView::LayoutManager::addView(View* child) {
    addView(child, -1);
}

void RecyclerView::LayoutManager::addView(View* child, int index) {
    addViewInt(child, index, false);
}

void RecyclerView::LayoutManager::addViewInt(View* child, int index, bool disappearing) {
    ViewHolder* holder = getChildViewHolderInt(child);
    if (disappearing || holder->isRemoved()) {
        // these views will be hidden at the end of the layout pass.
        mRecyclerView->mViewInfoStore->addToDisappearedInLayout(holder);
    } else {
        mRecyclerView->mViewInfoStore->removeFromDisappearedInLayout(holder);
    }
    RecyclerView::LayoutParams* lp = (RecyclerView::LayoutParams*) child->getLayoutParams();
    if (holder->wasReturnedFromScrap() || holder->isScrap()) {
        if (holder->isScrap()) {
            holder->unScrap();
        } else {
            holder->clearReturnedFromScrapFlag();
        }
        mChildHelper->attachViewToParent(child, index, child->getLayoutParams(), false);
        if (DISPATCH_TEMP_DETACH) {
            child->dispatchFinishTemporaryDetach();
        }
    } else if (child->getParent() == mRecyclerView) { // it was not a scrap but a valid child
        // ensure in correct position
        int currentIndex = mChildHelper->indexOfChild(child);
        if (index == -1) {
            index = mChildHelper->getChildCount();
        }
        if (currentIndex == -1) {
            LOGE("Added View has RecyclerView as parent but"
                 " view is not a real child. Unfiltered index:%d",
                 mRecyclerView->indexOfChild(child));
        }
        if (currentIndex != index) {
            mRecyclerView->mLayout->moveView(currentIndex, index);
        }
    } else {
        mChildHelper->addView(child, index, false);
        lp->mInsetsDirty = true;
        if (mSmoothScroller && mSmoothScroller->isRunning()) {
            mSmoothScroller->onChildAttachedToWindow(child);
        }
    }
    if (lp->mPendingInvalidate) {
        LOGD("consuming pending invalidate on child %p",lp->mViewHolder);
        holder->itemView->invalidate();
        lp->mPendingInvalidate = false;
    }
}

void RecyclerView::LayoutManager::removeView(View* child) {
    mChildHelper->removeView(child);
}

void RecyclerView::LayoutManager::removeViewAt(int index) {
    View* child = getChildAt(index);
    if (child != nullptr) {
        mChildHelper->removeViewAt(index);
    }
}

void RecyclerView::LayoutManager::removeAllViews() {
    // Only remove non-animating views
    const int childCount = getChildCount();
    for (int i = childCount - 1; i >= 0; i--) {
        mChildHelper->removeViewAt(i);
    }
}

int RecyclerView::LayoutManager::getBaseline() {
    return -1;
}

int RecyclerView::LayoutManager::getPosition(View* view) {
    return ((RecyclerView::LayoutParams*) view->getLayoutParams())->getViewLayoutPosition();
}

int RecyclerView::LayoutManager::getItemViewType(View* view) {
    return getChildViewHolderInt(view)->getItemViewType();
}

View*  RecyclerView::LayoutManager::findContainingItemView(View* view) {
    if (mRecyclerView == nullptr) {
        return nullptr;
    }
    View* found = mRecyclerView->findContainingItemView(view);
    if (found == nullptr) {
        return nullptr;
    }
    if (mChildHelper->isHidden(found)) {
        return nullptr;
    }
    return found;
}

View* RecyclerView::LayoutManager::findViewByPosition(int position) {
    const int childCount = getChildCount();
    for (int i = 0; i < childCount; i++) {
        View* child = getChildAt(i);
        ViewHolder* vh = getChildViewHolderInt(child);
        if (vh == nullptr) {
            continue;
        }
        if (vh->getLayoutPosition() == position && !vh->shouldIgnore()
                && (mRecyclerView->mState->isPreLayout() || !vh->isRemoved())) {
            return child;
        }
    }
    return nullptr;
}

void RecyclerView::LayoutManager::detachView(View* child) {
    const int ind = mChildHelper->indexOfChild(child);
    if (ind >= 0) {
        detachViewInternal(ind, child);
    }
}

void RecyclerView::LayoutManager::detachViewAt(int index) {
    detachViewInternal(index, getChildAt(index));
}

void RecyclerView::LayoutManager::detachViewInternal(int index,View* view) {
    if (DISPATCH_TEMP_DETACH) {
        view->dispatchStartTemporaryDetach();
    }
    mChildHelper->detachViewFromParent(index);
}

void RecyclerView::LayoutManager::attachView(View* child, int index, RecyclerView::LayoutParams* lp) {
    ViewHolder* vh = getChildViewHolderInt(child);
    if (vh->isRemoved()) {
        mRecyclerView->mViewInfoStore->addToDisappearedInLayout(vh);
    } else {
        mRecyclerView->mViewInfoStore->removeFromDisappearedInLayout(vh);
    }
    mChildHelper->attachViewToParent(child, index, lp, vh->isRemoved());
    if (DISPATCH_TEMP_DETACH)  {
        child->dispatchFinishTemporaryDetach();
    }
}

void RecyclerView::LayoutManager::attachView(View* child, int index) {
    attachView(child, index, (RecyclerView::LayoutParams*) child->getLayoutParams());
}

void RecyclerView::LayoutManager::attachView(View* child) {
    attachView(child, -1);
}

void RecyclerView::LayoutManager::removeDetachedView(View* child) {
    mRecyclerView->removeDetachedView(child, false);
}

void RecyclerView::LayoutManager::moveView(int fromIndex, int toIndex) {
    View* view = getChildAt(fromIndex);
    if (view == nullptr) {
        LOGE("Cannot move a child from non-existing index:%d",fromIndex/*+ mRecyclerView.toString()*/);
    }
    detachViewAt(fromIndex);
    attachView(view, toIndex);
}

void RecyclerView::LayoutManager::detachAndScrapView(View* child, Recycler& recycler) {
    const int index = mChildHelper->indexOfChild(child);
    scrapOrRecycleView(recycler, index, child);
}

void RecyclerView::LayoutManager::detachAndScrapViewAt(int index,Recycler& recycler) {
    View* child = getChildAt(index);
    scrapOrRecycleView(recycler, index, child);
}

void RecyclerView::LayoutManager::removeAndRecycleView(View* child, Recycler& recycler) {
    removeView(child);
    recycler.recycleView(child);
}

void RecyclerView::LayoutManager::removeAndRecycleViewAt(int index,Recycler& recycler) {
    View* view = getChildAt(index);
    removeViewAt(index);
    recycler.recycleView(view);
}

int RecyclerView::LayoutManager::getChildCount() const{
    return mChildHelper ? mChildHelper->getChildCount() : 0;
}

View* RecyclerView::LayoutManager::getChildAt(int index) {
    return mChildHelper ? mChildHelper->getChildAt(index) : nullptr;
}

int RecyclerView::LayoutManager::getWidthMode() const{
    return mWidthMode;
}

int RecyclerView::LayoutManager::getHeightMode() const{
    return mHeightMode;
}

int RecyclerView::LayoutManager::getWidth() const{
    return mWidth;
}

int RecyclerView::LayoutManager::getHeight() const{
    return mHeight;
}

int RecyclerView::LayoutManager::getPaddingLeft() {
    return mRecyclerView ? mRecyclerView->getPaddingLeft() : 0;
}

int RecyclerView::LayoutManager::getPaddingTop() {
    return mRecyclerView ? mRecyclerView->getPaddingTop() : 0;
}

int RecyclerView::LayoutManager::getPaddingRight() {
    return mRecyclerView ? mRecyclerView->getPaddingRight() : 0;
}

int RecyclerView::LayoutManager::getPaddingBottom() {
    return mRecyclerView ? mRecyclerView->getPaddingBottom() : 0;
}

int RecyclerView::LayoutManager::getPaddingStart() {
    return mRecyclerView ? mRecyclerView->getPaddingStart() : 0;
}

int RecyclerView::LayoutManager::getPaddingEnd() {
    return mRecyclerView ? mRecyclerView->getPaddingEnd() : 0;
}

bool RecyclerView::LayoutManager::isFocused() const{
    return mRecyclerView && mRecyclerView->isFocused();
}

bool RecyclerView::LayoutManager::hasFocus() const{
    return mRecyclerView && mRecyclerView->hasFocus();
}

View* RecyclerView::LayoutManager::getFocusedChild() {
    if (mRecyclerView == nullptr) {
        return nullptr;
    }
    View* focused = mRecyclerView->getFocusedChild();
    if ((focused == nullptr) || mChildHelper->isHidden(focused)) {
        return nullptr;
    }
    return focused;
}

int RecyclerView::LayoutManager::getItemCount() {
    Adapter* a = mRecyclerView ? mRecyclerView->getAdapter() : nullptr;
    return a ? a->getItemCount() : 0;
}

void RecyclerView::LayoutManager::offsetChildrenHorizontal(int dx) {
    if (mRecyclerView != nullptr) {
        mRecyclerView->offsetChildrenHorizontal(dx);
    }
}

void RecyclerView::LayoutManager::offsetChildrenVertical(int dy) {
    if (mRecyclerView != nullptr) {
        mRecyclerView->offsetChildrenVertical(dy);
    }
}

void RecyclerView::LayoutManager::ignoreView(View* view) {
    if (view->getParent() != mRecyclerView || mRecyclerView->indexOfChild(view) == -1) {
        // checking this because calling this method on a recycled or detached view may
        // cause loss of state.
        LOGE("View should be fully attached to be ignored mRecyclerView.exceptionLabel()");
    }
    ViewHolder* vh = getChildViewHolderInt(view);
    vh->addFlags(ViewHolder::FLAG_IGNORE);
    mRecyclerView->mViewInfoStore->removeViewHolder(vh);
}

void RecyclerView::LayoutManager::stopIgnoringView(View* view) {
    ViewHolder* vh = getChildViewHolderInt(view);
    vh->stopIgnoring();
    vh->resetInternal();
    vh->addFlags(ViewHolder::FLAG_INVALID);
}

void RecyclerView::LayoutManager::detachAndScrapAttachedViews(Recycler& recycler) {
    const size_t childCount = getChildCount();
    for (int i = int(childCount - 1); i >= 0; i--) {
        View* v = getChildAt(i);
        scrapOrRecycleView(recycler, i, v);
    }
}

void RecyclerView::LayoutManager::scrapOrRecycleView(Recycler& recycler, int index, View* view) {
    ViewHolder* viewHolder = getChildViewHolderInt(view);
    if (viewHolder->shouldIgnore()) {
        LOGD("ignoring view %p",viewHolder);
        return;
    }
    if (viewHolder->isInvalid() && !viewHolder->isRemoved()
            && !mRecyclerView->mAdapter->hasStableIds()) {
        removeViewAt(index);
        recycler.recycleViewHolderInternal(*viewHolder);
    } else {
        detachViewAt(index);
        recycler.scrapView(view);
        mRecyclerView->mViewInfoStore->onViewDetached(viewHolder);
    }
}

void RecyclerView::LayoutManager::removeAndRecycleScrapInt(Recycler& recycler) {
    const int scrapCount = recycler.getScrapCount();
    // Loop backward, recycler might be changed by removeDetachedView()
    for (int i = scrapCount - 1; i >= 0; i--) {
        View* scrap = recycler.getScrapViewAt(i);
        ViewHolder* vh = getChildViewHolderInt(scrap);
        if (vh->shouldIgnore()) {
            continue;
        }
        vh->setIsRecyclable(false);
        if (vh->isTmpDetached()) {
            mRecyclerView->removeDetachedView(scrap, false);
        }
        if (mRecyclerView->mItemAnimator != nullptr) {
            mRecyclerView->mItemAnimator->endAnimation(*vh);
        }
        vh->setIsRecyclable(true);
        recycler.quickRecycleScrapView(scrap);
    }
    recycler.clearScrap();
    if (scrapCount > 0) {
        mRecyclerView->invalidate();
    }
}


void RecyclerView::LayoutManager::measureChild(View* child, int widthUsed, int heightUsed) {
    LayoutParams* lp = (LayoutParams*) child->getLayoutParams();

    Rect insets = mRecyclerView->getItemDecorInsetsForChild(child);
    widthUsed += insets.left + insets.width;//insets.right
    heightUsed += insets.top + insets.height;//insets.bottom
    const int widthSpec = getChildMeasureSpec(getWidth(), getWidthMode(),
            getPaddingLeft() + getPaddingRight() + widthUsed, lp->width,
            canScrollHorizontally());
    const int heightSpec = getChildMeasureSpec(getHeight(), getHeightMode(),
            getPaddingTop() + getPaddingBottom() + heightUsed, lp->height,
            canScrollVertically());
    if (shouldMeasureChild(child, widthSpec, heightSpec, lp)) {
        child->measure(widthSpec, heightSpec);
    }
}

bool RecyclerView::LayoutManager::shouldReMeasureChild(View* child, int widthSpec, int heightSpec,const LayoutParams* lp) {
    return !mMeasurementCacheEnabled
            || !isMeasurementUpToDate(child->getMeasuredWidth(), widthSpec, lp->width)
            || !isMeasurementUpToDate(child->getMeasuredHeight(), heightSpec, lp->height);
}

bool RecyclerView::LayoutManager::shouldMeasureChild(View* child, int widthSpec, int heightSpec, const LayoutParams* lp) {
    return child->isLayoutRequested()
            || !mMeasurementCacheEnabled
            || !isMeasurementUpToDate(child->getWidth(), widthSpec, lp->width)
            || !isMeasurementUpToDate(child->getHeight(), heightSpec, lp->height);
}

bool RecyclerView::LayoutManager::isMeasurementCacheEnabled() const{
    return mMeasurementCacheEnabled;
}

void RecyclerView::LayoutManager::setMeasurementCacheEnabled(bool measurementCacheEnabled) {
    mMeasurementCacheEnabled = measurementCacheEnabled;
}

bool RecyclerView::LayoutManager::isMeasurementUpToDate(int childSize, int spec, int dimension) {
    const int specMode = MeasureSpec::getMode(spec);
    const int specSize = MeasureSpec::getSize(spec);
    if (dimension > 0 && childSize != dimension) {
        return false;
    }
    switch (specMode) {
    case MeasureSpec::UNSPECIFIED: return true;
    case MeasureSpec::AT_MOST:     return specSize >= childSize;
    case MeasureSpec::EXACTLY:     return  specSize == childSize;
    }
    return false;
}

void RecyclerView::LayoutManager::measureChildWithMargins(View* child, int widthUsed, int heightUsed) {
    const LayoutParams* lp = (LayoutParams*) child->getLayoutParams();

    const Rect insets = mRecyclerView->getItemDecorInsetsForChild(child);
    widthUsed += insets.left + insets.width;//right;
    heightUsed += insets.top + insets.height;//bottom;

    const int widthSpec = getChildMeasureSpec(getWidth(), getWidthMode(),
            getPaddingLeft() + getPaddingRight()
                    + lp->leftMargin + lp->rightMargin + widthUsed, lp->width,
            canScrollHorizontally());
    const int heightSpec = getChildMeasureSpec(getHeight(), getHeightMode(),
            getPaddingTop() + getPaddingBottom()
                    + lp->topMargin + lp->bottomMargin + heightUsed, lp->height,
            canScrollVertically());
    if (shouldMeasureChild(child, widthSpec, heightSpec, lp)) {
        child->measure(widthSpec, heightSpec);
    }
}

int RecyclerView::LayoutManager::getChildMeasureSpec(int parentSize, int padding, int childDimension,
        bool canScroll) {
    int size = std::max(0, parentSize - padding);
    int resultSize = 0;
    int resultMode = 0;
    if (canScroll) {
        if (childDimension >= 0) {
            resultSize = childDimension;
            resultMode = MeasureSpec::EXACTLY;
        } else {
            // MATCH_PARENT can't be applied since we can scroll in this dimension, wrap
            // instead using UNSPECIFIED.
            resultSize = 0;
            resultMode = MeasureSpec::UNSPECIFIED;
        }
    } else {
        if (childDimension >= 0) {
            resultSize = childDimension;
            resultMode = MeasureSpec::EXACTLY;
        } else if (childDimension == LayoutParams::MATCH_PARENT) {
            resultSize = size;
            // TODO this should be my spec.
            resultMode = MeasureSpec::EXACTLY;
        } else if (childDimension == LayoutParams::WRAP_CONTENT) {
            resultSize = size;
            resultMode = MeasureSpec::AT_MOST;
        }
    }
    return MeasureSpec::makeMeasureSpec(resultSize, resultMode);
}

int RecyclerView::LayoutManager::getChildMeasureSpec(int parentSize, int parentMode, int padding,
        int childDimension, bool canScroll) {
    int size = std::max(0, parentSize - padding);
    int resultSize = 0;
    int resultMode = 0;
    if (canScroll) {
        if (childDimension >= 0) {
            resultSize = childDimension;
            resultMode = MeasureSpec::EXACTLY;
        } else if (childDimension == LayoutParams::MATCH_PARENT) {
            switch (parentMode) {
            case MeasureSpec::AT_MOST:
	    case MeasureSpec::EXACTLY:
                    resultSize = size;
                    resultMode = parentMode;
                    break;
	    case MeasureSpec::UNSPECIFIED:
                    resultSize = 0;
                    resultMode = MeasureSpec::UNSPECIFIED;
                    break;
            }
        } else if (childDimension == LayoutParams::WRAP_CONTENT) {
            resultSize = 0;
            resultMode = MeasureSpec::UNSPECIFIED;
        }
    } else {
        if (childDimension >= 0) {
            resultSize = childDimension;
            resultMode = MeasureSpec::EXACTLY;
        } else if (childDimension == LayoutParams::MATCH_PARENT) {
            resultSize = size;
            resultMode = parentMode;
        } else if (childDimension == LayoutParams::WRAP_CONTENT) {
            resultSize = size;
            if (parentMode == MeasureSpec::AT_MOST || parentMode == MeasureSpec::EXACTLY) {
                resultMode = MeasureSpec::AT_MOST;
            } else {
                resultMode = MeasureSpec::UNSPECIFIED;
            }

        }
    }
    //noinspection WrongConstant
    return MeasureSpec::makeMeasureSpec(resultSize, resultMode);
}

int RecyclerView::LayoutManager::getDecoratedMeasuredWidth(View* child) {
    Rect& insets = ((LayoutParams*) child->getLayoutParams())->mDecorInsets;
    return child->getMeasuredWidth() + insets.left + insets.width;//right;
}

int RecyclerView::LayoutManager::getDecoratedMeasuredHeight(View* child) {
    Rect& insets = ((LayoutParams*) child->getLayoutParams())->mDecorInsets;
    return child->getMeasuredHeight() + insets.top + insets.height;//bottom;
}

void RecyclerView::LayoutManager::layoutDecorated(View* child, int left, int top, int width, int height) {
    Rect& insets = ((LayoutParams*) child->getLayoutParams())->mDecorInsets;
    child->layout(left + insets.left, top + insets.top, width -insets.left - insets.width,
            height - insets.top - insets.height);
}

void RecyclerView::LayoutManager::layoutDecoratedWithMargins(View* child, int left, int top, int width,
        int height) {
    const LayoutParams* lp = (LayoutParams*) child->getLayoutParams();
    const Rect& insets = lp->mDecorInsets;
    child->layout(left + insets.left + lp->leftMargin, top + insets.top + lp->topMargin,
            width - insets.left - insets.width - lp->leftMargin- lp->rightMargin,
            height - insets.top - insets.height - lp->topMargin - lp->bottomMargin);
}

void RecyclerView::LayoutManager::getTransformedBoundingBox(View* child, bool includeDecorInsets,
        Rect& out) {
    if (includeDecorInsets) {
        Rect& insets = ((LayoutParams*) child->getLayoutParams())->mDecorInsets;
        out.set(-insets.left, -insets.top,
            child->getWidth() + insets.left + insets.width,
            child->getHeight() + insets.top + insets.height);
    } else {
        out.set(0, 0, child->getWidth(), child->getHeight());
    }

    if (mRecyclerView != nullptr) {
        if (!child->hasIdentityMatrix()) {
            Cairo::Matrix childMatrix = child->getMatrix();
            RectF& tempRectF = mRecyclerView->mTempRectF;
            Cairo::Rectangle recd={double(out.left),double(out.top),
                double(out.width),double(out.height)};
            childMatrix.transform_rectangle(recd);
            out.set((int) std::floor(recd.x),(int) std::floor(recd.y),
                 (int) std::ceil(recd.width),(int) std::ceil(recd.height) );
            tempRectF.set(static_cast<float>(recd.x),static_cast<float>(recd.y),
                static_cast<float>(recd.width),static_cast<float>(recd.height));
        }
    }
    out.offset(child->getLeft(), child->getTop());
}

void RecyclerView::LayoutManager::getDecoratedBoundsWithMargins(View* view,Rect& outBounds) const{
    RecyclerView::getDecoratedBoundsWithMarginsInt(view, outBounds);
}

int RecyclerView::LayoutManager::getDecoratedLeft(View* child) const{
    return child->getLeft() - getLeftDecorationWidth(child);
}

int RecyclerView::LayoutManager::getDecoratedTop(View* child) const{
    return child->getTop() - getTopDecorationHeight(child);
}

int RecyclerView::LayoutManager::getDecoratedRight(View* child) const{
    return child->getRight() + getRightDecorationWidth(child);
}

int RecyclerView::LayoutManager::getDecoratedBottom(View* child) const{
    return child->getBottom() + getBottomDecorationHeight(child);
}

void RecyclerView::LayoutManager::calculateItemDecorationsForChild(View* child,Rect& outRect) {
    if (mRecyclerView == nullptr) {
        outRect.set(0, 0, 0, 0);
        return;
    }
    Rect insets = mRecyclerView->getItemDecorInsetsForChild(child);
    outRect = insets;//.set(insets);
}

int RecyclerView::LayoutManager::getTopDecorationHeight(View* child) const{
    return ((LayoutParams*) child->getLayoutParams())->mDecorInsets.top;
}

int RecyclerView::LayoutManager::getBottomDecorationHeight(View* child) const{
    return ((LayoutParams*) child->getLayoutParams())->mDecorInsets.height;
}

int RecyclerView::LayoutManager::getLeftDecorationWidth(View* child) const{
    return ((LayoutParams*) child->getLayoutParams())->mDecorInsets.left;
}

int RecyclerView::LayoutManager::getRightDecorationWidth(View* child) const{
    return ((LayoutParams*) child->getLayoutParams())->mDecorInsets.width;
}

View* RecyclerView::LayoutManager::onFocusSearchFailed(View* focused, int direction,Recycler& recycler,State& state) {
    return nullptr;
}

View* RecyclerView::LayoutManager::onInterceptFocusSearch(View* focused, int direction) {
    return nullptr;
}

void RecyclerView::LayoutManager::getChildRectangleOnScreenScrollAmount(View& child,const Rect& rect,int out[2]) {
    const int parentLeft = getPaddingLeft();
    const int parentTop = getPaddingTop();
    const int parentRight = getWidth() - getPaddingRight();
    const int parentBottom = getHeight() - getPaddingBottom();
    const int childLeft = child.getLeft() + rect.left - child.getScrollX();
    const int childTop = child.getTop() + rect.top - child.getScrollY();
    const int childRight = childLeft + rect.width;
    const int childBottom = childTop + rect.height;

    const int offScreenLeft = std::min(0, childLeft - parentLeft);
    const int offScreenTop = std::min(0, childTop - parentTop);
    const int offScreenRight = std::max(0, childRight - parentRight);
    const int offScreenBottom = std::max(0, childBottom - parentBottom);

    // Favor the "start" layout direction over the end when bringing one side or the other
    // of a large rect into view. If we decide to bring in end because start is already
    // visible, limit the scroll such that start won't go out of bounds.
    int dx;
    if (getLayoutDirection() == View::LAYOUT_DIRECTION_RTL) {
        dx = offScreenRight != 0 ? offScreenRight
                : std::max(offScreenLeft, childRight - parentRight);
    } else {
        dx = offScreenLeft != 0 ? offScreenLeft
                : std::min(childLeft - parentLeft, offScreenRight);
    }

    // Favor bringing the top into view over the bottom. If top is already visible and
    // we should scroll to make bottom visible, make sure top does not go out of bounds.
    const int dy = offScreenTop != 0 ? offScreenTop
            : std::min(childTop - parentTop, offScreenBottom);
    out[0] = dx;
    out[1] = dy;
}

bool RecyclerView::LayoutManager::requestChildRectangleOnScreen(RecyclerView& parent,
        View& child,const Rect& rect, bool immediate) {
    return requestChildRectangleOnScreen(parent, child, rect, immediate, false);
}

bool RecyclerView::LayoutManager::requestChildRectangleOnScreen(RecyclerView& parent,
        View& child,const Rect& rect, bool immediate,bool focusedChildVisible) {
    int dx,dy,scrollAmount[2];
    getChildRectangleOnScreenScrollAmount(child, rect,scrollAmount);
    dx = scrollAmount[0];
    dy = scrollAmount[1];
    if (!focusedChildVisible || isFocusedChildVisibleAfterScrolling(parent, dx, dy)) {
        if (dx != 0 || dy != 0) {
            if (immediate) {
                parent.scrollBy(dx, dy);
            } else {
                parent.smoothScrollBy(dx, dy);
            }
            return true;
        }
    }
    return false;
}

bool RecyclerView::LayoutManager::isViewPartiallyVisible(View* child, bool completelyVisible,
        bool acceptEndPointInclusion) {
    const int boundsFlag = (ViewBoundsCheck::FLAG_CVS_GT_PVS | ViewBoundsCheck::FLAG_CVS_EQ_PVS
            | ViewBoundsCheck::FLAG_CVE_LT_PVE | ViewBoundsCheck::FLAG_CVE_EQ_PVE);
    const bool isViewFullyVisible = mHorizontalBoundCheck->isViewWithinBoundFlags(child,boundsFlag)
            && mVerticalBoundCheck->isViewWithinBoundFlags(child, boundsFlag);
    if (completelyVisible) {
        return isViewFullyVisible;
    } else {
        return !isViewFullyVisible;
    }
}

bool RecyclerView::LayoutManager::isFocusedChildVisibleAfterScrolling(RecyclerView& parent, int dx, int dy) {
    View* focusedChild = parent.getFocusedChild();
    if (focusedChild == nullptr) {
        return false;
    }
    const int parentLeft = getPaddingLeft();
    const int parentTop = getPaddingTop();
    const int parentRight = getWidth() - getPaddingRight();
    const int parentBottom = getHeight() - getPaddingBottom();
    Rect& bounds = mRecyclerView->mTempRect;
    getDecoratedBoundsWithMargins(focusedChild, bounds);

    if ((bounds.left - dx >= parentRight) || (bounds.right() - dx <= parentLeft)
            || (bounds.top - dy >= parentBottom) || (bounds.bottom() - dy <= parentTop) ){
        return false;
    }
    return true;
}

bool RecyclerView::LayoutManager::onRequestChildFocus(RecyclerView& parent, View& child,
        View* focused) {
    // eat the request if we are in the middle of a scroll or layout
    return isSmoothScrolling() || parent.isComputingLayout();
}

bool RecyclerView::LayoutManager::onRequestChildFocus(RecyclerView& parent, State& state,
        View& child, View* focused) {
    return onRequestChildFocus(parent, child, focused);
}

void RecyclerView::LayoutManager::onAdapterChanged(Adapter* oldAdapter, Adapter* newAdapter) {
}

bool RecyclerView::LayoutManager::onAddFocusables(RecyclerView& recyclerView,
        std::vector<View*>& views, int direction, int focusableMode) {
    return false;
}

void RecyclerView::LayoutManager::onItemsChanged(RecyclerView& recyclerView) {
}

void RecyclerView::LayoutManager::onItemsAdded(RecyclerView& recyclerView, int positionStart,
        int itemCount) {
}

void RecyclerView::LayoutManager::onItemsRemoved(RecyclerView& recyclerView, int positionStart,
        int itemCount) {
}

void RecyclerView::LayoutManager::onItemsUpdated(RecyclerView& recyclerView, int positionStart,
        int itemCount) {
}

void RecyclerView::LayoutManager::onItemsUpdated(RecyclerView& recyclerView, int positionStart,
        int itemCount,Object* payload) {
    onItemsUpdated(recyclerView, positionStart, itemCount);
}

void RecyclerView::LayoutManager::onItemsMoved(RecyclerView& recyclerView, int from, int to,
        int itemCount) {
}

int RecyclerView::LayoutManager::computeHorizontalScrollExtent(State& state) {
    return 0;
}

int RecyclerView::LayoutManager::computeHorizontalScrollOffset(State& state) {
    return 0;
}

int RecyclerView::LayoutManager::computeHorizontalScrollRange(State& state) {
    return 0;
}

int RecyclerView::LayoutManager::computeVerticalScrollExtent(State& state) {
    return 0;
}

int RecyclerView::LayoutManager::computeVerticalScrollOffset(State& state) {
    return 0;
}

int RecyclerView::LayoutManager::computeVerticalScrollRange(State& state) {
    return 0;
}

void RecyclerView::LayoutManager::onMeasure(Recycler& recycler, State& state, int widthSpec,int heightSpec) {
    mRecyclerView->defaultOnMeasure(widthSpec, heightSpec);
}

bool RecyclerView::LayoutManager::prepareForDrop(View* view,View* target, int x, int y){
    return false;
}

void RecyclerView::LayoutManager::setMeasuredDimension(int widthSize, int heightSize) {
    mRecyclerView->setMeasuredDimension(widthSize, heightSize);
}

int RecyclerView::LayoutManager::getMinimumWidth() {
    return mRecyclerView->getMinimumWidth();
}

int RecyclerView::LayoutManager::getMinimumHeight() {
    return mRecyclerView->getMinimumHeight();
}

Parcelable* RecyclerView::LayoutManager::onSaveInstanceState() {
    return new Parcelable();
}

void RecyclerView::LayoutManager::onRestoreInstanceState(Parcelable& state) {

}

void RecyclerView::LayoutManager::stopSmoothScroller() {
    if (mSmoothScroller != nullptr) {
        mSmoothScroller->stop();
    }
}

void RecyclerView::LayoutManager::onSmoothScrollerStopped(SmoothScroller* smoothScroller) {
    if (mSmoothScroller == smoothScroller) {
        delete mSmoothScroller;
        mSmoothScroller = nullptr;
    }
}

void RecyclerView::LayoutManager::onScrollStateChanged(int state) {
}

void RecyclerView::LayoutManager::removeAndRecycleAllViews(Recycler& recycler) {
    for (int i = getChildCount() - 1; i >= 0; i--) {
        View* view = getChildAt(i);
        if (!getChildViewHolderInt(view)->shouldIgnore()) {
            removeAndRecycleViewAt(i, recycler);
        }
    }
}

// called by accessibility delegate
void RecyclerView::LayoutManager::onInitializeAccessibilityNodeInfo(AccessibilityNodeInfo& info) {
    onInitializeAccessibilityNodeInfo(*mRecyclerView->mRecycler, *mRecyclerView->mState, info);
}

void RecyclerView::LayoutManager::onInitializeAccessibilityNodeInfo(Recycler& recycler,
        State& state, AccessibilityNodeInfo& info) {
    if (mRecyclerView->canScrollVertically(-1) || mRecyclerView->canScrollHorizontally(-1)) {
        info.addAction(AccessibilityNodeInfo::ACTION_SCROLL_BACKWARD);
        info.setScrollable(true);
        //info.setGranularScrollingSupported(true);
    }
    if (mRecyclerView->canScrollVertically(1) || mRecyclerView->canScrollHorizontally(1)) {
        info.addAction(AccessibilityNodeInfo::ACTION_SCROLL_FORWARD);
        info.setScrollable(true);
        //info.setGranularScrollingSupported(true);
    }
    AccessibilityNodeInfo::CollectionInfo* collectionInfo =
            AccessibilityNodeInfo::CollectionInfo::obtain(getRowCountForAccessibility(recycler, state),
                 getColumnCountForAccessibility(recycler, state),
                 isLayoutHierarchical(recycler, state),
                 getSelectionModeForAccessibility(recycler, state));
    info.setCollectionInfo(collectionInfo);
}

// called by accessibility delegate
void RecyclerView::LayoutManager::onInitializeAccessibilityEvent(AccessibilityEvent& event) {
    onInitializeAccessibilityEvent(*mRecyclerView->mRecycler, *mRecyclerView->mState, event);
}

void RecyclerView::LayoutManager::onInitializeAccessibilityEvent(Recycler& recycler, State& state,
        AccessibilityEvent& event) {
    if (mRecyclerView == nullptr /*|| event == nullptr*/) {
        return;
    }
    event.setScrollable(mRecyclerView->canScrollVertically(1)
            || mRecyclerView->canScrollVertically(-1)
            || mRecyclerView->canScrollHorizontally(-1)
            || mRecyclerView->canScrollHorizontally(1));

    if (mRecyclerView->mAdapter != nullptr) {
        event.setItemCount(mRecyclerView->mAdapter->getItemCount());
    }
}

// called by accessibility delegate
void RecyclerView::LayoutManager::onInitializeAccessibilityNodeInfoForItem(View* host, AccessibilityNodeInfo& info) {
    ViewHolder* vh = getChildViewHolderInt(host);
    // avoid trying to create accessibility node info for removed children
    if (vh && !vh->isRemoved() && !mChildHelper->isHidden(vh->itemView)) {
        onInitializeAccessibilityNodeInfoForItem(*mRecyclerView->mRecycler,
                *mRecyclerView->mState, host, info);
    }
}

void RecyclerView::LayoutManager::onInitializeAccessibilityNodeInfoForItem(Recycler& recycler,
        State& state, View* host, AccessibilityNodeInfo& info) {
    int rowIndexGuess = canScrollVertically() ? getPosition(host) : 0;
    int columnIndexGuess = canScrollHorizontally() ? getPosition(host) : 0;
    AccessibilityNodeInfo::CollectionItemInfo* itemInfo =
            AccessibilityNodeInfo::CollectionItemInfo::obtain(rowIndexGuess, 1,columnIndexGuess, 1, false, false);
    info.setCollectionItemInfo(itemInfo);
}

void RecyclerView::LayoutManager::requestSimpleAnimationsInNextLayout() {
    mRequestedSimpleAnimations = true;
}

int RecyclerView::LayoutManager::getSelectionModeForAccessibility(Recycler& recycler,
        State& state) {
    return 0;//AccessibilityNodeInfo::CollectionInfo::SELECTION_MODE_NONE;
}

int RecyclerView::LayoutManager::getRowCountForAccessibility(Recycler& recycler, State& state) {
    if ((mRecyclerView == nullptr) || (mRecyclerView->mAdapter == nullptr)) {
        return 1;
    }
    return canScrollVertically() ? mRecyclerView->mAdapter->getItemCount() : 1;
}

int RecyclerView::LayoutManager::getColumnCountForAccessibility(Recycler& recycler,
        State& state) {
    if ((mRecyclerView == nullptr) || (mRecyclerView->mAdapter == nullptr)) {
        return 1;
    }
    return canScrollHorizontally() ? mRecyclerView->mAdapter->getItemCount() : 1;
}

bool RecyclerView::LayoutManager::isLayoutHierarchical(Recycler& recycler,State& state) {
    return false;
}

// called by accessibility delegate
bool RecyclerView::LayoutManager::performAccessibilityAction(int action, Bundle* args) {
    return performAccessibilityAction(*mRecyclerView->mRecycler, *mRecyclerView->mState, action, args);
}

static int floatCompare(float a, float b) {
    if (std::isnan(a)) {
        if (std::isnan(b)) {
            return 0; // Both are NaN
        }
        return 1; // a is NaN, b is not
    } else if (std::isnan(b)) {
        return -1; // b is NaN, a is not
    }

    if (a < b) {
        return -1;
    } else if (a > b) {
        return 1;
    } else {
        return 0;
    }
}
bool RecyclerView::LayoutManager::performAccessibilityAction(Recycler& recycler, State& state, int action, Bundle* args) {
    if (mRecyclerView == nullptr) {
        return false;
    }
    int vScroll = 0, hScroll = 0;
    int height = getHeight();
    int width = getWidth();
    Rect rect;
    // Gets the visible rect on the screen except for the rotation or scale cases which
    // might affect the result.
    if (mRecyclerView->hasIdentityMatrix() && mRecyclerView->getGlobalVisibleRect(rect,nullptr)) {
        height = rect.height;
        width = rect.width;
    }
    switch (action) {
    case AccessibilityNodeInfo::ACTION_SCROLL_BACKWARD:
        if (mRecyclerView->canScrollVertically(-1)) {
            vScroll = -(height - getPaddingTop() - getPaddingBottom());
        }
        if (mRecyclerView->canScrollHorizontally(-1)) {
            hScroll = -(width - getPaddingLeft() - getPaddingRight());
        }
        break;
    case AccessibilityNodeInfo::ACTION_SCROLL_FORWARD:
        if (mRecyclerView->canScrollVertically(1)) {
            vScroll = height - getPaddingTop() - getPaddingBottom();
        }
        if (mRecyclerView->canScrollHorizontally(1)) {
            hScroll = width - getPaddingLeft() - getPaddingRight();
        }
        break;
    }
    if ((vScroll == 0) && (hScroll == 0)) {
        return false;
    }

    float granularScrollAmount = 1.F; // The default value.

    /*if (args != nullptr) {
        granularScrollAmount = args.getFloat(AccessibilityNodeInfo::ACTION_ARGUMENT_SCROLL_AMOUNT_FLOAT, 1.F);
        if (granularScrollAmount < 0) {
            if (sDebugAssertionsEnabled) {
                throw new IllegalArgumentException(
                        "attempting to use ACTION_ARGUMENT_SCROLL_AMOUNT_FLOAT with a "
                                + "negative value (" + granularScrollAmount + ")");
            }
            return false;
        }
    }*/

    if (floatCompare(granularScrollAmount, INFINITY) == 0) {
        // Assume that the client wants to scroll as far as possible. For
        // ACTION_SCROLL_BACKWARD, this means scrolling to the beginning of the collection.
        // For ACTION_SCROLL_FORWARD, this means scrolling to the end of the collection.

        if (mRecyclerView->mAdapter == nullptr) {
            return false;
        }
        switch (action) {
        case AccessibilityNodeInfo::ACTION_SCROLL_BACKWARD:
            mRecyclerView->smoothScrollToPosition(0);
            break;
        case AccessibilityNodeInfo::ACTION_SCROLL_FORWARD:
            mRecyclerView->smoothScrollToPosition(mRecyclerView->mAdapter->getItemCount() - 1);
                break;
        }
        return true;
    }
    // No adjustments needed to scroll values if granular scroll amount is 1F, which is
    // the default, or 0F, which is undefined.
    if (floatCompare(1.F, granularScrollAmount) != 0 && floatCompare(0.F, granularScrollAmount) != 0) {
        hScroll = (int) (hScroll * granularScrollAmount);
        vScroll = (int) (vScroll * granularScrollAmount);
    }
    mRecyclerView->smoothScrollBy(hScroll, vScroll,nullptr,UNDEFINED_DURATION,true);
    return true;
}

// called by accessibility delegate
bool RecyclerView::LayoutManager::performAccessibilityActionForItem(View& view, int action, Bundle* args) {
    return performAccessibilityActionForItem(*mRecyclerView->mRecycler, *mRecyclerView->mState,view, action, args);
}

bool RecyclerView::LayoutManager::performAccessibilityActionForItem(Recycler& recycler, State& state, View& view, int action, Bundle* args) {
    return false;
}


RecyclerView::LayoutManager::Properties RecyclerView::LayoutManager::getProperties(Context* context,const AttributeSet& attrs,int defStyleAttr, int defStyleRes) {
    Properties properties;
    //TypedArray a = context.obtainStyledAttributes(attrs, R.styleable.RecyclerView, defStyleAttr, defStyleRes);
    properties.orientation = attrs.getInt("orientation",std::unordered_map<std::string,int>{
            {"horizontal",LinearLayout::HORIZONTAL},
            {"vertical",LinearLayout::VERTICAL}}, DEFAULT_ORIENTATION);//a.getInt(R.styleable.RecyclerView_android_orientation, DEFAULT_ORIENTATION);
    properties.spanCount = attrs.getInt("spanCount",1);//a.getInt(R.styleable.RecyclerView_spanCount, 1);
    properties.reverseLayout = attrs.getBoolean("reverseLayout",false);//a.getBoolean(R.styleable.RecyclerView_reverseLayout, false);
    properties.stackFromEnd = attrs.getBoolean("stackFromEnd",false);//a.getBoolean(R.styleable.RecyclerView_stackFromEnd, false);
    //a.recycle();
    return properties;
}

void RecyclerView::LayoutManager::setExactMeasureSpecsFrom(RecyclerView* recyclerView) {
    setMeasureSpecs(
            MeasureSpec::makeMeasureSpec(recyclerView->getWidth(), MeasureSpec::EXACTLY),
            MeasureSpec::makeMeasureSpec(recyclerView->getHeight(), MeasureSpec::EXACTLY)
    );
}

bool RecyclerView::LayoutManager::shouldMeasureTwice() {
    return false;
}

bool RecyclerView::LayoutManager::hasFlexibleChildInBothOrientations() {
    const int childCount = getChildCount();
    for (int i = 0; i < childCount; i++) {
        View* child = getChildAt(i);
        ViewGroup::LayoutParams* lp = child->getLayoutParams();
        if (lp->width < 0 && lp->height < 0) {
            return true;
        }
    }
    return false;
}

/////////////////////////////////RecyclerView::ItemDecoration/////////////////////////////////

void RecyclerView::ItemDecoration::onDraw(Canvas& c,RecyclerView& parent,State& state) {
    //onDraw(c, parent);
}


void RecyclerView::ItemDecoration::onDrawOver(Canvas& c,RecyclerView& parent,State& state) {
    //onDrawOver(c, parent);
}

void RecyclerView::ItemDecoration::getItemOffsets(Rect& outRect, View& view,RecyclerView& parent, State& state) {
    outRect.set(0,0,0,0);
    //getItemOffsets(outRect, ((LayoutParams*) view.getLayoutParams())->getViewLayoutPosition(),parent);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

/*public static class SimpleOnItemTouchListener implements RecyclerView.OnItemTouchListener {
    public bool onInterceptTouchEvent(RecyclerView& rv,MotionEvent& e){ return false; }
    public void onTouchEvent(RecyclerView& rv,MotionEvent& e){}
    public void onRequestDisallowInterceptTouchEvent(bool disallowIntercept) {}
}*/

////////////////////////////////////////////////Beginning of ViewHolder///////////////////////////////////////
std::vector<Object*> RecyclerView::ViewHolder::FULLUPDATE_PAYLOADS;
RecyclerView::ViewHolder::ViewHolder(View* itemView) {
    FATAL_IF(itemView == nullptr,"itemView may not be null");
    this->itemView = itemView;
    mFlags = 0;
    mIsRecyclableCount = 0;
    mInChangeScrap = false;
    mPosition = NO_POSITION;
    mOldPosition = NO_POSITION;
    mPreLayoutPosition = NO_POSITION;
    mItemId = NO_ID;
    mItemViewType = INVALID_TYPE;
    mScrapContainer = nullptr;
    mShadowedHolder = nullptr;
    mShadowingHolder= nullptr;
    mOwnerRecyclerView = nullptr;
    mNestedRecyclerView= nullptr;
    mBindingAdapter = nullptr;
}

RecyclerView::ViewHolder::~ViewHolder(){
    delete itemView;
}

void RecyclerView::ViewHolder::flagRemovedAndOffsetPosition(int newPosition, int offset, bool applyToPreLayout) {
    addFlags(ViewHolder::FLAG_REMOVED);
    offsetPosition(offset, applyToPreLayout);
    mPosition = newPosition;
}

void RecyclerView::ViewHolder::offsetPosition(int offset, bool applyToPreLayout) {
    if (mOldPosition == NO_POSITION) {
        mOldPosition = mPosition;
    }
    if (mPreLayoutPosition == NO_POSITION) {
        mPreLayoutPosition = mPosition;
    }
    if (applyToPreLayout) {
        mPreLayoutPosition += offset;
    }
    mPosition += offset;
    if (itemView->getLayoutParams() != nullptr) {
        ((LayoutParams*) itemView->getLayoutParams())->mInsetsDirty = true;
    }
}

void RecyclerView::ViewHolder::clearOldPosition() {
    mOldPosition = NO_POSITION;
    mPreLayoutPosition = NO_POSITION;
}

void RecyclerView::ViewHolder::saveOldPosition() {
    if (mOldPosition == NO_POSITION) {
        mOldPosition = mPosition;
    }
}

bool RecyclerView::ViewHolder::shouldIgnore() const{
    return (mFlags & FLAG_IGNORE) != 0;
}

/*int RecyclerView::ViewHolder::getPosition() const{
    return mPreLayoutPosition == NO_POSITION ? mPosition : mPreLayoutPosition;
}*/

int RecyclerView::ViewHolder::getLayoutPosition()const{
    return mPreLayoutPosition == NO_POSITION ? mPosition : mPreLayoutPosition;
}

/*int RecyclerView::ViewHolder::getAdapterPosition(){
#if 0
    if (mOwnerRecyclerView == nullptr) {
        return NO_POSITION;
    }
    return mOwnerRecyclerView->getAdapterPositionFor(this);
#else
    return getBindingAdapterPosition();
#endif
}*/

int RecyclerView::ViewHolder::getBindingAdapterPosition(){
    if ((mBindingAdapter == nullptr)||(mOwnerRecyclerView == nullptr)) {
        return NO_POSITION;
    }
    Adapter*rvAdapter = mOwnerRecyclerView->getAdapter();
    if (rvAdapter == nullptr) {
        return NO_POSITION;
    }
    const int globalPosition = mOwnerRecyclerView->getAdapterPositionInRecyclerView(this);
    if (globalPosition == NO_POSITION) {
        return NO_POSITION;
    }
    return rvAdapter->findRelativeAdapterPositionIn(*mBindingAdapter,*this, globalPosition);
}

int RecyclerView::ViewHolder::getAbsoluteAdapterPosition(){
    if (mOwnerRecyclerView == nullptr) {
        return NO_POSITION;
    }
    return mOwnerRecyclerView->getAdapterPositionInRecyclerView(this);
}

RecyclerView::Adapter*RecyclerView::ViewHolder::getBindingAdapter()const{
    return mBindingAdapter;
}

int RecyclerView::ViewHolder::getOldPosition() const{
    return mOldPosition;
}

long RecyclerView::ViewHolder::getItemId() const{
    return mItemId;
}

int RecyclerView::ViewHolder::getItemViewType()const {
    return mItemViewType;
}

bool RecyclerView::ViewHolder::isScrap() const{
    return mScrapContainer != nullptr;
}

void RecyclerView::ViewHolder::unScrap() {
    mScrapContainer->unscrapView(*this);
}

bool RecyclerView::ViewHolder::wasReturnedFromScrap()const {
    return (mFlags & FLAG_RETURNED_FROM_SCRAP) != 0;
}

void RecyclerView::ViewHolder::clearReturnedFromScrapFlag() {
    mFlags = mFlags & ~FLAG_RETURNED_FROM_SCRAP;
}

void RecyclerView::ViewHolder::clearTmpDetachFlag() {
    mFlags = mFlags & ~FLAG_TMP_DETACHED;
}

void RecyclerView::ViewHolder::stopIgnoring() {
    mFlags = mFlags & ~FLAG_IGNORE;
}

void RecyclerView::ViewHolder::setScrapContainer(Recycler* recycler, bool isChangeScrap) {
    mScrapContainer = recycler;
    mInChangeScrap = isChangeScrap;
}

bool RecyclerView::ViewHolder::isInvalid() const{
    return (mFlags & FLAG_INVALID) != 0;
}

bool RecyclerView::ViewHolder::needsUpdate() const{
    return (mFlags & FLAG_UPDATE) != 0;
}

bool RecyclerView::ViewHolder::isBound() const{
    return (mFlags & FLAG_BOUND) != 0;
}

bool RecyclerView::ViewHolder::isRemoved() const{
    return (mFlags & FLAG_REMOVED) != 0;
}

bool RecyclerView::ViewHolder::hasAnyOfTheFlags(int flags) const{
    return (mFlags & flags) != 0;
}

bool RecyclerView::ViewHolder::isTmpDetached() const{
    return (mFlags & FLAG_TMP_DETACHED) != 0;
}

bool RecyclerView::ViewHolder::isAttachedToTransitionOverlay() const{
    return (itemView->getParent() != nullptr) && (itemView->getParent() != mOwnerRecyclerView);
}

bool RecyclerView::ViewHolder::isAdapterPositionUnknown() const{
    return (mFlags & FLAG_ADAPTER_POSITION_UNKNOWN) != 0 || isInvalid();
}

void RecyclerView::ViewHolder::setFlags(int flags, int mask) {
    mFlags = (mFlags & ~mask) | (flags & mask);
}

void RecyclerView::ViewHolder::addFlags(int flags) {
    mFlags |= flags;
}

void RecyclerView::ViewHolder::addChangePayload(Object* payload) {
    if (payload == nullptr) {
        addFlags(FLAG_ADAPTER_FULLUPDATE);
    } else if ((mFlags & FLAG_ADAPTER_FULLUPDATE) == 0) {
        createPayloadsIfNeeded();
        mPayloads.push_back(payload);
    }
}

void RecyclerView::ViewHolder::createPayloadsIfNeeded() {
    //if (mPayloads.empty() == null)
    {
        mUnmodifiedPayloads = &mPayloads;// Collections.unmodifiableList(mPayloads);
    }
}

void RecyclerView::ViewHolder::clearPayload() {
    mPayloads.clear();
    mFlags = mFlags & ~FLAG_ADAPTER_FULLUPDATE;
}

std::vector<Object*>* RecyclerView::ViewHolder::getUnmodifiedPayloads() {
    if ((mFlags & FLAG_ADAPTER_FULLUPDATE) == 0) {
        if ( mPayloads.size() == 0) {
            // Initial state,  no update being called.
            return &FULLUPDATE_PAYLOADS;
        }
        // there are none-null payloads
        return mUnmodifiedPayloads;
    } else {
        // a full update has been called.
        return &FULLUPDATE_PAYLOADS;
    }
}

void RecyclerView::ViewHolder::resetInternal() {
    if (/*sDebugAssertionsEnabled &&*/ isTmpDetached()) {
        throw std::logic_error("Attempting to reset temp-detached ViewHolder: "
                ". ViewHolders should be fully detached before resetting.");
    }
    mFlags = 0;
    mPosition = NO_POSITION;
    mOldPosition = NO_POSITION;
    mItemId = NO_ID;
    mPreLayoutPosition = NO_POSITION;
    mIsRecyclableCount = 0;
    mShadowedHolder = nullptr;
    mShadowingHolder = nullptr;
    clearPayload();
    mWasImportantForAccessibilityBeforeHidden = View::IMPORTANT_FOR_ACCESSIBILITY_AUTO;
    mPendingAccessibilityState = PENDING_ACCESSIBILITY_STATE_NOT_SET;
    clearNestedRecyclerViewIfNotNested(*this);
}

void RecyclerView::ViewHolder::onEnteredHiddenState(RecyclerView& parent) {
    // While the view item is in hidden state, make it invisible for the accessibility.
    if (mPendingAccessibilityState != PENDING_ACCESSIBILITY_STATE_NOT_SET) {
        mWasImportantForAccessibilityBeforeHidden = mPendingAccessibilityState;
    } else {
        mWasImportantForAccessibilityBeforeHidden = itemView->getImportantForAccessibility();
    }
    parent.setChildImportantForAccessibilityInternal(this, View::IMPORTANT_FOR_ACCESSIBILITY_NO_HIDE_DESCENDANTS);
}

void RecyclerView::ViewHolder::onLeftHiddenState(RecyclerView& parent) {
    parent.setChildImportantForAccessibilityInternal(this,mWasImportantForAccessibilityBeforeHidden);
    mWasImportantForAccessibilityBeforeHidden = View::IMPORTANT_FOR_ACCESSIBILITY_AUTO;
}


std::string RecyclerView::ViewHolder::toString() {
    std::ostringstream oss;
    oss<<"ViewHolder{ hashCode() position="<<mPosition;
    oss<<" id=" <<mItemId<<", oldPos=" <<mOldPosition <<", pLpos:" << mPreLayoutPosition;
    if (isScrap()) {
        oss<<" scrap "<<(mInChangeScrap ? "[changeScrap]" : "[attachedScrap]");
    }
    if (isInvalid()) oss<<" invalid";
    if (!isBound()) oss<<" unbound";
    if (needsUpdate()) oss<<" update";
    if (isRemoved()) oss<<" removed";
    if (shouldIgnore()) oss<<" ignored";
    if (isTmpDetached()) oss<<" tmpDetached";
    if (!isRecyclable()) oss<<" not recyclable(" << mIsRecyclableCount << ")";
    if (isAdapterPositionUnknown()) oss<<" undefined adapter position";

    if (itemView->getParent() == nullptr) oss<<" no parent";
    oss<<"}";
    return oss.str();
}

void RecyclerView::ViewHolder::setIsRecyclable(bool recyclable) {
    mIsRecyclableCount = recyclable ? mIsRecyclableCount - 1 : mIsRecyclableCount + 1;
    if (mIsRecyclableCount < 0) {
        mIsRecyclableCount = 0;
        LOGE_IF(sDebugAssertionsEnabled,"isRecyclable decremented to %d is below 0: "
             "unmatched pair of setIsRecyable() calls for %p" , mIsRecyclableCount,this);
        LOGE("isRecyclable decremented to %d is below 0: "
              "unmatched pair of setIsRecyable() calls for %p" ,mIsRecyclableCount, this);
    } else if (!recyclable && mIsRecyclableCount == 1) {
        mFlags |= FLAG_NOT_RECYCLABLE;
    } else if (recyclable && mIsRecyclableCount == 0) {
        mFlags &= ~FLAG_NOT_RECYCLABLE;
    }
}

bool RecyclerView::ViewHolder::isRecyclable() const{
    return (mFlags & FLAG_NOT_RECYCLABLE) == 0
            && !itemView->hasTransientState();
}

bool RecyclerView::ViewHolder::shouldBeKeptAsChild() {
    return (mFlags & FLAG_NOT_RECYCLABLE) != 0;
}

bool RecyclerView::ViewHolder::doesTransientStatePreventRecycling() {
    return (mFlags & FLAG_NOT_RECYCLABLE) == 0 && itemView->hasTransientState();
}

bool RecyclerView::ViewHolder::isUpdated() const{
    return (mFlags & FLAG_UPDATE) != 0;
}

/////////////////////////////////////////////endof ViewHolder///////////////////////////////////////////

bool RecyclerView::setChildImportantForAccessibilityInternal(ViewHolder* viewHolder, int importantForAccessibility) {
    if (isComputingLayout()) {
        viewHolder->mPendingAccessibilityState = importantForAccessibility;
        mPendingAccessibilityImportanceChange.push_back(viewHolder);
        return false;
    }
    viewHolder->itemView->setImportantForAccessibility(importantForAccessibility);
    return true;
}

void RecyclerView::dispatchPendingImportantForAccessibilityChanges() {
    for (int i = int(mPendingAccessibilityImportanceChange.size() - 1); i >= 0; i--) {
        ViewHolder* viewHolder = mPendingAccessibilityImportanceChange.at(i);
        if (viewHolder->itemView->getParent() != this || viewHolder->shouldIgnore()) {
            continue;
        }
        const int state = viewHolder->mPendingAccessibilityState;
        if (state != ViewHolder::PENDING_ACCESSIBILITY_STATE_NOT_SET) {
            //noinspection WrongConstant
            viewHolder->itemView->setImportantForAccessibility(state);
            viewHolder->mPendingAccessibilityState = ViewHolder::PENDING_ACCESSIBILITY_STATE_NOT_SET;
        }
    }
    mPendingAccessibilityImportanceChange.clear();
}

int RecyclerView::getAdapterPositionInRecyclerView(ViewHolder* viewHolder) const{
    if (viewHolder->hasAnyOfTheFlags(ViewHolder::FLAG_INVALID
            | ViewHolder::FLAG_REMOVED | ViewHolder::FLAG_ADAPTER_POSITION_UNKNOWN)
            || !viewHolder->isBound()) {
        return RecyclerView::NO_POSITION;
    }
    return mAdapterHelper->applyPendingUpdatesToPosition(viewHolder->mPosition);
}

void RecyclerView::initFastScroller(StateListDrawable* verticalThumb, Drawable* verticalTrack,
   StateListDrawable* horizontalThumb,Drawable* horizontalTrack,const AttributeSet&atts) {
    if (verticalThumb == nullptr || verticalTrack == nullptr
            || horizontalThumb == nullptr || horizontalTrack == nullptr) {
        throw std::runtime_error("Trying to set fast scroller without both required drawables.");
    }
    //Resources resources = getContext().getResources();
    new FastScroller(this, verticalThumb, verticalTrack, horizontalThumb, horizontalTrack,
            atts.getDimensionPixelSize("default_thickness",4),//R.dimen.fastscroll_default_thickness),
            atts.getDimensionPixelSize("minimum_range",32),//R.dimen.fastscroll_minimum_range),
            atts.getDimensionPixelOffset("margin"));//R.dimen.fastscroll_margin));
}

//////////////////////////////RecyclerView::NestedScrollingChild//////////////////////////////////////

void RecyclerView::setNestedScrollingEnabled(bool enabled) {
    getScrollingChildHelper()->setNestedScrollingEnabled(enabled);
}

bool RecyclerView::isNestedScrollingEnabled() {
    return getScrollingChildHelper()->isNestedScrollingEnabled();
}

bool RecyclerView::startNestedScroll(int axes) {
    return getScrollingChildHelper()->startNestedScroll(axes);
}

bool RecyclerView::startNestedScroll(int axes, int type) {
    return getScrollingChildHelper()->startNestedScroll(axes, type);
}

void RecyclerView::stopNestedScroll() {
    getScrollingChildHelper()->stopNestedScroll();
}

void RecyclerView::stopNestedScroll(int type) {
    getScrollingChildHelper()->stopNestedScroll(type);
}

bool RecyclerView::hasNestedScrollingParent() {
    return getScrollingChildHelper()->hasNestedScrollingParent();
}

bool RecyclerView::hasNestedScrollingParent(int type) {
    return getScrollingChildHelper()->hasNestedScrollingParent(type);
}

bool RecyclerView::dispatchNestedScroll(int dxConsumed, int dyConsumed, int dxUnconsumed,
    int dyUnconsumed, int offsetInWindow[2]) {
    return getScrollingChildHelper()->dispatchNestedScroll(dxConsumed, dyConsumed,
            dxUnconsumed, dyUnconsumed, offsetInWindow);
}

bool RecyclerView::dispatchNestedScroll(int dxConsumed, int dyConsumed, int dxUnconsumed,
    int dyUnconsumed, int offsetInWindow[2], int type) {
    return getScrollingChildHelper()->dispatchNestedScroll(dxConsumed, dyConsumed,
            dxUnconsumed, dyUnconsumed, offsetInWindow, type);
}

bool RecyclerView::dispatchNestedScroll(int dxConsumed, int dyConsumed, int dxUnconsumed,int dyUnconsumed, int offsetInWindow[2], int type,int consumed[2]){
    return getScrollingChildHelper()->dispatchNestedScroll(dxConsumed, dyConsumed, dxUnconsumed, dyUnconsumed, offsetInWindow, type, consumed);
}

bool RecyclerView::dispatchNestedPreScroll(int dx, int dy, int consumed[2], int offsetInWindow[2]) {
    return getScrollingChildHelper()->dispatchNestedPreScroll(dx, dy, consumed, offsetInWindow);
}

bool RecyclerView::dispatchNestedPreScroll(int dx, int dy, int consumed[2], int offsetInWindow[2],int type) {
    return getScrollingChildHelper()->dispatchNestedPreScroll(dx, dy, consumed, offsetInWindow,type);
}

bool RecyclerView::dispatchNestedFling(float velocityX, float velocityY, bool consumed) {
    return getScrollingChildHelper()->dispatchNestedFling(velocityX, velocityY, consumed);
}

bool RecyclerView::dispatchNestedPreFling(float velocityX, float velocityY) {
    return getScrollingChildHelper()->dispatchNestedPreFling(velocityX, velocityY);
}

//////////////////////////////////////RecyclerView::LayoutParams///////////////////////////////////////

RecyclerView::LayoutParams::LayoutParams(Context* c,const AttributeSet& attrs)
    :ViewGroup::MarginLayoutParams(c, attrs){
}

RecyclerView::LayoutParams::LayoutParams(int width, int height)
    :ViewGroup::MarginLayoutParams(width, height){
}

RecyclerView::LayoutParams::LayoutParams(const ViewGroup::MarginLayoutParams& source)
    :ViewGroup::MarginLayoutParams(source){
}

RecyclerView::LayoutParams::LayoutParams(const ViewGroup::LayoutParams& source)
   :ViewGroup::MarginLayoutParams(source){
}

RecyclerView::LayoutParams::LayoutParams(const LayoutParams& source)
    :ViewGroup::MarginLayoutParams((const ViewGroup::LayoutParams&) source){
}

bool RecyclerView::LayoutParams::viewNeedsUpdate() {
    return mViewHolder->needsUpdate();
}

bool RecyclerView::LayoutParams::isViewInvalid() {
    return mViewHolder->isInvalid();
}

bool RecyclerView::LayoutParams::isItemRemoved() {
    return mViewHolder->isRemoved();
}

bool RecyclerView::LayoutParams::isItemChanged() {
    return mViewHolder->isUpdated();
}

/*//@Deprecated
int RecyclerView::LayoutParams::getViewPosition() {
    return mViewHolder->getPosition();
}*/

int RecyclerView::LayoutParams::getViewLayoutPosition() {
    return mViewHolder->getLayoutPosition();
}

/*int RecyclerView::LayoutParams::getViewAdapterPosition() {
    return mViewHolder->getBindingAdapterPosition();
}*/

int RecyclerView::LayoutParams::getAbsoluteAdapterPosition() {
    return mViewHolder->getAbsoluteAdapterPosition();
}

int RecyclerView::LayoutParams::getBindingAdapterPosition() {
    return mViewHolder->getBindingAdapterPosition();
}
//////////////////////////////////RecyclerView::AdapterDataObserver///////////////////////////////

void RecyclerView::AdapterDataObserver::onChanged() {
    // Do nothing
}

void RecyclerView::AdapterDataObserver::onItemRangeChanged(int positionStart, int itemCount) {
    // do nothing
}

void RecyclerView::AdapterDataObserver::onItemRangeChanged(int positionStart, int itemCount,Object* payload) {
    // fallback to onItemRangeChanged(positionStart, itemCount) if app
    // does not override this method.
    onItemRangeChanged(positionStart, itemCount);
}

void RecyclerView::AdapterDataObserver::onItemRangeInserted(int positionStart, int itemCount) {
    // do nothing
}

void RecyclerView::AdapterDataObserver::onItemRangeRemoved(int positionStart, int itemCount) {
    // do nothing
}

void RecyclerView::AdapterDataObserver::onItemRangeMoved(int fromPosition, int toPosition, int itemCount) {
    // do nothing
}

void RecyclerView::AdapterDataObserver::onStateRestorationPolicyChanged() {
    // do nothing
}
//////////////////////////////////////RecyclerView::SmoothScroller/////////////////////////////////////

RecyclerView::SmoothScroller::SmoothScroller() {
    mStarted = false;
    mRunning = false;
    mPendingInitialRun = false;
    mTargetPosition = RecyclerView::NO_POSITION;
    mRecyclerView = nullptr;
    mLayoutManager = nullptr;
    mTargetView = nullptr;
    mRecyclingAction = new Action(0, 0);
}

RecyclerView::SmoothScroller::~SmoothScroller(){
    delete mRecyclingAction;
}

void RecyclerView::SmoothScroller::start(RecyclerView* recyclerView, LayoutManager* layoutManager) {
    recyclerView->mViewFlinger->stop();
    LOGW_IF(mStarted,"An instance of SmoothScroller was started more than once. Each instance of SmoothScroller "
                "is intended to only be used once. You should create a new instance for each use.");
    mRecyclerView = recyclerView;
    mLayoutManager = layoutManager;
    if (mTargetPosition == RecyclerView::NO_POSITION) {
        LOGE("Invalid target position");
    }
    mRecyclerView->mState->mTargetPosition = mTargetPosition;
    mRunning = true;
    mPendingInitialRun = true;
    mTargetView = findViewByPosition(getTargetPosition());
    onStart();
    mRecyclerView->mViewFlinger->postOnAnimation();

    mStarted = true;
}

void RecyclerView::SmoothScroller::setTargetPosition(int targetPosition) {
    mTargetPosition = targetPosition;
}

bool RecyclerView::SmoothScroller::computeScrollVectorForPosition(int targetPosition,PointF&point) {
    LayoutManager* layoutManager = getLayoutManager();
    return layoutManager->computeScrollVectorForPosition(targetPosition,point);
}

RecyclerView::LayoutManager* RecyclerView::SmoothScroller::getLayoutManager() {
    return mLayoutManager;
}

void RecyclerView::SmoothScroller::stop() {
    if (!mRunning) {
        return;
    }
    mRunning = false;
    onStop();
    mRecyclerView->mState->mTargetPosition = RecyclerView::NO_POSITION;
    mTargetView = nullptr;
    mTargetPosition = RecyclerView::NO_POSITION;
    mPendingInitialRun = false;
    // trigger a cleanup
    mLayoutManager->onSmoothScrollerStopped(this);
    // clear references to avoid any potential leak by a custom smooth scroller
    mLayoutManager = nullptr;
    mRecyclerView = nullptr;
}

bool RecyclerView::SmoothScroller::isPendingInitialRun() const{
    return mPendingInitialRun;
}

bool RecyclerView::SmoothScroller::isRunning() const{
    return mRunning;
}

int RecyclerView::SmoothScroller::getTargetPosition() const{
    return mTargetPosition;
}

void RecyclerView::SmoothScroller::onAnimation(int dx, int dy) {
    RecyclerView* recyclerView = mRecyclerView;
    if ( (mTargetPosition == RecyclerView::NO_POSITION) || (recyclerView == nullptr)) {
        stop();
    }

    // The following if block exists to have the LayoutManager scroll 1 pixel in the correct
    // direction in order to cause the LayoutManager to draw two pages worth of views so
    // that the target view may be found before scrolling any further.  This is done to
    // prevent an initial scroll distance from scrolling past the view, which causes a
    // jittery looking animation.
    if (mPendingInitialRun && (mTargetView == nullptr) && mLayoutManager) {
        PointF pointF;
        const bool rc = computeScrollVectorForPosition(mTargetPosition,pointF);
        if (rc && (pointF.x != 0 || pointF.y != 0)) {
            recyclerView->scrollStep( (int) MathUtils::signum(pointF.x), (int) MathUtils::signum(pointF.y), nullptr);
        }
    }

    mPendingInitialRun = false;
    if (mTargetView != nullptr) {
        // verify target position
        if (getChildPosition(mTargetView) == mTargetPosition) {
            onTargetFound(mTargetView, *recyclerView->mState, *mRecyclingAction);
            mRecyclingAction->runIfNecessary(*recyclerView);
            stop();
        } else {
            LOGE("Passed over target position while smooth scrolling.");
            mTargetView = nullptr;
        }
    }
    if (mRunning) {
        onSeekTargetStep(dx, dy, *recyclerView->mState, *mRecyclingAction);
        const bool hadJumpTarget = mRecyclingAction->hasJumpTarget();
        mRecyclingAction->runIfNecessary(*recyclerView);
        if (hadJumpTarget) {
            // It is not stopped so needs to be restarted
            if (mRunning) {
                mPendingInitialRun = true;
                recyclerView->mViewFlinger->postOnAnimation();
            }
        }
    }
 }

int RecyclerView::SmoothScroller::getChildPosition(View* view) {
    return mRecyclerView->getChildLayoutPosition(view);
}

int RecyclerView::SmoothScroller::getChildCount() const{
    return mRecyclerView->mLayout->getChildCount();
}

View* RecyclerView::SmoothScroller::findViewByPosition(int position) {
    return mRecyclerView->mLayout->findViewByPosition(position);
}

//@Deprecated
void RecyclerView::SmoothScroller::instantScrollToPosition(int position) {
    mRecyclerView->scrollToPosition(position);
}

void RecyclerView::SmoothScroller::onChildAttachedToWindow(View* child) {
    if (getChildPosition(child) == getTargetPosition()) {
        mTargetView = child;
        LOGD_IF(sDebugAssertionsEnabled,"smooth scroll target view has been attached");
    }
}

void RecyclerView::SmoothScroller::normalize(PointF& scrollVector) {
    const float magnitude = (float) std::sqrt(scrollVector.x * scrollVector.x
            + scrollVector.y * scrollVector.y);
    scrollVector.x /= magnitude;
    scrollVector.y /= magnitude;
}

/////////////////////////////RecyclerView::SmoothScroller::Action/////////////////////////////////////

RecyclerView::SmoothScroller::Action::Action(int dx,int dy):Action(dx, dy, UNDEFINED_DURATION, nullptr){
}

RecyclerView::SmoothScroller::Action::Action(int dx, int dy, int duration):Action(dx, dy, duration, nullptr){
}

RecyclerView::SmoothScroller::Action::Action(int dx, int dy, int duration,Interpolator* interpolator) {
    mDx = dx;
    mDy = dy;
    mChanged = false;
    mDuration = duration;
    mInterpolator = interpolator;
    mConsecutiveUpdates = 0;
    mJumpToPosition = NO_POSITION;
}

void RecyclerView::SmoothScroller::Action::jumpTo(int targetPosition) {
    mJumpToPosition = targetPosition;
}

bool RecyclerView::SmoothScroller::Action::hasJumpTarget() {
    return mJumpToPosition >= 0;
}

void RecyclerView::SmoothScroller::Action::runIfNecessary(RecyclerView& recyclerView) {
    if (mJumpToPosition >= 0) {
        int position = mJumpToPosition;
        mJumpToPosition = NO_POSITION;
        recyclerView.jumpToPositionForSmoothScroller(position);
        mChanged = false;
        return;
    }
    if (mChanged) {
        validate();
        recyclerView.mViewFlinger->smoothScrollBy(mDx, mDy, mDuration, mInterpolator);
        mConsecutiveUpdates++;
        // A new action is being set in every animation step. This looks like a bad
        // implementation. Inform developer.
        LOGE_IF(mConsecutiveUpdates>10,"Smooth Scroll action is being updated too frequently. Make sure"
               " you are not changing it unless necessary");
        mChanged = false;
    } else {
        mConsecutiveUpdates = 0;
   }
}

void RecyclerView::SmoothScroller::Action::validate() {
    if (mInterpolator && mDuration < 1) {
        LOGE("If you provide an interpolator, you must set a positive duration");
    } else if (mDuration < 1) {
        LOGE("Scroll duration must be a positive number");
    }
}

int RecyclerView::SmoothScroller::Action::getDx() {
    return mDx;
}

void RecyclerView::SmoothScroller::Action::setDx(int dx) {
    mChanged = true;
    mDx = dx;
}

int RecyclerView::SmoothScroller::Action::getDy() {
    return mDy;
}

void RecyclerView::SmoothScroller::Action::setDy(int dy) {
    mChanged = true;
    mDy = dy;
}

int RecyclerView::SmoothScroller::Action::getDuration() {
    return mDuration;
}

void RecyclerView::SmoothScroller::Action::setDuration(int duration) {
    mChanged = true;
    mDuration = duration;
}

Interpolator* RecyclerView::SmoothScroller::Action::getInterpolator() {
    return mInterpolator;
}

void RecyclerView::SmoothScroller::Action::setInterpolator(Interpolator* interpolator) {
    mChanged = true;
    mInterpolator = interpolator;
}

void RecyclerView::SmoothScroller::Action::update(int dx,int dy,int duration,Interpolator* interpolator) {
    mDx = dx;
    mDy = dy;
    mDuration = duration;
    mInterpolator = interpolator;
    mChanged = true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/** called by CREATOR */
RecyclerView::SavedState::SavedState(Parcel& in):AbsSavedState(in){
    //super(in, loader);
    //mLayoutState = in.readParcelable(loader != null ? loader : LayoutManager.class.getClassLoader());
}

/**Called by onSaveInstanceState */
RecyclerView::SavedState::SavedState(Parcelable* superState):AbsSavedState(superState){
}

void RecyclerView::SavedState::writeToParcel(Parcel& dest, int flags) {
    AbsSavedState::writeToParcel(dest, flags);
    //dest.writeParcelable(mLayoutState, 0);
}

void RecyclerView::SavedState::copyFrom(SavedState& other) {
    mLayoutState = other.mLayoutState;
}


bool RecyclerView::AdapterDataObservable::hasObservers() const{
    return !mObservers.empty();
}

void RecyclerView::AdapterDataObservable::notifyChanged() {
    // since onChanged() is implemented by the app, it could do anything, including
    // removing itself from {@link mObservers} - and that could cause problems if
    // an iterator is used on the ArrayList {@link mObservers}.
    // to avoid such problems, just march thru the list in the reverse order.
    for (int i = int(mObservers.size() - 1); i >= 0; i--) {
        mObservers.at(i)->onChanged();
    }
}

void RecyclerView::AdapterDataObservable::notifyStateRestorationPolicyChanged() {
    for (int i = mObservers.size() - 1; i >= 0; i--) {
        mObservers.at(i)->onStateRestorationPolicyChanged();
    }
}

void RecyclerView::AdapterDataObservable::notifyItemRangeChanged(int positionStart, int itemCount) {
    notifyItemRangeChanged(positionStart, itemCount, nullptr);
}

void RecyclerView::AdapterDataObservable::notifyItemRangeChanged(int positionStart, int itemCount,Object* payload) {
    // since onItemRangeChanged() is implemented by the app, it could do anything, including
    // removing itself from {@link mObservers} - and that could cause problems if
    // an iterator is used on the ArrayList {@link mObservers}.
    // to avoid such problems, just march thru the list in the reverse order.
    for (int i = int(mObservers.size() - 1); i >= 0; i--) {
        mObservers.at(i)->onItemRangeChanged(positionStart, itemCount, payload);
    }
}

void RecyclerView::AdapterDataObservable::notifyItemRangeInserted(int positionStart, int itemCount) {
    // since onItemRangeInserted() is implemented by the app, it could do anything,
    // including removing itself from {@link mObservers} - and that could cause problems if
    // an iterator is used on the ArrayList {@link mObservers}.
    // to avoid such problems, just march thru the list in the reverse order.
    for (int i = int(mObservers.size() - 1); i >= 0; i--) {
        mObservers.at(i)->onItemRangeInserted(positionStart, itemCount);
    }
}

void RecyclerView::AdapterDataObservable::notifyItemRangeRemoved(int positionStart, int itemCount) {
    // since onItemRangeRemoved() is implemented by the app, it could do anything, including
    // removing itself from {@link mObservers} - and that could cause problems if
    // an iterator is used on the ArrayList {@link mObservers}.
    // to avoid such problems, just march thru the list in the reverse order.
    for (int i = int(mObservers.size() - 1); i >= 0; i--) {
        mObservers.at(i)->onItemRangeRemoved(positionStart, itemCount);
    }
}

void RecyclerView::AdapterDataObservable::notifyItemMoved(int fromPosition, int toPosition) {
    for (int i = int(mObservers.size() - 1); i >= 0; i--) {
        mObservers.at(i)->onItemRangeMoved(fromPosition, toPosition, 1);
    }
}

//////////////////////////////////////RecyclerView::State////////////////////////////////////

void RecyclerView::State::assertLayoutStep(int accepted) {
    if ((accepted & mLayoutStep) == 0) {
        LOGE("Layout state should be one of %d but it is %d",accepted,mLayoutStep);
    }
}


/** Owned by SmoothScroller */

void RecyclerView::State::prepareForNestedPrefetch(RecyclerView::Adapter* adapter) {
    mLayoutStep = STEP_START;
    mItemCount = adapter->getItemCount();
    mInPreLayout = false;
    mTrackOldChangeHolders = false;
    mIsMeasuring = false;
}

bool RecyclerView::State::isMeasuring() {
    return mIsMeasuring;
}

bool RecyclerView::State::isPreLayout() {
    return mInPreLayout;
}

bool RecyclerView::State::willRunPredictiveAnimations() {
    return mRunPredictiveAnimations;
}

bool RecyclerView::State::willRunSimpleAnimations() {
    return mRunSimpleAnimations;
}

void RecyclerView::State::remove(int resourceId) {
    if (0==mData.size()) {
        return;
    }
    mData.remove(resourceId);
}

Object* RecyclerView::State::get(int resourceId) {
    if (0==mData.size()) {
        return nullptr;
    }
    return mData.get(resourceId);
}

void RecyclerView::State::put(int resourceId, Object* data) {
    mData.put(resourceId, data);
}

int RecyclerView::State::getTargetScrollPosition() {
    return mTargetPosition;
}

bool RecyclerView::State::hasTargetScrollPosition() {
    return mTargetPosition != RecyclerView::NO_POSITION;
}

bool RecyclerView::State::didStructureChange() {
    return mStructureChanged;
}

int RecyclerView::State::getItemCount() {
    return mInPreLayout
            ? (mPreviousLayoutItemCount - mDeletedInvisibleItemCountSincePreviousLayout)
            : mItemCount;
}

int RecyclerView::State::getRemainingScrollHorizontal() {
    return mRemainingScrollHorizontal;
}

int RecyclerView::State::getRemainingScrollVertical() {
    return mRemainingScrollVertical;
}

///////////////////////////////////RecyclerView::ItemAnimator///////////////////////////////////

long RecyclerView::ItemAnimator::getMoveDuration() {
    return mMoveDuration;
}

void RecyclerView::ItemAnimator::setMoveDuration(long moveDuration) {
    mMoveDuration = moveDuration;
}

long RecyclerView::ItemAnimator::getAddDuration() {
    return mAddDuration;
}

void RecyclerView::ItemAnimator::setAddDuration(long addDuration) {
    mAddDuration = addDuration;
}

long RecyclerView::ItemAnimator::getRemoveDuration() {
    return mRemoveDuration;
}

void RecyclerView::ItemAnimator::setRemoveDuration(long removeDuration) {
    mRemoveDuration = removeDuration;
}

long RecyclerView::ItemAnimator::getChangeDuration() {
    return mChangeDuration;
}

void RecyclerView::ItemAnimator::setChangeDuration(long changeDuration) {
    mChangeDuration = changeDuration;
}

void RecyclerView::ItemAnimator::setListener(const ItemAnimator::ItemAnimatorListener& listener) {
    mListener = listener;
}

RecyclerView::ItemAnimator::ItemHolderInfo* RecyclerView::ItemAnimator::recordPreLayoutInformation(State& state,
       ViewHolder& viewHolder, int changeFlags,std::vector<Object*>& payloads) {
    return obtainHolderInfo()->setFrom(viewHolder);
}

RecyclerView::ItemAnimator::ItemHolderInfo* RecyclerView::ItemAnimator::recordPostLayoutInformation(State& state,ViewHolder& viewHolder) {
    return obtainHolderInfo()->setFrom(viewHolder);
}

int RecyclerView::ItemAnimator::buildAdapterChangeFlagsForAnimations(ViewHolder* viewHolder) {
    int flags = viewHolder->mFlags & (FLAG_INVALIDATED | FLAG_REMOVED | FLAG_CHANGED);
    if (viewHolder->isInvalid()) {
        return FLAG_INVALIDATED;
    }
    if ((flags & FLAG_INVALIDATED) == 0) {
        const int oldPos = viewHolder->getOldPosition();
        const int pos = viewHolder->getAbsoluteAdapterPosition();
        if (oldPos != NO_POSITION && pos != NO_POSITION && oldPos != pos) {
            flags |= FLAG_MOVED;
        }
    }
    return flags;
}

void RecyclerView::ItemAnimator::dispatchAnimationFinished(ViewHolder& viewHolder) {
    onAnimationFinished(viewHolder);
    if (mListener != nullptr) {
        mListener(viewHolder);//.onAnimationFinished(viewHolder);
    }
}

void RecyclerView::ItemAnimator::onAnimationFinished(ViewHolder& viewHolder) {
}

void RecyclerView::ItemAnimator::dispatchAnimationStarted(ViewHolder& viewHolder) {
    onAnimationStarted(viewHolder);
}

void RecyclerView::ItemAnimator::onAnimationStarted(ViewHolder& viewHolder) {

}

bool RecyclerView::ItemAnimator::isRunning(ItemAnimatorFinishedListener listener) {
    bool running = isRunning();
    if (listener != nullptr) {
        if (!running) {
            listener();//.onAnimationsFinished();
        } else {
            mFinishedListeners.push_back(listener);//add(listener);
        }
    }
    return running;
}

bool RecyclerView::ItemAnimator::canReuseUpdatedViewHolder(ViewHolder& viewHolder) {
    return true;
}

bool RecyclerView::ItemAnimator::canReuseUpdatedViewHolder(ViewHolder& viewHolder,std::vector<Object*>& payloads) {
    return canReuseUpdatedViewHolder(viewHolder);
}

void RecyclerView::ItemAnimator::dispatchAnimationsFinished() {
    const size_t count = mFinishedListeners.size();
    for (size_t i = 0; i < count; ++i) {
        auto ls = mFinishedListeners.at(i);
        if(ls)ls();//.onAnimationsFinished();
    }
    mFinishedListeners.clear();
}

RecyclerView::ItemAnimator::ItemHolderInfo* RecyclerView::ItemAnimator::obtainHolderInfo() {
    return new ItemHolderInfo();
}

///////////////////////////////ItemAnimator::ItemHolderInfo///////////////////////////////////

RecyclerView::ItemAnimator::ItemHolderInfo::ItemHolderInfo() {
}

RecyclerView::ItemAnimator::ItemHolderInfo* RecyclerView::ItemAnimator::ItemHolderInfo::setFrom(RecyclerView::ViewHolder& holder) {
    return setFrom(holder, 0);
}

RecyclerView::ItemAnimator::ItemHolderInfo* RecyclerView::ItemAnimator::ItemHolderInfo::setFrom(RecyclerView::ViewHolder& holder,int flags) {
    View* view = holder.itemView;
    this->left = view->getLeft();
    this->top = view->getTop();
    this->right = view->getRight();
    this->bottom = view->getBottom();
    return this;
}
//endof ItemAnimator
/////////////////////////////////////////////////////////////////////////////////////////
int RecyclerView::getChildDrawingOrder(int childCount, int i) {
    if (mChildDrawingOrderCallback)
        return mChildDrawingOrderCallback(childCount,i);//.onGetChildDrawingOrder(childCount, i);
    return ViewGroup::getChildDrawingOrder(childCount, i);
}

NestedScrollingChildHelper* RecyclerView::getScrollingChildHelper() {
    if (mScrollingChildHelper == nullptr) {
        mScrollingChildHelper = new NestedScrollingChildHelper(this);
    }
    return mScrollingChildHelper;
}

ScrollFeedbackProvider* RecyclerView::getScrollFeedbackProvider() {
    if (mScrollFeedbackProvider == nullptr) {
        mScrollFeedbackProvider = ScrollFeedbackProvider::createProvider(this);
    }
    return mScrollFeedbackProvider;
}

//reference:androidx.recyclerview.widget/RecyclerView
}/*endof namespace*/
