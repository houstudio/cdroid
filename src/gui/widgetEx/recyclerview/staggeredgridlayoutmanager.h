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
#ifndef __STAGGERED_GRIDLAYOUT_MANAGER_H__
#define __STAGGERED_GRIDLAYOUT_MANAGER_H__
#include <widgetEx/recyclerview/recyclerview.h>
#include <core/bitset.h>
namespace cdroid{
class OrientationHelper;

class StaggeredGridLayoutManager:public RecyclerView::LayoutManager{
private:
    static constexpr float MAX_SCROLL_FACTOR = 1.f / 3.f;
public:
    static constexpr bool _Debug = false;
    static constexpr int HORIZONTAL = RecyclerView::HORIZONTAL;
    static constexpr int VERTICAL = RecyclerView::VERTICAL;
    static constexpr int GAP_HANDLING_NONE = 0;
    static constexpr int GAP_HANDLING_LAZY = 1;
    static constexpr int GAP_HANDLING_MOVE_ITEMS_BETWEEN_SPANS = 2;
    static constexpr int INVALID_OFFSET = INT_MIN;
    class SavedState;
    class AnchorInfo;
    class FullSpanItem;
    class LazySpanLookup;
    class Span;
    class BitSet;
    class LayoutParams:public RecyclerView::LayoutParams {
    public:
        static constexpr int INVALID_SPAN_ID = -1;
        // Package scope to be able to access from tests.
        Span* mSpan;
        bool mFullSpan=false;
    public:
        LayoutParams(Context* c, const AttributeSet& attrs);
        LayoutParams(int width, int height);
        LayoutParams(const ViewGroup::MarginLayoutParams& source);
        LayoutParams(const ViewGroup::LayoutParams& source);
        LayoutParams(const RecyclerView::LayoutParams& source);
        void setFullSpan(bool fullSpan);
        bool isFullSpan();
        int getSpanIndex();
    };
private:
    int mSpanCount = -1;
    int mOrientation;
    int mSizePerSpan;
    int mFullSizeSpec;
    int mGapStrategy = GAP_HANDLING_MOVE_ITEMS_BETWEEN_SPANS;
    class LayoutState* mLayoutState;
    BitSet* mRemainingSpans;
    bool mLastLayoutFromEnd;
    bool mLastLayoutRTL;
    bool mLaidOutInvalidFullSpan = false;
    bool mSmoothScrollbarEnabled = true;
    SavedState* mPendingSavedState;
    Rect mTmpRect;
    AnchorInfo* mAnchorInfo;;
    std::vector<int> mPrefetchDistances;
    Runnable mCheckForGapsRunnable;
protected:
    std::vector<Span*>mSpans;
    OrientationHelper* mPrimaryOrientation;
    OrientationHelper* mSecondaryOrientation;
    bool mReverseLayout = false;
    bool mShouldReverseLayout = false;
    int mPendingScrollPosition = RecyclerView::NO_POSITION;
    int mPendingScrollPositionOffset = INVALID_OFFSET;
    LazySpanLookup* mLazySpanLookup;
private:
    void initLayoutManager();
    void resolveShouldLayoutReverse();
    void createOrientationHelpers();
    bool checkSpanForGap(Span& span);
    void onLayoutChildren(RecyclerView::Recycler& recycler, RecyclerView::State& state, bool shouldCheckForGaps);
    void repositionToWrapContentIfNecessary();
    void applyPendingSavedState(AnchorInfo& anchorInfo);
    bool updateAnchorFromChildren(RecyclerView::State& state, AnchorInfo& anchorInfo);
    int computeScrollOffset(RecyclerView::State& state);
    int computeScrollExtent(RecyclerView::State& state);
    int computeScrollRange(RecyclerView::State& state);
    void measureChildWithDecorationsAndMargin(View* child, LayoutParams& lp, bool alreadyMeasured);
    void measureChildWithDecorationsAndMargin(View* child, int widthSpec,int heightSpec, bool alreadyMeasured);
    int updateSpecWithExtra(int spec, int startInset, int endInset);
    void fixEndGap(RecyclerView::Recycler& recycler, RecyclerView::State& state,  bool canOffsetChildren);
    void fixStartGap(RecyclerView::Recycler& recycler, RecyclerView::State& state, bool canOffsetChildren);
    void updateLayoutState(int anchorPosition, RecyclerView::State& state);
    void setLayoutStateDirection(int direction);
    void handleUpdate(int positionStart, int itemCountOrToPosition, int cmd);
    int fill(RecyclerView::Recycler& recycler, LayoutState& layoutState, RecyclerView::State& state);
    /*LazySpanLookup::FullSpanItem*/void* createFullSpanItemFromEnd(int newItemTop);
    /*LazySpanLookup::FullSpanItem*/void* createFullSpanItemFromStart(int newItemBottom);
    void attachViewToSpans(View* view, LayoutParams& lp, LayoutState& layoutState);
    void recycle(RecyclerView::Recycler& recycler, LayoutState& layoutState);
    void appendViewToAllSpans(View* view);
    void prependViewToAllSpans(View* view);
    void updateAllRemainingSpans(int layoutDir, int targetLine);
    void updateRemainingSpans(Span& span, int layoutDir, int targetLine);
    int getMaxStart(int def);
    int getMinStart(int def);
    int getMaxEnd(int def);
    int getMinEnd(int def);
    void recycleFromStart(RecyclerView::Recycler& recycler, int line);
    void recycleFromEnd(RecyclerView::Recycler& recycler, int line);
    bool preferLastSpan(int layoutDir);
    Span* getNextSpan(LayoutState& layoutState);
    int calculateScrollDirectionForPosition(int position);
    int findFirstReferenceChildPosition(int itemCount);
    int findLastReferenceChildPosition(int itemCount);
    int convertFocusDirectionToLayoutDirection(int focusDirection);
protected:
    bool checkForGaps();
    View* hasGapsToFix();
    bool isLayoutRTL();
    void updateAnchorInfoForLayout(RecyclerView::State& state, AnchorInfo& anchorInfo);
    bool updateAnchorFromPendingData(RecyclerView::State& state, AnchorInfo& anchorInfo);
    void updateMeasureSpecs(int totalSpace);
    int findFirstVisibleItemPositionInt();
    View* findFirstVisibleItemClosestToStart(bool fullyVisible);
    View* findFirstVisibleItemClosestToEnd(bool fullyVisible);
    bool areAllEndsEqual();
    bool areAllStartsEqual();
    void prepareLayoutStateForDelta(int delta, RecyclerView::State state);
    int scrollBy(int dt, RecyclerView::Recycler& recycler, RecyclerView::State& state);
    int getLastChildPosition();
    int getFirstChildPosition();

public:
    StaggeredGridLayoutManager(Context* context,const AttributeSet& attrs);//, int defStyleAttr,int defStyleRes);
    StaggeredGridLayoutManager(int spanCount, int orientation);
	~StaggeredGridLayoutManager()override;
    bool isAutoMeasureEnabled()const override;
    void onScrollStateChanged(int state)override;
    void onDetachedFromWindow(RecyclerView& view, RecyclerView::Recycler& recycler)override;
    void setSpanCount(int spanCount);
    void setOrientation(int orientation);
    void setReverseLayout(bool reverseLayout);
    int getGapStrategy();
    void setGapStrategy(int gapStrategy);
    void assertNotInLayoutOrScroll(const std::string& message)override;
    int getSpanCount();
    void invalidateSpanAssignments();
    bool getReverseLayout();
    void setMeasuredDimension(Rect& childrenBounds, int wSpec, int hSpec)override;
    void onLayoutChildren(RecyclerView::Recycler& recycler, RecyclerView::State& state)override;
    void onLayoutCompleted(RecyclerView::State& state)override;
    void onAdapterChanged(RecyclerView::Adapter* oldAdapter, RecyclerView::Adapter* newAdapter)override;
    bool supportsPredictiveItemAnimations()override;
    int findFirstVisibleItemPositions(std::vector<int>& into);
    int findFirstCompletelyVisibleItemPositions(std::vector<int>& into);
    int findLastVisibleItemPositions(std::vector<int>&into);
    int findLastCompletelyVisibleItemPositions(std::vector<int>& into);
    int computeHorizontalScrollOffset(RecyclerView::State& state)override;
    int computeVerticalScrollOffset(RecyclerView::State& state)override;
    int computeHorizontalScrollExtent(RecyclerView::State& state)override;


