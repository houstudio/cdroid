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
#ifndef __RECYCLER_VIEW_H__
#define __RECYCLER_VIEW_H__
#include <view/viewgroup.h>
#include <widget/observable.h>
#include <widget/nestedscrollview.h>
#include <widget/nestedscrollinghelper.h>
#include <widgetEx/recyclerview/viewboundscheck.h>
#include <widget/linearlayout.h>

namespace cdroid{
class ChildHelper;
class AdapterHelper;
class ViewInfoStore;
class GapWorker;
class GridLayoutManager;
class RecyclerViewAccessibilityDelegate;

class RecyclerView:public ViewGroup{
private:
    //static constexpr bool _Debug=false;
    static constexpr bool FORCE_INVALIDATE_DISPLAY_LIST = false;//Build.VERSION.SDK_INT == 18 || Build.VERSION.SDK_INT == 19 || Build.VERSION.SDK_INT == 20;
    static constexpr bool ALLOW_SIZE_IN_UNSPECIFIED_SPEC = false;//Build.VERSION.SDK_INT >= 23;
    static constexpr bool POST_UPDATES_ON_ANIMATION = true;//Build.VERSION.SDK_INT >= 16;
    static constexpr bool ALLOW_THREAD_GAP_WORK = true;//Build.VERSION.SDK_INT >= 21;
    static constexpr bool FORCE_ABS_FOCUS_SEARCH_DIRECTION = false;//Build.VERSION.SDK_INT <= 15;
    static constexpr bool IGNORE_DETACHED_FOCUSED_CHILD = false;//Build.VERSION.SDK_INT <= 15;
    static constexpr bool DISPATCH_TEMP_DETACH = false;
    static constexpr int INVALID_POINTER = -1;

    static constexpr float FLING_DESTRETCH_FACTOR = 4.f;
    static constexpr float SCROLL_FRICTION = 0.015f;
    static constexpr float INFLEXION = 0.35f; // Tension lines cross at (INFLEXION, 1)
    static constexpr float DECELERATION_RATE = 2.3582018154259448f;//(float) (std::log(0.78) / std::log(0.9));
    class RecyclerViewDataObserver;
    class ViewFlinger;
protected:
    static bool sDebugAssertionsEnabled;
    static bool sVerboseLoggingEnabled;
    class AdapterDataObservable;
    friend class GapWorker;
public:
    static constexpr int HORIZONTAL = LinearLayout::HORIZONTAL;
    static constexpr int VERTICAL = LinearLayout::VERTICAL;
    static constexpr int DEFAULT_ORIENTATION = VERTICAL;
    static constexpr int NO_POSITION = -1;
    static constexpr long NO_ID = -1;
    static constexpr int INVALID_TYPE = -1;
    static constexpr int TOUCH_SLOP_DEFAULT = 0;
    static constexpr int TOUCH_SLOP_PAGING = 1;
    static constexpr int UNDEFINED_DURATION = INT_MIN;
    static constexpr int SCROLL_STATE_IDLE = 0;
    static constexpr int SCROLL_STATE_DRAGGING = 1;
    static constexpr int SCROLL_STATE_SETTLING = 2; 
    static constexpr long FOREVER_NS = LONG_MAX;
public:
    class SmoothScroller;
    class FastScroller;
    class Adapter;
    class ViewHolder;
    class Recycler;
    class State;
    class SavedState;
    class LayoutManager;
    class RecycledViewPool;
    class EdgeEffectFactory;
    class AdapterDataObserver;
    class ItemDecoration;
    class OnItemTouchListener;
    friend GridLayoutManager;
    friend RecyclerViewAccessibilityDelegate;
    DECLARE_UIEVENT(bool,OnFlingListener,int,int);
    class ItemAnimator{
    public:
        class ItemHolderInfo {
        public:
            int left;
            int top;
            int right;
            int bottom;
            int changeFlags;
            ItemHolderInfo();
            ItemHolderInfo* setFrom(RecyclerView::ViewHolder& holder);
            ItemHolderInfo* setFrom(RecyclerView::ViewHolder& holder,int flags);
        };
        /*struct ItemAnimatorListener {
            CallbackBase<void,ViewHolder&> onAnimationFinished;
        };*/
        //ItemAnimator::ItemAnimatorListener
        /*struct ItemAnimatorFinishedListener {
            CallbackBase<void> onAnimationsFinished;
        };*/
        DECLARE_UIEVENT(void,ItemAnimatorListener,ViewHolder&);
        DECLARE_UIEVENT(void,ItemAnimatorFinishedListener);
    private:
        friend RecyclerView;
        ItemAnimatorListener mListener;
        std::vector<ItemAnimatorFinishedListener> mFinishedListeners;
        long mAddDuration = 120;
        long mRemoveDuration = 120;
        long mMoveDuration = 250;
        long mChangeDuration = 250;
    protected:
        void setListener(const ItemAnimatorListener& listener);
        static int buildAdapterChangeFlagsForAnimations(ViewHolder* viewHolder);
    public:
        static constexpr int FLAG_CHANGED = 1<<1;//ViewHolder::FLAG_UPDATE;
        static constexpr int FLAG_REMOVED = 1<<3;//ViewHolder::FLAG_REMOVED;
        static constexpr int FLAG_INVALIDATED = 1<<2;//ViewHolder::FLAG_INVALID;
        static constexpr int FLAG_MOVED = 1<<11;//ViewHolder::FLAG_MOVED;
        static constexpr int FLAG_APPEARED_IN_PRE_LAYOUT = 1<<12;//ViewHolder::FLAG_APPEARED_IN_PRE_LAYOUT;
    public:
        virtual ~ItemAnimator()=default;
        long getMoveDuration();
        void setMoveDuration(long moveDuration);
        long getAddDuration();
        void setAddDuration(long addDuration);
        long getRemoveDuration();
        void setRemoveDuration(long removeDuration);
        long getChangeDuration();
        void setChangeDuration(long changeDuration);
        ItemHolderInfo* recordPreLayoutInformation(State& state,ViewHolder& viewHolder, int changeFlags,std::vector<Object*>& payloads);
        ItemHolderInfo* recordPostLayoutInformation(State& state,ViewHolder& viewHolder);

