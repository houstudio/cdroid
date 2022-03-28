#ifndef __LIST_VIEW_H__
#define __LIST_VIEW_H__
#include <widget/abslistview.h>
#include <widget/headerviewlistadapter.h>
namespace cdroid{

class FixedViewInfo {/** The view to add to the list */
public:
    View* view;
    /** The data backing the view. This is returned from {@link ListAdapter#getItem(int)}. */
    void*data;
    /** <code>true</code> if the fixed view should be selectable in the list */
    bool isSelectable;
};
class ListView:public AbsListView{
private:
    class ArrowScrollFocusResult {
    private:
        int mSelectedPosition;
        int mAmountToScroll;
    public:
        void populate(int selectedPosition, int amountToScroll) {
            mSelectedPosition = selectedPosition;
            mAmountToScroll = amountToScroll;
        }
        int getSelectedPosition(){return mSelectedPosition;}
        int getAmountToScroll() { return mAmountToScroll; }
    };
    class FocusSelector:public Runnable {
    private:
        // the selector is waiting to set selection on the list view
        static constexpr int STATE_SET_SELECTION = 1;
        // the selector set the selection on the list view, waiting for a layoutChildren pass
        static constexpr int STATE_WAIT_FOR_LAYOUT = 2;
        // the selector's selection has been honored and it is waiting to request focus on the
        // target child.
        static constexpr int STATE_REQUEST_FOCUS = 3;

        int mAction;
        int mPosition;
        int mPositionTop;
        ListView*mLV;
    public:
        FocusSelector(ListView*lv);
        FocusSelector& setupForSetSelection(int position, int top);
        void operator()(); 
        bool setupFocusIfValid(int position);
        void onLayoutComplete();
    };	
    bool mIsCacheColorOpaque;
    bool mDividerIsOpaque;
    bool mHeaderDividersEnabled;
    bool mFooterDividersEnabled;
    bool mItemsCanFocus;
    bool mAreAllItemsSelectable;
    ArrowScrollFocusResult mArrowScrollFocusResult;
    FocusSelector* mFocusSelector;

    void initListView(const AttributeSet&attrs);
    void clearRecycledState(std::vector<FixedViewInfo*>& infos);
    bool showingTopFadingEdge();
    bool showingBottomFadingEdge();
    void removeFixedViewInfo(View* v, std::vector<FixedViewInfo*>& where);
    void removeUnusedFixedViews(std::vector<FixedViewInfo*>& infoList);

    bool isDirectChildHeaderOrFooter(View* child);

    int getTopSelectionPixel(int childrenTop, int fadingEdgeLength, int selectedPosition);
    int getBottomSelectionPixel(int childrenBottom, int fadingEdgeLength,int selectedPosition);
    bool shouldAdjustHeightForDivider(int itemIndex);
    void adjustViewsUpOrDown();
    void setupChild(View* child, int position, int y, bool flowDown, int childrenLeft,
            bool selected, bool isAttachedToWindow);
    View*makeAndAddView(int position, int y, bool flow, int childrenLeft,bool selected);
    void correctTooLow(int childCount);
    void correctTooHigh(int childCount);
    void fillAboveAndBelow(View* sel, int position);
    View* fillFromSelection(int selectedTop, int childrenTop, int childrenBottom);
    View* fillSpecific(int position, int top);
    void measureScrapChild(View* child, int position, int widthMeasureSpec, int heightHint);

    View* addViewAbove(View* theView, int position);
    View* addViewBelow(View* theView, int position);

    View* moveSelection(View* oldSel, View* newSel, int delta, int childrenTop, int childrenBottom);
    int positionOfNewFocus(View* newFocus);
    int lookForSelectablePositionOnScreen(int direction);
    void measureItem(View* child);
    void relayoutMeasuredItem(View* child);
    void measureAndAdjustDown(View* child, int childIndex, int numChildren);
    bool handleHorizontalFocusWithinListItem(int direction);
    void handleNewSelectionChange(View* selectedView, int direction, int newSelectedPosition, bool newFocusAssigned);
    class ArrowScrollFocusResult* arrowScrollFocused(int direction);
    int getArrowScrollPreviewLength();
    int amountToScroll(int direction, int nextSelectedPosition);
    int amountToScrollToNewFocus(int direction, View* newFocus, int positionOfNewFocus);
    int distanceToView(View* descendant);
    bool isViewAncestorOf(View* child, View* parent);
    void scrollListItemsBy(int amount);
    int nextSelectedPositionForDirection(View* selectedView, int selectedPos, int direction);
    bool arrowScroll(int direction);
    bool arrowScrollImpl(int direction);
    bool commonKey(int keyCode, int count, KeyEvent& event);
protected:
    int  mDividerHeight;
    Drawable * mDivider;
    Drawable * mOverScrollHeader;
    Drawable * mOverScrollFooter;
    std::vector<FixedViewInfo*>mHeaderViewInfos;
    std::vector<FixedViewInfo*>mFooterViewInfos;

