#ifndef __LINEAR_LAYOUT_MANAGER_H__
#define __LINEAR_LAYOUT_MANAGER_H__
#include <widgetEx/recyclerview/recyclerview.h>
#include <core/parcel.h>
#include <core/parcelable.h>
namespace cdroid{
class GridLayoutManager;
class LinearLayoutManager:public RecyclerView::LayoutManager{
private:
    static constexpr bool _DEBUG=false;
    static constexpr float MAX_SCROLL_FACTOR = 1.f / 3.f;
public:
    static constexpr int HORIZONTAL = RecyclerView::HORIZONTAL;
    static constexpr int VERTICAL = RecyclerView::VERTICAL;
    static constexpr int INVALID_OFFSET = INT_MIN;
protected:
    class LayoutChunkResult;
public:
    class LayoutState;
    class SavedState;
    class AnchorInfo;
private:
    LayoutState* mLayoutState;
    bool mLastStackFromEnd;
    bool mReverseLayout = false;
    bool mStackFromEnd = false;
    bool mSmoothScrollbarEnabled = true;
    bool mRecycleChildrenOnDetach;
    LayoutChunkResult* mLayoutChunkResult;// = new LayoutChunkResult();
    int mInitialPrefetchItemCount = 2;
    int mReusableIntPair[2];
private:
    void resolveShouldLayoutReverse();
    void layoutForPredictiveAnimations(RecyclerView::Recycler& recycler,
            RecyclerView::State& state, int startOffset,int endOffset);
    void updateAnchorInfoForLayout(RecyclerView::Recycler& recycler, RecyclerView::State& state,
            AnchorInfo& anchorInfo);
    bool updateAnchorFromChildren(RecyclerView::Recycler& recycler,
            RecyclerView::State& state, AnchorInfo& anchorInfo);
    bool updateAnchorFromPendingData(RecyclerView::State& state, AnchorInfo& anchorInfo);
    int fixLayoutEndGap(int endOffset, RecyclerView::Recycler& recycler,
            RecyclerView::State& state, bool canOffsetChildren);
    int fixLayoutStartGap(int startOffset, RecyclerView::Recycler& recycler,
            RecyclerView::State& state, bool canOffsetChildren);
    void updateLayoutStateToFillEnd(AnchorInfo& anchorInfo);
    void updateLayoutStateToFillEnd(int itemPosition, int offset);
    void updateLayoutStateToFillStart(AnchorInfo& anchorInfo);
    void updateLayoutStateToFillStart(int itemPosition, int offset);

    int computeScrollOffset(RecyclerView::State& state);
    int computeScrollExtent(RecyclerView::State& state);
    int computeScrollRange(RecyclerView::State& state);

    void updateLayoutState(int layoutDirection, int requiredSpace,
            bool canUseExistingSpace, RecyclerView::State& state);

    void recycleChildren(RecyclerView::Recycler& recycler, int startIndex, int endIndex);
    void recycleViewsFromStart(RecyclerView::Recycler& recycler, int scrollingOffset,int noRecycleSpace);
    void recycleViewsFromEnd(RecyclerView::Recycler& recycler, int scrollingOffset,int noRecycleSpace);
    void recycleByLayoutState(RecyclerView::Recycler& recycler, LayoutState& layoutState);

    View* getChildClosestToStart();
    View* getChildClosestToEnd();

    View* findReferenceChildClosestToEnd(RecyclerView::Recycler& recycler, RecyclerView::State& state);
    View* findReferenceChildClosestToStart(RecyclerView::Recycler& recycler,RecyclerView::State& state);
    View* findFirstReferenceChild(RecyclerView::Recycler& recycler, RecyclerView::State& state);
    View* findLastReferenceChild(RecyclerView::Recycler& recycler, RecyclerView::State& state);

    View* findPartiallyOrCompletelyInvisibleChildClosestToEnd();
    View* findPartiallyOrCompletelyInvisibleChildClosestToStart();
    View* findFirstPartiallyOrCompletelyInvisibleChild();
    View* findLastPartiallyOrCompletelyInvisibleChild();

