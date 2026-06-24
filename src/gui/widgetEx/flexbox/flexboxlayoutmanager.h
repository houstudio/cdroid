#ifndef __FLEXBOX_LAYOUT_MANAGER_H__
#define __FLEXBOX_LAYOUT_MANAGER_H__

#include <widgetEx/recyclerview/recyclerview.h>
#include <widgetEx/recyclerview/orientationhelper.h>
#include <widgetEx/flexbox/flexline.h>
#include <widgetEx/flexbox/flexitem.h>
#include <widgetEx/flexbox/flexwrap.h>
#include <widgetEx/flexbox/aligndefs.h>
#include <widgetEx/flexbox/justifycontent.h>
#include <widgetEx/flexbox/flexdirection.h>
#include <widgetEx/flexbox/flexboxhelper.h>
#include <widgetEx/flexbox/flexcontainer.h>
#include <core/parcel.h>
#include <core/parcelable.h>
#include <core/sparsearray.h>

namespace cdroid {
class OrientationHelper;
class LayoutChunkResult;

class FlexboxLayoutManager : public RecyclerView::LayoutManager, public FlexContainer {
public:
    class LayoutParams;
    class SavedState;
private:
    class LayoutState;
    class AnchorInfo;
    void init();

    int mFlexDirection = (int)FlexDirection::ROW;
    int mFlexWrap = (int)FlexWrap::NOWRAP;
    int mJustifyContent = (int)JustifyContent::FLEX_START;
    int mAlignItems = (int)AlignItems::FLEX_START;
    int mAlignContent = (int)AlignContent::FLEX_START;
    int mMaxLine = -1;

    bool mShouldReverseLayout = false;
    bool mRecycleChildrenOnDetach = false;
    int mPendingScrollPosition = RecyclerView::NO_POSITION;
    int mPendingScrollPositionOffset = INT_MIN;
    class SavedState* mPendingSavedState = nullptr;

    FlexboxHelper* mFlexboxHelper = nullptr;
    RecyclerView::Recycler *mRecycler;
    RecyclerView::State *mState;
    std::vector<FlexLine> mFlexLines;
    FlexboxHelper::FlexLinesResult* mFlexLinesResult = nullptr;

    std::vector<int> mReorderedIndices;
    SparseIntArray mOrderCache;
    bool mOrderChanged = false;

    int mAnchorPosition = RecyclerView::NO_POSITION;
    int mAnchorOffset = 0;
    bool mAnchorLayoutFromEnd = false;

    /** True if the layout direction is right to left, false otherwise. */
    bool mIsRtl = false;
    bool mFromBottomToTop = false;

    View* mParent = nullptr;
    int mDirtyPosition = RecyclerView::NO_POSITION;

    AnchorInfo* mAnchorInfo = nullptr;
    int mLastWidth = INT_MIN;
    int mLastHeight = INT_MIN;

    SparseArray<View*> mViewCache;

    OrientationHelper* mOrientationHelper = nullptr;
    OrientationHelper* mSubOrientationHelper = nullptr;
    LayoutState* mLayoutState = nullptr;
protected:
    void ensureFlexboxHelper();
    void ensureOrientationHelper();
    void resolveLayoutDirection();
    void updateAnchorInfoForLayout(RecyclerView::State& state, AnchorInfo* anchorInfo);
    bool updateAnchorFromPendingState(RecyclerView::State& state, AnchorInfo* anchorInfo, SavedState* savedState);
    bool updateAnchorFromChildren(RecyclerView::State& state, AnchorInfo* anchorInfo);
    bool updateAnchorFromPendingData(RecyclerView::State& state);
    View* findFirstReferenceChild(int itemCount);
    View* findLastReferenceChild(int itemCount);
    View* findReferenceChild(int start, int end, int itemCount);
    View* findFirstReferenceViewInLine(View* firstFound, FlexLine* flexLine);
    View* findLastReferenceViewInLine(View* lastFound, FlexLine* flexLine);
    void ensureLayoutState();

    bool isLayoutRTL();

    int fixLayoutStartGap(int startOffset, RecyclerView::Recycler& recycler, RecyclerView::State& state, bool canOffsetChildren);
    int fixLayoutEndGap(int endOffset, RecyclerView::Recycler& recycler, RecyclerView::State& state, bool canOffsetChildren);

    int handleScrollingMainOrientation(int delta, RecyclerView::Recycler& recycler, RecyclerView::State& state);
    int handleScrollingSubOrientation(int delta);

