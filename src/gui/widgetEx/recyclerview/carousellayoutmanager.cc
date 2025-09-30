#include <widgetEx/recyclerview/carousellayoutmanager.h>
#include <widgetEx/recyclerview/linearsmoothscroller.h>
#include <core/mathutils.h>
/*REF:
*https://gitee.com/Cserfox/Awesome-RecyclerView-LayoutManager
*https://github.com/Azoft/CarouselLayoutManager
*/

namespace cdroid{

CarouselLayoutManager::ItemTransformation::ItemTransformation(float scaleX, float scaleY, float translationX, float translationY) {
    mScaleX = scaleX;
    mScaleY = scaleY;
    mTranslationX = translationX;
    mTranslationY = translationY;
}

CarouselLayoutManager::CarouselLayoutManager(int orientation)
    :CarouselLayoutManager(orientation, CIRCLE_LAYOUT){
}

CarouselLayoutManager::CarouselLayoutManager(int orientation, bool circleLayout) {
    if (HORIZONTAL != orientation && VERTICAL != orientation) {
        throw std::runtime_error("orientation should be HORIZONTAL or VERTICAL");
    }
    mOrientation = orientation;
    mCircleLayout = circleLayout;
    mDecoratedChildWidth  = INT_MIN;
    mDecoratedChildHeight = INT_MIN;
    mDecoratedChildSizeInvalid = false;
    mPendingScrollPosition = INVALID_POSITION;
    mLayoutHelper = new LayoutHelper(MAX_VISIBLE_ITEMS);
}

/**
 * Change circle layout type
 */
void CarouselLayoutManager::setCircleLayout(bool circleLayout) {
    if (mCircleLayout != circleLayout) {
        mCircleLayout = circleLayout;
        requestLayout();
    }
}

/**
 * Setup {@link CarouselLayoutManager.PostLayoutListener} for this LayoutManager.
 * Its methods will be called for each visible view item after general LayoutManager layout finishes. <br />
 * <br />
 * Generally this method should be used for scaling and translating view item for better (different) view presentation of layouting.
 *
 * @param postLayoutListener listener for item layout changes. Can be null.
 */
void CarouselLayoutManager::setPostLayoutListener(const PostLayoutListener& postLayoutListener) {
    mViewPostLayout = postLayoutListener;
    requestLayout();
}

/**
 * Setup maximum visible (layout) items on each side of the center item.
 * Basically during scrolling there can be more visible items (+1 item on each side), but in idle state this is the only reached maximum.
 *
 * @param maxVisibleItems should be great then 0, if bot an {@link IllegalAccessException} will be thrown
 */
void CarouselLayoutManager::setMaxVisibleItems(int maxVisibleItems) {
    if (0 > maxVisibleItems) {
        throw std::runtime_error("maxVisibleItems can't be less then 0");
    }
    mLayoutHelper->mMaxVisibleItems = maxVisibleItems;
    requestLayout();
}

/**
 * @return current setup for maximum visible items.
 * @see #setMaxVisibleItems(int)
 */
int CarouselLayoutManager::getMaxVisibleItems() const{
    return mLayoutHelper->mMaxVisibleItems;
}

RecyclerView::LayoutParams* CarouselLayoutManager::generateDefaultLayoutParams() const{
    return new RecyclerView::LayoutParams(ViewGroup::LayoutParams::WRAP_CONTENT, ViewGroup::LayoutParams::WRAP_CONTENT);
}

/**
 * @return current layout orientation
 * @see #VERTICAL
 * @see #HORIZONTAL
 */
int CarouselLayoutManager::getOrientation() const{
    return mOrientation;
}

bool CarouselLayoutManager::canScrollHorizontally() const{
    return 0 != getChildCount() && HORIZONTAL == mOrientation;
}

bool CarouselLayoutManager::canScrollVertically() const{
    return 0 != getChildCount() && VERTICAL == mOrientation;
}

/**
 * @return current layout center item
 */
int CarouselLayoutManager::getCenterItemPosition() const{
    return mCenterItemPosition;
}

/**
 * @param onCenterItemSelectionListener listener that will trigger when ItemSelectionChanges. can't be null
 */
void CarouselLayoutManager::addOnItemSelectionListener(OnCenterItemSelectionListener onCenterItemSelectionListener) {
    mOnCenterItemSelectionListeners.push_back(onCenterItemSelectionListener);
}

/**
 * @param onCenterItemSelectionListener listener that was previously added by {@link #addOnItemSelectionListener(OnCenterItemSelectionListener)}
 */
void CarouselLayoutManager::removeOnItemSelectionListener(OnCenterItemSelectionListener onCenterItemSelectionListener) {
    auto it = std::find(mOnCenterItemSelectionListeners.begin(),
		    mOnCenterItemSelectionListeners.end(),onCenterItemSelectionListener);
    mOnCenterItemSelectionListeners.erase(it);//remove(onCenterItemSelectionListener);
}

void CarouselLayoutManager::scrollToPosition(int position) {
    FATAL_IF(position<0,"position can't be less then 0. position is : %d" ,position);
    mPendingScrollPosition = position;
    requestLayout();
}

class CarouselLayoutManager::CarouselLinearSmoothScroller:public LinearSmoothScroller{
private:
    CarouselLayoutManager*mLM;
public:
    CarouselLinearSmoothScroller(Context*ctx,CarouselLayoutManager*lm):LinearSmoothScroller(ctx),mLM(lm){
    }
    int calculateDyToMakeVisible(View* view, int snapPreference) override{
        if (!mLM->canScrollVertically()) {
            return 0;
        }
        return mLM->getOffsetForCurrentView(*view);
    }
    int calculateDxToMakeVisible(View* view, int snapPreference) override{
        if (!mLM->canScrollHorizontally()) {
            return 0;
        }
        return mLM->getOffsetForCurrentView(*view);
    }
};

void CarouselLayoutManager::smoothScrollToPosition(RecyclerView& recyclerView, RecyclerView::State& state, int position) {
    LinearSmoothScroller* linearSmoothScroller = new CarouselLinearSmoothScroller(recyclerView.getContext(),this);
    linearSmoothScroller->setTargetPosition(position);
    startSmoothScroll(linearSmoothScroller);
}

bool CarouselLayoutManager::computeScrollVectorForPosition(int targetPosition,PointF&pt) {
    const float directionDistance = getScrollDirection(targetPosition);
    //noinspection NumericCastThatLosesPrecision
    const int direction = (int) -MathUtils::signum(directionDistance);
    pt.x = pt.y = 0;
    if (HORIZONTAL == mOrientation) {
        pt.x = direction;
    } else {
        pt.y = direction;
    }
    return getChildCount()>0;
}

float CarouselLayoutManager::getScrollDirection(int targetPosition) const{
    float currentScrollPosition = makeScrollPositionInRange0ToCount(getCurrentScrollPosition(), mItemsCount);

    if (mCircleLayout) {
        float t1 = currentScrollPosition - targetPosition;
        float t2 = std::abs(t1) - mItemsCount;
        if (std::abs(t1) > std::abs(t2)) {
            return MathUtils::signum(t1) * t2;
        } else {
            return t1;
        }
    } else {
        return currentScrollPosition - targetPosition;
    }
}

int CarouselLayoutManager::scrollVerticallyBy(int dy, RecyclerView::Recycler& recycler, RecyclerView::State& state) {
    if (HORIZONTAL == mOrientation) {
        return 0;
    }
    return scrollBy(dy, recycler, state);
}

int CarouselLayoutManager::scrollHorizontallyBy(int dx, RecyclerView::Recycler& recycler, RecyclerView::State& state) {
    if (VERTICAL == mOrientation) {
        return 0;
    }
    return scrollBy(dx, recycler, state);
}

/**
 * This method is called from {@link #scrollHorizontallyBy(int, RecyclerView.Recycler, RecyclerView.State)} and
 * {@link #scrollVerticallyBy(int, RecyclerView.Recycler, RecyclerView.State)} to calculate needed scroll that is allowed. <br />
 * <br />
 * This method may do relayout work.
 *
 * @param diff     distance that we want to scroll by
 * @param recycler Recycler to use for fetching potentially cached views for a position
 * @param state    Transient state of RecyclerView
 * @return distance that we actually scrolled by
 */
int CarouselLayoutManager::scrollBy(int diff, RecyclerView::Recycler& recycler, RecyclerView::State& state) {
    if ((INT_MIN == mDecoratedChildWidth) || (INT_MIN == mDecoratedChildHeight)) {
        return 0;
    }
    if (0 == getChildCount() || 0 == diff) {
        return 0;
    }
    int resultScroll;
    if (mCircleLayout) {
        resultScroll = diff;

        mLayoutHelper->mScrollOffset += resultScroll;

        int maxOffset = getScrollItemSize() * mItemsCount;
        while (0 > mLayoutHelper->mScrollOffset) {
            mLayoutHelper->mScrollOffset += maxOffset;
        }
        while (mLayoutHelper->mScrollOffset > maxOffset) {
            mLayoutHelper->mScrollOffset -= maxOffset;
        }

        mLayoutHelper->mScrollOffset -= resultScroll;
    } else {
        int maxOffset = getMaxScrollOffset();

        if (0 > mLayoutHelper->mScrollOffset + diff) {
            resultScroll = -mLayoutHelper->mScrollOffset; //to make it 0
        } else if (mLayoutHelper->mScrollOffset + diff > maxOffset) {
            resultScroll = maxOffset - mLayoutHelper->mScrollOffset; //to make it maxOffset
        } else {
            resultScroll = diff;
        }
    }
    if (0 != resultScroll) {
        mLayoutHelper->mScrollOffset += resultScroll;
        fillData(recycler, state);
    }
    return resultScroll;
}

void CarouselLayoutManager::onMeasure(RecyclerView::Recycler& recycler, RecyclerView::State& state, int widthSpec, int heightSpec) {
    mDecoratedChildSizeInvalid = true;
    LayoutManager::onMeasure(recycler, state, widthSpec, heightSpec);
}

void CarouselLayoutManager::onAdapterChanged(RecyclerView::Adapter* oldAdapter, RecyclerView::Adapter* newAdapter) {
    LayoutManager::onAdapterChanged(oldAdapter, newAdapter);

    removeAllViews();
}

void CarouselLayoutManager::onLayoutChildren(RecyclerView::Recycler& recycler, RecyclerView::State& state) {
    if (0 == state.getItemCount()) {
        removeAndRecycleAllViews(recycler);
        selectItemCenterPosition(INVALID_POSITION);
        return;
    }

    detachAndScrapAttachedViews(recycler);

    if ((INT_MIN == mDecoratedChildWidth) || mDecoratedChildSizeInvalid) {
        std::vector<RecyclerView::ViewHolder*> scrapList = recycler.getScrapList();

        bool shouldRecycle;
        View* view;
        if (scrapList.empty()) {
            shouldRecycle = true;
            int itemsCount = state.getItemCount();
            view = recycler.getViewForPosition(mPendingScrollPosition == INVALID_POSITION ?
                         0 : std::max(0, std::min(itemsCount - 1, mPendingScrollPosition)) );
            addView(view);
        } else {
            shouldRecycle = false;
            view = scrapList.at(0)->itemView;
        }
        measureChildWithMargins(view, 0, 0);

        const int decoratedChildWidth = getDecoratedMeasuredWidth(view);
        const int decoratedChildHeight = getDecoratedMeasuredHeight(view);
        if (shouldRecycle) {
            detachAndScrapView(view, recycler);
        }

        if ((INT_MIN != mDecoratedChildWidth) && ((mDecoratedChildWidth != decoratedChildWidth) || (mDecoratedChildHeight != decoratedChildHeight))) {
            if ((INVALID_POSITION == mPendingScrollPosition) && (nullptr == mPendingCarouselSavedState)) {
                mPendingScrollPosition = mCenterItemPosition;
            }
        }

        mDecoratedChildWidth = decoratedChildWidth;
        mDecoratedChildHeight = decoratedChildHeight;
        mDecoratedChildSizeInvalid = false;
    }

    if (INVALID_POSITION != mPendingScrollPosition) {
        int itemsCount = state.getItemCount();
        mPendingScrollPosition = 0 == itemsCount ? INVALID_POSITION : std::max(0, std::min(itemsCount - 1, mPendingScrollPosition));
    }
    if (INVALID_POSITION != mPendingScrollPosition) {
        mLayoutHelper->mScrollOffset = calculateScrollForSelectingPosition(mPendingScrollPosition, state);
        mPendingScrollPosition = INVALID_POSITION;
        mPendingCarouselSavedState = nullptr;
    } else if (mPendingCarouselSavedState) {
        mLayoutHelper->mScrollOffset = calculateScrollForSelectingPosition(mPendingCarouselSavedState->mCenterItemPosition, state);
        mPendingCarouselSavedState = nullptr;
    } else if (state.didStructureChange() && INVALID_POSITION != mCenterItemPosition) {
        mLayoutHelper->mScrollOffset = calculateScrollForSelectingPosition(mCenterItemPosition, state);
    }

    fillData(recycler, state);
}

int CarouselLayoutManager::calculateScrollForSelectingPosition(int itemPosition, RecyclerView::State& state) {
    if (itemPosition == INVALID_POSITION) {
        return 0;
    }

    int fixedItemPosition = itemPosition < state.getItemCount() ? itemPosition : state.getItemCount() - 1;
    return fixedItemPosition * (VERTICAL == mOrientation ? mDecoratedChildHeight : mDecoratedChildWidth);
}

void CarouselLayoutManager::fillData(RecyclerView::Recycler& recycler, RecyclerView::State& state) {
    const float currentScrollPosition = getCurrentScrollPosition();

    generateLayoutOrder(currentScrollPosition, state);
    detachAndScrapAttachedViews(recycler);
    recyclerOldViews(recycler);

    const int width = getWidthNoPadding();
    const int height = getHeightNoPadding();
    if (VERTICAL == mOrientation) {
        fillDataVertical(recycler, width, height);
    } else {
        fillDataHorizontal(recycler, width, height);
    }

    recycler.clear();

    detectOnItemSelectionChanged(currentScrollPosition, state);
}

void CarouselLayoutManager::detectOnItemSelectionChanged(float currentScrollPosition, RecyclerView::State& state) {
    float absCurrentScrollPosition = makeScrollPositionInRange0ToCount(currentScrollPosition, state.getItemCount());
    int centerItem = std::round(absCurrentScrollPosition);

    if (mCenterItemPosition != centerItem) {
        mCenterItemPosition = centerItem;
        /*new Handler(Looper.getMainLooper()).post(new Runnable() {
            @Override
            public void run() {
                selectItemCenterPosition(centerItem);
            }
        });*/
    }
}

void CarouselLayoutManager::selectItemCenterPosition(int centerItem) {
    for (OnCenterItemSelectionListener onCenterItemSelectionListener : mOnCenterItemSelectionListeners) {
        onCenterItemSelectionListener(centerItem);
    }
}

void CarouselLayoutManager::fillDataVertical(RecyclerView::Recycler& recycler, int width, int height) {
    int start = (width - mDecoratedChildWidth) / 2;

    int centerViewTop = (height - mDecoratedChildHeight) / 2;

    for (int i = 0, count = mLayoutHelper->mLayoutOrder.size(); i < count; ++i) {
        std::shared_ptr<LayoutOrder> layoutOrder = mLayoutHelper->mLayoutOrder[i];
        const int offset = getCardOffsetByPositionDiff(layoutOrder->mItemPositionDiff);
        const int top = centerViewTop + offset;
        fillChildItem(start, top, mDecoratedChildWidth, mDecoratedChildHeight, *layoutOrder, recycler, i);
    }
}

void CarouselLayoutManager::fillDataHorizontal(RecyclerView::Recycler& recycler, int width, int height) {
    int top = (height - mDecoratedChildHeight) / 2;

    int centerViewStart = (width - mDecoratedChildWidth) / 2;
LOGD("count=%d",mLayoutHelper->mLayoutOrder.size());
    for (int i = 0, count = mLayoutHelper->mLayoutOrder.size(); i < count; ++i) {
        std::shared_ptr<LayoutOrder> layoutOrder = mLayoutHelper->mLayoutOrder[i];
        const int offset = getCardOffsetByPositionDiff(layoutOrder->mItemPositionDiff);
        const int start = centerViewStart + offset;
        fillChildItem(start, top, mDecoratedChildWidth,mDecoratedChildHeight, *layoutOrder, recycler, i);
    }
}


void CarouselLayoutManager::fillChildItem(int start, int top, int width, int height,
		LayoutOrder& layoutOrder, RecyclerView::Recycler& recycler, int i) {
    View* view = bindChild(layoutOrder.mItemAdapterPosition, recycler);
    view->setElevation(i);
    ItemTransformation transformation = {1,1,0,0};
    view->layout(start, top, width,height);
    if ((mViewPostLayout&&mViewPostLayout(*view,transformation,layoutOrder.mItemPositionDiff, mOrientation, layoutOrder.mItemAdapterPosition))){
	view->setTranslationX(transformation.mTranslationX);
	view->setTranslationY(transformation.mTranslationY);
        view->setScaleX(transformation.mScaleX);
        view->setScaleY(transformation.mScaleY);
    }
}

/**
 * @return current scroll position of center item. this value can be in any range if it is cycle layout.
 * if this is not, that then it is in [0, {@link #mItemsCount - 1}]
 */
float CarouselLayoutManager::getCurrentScrollPosition() const{
    int fullScrollSize = getMaxScrollOffset();
    if (0 == fullScrollSize) {
        return 0;
    }
    return 1.0f * mLayoutHelper->mScrollOffset / getScrollItemSize();
}

/**
 * @return maximum scroll value to fill up all items in layout. Generally this is only needed for non cycle layouts.
 */
int CarouselLayoutManager::getMaxScrollOffset() const{
    return getScrollItemSize() * (mItemsCount - 1);
}

/**
 * Because we can support old Android versions, we should layout our children in specific order to make our center view in the top of layout
 * (this item should layout last). So this method will calculate layout order and fill up {@link #mLayoutHelper} object.
 * This object will be filled by only needed to layout items. Non visible items will not be there.
 *
 * @param currentScrollPosition current scroll position this is a value that indicates position of center item
 *                              (if this value is int, then center item is really in the center of the layout, else it is near state).
 *                              Be aware that this value can be in any range is it is cycle layout
 * @param state                 Transient state of RecyclerView
 * @see #getCurrentScrollPosition()
 */
void CarouselLayoutManager::generateLayoutOrder(float currentScrollPosition, RecyclerView::State& state) {
    mItemsCount = state.getItemCount();
    const float absCurrentScrollPosition = makeScrollPositionInRange0ToCount(currentScrollPosition, mItemsCount);
    const int centerItem = std::round(absCurrentScrollPosition);

    if (mCircleLayout && 1 < mItemsCount) {
        const int layoutCount = std::min(mLayoutHelper->mMaxVisibleItems * 2 + 1, mItemsCount);

        mLayoutHelper->initLayoutOrder(layoutCount);

        int countLayoutHalf = layoutCount / 2;
        // before center item
        for (int i = 1; i <= countLayoutHalf; ++i) {
            int position = static_cast<int>(std::round(absCurrentScrollPosition - i + mItemsCount)) % mItemsCount;
            mLayoutHelper->setLayoutOrder(countLayoutHalf - i, position, centerItem - absCurrentScrollPosition - i);
        }
        // after center item
        for (int i = layoutCount - 1; i >= countLayoutHalf + 1; --i) {
            int position = static_cast<int>(std::round(absCurrentScrollPosition - i + layoutCount)) % mItemsCount;
            mLayoutHelper->setLayoutOrder(i - 1, position, centerItem - absCurrentScrollPosition + layoutCount - i);
        }
        mLayoutHelper->setLayoutOrder(layoutCount - 1, centerItem, centerItem - absCurrentScrollPosition);

    } else {
        const int firstVisible = std::max(centerItem - mLayoutHelper->mMaxVisibleItems, 0);
        const int lastVisible = std::min(centerItem + mLayoutHelper->mMaxVisibleItems, mItemsCount - 1);
        const int layoutCount = lastVisible - firstVisible + 1;

        mLayoutHelper->initLayoutOrder(layoutCount);

        for (int i = firstVisible; i <= lastVisible; ++i) {
            if (i == centerItem) {
                mLayoutHelper->setLayoutOrder(layoutCount - 1, i, i - absCurrentScrollPosition);
            } else if (i < centerItem) {
                mLayoutHelper->setLayoutOrder(i - firstVisible, i, i - absCurrentScrollPosition);
            } else {
                mLayoutHelper->setLayoutOrder(layoutCount - (i - centerItem) - 1, i, i - absCurrentScrollPosition);
            }
        }
    }
}

int CarouselLayoutManager::getWidthNoPadding() {
    return getWidth() - getPaddingStart() - getPaddingEnd();
}

int CarouselLayoutManager::getHeightNoPadding() {
    return getHeight() - getPaddingEnd() - getPaddingStart();
}

View* CarouselLayoutManager::bindChild(int position, RecyclerView::Recycler& recycler) {
    View* view = recycler.getViewForPosition(position);

    addView(view);
    measureChildWithMargins(view, 0, 0);

    return view;
}

void CarouselLayoutManager::recyclerOldViews(RecyclerView::Recycler& recycler) {
    auto scrapList = recycler.getScrapList();
    for (RecyclerView::ViewHolder* viewHolder : scrapList) {
        int adapterPosition = viewHolder->getAdapterPosition();
        bool found = false;
        for (std::shared_ptr<LayoutOrder> layoutOrder : mLayoutHelper->mLayoutOrder) {
            if (layoutOrder->mItemAdapterPosition == adapterPosition) {
                found = true;
                break;
            }
        }
        if (!found) {
            recycler.recycleView(viewHolder->itemView);
        }
    }
}

/**
 * Called during {@link #fillData(RecyclerView.Recycler, RecyclerView.State)} to calculate item offset from layout center line. <br />
 * <br />
 * Returns {@link #convertItemPositionDiffToSmoothPositionDiff(float)} * (size off area above center item when it is on the center). <br />
 * Sign is: plus if this item is bellow center line, minus if not<br />
 * <br />
 * ----- - area above it<br />
 * ||||| - center item<br />
 * ----- - area bellow it (it has the same size as are above center item)<br />
 *
 * @param itemPositionDiff current item difference with layout center line. if this is 0, then this item center is in layout center line.
 *                         if this is 1 then this item is bellow the layout center line in the full item size distance.
 * @return offset in scroll px coordinates.
 */
int CarouselLayoutManager::getCardOffsetByPositionDiff(float itemPositionDiff) {
    const double smoothPosition = convertItemPositionDiffToSmoothPositionDiff(itemPositionDiff);

    int dimenDiff;
    if (VERTICAL == mOrientation) {
        dimenDiff = (getHeightNoPadding() - mDecoratedChildHeight) / 2;
    } else {
        dimenDiff = (getWidthNoPadding() - mDecoratedChildWidth) / 2;
    }
    //noinspection NumericCastThatLosesPrecision
    return (int) std::round(MathUtils::signum(itemPositionDiff) * dimenDiff * smoothPosition);
}

/**
 * Called during {@link #getCardOffsetByPositionDiff(float)} for better item movement. <br/>
 * Current implementation speed up items that are far from layout center line and slow down items that are close to this line.
 * This code is full of maths. If you want to make items move in a different way, probably you should override this method.<br />
 * Please see code comments for better explanations.
 *
 * @param itemPositionDiff current item difference with layout center line. if this is 0, then this item center is in layout center line.
 *                         if this is 1 then this item is bellow the layout center line in the full item size distance.
 * @return smooth position offset. needed for scroll calculation and better user experience.
 * @see #getCardOffsetByPositionDiff(float)
 */
double CarouselLayoutManager::convertItemPositionDiffToSmoothPositionDiff(float itemPositionDiff) {
    // generally item moves the same way above center and bellow it. So we don't care about diff sign.
    const float absIemPositionDiff = std::abs(itemPositionDiff);

    // we detect if this item is close for center or not. We use (1 / maxVisibleItem) ^ (1/3) as close definer.
    if (absIemPositionDiff > std::pow(1.0f / mLayoutHelper->mMaxVisibleItems, 1.0f / 3.f)) {
        // this item is far from center line, so we should make it move like square root function
        return std::pow(absIemPositionDiff / mLayoutHelper->mMaxVisibleItems, 1.0f / 2.0f);
    } else {
        // this item is close from center line. we should slow it down and don't make it speed up very quick.
        // so square function in range of [0, (1/maxVisible)^(1/3)] is quite good in it;
        return std::pow(absIemPositionDiff, 2.0f);
    }
}

/**
 * @return full item size
 */
int CarouselLayoutManager::getScrollItemSize() const{
    if (VERTICAL == mOrientation) {
        return mDecoratedChildHeight;
    } else {
        return mDecoratedChildWidth;
    }
}

Parcelable* CarouselLayoutManager::onSaveInstanceState() {
    if (mPendingCarouselSavedState) {
        return new CarouselSavedState(*mPendingCarouselSavedState);
    }
    CarouselSavedState* savedState = new CarouselSavedState(LayoutManager::onSaveInstanceState());
    savedState->mCenterItemPosition = mCenterItemPosition;
    return savedState;
}

void CarouselLayoutManager::onRestoreInstanceState(Parcelable& state) {
    if (dynamic_cast<CarouselSavedState*>(&state)) {
        mPendingCarouselSavedState = (CarouselSavedState*) &state;
	LayoutManager::onRestoreInstanceState(*mPendingCarouselSavedState->mSuperState);
    } else {
	LayoutManager::onRestoreInstanceState(state);
    }
}

/**
 * @return Scroll offset from nearest item from center
 */
int CarouselLayoutManager::getOffsetCenterView() {
    return std::round(getCurrentScrollPosition()) * getScrollItemSize() - mLayoutHelper->mScrollOffset;
}

int CarouselLayoutManager::getOffsetForCurrentView(View& view) {
    int targetPosition = getPosition(&view);
    float directionDistance = getScrollDirection(targetPosition);

    return std::round(directionDistance * getScrollItemSize());
}

/**
 * Helper method that make scroll in range of [0, count). Generally this method is needed only for cycle layout.
 *
 * @param currentScrollPosition any scroll position range.
 * @param count                 adapter items count
 * @return good scroll position in range of [0, count)
 */
float CarouselLayoutManager::makeScrollPositionInRange0ToCount(float currentScrollPosition, int count){
    float absCurrentScrollPosition = currentScrollPosition;
    while (0 > absCurrentScrollPosition) {
        absCurrentScrollPosition += count;
    }
    while (std::round(absCurrentScrollPosition) >= count) {
        absCurrentScrollPosition -= count;
    }
    return absCurrentScrollPosition;
}

CarouselLayoutManager::LayoutHelper::LayoutHelper(int maxVisibleItems) {
    mMaxVisibleItems = maxVisibleItems;
}

/**
 * Called before any fill calls. Needed to recycle old items and init new array list. Generally this list is an array an it is reused.
 *
 * @param layoutCount items count that will be layout
 */
void CarouselLayoutManager::LayoutHelper::initLayoutOrder(int layoutCount) {
    if (mLayoutOrder.size() != layoutCount) {
        if (mLayoutOrder.size()) {
            recycleItems(mLayoutOrder);
        }
        mLayoutOrder.resize(layoutCount);//mLayoutOrder= new LayoutOrder[layoutCount];
        fillLayoutOrder();
    }
}

void CarouselLayoutManager::LayoutHelper::setLayoutOrder(int arrayPosition, int itemAdapterPosition, float itemPositionDiff) {
    std::shared_ptr<LayoutOrder> item = mLayoutOrder[arrayPosition];
    item->mItemAdapterPosition = itemAdapterPosition;
    item->mItemPositionDiff = itemPositionDiff;
}

/**
 * Checks is this screen Layout has this adapterPosition view in layout
 *
 * @param adapterPosition adapter position of item for future data filling logic
 * @return true is adapterItem is in layout
 */
bool CarouselLayoutManager::LayoutHelper::hasAdapterPosition(int adapterPosition) {
    if (mLayoutOrder.size()) {
        for (std::shared_ptr<LayoutOrder> layoutOrder : mLayoutOrder) {
            if (layoutOrder->mItemAdapterPosition == adapterPosition) {
                return true;
            }
        }
    }
    return false;
}

void CarouselLayoutManager::LayoutHelper::recycleItems(const std::vector<std::shared_ptr<LayoutOrder>>&layoutOrders) {
    for (std::shared_ptr<LayoutOrder> layoutOrder : layoutOrders) {
        //noinspection ObjectAllocationInLoop
        mReusedItems.push_back(std::weak_ptr<LayoutOrder>(layoutOrder));//add(new WeakReference<>(layoutOrder));
    }
}

void CarouselLayoutManager::LayoutHelper::fillLayoutOrder() {
    for (int i = 0, length = mLayoutOrder.size(); i < length; ++i) {
        if (nullptr == mLayoutOrder[i]) {
            mLayoutOrder[i] = createLayoutOrder();
        }
    }
}

std::shared_ptr<CarouselLayoutManager::LayoutOrder> CarouselLayoutManager::LayoutHelper::createLayoutOrder() {
    for(auto it=mReusedItems.begin();it!=mReusedItems.end();it++){
        std::weak_ptr<LayoutOrder>layoutOrder = *it;
        it = mReusedItems.erase(it);
        if(!layoutOrder.expired()){
	        return layoutOrder.lock();//it = mReusedItems.erase(it);
        }
    }
    return std::make_shared<LayoutOrder>();
}

CarouselLayoutManager::CarouselSavedState::CarouselSavedState(Parcelable* superState) {
    //mSuperState = superState;
}

CarouselLayoutManager::CarouselSavedState::CarouselSavedState(Parcel& in) {
    //mSuperState = in.readParcelable(Parcelable.class.getClassLoader());
    mCenterItemPosition = in.readInt();
}

CarouselLayoutManager::CarouselSavedState::CarouselSavedState(CarouselSavedState& other) {
    //mSuperState = other.mSuperState;
    mCenterItemPosition = other.mCenterItemPosition;
}

int CarouselLayoutManager::CarouselSavedState::describeContents() {
    return 0;
}

void CarouselLayoutManager::CarouselSavedState::writeToParcel(Parcel& parcel, int i) {
    //parcel.writeParcelable(mSuperState, i);
    parcel.writeInt(mCenterItemPosition);
}
}/*endof namespace*/
