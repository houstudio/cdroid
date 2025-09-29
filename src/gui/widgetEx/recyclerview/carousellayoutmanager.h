#ifndef __CAROUSEL_LAYOUTMANAGER_H__
#define __CAROUSEL_LAYOUTMANAGER_H__
#include <widgetEx/recyclerview/recyclerview.h>
#include <widgetEx/recyclerview/orientationhelper.h>

namespace cdroid{

class CarouselLayoutManager :public RecyclerView::LayoutManager{
public:
    static constexpr int HORIZONTAL = OrientationHelper::HORIZONTAL;
    static constexpr int VERTICAL = OrientationHelper::VERTICAL;

    static constexpr int INVALID_POSITION = -1;
    static constexpr int MAX_VISIBLE_ITEMS = 3;

    typedef CallbackBase<void,int>OnCenterItemSelectionListener;
    struct ItemTransformation {
        float mScaleX;
        float mScaleY;
        float mTranslationX;
        float mTranslationY;
        ItemTransformation(float scaleX, float scaleY, float translationX, float translationY);
    };
   /**
    * Called after child layout finished. Generally you can do any translation and scaling work here.
    *
    * @param child                    view that was layout
    * @param itemPositionToCenterDiff view center line difference to layout center. 
    *          if > 0 then this item is bellow layout center line, else if not
    * @param orientation              layoutManager orientation {@link #getLayoutDirection()}
    * @param itemPositionInAdapter    item position inside adapter for this layout pass
    */
    DECLARE_UIEVENT(bool,PostLayoutListener,View&,ItemTransformation&,float,int,int);
private:
    static constexpr bool CIRCLE_LAYOUT = false;
    struct LayoutOrder;
    class LayoutHelper;
    class CarouselLinearSmoothScroller;
    friend CarouselLinearSmoothScroller;
    int mDecoratedChildWidth;
    int mDecoratedChildHeight;
    int mOrientation;
    int mPendingScrollPosition;
    bool mDecoratedChildSizeInvalid;
    bool mCircleLayout;

    LayoutHelper* mLayoutHelper;
    PostLayoutListener mViewPostLayout;
    std::vector<OnCenterItemSelectionListener> mOnCenterItemSelectionListeners;
    int mCenterItemPosition = INVALID_POSITION;
    int mItemsCount;

private:
    int calculateScrollForSelectingPosition(int itemPosition, RecyclerView::State& state);
    void fillData(RecyclerView::Recycler& recycler, RecyclerView::State& state);
    void detectOnItemSelectionChanged(float currentScrollPosition, RecyclerView::State& state);
    void selectItemCenterPosition(int centerItem);
    void fillDataVertical(RecyclerView::Recycler& recycler, int width, int height);
    void fillDataHorizontal(RecyclerView::Recycler& recycler, int width, int height);
    void fillChildItem(int start, int top, int width, int height, LayoutOrder& layoutOrder, RecyclerView::Recycler& recycler, int i);
    /**
     * @return current scroll position of center item. this value can be in any range if it is cycle layout.
     * if this is not, that then it is in [0, {@link #mItemsCount - 1}]
     */
    float getCurrentScrollPosition()const;
    /**
     * @return maximum scroll value to fill up all items in layout. Generally this is only needed for non cycle layouts.
     */
    int getMaxScrollOffset()const;
    float getScrollDirection(int targetPosition)const;
    void generateLayoutOrder(float currentScrollPosition,RecyclerView::State& state);
    View* bindChild(int position, RecyclerView::Recycler& recycler);
    void recyclerOldViews(RecyclerView::Recycler& recycler);
    static float makeScrollPositionInRange0ToCount(float currentScrollPosition, int count);
protected:
    class CarouselSavedState;
    CarouselSavedState* mPendingCarouselSavedState;
    int scrollBy(int diff,RecyclerView::Recycler& recycler, RecyclerView::State& state);
    int getCardOffsetByPositionDiff(float itemPositionDiff);
    double convertItemPositionDiffToSmoothPositionDiff(float itemPositionDiff);
    int getScrollItemSize()const;
    int getOffsetCenterView();
    int getOffsetForCurrentView(View& view);
public:
    /**
     * @param orientation should be {@link #VERTICAL} or {@link #HORIZONTAL}
     */
    CarouselLayoutManager(int orientation);
    CarouselLayoutManager(int orientation, bool circleLayout);
    /**
     * Change circle layout type
     */
    void setCircleLayout(bool circleLayout);
    /**
     * Setup {@link CarouselLayoutManager::PostLayoutListener} for this LayoutManager.
     * Its methods will be called for each visible view item after general LayoutManager layout finishes. <br />
     * <br />
     * Generally this method should be used for scaling and translating view item for better (different) view presentation of layouting.
     *
     * @param postLayoutListener listener for item layout changes. Can be null.
     */
    void setPostLayoutListener(const PostLayoutListener& postLayoutListener);

