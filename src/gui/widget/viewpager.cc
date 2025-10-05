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
#include <widget/viewpager.h>
#include <focusfinder.h>
#include <porting/cdtypes.h>
#include <utils/mathutils.h>
#include <porting/cdlog.h>
#include <cfloat>
#include <limits>

//https://www.androidos.net.cn/android/9.0.0_r8/xref/frameworks/support/viewpager/src/main/java/androidx/viewpager/widget/ViewPager.java
namespace cdroid{
class VPInterpolator:public Interpolator{
public:
    float getInterpolation(float t)const override{
        t -= 1.0f;
        return t * t * t * t * t + 1.0f;
    }
};

static VPInterpolator sVPInterpolator;

DECLARE_WIDGET(ViewPager);

ViewPager::ViewPager(int w,int h):ViewGroup(w,h){
    initViewPager();    
}

ViewPager::~ViewPager(){
    delete mObserver;
    delete mLeftEdge;
    delete mRightEdge;
    delete mMarginDrawable;
    delete mScroller;
    for(auto item: mItems){
        if(item != &mTempItem)delete item;
    }
    if(mVelocityTracker){
        mVelocityTracker->recycle();
        mVelocityTracker = nullptr;
    }
}

ViewPager::ViewPager(Context* context,const AttributeSet& attrs)
  :ViewGroup(context,attrs){
    initViewPager();
}

void ViewPager::initViewPager(){
    setWillNotDraw(false);
    setDescendantFocusability(FOCUS_AFTER_DESCENDANTS);
    setFocusable(true);
    Context* context = getContext();
    mInterpolator = &sVPInterpolator;
    mVelocityTracker = nullptr;
    mScroller = new Scroller(context, mInterpolator);
    mFakeDragging = false;
    mIsScrollStarted = false;
    mPopulatePending = false;
    mIsBeingDragged  = false;
    mIsUnableToDrag  = false;
    mScrollingCacheEnabled = false;
    mScrollState = SCROLL_STATE_IDLE;
    mAdapter  = nullptr;
    mObserver = nullptr;
    mInLayout = false;
    mFirstLayout = true;
    mPageTransformer = nullptr;
    mDrawingOrder = DRAW_ORDER_DEFAULT;
    mActivePointerId =INVALID_POINTER;
    mExpectedAdapterCount=0;
    mTopPageBounds   = 0;
    mBottomPageBounds= 0;
    mPageTransformerLayerType = View::LAYER_TYPE_NONE;
    mLastMotionX = mLastMotionY = 0;
    mInitialMotionX = mInitialMotionY = 0;
    ViewConfiguration configuration = ViewConfiguration::get(context);
    const float density = context->getDisplayMetrics().density;

    mTouchSlop = configuration.getScaledPagingTouchSlop();
    mMinimumVelocity = (int) (MIN_FLING_VELOCITY * density);
    mMaximumVelocity = configuration.getScaledMaximumFlingVelocity();
    mLeftEdge  = new EdgeEffect(context);
    mRightEdge = new EdgeEffect(context);
    mMarginDrawable =nullptr;

    mCurItem = 0;
    mPageMargin =0;
    mOffscreenPageLimit =3;
    mFirstOffset=-FLT_MAX;//std::numeric_limits<float>::max();
    mLastOffset = FLT_MAX;//std::numeric_limits<float>::max();

    mFlingDistance = (int) (MIN_DISTANCE_FOR_FLING * density);
    mCloseEnough   = (int) (CLOSE_ENOUGH * density);
    mDefaultGutterSize = (int) (DEFAULT_GUTTER_SIZE * density);
    mGutterSize = 0;
    mEndScrollRunnable=[this](){
        setScrollState(SCROLL_STATE_IDLE);
        populate();	
    };
}

ViewPager::ItemInfo::ItemInfo(){
    object=nullptr;
    position=0;
    scrolling=false;
    widthFactor=.0f;
    offset=.0f;
}

void ViewPager::onDetachedFromWindow() {
    removeCallbacks(mEndScrollRunnable);
    // To be on the safe side, abort the scroller
    if (mScroller && (mScroller->isFinished()==false)) {
        mScroller->abortAnimation();
    }
    ViewGroup::onDetachedFromWindow();
}

void ViewPager::setScrollState(int newState){
    if (mScrollState == newState) {
        return;
    }

    mScrollState = newState;
    if (mPageTransformer != nullptr) {
        // PageTransformers can do complex things that benefit from hardware layers.
        enableLayers(newState != SCROLL_STATE_IDLE);
    }
    dispatchOnScrollStateChanged(newState);
}

void ViewPager::setAdapter(PagerAdapter* adapter){
    if (mAdapter) {
        mAdapter->setViewPagerObserver(nullptr);
        mAdapter->startUpdate(this);
        for (int i = 0; i < mItems.size(); i++) {
            ItemInfo* ii = mItems[i];
            mAdapter->destroyItem(this, ii->position, ii->object);
            if(ii != &mTempItem)delete ii;
        }
        mAdapter->finishUpdate(this);
        mItems.clear();
        removeNonDecorViews();
        mCurItem = 0;
        scrollTo(0, 0);
    }

    PagerAdapter* oldAdapter = mAdapter;
    mAdapter = adapter;
    mExpectedAdapterCount = 0;

    if (mAdapter != nullptr) {
        if (mObserver == nullptr)  mObserver = new PagerObserver(this);
        mAdapter->setViewPagerObserver(mObserver);
        mPopulatePending = false;
        const bool wasFirstLayout = mFirstLayout;
        mFirstLayout = true;
        mExpectedAdapterCount = mAdapter->getCount();
        if (mRestoredCurItem >= 0) {
            //mAdapter->restoreState(mRestoredAdapterState, mRestoredClassLoader);
            setCurrentItemInternal(mRestoredCurItem, false, true);
            mRestoredCurItem = -1;
            //mRestoredAdapterState = nullptr;
            //mRestoredClassLoader = nullptr;
        } else if (!wasFirstLayout) {
            populate();
        } else {
            requestLayout();
        }
    }
    for(auto listener:mAdapterChangeListeners)
        if(listener&&oldAdapter != adapter)
           listener(*this, oldAdapter, adapter);
}

void ViewPager::removeNonDecorViews() {
    for (int i = 0; i < getChildCount(); i++) {
        View* child = getChildAt(i);
        LayoutParams* lp = (LayoutParams*) child->getLayoutParams();
        if (!lp->isDecor) {
            removeViewAt(i);
            i--;
        }
    }
}

PagerAdapter* ViewPager::getAdapter() {
    return mAdapter;
}

void ViewPager::addOnAdapterChangeListener(const OnAdapterChangeListener& listener){
    mAdapterChangeListeners.push_back(listener);
}

void ViewPager::removeOnAdapterChangeListener(const OnAdapterChangeListener& listener){
}

int ViewPager::getClientWidth() {
    return getMeasuredWidth() - getPaddingLeft() - getPaddingRight();
}

int ViewPager::getCurrentItem()const{
   return mCurItem;
}

void ViewPager::setCurrentItem(int item) {
    mPopulatePending = false;
    setCurrentItemInternal(item, !mFirstLayout, false);
}

void ViewPager::setCurrentItem(int item, bool smoothScroll) {
    mPopulatePending = false;
    setCurrentItemInternal(item, smoothScroll, false);
}

void ViewPager::setCurrentItemInternal(int item, bool smoothScroll, bool always) {
    setCurrentItemInternal(item, smoothScroll, always, 0);
}

void ViewPager::setCurrentItemInternal(int item, bool smoothScroll, bool always,int velocity){
    if (mAdapter == nullptr || mAdapter->getCount() <= 0) {
        setScrollingCacheEnabled(false);
        return;
    }
    if (!always && mCurItem == item && mItems.size()) {
        setScrollingCacheEnabled(false);
        return;
    }

    if (item < 0) {
        item = 0;
    } else if (item >= mAdapter->getCount()) {
        item = mAdapter->getCount() - 1;
    }

    const int pageLimit = mOffscreenPageLimit;
    if (item > (mCurItem + pageLimit) || item < (mCurItem - pageLimit)) {
        // We are doing a jump by more than one page.  To avoid
        // glitches, we want to keep all current pages in the view
        // until the scroll ends.
        for (int i = 0; i < mItems.size(); i++) {
            mItems[i]->scrolling = true;
        }
    }

    const bool dispatchSelected = mCurItem != item;

    if (mFirstLayout) {
        // We don't have any idea how big we are yet and shouldn't have any pages either.
        // Just set things up and let the pending layout handle things.
        mCurItem = item;
        if(dispatchSelected)
            dispatchOnPageSelected(item);
        requestLayout();
    } else {
        populate(item);
        scrollToItem(item, smoothScroll, velocity, dispatchSelected);
    }
}

void ViewPager::scrollToItem(int item, bool smoothScroll, int velocity, bool dispatchSelected){
    int destX=0;
    const ItemInfo* curInfo = infoForPosition(item);
    if(curInfo)
        destX= (int) (getClientWidth() * std::max(mFirstOffset,
                    std::min(curInfo->offset, mLastOffset)));
    if (smoothScroll) {
        smoothScrollTo(destX, 0, velocity);
        if(dispatchSelected)dispatchOnPageSelected(item);
    } else {
        if(dispatchSelected)dispatchOnPageSelected(item);
        completeScroll(false);
        scrollTo(destX, 0);
        pageScrolled(destX);
    }
}

void ViewPager::addOnPageChangeListener(const OnPageChangeListener& listener){
    auto it = std::find(mOnPageChangeListeners.begin(),mOnPageChangeListeners.end(),listener);
    if(it == mOnPageChangeListeners.end())
        mOnPageChangeListeners.push_back(listener);
}

void ViewPager::removeOnPageChangeListener(const OnPageChangeListener& listener){
    auto it = std::find(mOnPageChangeListeners.begin(),mOnPageChangeListeners.end(),listener);
    if(it !=mOnPageChangeListeners.end())
       mOnPageChangeListeners.erase(it);
}

void ViewPager::clearOnPageChangeListeners() {
    mOnPageChangeListeners.clear();
}
 
void ViewPager::dispatchOnPageScrolled(int position, float offset, int offsetPixels) {
    for (auto ls:mOnPageChangeListeners) {
        if (ls.onPageScrolled) {
            ls.onPageScrolled(position, offset, offsetPixels);
        }
    }

    if (mInternalPageChangeListener.onPageScrolled) {
        mInternalPageChangeListener.onPageScrolled(position, offset, offsetPixels);
    }
}

void ViewPager::dispatchOnPageSelected(int position) {
    for (auto ls:mOnPageChangeListeners) {
        if (ls.onPageSelected) {
            ls.onPageSelected(position);
        }
    }

    if (mInternalPageChangeListener.onPageSelected) {
        mInternalPageChangeListener.onPageSelected(position);
    }
}

void ViewPager::dispatchOnScrollStateChanged(int state) {
    for (auto ls:mOnPageChangeListeners) {
        if (ls.onPageScrollStateChanged) {
            ls.onPageScrollStateChanged(state);
        }
    }
    if (mInternalPageChangeListener.onPageScrollStateChanged) {
        mInternalPageChangeListener.onPageScrollStateChanged(state);
    }
}

void ViewPager::setPageTransformer(bool reverseDrawingOrder, PageTransformer* transformer) {
    const bool hasTransformer = transformer != nullptr;
    const bool needsPopulate = hasTransformer != (mPageTransformer != nullptr);
    mPageTransformer = transformer;
    setChildrenDrawingOrderEnabled(hasTransformer);
    if (hasTransformer) {
        mDrawingOrder = reverseDrawingOrder ? DRAW_ORDER_REVERSE : DRAW_ORDER_FORWARD;
    } else {
        mDrawingOrder = DRAW_ORDER_DEFAULT;
    }
    if (needsPopulate) populate();
}

int ViewPager::getChildDrawingOrder(int childCount, int i){
    const int index = mDrawingOrder == DRAW_ORDER_REVERSE ? childCount - 1 - i : i;
    const int result = ((LayoutParams*) mDrawingOrderedChildren.at(index)->getLayoutParams())->childIndex;
    return result;
}

ViewPager::OnPageChangeListener ViewPager::setInternalPageChangeListener(const OnPageChangeListener& listener) {
    OnPageChangeListener oldListener = mInternalPageChangeListener;
    mInternalPageChangeListener = listener;
    return oldListener;
}

int ViewPager::getOffscreenPageLimit()const{
    return mOffscreenPageLimit;
}

void ViewPager::setOffscreenPageLimit(int limit){
    if (limit < DEFAULT_OFFSCREEN_PAGES) {
        LOGD("Requested offscreen page limit %d too small; defaulting to ",limit,DEFAULT_OFFSCREEN_PAGES);
        limit = DEFAULT_OFFSCREEN_PAGES;
    }
    if (limit != mOffscreenPageLimit) {
        mOffscreenPageLimit = limit;
        populate();
    }
}

int ViewPager::getPageMargin()const{
    return mPageMargin;
}

void ViewPager::setPageMargin(int marginPixels){
    const int oldMargin = mPageMargin;
    mPageMargin = marginPixels;

    const int width = getWidth();
    recomputeScrollPosition(width, width, marginPixels, oldMargin);
    requestLayout();
}

void ViewPager::setPageMarginDrawable(Drawable* d){
    mMarginDrawable = d;
    if (d != nullptr) refreshDrawableState();
    setWillNotDraw(d == nullptr);
    invalidate();    
}

void ViewPager::setPageMarginDrawable(const std::string&resId){
    setPageMarginDrawable(getContext()->getDrawable(resId));
}

bool ViewPager::verifyDrawable(Drawable* who)const{
    return ViewGroup::verifyDrawable(who) || who == mMarginDrawable;
}

void ViewPager::drawableStateChanged(){
    ViewGroup::drawableStateChanged();
    Drawable* d = mMarginDrawable;
    if (d  && d->isStateful() && d->setState(getDrawableState())){
        invalidateDrawable(*d);
    }
}

static float distanceInfluenceForSnapDuration(float f) {
    f -= 0.5f; // center the values about 0.
    f *= 0.3f * (float) M_PI / 2.0f;
    return (float) std::sin(f);
}

void ViewPager::smoothScrollTo(int x, int y) {
    smoothScrollTo(x, y, 0);
}

void ViewPager::smoothScrollTo(int x, int y, int velocity){
    if (getChildCount() == 0) {
        // Nothing to do.
        setScrollingCacheEnabled(false);
        return;
    }
    const bool wasScrolling = mScroller && !mScroller->isFinished();
    int sx = getScrollX();
    const int sy = getScrollY();
    const int dx = x - sx;
    const int dy = y - sy;
    if(wasScrolling){
        sx = mIsScrollStarted ? mScroller->getCurrX() : mScroller->getStartX();
        mScroller->abortAnimation();
        setScrollingCacheEnabled(false);
    }
    if (dx == 0 && dy == 0) {
        completeScroll(false);
        populate();
        setScrollState(SCROLL_STATE_IDLE);
        return;
    }

    setScrollingCacheEnabled(true);
    setScrollState(SCROLL_STATE_SETTLING);

    const int width = getClientWidth();
    const int halfWidth = width / 2;
    const float distanceRatio = std::min(1.f, 1.0f * std::abs(dx) / width);
    const float distance = halfWidth + halfWidth
          * distanceInfluenceForSnapDuration(distanceRatio);

    int duration;
    velocity = std::abs(velocity);
    if (velocity > 0) {
        duration = 4 * std::round(1000 * std::abs(distance / velocity));
    } else {
        const float pageWidth = width * mAdapter->getPageWidth(mCurItem);
        const float pageDelta = (float) std::abs(dx) / (pageWidth + mPageMargin);
        duration = (int) ((pageDelta + 1) * 100);
    }
    duration = std::min(duration,(int)MAX_SETTLE_DURATION);
    mIsScrollStarted = false;
    // Reset the "scroll started" flag. It will be flipped to true in all places
    // where we call computeScrollOffset().
    mScroller->startScroll(sx, sy, dx, dy, duration);
    postInvalidateOnAnimation();
}

ViewPager::ItemInfo* ViewPager::addNewItem(int position, int index){
    ItemInfo* ii = new ItemInfo();
    ii->position = position;
    ii->object = mAdapter->instantiateItem(this, position);
    View* view=(View*)ii->object;
    LayoutParams*lp=(LayoutParams*)view->getLayoutParams();
    lp->isDecor = false;
    ii->widthFactor = mAdapter->getPageWidth(position);
    ii->scrolling=false;
    if (index < 0 || index >= mItems.size()) {
        mItems.push_back(ii);
    } else {
        mItems.insert(mItems.begin()+index, ii);
    }
    return ii;
}

void ViewPager::dataSetChanged(){
    const int adapterCount = mAdapter->getCount();
    mExpectedAdapterCount = adapterCount;
    bool needPopulate = mItems.size() < mOffscreenPageLimit * 2 + 1
                && mItems.size() < adapterCount;
    int newCurrItem = mCurItem;

    bool isUpdating = false;
    for (int i = 0; i < mItems.size(); i++) {
        ItemInfo* ii = mItems[i];
        int newPos = mAdapter->getItemPosition(ii->object);

        if (newPos == PagerAdapter::POSITION_UNCHANGED) {
            continue;
        }

        if (newPos == PagerAdapter::POSITION_NONE) {
            ItemInfo*item = mItems.at(i);
            mItems.erase(mItems.begin()+i);
            i--;

            if (!isUpdating) {
                mAdapter->startUpdate(this);
                isUpdating = true;
            }

            mAdapter->destroyItem(this, ii->position, ii->object);
            needPopulate = true;

            if (mCurItem == ii->position) { // Keep the current item in the valid range
                newCurrItem = std::max(0, std::min(mCurItem, adapterCount - 1));
                needPopulate = true;
            }
            if(item != &mTempItem)delete item;
            continue;
        }

        if (ii->position != newPos) {
            if (ii->position == mCurItem) {// Our current item changed position. Follow it.
                newCurrItem = newPos;
            }
            ii->position = newPos;
            needPopulate = true;
        }
    }

    if (isUpdating)  mAdapter->finishUpdate(this);

    std::sort(mItems.begin(),mItems.end(),[](ItemInfo*a,ItemInfo*b)->bool{return a->position < b->position;});

    if (needPopulate) {  // Reset our known page widths; populate will recompute them.
        for (auto child:mChildren){
            LayoutParams* lp = (LayoutParams*) child->getLayoutParams();
            if (!lp->isDecor) lp->widthFactor = 0.f;
        }

        setCurrentItemInternal(newCurrItem, false, true);
        requestLayout();
    }
}

void ViewPager::populate() {
    populate(mCurItem);
}

void ViewPager::populate(int newCurrentItem){
    ItemInfo* oldCurInfo = nullptr;
    if (mCurItem != newCurrentItem) {
        oldCurInfo = infoForPosition(mCurItem);
        mCurItem = newCurrentItem;
    }

    if (mAdapter == nullptr) {
        sortChildDrawingOrder();
        return;
    }
    // Also, don't populate until we are attached to a window.  This is to
    // avoid trying to populate before we have restored our view hierarchy
    // state and conflicting with what is restored.
    if(!isAttachedToWindow()){
        return ;
    }
    // Bail now if we are waiting to populate.  This is to hold off
    // on creating views from the time the user releases their finger to
    // fling to a new position until we have finished the scroll to
    // that position, avoiding glitches from happening at that point.
    if (mPopulatePending) {
        LOGV("populate is pending, skipping for now...");
        sortChildDrawingOrder();
        return;
    }

    mAdapter->startUpdate(this);

    const int pageLimit = mOffscreenPageLimit;
    const int startPos = std::max(0, mCurItem - pageLimit);
    const int N = mAdapter->getCount();
    const int endPos = std::min(N - 1, mCurItem + pageLimit);

    LOGE_IF(N != mExpectedAdapterCount,"The application's PagerAdapter changed the adapter's contents without "
                    "calling PagerAdapter#notifyDataSetChanged! Expected adapter item count: %d , found: %d",
                    mExpectedAdapterCount,N);

    // Locate the currently focused item or add it if needed.
    int curIndex = -1;
    ItemInfo* curItem = nullptr;
    for (curIndex = 0; curIndex < mItems.size(); curIndex++) {
        ItemInfo* ii = mItems[curIndex];
        if (ii->position >= mCurItem) {
            if (ii->position == mCurItem) curItem = ii;
            break;
        }
    }

    if (curItem == nullptr && N > 0) {
        curItem = addNewItem(mCurItem, curIndex);
    }

    // Fill 3x the available width or up to the number of offscreen
    // pages requested to either side, whichever is larger.
    // If we have no current item we have no work to do.
    if (curItem != nullptr) {
        float extraWidthLeft = 0.f;
        int itemIndex = curIndex - 1;
        ItemInfo* ii = itemIndex >= 0 ? mItems[itemIndex] : nullptr;
        const int clientWidth = getClientWidth();
        const float leftWidthNeeded = clientWidth <= 0 ? 0 :
                2.f - curItem->widthFactor + (float) getPaddingLeft() / (float) clientWidth;
        for (int pos = mCurItem - 1; pos >= 0; pos--) {//left items of curitem
            if (extraWidthLeft >= leftWidthNeeded && pos < startPos) {
                if (ii == nullptr)  break;
                if (pos == ii->position && !ii->scrolling) {
                    mItems.erase(mItems.begin()+itemIndex);
                    mAdapter->destroyItem(this, pos, ii->object);
                    LOGV("destroyItem() with pos:%d/%d view:%p curitem=%d/%d",pos,ii->position,ii->object,mCurItem,mItems.size());
                    if(ii != &mTempItem)delete ii;
                    itemIndex--;
                    curIndex--;
                    ii = itemIndex >= 0 ? mItems[itemIndex] : nullptr;
                }
            } else if (ii != nullptr && pos == ii->position) {
                extraWidthLeft += ii->widthFactor;
                itemIndex--;
                ii = itemIndex >= 0 ? mItems[itemIndex] : nullptr;
            } else {
                ii = addNewItem(pos, itemIndex + 1);
                extraWidthLeft += ii->widthFactor;
                curIndex++;
                ii = itemIndex >= 0 ? mItems[itemIndex] : nullptr;
            }
        }

        float extraWidthRight = curItem->widthFactor;
        itemIndex = curIndex + 1;
        if (extraWidthRight < 2.f) {
            const float rightWidthNeeded = clientWidth <= 0 ? 0 :
                    (float) getPaddingRight() / (float) clientWidth + 2.f;
            ii = itemIndex < mItems.size() ? mItems[itemIndex] : nullptr;

            for (int pos = mCurItem + 1; pos < N; pos++) {//right items of curitems
                if (extraWidthRight >= rightWidthNeeded && pos > endPos) {
                    if (ii == nullptr)  break;

                    if (pos == ii->position && !ii->scrolling) {
                        mItems.erase(mItems.begin()+itemIndex);
                        mAdapter->destroyItem(this, pos, ii->object);
                        if(ii != &mTempItem)delete ii;
                        ii = itemIndex < mItems.size() ? mItems[itemIndex] : nullptr;
                    }
                } else if (ii != nullptr && pos == ii->position) {
                    extraWidthRight += ii->widthFactor;
                    itemIndex++;
                    ii = itemIndex < mItems.size() ? mItems.at(itemIndex) : nullptr;
                } else {
                    ii = addNewItem(pos, itemIndex);
                    itemIndex++;
                    extraWidthRight += ii->widthFactor;
                    ii = itemIndex < mItems.size() ? mItems.at(itemIndex) : nullptr;
                }
            }
        }
        calculatePageOffsets(curItem, curIndex, oldCurInfo);
        mAdapter->setPrimaryItem(this, mCurItem, curItem != nullptr ? curItem->object : nullptr);
    }

    mAdapter->finishUpdate(this);

    // Check width measurement of current pages and drawing sort order.
    // Update LayoutParams as needed.
    for (int i=0;i<getChildCount();i++){
        View*child=getChildAt(i);
        LayoutParams* lp = (LayoutParams*) child->getLayoutParams();
        lp->childIndex = i;
        if (!lp->isDecor && lp->widthFactor == 0.f) {
            // 0 means requery the adapter for this, it doesn't have a valid width.
            ItemInfo* ii = infoForChild(child);
            if (ii != nullptr) {
                lp->widthFactor = ii->widthFactor;
                lp->position = ii->position;
            }
        }
    }
    sortChildDrawingOrder();

    if (hasFocus()) {
        View* currentFocused = findFocus();
        ItemInfo* ii = currentFocused != nullptr ? infoForAnyChild(currentFocused) : nullptr;
        if (ii == nullptr || ii->position != mCurItem) {
            for (int i = 0; i < getChildCount(); i++) {
                View* child = getChildAt(i);
                ii = infoForChild(child);
                if (ii != nullptr && ii->position == mCurItem) {
                    if (child->requestFocus(View::FOCUS_FORWARD)) {
                        break;
                    }
                }
            }
        }
    }
}

void ViewPager::sortChildDrawingOrder(){
    if (mDrawingOrder != DRAW_ORDER_DEFAULT) {
        mDrawingOrderedChildren = mChildren;
        std::sort(mDrawingOrderedChildren.begin(),mDrawingOrderedChildren.end(), [](View*lhs,View*rhs)->bool{
            LayoutParams* llp = (LayoutParams*) lhs->getLayoutParams();
            LayoutParams* rlp = (LayoutParams*) rhs->getLayoutParams();
            if (llp->isDecor != rlp->isDecor) {
                return !llp->isDecor ;//? 1 : -1;
            }
            return (llp->position < rlp->position);
        });
   }
}

void ViewPager::calculatePageOffsets(ItemInfo* curItem, int curIndex, ItemInfo* oldCurInfo){
    const int N = mAdapter->getCount();
    const int width = getClientWidth();
    const float marginOffset = width > 0 ? (float) mPageMargin / width : 0;
    // Fix up offsets for later layout.
    if (oldCurInfo != nullptr) {
        const int oldCurPosition = oldCurInfo->position;
        // Base offsets off of oldCurInfo.
        if (oldCurPosition < curItem->position) {
            int itemIndex = 0;
            ItemInfo*ii = nullptr;
            float offset = oldCurInfo->offset + oldCurInfo->widthFactor + marginOffset;
            for (int pos = oldCurPosition + 1;
                    pos <= curItem->position && itemIndex < mItems.size(); pos++) {
                ii = mItems.at(itemIndex);
                while (pos > ii->position && itemIndex < mItems.size() - 1) {
                    itemIndex++;
                    ii = mItems.at(itemIndex);
                }
                while (pos < ii->position) {
                    // We don't have an item populated for this,
                    // ask the adapter for an offset.
                    offset += mAdapter->getPageWidth(pos) + marginOffset;
                    pos++;
                }
                ii->offset = offset;
                offset += ii->widthFactor + marginOffset;
            }
        } else if (oldCurPosition > curItem->position) {
            int itemIndex = mItems.size() - 1;
            ItemInfo*ii = nullptr;
            float offset = oldCurInfo->offset;
            for (int pos = oldCurPosition - 1;
                pos >= curItem->position && itemIndex >= 0; pos--) {
                ii = mItems.at(itemIndex);
                while (pos < ii->position && itemIndex > 0) {
                    itemIndex--;
                    ii = mItems.at(itemIndex);
                }
                while (pos > ii->position) {
                    // We don't have an item populated for this,
                    // ask the adapter for an offset.
                    offset -= mAdapter->getPageWidth(pos) + marginOffset;
                    pos--;
                }
                offset -= ii->widthFactor + marginOffset;
                ii->offset = offset;
            }
        }
    }
     
    // Base all offsets off of curItem.
    const int itemCount = mItems.size();
    float offset = curItem->offset;
    int pos = curItem->position - 1;
    mFirstOffset= (curItem->position == 0) ? curItem->offset : (-FLT_MAX);//std::numeric_limits<float>::max());
    mLastOffset = (curItem->position == N - 1)
             ? (curItem->offset + curItem->widthFactor - 1) : FLT_MAX;//std::numeric_limits<float>::max();

    // Previous pages
    for (int i = curIndex - 1; i >= 0; i--, pos--) {
         ItemInfo* ii = mItems.at(i);
         while (pos > ii->position) {
             offset -= mAdapter->getPageWidth(pos--) + marginOffset;
         }
         offset -= ii->widthFactor + marginOffset;
         ii->offset = offset;
         if (ii->position == 0) mFirstOffset = offset;
     }

    offset = curItem->offset + curItem->widthFactor + marginOffset;
    pos = curItem->position + 1;

     // Next pages
    for (int i = curIndex + 1; i < itemCount; i++, pos++) {
         ItemInfo* ii = mItems.at(i);
         while (pos < ii->position) {
             offset += mAdapter->getPageWidth(pos++) + marginOffset;
         }
         if (ii->position == N - 1) {
             mLastOffset = offset + ii->widthFactor - 1;
         }
         ii->offset = offset;
         offset += ii->widthFactor + marginOffset;
    }
    mNeedCalculatePageOffsets = false;
}

void ViewPager::addView(View* child, int index, ViewGroup::LayoutParams* params){
    if (!checkLayoutParams(params)) {
        ViewGroup::LayoutParams*olp = params;
        params = generateLayoutParams(params);
        if(olp!=child->getLayoutParams()){
            //param is not got from child,we must destroied it.
            delete olp;
        }
    }
    LayoutParams* lp = (LayoutParams*) params;
    //Any views added via inflation should be classed as part of the decor
    //lp->isDecor |=  isDecorView(child);isDecor has been setted in addNewItem(after Adapter::instantiateItem)
    if (mInLayout) {
        LOGD_IF(lp != nullptr && lp->isDecor,"Cannot add pager decor view during layout");
        lp->needsMeasure = true;
        addViewInLayout(child, index, params);
    } else {
        ViewGroup::addView(child, index, params);
    }

    if (USE_CACHE) {
        if (child->getVisibility() != GONE) {
            child->setDrawingCacheEnabled(mScrollingCacheEnabled);
        } else {
            child->setDrawingCacheEnabled(false);
        }
    }    
}


void ViewPager::removeView(View* view){
    if (mInLayout) {
        removeViewInLayout(view);
    } else {
        ViewGroup::removeView(view);
    }
}

void* ViewPager::getCurrent()const{
    ItemInfo* itemInfo = infoForPosition(getCurrentItem());
    return itemInfo ? itemInfo->object:nullptr;
}

ViewPager::ItemInfo* ViewPager::infoForChild(View* child){
    for (int i = 0; i < mItems.size(); i++) {
        ItemInfo* ii = mItems[i];
        if (mAdapter->isViewFromObject(child, ii->object)) {
            return ii;
        }
    }
    return nullptr;
}

ViewPager::ItemInfo* ViewPager::infoForAnyChild(View* child){
    ViewGroup* parent;
    while ((parent = child->getParent()) != this) {
        if (parent == nullptr /*|| !(parent instanceof View)*/) {
            return nullptr;
        }
        child = (View*) parent;
    }
    return infoForChild(child);
}

ViewPager::ItemInfo* ViewPager::infoForPosition(int position)const{
    for (int i = 0; i < mItems.size(); i++) {
        ItemInfo* ii = mItems[i];
        if (ii->position == position) {
            return ii;
        }
    }
    return nullptr;
}

void ViewPager::onAttachedToWindow() {
    ViewGroup::onAttachedToWindow();
    mFirstLayout = true;
}

void ViewPager::onMeasure(int widthMeasureSpec, int heightMeasureSpec){
    setMeasuredDimension(getDefaultSize(0, widthMeasureSpec),getDefaultSize(0, heightMeasureSpec));

    const int measuredWidth = getMeasuredWidth();
    const int maxGutterSize = measuredWidth / 10;
    mGutterSize = std::min(maxGutterSize, mDefaultGutterSize);

    // Children are just made to fill our space.
    int childWidthSize = measuredWidth - getPaddingLeft() - getPaddingRight();
    int childHeightSize = getMeasuredHeight() - getPaddingTop() - getPaddingBottom();

    /* Make sure all children have been properly measured. Decor views first.
     * Right now we cheat and make this less complicated by assuming decor
     * views won't intersect. We will pin to edges based on gravity.*/

    for (auto child:mChildren){
        if (child->getVisibility() != GONE) {
            LayoutParams* lp = (LayoutParams*) child->getLayoutParams();
            if (lp != nullptr && lp->isDecor) {
                int hgrav = lp->gravity & Gravity::HORIZONTAL_GRAVITY_MASK;
                int vgrav = lp->gravity & Gravity::VERTICAL_GRAVITY_MASK;
                int widthMode = MeasureSpec::AT_MOST;
                int heightMode = MeasureSpec::AT_MOST;
                const bool consumeVertical  = vgrav == Gravity::TOP || vgrav == Gravity::BOTTOM;
                const bool consumeHorizontal= hgrav == Gravity::LEFT || hgrav == Gravity::RIGHT;

                if (consumeVertical) {
                    widthMode = MeasureSpec::EXACTLY;
                } else if (consumeHorizontal) {
                    heightMode = MeasureSpec::EXACTLY;
                }

                int widthSize = childWidthSize;
                int heightSize = childHeightSize;
                if (lp->width != LayoutParams::WRAP_CONTENT) {
                    widthMode = MeasureSpec::EXACTLY;
                    if (lp->width != LayoutParams::MATCH_PARENT) {
                        widthSize = lp->width;
                    }
                }
                if (lp->height != LayoutParams::WRAP_CONTENT) {
                    heightMode = MeasureSpec::EXACTLY;
                    if (lp->height != LayoutParams::MATCH_PARENT) {
                        heightSize = lp->height;
                    }
                }
                const int widthSpec = MeasureSpec::makeMeasureSpec(widthSize, widthMode);
                const int heightSpec = MeasureSpec::makeMeasureSpec(heightSize, heightMode);
                child->measure(widthSpec, heightSpec);

                if (consumeVertical) {
                    childHeightSize -= child->getMeasuredHeight();
                } else if (consumeHorizontal) {
                    childWidthSize -= child->getMeasuredWidth();
                }
            }
        }
    }

    mChildWidthMeasureSpec = MeasureSpec::makeMeasureSpec(childWidthSize, MeasureSpec::EXACTLY);
    mChildHeightMeasureSpec = MeasureSpec::makeMeasureSpec(childHeightSize, MeasureSpec::EXACTLY);

    // Make sure we have created all fragments that we need to have shown.
    mInLayout = true;
    populate();
    mInLayout = false;

    // Page views next.
    for (auto child:mChildren){
        if (child->getVisibility() != GONE) {
            LayoutParams* lp = (LayoutParams*) child->getLayoutParams();
            const float widthFactor = lp?lp->widthFactor:1.f;
            if ((lp == nullptr) || !lp->isDecor) {
                const int widthSpec = MeasureSpec::makeMeasureSpec(
                       (int) (childWidthSize * widthFactor), MeasureSpec::EXACTLY);
                child->measure(widthSpec, mChildHeightMeasureSpec);
            }
        }
    }
}

void ViewPager::onSizeChanged(int w, int h, int oldw, int oldh){
    ViewGroup::onSizeChanged(w, h, oldw, oldh);

    // Make sure scroll position is set correctly.
    if (w != oldw) {
        recomputeScrollPosition(w, oldw, mPageMargin, mPageMargin);
    }
}

void ViewPager::recomputeScrollPosition(int width, int oldWidth, int margin, int oldMargin){
     if (oldWidth > 0 && mItems.size()) {
        if (!mScroller->isFinished()) {
            mScroller->setFinalX(getCurrentItem() * getClientWidth());
        } else{
            const int widthWithMargin = width - getPaddingLeft() - getPaddingRight() + margin;
            const int oldWidthWithMargin = oldWidth - getPaddingLeft() - getPaddingRight()+ oldMargin;
            const int xpos = getScrollX();
            const float pageOffset = (float) xpos / oldWidthWithMargin;
            const int newOffsetPixels = (int) (pageOffset * widthWithMargin);

            scrollTo(newOffsetPixels, getScrollY());
        }
    } else {
        const ItemInfo* ii = infoForPosition(mCurItem);
        const float scrollOffset = ii != nullptr ? std::min(ii->offset, mLastOffset) : 0;
        const int scrollPos = (int) (scrollOffset * (width - getPaddingLeft() - getPaddingRight()));
        if (scrollPos != getScrollX()) {
            completeScroll(false);
            scrollTo(scrollPos, getScrollY());
        }
    }
}

void ViewPager::onLayout(bool changed, int l, int t, int width, int height){
    const int count = getChildCount();
    int paddingLeft = getPaddingLeft();
    int paddingTop = getPaddingTop();
    int paddingRight = getPaddingRight();
    int paddingBottom = getPaddingBottom();
    const int scrollX = getScrollX();
    
    int decorCount = 0;
    
    // First pass - decor views. We need to do this in two passes so that
    // we have the proper offsets for non-decor views later.
    for (int i = 0; i < count; i++) {
        View* child = getChildAt(i);
        LayoutParams* lp = (LayoutParams*) child->getLayoutParams();
        if (child->getVisibility() == GONE || lp->isDecor==false)
            continue;
        int childLeft = 0, childTop = 0;
                
        const int hgrav = lp->gravity & Gravity::HORIZONTAL_GRAVITY_MASK;
        const int vgrav = lp->gravity & Gravity::VERTICAL_GRAVITY_MASK;
        switch (hgrav) {
        default:childLeft = paddingLeft; break;
        case Gravity::LEFT:
             childLeft = paddingLeft;
             paddingLeft += child->getMeasuredWidth();
             break;
        case Gravity::CENTER_HORIZONTAL:
             childLeft = std::max((width - child->getMeasuredWidth()) / 2,paddingLeft);
             break;
        case Gravity::RIGHT:
             childLeft = width - paddingRight - child->getMeasuredWidth();
             paddingRight += child->getMeasuredWidth();
             break;
        }
        switch (vgrav) {
        default:
            childTop = paddingTop;
            break;
        case Gravity::TOP:
            childTop = paddingTop;
            paddingTop += child->getMeasuredHeight();
            break;
        case Gravity::CENTER_VERTICAL:
            childTop = std::max((height - child->getMeasuredHeight()) / 2,paddingTop);
            break;
        case Gravity::BOTTOM:
            childTop = height - paddingBottom - child->getMeasuredHeight();
            paddingBottom += child->getMeasuredHeight();
            break;
        }
        childLeft += scrollX;
        child->layout(childLeft, childTop, child->getMeasuredWidth(), child->getMeasuredHeight());
        LOGV("decorchild[%d]%p setTo(%d,%d,%d,%d)",i,child,childLeft,childTop,child->getMeasuredWidth(), child->getMeasuredHeight());
        decorCount++;
    }
    
    const int childWidth = width - paddingLeft - paddingRight;
    // Page views. Do this once we have the right padding offsets from above.
    for (int i = 0; i < count; i++) {
        View* child = getChildAt(i);
        LayoutParams* lp = (LayoutParams*) child->getLayoutParams();
        if (child->getVisibility() == GONE||lp->isDecor) continue;

        ItemInfo* ii=infoForChild(child);
        if(ii==nullptr) continue;
        if (lp->needsMeasure) {
            // This was added during layout and needs measurement.
            // Do it now that we know what we're working with.
            lp->needsMeasure = false;
            int widthSpec = MeasureSpec::makeMeasureSpec( (int) (childWidth * lp->widthFactor),MeasureSpec::EXACTLY);
            int heightSpec= MeasureSpec::makeMeasureSpec( (int) (height - paddingTop - paddingBottom),MeasureSpec::EXACTLY);
            child->measure(widthSpec, heightSpec);
        }

        int childMeasuredWidth = child->getMeasuredWidth();
        int startOffset = (int) (childWidth * ii->offset);
        int childLeft;
        if (isLayoutRtl()) {
            childLeft = MAX_SCROLL_X - paddingRight - startOffset - childMeasuredWidth;
        } else {
            childLeft = paddingLeft + startOffset;
        }

        int childTop = paddingTop;        
        LOGV("nonDecorChild[%d]%p setTo(%d,%d,%d,%d)",i,child,childLeft, childTop,
               child->getMeasuredWidth(), child->getMeasuredHeight());
        child->layout(childLeft, childTop, child->getMeasuredWidth(), child->getMeasuredHeight());

    }
    mTopPageBounds = paddingTop;
    mBottomPageBounds = height - paddingBottom;
    mDecorChildCount = decorCount;
    
    if (mFirstLayout) {
        scrollToItem(mCurItem, false, 0, false);
    }
    mFirstLayout = false;
}

void ViewPager::computeScroll(){
    if (!mScroller->isFinished() && mScroller->computeScrollOffset()) {
        int oldX = getScrollX();
        int oldY = getScrollY();
        int x = mScroller->getCurrX();
        int y = mScroller->getCurrY();

        if (oldX != x || oldY != y) {
            scrollTo(x, y);
            if (!pageScrolled(x)) {
                mScroller->abortAnimation();
                scrollTo(0, y);
            }
        }

        // Keep on drawing until the animation has finished.
        postInvalidateOnAnimation();
        return;
    }

    // Done with scroll, clean up state.
    completeScroll(true);
}

bool ViewPager::pageScrolled(int scrollX){
   if (mItems.size() == 0) {
        if(mFirstLayout)return false;
        mCalledSuper = false;
        onPageScrolled(0, 0, 0);
        if (!mCalledSuper) {
            throw std::runtime_error("onPageScrolled did not call superclass implementation");
        }
        return false;
    }

    ItemInfo* ii = infoForCurrentScrollPosition();
    int width = getClientWidth();
    int widthWithMargin = width + mPageMargin;
    float marginOffset = (float) mPageMargin / width;
    int currentPage = ii->position;
    float pageOffset = (((float) scrollX / width) - ii->offset)
                / (ii->widthFactor + marginOffset);
    int offsetPixels = (int) (pageOffset * widthWithMargin);

    mCalledSuper = false;
    onPageScrolled(currentPage, pageOffset, offsetPixels);
    LOGE_IF(!mCalledSuper,"onPageScrolled did not call superclass implementation");
    return true;
}

void ViewPager::onPageScrolled(int position, float offset, int offsetPixels){
    if (mDecorChildCount > 0) {
        int scrollX = getScrollX();
        int paddingLeft = getPaddingLeft();
        int paddingRight = getPaddingRight();
        int width = getWidth();
        int childCount = getChildCount();
        for (int i = 0; i < childCount; i++) {
            View* child = getChildAt(i);
            LayoutParams* lp = (LayoutParams*) child->getLayoutParams();
            if (!lp->isDecor) continue;

            int hgrav = lp->gravity & Gravity::HORIZONTAL_GRAVITY_MASK;
            int childLeft = 0;
            switch (hgrav) {
            default:childLeft = paddingLeft;   break;
            case Gravity::LEFT:
                 childLeft = paddingLeft;
                 paddingLeft += child->getWidth();
                 break;
            case Gravity::CENTER_HORIZONTAL:
                 childLeft = std::max((width - child->getMeasuredWidth()) / 2,paddingLeft);
                 break;
            case Gravity::RIGHT:
                 childLeft = width - paddingRight - child->getMeasuredWidth();
                 paddingRight += child->getMeasuredWidth();
                 break;
            }
            childLeft += scrollX;

            int childOffset = childLeft - child->getLeft();
            if (childOffset != 0) {
                child->offsetLeftAndRight(childOffset);
            }
        }
    }

    dispatchOnPageScrolled(position, offset, offsetPixels);

    if (mPageTransformer != nullptr) {
        int scrollX = getScrollX();
        int childCount = getChildCount();
        for (int i = 0; i < childCount; i++) {
             View* child = getChildAt(i);
             LayoutParams* lp = (LayoutParams*) child->getLayoutParams();

             if (lp->isDecor) continue;
             float transformPos = (float) (child->getLeft() - scrollX) / getClientWidth();
             mPageTransformer->transformPage(*child, transformPos);
        }
    }
    mCalledSuper = true;
}

void ViewPager::completeScroll(bool postEvents){
    bool needPopulate = mScrollState == SCROLL_STATE_SETTLING;
    if (needPopulate) {
        // Done with scroll, no longer want to cache view drawing.
        setScrollingCacheEnabled(false);
        if(!mScroller->isFinished()){
            mScroller->abortAnimation();
            int oldX = getScrollX();
            int oldY = getScrollY();
            int x = mScroller->getCurrX();
            int y = mScroller->getCurrY();
            if (oldX != x || oldY != y) {
                scrollTo(x, y);
                if (x != oldX) pageScrolled(x);
            }
        }
    }
    mPopulatePending = false;
    for (int i = 0; i < mItems.size(); i++) {
        ItemInfo* ii = mItems.at(i);
        if (ii->scrolling) {
            needPopulate = true;
            ii->scrolling = false;
        }
    }
    if (needPopulate) {
        if (postEvents) 
            postOnAnimation(mEndScrollRunnable);
        else
            mEndScrollRunnable();
    }
}

bool ViewPager::isGutterDrag(float x, float dx){
    return (x < mGutterSize && dx > 0) || (x > getWidth() - mGutterSize && dx < 0);
}

void ViewPager::enableLayers(bool enable) {
    const int childCount = getChildCount();
    for (int i = 0; i < childCount; i++) {
        const int layerType = enable ? mPageTransformerLayerType : View::LAYER_TYPE_NONE;
        getChildAt(i)->setLayerType(layerType);//, nullptr);
    }
}

bool ViewPager::onInterceptTouchEvent(MotionEvent& ev){
    int action = ev.getAction() & MotionEvent::ACTION_MASK;
    
    // Always take care of the touch gesture being complete.
    if (action == MotionEvent::ACTION_CANCEL || action == MotionEvent::ACTION_UP) {
        // Release the drag.
        resetTouch();
        return false;
    }
    
    // Nothing more to do here if we have decided whether or not we
    // are dragging.
    if (action != MotionEvent::ACTION_DOWN) {
        if (mIsBeingDragged) return true;
        if (mIsUnableToDrag) return false;
    }
    
    switch (action) {
    case MotionEvent::ACTION_MOVE: {
            /* mIsBeingDragged == false, otherwise the shortcut would have caught it. Check
             * whether the user has moved far enough from his original down touch. */
    
            /* Locally do absolute value. mLastMotionY is set to the y value of the down event.*/
            int activePointerId = mActivePointerId;
            if (activePointerId == INVALID_POINTER) {
                // If we don't have a valid id, the touch down wasn't on content.
                break;
            }
    
            int pointerIndex = ev.findPointerIndex(activePointerId);
            float x = ev.getX(pointerIndex);
            float dx = x - mLastMotionX;
            float xDiff = std::abs(dx);
            float y = ev.getY(pointerIndex);
            float yDiff = std::abs(y - mInitialMotionY);

            if (dx != 0 && !isGutterDrag(mLastMotionX, dx)
                    && canScroll(this, false, (int) dx, (int) x, (int) y)) {
                // Nested view has scrollable area under this point. Let it be handled there.
                mLastMotionX = x;
                mLastMotionY = y;
                mIsUnableToDrag = true;
                return false;
            }
            if (xDiff > mTouchSlop && xDiff * 0.5f > yDiff) {
                mIsBeingDragged = true;
                requestParentDisallowInterceptTouchEvent(true);
                setScrollState(SCROLL_STATE_DRAGGING);
                mLastMotionX = dx > 0 ? mInitialMotionX + mTouchSlop : mInitialMotionX - mTouchSlop;
                mLastMotionY = y;
                setScrollingCacheEnabled(true);
            } else if (yDiff > mTouchSlop) {
                // The finger has moved enough in the vertical
                // direction to be counted as a drag...  abort
                // any attempt to drag horizontally, to work correctly
                // with children that have scrolling containers.
                LOGV("Starting unable to drag! yDiff=%f mTouchSlop=%d",yDiff,mTouchSlop);
                mIsUnableToDrag = true;
            }
            if (mIsBeingDragged) {
                // Scroll to follow the motion event
                if (performDrag(x)) postInvalidateOnAnimation();
            }
            break;
        }
    
    case MotionEvent::ACTION_DOWN: {
            /* Remember location of down touch. ACTION_DOWN always refers to pointer index 0. */
            mLastMotionX = mInitialMotionX = ev.getX();
            mLastMotionY = mInitialMotionY = ev.getY();
            mActivePointerId = ev.getPointerId(0);
            mIsUnableToDrag = false;

            mIsScrollStarted = true; 
            mScroller->computeScrollOffset();
            if (mScrollState == SCROLL_STATE_SETTLING
                    && std::abs(mScroller->getFinalX() - mScroller->getCurrX()) > mCloseEnough) {
                // Let the user 'catch' the pager as it animates.
                mScroller->abortAnimation();
                mPopulatePending = false;
                populate();
                mIsBeingDragged = true;
                requestParentDisallowInterceptTouchEvent(true);
                setScrollState(SCROLL_STATE_DRAGGING);
            } else {
                completeScroll(false);
                mIsBeingDragged = false;
            }
            break;
        }
    
    case MotionEvent::ACTION_POINTER_UP:
         onSecondaryPointerUp(ev);
         break;
    }
    
    if (mVelocityTracker == nullptr) {
        mVelocityTracker = VelocityTracker::obtain();
    }
    mVelocityTracker->addMovement(ev);
    
    /* The only time we want to intercept motion events is if we are in the drag mode.*/
    return mIsBeingDragged;
}

bool ViewPager::onTouchEvent(MotionEvent& ev){
    if (mFakeDragging) {
        // A fake drag is in progress already, ignore this real one
        // but still eat the touch events.
        // (It is likely that the user is multi-touching the screen.)
        return true;
    }
    if (ev.getAction() == MotionEvent::ACTION_DOWN && ev.getEdgeFlags() != 0) {
        // Don't handle edge touches immediately -- they may actually belong to one of our
        // descendants.
        return false;
    }
    
    if (mAdapter == nullptr || mAdapter->getCount() == 0) {
        // Nothing to present or scroll; nothing to touch.
        return false;
    }
    
    if (mVelocityTracker == nullptr) {
        mVelocityTracker = VelocityTracker::obtain();
    }
    mVelocityTracker->addMovement(ev);
    
    const int action = ev.getAction();
    bool needsInvalidate = false;
    
    switch (action & MotionEvent::ACTION_MASK) {
    case MotionEvent::ACTION_DOWN: 
         mScroller->abortAnimation();
         mPopulatePending = false;
         populate();
	 
         // Remember where the motion event started
         mLastMotionX = mInitialMotionX = ev.getX();
         mLastMotionY = mInitialMotionY = ev.getY();
         mActivePointerId = ev.getPointerId(0);
         break;
    case MotionEvent::ACTION_MOVE:
         if (!mIsBeingDragged) {
             int pointerIndex = ev.findPointerIndex(mActivePointerId);
             if(pointerIndex==-1){
                  needsInvalidate = resetTouch();
                  break;
             }
             float x = ev.getX(pointerIndex);
             float xDiff = std::abs(x - mLastMotionX);
             float y = ev.getY(pointerIndex);
             float yDiff = std::abs(y - mLastMotionY);

             LOGV("Moved x to %f,%f diff=%f,%f",x,y,xDiff,yDiff);
             if (xDiff > mTouchSlop && xDiff > yDiff) {
                 LOGV("Starting drag!");
                 mIsBeingDragged = true;
                 requestParentDisallowInterceptTouchEvent(true);
                 mLastMotionX = x - mInitialMotionX > 0 ? mInitialMotionX + mTouchSlop :
                         mInitialMotionX - mTouchSlop;
                 mLastMotionY = y;
                 setScrollState(SCROLL_STATE_DRAGGING);
                 setScrollingCacheEnabled(true);
	 
                 // Disallow Parent Intercept, just in case
                 ViewGroup* parent = getParent();
                 if (parent != nullptr) {
                     parent->requestDisallowInterceptTouchEvent(true);
                 }
             }
         }
         // Not else! Note that mIsBeingDragged can be set above.
         if (mIsBeingDragged) {
             // Scroll to follow the motion event
             int activePointerIndex = ev.findPointerIndex(mActivePointerId);
             float x = ev.getX(activePointerIndex);
             needsInvalidate |= performDrag(x);
         }
         break;
    case MotionEvent::ACTION_UP:
         if (mIsBeingDragged) {
             mVelocityTracker->computeCurrentVelocity(1000, mMaximumVelocity);
             int initialVelocity = (int) mVelocityTracker->getXVelocity(mActivePointerId);
             
             mPopulatePending = true;

             const int width = getClientWidth();
             const int scrollX = getScrollX();
             const ItemInfo* ii = infoForCurrentScrollPosition();
             const float marginOffset = (float) mPageMargin / width;
             const int currentPage = ii->position;
             const float pageOffset= (((float) scrollX / width) - ii->offset)
                            / (ii->widthFactor + marginOffset);

             const int activePointerIndex = ev.findPointerIndex(mActivePointerId);
             float x = ev.getX(activePointerIndex);
             int totalDelta = (int) (x - mInitialMotionX);
             int nextPage = determineTargetPage(currentPage, pageOffset, initialVelocity, totalDelta);
             setCurrentItemInternal(nextPage, true, true, initialVelocity);
             needsInvalidate = resetTouch();
         }
         break;
    case MotionEvent::ACTION_CANCEL:
          if (mIsBeingDragged) {
              scrollToItem(mCurItem, true, 0, false);
              needsInvalidate = resetTouch();
          }
          break;
    case MotionEvent::ACTION_POINTER_DOWN: {
            int index = ev.getActionIndex();
            float x = ev.getX(index);
            mLastMotionX = x;
            mActivePointerId = ev.getPointerId(index);
            break;
        }
    case MotionEvent::ACTION_POINTER_UP:
         onSecondaryPointerUp(ev);
         mLastMotionX = ev.getX(ev.findPointerIndex(mActivePointerId));
         break;
    }
    if (needsInvalidate)postInvalidateOnAnimation();
    return true;
}

bool ViewPager::resetTouch() {
    bool needsInvalidate;
    mActivePointerId = INVALID_POINTER;
    endDrag();
    mLeftEdge->onRelease();
    mRightEdge->onRelease();
    needsInvalidate = mLeftEdge->isFinished() || mRightEdge->isFinished();
    return needsInvalidate;
}

void ViewPager::requestParentDisallowInterceptTouchEvent(bool disallowIntercept){
     ViewGroup* parent = getParent();
     if (parent != nullptr) {
         parent->requestDisallowInterceptTouchEvent(disallowIntercept);
     }
}

bool ViewPager::performDrag(float x){
    bool needsInvalidate = false;

    float deltaX = mLastMotionX - x;
    mLastMotionX = x;

    float oldScrollX = getScrollX();
    float scrollX = oldScrollX + deltaX;
    int width = getClientWidth();

    float leftBound = width * mFirstOffset;
    float rightBound = width * mLastOffset;
    bool leftAbsolute = true;
    bool rightAbsolute = true;

    ItemInfo* firstItem = mItems.at(0);
    ItemInfo* lastItem = mItems.at(mItems.size() - 1);
    if (firstItem->position != 0) {
        leftAbsolute = false;
        leftBound = firstItem->offset * width;
    }
    if (lastItem->position != mAdapter->getCount() - 1) {
        rightAbsolute = false;
        rightBound = lastItem->offset * width;
    }

    if (scrollX < leftBound) {
        if (leftAbsolute) {
            const float over = leftBound - scrollX;
            mLeftEdge->onPull(std::abs(over) / width);
            needsInvalidate = true;
        }
        scrollX = leftBound;
    } else if (scrollX > rightBound) {
        if (rightAbsolute) {
            const float over = scrollX - rightBound;
            mRightEdge->onPull(std::abs(over) / width);
            needsInvalidate = true;
        }
        scrollX = rightBound;
    }
    // Don't lose the rounded component
    mLastMotionX += scrollX - (int) scrollX;
    scrollTo((int) scrollX, getScrollY());
    pageScrolled((int) scrollX);

    return needsInvalidate;
}

ViewPager::ItemInfo* ViewPager::infoForCurrentScrollPosition() {
    int width = getClientWidth();
    float scrollOffset = width > 0 ? (float) getScrollX() / width : 0;
    float marginOffset = width > 0 ? (float) mPageMargin / width : 0;
    int lastPos = -1;
    float lastOffset = 0.f;
    float lastWidth = 0.f;
    bool first = true;

    ItemInfo* lastItem = nullptr;
    for (int i = 0; i < mItems.size(); i++) {
        ItemInfo* ii = mItems.at(i);
        float offset;
        if (!first && ii->position != lastPos + 1) {
            // Create a synthetic item for a missing page.
            ii = &mTempItem;
            ii->offset = lastOffset + lastWidth + marginOffset;
            ii->position = lastPos + 1;
            ii->widthFactor = mAdapter->getPageWidth(ii->position);
            i--;
        }
        offset = ii->offset;

        float leftBound = offset;
        float rightBound = offset + ii->widthFactor + marginOffset;
        if (first || scrollOffset >= leftBound) {
            if (scrollOffset < rightBound || i == mItems.size() - 1) {
                return ii;
            }
        } else {
            return lastItem;
        }
        first = false;
        lastPos = ii->position;
        lastOffset = offset;
        lastWidth = ii->widthFactor;
        lastItem = ii;
    }

    return lastItem;
}

int ViewPager::determineTargetPage(int currentPage, float pageOffset, int velocity, int deltaX){
    int targetPage;
    if (std::abs(deltaX) > mFlingDistance && std::abs(velocity) > mMinimumVelocity) {
        targetPage = currentPage - (velocity < 0 ? mLeftIncr : 0);
    } else {
        float truncator = currentPage >= mCurItem ? 0.4f : 0.6f;
        targetPage = currentPage + (int) (pageOffset + truncator);
    }

    if (mItems.size() > 0) {
        ItemInfo* firstItem = mItems.at(0);
        ItemInfo* lastItem = mItems.at(mItems.size() - 1);

        // Only let the user target pages we have items for
        targetPage = MathUtils::constrain(targetPage, firstItem->position, lastItem->position);
    }
    return targetPage;
}

void ViewPager::draw(Canvas& canvas){
     ViewGroup::draw(canvas);
     bool needsInvalidate = false;

     int overScrollMode = getOverScrollMode();
     if (overScrollMode == View::OVER_SCROLL_ALWAYS
           || (overScrollMode == View::OVER_SCROLL_IF_CONTENT_SCROLLS
                    && mAdapter != nullptr && mAdapter->getCount() > 1)) {
        if (!mLeftEdge->isFinished()) {
            int height = getHeight() - getPaddingTop() - getPaddingBottom();
            int width = getWidth();

            canvas.save();
            canvas.rotate_degrees(270);
            canvas.translate(-height + getPaddingTop(), mFirstOffset * width);
            mLeftEdge->setSize(height, width);
            needsInvalidate |= mLeftEdge->draw(canvas);
            canvas.restore();
        }
        if (!mRightEdge->isFinished()) {
            int width = getWidth();
            int height = getHeight() - getPaddingTop() - getPaddingBottom();

            canvas.save();
            canvas.rotate_degrees(90);
            canvas.translate(-getPaddingTop(), -(mLastOffset + 1) * width);
            mRightEdge->setSize(height, width);
            needsInvalidate |= mRightEdge->draw(canvas);
            canvas.restore();
        }
    } else {
        mLeftEdge->finish();
        mRightEdge->finish();
    }

    if (needsInvalidate) {
        // Keep animating
        postInvalidateOnAnimation();
    }
}

void ViewPager::onDraw(Canvas& canvas) {
    ViewGroup::onDraw(canvas);

    // Draw the margin drawable between pages if needed.
    if (mPageMargin > 0 && mMarginDrawable && mItems.size() > 0 && mAdapter) {
        int scrollX = getScrollX();
        int width = getWidth();

        float marginOffset = (float) mPageMargin / width;
        int itemIndex = 0;
        ItemInfo* ii = mItems.at(0);
        float offset = ii->offset;

        const int itemCount = mItems.size();
        const int firstPos = ii->position;
        const int lastPos = mItems.at(itemCount - 1)->position;
        for (int pos = firstPos; pos < lastPos; pos++) {
            while (pos > ii->position && itemIndex < itemCount) {
                ii = mItems.at(++itemIndex);
            }

            float itemOffset;
            float widthFactor;
            if (pos == ii->position) {
                itemOffset = ii->offset;
                widthFactor= ii->widthFactor;
            } else {
                itemOffset = offset;
                widthFactor = mAdapter->getPageWidth(pos);
            }

            float left;
            float scaledOffset = itemOffset * width;
            if (isLayoutRtl()) {
                left = MAX_SCROLL_X - scaledOffset;
            } else {
                left = scaledOffset + widthFactor * width;
            }

            offset = itemOffset + widthFactor + marginOffset;

            if (left + mPageMargin > scrollX) {
                mMarginDrawable->setBounds((int) left, mTopPageBounds,
                     (int) (mPageMargin + 0.5f), mBottomPageBounds);
                mMarginDrawable->draw(canvas);
            }

            if (left > scrollX + width) {
                break; // No more visible, no sense in continuing
            }
        }
    }
}

bool ViewPager::beginFakeDrag(){
    if (mIsBeingDragged) {
        return false;
    }
    mFakeDragging = true;
    setScrollState(SCROLL_STATE_DRAGGING);
    mInitialMotionX = mLastMotionX = 0;
    if (mVelocityTracker == nullptr) {
        mVelocityTracker = VelocityTracker::obtain();
    } else {
        mVelocityTracker->clear();
    }
    const auto time = SystemClock::uptimeMillis();
    MotionEvent* ev = MotionEvent::obtain(time, time, MotionEvent::ACTION_DOWN, 0, 0, 0);
    mVelocityTracker->addMovement(*ev);
    ev->recycle();
    mFakeDragBeginTime = time;
    return true;
}

void ViewPager::endFakeDrag(){
    LOGE_IF(!mFakeDragging,"No fake drag in progress. Call beginFakeDrag first.");

    if (mAdapter != nullptr) {
        mVelocityTracker->computeCurrentVelocity(1000, mMaximumVelocity);
        int initialVelocity = (int) mVelocityTracker->getXVelocity(mActivePointerId);
        mPopulatePending = true;
        const int width = getClientWidth();
        const int scrollX = getScrollX();
        const ItemInfo* ii = infoForCurrentScrollPosition();
        const int currentPage = ii->position;
        const float pageOffset = (((float) scrollX / width) - ii->offset) / ii->widthFactor;
        const int totalDelta = (int) (mLastMotionX - mInitialMotionX);
        int nextPage = determineTargetPage(currentPage, pageOffset, initialVelocity,
                totalDelta);
        setCurrentItemInternal(nextPage, true, true, initialVelocity);
    }
    endDrag();

    mFakeDragging = false;
}

void ViewPager::fakeDragBy(float xOffset){
    if (!mFakeDragging) {
        LOGE("No fake drag in progress. Call beginFakeDrag first.");
    }

    if (mAdapter == nullptr)  {
        return;
    }

    mLastMotionX += xOffset;

    float oldScrollX = getScrollX();
    float scrollX = oldScrollX - xOffset;
    const int width = getClientWidth();

    float leftBound = width * mFirstOffset;
    float rightBound = width * mLastOffset;

    const ItemInfo* firstItem = mItems.at(0);
    const ItemInfo* lastItem = mItems.at(mItems.size() - 1);
    if (firstItem->position != 0) {
        leftBound = firstItem->offset * width;
    }
    if (lastItem->position != mAdapter->getCount() - 1) {
        rightBound = lastItem->offset * width;
    }

    if (scrollX < leftBound) {
        scrollX = leftBound;
    } else if (scrollX > rightBound) {
        scrollX = rightBound;
    }
    // Don't lose the rounded component
    mLastMotionX += scrollX - (int) scrollX;
    scrollTo((int) scrollX, getScrollY());
    pageScrolled((int) scrollX);

    // Synthesize an event for the VelocityTracker.
    const auto time = SystemClock::uptimeMillis();
    MotionEvent* ev = MotionEvent::obtain(mFakeDragBeginTime, time, MotionEvent::ACTION_MOVE,
            mLastMotionX, 0, 0);
    mVelocityTracker->addMovement(*ev);
    ev->recycle();
}

bool ViewPager::isFakeDragging()const{
    return mFakeDragging;
}

void ViewPager::onSecondaryPointerUp(MotionEvent&ev){
    int pointerIndex = ev.getActionIndex();
    int pointerId = ev.getPointerId(pointerIndex);
    if (pointerId == mActivePointerId) {
        // This was our active pointer going up. Choose a new
        // active pointer and adjust accordingly.
        const int newPointerIndex = pointerIndex == 0 ? 1 : 0;
        mLastMotionX = ev.getX(newPointerIndex);
        mActivePointerId = ev.getPointerId(newPointerIndex);
        if (mVelocityTracker != nullptr) {
            mVelocityTracker->clear();
        }
    }
}

void ViewPager::endDrag() {
    mIsBeingDragged = false;
    mIsUnableToDrag = false;

    if (mVelocityTracker != nullptr) {
        mVelocityTracker->recycle();
        mVelocityTracker = nullptr;
    }
}

void ViewPager::setScrollingCacheEnabled(bool bEnabled){
    if (mScrollingCacheEnabled != bEnabled) {
        mScrollingCacheEnabled = bEnabled;
        if (USE_CACHE) {
            int size = getChildCount();
            for (int i = 0; i < size; ++i) {
                View* child = getChildAt(i);
                if (child->getVisibility() != GONE) {
                    child->setDrawingCacheEnabled(bEnabled);
                }
            }
        }
    }
}

bool ViewPager::canScrollHorizontally(int direction) {
    if (mAdapter == nullptr) return false;

    int width = getClientWidth();
    int scrollX = getScrollX();
    if (direction < 0) {
        return (scrollX > (int) (width * mFirstOffset));
    } else if (direction > 0) {
        return (scrollX < (int) (width * mLastOffset));
    } else {
        return false;
    }
}

bool ViewPager::canScroll(View* v, bool checkV, int dx, int x, int y) {
    if (dynamic_cast<ViewGroup*>(v)) {
        ViewGroup* group = (ViewGroup*) v;
        const int scrollX = v->getScrollX();
        const int scrollY = v->getScrollY();
        const int count = group->getChildCount();
        // Count backwards - let topmost views consume scroll distance first.
        for (int i = count - 1; i >= 0; i--) {
            // TODO: Add support for transformed views.
            View* child = group->getChildAt(i);
            if (x + scrollX >= child->getLeft() && x + scrollX < child->getRight()
                    && y + scrollY >= child->getTop() && y + scrollY < child->getBottom()
                    && canScroll(child, true, dx, x + scrollX - child->getLeft(),
                            y + scrollY - child->getTop())) {
                return true;
            }
        }
    }
    return checkV && v->canScrollHorizontally(-dx);
}

bool ViewPager::dispatchKeyEvent(KeyEvent& event){
    return ViewGroup::dispatchKeyEvent(event) || executeKeyEvent(event);
}

bool  ViewPager::executeKeyEvent(KeyEvent& event){
    bool handled = false;
    if (event.getAction() == KeyEvent::ACTION_DOWN) {
        switch (event.getKeyCode()) {
        case KeyEvent::KEYCODE_DPAD_LEFT:
            if (event.hasModifiers(KeyEvent::META_ALT_ON)) {
                handled = pageLeft();
            } else {
                handled = arrowScroll(FOCUS_LEFT);
            }
            break;
        case KeyEvent::KEYCODE_DPAD_RIGHT:
            if (event.hasModifiers(KeyEvent::META_ALT_ON)) {
                handled = pageRight();
            } else {
                handled = arrowScroll(FOCUS_RIGHT);
            }
            break;
        case KeyEvent::KEYCODE_TAB:
            if (event.hasNoModifiers()) {
                handled = arrowScroll(FOCUS_FORWARD);
            } else if (event.hasModifiers(KeyEvent::META_SHIFT_ON)) {
                handled = arrowScroll(FOCUS_BACKWARD);
            }
            break;
        }
    }
    return handled;
}

bool ViewPager::arrowScroll(int direction){
    View* currentFocused = findFocus();
    if (currentFocused == this) {
        currentFocused = nullptr;
    } else if (currentFocused != nullptr) {
        bool isChild = false;
        for (ViewGroup* parent = currentFocused->getParent(); /*parent instanceof ViewGroup*/;
                parent = parent->getParent()) {
            if (parent == this) {
                isChild = true;
                break;
            }
        }
        if (!isChild) {
            // This would cause the focus search down below to fail in fun ways.
            std::ostringstream sb;
            sb<<typeid(currentFocused).name();
            for (ViewGroup* parent = currentFocused->getParent(); parent;parent = parent->getParent()) {
                sb<<" => "<<typeid(parent).name();
            }
            LOGD("arrowScroll tried to find focus based on non-child "
                  "current focused view ",sb.str().c_str());
            currentFocused = nullptr;
        }
    }

    bool handled = false;

    View* nextFocused = FocusFinder::getInstance().findNextFocus(this, currentFocused,  direction);
    if (nextFocused != nullptr && nextFocused != currentFocused) {
        Rect mTempRect;
        if (direction == View::FOCUS_LEFT) {
            // If there is nothing to the left, or this is causing us to
            // jump to the right, then what we really want to do is page left.
            int nextLeft = getChildRectInPagerCoordinates(mTempRect, nextFocused).left;
            int currLeft = getChildRectInPagerCoordinates(mTempRect, currentFocused).left;
            if (currentFocused != nullptr && nextLeft >= currLeft) {
                handled = pageLeft();
            } else {
                handled = nextFocused->requestFocus();
            }
        } else if (direction == View::FOCUS_RIGHT) {
            // If there is nothing to the right, or this is causing us to
            // jump to the left, then what we really want to do is page right.
            int nextLeft = getChildRectInPagerCoordinates(mTempRect, nextFocused).left;
            int currLeft = getChildRectInPagerCoordinates(mTempRect, currentFocused).left;
            if (currentFocused != nullptr && nextLeft <= currLeft) {
                handled = pageRight();
            } else {
                handled = nextFocused->requestFocus();
            }
        }
    } else if (direction == FOCUS_LEFT || direction == FOCUS_BACKWARD) {
        // Trying to move left and nothing there; try to page.
        handled = pageLeft();
    } else if (direction == FOCUS_RIGHT || direction == FOCUS_FORWARD) {
        // Trying to move right and nothing there; try to page.
        handled = pageRight();
    }
    if (handled) {
        playSoundEffect(SoundEffectConstants::getContantForFocusDirection(direction));
    }
    return handled;
}

Rect ViewPager::getChildRectInPagerCoordinates(Rect& outRect, View* child){
    if (child == nullptr) {
        outRect.set(0, 0, 0, 0);
        return outRect;
    }
    outRect.left  = child->getLeft();
    outRect.width = child->getWidth();;
    outRect.top   = child->getTop();
    outRect.height= child->getHeight();

    ViewGroup* parent = child->getParent();
    while (/*parent instanceof ViewGroup &&*/ parent != this) {
        ViewGroup* group = (ViewGroup*) parent;
        outRect.left += group->getLeft();
        outRect.width += group->getWidth();
        outRect.top+= group->getTop();
        outRect.height += group->getHeight();

        parent = group->getParent();
    }
    return outRect; 
}

bool ViewPager::pageLeft() {
    if (mCurItem > 0) {
        setCurrentItem(mCurItem + mLeftIncr, true);
        return true;
    }
    return false;
}

bool ViewPager::pageRight() {
    if (mAdapter != nullptr && mCurItem < (mAdapter->getCount() - 1)) {
        setCurrentItem(mCurItem - mLeftIncr, true);
        return true;
    }
    return false;
}

void ViewPager::onRtlPropertiesChanged(int layoutDirection){
    ViewGroup::onRtlPropertiesChanged(layoutDirection);
    mLeftIncr = (layoutDirection == LAYOUT_DIRECTION_LTR)?-1:1;
}

void ViewPager::addFocusables(std::vector<View*>& views, int direction, int focusableMode){
    int focusableCount = views.size();

    int descendantFocusability = getDescendantFocusability();

    if (descendantFocusability != FOCUS_BLOCK_DESCENDANTS) {
        for (auto child:mChildren){
            if (child->getVisibility() == VISIBLE) {
                ItemInfo* ii = infoForChild(child);
                if (ii && ii->position == mCurItem) {
                    child->addFocusables(views, direction, focusableMode);
                }
            }
        }
    }

    // we add ourselves (if focusable) in all cases except for when we are
    // FOCUS_AFTER_DESCENDANTS and there are some descendants focusable.  this is
    // to avoid the focus search finding layouts when a more precise search
    // among the focusable children would be more interesting.
    if (descendantFocusability != FOCUS_AFTER_DESCENDANTS
            || (focusableCount == views.size())) { // No focusable descendants
        // Note that we can't call the superclass here, because it will
        // add all views in.  So we need to do the same thing View does.
        if (!isFocusable()) {
            return;
        }
        if ((focusableMode & FOCUSABLES_TOUCH_MODE) == FOCUSABLES_TOUCH_MODE
               && isInTouchMode() && !isFocusableInTouchMode()) {
            return;
        }
        views.push_back(this);
    }
}

void ViewPager::addTouchables(std::vector<View*>& views){
    for (auto child:mChildren){
        if (child->getVisibility() == VISIBLE) {
            ItemInfo* ii = infoForChild(child);
            if (ii  && ii->position == mCurItem) {
                child->addTouchables(views);
            }
        }
    }
}

bool ViewPager::onRequestFocusInDescendants(int direction,Rect* previouslyFocusedRect){
    int index;
    int increment;
    int end;
    int count = getChildCount();
    if ((direction & FOCUS_FORWARD) != 0) {
        index = 0;
        increment = 1;
        end = count;
    } else {
        index = count - 1;
        increment = -1;
        end = -1;
    }
    for (int i = index; i != end; i += increment) {
        View* child = getChildAt(i);
        if (child->getVisibility() == VISIBLE) {
            ItemInfo* ii = infoForChild(child);
            if (ii  && ii->position == mCurItem) {
                if (child->requestFocus(direction, previouslyFocusedRect)) {
                    return true;
                }
            }
        }
    }
    return false;
}

ViewPager::LayoutParams* ViewPager::generateDefaultLayoutParams(){
    return new LayoutParams();
}

ViewPager::LayoutParams* ViewPager::generateLayoutParams(ViewGroup::LayoutParams* p){
    return generateDefaultLayoutParams();
}

bool ViewPager::checkLayoutParams(ViewGroup::LayoutParams* p){
    return dynamic_cast<LayoutParams*>(p) && ViewGroup::checkLayoutParams(p);
}

ViewPager::LayoutParams* ViewPager::generateLayoutParams(const AttributeSet& attrs){
    return new LayoutParams(getContext(),attrs);
}

bool ViewPager::canScroll() {
    return mAdapter && mAdapter->getCount() > 1;
}

ViewPager::LayoutParams::LayoutParams()
  :ViewGroup::LayoutParams(MATCH_PARENT, MATCH_PARENT){
    isDecor = true;
    gravity = Gravity::NO_GRAVITY;
    widthFactor = .0f;//.0f wil ask adapter for this value
    needsMeasure= true;
    position   =-1;
    childIndex =-1;
}

ViewPager::LayoutParams::LayoutParams(Context*ctx,const AttributeSet&atts)
  :ViewGroup::LayoutParams(ctx,atts){
    isDecor = true;
    gravity = Gravity::TOP;
    widthFactor = .0f;
    needsMeasure= true;
    position  = -1;
    childIndex= -1;
}

}//endof namespace
