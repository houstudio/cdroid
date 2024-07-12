#include <widgetEx/viewpager2.h>
#include <widgetEx/recyclerview/pagersnaphelper.h>
#include <widgetEx/scrolleventadapter.h>
#include <widgetEx/fakedrag.h>
#include <widgetEx/compositeonpagechangecallback.h>

namespace cdroid{

DECLARE_WIDGET(ViewPager2)

class PageTransformerAdapter:public ViewPager2::OnPageChangeCallback {
private:
    LinearLayoutManager* mLayoutManager;
    ViewPager2::PageTransformer* mPageTransformer;
public:
    PageTransformerAdapter(LinearLayoutManager* layoutManager) {
        mLayoutManager = layoutManager;
	onPageScrolled = std::bind(&PageTransformerAdapter::doPageScrolled,this,
            std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
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

void ViewPager2::initialize(Context* context,const AttributeSet& attrs) {
    /*mAccessibilityProvider = sFeatureEnhancedA11yEnabled
            ? new PageAwareAccessibilityProvider()
            : new BasicAccessibilityProvider();*/
    mCurrentItemDataSetChangeObserver = nullptr;
    mRecyclerView = new RecyclerViewImpl(context,attrs);
    mRecyclerView->mVP =this;
    mRecyclerView->setId(View::generateViewId());
    mRecyclerView->setDescendantFocusability(FOCUS_BEFORE_DESCENDANTS);

    mLayoutManager = new LinearLayoutManagerImpl(context,this);
    mRecyclerView->setLayoutManager(mLayoutManager);
    mRecyclerView->setScrollingTouchSlop(RecyclerView::TOUCH_SLOP_PAGING);
    setOrientation(context, attrs);

    mRecyclerView->setLayoutParams(
            new ViewGroup::LayoutParams(LayoutParams::MATCH_PARENT, LayoutParams::MATCH_PARENT));
    attachViewToParent(mRecyclerView, 0, mRecyclerView->getLayoutParams());

    RecyclerView::OnChildAttachStateChangeListener ls;
    ls.onChildViewAttachedToWindow=[](View&view){
        RecyclerView::LayoutParams* layoutParams = (RecyclerView::LayoutParams*) view.getLayoutParams();
        if (layoutParams->width != LayoutParams::MATCH_PARENT
                || layoutParams->height != LayoutParams::MATCH_PARENT) {
            FATAL("Pages must fill the whole ViewPager2 (use match_parent)");
        }
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
    mRecyclerView->addOnScrollListener(*mScrollEventAdapter);

    mPageChangeEventDispatcher = new CompositeOnPageChangeCallback(3);
    mScrollEventAdapter->setOnPageChangeCallback(*mPageChangeEventDispatcher);

    // Callback that updates mCurrentItem after swipes. Also triggered in other cases, but in
    // all those cases mCurrentItem will only be overwritten with the same value.
    OnPageChangeCallback currentItemUpdater;// = new OnPageChangeCallback() {
    currentItemUpdater.onPageSelected=[this](int position) {
            if (mCurrentItem != position) {
                mCurrentItem = position;
                //mAccessibilityProvider.onSetNewCurrentItem();
            }
        };

    currentItemUpdater.onPageScrollStateChanged=[this](int newState) {
            if (newState == SCROLL_STATE_IDLE) {
                updateCurrentItem();
            }
        };

    // Prevents focus from remaining on a no-longer visible page
    OnPageChangeCallback focusClearer;// = new OnPageChangeCallback() {
    focusClearer.onPageSelected=[this](int position) {
            clearFocus();
            if (hasFocus()) { // if clear focus did not succeed
                mRecyclerView->requestFocus(View::FOCUS_FORWARD);
            }
        };

    // Add currentItemUpdater before mExternalPageChangeCallbacks, because we need to update
    // internal state first
    mPageChangeEventDispatcher->addOnPageChangeCallback(currentItemUpdater);
    mPageChangeEventDispatcher->addOnPageChangeCallback(focusClearer);
    // Allow a11y to register its listeners after currentItemUpdater (so it has the
    // right data). TODO: replace ordering comments with a test.
    //mAccessibilityProvider->onInitialize(mPageChangeEventDispatcher, mRecyclerView);
    mExternalPageChangeCallbacks = new CompositeOnPageChangeCallback(0);
    mPageChangeEventDispatcher->addOnPageChangeCallback(*mExternalPageChangeCallbacks);

    // Add mPageTransformerAdapter after mExternalPageChangeCallbacks, because page transform
    // events must be fired after scroll events
    mPageTransformerAdapter = new PageTransformerAdapter(mLayoutManager);
    mPageChangeEventDispatcher->addOnPageChangeCallback(*mPageTransformerAdapter);

    //attachViewToParent(mRecyclerView, 0, mRecyclerView->getLayoutParams());

}

#if 0
CharSequence ViewPager2::getAccessibilityClassName() {
    if (mAccessibilityProvider.handlesGetAccessibilityClassName()) {
        return mAccessibilityProvider.onGetAccessibilityClassName();
    }
    return super.getAccessibilityClassName();
}
#endif

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
    //mAccessibilityProvider.onRestorePendingState();
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
    //mAccessibilityProvider.onDetachAdapter(currentAdapter);
    unregisterCurrentItemDataSetTracker(currentAdapter);
    mRecyclerView->setAdapter(adapter);
    mCurrentItem = 0;
    restorePendingState();
    //mAccessibilityProvider->onAttachAdapter(adapter);
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
        mPageChangeEventDispatcher->onPageSelected(snapPosition);
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
    //mAccessibilityProvider.onSetOrientation();
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
    //mAccessibilityProvider->onSetNewCurrentItem();

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
    if (snapDistance[0] != 0 || snapDistance[1] != 0) {
        mRecyclerView->smoothScrollBy(snapDistance[0], snapDistance[1]);
    }
}

void ViewPager2::setUserInputEnabled(bool enabled) {
    mUserInputEnabled = enabled;
    //mAccessibilityProvider.onSetUserInputEnabled();
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

void ViewPager2::registerOnPageChangeCallback(OnPageChangeCallback callback) {
    mExternalPageChangeCallbacks->addOnPageChangeCallback(callback);
}

void ViewPager2::unregisterOnPageChangeCallback(OnPageChangeCallback callback) {
    mExternalPageChangeCallbacks->removeOnPageChangeCallback(callback);
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

View& ViewPager2::setLayoutDirection(int layoutDirection) {
    ViewGroup::setLayoutDirection(layoutDirection);
    //mAccessibilityProvider.onSetLayoutDirection();
    return *this;
}

#if 0
void ViewPager2::onInitializeAccessibilityNodeInfo(AccessibilityNodeInfo info) {
    super.onInitializeAccessibilityNodeInfo(info);
    mAccessibilityProvider.onInitializeAccessibilityNodeInfo(info);
}

bool ViewPager2::performAccessibilityAction(int action, Bundle arguments) {
    if (mAccessibilityProvider.handlesPerformAccessibilityAction(action, arguments)) {
        return mAccessibilityProvider.onPerformAccessibilityAction(action, arguments);
    }
    return super.performAccessibilityAction(action, arguments);
}
#endif

ViewPager2::RecyclerViewImpl::RecyclerViewImpl(Context* context,const AttributeSet&attr)
    :RecyclerView(context,attr){
}

#if 0
CharSequence ViewPager2::RecyclerViewImpl::getAccessibilityClassName() {
    if (mAccessibilityProvider.handlesRvGetAccessibilityClassName()) {
        return mAccessibilityProvider.onRvGetAccessibilityClassName();
    }
    return super.getAccessibilityClassName();
}

void ViewPager2::RecyclerViewImpl::onInitializeAccessibilityEvent(AccessibilityEvent& event) {
    super.onInitializeAccessibilityEvent(event);
    event.setFromIndex(mCurrentItem);
    event.setToIndex(mCurrentItem);
    mAccessibilityProvider.onRvInitializeAccessibilityEvent(event);*/
}
#endif

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

#if 0 
bool ViewPager2::LinearLayoutManagerImpl::performAccessibilityAction(@NonNull RecyclerView::Recycler recycler,
        @NonNull RecyclerView::State state, int action, @Nullable Bundle args) {
    if (mAccessibilityProvider.handlesLmPerformAccessibilityAction(action)) {
        return mAccessibilityProvider.onLmPerformAccessibilityAction(action);
    }
    return super.performAccessibilityAction(recycler, state, action, args);
}

void ViewPager2::LinearLayoutManagerImpl::onInitializeAccessibilityNodeInfo(@NonNull RecyclerView::Recycler recycler,
        @NonNull RecyclerView::State state, @NonNull AccessibilityNodeInfoCompat info) {
    super.onInitializeAccessibilityNodeInfo(recycler, state, info);
    mAccessibilityProvider.onLmInitializeAccessibilityNodeInfo(info);
}
#endif

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

#if 0
    // TODO(b/141956012): Suppressed during upgrade to AGP 3.6.
    class BasicAccessibilityProvider extends AccessibilityProvider {
        @Override
        public bool handlesLmPerformAccessibilityAction(int action) {
            return (action == AccessibilityNodeInfoCompat.ACTION_SCROLL_BACKWARD
                    || action == AccessibilityNodeInfoCompat.ACTION_SCROLL_FORWARD)
                    && !isUserInputEnabled();
        }

        @Override
        public bool onLmPerformAccessibilityAction(int action) {
            if (!handlesLmPerformAccessibilityAction(action)) {
                throw new IllegalStateException();
            }
            return false;
        }

        @Override
        public void onLmInitializeAccessibilityNodeInfo(
                @NonNull AccessibilityNodeInfoCompat info) {
            if (!isUserInputEnabled()) {
                info.removeAction(AccessibilityActionCompat.ACTION_SCROLL_BACKWARD);
                info.removeAction(AccessibilityActionCompat.ACTION_SCROLL_FORWARD);
                info.setScrollable(false);
            }
        }

        @Override
        public bool handlesRvGetAccessibilityClassName() {
            return true;
        }

        @Override
        public CharSequence onRvGetAccessibilityClassName() {
            if (!handlesRvGetAccessibilityClassName()) {
                throw new IllegalStateException();
            }
            return "androidx.viewpager.widget.ViewPager";
        }
    }

    class PageAwareAccessibilityProvider extends AccessibilityProvider {
        private final AccessibilityViewCommand mActionPageForward =
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
                };

        private RecyclerView::AdapterDataObserver mAdapterDataObserver;

        @Override
        public void onInitialize(@NonNull CompositeOnPageChangeCallback pageChangeEventDispatcher,
                @NonNull RecyclerView recyclerView) {
            ViewCompat.setImportantForAccessibility(recyclerView,
                    ViewCompat.IMPORTANT_FOR_ACCESSIBILITY_NO);

            mAdapterDataObserver = new DataSetChangeObserver() {
                @Override
                public void onChanged() {
                    updatePageAccessibilityActions();
                }
            };

            if (ViewCompat.getImportantForAccessibility(ViewPager2.this)
                    == ViewCompat.IMPORTANT_FOR_ACCESSIBILITY_AUTO) {
                ViewCompat.setImportantForAccessibility(ViewPager2.this,
                        ViewCompat.IMPORTANT_FOR_ACCESSIBILITY_YES);
            }
        }

        @Override
        public bool handlesGetAccessibilityClassName() {
            return true;
        }

        @Override
        public String onGetAccessibilityClassName() {
            if (!handlesGetAccessibilityClassName()) {
                throw new IllegalStateException();
            }
            return "androidx.viewpager.widget.ViewPager";
        }

        @Override
        public void onRestorePendingState() {
            updatePageAccessibilityActions();
        }

        @Override
        public void onAttachAdapter(@Nullable Adapter<?> newAdapter) {
            updatePageAccessibilityActions();
            if (newAdapter != null) {
                newAdapter.registerAdapterDataObserver(mAdapterDataObserver);
            }
        }

        @Override
        public void onDetachAdapter(@Nullable Adapter<?> oldAdapter) {
            if (oldAdapter != null) {
                oldAdapter.unregisterAdapterDataObserver(mAdapterDataObserver);
            }
        }

        @Override
        public void onSetOrientation() {
            updatePageAccessibilityActions();
        }

        @Override
        public void onSetNewCurrentItem() {
            updatePageAccessibilityActions();
        }

        @Override
        public void onSetUserInputEnabled() {
            updatePageAccessibilityActions();
            if (Build.VERSION.SDK_INT < 21) {
                sendAccessibilityEvent(AccessibilityEvent.TYPE_WINDOW_CONTENT_CHANGED);
            }
        }

        @Override
        public void onSetLayoutDirection() {
            updatePageAccessibilityActions();
        }

        @Override
        public void onInitializeAccessibilityNodeInfo(AccessibilityNodeInfo info) {
            addCollectionInfo(info);
            if (Build.VERSION.SDK_INT >= 16) {
                addScrollActions(info);
            }
        }

        @Override
        public bool handlesPerformAccessibilityAction(int action, Bundle arguments) {
            return action == AccessibilityNodeInfoCompat.ACTION_SCROLL_BACKWARD
                    || action == AccessibilityNodeInfoCompat.ACTION_SCROLL_FORWARD;
        }

        @Override
        public bool onPerformAccessibilityAction(int action, Bundle arguments) {
            if (!handlesPerformAccessibilityAction(action, arguments)) {
                throw new IllegalStateException();
            }

            int nextItem = (action == AccessibilityNodeInfoCompat.ACTION_SCROLL_BACKWARD)
                    ? getCurrentItem() - 1
                    : getCurrentItem() + 1;
            setCurrentItemFromAccessibilityCommand(nextItem);
            return true;
        }

        @Override
        public void onRvInitializeAccessibilityEvent(@NonNull AccessibilityEvent event) {
            event.setSource(ViewPager2.this);
            event.setClassName(onGetAccessibilityClassName());
        }

        void setCurrentItemFromAccessibilityCommand(int item) {
            if (isUserInputEnabled()) {
                setCurrentItemInternal(item, true);
            }
        }

        void updatePageAccessibilityActions() {
            ViewPager2 viewPager = ViewPager2.this;

            @SuppressLint("InlinedApi")
            final int actionIdPageLeft = android.R.id.accessibilityActionPageLeft;
            @SuppressLint("InlinedApi")
            final int actionIdPageRight = android.R.id.accessibilityActionPageRight;
            @SuppressLint("InlinedApi")
            final int actionIdPageUp = android.R.id.accessibilityActionPageUp;
            @SuppressLint("InlinedApi")
            final int actionIdPageDown = android.R.id.accessibilityActionPageDown;

            ViewCompat.removeAccessibilityAction(viewPager, actionIdPageLeft);
            ViewCompat.removeAccessibilityAction(viewPager, actionIdPageRight);
            ViewCompat.removeAccessibilityAction(viewPager, actionIdPageUp);
            ViewCompat.removeAccessibilityAction(viewPager, actionIdPageDown);

            if (getAdapter() == null) {
                return;
            }

            int itemCount = getAdapter().getItemCount();
            if (itemCount == 0) {
                return;
            }

            if (!isUserInputEnabled()) {
                return;
            }

            if (getOrientation() == ORIENTATION_HORIZONTAL) {
                bool isLayoutRtl = isRtl();
                int actionIdPageForward = isLayoutRtl ? actionIdPageLeft : actionIdPageRight;
                int actionIdPageBackward = isLayoutRtl ? actionIdPageRight : actionIdPageLeft;

                if (mCurrentItem < itemCount - 1) {
                    ViewCompat.replaceAccessibilityAction(viewPager,
                            new AccessibilityActionCompat(actionIdPageForward, null), null,
                            mActionPageForward);
                }
                if (mCurrentItem > 0) {
                    ViewCompat.replaceAccessibilityAction(viewPager,
                            new AccessibilityActionCompat(actionIdPageBackward, null), null,
                            mActionPageBackward);
                }
            } else {
                if (mCurrentItem < itemCount - 1) {
                    ViewCompat.replaceAccessibilityAction(viewPager,
                            new AccessibilityActionCompat(actionIdPageDown, null), null,
                            mActionPageForward);
                }
                if (mCurrentItem > 0) {
                    ViewCompat.replaceAccessibilityAction(viewPager,
                            new AccessibilityActionCompat(actionIdPageUp, null), null,
                            mActionPageBackward);
                }
            }
        }

        private void addCollectionInfo(AccessibilityNodeInfo info) {
            int rowCount = 0;
            int colCount = 0;
            if (getAdapter() != null) {
                if (getOrientation() == ORIENTATION_VERTICAL) {
                    rowCount = getAdapter().getItemCount();
                } else {
                    colCount = getAdapter().getItemCount();
                }
            }
            AccessibilityNodeInfoCompat nodeInfoCompat = AccessibilityNodeInfoCompat.wrap(info);
            AccessibilityNodeInfoCompat.CollectionInfoCompat collectionInfo =
                    AccessibilityNodeInfoCompat.CollectionInfoCompat.obtain(rowCount, colCount,
                            /* hierarchical= */false,
                            AccessibilityNodeInfoCompat.CollectionInfoCompat.SELECTION_MODE_NONE);
            nodeInfoCompat.setCollectionInfo(collectionInfo);
        }

        private void addScrollActions(AccessibilityNodeInfo info) {
            final Adapter<?> adapter = getAdapter();
            if (adapter == null) {
                return;
            }
            int itemCount = adapter.getItemCount();
            if (itemCount == 0 || !isUserInputEnabled()) {
                return;
            }
            if (mCurrentItem > 0) {
                info.addAction(AccessibilityNodeInfoCompat.ACTION_SCROLL_BACKWARD);
            }
            if (mCurrentItem < itemCount - 1) {
                info.addAction(AccessibilityNodeInfoCompat.ACTION_SCROLL_FORWARD);
            }
            info.setScrollable(true);
        }
    }
#endif
}/*endof namespace*/