        virtual bool animateDisappearance(ViewHolder& viewHolder,ItemHolderInfo& preLayoutInfo, ItemHolderInfo* postLayoutInfo)=0;
        virtual bool animateAppearance(ViewHolder& viewHolder,ItemHolderInfo* preLayoutInfo, ItemHolderInfo& postLayoutInfo)=0;
        virtual bool animatePersistence(ViewHolder& viewHolder,ItemHolderInfo& preLayoutInfo, ItemHolderInfo& postLayoutInfo)=0;
        virtual bool animateChange(ViewHolder& oldHolder,ViewHolder& newHolder,ItemHolderInfo& preLayoutInfo,ItemHolderInfo& postLayoutInfo)=0;
        virtual void runPendingAnimations()=0;
        virtual void endAnimation(ViewHolder& item)=0;
        virtual void endAnimations()=0;
        virtual bool isRunning()=0;
        void dispatchAnimationFinished(ViewHolder& viewHolder);/*final*/
        virtual void onAnimationFinished(ViewHolder& viewHolder);
        void dispatchAnimationStarted(ViewHolder& viewHolder);/*final*/
        virtual void onAnimationStarted(ViewHolder& viewHolder);
        bool isRunning(ItemAnimatorFinishedListener listener);/*final*/
        virtual bool canReuseUpdatedViewHolder(ViewHolder& viewHolder);
        virtual bool canReuseUpdatedViewHolder(ViewHolder& viewHolder,std::vector<Object*>& payloads);
        void dispatchAnimationsFinished();/*final*/
        ItemHolderInfo* obtainHolderInfo();
    };
    class LayoutParams:public ViewGroup::MarginLayoutParams{
    protected:
        friend RecyclerView;
        RecyclerView::ViewHolder* mViewHolder;
        Rect mDecorInsets;
        bool mInsetsDirty = true;
        bool mPendingInvalidate = false;
    public:
        LayoutParams(Context* c,const AttributeSet& attrs);
        LayoutParams(int width, int height);
        LayoutParams(const ViewGroup::MarginLayoutParams& source);
        LayoutParams(const ViewGroup::LayoutParams& source);
        LayoutParams(const LayoutParams& source);
        bool viewNeedsUpdate();
        bool isViewInvalid();
        bool isItemRemoved();
        bool isItemChanged();
        //[[deprecated("getViewPosition is deprecated use getViewAdapterPosition PLS.")]]
        int getViewLayoutPosition();
        //[[deprecated("getViewAdapterPosition is deprecated use getBindingAdapterPosition PLS.")]]
        int getAbsoluteAdapterPosition();
        int getBindingAdapterPosition();
    };
public:/*public classes*/
    class OnScrollListener:public EventSet{
    public:
        CallbackBase<void,RecyclerView&,int>onScrollStateChanged;//(RecyclerView& recyclerView, int newState){};
        CallbackBase<void,RecyclerView&,int,int>onScrolled;//(RecyclerView& recyclerView, int dx, int dy){};
    };
    typedef CallbackBase<void,ViewHolder&>RecyclerListener;// onViewRecycled(ViewHolder& holder);
    struct OnChildAttachStateChangeListener {
        CallbackBase<void,View&>onChildViewAttachedToWindow;
        CallbackBase<void,View&>onChildViewDetachedFromWindow;
    };
    typedef CallbackBase<int,int,int>ChildDrawingOrderCallback;
    typedef std::function<View*(Recycler&,int,int)>ViewCacheExtension;
    /*class ViewCacheExtension {
    public:
        virtual View* getViewForPositionAndType(Recycler& recycler, int position,int type)=0;
    };*/

private:/*private variables*/
    int mInterceptRequestLayoutDepth;
    int mEatenAccessibilityChangeFlags;
    AccessibilityManager* mAccessibilityManager;
    std::vector<OnChildAttachStateChangeListener> mOnChildAttachStateListeners;
    int mLayoutOrScrollCounter = 0;
    int mDispatchScrollCounter = 0;
    EdgeEffectFactory* mEdgeEffectFactory;
    EdgeEffect* mLeftGlow, *mTopGlow, *mRightGlow, *mBottomGlow;    

    int mScrollState = SCROLL_STATE_IDLE;
    int mScrollPointerId = INVALID_POINTER;
    VelocityTracker* mVelocityTracker;
    int mInitialTouchX;
    int mInitialTouchY;
    int mLastTouchX;
    int mLastTouchY;
    int mTouchSlop;
    int mMinFlingVelocity;
    int mMaxFlingVelocity;
    OnFlingListener mOnFlingListener;
    float mPhysicalCoef;
    // This value is used when handling rotary encoder generic motion events.
    bool mIgnoreMotionEventTillDown;
    bool mPreserveFocusAfterLayout = true;
    bool mLastAutoMeasureSkippedDueToExact;
    bool mLowResRotaryEncoderFeature;
    bool mClipToPadding;

    OnScrollListener mScrollListener;
    std::vector<OnScrollListener> mScrollListeners;
    ItemAnimator::ItemAnimatorListener mItemAnimatorListener;
    ChildDrawingOrderCallback mChildDrawingOrderCallback;
    NestedScrollingChildHelper* mScrollingChildHelper;
    ScrollFeedbackProvider* mScrollFeedbackProvider;
 
    int mMinMaxLayoutPositions[2];
    int mScrollOffset[2];
    int mScrollConsumed[2];
    int mNestedOffsets[2];
    int mScrollStepConsumed[2];
    int mReusableIntPair[2];
    int mLastAutoMeasureNonExactMeasuredWidth = 0;
    int mLastAutoMeasureNonExactMeasuredHeight = 0;
    
    RecyclerViewDataObserver* mObserver;
    Recycler* mRecycler;
    SavedState* mPendingSavedState;
    AdapterHelper* mAdapterHelper;
    ChildHelper* mChildHelper;
    ViewInfoStore* mViewInfoStore;
    Runnable mItemAnimatorRunner;
    Runnable mUpdateChildViewsRunnable;
    void*/*ViewInfoStore_ProcessCallback*/ mViewInfoProcessCallback;
private:
    void initRecyclerView();
    void doItemAnimator();
    void doUpdateChildViews();
    void doAnimatorFinished(ViewHolder&holder);/*binding for ItemAnimatorRestoreListerner::onAnimationFinished*/
    void dispatchUpdate(void*/*AdapterHelper::UpdateOp*/ op);
    void initAutofill();
    void createLayoutManager(Context* context,const std::string& className,
            const AttributeSet& attrs/*,int defStyleAttr, int defStyleRes*/);
    std::string getFullClassName(Context* context,const std::string& className);
    void initChildrenHelper();
    void setAdapterInternal(Adapter* adapter, bool compatibleWithPrevious,bool removeAndRecycleViews);
    void addAnimatingView(ViewHolder& viewHolder);
    bool hasUpdatedView();
    float getSplineFlingDistance(int velocity);
    bool flingNoThresholdCheck(int velocityX, int velocityY);
    bool fling(int velocityX, int velocityY, int minFlingVelocity, int maxFlingVelocity);
    void startNestedScrollForType(int type);
    void nestedScrollByInternal(
            int x,int y,int horizontalAxis,int verticalAxis,
            MotionEvent* motionEvent,int type);
    bool shouldAbsorb(EdgeEffect* edgeEffect, int velocity, int size);
    int consumeFlingInStretch(int unconsumed, EdgeEffect* startGlow, EdgeEffect* endGlow,int size);
    void stopScrollersInternal();
    void pullGlows(MotionEvent* ev,
            float x,int horizontalAxis,float overscrollX,
            float y,int verticalAxis,float overscrollY);
    int releaseHorizontalGlow(int deltaX, float y);
    int releaseVerticalGlow(int deltaY, float x);
    void releaseGlows();

    bool isPreferredNextFocus(View* focused, View* next, int direction);
    void requestChildOnScreen(View* child,View* focused);
    void resetScroll();
    void cancelScroll();
    void onPointerUp(MotionEvent& e);
    void dispatchContentChangedIfNecessary();
    bool predictiveItemAnimationsEnabled();
    void processAdapterUpdatesAndSetAnimationFlags();
    void saveFocusInfo();
    void resetFocusInfo();
    View* findNextViewToFocus();
    void recoverFocusFromState();
    int getDeepestFocusedViewWithId(View* view);

    void dispatchLayoutStep1();
    void dispatchLayoutStep2();
    void dispatchLayoutStep3();

    void handleMissingPreInfoForChangeError(long key,ViewHolder* holder, ViewHolder* oldChangeViewHolder);
    void findMinMaxChildLayoutPositions(int*into);
    bool dispatchToOnItemTouchListeners(MotionEvent& e);
    bool findInterceptingOnItemTouchListener(MotionEvent& e);
    bool stopGlowAnimations(MotionEvent& e);
    bool didChildRangeChange(int minPositionPreLayout, int maxPositionPreLayout);
    void animateChange(ViewHolder& oldHolder,ViewHolder& newHolder,ItemAnimator::ItemHolderInfo& preInfo,
	     ItemAnimator::ItemHolderInfo& postInfo,bool oldHolderDisappearing, bool newHolderDisappearing);
    NestedScrollingChildHelper* getScrollingChildHelper();
    ScrollFeedbackProvider* getScrollFeedbackProvider();
protected:
    static constexpr int MAX_SCROLL_DURATION = 2000;
    Rect mTempRect;
    Rect mTempRect2;
    RectF mTempRectF;
    Adapter* mAdapter;
    LayoutManager* mLayout;
    RecyclerListener mRecyclerListener;
    OnItemTouchListener* mInterceptingOnItemTouchListener;
    std::vector<RecyclerListener>mRecyclerListeners;
    std::vector<ItemDecoration*> mItemDecorations;
    std::vector<OnItemTouchListener> mOnItemTouchListeners;
    bool mIsAttached;
    bool mHasFixedSize;
    bool mEnableFastScroller;
    bool mFirstLayoutComplete;
    bool mLayoutWasDefered;
    bool mLayoutSuppressed;
    bool mAdapterUpdateDuringMeasure;
    bool mDataSetHasChangedAfterLayout = false;
    bool mDispatchItemsChangedEvent = false;
 // For use in item animations
    bool mItemsAddedOrRemoved = false;
    bool mItemsChanged = false;
    bool mPostedAnimatorRunner = false;
    // This value is used when handling rotary encoder generic motion events.
    float mScaledHorizontalScrollFactor;
    float mScaledVerticalScrollFactor;