    void logChildren();
protected:
    int mOrientation = RecyclerView::DEFAULT_ORIENTATION;
    class OrientationHelper* mOrientationHelper;
    bool mShouldReverseLayout = false;
    int mPendingScrollPosition = RecyclerView::NO_POSITION;
    int mPendingScrollPositionOffset = INVALID_OFFSET;
    SavedState* mPendingSavedState;
    AnchorInfo* mAnchorInfo;// = new AnchorInfo();
    bool isLayoutRTL();
    void ensureLayoutState();
    LayoutState* createLayoutState();
    void validateChildOrder();
protected:
    int getExtraLayoutSpace(RecyclerView::State& state);
    virtual void calculateExtraLayoutSpace(RecyclerView::State& state,int extraLayoutSpace[2]);
    virtual void onAnchorReady(RecyclerView::Recycler& recycler, RecyclerView::State& state,
            AnchorInfo& anchorInfo, int firstLayoutItemDirection);
    bool resolveIsInfinite();
    virtual void collectPrefetchPositionsForLayoutState(RecyclerView::State& state, LayoutState& layoutState,
            LayoutPrefetchRegistry& layoutPrefetchRegistry);
    int scrollBy(int delta, RecyclerView::Recycler& recycler, RecyclerView::State& state);
    int fill(RecyclerView::Recycler& recycler, LayoutState& layoutState,
            RecyclerView::State& state, bool stopOnFocusable);
    virtual void layoutChunk(RecyclerView::Recycler& recycler, RecyclerView::State& state,
            LayoutState& layoutState, LayoutChunkResult& result);
    bool shouldMeasureTwice();
    int convertFocusDirectionToLayoutDirection(int focusDirection);
    View* findFirstVisibleChildClosestToStart(bool completelyVisible,bool acceptPartiallyVisible);
    View* findFirstVisibleChildClosestToEnd(bool completelyVisible,bool acceptPartiallyVisible);
    virtual View* findReferenceChild(RecyclerView::Recycler& recycler, RecyclerView::State& state,
            int start, int end, int itemCount);
    View* findOneVisibleChild(int fromIndex, int toIndex, bool completelyVisible,bool acceptPartiallyVisible);
    View* findOnePartiallyOrCompletelyInvisibleChild(int fromIndex, int toIndex);
public:
    LinearLayoutManager(Context* context);
    LinearLayoutManager(Context* context,int orientation,bool reverseLayout);
    LinearLayoutManager(Context* context, const AttributeSet& attrs);
	~LinearLayoutManager()override;
    bool isAutoMeasureEnabled()const override;
    RecyclerView::LayoutParams* generateDefaultLayoutParams()const override;
    virtual bool getRecycleChildrenOnDetach()const;
    void setRecycleChildrenOnDetach(bool recycleChildrenOnDetach);
    void onDetachedFromWindow(RecyclerView* view, RecyclerView::Recycler& recycler)override;
    //void onInitializeAccessibilityEvent(AccessibilityEvent& event)override;
    Parcelable* onSaveInstanceState()override;
    void onRestoreInstanceState(Parcelable& state)override;
    bool canScrollHorizontally()const override;
    bool canScrollVertically()const override;
    virtual void setStackFromEnd(bool stackFromEnd);
    bool getStackFromEnd()const;
    int getOrientation()const;
    void setOrientation(int orientation);
    bool getReverseLayout()const;
    void setReverseLayout(bool reverseLayout);
    View* findViewByPosition(int position)override;
    void smoothScrollToPosition(RecyclerView& recyclerView, RecyclerView::State& state,int position)override;
    bool computeScrollVectorForPosition(int targetPosition,PointF&point)override;
    void onLayoutChildren(RecyclerView::Recycler& recycler, RecyclerView::State& state)override;
    void onLayoutCompleted(RecyclerView::State& state)override;
    void scrollToPosition(int position)override;
    void scrollToPositionWithOffset(int position, int offset);
    int scrollHorizontallyBy(int dx, RecyclerView::Recycler& recycler, RecyclerView::State& state);
    int scrollVerticallyBy(int dy, RecyclerView::Recycler& recycler, RecyclerView::State& state);
    int computeHorizontalScrollOffset(RecyclerView::State& state)override;
    int computeVerticalScrollOffset(RecyclerView::State& state)override;
    int computeHorizontalScrollExtent(RecyclerView::State& state)override;
    int computeVerticalScrollExtent(RecyclerView::State& state)override;
    int computeHorizontalScrollRange(RecyclerView::State& state)override;
    int computeVerticalScrollRange(RecyclerView::State& state)override;
    void setSmoothScrollbarEnabled(bool enabled);
    bool isSmoothScrollbarEnabled();
    void collectInitialPrefetchPositions(int adapterItemCount,LayoutPrefetchRegistry& layoutPrefetchRegistry);
    void setInitialPrefetchItemCount(int itemCount);
    int getInitialPrefetchItemCount();
    void collectAdjacentPrefetchPositions(int dx, int dy, RecyclerView::State& state,
            LayoutPrefetchRegistry& layoutPrefetchRegistry);
    void assertNotInLayoutOrScroll(const std::string& message);