    /**
     * Setup maximum visible (layout) items on each side of the center item.
     * Basically during scrolling there can be more visible items (+1 item on each side), but in idle state this is the only reached maximum.
     *
     * @param maxVisibleItems should be great then 0, if bot an {@link IllegalAccessException} will be thrown
     */
    void setMaxVisibleItems(int maxVisibleItems);

    /**
     * @return current setup for maximum visible items.
     * @see #setMaxVisibleItems(int)
     */
    int getMaxVisibleItems()const;

    RecyclerView::LayoutParams* generateDefaultLayoutParams()const override;

    /**
     * @return current layout orientation
     * @see #VERTICAL
     * @see #HORIZONTAL
     */
    int getOrientation()const;

    bool canScrollHorizontally()const override;
    bool canScrollVertically()const override;

    /**
     * @return current layout center item
     */
    int getCenterItemPosition()const;

    /**
     * @param onCenterItemSelectionListener listener that will trigger when ItemSelectionChanges. can't be null
     */
    void addOnItemSelectionListener(OnCenterItemSelectionListener onCenterItemSelectionListener);

    /**
     * @param onCenterItemSelectionListener listener that was previously added by {@link #addOnItemSelectionListener(OnCenterItemSelectionListener)}
     */
    void removeOnItemSelectionListener(OnCenterItemSelectionListener onCenterItemSelectionListener);

    void scrollToPosition(int position) override;

    void smoothScrollToPosition(RecyclerView& recyclerView, RecyclerView::State& state, int position)override;
    bool computeScrollVectorForPosition(int targetPosition,PointF&scrollVector)override;
    int scrollVerticallyBy(int dy, RecyclerView::Recycler& recycler, RecyclerView::State& state)override;
    int scrollHorizontallyBy(int dx, RecyclerView::Recycler& recycler, RecyclerView::State& state)override;
    void onMeasure(RecyclerView::Recycler& recycler, RecyclerView::State& state, int widthSpec, int heightSpec)override;
    void onAdapterChanged(RecyclerView::Adapter* oldAdapter, RecyclerView::Adapter* newAdapter)override;
    void onLayoutChildren(RecyclerView::Recycler& recycler,RecyclerView::State& state)override;

    int getWidthNoPadding();
    int getHeightNoPadding();
    Parcelable* onSaveInstanceState()override;
    void onRestoreInstanceState(Parcelable& state)override;

    /**
     * This interface methods will be called for each visible view item after general LayoutManager layout finishes. <br />
     * <br />
     * Generally this method should be used for scaling and translating view item for better (different) view presentation of layouting.
     */

};/*end carousellayoutmanager*/

struct CarouselLayoutManager::LayoutOrder {
    int mItemAdapterPosition;
    float mItemPositionDiff;
    friend LayoutHelper;
};

class CarouselLayoutManager::LayoutHelper {
private :
    friend CarouselLayoutManager;
    int mMaxVisibleItems;
    int mScrollOffset;
    std::vector<std::shared_ptr<LayoutOrder>> mLayoutOrder;
    std::vector<std::weak_ptr<LayoutOrder>> mReusedItems;
private:
    void recycleItems(const std::vector<std::shared_ptr<LayoutOrder>>& layoutOrders);
    void fillLayoutOrder();
    std::shared_ptr<LayoutOrder> createLayoutOrder();
public:
    LayoutHelper(int maxVisibleItems);

    void initLayoutOrder(int layoutCount);
    void setLayoutOrder(int arrayPosition, int itemAdapterPosition, float itemPositionDiff);
    bool hasAdapterPosition(int adapterPosition);
};

class CarouselLayoutManager::CarouselSavedState :public Parcelable {
private:
    friend CarouselLayoutManager;
    Parcelable* mSuperState;
    int mCenterItemPosition;
    CarouselSavedState(Parcel& in);
protected:
    CarouselSavedState(Parcelable* superState);
    CarouselSavedState(CarouselSavedState& other);
public:
    int describeContents();
    void writeToParcel(Parcel& parcel, int i);
};

}/*endof namespace*/
#endif/*__CAROUSEL_LAYOUTMANAGER_H__*/