    ItemAnimator* mItemAnimator;
    ViewFlinger* mViewFlinger;
    GapWorker* mGapWorker;
    /*GapWorker::LayoutPrefetchRegistryImpl*/void* mPrefetchRegistry;
    State* mState;
    RecyclerViewAccessibilityDelegate* mAccessibilityDelegate;
    std::vector<ViewHolder*> mPendingAccessibilityImportanceChange;

    void initAdapterManager();
    Parcelable* onSaveInstanceState()override;
    void onRestoreInstanceState(Parcelable& state)override;
    void dispatchSaveInstanceState(SparseArray<Parcelable*>& container)override;
    void dispatchRestoreInstanceState(SparseArray<Parcelable*>& container)override;
    bool removeAnimatingView(View* view);
    void consumePendingUpdateOperations();
    int consumeFlingInHorizontalStretch(int unconsumedX);
    int consumeFlingInVerticalStretch(int unconsumedY);
    bool scrollByInternal(int x, int y,int horizontlAxis,int verticalAxis, MotionEvent* ev,int type);
    void startInterceptRequestLayout();
    void stopInterceptRequestLayout(bool performLayoutChildren);
    void considerReleasingGlowsOnScroll(int dx, int dy);
    void absorbGlows(int velocityX, int velocityY);
    void ensureLeftGlow();
    void ensureRightGlow();
    void ensureTopGlow();
    void ensureBottomGlow();
    void invalidateGlows();

    bool onRequestFocusInDescendants(int direction, Rect* previouslyFocusedRect)override;
    void onAttachedToWindow()override;
    void onDetachedFromWindow()override;

    void assertInLayoutOrScroll(const std::string& message);
    void assertNotInLayoutOrScroll(const std::string& message);

    void onMeasure(int widthSpec, int heightSpec)override;
    void defaultOnMeasure(int widthSpec, int heightSpec);
    void onSizeChanged(int w, int h, int oldw, int oldh)override;
    void onEnterLayoutOrScroll();
    void onExitLayoutOrScroll();
    void onExitLayoutOrScroll(bool enableChangeEvents);
    bool isAccessibilityEnabled();
    bool shouldDeferAccessibilityEvent(AccessibilityEvent& event);
    void postAnimationRunner();
    void dispatchLayout();
    void fillRemainingScrollValues(State& state);

    void recordAnimationInfoIfBouncedHiddenView(ViewHolder* viewHolder,ItemAnimator::ItemHolderInfo* animationInfo);
    void removeDetachedView(View* child, bool animate)override;
    long getChangedHolderKey(ViewHolder& holder);
    void animateAppearance(ViewHolder& itemHolder,ItemAnimator::ItemHolderInfo* preLayoutInfo,ItemAnimator::ItemHolderInfo& postLayoutInfo);
    void animateDisappearance(ViewHolder& holder,ItemAnimator::ItemHolderInfo& preLayoutInfo,ItemAnimator::ItemHolderInfo* postLayoutInfo);
    void onLayout(bool changed, int l, int t, int w, int h)override;
    void markItemDecorInsetsDirty();

    bool checkLayoutParams(const ViewGroup::LayoutParams* p)const override;
    LayoutParams* generateDefaultLayoutParams()const override;
    LayoutParams* generateLayoutParams(const ViewGroup::LayoutParams* p)const override;

    void saveOldPositions();
    void clearOldPositions();
    void offsetPositionRecordsForMove(int from, int to);
    void offsetPositionRecordsForInsert(int positionStart, int itemCount);
    void offsetPositionRecordsForRemove(int positionStart, int itemCount,bool applyToPreLayout);
    void viewRangeUpdate(int positionStart, int itemCount, Object* payload);
    bool canReuseUpdatedViewHolder(ViewHolder& viewHolder);
    void processDataSetCompletelyChanged(bool dispatchItemsChanged);
    void markKnownViewsInvalid();

    ViewHolder* findViewHolderForPosition(int position, bool checkNewPosition);
    Rect getItemDecorInsetsForChild(View* child);
    void scrollStep(int dx, int dy,int* consumed);
    void dispatchOnScrolled(int hresult, int vresult);
    void dispatchOnScrollStateChanged(int state);

    void repositionShadowingViews();
    static RecyclerView*findNestedRecyclerView(View* view);
    static void clearNestedRecyclerViewIfNotNested(ViewHolder& holder);
    int64_t getNanoTime();
    void dispatchChildDetached(View* child);
    void dispatchChildAttached(View* child);
    bool setChildImportantForAccessibilityInternal(ViewHolder* viewHolder,int importantForAccessibility);
    void dispatchPendingImportantForAccessibilityChanges();
    int getAdapterPositionInRecyclerView(ViewHolder* viewHolder)const;
    void initFastScroller(StateListDrawable* verticalThumbDrawable, Drawable* verticalTrackDrawable, 
             StateListDrawable* horizontalThumbDrawable, Drawable* horizontalTrackDrawable,const AttributeSet&);
    int getChildDrawingOrder(int childCount, int i)override;
public:
    RecyclerView(int w,int h);
    RecyclerView(Context* context,const AttributeSet& attrs);
    ~RecyclerView()override;
    RecyclerViewAccessibilityDelegate* getCompatAccessibilityDelegate();
    void setAccessibilityDelegate(RecyclerViewAccessibilityDelegate* accessibilityDelegate);
    void setHasFixedSize(bool hasFixedSize);
    bool hasFixedSize()const;
    void setClipToPadding(bool clipToPadding)override;
    bool getClipToPadding()const override;
    void setScrollingTouchSlop(int slopConstant);
    void swapAdapter(Adapter* adapter, bool removeAndRecycleExistingViews);
    void setAdapter(Adapter* adapter);
    Adapter* getAdapter();
    void removeAndRecycleViews();
    void setRecyclerListener(const RecyclerListener& listener);
    void addRecyclerListener(const RecyclerListener& listener);
    void removeRecyclerListener(const RecyclerListener& listener);