    int computeVerticalScrollExtent(RecyclerView::State& state)override;
    int computeHorizontalScrollRange(RecyclerView::State& state)override;
    int computeVerticalScrollRange(RecyclerView::State& state)override;
    void onRestoreInstanceState(Parcelable& state)override;
    Parcelable* onSaveInstanceState()override;
    //void onInitializeAccessibilityNodeInfoForItem(RecyclerView::Recycler& recycler,
    //      RecyclerView::State& state, View host, AccessibilityNodeInfoCompat info)override;
    //void onInitializeAccessibilityEvent(AccessibilityEvent& event)override;
    int getRowCountForAccessibility(RecyclerView::Recycler& recycler, RecyclerView::State& state)override;
    int getColumnCountForAccessibility(RecyclerView::Recycler& recycler,RecyclerView::State& state)override;

    void offsetChildrenHorizontal(int dx)override;
    void offsetChildrenVertical(int dy)override;

    void onItemsRemoved(RecyclerView& recyclerView, int positionStart, int itemCount)override;
    void onItemsAdded(RecyclerView& recyclerView, int positionStart, int itemCount)override;
    void onItemsChanged(RecyclerView& recyclerView)override;
    void onItemsMoved(RecyclerView& recyclerView, int from, int to, int itemCount)override;
    void onItemsUpdated(RecyclerView& recyclerView, int positionStart, int itemCount,Object* payload)override;
    bool canScrollVertically()const override;
    bool canScrollHorizontally()const override;
    int scrollHorizontallyBy(int dx, RecyclerView::Recycler& recycler, RecyclerView::State& state)override;
    int scrollVerticallyBy(int dy, RecyclerView::Recycler& recycler, RecyclerView::State& state)override;
    bool computeScrollVectorForPosition(int targetPosition,PointF&)override;
    void smoothScrollToPosition(RecyclerView& recyclerView, RecyclerView::State& state,int position)override;
    void scrollToPosition(int position)override;
    void scrollToPositionWithOffset(int position, int offset);
    void collectAdjacentPrefetchPositions(int dx, int dy, RecyclerView::State& state,
          LayoutPrefetchRegistry& layoutPrefetchRegistry)override;
    LayoutParams* generateDefaultLayoutParams()const override;
    LayoutParams* generateLayoutParams(Context* c,const AttributeSet& attrs)const override;
    LayoutParams* generateLayoutParams(const ViewGroup::LayoutParams& lp)const override;
    bool checkLayoutParams(const RecyclerView::LayoutParams* lp)const override;
    int getOrientation()const;
    View* onFocusSearchFailed(View* focused, int direction, RecyclerView::Recycler& recycler, RecyclerView::State& state)override;
};/*StaggeredGridLayoutManager*/

// Package scoped to access from tests.
class StaggeredGridLayoutManager::Span {
    static constexpr int INVALID_LINE =INT_MIN;// Integer.MIN_VALUE;
    std::vector<View*> mViews;// = new ArrayList<>();
    int mCachedStart = INVALID_LINE;
    int mCachedEnd = INVALID_LINE;
    int mDeletedSize = 0;
    int mIndex;
    StaggeredGridLayoutManager*mLM;
    friend StaggeredGridLayoutManager;
    friend LayoutParams;
protected:
    Span(StaggeredGridLayoutManager*lm,int index);
    int getStartLine(int def);
    void calculateCachedStart();
    // Use this one when default value does not make sense and not having a value means a bug.
    int getStartLine();
    int getEndLine(int def);
    void calculateCachedEnd();

