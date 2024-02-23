#ifndef __GRID_LAYOUT_MANAGER_H__
#define __GRID_LAYOUT_MANAGER_H__
#include <widgetEx/recyclerview/linearlayoutmanager.h>
namespace cdroid{

class GridLayoutManager:public LinearLayoutManager {
private:
    static constexpr bool _DEBUG = false;
public:
    static constexpr int DEFAULT_SPAN_COUNT = -1;
    class SpanSizeLookup;
    class DefaultSpanSizeLookup;
    class LayoutParams;
protected:
    bool mPendingSpanCountChange = false;
    int mSpanCount = DEFAULT_SPAN_COUNT;
    std::vector<int>mCachedBorders;/*int []*/
    std::vector<View*>mSet;//[] mSet;
    SparseIntArray mPreLayoutSpanSizeCache;
    SparseIntArray mPreLayoutSpanIndexCache;;
    SpanSizeLookup* mSpanSizeLookup;// = new DefaultSpanSizeLookup();
    // re-used variable to acquire decor insets from RecyclerView
    Rect mDecorInsets;
private:
    void clearPreLayoutSpanMappingCache();
    void cachePreLayoutSpanMapping();
    void calculateItemBorders(int totalSpace);
    void updateMeasurements();
    void ensureViewSet();
    void ensureAnchorIsInCorrectSpan(RecyclerView::Recycler& recycler,
            RecyclerView::State& state, AnchorInfo& anchorInfo, int itemDirection);
    int getSpanGroupIndex(RecyclerView::Recycler& recycler, RecyclerView::State& state,int viewPosition);
    int getSpanIndex(RecyclerView::Recycler& recycler, RecyclerView::State& state, int pos);
    int getSpanSize(RecyclerView::Recycler& recycler, RecyclerView::State& state, int pos);
    void measureChild(View* view, int otherDirParentSpecMode, bool alreadyMeasured);
    void guessMeasurement(float maxSizeInOther, int currentOtherDirSize);
    void measureChildWithDecorationsAndMargin(View* child, int widthSpec, int heightSpec,
            bool alreadyMeasured);
    void assignSpans(RecyclerView::Recycler& recycler, RecyclerView::State& state, int count,
            int consumedSpanCount, bool layingOutInPrimaryDirection);

protected:
    static int* calculateItemBorders(std::vector<int>&cachedBorders, int spanCount, int totalSpace);
    int getSpaceForSpanRange(int startSpan, int spanSize); 

    void onAnchorReady(RecyclerView::Recycler& recycler, RecyclerView::State& state,
                       AnchorInfo& anchorInfo, int itemDirection)override;
    View* findReferenceChild(RecyclerView::Recycler& recycler, RecyclerView::State& state,
                            int start, int end, int itemCount)override;
    void collectPrefetchPositionsForLayoutState(RecyclerView::State& state, LayoutState& layoutState,
            LayoutPrefetchRegistry& layoutPrefetchRegistry)override;
    void layoutChunk(RecyclerView::Recycler& recycler, RecyclerView::State& state,
            LayoutState& layoutState, LayoutChunkResult& result)override;
public:
    GridLayoutManager(Context* context, const AttributeSet& attrs);
    GridLayoutManager(Context* context, int spanCount);
    GridLayoutManager(Context* context, int spanCount,int orientation, bool reverseLayout);
    ~GridLayoutManager()override;
    void setStackFromEnd(bool stackFromEnd)override;
    int getRowCountForAccessibility(RecyclerView::Recycler& recycler, RecyclerView::State& state)override;
    int getColumnCountForAccessibility(RecyclerView::Recycler& recycler, RecyclerView::State& state)override;
    //void onInitializeAccessibilityNodeInfoForItem(RecyclerView::Recycler& recycler,
    //        RecyclerView::State& state, View* host, AccessibilityNodeInfo& info)override;
    void onLayoutChildren(RecyclerView::Recycler& recycler, RecyclerView::State& state)override;
    void onLayoutCompleted(RecyclerView::State& state)override;

    void onItemsAdded(RecyclerView& recyclerView, int positionStart, int itemCount)override;

    void onItemsChanged(RecyclerView& recyclerView)override;
    void onItemsRemoved(RecyclerView& recyclerView, int positionStart, int itemCount)override;
    void onItemsUpdated(RecyclerView& recyclerView, int positionStart, int itemCount,Object* payload)override;
    void onItemsMoved(RecyclerView& recyclerView, int from, int to, int itemCount)override;
    RecyclerView::LayoutParams* generateDefaultLayoutParams()const override;
    RecyclerView::LayoutParams* generateLayoutParams(Context* c,const AttributeSet& attrs)const override;
    RecyclerView::LayoutParams* generateLayoutParams(const ViewGroup::LayoutParams& lp)const override;

    bool checkLayoutParams(const RecyclerView::LayoutParams* lp)const override;
    void setSpanSizeLookup(SpanSizeLookup* spanSizeLookup);
    SpanSizeLookup* getSpanSizeLookup();
    void setMeasuredDimension(Rect& childrenBounds, int wSpec, int hSpec)override;

    int scrollHorizontallyBy(int dx, RecyclerView::Recycler& recycler,RecyclerView::State& state)override;
    int scrollVerticallyBy(int dy, RecyclerView::Recycler& recycler,RecyclerView::State& state)override;
    int getSpanCount();
    void setSpanCount(int spanCount); 

    View* onFocusSearchFailed(View* focused, int focusDirection,
            RecyclerView::Recycler& recycler, RecyclerView::State& state)override;
    bool supportsPredictiveItemAnimations()override;

};

class GridLayoutManager::SpanSizeLookup {
protected:
    SparseIntArray mSpanIndexCache;// = new SparseIntArray();
private:
    bool mCacheSpanIndices = false;
protected:
    friend class GridLayoutManager;
    int getCachedSpanIndex(int position, int spanCount);
    int findReferenceIndexFromCache(int position);
public:
    virtual int getSpanSize(int position)=0;
    void setSpanIndexCacheEnabled(bool cacheSpanIndices);
    void invalidateSpanIndexCache();
    bool isSpanIndexCacheEnabled();
    virtual int getSpanIndex(int position, int spanCount);
    int getSpanGroupIndex(int adapterPosition, int spanCount);
};

class GridLayoutManager::DefaultSpanSizeLookup:public GridLayoutManager::SpanSizeLookup {
public:
    int getSpanSize(int position)override;
    int getSpanIndex(int position, int spanCount)override;
};

class GridLayoutManager::LayoutParams:public RecyclerView::LayoutParams {
public:
    static constexpr int INVALID_SPAN_ID = -1;
protected:
    int mSpanIndex = INVALID_SPAN_ID;
    int mSpanSize = 0;
    friend GridLayoutManager;
public:
    LayoutParams(Context* c,const AttributeSet& attrs);
    LayoutParams(int width, int height);
    LayoutParams(const ViewGroup::MarginLayoutParams& source);
    LayoutParams(const ViewGroup::LayoutParams& source);
    LayoutParams(const RecyclerView::LayoutParams& source);
    int getSpanIndex()const;
    int getSpanSize()const;
};
}/*endof namespace*/
#endif/*__GRID_LAYOUT_MANAGER_H__*/