    int getBaseline()override;
    void addOnChildAttachStateChangeListener(const OnChildAttachStateChangeListener& listener);
    void removeOnChildAttachStateChangeListener(const OnChildAttachStateChangeListener& listener);
    void clearOnChildAttachStateChangeListeners();
    void setLayoutManager(LayoutManager* layout);
    LayoutManager* getLayoutManager();
    void setOnFlingListener(const OnFlingListener& onFlingListener);
    OnFlingListener getOnFlingListener();
    RecycledViewPool& getRecycledViewPool();
    void setRecycledViewPool(RecycledViewPool* pool);
    void setViewCacheExtension(const ViewCacheExtension& extension);
    void setItemViewCacheSize(int size);
    int getScrollState()const;
    void setScrollState(int state);
    void addItemDecoration(ItemDecoration* decor, int index);
    void addItemDecoration(ItemDecoration* decor);
    ItemDecoration* getItemDecorationAt(int index);
    int getItemDecorationCount()const;
    void removeItemDecorationAt(int index);
    void removeItemDecoration(ItemDecoration* decor);
    void setChildDrawingOrderCallback(ChildDrawingOrderCallback childDrawingOrderCallback);
    void setOnScrollListener(const OnScrollListener& listener);
    void addOnScrollListener(const OnScrollListener& listener);
    void removeOnScrollListener(const OnScrollListener& listener);
    void clearOnScrollListeners();
    void scrollToPosition(int position);
    void jumpToPositionForSmoothScroller(int position);
    void smoothScrollToPosition(int position);
    void scrollTo(int x, int y)override;
    void scrollBy(int x, int y)override;
    void nestedScrollBy(int x, int y);
    bool dispatchKeyEvent(KeyEvent&)override;
    int computeHorizontalScrollOffset()override;
    int computeHorizontalScrollExtent()override;
    int computeHorizontalScrollRange()override;
    int computeVerticalScrollOffset()override;
    int computeVerticalScrollExtent()override;
    int computeVerticalScrollRange()override;
    void suppressLayout(bool suppress);
    bool isLayoutSuppressed()const;
    void smoothScrollBy(int dx,int dy);
    void smoothScrollBy(int dx,int dy,const Interpolator* interpolator);
    void smoothScrollBy(int dx,int dy,const Interpolator* interpolator,int duration);
    void smoothScrollBy(int dx,int dy,const Interpolator* interpolator,int duration,bool withtNestedScrolling);
    bool fling(int velocityX, int velocityY);
    void stopScroll();
    int getMinFlingVelocity()const;
    int getMaxFlingVelocity()const;
    void setEdgeEffectFactory(EdgeEffectFactory* edgeEffectFactory);
    EdgeEffectFactory* getEdgeEffectFactory();
    View* focusSearch(View* focused, int direction)override;
    void requestChildFocus(View* child, View* focused)override;
    bool requestChildRectangleOnScreen(View* child, Rect& rect, bool immediate)override;
    void addFocusables(std::vector<View*>& views, int direction, int focusableMode)override;
    bool isAttachedToWindow()const override;

    void addOnItemTouchListener(OnItemTouchListener listener);
    void removeOnItemTouchListener(OnItemTouchListener listener);

    bool onInterceptTouchEvent(MotionEvent& e)override;
    void requestDisallowInterceptTouchEvent(bool disallowIntercept)override;
    bool onTouchEvent(MotionEvent& e)override;
    bool onGenericMotionEvent(MotionEvent& event)override;
    void setItemAnimator(ItemAnimator* animator);
    ItemAnimator* getItemAnimator();
    bool isComputingLayout();
    void sendAccessibilityEventUnchecked(AccessibilityEvent& event)override;
    bool dispatchPopulateAccessibilityEvent(AccessibilityEvent& event)override;
    void requestLayout()override;
    void draw(Canvas& c)override;
    void onDraw(Canvas& c)override;
    LayoutParams* generateLayoutParams(const AttributeSet& attrs)const override;
    bool isAnimating();
    void invalidateItemDecorations();
    bool getPreserveFocusAfterLayout()const;
    void setPreserveFocusAfterLayout(bool preserveFocusAfterLayout);
    ViewHolder* getChildViewHolder(View* child);
    View* findContainingItemView(View* view);
    ViewHolder* findContainingViewHolder(View* view);
    static ViewHolder* getChildViewHolderInt(View* child);
    int getChildPosition(View* child);
    int getChildAdapterPosition(View* child);
    int getChildLayoutPosition(View* child);
    long getChildItemId(View* child);
    ViewHolder* findViewHolderForPosition(int position);
    ViewHolder* findViewHolderForLayoutPosition(int position);
    ViewHolder* findViewHolderForAdapterPosition(int position);
    ViewHolder* findViewHolderForItemId(long id);
    View* findChildViewUnder(float x, float y);
    bool drawChild(Canvas& canvas, View* child, int64_t drawingTime)override;
    void offsetChildrenVertical(int dy);
    void onChildAttachedToWindow(View* child);
    void onChildDetachedFromWindow(View* child);
    void offsetChildrenHorizontal(int dx);
    void getDecoratedBoundsWithMargins(View*view,Rect& outBounds)const;
    static void getDecoratedBoundsWithMarginsInt(View* view, Rect& outBounds);
    void onScrolled(int dx,int dy);
    void onScrollStateChanged(int state);
    bool hasPendingAdapterUpdates();