    void updateLayoutStateToFillEnd(AnchorInfo* anchorInfo, bool fromNextLine, bool considerInfinite);
    void updateLayoutStateToFillStart(AnchorInfo* anchorInfo, bool fromPreviousLine, bool considerInfinite);
    void updateLayoutState(int layoutDirection, int absDelta);
    void updateFlexLines(int childCount);

    int fill(RecyclerView::Recycler& recycler, class LayoutState& layoutState, RecyclerView::State& state, bool stopOnFocusable);
    void layoutChunk(RecyclerView::Recycler& recycler, RecyclerView::State& state, class LayoutState& layoutState, class LayoutChunkResult& result);

    void recycleChildren(RecyclerView::Recycler& recycler, int startIndex, int endIndex);
    void recycleByLayoutState(RecyclerView::Recycler& recycler, LayoutState& layoutState);
    void recycleFlexLinesFromStart(RecyclerView::Recycler& recycler, class LayoutState& layoutState);
    void recycleFlexLinesFromEnd(RecyclerView::Recycler& recycler, class LayoutState& layoutState);
    bool canViewBeRecycledFromStart(View* view, int scrollingOffset);
    bool canViewBeRecycledFromEnd(View* view, int scrollingOffset);

    int layoutFlexLine(FlexLine& flexLine, class LayoutState& layoutState, RecyclerView::Recycler& recycler);
    int layoutFlexLineMainAxisHorizontal(FlexLine& flexLine, LayoutState& layoutState);
    int layoutFlexLineMainAxisVertical(FlexLine& flexLine, LayoutState& layoutState);

    View* getChildClosestToStart();
    int computeScrollOffset(RecyclerView::State& state);
    int computeScrollExtent(RecyclerView::State& state);
    int computeScrollRange(RecyclerView::State& state);
public:
    void scrollToPositionWithOffset(int position, int offset);
public:

public:
    FlexboxLayoutManager(Context* context);
    FlexboxLayoutManager(Context* context, int flexDirection);
    FlexboxLayoutManager(Context* context, int flexDirection, int flexWrap);
    FlexboxLayoutManager(Context* context, const AttributeSet& attrs);
    ~FlexboxLayoutManager() override;

    bool isAutoMeasureEnabled() const override;
    RecyclerView::LayoutParams* generateDefaultLayoutParams() const override;
    void onDetachedFromWindow(RecyclerView& view, RecyclerView::Recycler& recycler) override;
    Parcelable* onSaveInstanceState() override;
    void onRestoreInstanceState(Parcelable& state) override;
    bool canScrollHorizontally() const override;
    bool canScrollVertically() const override;

    void onLayoutChildren(RecyclerView::Recycler& recycler, RecyclerView::State& state) override;
    void onLayoutCompleted(RecyclerView::State& state) override;
    void scrollToPosition(int position) override;
    int scrollHorizontallyBy(int dx, RecyclerView::Recycler& recycler, RecyclerView::State& state) override;
    int scrollVerticallyBy(int dy, RecyclerView::Recycler& recycler, RecyclerView::State& state) override;

    int computeHorizontalScrollOffset(RecyclerView::State& state) override;
    int computeVerticalScrollOffset(RecyclerView::State& state) override;
    int computeHorizontalScrollExtent(RecyclerView::State& state) override;
    int computeVerticalScrollExtent(RecyclerView::State& state) override;
    int computeHorizontalScrollRange(RecyclerView::State& state) override;
    int computeVerticalScrollRange(RecyclerView::State& state) override;

    bool supportsPredictiveItemAnimations() override;
 
    // FlexContainer interface implementation
    int getFlexItemCount() override;
    View* getFlexItemAt(int index) override;
    View* getReorderedFlexItemAt(int index) override;

    void addView(View* view) override;
    void addView(View* view, int index) override;
    void removeAllViews() override;
    void removeViewAt(int index) override;

    int getFlexDirection() override;
    void setFlexDirection(int flexDirection) override;

    int getFlexWrap() override;
    void setFlexWrap(int flexWrap) override;

    int getJustifyContent() override;
    void setJustifyContent(int justifyContent) override;

    int getAlignContent() override;
    void setAlignContent(int alignContent) override;
    int getAlignItems() override;
    void setAlignItems(int alignItems) override;

    std::vector<FlexLine> getFlexLines() override;
    bool isMainAxisDirectionHorizontal() const;

    int getDecorationLengthMainAxis(View* view, int index, int indexInFlexLine) override;
    int getDecorationLengthCrossAxis(View* view) override;

    int getPaddingTop() override;
    int getPaddingLeft() override;
    int getPaddingRight() override;
    int getPaddingBottom() override;
    int getPaddingStart() override;
    int getPaddingEnd() override;