    // Use this one when default value does not make sense and not having a value means a bug.
    int getEndLine();

    void prependToSpan(View* view);

    void appendToSpan(View* view);

    // Useful method to preserve positions on a re-layout.
    void cacheReferenceLineAndClear(bool reverseLayout, int offset);
    void clear();
    void invalidateCache();
    void setLine(int line);
    void popEnd();
    void popStart();
    LayoutParams* getLayoutParams(View* view);
    void onOffset(int dt);
    int findOnePartiallyOrCompletelyVisibleChild(int fromIndex, int toIndex,
            bool completelyVisible, bool acceptCompletelyVisible,bool acceptEndPointInclusion);

    int findOneVisibleChild(int fromIndex, int toIndex, bool completelyVisible);
    int findOnePartiallyVisibleChild(int fromIndex, int toIndex, bool acceptEndPointInclusion);
public: 
    int getDeletedSize();
    int findFirstVisibleItemPosition();
    int findFirstPartiallyVisibleItemPosition();
    int findFirstCompletelyVisibleItemPosition();
    int findLastVisibleItemPosition();

    int findLastPartiallyVisibleItemPosition();
    int findLastCompletelyVisibleItemPosition();
    View* getFocusableViewAfter(int referenceChildPosition, int layoutDir);
};

class StaggeredGridLayoutManager::LazySpanLookup {
public:
    class FullSpanItem;
private:
    const static constexpr int MIN_SIZE = 10;
protected:
    std::vector<int> mData;
    std::vector<FullSpanItem*> mFullSpanItems;
private:
    friend StaggeredGridLayoutManager;
    void offsetFullSpansForRemoval(int positionStart, int itemCount);
    void offsetFullSpansForAddition(int positionStart, int itemCount);
    int invalidateFullSpansAfter(int position);
protected:
    int forceInvalidateAfter(int position);
    int invalidateAfter(int position);
    int getSpan(int position);
    void setSpan(int position, Span* span);
    int sizeForPosition(int position);
    void ensureSize(int position);
    void clear();
    void offsetForRemoval(int positionStart, int itemCount);
    void offsetForAddition(int positionStart, int itemCount);
public:
    void addFullSpanItem(FullSpanItem* fullSpanItem);
    FullSpanItem* getFullSpanItem(int position);