    //override of NestedScrollingChild 
    void setNestedScrollingEnabled(bool enabled);
    bool isNestedScrollingEnabled();
    bool startNestedScroll(int axes);
    bool startNestedScroll(int axes, int type);
    void stopNestedScroll();
    void stopNestedScroll(int type);
    bool hasNestedScrollingParent();
    bool hasNestedScrollingParent(int type);
    bool dispatchNestedScroll(int dxConsumed, int dyConsumed, int dxUnconsumed,int dyUnconsumed, int offsetInWindow[2])override;
    bool dispatchNestedScroll(int dxConsumed, int dyConsumed, int dxUnconsumed,int dyUnconsumed, int offsetInWindow[2], int type);
    bool dispatchNestedScroll(int dxConsumed, int dyConsumed, int dxUnconsumed,int dyUnconsumed, int offsetInWindow[2], int type,int consumed[2]);
    bool dispatchNestedPreScroll(int dx, int dy, int consumed[2], int offsetInWindow[2])override;
    bool dispatchNestedPreScroll(int dx, int dy, int consumed[2], int offsetInWindow[2],int type);
    bool dispatchNestedFling(float velocityX, float velocityY, bool consumed)override;
    bool dispatchNestedPreFling(float velocityX, float velocityY)override;
};

class RecyclerView::ViewFlinger{
private:
    friend RecyclerView;
    int mLastFlingX;
    int mLastFlingY;
    bool mEatRunOnAnimationRequest=false;
    bool mReSchedulePostAnimationCallback = false;
    OverScroller* mOverScroller;
    const Interpolator* mInterpolator;
    RecyclerView* mRV;
    Runnable mRunnable;
    int computeScrollDuration(int dx, int dy);
    void internalPostOnAnimation();
public:
    ViewFlinger(RecyclerView*v);
    ~ViewFlinger();
    void run();
    void disableRunOnAnimationRequests();
    //void enableRunOnAnimationRequests();
    void postOnAnimation();
    void fling(int velocityX, int velocityY);
    void smoothScrollBy(int dx, int dy, int duration,const Interpolator* interpolator);
    void stop();
};

class RecyclerView::AdapterDataObserver{
public:
    virtual void onChanged();
    virtual void onItemRangeChanged(int positionStart, int itemCount);
    virtual void onItemRangeChanged(int positionStart, int itemCount,Object* payload);
    virtual void onItemRangeInserted(int positionStart, int itemCount);
    virtual void onItemRangeRemoved(int positionStart, int itemCount);
    virtual void onItemRangeMoved(int fromPosition, int toPosition, int itemCount);
    virtual void onStateRestorationPolicyChanged();
};

class RecyclerView::Adapter{
public:
    enum StateRestorationPolicy {
        /**
         * Adapter is ready to restore State immediately, RecyclerView will provide the state
         * to the LayoutManager in the next layout pass.
         */
        ALLOW,
        /**
         * Adapter is ready to restore State when it has more than 0 items. RecyclerView will
         * provide the state to the LayoutManager as soon as the Adapter has 1 or more items.
         */
        PREVENT_WHEN_EMPTY,
        /**
         * RecyclerView will not restore the state for the Adapter until a call to
         * {@link #setStateRestorationPolicy(StateRestorationPolicy)} is made with either
         * {@link #ALLOW} or {@link #PREVENT_WHEN_EMPTY}.
         */
        PREVENT
    };
private:
    AdapterDataObservable* mObservable;
    bool mHasStableIds;
    StateRestorationPolicy mStateRestorationPolicy = StateRestorationPolicy::ALLOW;
public:
    Adapter();
    virtual ~Adapter();
    virtual ViewHolder*onCreateViewHolder(ViewGroup* parent, int viewType)=0;
    virtual void onBindViewHolder(ViewHolder& holder, int position)=0;
    void onBindViewHolder(ViewHolder& holder, int position,std::vector<Object*>& payloads);
    int findRelativeAdapterPositionIn(Adapter&,ViewHolder&,int localPosition);
    ViewHolder*createViewHolder(ViewGroup* parent, int viewType);
    void bindViewHolder(ViewHolder& holder, int position);
    virtual int getItemViewType(int position);
    void setHasStableIds(bool hasStableIds);
    virtual long getItemId(int position);
    virtual int getItemCount()=0;
    virtual bool hasStableIds();
    virtual void onViewRecycled(ViewHolder& holder);
    virtual bool onFailedToRecycleView(ViewHolder& holder);
    virtual void onViewAttachedToWindow(ViewHolder& holder);
    virtual void onViewDetachedFromWindow(ViewHolder& holder);
    bool hasObservers()const;
    void registerAdapterDataObserver(AdapterDataObserver* observer);
    void unregisterAdapterDataObserver(AdapterDataObserver* observer);
    virtual void onAttachedToRecyclerView(RecyclerView& recyclerView);
    virtual void onDetachedFromRecyclerView(RecyclerView& recyclerView);
    virtual void notifyDataSetChanged()final;
    virtual void notifyItemChanged(int position)final;
    virtual void notifyItemChanged(int position,Object* payload)final;
    virtual void notifyItemRangeChanged(int positionStart, int itemCount)final;
    virtual void notifyItemRangeChanged(int positionStart, int itemCount,Object* payload)final;
    virtual void notifyItemInserted(int position)final;
    virtual void notifyItemMoved(int fromPosition, int toPosition)final;
    virtual void notifyItemRangeInserted(int positionStart, int itemCount)final;
    virtual void notifyItemRemoved(int position)final;
    virtual void notifyItemRangeRemoved(int positionStart, int itemCount)final;
    void setStateRestorationPolicy(StateRestorationPolicy);
    StateRestorationPolicy getStateRestorationPolicy()const;
    bool canRestoreState();
};
class RecyclerView::ItemDecoration{
public:
    virtual ~ItemDecoration()=default;
    virtual void onDraw(Canvas& c,RecyclerView& parent,State& state);
    virtual void onDrawOver(Canvas& c,RecyclerView& parent,State& state);
    virtual void getItemOffsets(Rect& outRect, View& view,RecyclerView& parent, State& state);
};
class RecyclerView::OnItemTouchListener:public EventSet{
public:
    CallbackBase<bool,RecyclerView&,MotionEvent&>onInterceptTouchEvent;//(RecyclerView& rv,MotionEvent& e);
    CallbackBase<void,RecyclerView&,MotionEvent&>onTouchEvent;//(RecyclerView& rv,MotionEvent& e);
    CallbackBase<void,bool>onRequestDisallowInterceptTouchEvent;//(bool disallowIntercept);
};

class RecyclerView::LayoutManager{
private:
    friend GapWorker;
    friend RecyclerView;
    friend ViewInfoStore;
    friend RecyclerViewAccessibilityDelegate;
    bool mMeasurementCacheEnabled = true;
    bool mItemPrefetchEnabled = true;
    int mWidthMode, mHeightMode;
    int mWidth, mHeight;
    ViewBoundsCheck::Callback mHorizontalBoundCheckCallback;
    ViewBoundsCheck::Callback mVerticalBoundCheckCallback;
private:
    void addViewInt(View* child, int index, bool disappearing);
    void detachViewInternal(int index,View* view);
    void scrapOrRecycleView(Recycler& recycler, int index, View* view);
    static bool isMeasurementUpToDate(int childSize, int spec, int dimension);
    void getChildRectangleOnScreenScrollAmount(View& child,const Rect& rect,int out[2]);
    bool isFocusedChildVisibleAfterScrolling(RecyclerView& parent, int dx, int dy);
    void onSmoothScrollerStopped(SmoothScroller* smoothScroller);
protected:
    ChildHelper* mChildHelper;
    RecyclerView* mRecyclerView;
    ViewBoundsCheck* mHorizontalBoundCheck;
    ViewBoundsCheck* mVerticalBoundCheck;
    SmoothScroller* mSmoothScroller;
    bool mRequestedSimpleAnimations = false;
    bool mIsAttachedToWindow = false;
    bool mAutoMeasure = false;
    int mPrefetchMaxCountObserved;
    bool mPrefetchMaxObservedInInitialPrefetch;
protected:
    void setRecyclerView(RecyclerView* recyclerView);
    void setMeasureSpecs(int wSpec, int hSpec);
    void setMeasuredDimensionFromChildren(int widthSpec, int heightSpec);
    void dispatchAttachedToWindow(RecyclerView& view);
    void dispatchDetachedFromWindow(RecyclerView& view, Recycler& recycler);
    void removeAndRecycleScrapInt(Recycler& recycler);
    bool shouldReMeasureChild(View* child, int widthSpec, int heightSpec,const LayoutParams* lp);
    bool shouldMeasureChild(View* child, int widthSpec, int heightSpec,const LayoutParams* lp);
    void stopSmoothScroller();
    virtual void onInitializeAccessibilityNodeInfo(AccessibilityNodeInfo& info);
    virtual void onInitializeAccessibilityNodeInfoForItem(View* host, AccessibilityNodeInfo& info);
    virtual bool performAccessibilityAction(int action, Bundle* args);
    virtual bool performAccessibilityActionForItem(View& view, int action,Bundle* args);
    void setExactMeasureSpecsFrom(RecyclerView* recyclerView);
    virtual bool shouldMeasureTwice();
    bool hasFlexibleChildInBothOrientations();
public:
    class LayoutPrefetchRegistry{
    public:
        virtual void addPosition(int layoutPosition, int pixelDistance)=0;
    };
    struct Properties {
        /** @attr ref androidx.recyclerview.R.styleable#RecyclerView_android_orientation */
        int orientation;
        /** @attr ref androidx.recyclerview.R.styleable#RecyclerView_spanCount */
        int spanCount;
        /** @attr ref androidx.recyclerview.R.styleable#RecyclerView_reverseLayout */
        bool reverseLayout;
        /** @attr ref androidx.recyclerview.R.styleable#RecyclerView_stackFromEnd */
        bool stackFromEnd;
    };
public:
    LayoutManager();
    virtual ~LayoutManager();
    virtual void setMeasuredDimension(Rect& childrenBounds, int wSpec, int hSpec);
    void requestLayout();
    void assertInLayoutOrScroll(const std::string& message);
    static int chooseSize(int spec, int desired, int min);
    virtual void assertNotInLayoutOrScroll(const std::string& message);
    void setAutoMeasureEnabled(bool enabled);
    virtual bool isAutoMeasureEnabled()const;
    virtual bool supportsPredictiveItemAnimations();
    void setItemPrefetchEnabled(bool enabled);
    bool isItemPrefetchEnabled();
    virtual void collectAdjacentPrefetchPositions(int dx, int dy, State& state, LayoutPrefetchRegistry& layoutPrefetchRegistry);
    virtual void collectInitialPrefetchPositions(int adapterItemCount, LayoutPrefetchRegistry& layoutPrefetchRegistry);
    bool isAttachedToWindow();
    void postOnAnimation(Runnable& action);
    bool removeCallbacks(Runnable& action);
    virtual void onAttachedToWindow(RecyclerView& view);
    virtual void onDetachedFromWindow(RecyclerView& view);
    virtual void onDetachedFromWindow(RecyclerView& view, Recycler& recycler);
    bool getClipToPadding();
    virtual void onLayoutChildren(Recycler& recycler, State& state);
    virtual void onLayoutCompleted(State& state);
    virtual LayoutParams* generateDefaultLayoutParams()const=0;
    virtual bool checkLayoutParams(const LayoutParams* lp)const;
    virtual LayoutParams* generateLayoutParams(const ViewGroup::LayoutParams& lp)const;
    virtual LayoutParams* generateLayoutParams(Context* c,const AttributeSet& attrs)const;
    virtual int scrollHorizontallyBy(int dx, Recycler& recycler, State& state);
    virtual int scrollVerticallyBy(int dy, Recycler& recycler, State& state);
    virtual bool canScrollHorizontally()const;
    virtual bool canScrollVertically()const;
    virtual void scrollToPosition(int position);
    virtual void smoothScrollToPosition(RecyclerView& recyclerView, State& state,int position);
    virtual bool computeScrollVectorForPosition(int targetPosition,PointF&scrollVector);
    void startSmoothScroll(SmoothScroller* smoothScroller);
    bool isSmoothScrolling() const;
    int getLayoutDirection() const;
    virtual bool isLayoutReversed()const;
    void endAnimation(View* view);
    void addDisappearingView(View* child);
    void addDisappearingView(View* child, int index);
    void addView(View* child);
    void addView(View* child, int index);
    void removeView(View* child);
    void removeViewAt(int index);
    void removeAllViews();
    int getBaseline();
    int getPosition(View* view);
    int getItemViewType(View* view);
    virtual View* findContainingItemView(View* view);
    virtual View* findViewByPosition(int position);
    void detachView(View* child);
    void detachViewAt(int index);
    void attachView(View* child, int index, RecyclerView::LayoutParams* lp);
    void attachView(View* child, int index);
    void attachView(View* child);
    void removeDetachedView(View* child);
    void moveView(int fromIndex, int toIndex);
    void detachAndScrapView(View* child, Recycler& recycler);
    void detachAndScrapViewAt(int index,Recycler& recycler);
    void removeAndRecycleView(View* child, Recycler& recycler);
    void removeAndRecycleViewAt(int index,Recycler& recycler);
    int getChildCount()const;
    View* getChildAt(int index);
    int getWidthMode()const;
    int getHeightMode()const;
    int getWidth()const;
    int getHeight()const;
    int getPaddingLeft();
    int getPaddingTop();
    int getPaddingRight();
    int getPaddingBottom();
    int getPaddingStart();
    int getPaddingEnd();
    bool isFocused()const;
    bool hasFocus()const;
    View* getFocusedChild();
    int getItemCount();
    virtual void offsetChildrenHorizontal(int dx);
    virtual void offsetChildrenVertical(int dy);
    void ignoreView(View* view);
    void stopIgnoringView(View* view);
    void detachAndScrapAttachedViews(Recycler& recycler);
    virtual void measureChild(View* child, int widthUsed, int heightUsed);
    bool isMeasurementCacheEnabled()const;
    void setMeasurementCacheEnabled(bool measurementCacheEnabled);
    virtual void measureChildWithMargins(View* child, int widthUsed, int heightUsed);
    static int getChildMeasureSpec(int parentSize, int padding, int childDimension,bool canScroll);
    static int getChildMeasureSpec(int parentSize, int parentMode, int padding,int childDimension, bool canScroll);
    int getDecoratedMeasuredWidth(View* child);
    int getDecoratedMeasuredHeight(View* child);
    void layoutDecorated(View* child, int left, int top, int width, int height);
    void layoutDecoratedWithMargins(View* child, int left, int top, int width,int height);
    void getTransformedBoundingBox(View* child, bool includeDecorInsets,Rect& out);
    void getDecoratedBoundsWithMargins(View* view,Rect& outBounds) const;
    int getDecoratedLeft(View* child) const;
    int getDecoratedTop(View* child) const;
    int getDecoratedRight(View* child) const;
    int getDecoratedBottom(View* child) const;
    void calculateItemDecorationsForChild(View* child,Rect& outRect);
    int getTopDecorationHeight(View* child) const;
    int getBottomDecorationHeight(View* child) const;
    int getLeftDecorationWidth(View* child) const;
    int getRightDecorationWidth(View* child) const;
    virtual View* onFocusSearchFailed(View* focused, int direction,Recycler& recycler,State& state);
    virtual View* onInterceptFocusSearch(View* focused, int direction);
    virtual bool requestChildRectangleOnScreen(RecyclerView& parent,View& child,const Rect& rect, bool immediate);
    virtual bool requestChildRectangleOnScreen(RecyclerView& parent,View& child,const Rect& rect, bool immediate,bool focusedChildVisible);
    bool isViewPartiallyVisible(View* child, bool completelyVisible,bool acceptEndPointInclusion);
    virtual bool onRequestChildFocus(RecyclerView& parent, View& child,View* focused);
    virtual bool onRequestChildFocus(RecyclerView& parent, State& state,View& child, View* focused);
    virtual void onAdapterChanged(Adapter* oldAdapter, Adapter* newAdapter);
    virtual bool onAddFocusables(RecyclerView& recyclerView,std::vector<View*>& views, int direction, int focusableMode);
    virtual void onItemsChanged(RecyclerView& recyclerView);
    virtual void onItemsAdded(RecyclerView& recyclerView, int positionStart,int itemCount);
    virtual void onItemsRemoved(RecyclerView& recyclerView, int positionStart,int itemCount);
    virtual void onItemsUpdated(RecyclerView& recyclerView, int positionStart,int itemCount);
    virtual void onItemsUpdated(RecyclerView& recyclerView, int positionStart,int itemCount,Object* payload);
    virtual void onItemsMoved(RecyclerView& recyclerView, int from, int to,int itemCount);
    virtual int computeHorizontalScrollExtent(State& state);
    virtual int computeHorizontalScrollOffset(State& state);
    virtual int computeHorizontalScrollRange(State& state);
    virtual int computeVerticalScrollExtent(State& state);
    virtual int computeVerticalScrollOffset(State& state);
    virtual int computeVerticalScrollRange(State& state);
    virtual void onMeasure(Recycler& recycler, State& state, int widthSpec,int heightSpec);
    virtual bool prepareForDrop(View* view,View* target, int x, int y);
    void setMeasuredDimension(int widthSize, int heightSize);
    int getMinimumWidth();
    int getMinimumHeight();
    virtual Parcelable* onSaveInstanceState();
    virtual void onRestoreInstanceState(Parcelable& state);
    virtual void onScrollStateChanged(int state);
    void removeAndRecycleAllViews(Recycler& recycler);
    virtual void onInitializeAccessibilityNodeInfo(Recycler& recycler,State& state, AccessibilityNodeInfo& info);
    virtual void onInitializeAccessibilityEvent(AccessibilityEvent& event);
    virtual void onInitializeAccessibilityEvent(Recycler& recycler, State& state,AccessibilityEvent& event);
    virtual void onInitializeAccessibilityNodeInfoForItem(Recycler& recycler,State& state, View* host, AccessibilityNodeInfo& info);
    void requestSimpleAnimationsInNextLayout();
    int getSelectionModeForAccessibility(Recycler& recycler,State& state);
    virtual int getRowCountForAccessibility(Recycler& recycler, State& state);
    virtual int getColumnCountForAccessibility(Recycler& recycler,State& state);
    bool isLayoutHierarchical(Recycler& recycler,State& state);
    virtual bool performAccessibilityAction(Recycler& recycler, State& state,int action, Bundle* args);
    virtual bool performAccessibilityActionForItem(Recycler& recycler,State& state, View& view, int action, Bundle* args);
    static Properties getProperties(Context* context,const AttributeSet& attrs,int defStyleAttr, int defStyleRes);
};

class RecyclerView::EdgeEffectFactory {
public:
    friend RecyclerView;
    static constexpr int DIRECTION_LEFT = 0;
    static constexpr int DIRECTION_TOP = 1;
    static constexpr int DIRECTION_RIGHT = 2;
    static constexpr int DIRECTION_BOTTOM = 3;
protected:
    virtual EdgeEffect* createEdgeEffect(RecyclerView& view,int direction);
};

class RecyclerView::RecycledViewPool{
private:
    friend RecyclerView::Recycler;
protected:
    class ScrapData {
    public:
        std::vector<ViewHolder*> mScrapHeap;
        int mMaxScrap = DEFAULT_MAX_SCRAP;
        long mCreateRunningAverageNs = 0;
        long mBindRunningAverageNs = 0;
    };
    SparseArray<ScrapData*> mScrap;
    int mAttachCountForClearing = 0;
    std::set<Adapter*> mAttachedAdaptersForPoolingContainer;