    int getChildWidthMeasureSpec(int widthSpec, int padding, int childDimension) override;
    int getChildHeightMeasureSpec(int heightSpec, int padding, int childDimension) override;

    int getLargestMainSize() override;
    int getSumOfCrossSize() override;

    void onNewFlexItemAdded(View* view, int index, int indexInFlexLine, FlexLine& flexLine) override;
    void onNewFlexLineAdded(FlexLine& flexLine) override;
    void setFlexLines(const std::vector<FlexLine>& flexLines) override;

    int getMaxLine() override;
    void setMaxLine(int maxLine) override;

    std::vector<FlexLine> getFlexLinesInternal() override;
    void updateViewCache(int position, View* view) override;

    bool computeScrollVectorForPosition(int targetPosition, PointF& scrollVector) override;
    RecyclerView::LayoutParams* generateLayoutParams(Context* c, const AttributeSet& attrs) const override;
    bool checkLayoutParams(const RecyclerView::LayoutParams* lp) const override;
    void onAdapterChanged(RecyclerView::Adapter* oldAdapter, RecyclerView::Adapter* newAdapter) override;
    void onItemsAdded(RecyclerView& recyclerView, int positionStart, int itemCount) override;
    void onItemsUpdated(RecyclerView& recyclerView, int positionStart, int itemCount, Object* payload) override;
    void onItemsUpdated(RecyclerView& recyclerView, int positionStart, int itemCount) override;
    void onItemsRemoved(RecyclerView& recyclerView, int positionStart, int itemCount) override;
    void onItemsMoved(RecyclerView& recyclerView, int from, int to, int itemCount) override;
    void onAttachedToWindow(RecyclerView& recyclerView) override;

    int findFirstVisibleItemPosition();
    int findFirstCompletelyVisibleItemPosition();
    int findLastVisibleItemPosition();
    int findLastCompletelyVisibleItemPosition();

    bool getRecycleChildrenOnDetach();
    void setRecycleChildrenOnDetach(bool recycleChildrenOnDetach);

private:
    void updateDirtyPosition(int positionStart);
    View* findOneVisibleChild(int fromIndex, int toIndex, bool completelyVisible);
    bool isViewVisible(View* view, bool completelyVisible);
};
class FlexboxLayoutManager::LayoutState {
public:
    static constexpr int LAYOUT_START = -1;
    static constexpr int LAYOUT_END = 1;
    static constexpr int INVALID_LAYOUT = INT_MIN;
    static constexpr int ITEM_DIRECTION_HEAD = -1;
    static constexpr int ITEM_DIRECTION_TAIL = 1;
    static constexpr int SCROLLING_OFFSET_NaN = INT_MIN;

    bool mRecycle = true;
    bool mShouldRecycle = true;
    int mScrollingOffset = 0;
    int mLastScrollDelta = 0;

    int mAvailable;
    int mCurrentPosition;
    int mItemDirection;
    int mLayoutDirection;
    int mStartLine = 0;
    int mEndLine = 0;
    bool mStopInFocusable;
    bool mInfinite;

    int mFlexLinePosition = -1;
    int mOffset = 0;
    int mPosition = 0;

    bool hasMore(RecyclerView::State& state) {
        return mCurrentPosition >= 0 && mCurrentPosition < state.getItemCount();
    }
    View* next(RecyclerView::Recycler& recycler) {
        View* view = recycler.getViewForPosition(mCurrentPosition);
        mCurrentPosition += mItemDirection;
        return view;
    }
};
class FlexboxLayoutManager::AnchorInfo {
public:
    int mPosition = RecyclerView::NO_POSITION;
    int mFlexLinePosition = RecyclerView::NO_POSITION;
    int mCoordinate = 0;
    int mPerpendicularCoordinate = 0;
    bool mLayoutFromEnd = false;
    bool mValid = false;
    bool mAssignedFromSavedState = false;

    void reset() {
        mPosition = RecyclerView::NO_POSITION;
        mFlexLinePosition = RecyclerView::NO_POSITION;
        mCoordinate = INT_MIN;
        mValid = false;
        mAssignedFromSavedState = false;
    }

    void assignCoordinateFromPadding(FlexboxLayoutManager* manager) {
        if (!manager->isMainAxisDirectionHorizontal() && manager->mIsRtl) {
            mCoordinate = mLayoutFromEnd ? manager->mOrientationHelper->getEndAfterPadding()
                    : manager->getWidth() - manager->mOrientationHelper->getStartAfterPadding();
        } else {
            mCoordinate = mLayoutFromEnd ? manager->mOrientationHelper->getEndAfterPadding()
                    : manager->mOrientationHelper->getStartAfterPadding();
        }
    }

