#include <widgetEx/recyclerview/gridlayoutmanager.h>
#include <widgetEx/recyclerview/orientationhelper.h>

namespace cdroid{

GridLayoutManager::GridLayoutManager(Context* context,const AttributeSet& attrs)
   :LinearLayoutManager(context, attrs){
    Properties* properties = getProperties(context, attrs,0,0);// defStyleAttr, defStyleRes);
    mSpanSizeLookup = new DefaultSpanSizeLookup();
    setSpanCount(properties->spanCount);
    delete properties;
    mPendingSpanCountChange = false;
}

GridLayoutManager::GridLayoutManager(Context* context, int spanCount)
    :LinearLayoutManager(context){
    mSpanSizeLookup = new DefaultSpanSizeLookup();
    setSpanCount(spanCount);
}

GridLayoutManager::GridLayoutManager(Context* context, int spanCount,int orientation, bool reverseLayout)
    :LinearLayoutManager(context, orientation, reverseLayout){
    mSpanSizeLookup = new DefaultSpanSizeLookup();
    setSpanCount(spanCount);
}

GridLayoutManager::~GridLayoutManager(){
    delete mSpanSizeLookup;
}

void GridLayoutManager::setStackFromEnd(bool stackFromEnd) {
    LOGE_IF(stackFromEnd,"GridLayoutManager does not support stack from end. Consider using reverse layout");
    LinearLayoutManager::setStackFromEnd(false);
}

int GridLayoutManager::getRowCountForAccessibility(RecyclerView::Recycler& recycler,
        RecyclerView::State& state) {
    if (mOrientation == HORIZONTAL) {
        return mSpanCount;
    }
    if (state.getItemCount() < 1) {
        return 0;
    }

    // Row count is one more than the last item's row index.
    return getSpanGroupIndex(recycler, state, state.getItemCount() - 1) + 1;
}

int GridLayoutManager::getColumnCountForAccessibility(RecyclerView::Recycler& recycler,
        RecyclerView::State& state) {
    if (mOrientation == VERTICAL) {
        return mSpanCount;
    }
    if (state.getItemCount() < 1) {
        return 0;
    }

    // Column count is one more than the last item's column index.
    return getSpanGroupIndex(recycler, state, state.getItemCount() - 1) + 1;
}
#if 0
void GridLayoutManager::onInitializeAccessibilityNodeInfoForItem(RecyclerView::Recycler& recycler,
        RecyclerView::State& state, View* host, AccessibilityNodeInfoCompat info) {
    ViewGroup.LayoutParams lp = host.getLayoutParams();
    if (!(lp instanceof LayoutParams)) {
        super.onInitializeAccessibilityNodeInfoForItem(host, info);
        return;
    }
    LayoutParams glp = (LayoutParams) lp;
    int spanGroupIndex = getSpanGroupIndex(recycler, state, glp.getViewLayoutPosition());
    if (mOrientation == HORIZONTAL) {
        info.setCollectionItemInfo(AccessibilityNodeInfoCompat.CollectionItemInfoCompat.obtain(
                glp.getSpanIndex(), glp.getSpanSize(),
                spanGroupIndex, 1,
                mSpanCount > 1 && glp.getSpanSize() == mSpanCount, false));
    } else { // VERTICAL
        info.setCollectionItemInfo(AccessibilityNodeInfoCompat.CollectionItemInfoCompat.obtain(
                spanGroupIndex , 1,
                glp.getSpanIndex(), glp.getSpanSize(),
                mSpanCount > 1 && glp.getSpanSize() == mSpanCount, false));
    }
}
#endif
void GridLayoutManager::onLayoutChildren(RecyclerView::Recycler& recycler, RecyclerView::State& state) {
    if (state.isPreLayout()) {
        cachePreLayoutSpanMapping();
    }
    LinearLayoutManager::onLayoutChildren(recycler, state);
    if (_DEBUG) {
        validateChildOrder();
    }
    clearPreLayoutSpanMappingCache();
}

void GridLayoutManager::onLayoutCompleted(RecyclerView::State& state) {
    LinearLayoutManager::onLayoutCompleted(state);
    mPendingSpanCountChange = false;
}

void GridLayoutManager::clearPreLayoutSpanMappingCache() {
    mPreLayoutSpanSizeCache.clear();
    mPreLayoutSpanIndexCache.clear();
}

void GridLayoutManager::cachePreLayoutSpanMapping() {
    const int childCount = getChildCount();
    for (int i = 0; i < childCount; i++) {
        LayoutParams* lp = (LayoutParams*) getChildAt(i)->getLayoutParams();
        const int viewPosition = lp->getViewLayoutPosition();
        mPreLayoutSpanSizeCache.put(viewPosition, lp->getSpanSize());
        mPreLayoutSpanIndexCache.put(viewPosition, lp->getSpanIndex());
    }
}

void GridLayoutManager::onItemsAdded(RecyclerView& recyclerView, int positionStart, int itemCount) {
    mSpanSizeLookup->invalidateSpanIndexCache();
}

void GridLayoutManager::onItemsChanged(RecyclerView& recyclerView) {
    mSpanSizeLookup->invalidateSpanIndexCache();
}

void GridLayoutManager::onItemsRemoved(RecyclerView& recyclerView, int positionStart, int itemCount) {
    mSpanSizeLookup->invalidateSpanIndexCache();
}

void GridLayoutManager::onItemsUpdated(RecyclerView& recyclerView, int positionStart, int itemCount,
        Object* payload) {
    mSpanSizeLookup->invalidateSpanIndexCache();
}

void GridLayoutManager::onItemsMoved(RecyclerView& recyclerView, int from, int to, int itemCount) {
    mSpanSizeLookup->invalidateSpanIndexCache();
}

RecyclerView::LayoutParams* GridLayoutManager::generateDefaultLayoutParams()const {
    if (mOrientation == HORIZONTAL) {
        return new LayoutParams(LayoutParams::WRAP_CONTENT,LayoutParams::MATCH_PARENT);
    } else {
        return new LayoutParams(LayoutParams::MATCH_PARENT,LayoutParams::WRAP_CONTENT);
    }
}

RecyclerView::LayoutParams* GridLayoutManager::generateLayoutParams(Context* c,const AttributeSet& attrs)const{
    return new LayoutParams(c, attrs);
}

RecyclerView::LayoutParams* GridLayoutManager::generateLayoutParams(const ViewGroup::LayoutParams& lp)const{
    if (dynamic_cast<const ViewGroup::MarginLayoutParams*>(&lp)) {
        return new LayoutParams((ViewGroup::MarginLayoutParams&) lp);
    } else {
        return new LayoutParams(lp);
    }
}

bool GridLayoutManager::checkLayoutParams(const RecyclerView::LayoutParams* lp)const {
    return dynamic_cast<const LayoutParams*>(lp);
}

void GridLayoutManager::setSpanSizeLookup(SpanSizeLookup* spanSizeLookup) {
    mSpanSizeLookup = spanSizeLookup;
}

GridLayoutManager::SpanSizeLookup* GridLayoutManager::getSpanSizeLookup() {
    return mSpanSizeLookup;
}

void GridLayoutManager::updateMeasurements() {
    int totalSpace;
    if (getOrientation() == VERTICAL) {
        totalSpace = getWidth() - getPaddingRight() - getPaddingLeft();
    } else {
        totalSpace = getHeight() - getPaddingBottom() - getPaddingTop();
    }
    calculateItemBorders(totalSpace);
}

void GridLayoutManager::setMeasuredDimension(Rect& childrenBounds, int wSpec, int hSpec) {
    if (mCachedBorders.empty()){// == null) {
	LinearLayoutManager::setMeasuredDimension(childrenBounds, wSpec, hSpec);
    }
    int width, height;
    const int horizontalPadding = getPaddingLeft() + getPaddingRight();
    const int verticalPadding = getPaddingTop() + getPaddingBottom();
    if (mOrientation == VERTICAL) {
        const int usedHeight = childrenBounds.height + verticalPadding;
        height = chooseSize(hSpec, usedHeight, getMinimumHeight());
        width = chooseSize(wSpec, mCachedBorders[mCachedBorders.size() - 1] + horizontalPadding,
                getMinimumWidth());
    } else {
        const int usedWidth = childrenBounds.width + horizontalPadding;
        width = chooseSize(wSpec, usedWidth, getMinimumWidth());
        height = chooseSize(hSpec, mCachedBorders[mCachedBorders.size() - 1] + verticalPadding,
                getMinimumHeight());
    }
    LinearLayoutManager::setMeasuredDimension(width, height);
}

void GridLayoutManager::calculateItemBorders(int totalSpace) {
    //mCachedBorders = calculateItemBorders(mCachedBorders, mSpanCount, totalSpace);
    calculateItemBorders(mCachedBorders, mSpanCount, totalSpace);
}

int* GridLayoutManager::calculateItemBorders(std::vector<int>& cachedBorders, int spanCount, int totalSpace) {
    if (cachedBorders.empty() || cachedBorders.size() != spanCount + 1
            || cachedBorders[cachedBorders.size() - 1] != totalSpace) {
        cachedBorders.resize(spanCount +1);// = new int[spanCount + 1];
    }
    cachedBorders[0] = 0;
    int sizePerSpan = totalSpace / spanCount;
    int sizePerSpanRemainder = totalSpace % spanCount;
    int consumedPixels = 0;
    int additionalSize = 0;
    for (int i = 1; i <= spanCount; i++) {
        int itemSize = sizePerSpan;
        additionalSize += sizePerSpanRemainder;
        if (additionalSize > 0 && (spanCount - additionalSize) < sizePerSpanRemainder) {
            itemSize += 1;
            additionalSize -= spanCount;
        }
        consumedPixels += itemSize;
        cachedBorders[i] = consumedPixels;
    }
    return cachedBorders.data();
}

int GridLayoutManager::getSpaceForSpanRange(int startSpan, int spanSize) {
    if (mOrientation == VERTICAL && isLayoutRTL()) {
        return mCachedBorders[mSpanCount - startSpan]
                - mCachedBorders[mSpanCount - startSpan - spanSize];
    } else {
        return mCachedBorders[startSpan + spanSize] - mCachedBorders[startSpan];
    }
}

void GridLayoutManager::onAnchorReady(RecyclerView::Recycler& recycler, RecyclerView::State& state,
                   AnchorInfo& anchorInfo, int itemDirection) {
    LinearLayoutManager::onAnchorReady(recycler, state, anchorInfo, itemDirection);
    updateMeasurements();
    if (state.getItemCount() > 0 && !state.isPreLayout()) {
        ensureAnchorIsInCorrectSpan(recycler, state, anchorInfo, itemDirection);
    }
    ensureViewSet();
}

void GridLayoutManager::ensureViewSet() {
    if (mSet.empty() || mSet.size() != mSpanCount) {
        mSet.resize(mSpanCount);// = new View[mSpanCount];
    }
}

int GridLayoutManager::scrollHorizontallyBy(int dx, RecyclerView::Recycler& recycler,
        RecyclerView::State& state) {
    updateMeasurements();
    ensureViewSet();
    return LinearLayoutManager::scrollHorizontallyBy(dx, recycler, state);
}

int GridLayoutManager::scrollVerticallyBy(int dy, RecyclerView::Recycler& recycler,
        RecyclerView::State& state) {
    updateMeasurements();
    ensureViewSet();
    return LinearLayoutManager::scrollVerticallyBy(dy, recycler, state);
}

void GridLayoutManager::ensureAnchorIsInCorrectSpan(RecyclerView::Recycler& recycler,
        RecyclerView::State& state, AnchorInfo& anchorInfo, int itemDirection) {
    const bool layingOutInPrimaryDirection =
            itemDirection == LayoutState::ITEM_DIRECTION_TAIL;
    int span = getSpanIndex(recycler, state, anchorInfo.mPosition);
    if (layingOutInPrimaryDirection) {
        // choose span 0
        while (span > 0 && anchorInfo.mPosition > 0) {
            anchorInfo.mPosition--;
            span = getSpanIndex(recycler, state, anchorInfo.mPosition);
        }
    } else {
        // choose the max span we can get. hopefully last one
        const int indexLimit = state.getItemCount() - 1;
        int pos = anchorInfo.mPosition;
        int bestSpan = span;
        while (pos < indexLimit) {
            int next = getSpanIndex(recycler, state, pos + 1);
            if (next > bestSpan) {
                pos += 1;
                bestSpan = next;
            } else {
                break;
            }
        }
        anchorInfo.mPosition = pos;
    }
}

View* GridLayoutManager::findReferenceChild(RecyclerView::Recycler& recycler, RecyclerView::State& state,
                        int start, int end, int itemCount) {
    ensureLayoutState();
    View* invalidMatch = nullptr;
    View* outOfBoundsMatch = nullptr;
    const int boundsStart = mOrientationHelper->getStartAfterPadding();
    const int boundsEnd = mOrientationHelper->getEndAfterPadding();
    const int diff = end > start ? 1 : -1;

    for (int i = start; i != end; i += diff) {
        View* view = getChildAt(i);
        const int position = getPosition(view);
        if (position >= 0 && position < itemCount) {
            const int span = getSpanIndex(recycler, state, position);
            if (span != 0) {
                continue;
            }
            if (((RecyclerView::LayoutParams*) view->getLayoutParams())->isItemRemoved()) {
                if (invalidMatch == nullptr) {
                    invalidMatch = view; // removed item, least preferred
                }
            } else if (mOrientationHelper->getDecoratedStart(view) >= boundsEnd
                    || mOrientationHelper->getDecoratedEnd(view) < boundsStart) {
                if (outOfBoundsMatch == nullptr) {
                    outOfBoundsMatch = view; // item is not visible, less preferred
                }
            } else {
                return view;
            }
        }
    }
    return outOfBoundsMatch != nullptr ? outOfBoundsMatch : invalidMatch;
}

int GridLayoutManager::getSpanGroupIndex(RecyclerView::Recycler& recycler, RecyclerView::State& state,
        int viewPosition) {
    if (!state.isPreLayout()) {
        return mSpanSizeLookup->getSpanGroupIndex(viewPosition, mSpanCount);
    }
    const int adapterPosition = recycler.convertPreLayoutPositionToPostLayout(viewPosition);
    if (adapterPosition == -1) {
        if (_DEBUG) {
            LOGD("Cannot find span group index for position %d",viewPosition);
        }
        LOGW("Cannot find span size for pre layout position. ",viewPosition);
        return 0;
    }
    return mSpanSizeLookup->getSpanGroupIndex(adapterPosition, mSpanCount);
}

int GridLayoutManager::getSpanIndex(RecyclerView::Recycler& recycler, RecyclerView::State& state, int pos) {
    if (!state.isPreLayout()) {
        return mSpanSizeLookup->getCachedSpanIndex(pos, mSpanCount);
    }
    const int cached = mPreLayoutSpanIndexCache.get(pos, -1);
    if (cached != -1) {
        return cached;
    }
    const int adapterPosition = recycler.convertPreLayoutPositionToPostLayout(pos);
    if (adapterPosition == -1) {
        if (_DEBUG) {
            LOGD("Cannot find span index for pre layout position. It is"
                   " not cached, not in the adapter. Pos:%d",pos);
        }
        LOGW("Cannot find span size for pre layout position. It is"
               " not cached, not in the adapter. Pos:%d",pos);
        return 0;
    }
    return mSpanSizeLookup->getCachedSpanIndex(adapterPosition, mSpanCount);
}

int GridLayoutManager::getSpanSize(RecyclerView::Recycler& recycler, RecyclerView::State& state, int pos) {
    if (!state.isPreLayout()) {
        return mSpanSizeLookup->getSpanSize(pos);
    }
    const int cached = mPreLayoutSpanSizeCache.get(pos, -1);
    if (cached != -1) {
        return cached;
    }
    const int adapterPosition = recycler.convertPreLayoutPositionToPostLayout(pos);
    if (adapterPosition == -1) {
        if (_DEBUG) {
            LOGD("Cannot find span size for pre layout position. It is"
                 " not cached, not in the adapter. Pos:%d",pos);
        }
        LOGW("Cannot find span size for pre layout position. It is"
               " not cached, not in the adapter. Pos:",pos);
        return 1;
    }
    return mSpanSizeLookup->getSpanSize(adapterPosition);
}

void GridLayoutManager::collectPrefetchPositionsForLayoutState(RecyclerView::State& state, LayoutState& layoutState,
        LayoutPrefetchRegistry& layoutPrefetchRegistry) {
    int remainingSpan = mSpanCount;
    int count = 0;
    while (count < mSpanCount && layoutState.hasMore(state) && remainingSpan > 0) {
        const int pos = layoutState.mCurrentPosition;
        layoutPrefetchRegistry(pos, std::max(0, layoutState.mScrollingOffset));//addPosition
        const int spanSize = mSpanSizeLookup->getSpanSize(pos);
        remainingSpan -= spanSize;
        layoutState.mCurrentPosition += layoutState.mItemDirection;
        count++;
    }
}

void GridLayoutManager::layoutChunk(RecyclerView::Recycler& recycler, RecyclerView::State& state,
        LayoutState& layoutState, LayoutChunkResult& result) {
    const int otherDirSpecMode = mOrientationHelper->getModeInOther();
    const bool flexibleInOtherDir = otherDirSpecMode != MeasureSpec::EXACTLY;
    const int currentOtherDirSize = getChildCount() > 0 ? mCachedBorders[mSpanCount] : 0;
    // if grid layout's dimensions are not specified, let the new row change the measurements
    // This is not perfect since we not covering all rows but still solves an important case
    // where they may have a header row which should be laid out according to children.
    if (flexibleInOtherDir) {
        updateMeasurements(); //  reset measurements
    }
    const bool layingOutInPrimaryDirection =
            layoutState.mItemDirection == LayoutState::ITEM_DIRECTION_TAIL;
    int count = 0;
    int consumedSpanCount = 0;
    int remainingSpan = mSpanCount;
    if (!layingOutInPrimaryDirection) {
        int itemSpanIndex = getSpanIndex(recycler, state, layoutState.mCurrentPosition);
        int itemSpanSize = getSpanSize(recycler, state, layoutState.mCurrentPosition);
        remainingSpan = itemSpanIndex + itemSpanSize;
    }
    while (count < mSpanCount && layoutState.hasMore(state) && remainingSpan > 0) {
        int pos = layoutState.mCurrentPosition;
        const int spanSize = getSpanSize(recycler, state, pos);
        if (spanSize > mSpanCount) {
            FATAL("Item at position %d requires %d spans but GridLayoutManager has only %d spans.",pos,spanSize,mSpanCount);
        }
        remainingSpan -= spanSize;
        if (remainingSpan < 0) {
            break; // item did not fit into this row or column
        }
        View* view = layoutState.next(recycler);
        if (view == nullptr) {
            break;
        }
        consumedSpanCount += spanSize;
        mSet[count] = view;
        count++;
    }

    if (count == 0) {
        result.mFinished = true;
        return;
    }

    int maxSize = 0;
    float maxSizeInOther = 0; // use a float to get size per span

    // we should assign spans before item decor offsets are calculated
    assignSpans(recycler, state, count, consumedSpanCount, layingOutInPrimaryDirection);
    for (int i = 0; i < count; i++) {
        View* view = mSet[i];
        if (layoutState.mScrapList.empty()){// == null) {
            if (layingOutInPrimaryDirection) {
                addView(view);
            } else {
                addView(view, 0);
            }
        } else {
            if (layingOutInPrimaryDirection) {
                addDisappearingView(view);
            } else {
                addDisappearingView(view, 0);
            }
        }
        calculateItemDecorationsForChild(view, mDecorInsets);

        measureChild(view, otherDirSpecMode, false);
        const int size = mOrientationHelper->getDecoratedMeasurement(view);
        if (size > maxSize) {
            maxSize = size;
        }
        const LayoutParams* lp = (LayoutParams*) view->getLayoutParams();
        const float otherSize = 1.f * mOrientationHelper->getDecoratedMeasurementInOther(view)
                / lp->mSpanSize;
        if (otherSize > maxSizeInOther) {
            maxSizeInOther = otherSize;
        }
    }
    if (flexibleInOtherDir) {
        // re-distribute columns
        guessMeasurement(maxSizeInOther, currentOtherDirSize);
        // now we should re-measure any item that was match parent.
        maxSize = 0;
        for (int i = 0; i < count; i++) {
            View* view = mSet[i];
            measureChild(view, MeasureSpec::EXACTLY, true);
            const int size = mOrientationHelper->getDecoratedMeasurement(view);
            if (size > maxSize) {
                maxSize = size;
            }
        }
    }

    // Views that did not measure the maxSize has to be re-measured
    // We will stop doing this once we introduce Gravity in the GLM layout params
    for (int i = 0; i < count; i++) {
        View* view = mSet[i];
        if (mOrientationHelper->getDecoratedMeasurement(view) != maxSize) {
            LayoutParams* lp = (LayoutParams*) view->getLayoutParams();
            Rect decorInsets = lp->mDecorInsets;
            const int verticalInsets = decorInsets.top + decorInsets.height;//bottom
                    + lp->topMargin + lp->bottomMargin;
            const int horizontalInsets = decorInsets.left + decorInsets.width;//right
                    + lp->leftMargin + lp->rightMargin;
            const int totalSpaceInOther = getSpaceForSpanRange(lp->mSpanIndex, lp->mSpanSize);
            int wSpec, hSpec;
            if (mOrientation == VERTICAL) {
                wSpec = getChildMeasureSpec(totalSpaceInOther, MeasureSpec::EXACTLY,
                        horizontalInsets, lp->width, false);
                hSpec = MeasureSpec::makeMeasureSpec(maxSize - verticalInsets,
                        MeasureSpec::EXACTLY);
            } else {
                wSpec = MeasureSpec::makeMeasureSpec(maxSize - horizontalInsets,
                        MeasureSpec::EXACTLY);
                hSpec = getChildMeasureSpec(totalSpaceInOther, MeasureSpec::EXACTLY,
                        verticalInsets, lp->height, false);
            }
            measureChildWithDecorationsAndMargin(view, wSpec, hSpec, true);
        }
    }

    result.mConsumed = maxSize;

    int left = 0, right = 0, top = 0, bottom = 0;
    if (mOrientation == VERTICAL) {
        if (layoutState.mLayoutDirection == LayoutState::LAYOUT_START) {
            bottom = layoutState.mOffset;
            top = bottom - maxSize;
        } else {
            top = layoutState.mOffset;
            bottom = top + maxSize;
        }
    } else {
        if (layoutState.mLayoutDirection == LayoutState::LAYOUT_START) {
            right = layoutState.mOffset;
            left = right - maxSize;
        } else {
            left = layoutState.mOffset;
            right = left + maxSize;
        }
    }
    for (int i = 0; i < count; i++) {
        View* view = mSet[i];
        LayoutParams* params = (LayoutParams*) view->getLayoutParams();
        if (mOrientation == VERTICAL) {
            if (isLayoutRTL()) {
                right = getPaddingLeft() + mCachedBorders[mSpanCount - params->mSpanIndex];
                left = right - mOrientationHelper->getDecoratedMeasurementInOther(view);
            } else {
                left = getPaddingLeft() + mCachedBorders[params->mSpanIndex];
                right = left + mOrientationHelper->getDecoratedMeasurementInOther(view);
            }
        } else {
            top = getPaddingTop() + mCachedBorders[params->mSpanIndex];
            bottom = top + mOrientationHelper->getDecoratedMeasurementInOther(view);
        }
        // We calculate everything with View's bounding box (which includes decor and margins)
        // To calculate correct layout position, we subtract margins.
        layoutDecoratedWithMargins(view, left, top, right-left, bottom-top);
        LOGD_IF(_DEBUG,"laid out child at position %d,with*%d,%d,%d,%d) span:%d, spanSize:%d",
	    getPosition(view),(left + params->leftMargin),(top + params->topMargin),(right - params->rightMargin),
	    (bottom - params->bottomMargin), params->mSpanIndex , params->mSpanSize);
        // Consume the available space if the view is not removed OR changed
        if (params->isItemRemoved() || params->isItemChanged()) {
            result.mIgnoreConsumed = true;
        }
        result.mFocusable |= view->hasFocusable();
    }
    for(auto& m:mSet)m=nullptr;//Arrays.fill(mSet, null);
}

void GridLayoutManager::measureChild(View* view, int otherDirParentSpecMode, bool alreadyMeasured) {
    const LayoutParams* lp = (const LayoutParams*) view->getLayoutParams();
    const Rect& decorInsets = lp->mDecorInsets;
    const int verticalInsets = decorInsets.top + decorInsets.height;//bottom
            + lp->topMargin + lp->bottomMargin;
    const int horizontalInsets = decorInsets.left + decorInsets.width;//right
            + lp->leftMargin + lp->rightMargin;
    const int availableSpaceInOther = getSpaceForSpanRange(lp->mSpanIndex, lp->mSpanSize);
    int wSpec, hSpec;
    if (mOrientation == VERTICAL) {
        wSpec = getChildMeasureSpec(availableSpaceInOther, otherDirParentSpecMode,
                horizontalInsets, lp->width, false);
        hSpec = getChildMeasureSpec(mOrientationHelper->getTotalSpace(), getHeightMode(),
                verticalInsets, lp->height, true);
    } else {
        hSpec = getChildMeasureSpec(availableSpaceInOther, otherDirParentSpecMode,
                verticalInsets, lp->height, false);
        wSpec = getChildMeasureSpec(mOrientationHelper->getTotalSpace(), getWidthMode(),
                horizontalInsets, lp->width, true);
    }
    measureChildWithDecorationsAndMargin(view, wSpec, hSpec, alreadyMeasured);
}

void GridLayoutManager::guessMeasurement(float maxSizeInOther, int currentOtherDirSize) {
    const int contentSize = std::round(maxSizeInOther * mSpanCount);
    // always re-calculate because borders were stretched during the fill
    calculateItemBorders(std::max(contentSize, currentOtherDirSize));
}

void GridLayoutManager::measureChildWithDecorationsAndMargin(View* child, int widthSpec, int heightSpec,
        bool alreadyMeasured) {
    RecyclerView::LayoutParams* lp = (RecyclerView::LayoutParams*) child->getLayoutParams();
    bool measure;
    if (alreadyMeasured) {
        measure = shouldReMeasureChild(child, widthSpec, heightSpec, lp);
    } else {
        measure = shouldMeasureChild(child, widthSpec, heightSpec, lp);
    }
    if (measure) {
        child->measure(widthSpec, heightSpec);
    }
}

void GridLayoutManager::assignSpans(RecyclerView::Recycler& recycler, RecyclerView::State& state, int count,
        int consumedSpanCount, bool layingOutInPrimaryDirection) {
    // spans are always assigned from 0 to N no matter if it is RTL or not.
    // RTL is used only when positioning the view.
    int span, start, end, diff;
    // make sure we traverse from min position to max position
    if (layingOutInPrimaryDirection) {
        start = 0;
        end = count;
        diff = 1;
    } else {
        start = count - 1;
        end = -1;
        diff = -1;
    }
    span = 0;
    for (int i = start; i != end; i += diff) {
        View* view = mSet[i];
        LayoutParams* params = (LayoutParams*) view->getLayoutParams();
        params->mSpanSize = getSpanSize(recycler, state, getPosition(view));
        params->mSpanIndex = span;
        span += params->mSpanSize;
    }
}

int GridLayoutManager::getSpanCount() {
    return mSpanCount;
}

void GridLayoutManager::setSpanCount(int spanCount) {
    if (spanCount == mSpanCount) {
        return;
    }
    mPendingSpanCountChange = true;
    if (spanCount < 1) {
        FATAL("Span count should be at least 1. Provided %d",spanCount);
    }
    mSpanCount = spanCount;
    mSpanSizeLookup->invalidateSpanIndexCache();
    requestLayout();
}


View* GridLayoutManager::onFocusSearchFailed(View* focused, int focusDirection,
            RecyclerView::Recycler& recycler, RecyclerView::State& state) {
    View* prevFocusedChild = findContainingItemView(focused);
    if (prevFocusedChild == nullptr) {
        return nullptr;
    }
    LayoutParams* lp = (LayoutParams*) prevFocusedChild->getLayoutParams();
    const int prevSpanStart = lp->mSpanIndex;
    const int prevSpanEnd = lp->mSpanIndex + lp->mSpanSize;
    View* view = LinearLayoutManager::onFocusSearchFailed(focused, focusDirection, recycler, state);
    if (view == nullptr) {
        return nullptr;
    }
    // LinearLayoutManager finds the last child. What we want is the child which has the same
    // spanIndex.
    const int layoutDir = convertFocusDirectionToLayoutDirection(focusDirection);
    const bool ascend = (layoutDir == LayoutState::LAYOUT_END) != mShouldReverseLayout;
    int start, inc, limit;
    if (ascend) {
        start = getChildCount() - 1;
        inc = -1;
        limit = -1;
    } else {
        start = 0;
        inc = 1;
        limit = getChildCount();
    }
    const bool preferLastSpan = mOrientation == VERTICAL && isLayoutRTL();

    // The focusable candidate to be picked if no perfect focusable candidate is found.
    // The best focusable candidate is the one with the highest amount of span overlap with
    // the currently focused view.
    View* focusableWeakCandidate = nullptr; // somewhat matches but not strong
    int focusableWeakCandidateSpanIndex = -1;
    int focusableWeakCandidateOverlap = 0; // how many spans overlap

    // The unfocusable candidate to become visible on the screen next, if no perfect or
    // weak focusable candidates are found to receive focus next.
    // We are only interested in partially visible unfocusable views. These are views that are
    // not fully visible, that is either partially overlapping, or out-of-bounds and right below
    // or above RV's padded bounded area. The best unfocusable candidate is the one with the
    // highest amount of span overlap with the currently focused view.
    View* unfocusableWeakCandidate = nullptr; // somewhat matches but not strong
    int unfocusableWeakCandidateSpanIndex = -1;
    int unfocusableWeakCandidateOverlap = 0; // how many spans overlap

    // The span group index of the start child. This indicates the span group index of the
    // next focusable item to receive focus, if a focusable item within the same span group
    // exists. Any focusable item beyond this group index are not relevant since they
    // were already stored in the layout before onFocusSearchFailed call and were not picked
    // by the focusSearch algorithm.
    int focusableSpanGroupIndex = getSpanGroupIndex(recycler, state, start);
    for (int i = start; i != limit; i += inc) {
        int spanGroupIndex = getSpanGroupIndex(recycler, state, i);
        View* candidate = getChildAt(i);
        if (candidate == prevFocusedChild) {
            break;
        }

        if (candidate->hasFocusable() && spanGroupIndex != focusableSpanGroupIndex) {
            // We are past the allowable span group index for the next focusable item.
            // The search only continues if no focusable weak candidates have been found up
            // until this point, in order to find the best unfocusable candidate to become
            // visible on the screen next.
            if (focusableWeakCandidate != nullptr) {
                break;
            }
            continue;
        }

        const LayoutParams* candidateLp = (LayoutParams*) candidate->getLayoutParams();
        const int candidateStart = candidateLp->mSpanIndex;
        const int candidateEnd = candidateLp->mSpanIndex + candidateLp->mSpanSize;
        if (candidate->hasFocusable() && candidateStart == prevSpanStart
                && candidateEnd == prevSpanEnd) {
            return candidate; // perfect match
        }
        bool assignAsWeek = false;
        if ((candidate->hasFocusable() && focusableWeakCandidate == nullptr)
                || (!candidate->hasFocusable() && unfocusableWeakCandidate == nullptr)) {
            assignAsWeek = true;
        } else {
            int maxStart = std::max(candidateStart, prevSpanStart);
            int minEnd = std::min(candidateEnd, prevSpanEnd);
            int overlap = minEnd - maxStart;
            if (candidate->hasFocusable()) {
                if (overlap > focusableWeakCandidateOverlap) {
                    assignAsWeek = true;
                } else if (overlap == focusableWeakCandidateOverlap
                        && preferLastSpan == (candidateStart
                        > focusableWeakCandidateSpanIndex)) {
                    assignAsWeek = true;
                }
            } else if (focusableWeakCandidate == nullptr
                    && isViewPartiallyVisible(candidate, false, true)) {
                if (overlap > unfocusableWeakCandidateOverlap) {
                    assignAsWeek = true;
                } else if (overlap == unfocusableWeakCandidateOverlap
                        && preferLastSpan == (candidateStart
                                > unfocusableWeakCandidateSpanIndex)) {
                    assignAsWeek = true;
                }
            }
        }

        if (assignAsWeek) {
            if (candidate->hasFocusable()) {
                focusableWeakCandidate = candidate;
                focusableWeakCandidateSpanIndex = candidateLp->mSpanIndex;
                focusableWeakCandidateOverlap = std::min(candidateEnd, prevSpanEnd)
                        - std::max(candidateStart, prevSpanStart);
            } else {
                unfocusableWeakCandidate = candidate;
                unfocusableWeakCandidateSpanIndex = candidateLp->mSpanIndex;
                unfocusableWeakCandidateOverlap = std::min(candidateEnd, prevSpanEnd)
                        - std::max(candidateStart, prevSpanStart);
            }
        }
    }
    return (focusableWeakCandidate != nullptr) ? focusableWeakCandidate : unfocusableWeakCandidate;
}

bool GridLayoutManager::supportsPredictiveItemAnimations() {
    return mPendingSavedState == nullptr && !mPendingSpanCountChange;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GridLayoutManager::SpanSizeLookup::setSpanIndexCacheEnabled(bool cacheSpanIndices) {
    mCacheSpanIndices = cacheSpanIndices;
}

void GridLayoutManager::SpanSizeLookup::invalidateSpanIndexCache() {
    mSpanIndexCache.clear();
}

bool GridLayoutManager::SpanSizeLookup::isSpanIndexCacheEnabled() {
    return mCacheSpanIndices;
}

int GridLayoutManager::SpanSizeLookup::getCachedSpanIndex(int position, int spanCount) {
    if (!mCacheSpanIndices) {
        return getSpanIndex(position, spanCount);
    }
    const int existing = mSpanIndexCache.get(position, -1);
    if (existing != -1) {
        return existing;
    }
    const int value = getSpanIndex(position, spanCount);
    mSpanIndexCache.put(position, value);
    return value;
}

int GridLayoutManager::SpanSizeLookup::getSpanIndex(int position, int spanCount) {
    int positionSpanSize = getSpanSize(position);
    if (positionSpanSize == spanCount) {
        return 0; // quick return for full-span items
    }
    int span = 0;
    int startPos = 0;
    // If caching is enabled, try to jump
    if (mCacheSpanIndices && mSpanIndexCache.size() > 0) {
        int prevKey = findReferenceIndexFromCache(position);
        if (prevKey >= 0) {
            span = mSpanIndexCache.get(prevKey) + getSpanSize(prevKey);
            startPos = prevKey + 1;
        }
    }
    for (int i = startPos; i < position; i++) {
        int size = getSpanSize(i);
        span += size;
        if (span == spanCount) {
            span = 0;
        } else if (span > spanCount) {
            // did not fit, moving to next row / column
            span = size;
        }
    }
    if (span + positionSpanSize <= spanCount) {
        return span;
    }
    return 0;
}

int GridLayoutManager::SpanSizeLookup::findReferenceIndexFromCache(int position) {
    int lo = 0;
    int hi = mSpanIndexCache.size() - 1;

    while (lo <= hi) {
        int mid = (lo + hi) >> 1;
        int midVal = mSpanIndexCache.keyAt(mid);
        if (midVal < position) {
            lo = mid + 1;
        } else {
            hi = mid - 1;
        }
    }
    int index = lo - 1;
    if (index >= 0 && index < mSpanIndexCache.size()) {
        return mSpanIndexCache.keyAt(index);
    }
    return -1;
}

int GridLayoutManager::SpanSizeLookup::getSpanGroupIndex(int adapterPosition, int spanCount) {
    int span = 0;
    int group = 0;
    int positionSpanSize = getSpanSize(adapterPosition);
    for (int i = 0; i < adapterPosition; i++) {
        int size = getSpanSize(i);
        span += size;
        if (span == spanCount) {
            span = 0;
            group++;
        } else if (span > spanCount) {
            // did not fit, moving to next row / column
            span = size;
            group++;
        }
    }
    if (span + positionSpanSize > spanCount) {
        group++;
    }
    return group;
}

int GridLayoutManager::DefaultSpanSizeLookup::getSpanSize(int position) {
    return 1;
}

int GridLayoutManager::DefaultSpanSizeLookup::getSpanIndex(int position, int spanCount) {
    return position % spanCount;
}

GridLayoutManager::LayoutParams::LayoutParams(Context* c, const AttributeSet& attrs)
    :RecyclerView::LayoutParams(c, attrs){
}

GridLayoutManager::LayoutParams::LayoutParams(int width, int height)
    :RecyclerView::LayoutParams(width, height){
}

GridLayoutManager::LayoutParams::LayoutParams(const ViewGroup::MarginLayoutParams& source)
    :RecyclerView::LayoutParams(source){
}

GridLayoutManager::LayoutParams::LayoutParams(const ViewGroup::LayoutParams& source)
    :RecyclerView::LayoutParams(source){
}

GridLayoutManager::LayoutParams::LayoutParams(const RecyclerView::LayoutParams& source)
    :RecyclerView::LayoutParams(source){
}

int GridLayoutManager::LayoutParams::getSpanIndex()const{
    return mSpanIndex;
}

int GridLayoutManager::LayoutParams::getSpanSize()const{
    return mSpanSize;
}

}/*endof namespace*/