    int size();
    long runningAverage(long oldAverage, long newValue);
    void factorInCreateTime(int viewType, long createTimeNs);
    void factorInBindTime(int viewType, long bindTimeNs);
    bool willCreateInTime(int viewType, long approxCurrentNs, long deadlineNs);
    bool willBindInTime(int viewType, long approxCurrentNs, long deadlineNs);
    void attach();
    void detach();
    void onAdapterChanged(Adapter* oldAdapter, Adapter* newAdapter,bool compatibleWithPrevious);
    ScrapData* getScrapDataForType(int viewType);
    void attachForPoolingContainer(Adapter*adapter);
    void detachForPoolingContainer(Adapter*adapter, bool isBeingReplaced);
public:
    static constexpr int DEFAULT_MAX_SCRAP = 5;
    RecycledViewPool();
    virtual ~RecycledViewPool();
    void clear();
    void setMaxRecycledViews(int viewType, int max);
    int getRecycledViewCount(int viewType);
    ViewHolder* getRecycledView(int viewType);
    void putRecycledView(ViewHolder* scrap);
};

class RecyclerView::Recycler{
private:
    friend GapWorker;
    friend RecyclerView;
    RecyclerView*mRV;
    static constexpr int DEFAULT_CACHE_SIZE = 2;
    std::vector<ViewHolder*> mUnmodifiableAttachedScrap;
    int mRequestedCacheMax = DEFAULT_CACHE_SIZE;
    RecyclerView::ViewCacheExtension mViewCacheExtension;