    HeaderViewListAdapter* wrapHeaderListAdapterInternal(
        const std::vector<FixedViewInfo*>& headerViewInfos,
        const std::vector<FixedViewInfo*>& footerViewInfos,
        Adapter* adapter);
    void wrapHeaderListAdapterInternal();
    void resetList()override;
    void onDetachedFromWindow()override;
    void layoutChildren()override;
    View*findViewByPredicateTraversal(std::function<bool(const View*)>predicate,View* childToSkip)override;
    View* findViewByPredicateInHeadersOrFooters(const std::vector<FixedViewInfo*>&where,
            std::function<bool(const View*)>predicate, View* childToSkip);
    View* findViewWithTagInHeadersOrFooters(std::vector<FixedViewInfo*>& where, void* tag);
    int getHeightForPosition(int position)override;
    View* fillFromTop(int nextTop);
    View* fillDown(int pos, int nextTop);
    View* fillUp(int pos, int nextBottom);
    View* fillFromMiddle(int childrenTop, int childrenBottom);
    void fillGap(bool down)override;
    bool fullScroll(int direction);
    bool pageScroll(int direction);
    bool trackMotionScroll(int deltaY, int incrementalDeltaY)override;

    int findMotionRow(int y)override;
    bool canAnimate()const override;
    void setSelectionInt(int position);
    void drawDivider(Canvas&canvas,const Rect&bounds, int childIndex);
    void drawOverscrollHeader(Canvas&canvas, Drawable* drawable,Rect& bounds);
    void drawOverscrollFooter(Canvas&canvas, Drawable* drawable,Rect& bounds);

    bool recycleOnMeasure();
    int lookForSelectablePositionAfter(int current, int position, bool lookDown);
    int measureHeightOfChildren(int widthMeasureSpec, int startPosition, int endPosition,
            int maxHeight, int disallowPartialChildPosition);
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec)override;
    void onSizeChanged(int w, int h, int oldw, int oldh)override;
    void onFocusChanged(bool gainFocus, int direction,Rect* previouslyFocusedRect)override;
    bool drawChild(Canvas&,View*,long)override;
    void dispatchDraw(Canvas&)override;
public:
    ListView(int w,int h);
    ListView(Context* context,const AttributeSet& attrs);
    ~ListView();
    void setAdapter(Adapter* adapter)override;
    void setSelection(int position);
    bool getItemsCanFocus()const;
    void setItemsCanFocus(bool itemsCanFocus);
    bool isOpaque()const override;
    void setDividerHeight(int);
    int getDividerHeight()const;
    Drawable*getDevider();
    void setDivider(Drawable*d);
    int getMaxScrollAmount()const;
    void setOverscrollHeader(Drawable*);
    Drawable*getOverscrollHeader()const;
    void setOverscrollFooter(Drawable*);
    Drawable* getOverscrollFooter()const;
    void addHeaderView(View* v,void* data, bool isSelectable);
    void addHeaderView(View* v);
    int getHeaderViewsCount()const;
    bool removeHeaderView(View* v);
    void addFooterView(View* v,void* data, bool isSelectable);
    void addFooterView(View* v);
    int getFooterViewsCount()const;
    bool removeFooterView(View* v);
    void setHeaderDividersEnabled(bool headerDividersEnabled);
    bool areHeaderDividersEnabled()const;
    void setFooterDividersEnabled(bool footerDividersEnabled);
    bool areFooterDividersEnabled()const;
    bool requestChildRectangleOnScreen(View* child, Rect& rect, bool immediate);
    bool onKeyDown(int keyCode, KeyEvent& event)override;
    bool onKeyMultiple(int keyCode, int repeatCount, KeyEvent& event)override;
    void setSelectionAfterHeaderView();
    bool dispatchKeyEvent(KeyEvent& event)override;
};

}//namespace
#endif