    FullSpanItem* getFirstFullSpanItemInRange(int minPos, int maxPos, int gapDir, bool hasUnwantedGapAfter);

    class FullSpanItem:public Parcelable {
        int mPosition;
        int mGapDir;
	std::vector<int>mGapPerSpan;
        // A full span may be laid out in primary direction but may have gaps due to
        // invalidation of views after it. This is recorded during a reverse scroll and if
        // view is still on the screen after scroll stops, we have to recalculate layout
        bool mHasUnwantedGapAfter;
    protected:
        friend StaggeredGridLayoutManager;	
        FullSpanItem(Parcel& in);
        FullSpanItem();
        int getGapForSpan(int spanIndex);
    public:
        int describeContents();//override;
        void writeToParcel(Parcel& dest, int flags)override;
    };
};


class StaggeredGridLayoutManager::SavedState:public Parcelable {
protected:
    int mAnchorPosition;
    int mVisibleAnchorPosition; // Replacement for span info when spans are invalidated
    int mSpanOffsetsSize;
    std::vector<int>mSpanOffsets;
    int mSpanLookupSize;
    std::vector<int> mSpanLookup;
    std::vector<LazySpanLookup::FullSpanItem*> mFullSpanItems;
    bool mReverseLayout;
    bool mAnchorLayoutFromEnd;
    bool mLastLayoutRTL;
protected:
    friend StaggeredGridLayoutManager;
    void invalidateSpanInfo();
    void invalidateAnchorPositionInfo();
public:
    SavedState();
    SavedState(Parcel& in);
    SavedState(SavedState& other);
    int describeContents();//override;
    void writeToParcel(Parcel& dest, int flags)override;
};

class StaggeredGridLayoutManager::AnchorInfo {

    int mPosition;
    int mOffset;
    bool mLayoutFromEnd;
    bool mInvalidateOffsets;
    bool mValid;
    // this is where we save span reference lines in case we need to re-use them for multi-pass
    // measure steps
    StaggeredGridLayoutManager*mLM;
    std::vector<int>mSpanReferenceLines;
protected:
    friend StaggeredGridLayoutManager;
    AnchorInfo(StaggeredGridLayoutManager*lm);
    void reset();
    void saveSpanReferenceLines(std::vector<Span*>& spans);
    void assignCoordinateFromPadding();
    void assignCoordinateFromPadding(int addedDistance);
};

class StaggeredGridLayoutManager::BitSet{
private:
    static constexpr int ADDRESS_BITS_PER_WORD = 5;
    static constexpr int BITS_PER_WORD = 1<<ADDRESS_BITS_PER_WORD;
    static constexpr int BIT_INDEX_MASK = BITS_PER_WORD - 1;
    static constexpr uint32_t WORD_MASK = 0xffffffffL;
private:
    int wordsInUse;
    int wordSize;
    bool sizeIsSticky;
    uint32_t *words;
    static int wordIndex(int bitIndex);
    static void checkRange(int fromIndex, int toIndex);
    void expandTo(int wordIndex);
    void ensureCapacity(int wordsRequired);
    void checkInvariants();
    void recalculateWordsInUse();
public:
    BitSet();
    BitSet(int nbits);
    void initWords(int nbits);
    void set(int bitIndex);
    void set(int bitIndex, bool value);
    void set(int fromIndex, int toIndex, bool value);
    void clear(int bitIndex);
    void clear(int fromIndex, int toIndex);
    void clear();
    bool get(int bitIndex);
    BitSet* get(int fromIndex, int toIndex);
    int nextSetBit(int fromIndex);
    int nextClearBit(int fromIndex);
    int previousSetBit(int fromIndex);
    int previousClearBit(int fromIndex);
    int length()const;
    bool isEmpty()const;
    bool intersects(const BitSet& set);
    int cardinality()const;
    BitSet& operator&&(const BitSet& set);
    BitSet& operator||(const BitSet& set);
    BitSet& operator^(const BitSet& set);
    BitSet& andNot(const BitSet& set);
    int hashCode()const;
    int size()const;
};
}/*endof namespace*/
#endif/*__STAGGERED_GRIDLAYOUT_MANAGER_H__*/