    bool tryBindViewHolderByDeadline(ViewHolder& holder, int offsetPosition,
            int position, long deadlineNs);
    void attachAccessibilityDelegateOnBind(ViewHolder& holder);
    void invalidateDisplayListInt(ViewHolder& holder);
    void invalidateDisplayListInt(ViewGroup& viewGroup, bool invalidateThis);
protected:
    int mViewCacheMax = DEFAULT_CACHE_SIZE;
    std::vector<ViewHolder*> mAttachedScrap;
    std::vector<ViewHolder*>* mChangedScrap;
    std::vector<ViewHolder*> mCachedViews;
    RecycledViewPool* mRecyclerPool;
    void updateViewCacheSize();
    bool validateViewHolderForOffsetPosition(ViewHolder* holder);
    View* getViewForPosition(int position, bool dryRun);
    ViewHolder* tryGetViewHolderForPositionByDeadline(int position, bool dryRun, long deadlineNs);
    void recycleViewInternal(View* view);
    void recycleAndClearCachedViews();
    void recycleCachedViewAt(int cachedViewIndex);
    void recycleViewHolderInternal(ViewHolder& holder);
    void addViewHolderToRecycledViewPool(ViewHolder& holder, bool dispatchRecycled);
    void quickRecycleScrapView(View* view);
    void scrapView(View* view);
    void unscrapView(ViewHolder& holder);
    int getScrapCount();
    View* getScrapViewAt(int index);
    void clearScrap();
    ViewHolder* getChangedScrapViewForPosition(int position);
    ViewHolder* getScrapOrHiddenOrCachedHolderForPosition(int position, bool dryRun);
    ViewHolder* getScrapOrCachedViewForId(long id, int type, bool dryRun);
    void dispatchViewRecycled(ViewHolder& holder);
    void onAdapterChanged(Adapter* oldAdapter, Adapter* newAdapter, bool compatibleWithPrevious);
    void offsetPositionRecordsForMove(int from, int to);
    void offsetPositionRecordsForInsert(int insertedAt, int count);
    void offsetPositionRecordsForRemove(int removedFrom, int count, bool applyToPreLayout);
    void setViewCacheExtension(const ViewCacheExtension& extension);
    void setRecycledViewPool(RecycledViewPool* pool);
    RecycledViewPool& getRecycledViewPool();
    void viewRangeUpdate(int positionStart, int itemCount);
    void markKnownViewsInvalid();
    void clearOldPositions();
    void markItemDecorInsetsDirty();

    void maybeSendPoolingContainerAttach();
    void poolingContainerDetach(Adapter*adapter);
    void poolingContainerDetach(Adapter*adapter, bool isBeingReplaced);
    void onAttachedToWindow();
    void onDetachedFromWindow();
public:
    Recycler(RecyclerView*rv);
    virtual ~Recycler();
    void clear();
    void setViewCacheSize(int viewCount);
    std::vector<ViewHolder*> getScrapList();
    void bindViewToPosition(View* view, int position);
    int convertPreLayoutPositionToPostLayout(int position);
    View* getViewForPosition(int position);
    void recycleView(View* view);
};

class RecyclerView::ViewHolder{
private:
    friend ChildHelper;
    friend GapWorker;
    friend RecyclerView;
    friend RecyclerView::ItemAnimator;
    int mFlags;
    int mIsRecyclableCount = 0;
    int mWasImportantForAccessibilityBeforeHidden=View::IMPORTANT_FOR_ACCESSIBILITY_AUTO;
    bool mInChangeScrap = false;
    Recycler* mScrapContainer = nullptr;
    static std::vector<Object*> FULLUPDATE_PAYLOADS;