    int findFirstVisibleItemPosition();
    int findFirstCompletelyVisibleItemPosition();
    int findLastVisibleItemPosition();
    int findLastCompletelyVisibleItemPosition();
    View* onFocusSearchFailed(View* focused, int focusDirection,
            RecyclerView::Recycler& recycler, RecyclerView::State& state);
    bool supportsPredictiveItemAnimations()override;
    void prepareForDrop(View* view,View* target, int x, int y);
};

class LinearLayoutManager::LayoutState {
protected:
    static constexpr int LAYOUT_START = -1;
    static constexpr int LAYOUT_END = 1;
    static constexpr int INVALID_LAYOUT = INT_MIN;//Integer.MIN_VALUE;
    static constexpr int ITEM_DIRECTION_HEAD = -1;
    static constexpr int ITEM_DIRECTION_TAIL = 1;
    static constexpr int SCROLLING_OFFSET_NaN = INT_MIN;//Integer.MIN_VALUE;
protected:
    int mOffset;
    int mAvailable;
    int mCurrentPosition;
    int mItemDirection;
    int mLayoutDirection;
    int mScrollingOffset;
    int mExtraFillSpace = 0;
    int mNoRecycleSpace = 0;
    int mLastScrollDelta;
    bool mRecycle = true;
    bool mIsPreLayout = false;
    bool mInfinite;
    std::vector<RecyclerView::ViewHolder*> mScrapList;
private:
    friend LinearLayoutManager;
    friend GridLayoutManager;
    View* nextViewFromScrapList();
public:
    bool hasMore(RecyclerView::State& state);
    View* next(RecyclerView::Recycler& recycler);
    void log();
public:
    void assignPositionFromScrapList();
    void assignPositionFromScrapList(View* ignore);
    View* nextViewInLimitedList(View* ignore);
};

class LinearLayoutManager::SavedState:public Parcelable {
protected:
    int mAnchorPosition;
    int mAnchorOffset;
    bool mAnchorLayoutFromEnd;
protected:
    friend LinearLayoutManager;
    void invalidateAnchor();
public:
    SavedState();
    SavedState(Parcel& in);
    SavedState(SavedState& other);
    bool hasValidAnchor()const;
    int describeContents();
    void writeToParcel(Parcel& dest, int flags);
};
class LinearLayoutManager::AnchorInfo {
protected:
    friend LinearLayoutManager;
    friend GridLayoutManager;
    class OrientationHelper* mOrientationHelper;
    int mPosition;
    int mCoordinate;
    bool mLayoutFromEnd;
    bool mValid;
protected:
    AnchorInfo();
    virtual ~AnchorInfo();
    void reset();
    void assignCoordinateFromPadding();
    bool isViewValidAsAnchor(View* child, RecyclerView::State& state);
public:
    void assignFromViewAndKeepVisibleRect(View* child, int position);
    void assignFromView(View* child, int position);
};

class LinearLayoutManager::LayoutChunkResult {
public:
    int mConsumed;
    bool mFinished;
    bool mIgnoreConsumed;
    bool mFocusable;
public:
    void resetInternal();
};
}/*endof namespace*/
#endif/*__LINEAR_LAYOUT_MANAGER_H__*/