    void assignFromView(View* anchor, FlexboxLayoutManager* manager) {
        OrientationHelper* orientationHelper;
        if (manager->mFlexWrap == (int)FlexWrap::NOWRAP) {
            orientationHelper = manager->mSubOrientationHelper;
        } else {
            orientationHelper = manager->mOrientationHelper;
        }
        if (!manager->isMainAxisDirectionHorizontal() && manager->mIsRtl) {
            if (mLayoutFromEnd) {
                mCoordinate = orientationHelper->getDecoratedStart(anchor) +
                        orientationHelper->getTotalSpaceChange();
            } else {
                mCoordinate = orientationHelper->getDecoratedEnd(anchor);
            }
        } else {
            if (mLayoutFromEnd) {
                mCoordinate = orientationHelper->getDecoratedEnd(anchor) +
                        orientationHelper->getTotalSpaceChange();
            } else {
                mCoordinate = orientationHelper->getDecoratedStart(anchor);
            }
        }
        mPosition = manager->getPosition(anchor);
        mAssignedFromSavedState = false;
        int flexLinePosition = manager->mFlexboxHelper->getIndexToFlexLine()[mPosition != RecyclerView::NO_POSITION ? mPosition : 0];
        mFlexLinePosition = flexLinePosition != RecyclerView::NO_POSITION ? flexLinePosition : 0;
        if (manager->mFlexLines.size() > mFlexLinePosition) {
            mPosition = manager->mFlexLines[mFlexLinePosition].getFirstIndex();
        }
    }
};
class FlexboxLayoutManager::LayoutParams:public RecyclerView::LayoutParams, public FlexItem {
private:
    float mFlexGrow = FlexItem::FLEX_GROW_DEFAULT;
    float mFlexShrink = FlexItem::FLEX_SHRINK_DEFAULT;
    int mAlignSelf = (int)AlignSelf::AUTO;
    float mFlexBasisPercent = FlexItem::FLEX_BASIS_PERCENT_DEFAULT;
    int mMinWidth = 0;
    int mMinHeight = 0;
    int mMaxWidth = FlexItem::MAX_SIZE;
    int mMaxHeight = FlexItem::MAX_SIZE;
    bool mWrapBefore = false;
    int mOrder = FlexItem::ORDER_DEFAULT;

public:
    LayoutParams(Context* c, const AttributeSet& attrs);
    LayoutParams(int width, int height);
    LayoutParams(const ViewGroup::MarginLayoutParams& source);
    LayoutParams(const RecyclerView::LayoutParams& source);

    // FlexItem interface implementation
    int getWidth() override;
    void setWidth(int width) override;
    int getHeight() override;
    void setHeight(int height) override;
    int getOrder() override;
    void setOrder(int order) override;
    float getFlexGrow() override;
    void setFlexGrow(float flexGrow) override;
    float getFlexShrink() override;
    void setFlexShrink(float flexShrink) override;
    int getAlignSelf() override;
    void setAlignSelf(int alignSelf) override;
    int getMinWidth() override;
    void setMinWidth(int minWidth) override;
    int getMinHeight() override;
    void setMinHeight(int minHeight) override;
    int getMaxWidth() override;
    void setMaxWidth(int maxWidth) override;
    int getMaxHeight() override;
    void setMaxHeight(int maxHeight) override;
    bool isWrapBefore() override;
    void setWrapBefore(bool wrapBefore) override;
    float getFlexBasisPercent() override;
    void setFlexBasisPercent(float flexBasisPercent) override;
    int getMarginLeft() override;
    int getMarginTop() override;
    int getMarginRight() override;
    int getMarginBottom() override;
    int getMarginStart() override;
    int getMarginEnd() override;
};
class FlexboxLayoutManager::SavedState : public Parcelable {
private:
    int mAnchorPosition = RecyclerView::NO_POSITION;
    int mAnchorOffset = 0;
    bool mAnchorLayoutFromEnd = false;

public:
    SavedState();
    SavedState(Parcel& in);
    SavedState(const SavedState& other);

    bool hasValidAnchor(int itemCount) const;
    void invalidateAnchor();
    int describeContents();
    void writeToParcel(Parcel& dest, int flags);

    friend class FlexboxLayoutManager;
};

} // namespace cdroid

#endif /* __FLEXBOX_LAYOUT_MANAGER_H__ */