    void createPayloadsIfNeeded();
    void onEnteredHiddenState(RecyclerView& parent);
    void onLeftHiddenState(RecyclerView& parent);
    bool shouldBeKeptAsChild();
    bool doesTransientStatePreventRecycling();
protected:
    static constexpr int FLAG_BOUND = 1 << 0;
    static constexpr int FLAG_UPDATE = 1 << 1;
    static constexpr int FLAG_INVALID = 1 << 2;
    static constexpr int FLAG_REMOVED = 1 << 3;
    static constexpr int FLAG_NOT_RECYCLABLE = 1 << 4;
    static constexpr int FLAG_RETURNED_FROM_SCRAP = 1 << 5;
    static constexpr int FLAG_IGNORE = 1 << 7;
    static constexpr int FLAG_TMP_DETACHED = 1 << 8;
    static constexpr int FLAG_ADAPTER_POSITION_UNKNOWN = 1 << 9;
    static constexpr int FLAG_ADAPTER_FULLUPDATE = 1 << 10;
    static constexpr int FLAG_MOVED = 1 << 11;
    static constexpr int FLAG_APPEARED_IN_PRE_LAYOUT = 1 << 12;
    static constexpr int PENDING_ACCESSIBILITY_STATE_NOT_SET = -1;
    static constexpr int FLAG_BOUNCED_FROM_HIDDEN_LIST = 1 << 13;
    static constexpr int FLAG_SET_A11Y_ITEM_DELEGATE = 1 << 14;
protected:
    int mPosition = NO_POSITION;
    int mOldPosition = NO_POSITION;
    long mItemId = NO_ID;
    int mItemViewType = INVALID_TYPE;
    int mPreLayoutPosition = NO_POSITION;
    int mPendingAccessibilityState = PENDING_ACCESSIBILITY_STATE_NOT_SET;
    ViewHolder* mShadowedHolder;
    ViewHolder* mShadowingHolder;
    RecyclerView* mNestedRecyclerView;
    RecyclerView* mOwnerRecyclerView;
    std::vector<Object*> mPayloads;
    std::vector<Object*>* mUnmodifiedPayloads;
    RecyclerView::Adapter* mBindingAdapter;
public:
    View*itemView;
public:
    ViewHolder(View* itemView);
    virtual ~ViewHolder();
    void flagRemovedAndOffsetPosition(int newPosition, int offset, bool applyToPreLayout);
    void offsetPosition(int offset, bool applyToPreLayout);
    void clearOldPosition();
    void saveOldPosition();
    bool isScrap()const;
    void unScrap();
    bool wasReturnedFromScrap()const;
    void clearReturnedFromScrapFlag();
    void clearTmpDetachFlag();
    void stopIgnoring();
    void setScrapContainer(Recycler* recycler, bool isChangeScrap);
    bool needsUpdate()const;
    bool isBound()const;
    bool hasAnyOfTheFlags(int flags)const;
    bool isTmpDetached()const;
    bool isAttachedToTransitionOverlay()const;
    bool isAdapterPositionUnknown()const;
    void setFlags(int flags, int mask);
    void addFlags(int flags);
    void addChangePayload(Object* payload);
    void clearPayload();
    std::vector<Object*>* getUnmodifiedPayloads();
    void resetInternal();
    bool isUpdated()const;
    /**
     * @see #getLayoutPosition()
     * @see #getBindingAdapterPosition()
     * @see #getAbsoluteAdapterPosition()
     * @deprecated This method is deprecated because its meaning is ambiguous due to the async
     * handling of adapter updates. You should use {@link #getLayoutPosition()},
     * {@link #getBindingAdapterPosition()} or {@link #getAbsoluteAdapterPosition()}
     * depending on your use case.
     */
    //[[deprecated("use getLayoutPosition,getBindingAdapterPosition,getAbsoluteAdapterPosition")]]
    //int getPosition() const;
    int getLayoutPosition() const;
    /**
     * @return {@link #getBindingAdapterPosition()}
     * @deprecated This method is confusing when adapters nest other adapters.
     * If you are calling this in the context of an Adapter, you probably want to call
     * {@link #getBindingAdapterPosition()} or if you want the position as {@link RecyclerView}
     * sees it, you should call {@link #getAbsoluteAdapterPosition()}.
     */
    //[[deprecated("use getBindingAdapterPosition or getAbsoluteAdapterPosition")]]
    //int getAdapterPosition();
    int getBindingAdapterPosition();
    int getAbsoluteAdapterPosition();
    RecyclerView::Adapter*getBindingAdapter()const;
    int getOldPosition()const;
    long getItemId()const;
    int getItemViewType()const;
    std::string toString();
    void setIsRecyclable(bool recyclable);
    bool shouldIgnore()const;
    bool isInvalid()const;
    bool isRecyclable()const;
    bool isRemoved()const;
};

class RecyclerView::SmoothScroller{
public:
    class Action;
    friend RecyclerView;
private:
    int mTargetPosition = RecyclerView::NO_POSITION;
    RecyclerView* mRecyclerView;
    RecyclerView::LayoutManager* mLayoutManager;
    bool mPendingInitialRun;
    bool mRunning;
    View* mTargetView;
    Action* mRecyclingAction;
    bool mStarted;
private:
    void onAnimation(int dx, int dy);
protected:
    void start(RecyclerView* recyclerView, RecyclerView::LayoutManager* layoutManager);
    void stop();
    void onChildAttachedToWindow(View* child);
    void normalize(PointF& scrollVector);
    virtual void onStart()=0;
    virtual void onStop()=0;
    virtual void onSeekTargetStep(int dx,int dy,State& state,Action& action)=0;
    virtual void onTargetFound(View* targetView,State& state,Action& action)=0;
public:
    SmoothScroller();
    virtual ~SmoothScroller();
    void setTargetPosition(int targetPosition);
    bool computeScrollVectorForPosition(int targetPosition,PointF&scrollVector);
    RecyclerView::LayoutManager* getLayoutManager();
    bool isPendingInitialRun() const;
    bool isRunning() const;
    int getTargetPosition() const;
    int getChildPosition(View* view);
    int getChildCount() const;
    View* findViewByPosition(int position);
    void instantScrollToPosition(int position);
};

class RecyclerView::SmoothScroller::Action{
public:
    static constexpr int UNDEFINED_DURATION = RecyclerView::UNDEFINED_DURATION;
    friend RecyclerView::SmoothScroller;
private:
    int mDx;
    int mDy;
    int mDuration;
    int mJumpToPosition = NO_POSITION;//RecyclerView::NO_POSITION;
    const Interpolator* mInterpolator;
    bool mChanged = false;
    int mConsecutiveUpdates = 0;
    void validate();
protected:
    bool hasJumpTarget();
    void runIfNecessary(RecyclerView& recyclerView);
public:
    Action(int dx,int dy);
    Action(int dx, int dy, int duration);
    Action(int dx, int dy, int duration,Interpolator* interpolator);
    void jumpTo(int targetPosition);
    int getDx();
    void setDx(int dx);
    int getDy();
    void setDy(int dy);
    int getDuration();
    void setDuration(int duration);
    const Interpolator* getInterpolator()const;
    void setInterpolator(const Interpolator* interpolator);
    void update(int dx,int dy,int duration,const Interpolator* interpolator);
};

class RecyclerView::AdapterDataObservable:public Observable<AdapterDataObserver> {
public:
    bool hasObservers()const;
    void notifyChanged();
    void notifyStateRestorationPolicyChanged();
    void notifyItemRangeChanged(int positionStart, int itemCount);
    void notifyItemRangeChanged(int positionStart, int itemCount,Object* payload);
    void notifyItemRangeInserted(int positionStart, int itemCount);
    void notifyItemRangeRemoved(int positionStart, int itemCount);
    void notifyItemMoved(int fromPosition, int toPosition);
};

class RecyclerView::RecyclerViewDataObserver:public RecyclerView::AdapterDataObserver{
private:
    RecyclerView*mRV;
protected:
    void triggerUpdateProcessor();
public:
    RecyclerViewDataObserver(RecyclerView*rv);
    void onChanged()override;
    void onItemRangeChanged(int positionStart, int itemCount, Object* payload)override;
    void onItemRangeInserted(int positionStart, int itemCount)override;
    void onItemRangeRemoved(int positionStart, int itemCount)override;
    void onItemRangeMoved(int fromPosition, int toPosition, int itemCount)override;
    void onStateRestorationPolicyChanged()override;
};

class RecyclerView::SavedState:public AbsSavedState{
protected:
    Parcelable*mLayoutState;
    friend RecyclerView;
public:
    SavedState(Parcel&in);
    SavedState(Parcelable*superState);
    void writeToParcel(Parcel&dest,int flags)override;
    void copyFrom(SavedState&other);
};

class RecyclerView::State{
protected:
    friend GapWorker;
    friend RecyclerView;
    static constexpr int STEP_START = 1;
    static constexpr int STEP_LAYOUT = 1 << 1;
    static constexpr int STEP_ANIMATIONS = 1 << 2;
    int mPreviousLayoutItemCount = 0;
    int mDeletedInvisibleItemCountSincePreviousLayout = 0;
    int mLayoutStep = STEP_START;
    int mItemCount = 0;
    bool mStructureChanged = false;
    bool mInPreLayout = false;
    bool mTrackOldChangeHolders = false;
    bool mIsMeasuring = false;
    bool mRunSimpleAnimations = false;
    bool mRunPredictiveAnimations = false;
    long mFocusedItemId;
    int mFocusedItemPosition;
    int mFocusedSubChildId;
    int mRemainingScrollHorizontal;
    int mRemainingScrollVertical;    
    int mTargetPosition = RecyclerView::NO_POSITION;
private:
    SparseArray<Object*> mData;
protected:
    void assertLayoutStep(int accepted);
    //State& reset();
    void prepareForNestedPrefetch(Adapter* adapter);
public:
    bool isMeasuring();
    bool isPreLayout();
    bool willRunPredictiveAnimations();
    bool willRunSimpleAnimations();
    void remove(int resourceId);
    Object* get(int resourceId);
    void put(int resourceId, Object* data);
    int getTargetScrollPosition();
    bool hasTargetScrollPosition();
    bool didStructureChange();
    int getItemCount();
    int getRemainingScrollHorizontal();
    int getRemainingScrollVertical();
};

}/*endof namespace*/
#endif/*__RECYCLER_VIEW_H__*/
