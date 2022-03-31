#include <widget/abslistview.h>
#include <widget/checkable.h>
#include <widget/recyclebin.h>
#include <widget/fastscroller.h>
#include <cdtypes.h>
#include <cdlog.h>


namespace cdroid {
#define PROFILE_SCROLLING false

void AbsListView::FLY_start(int initialVelocity) {
    int initialY = initialVelocity < 0 ? INT_MAX : 0;
    mLastFlingY = initialY;
    mScroller->setInterpolator(nullptr);
    mScroller->fling(0, initialY, 0, initialVelocity, 0, INT_MAX, 0, INT_MAX);

    mTouchMode = TOUCH_MODE_FLING;
    mSuppressIdleStateChangeCall = false;
    postOnAnimation(mFlingRunnable);
    if (PROFILE_FLINGING) {
        if (!mFlingProfilingStarted) {
            mFlingProfilingStarted = true;
        }
    }

    /*if (mFlingStrictSpan == nullptr) {
         mFlingStrictSpan = StrictMode.enterCriticalSpan("AbsListView-fling");
    }*/
}

void AbsListView::FLY_startSpringback() {
    mSuppressIdleStateChangeCall = false;
    if (mScroller->springBack(0, mScrollY, 0, 0, 0, 0)) {
        mTouchMode = TOUCH_MODE_OVERFLING;
        invalidate();
        postOnAnimation(mFlingRunnable);
    } else {
        mTouchMode = TOUCH_MODE_REST;
        reportScrollStateChange(OnScrollListener::SCROLL_STATE_IDLE);
    }
}

void AbsListView::FLY_startOverfling(int initialVelocity) {
    mScroller->setInterpolator(nullptr);
    mScroller->fling(0, mScrollY, 0, initialVelocity, 0, 0,INT_MIN,INT_MAX, 0, getHeight());
    mTouchMode = TOUCH_MODE_OVERFLING;
    mSuppressIdleStateChangeCall = false;
    invalidate();
    postOnAnimation(mFlingRunnable);
}

void AbsListView::FLY_edgeReached(int delta) {
    mScroller->notifyVerticalEdgeReached(mScrollY, 0, mOverflingDistance);
    int overscrollMode = getOverScrollMode();
    if ( (overscrollMode == OVER_SCROLL_ALWAYS) || (overscrollMode == OVER_SCROLL_IF_CONTENT_SCROLLS && !contentFits())) {
        mTouchMode = TOUCH_MODE_OVERFLING;
        int vel = (int) mScroller->getCurrVelocity();
        if (delta > 0) mEdgeGlowTop->onAbsorb(vel);
        else  mEdgeGlowBottom->onAbsorb(vel);
    } else {
        mTouchMode = TOUCH_MODE_REST;
        if (mPositionScroller != nullptr) mPositionScroller->stop();
    }
    invalidate();
    postOnAnimation(mFlingRunnable);
}

void AbsListView::FLY_startScroll(int distance, int duration, bool linear, bool suppressEndFlingStateChangeCall) {
    int initialY = distance < 0 ? INT_MAX : 0;
    mLastFlingY = initialY;
    mScroller->setInterpolator(linear ? new LinearInterpolator() : nullptr);
    mScroller->startScroll(0, initialY, 0, distance, duration);
    mTouchMode = TOUCH_MODE_FLING;
    mSuppressIdleStateChangeCall = suppressEndFlingStateChangeCall;
    postOnAnimation(mFlingRunnable);
}

void AbsListView::FLY_endFling() {
    mTouchMode = TOUCH_MODE_REST;

    removeCallbacks(mFlingRunnable);
    removeCallbacks(mCheckFlywheel);

    if (!mSuppressIdleStateChangeCall) reportScrollStateChange(OnScrollListener::SCROLL_STATE_IDLE);

    clearScrollingCache();
    mScroller->abortAnimation();

    /*if (mFlingStrictSpan != nullptr) {
        mFlingStrictSpan.finish();
        mFlingStrictSpan = nullptr;
    }*/
}

void AbsListView::FLY_wheelTouch() {
    postDelayed(mCheckFlywheel, FLYWHEEL_TIMEOUT);
}

void AbsListView::FLY_Proc(){
    switch (mTouchMode) {
    default:  FLY_endFling(); return ;
    case TOUCH_MODE_SCROLL://3
         if (mScroller->isFinished()) return ;
           // Fall through
    case TOUCH_MODE_FLING: {//4
         if (mDataChanged) layoutChildren();

         if (mItemCount == 0 || getChildCount() == 0) {
             FLY_endFling();
             return ;
         }

         bool more = mScroller->computeScrollOffset();
         int y = mScroller->getCurrY();

         // Flip sign to convert finger direction to list items direction
         // (e.g. finger moving down means list is moving towards the top)
         int delta = mLastFlingY - y;

         // Pretend that each frame of a fling scroll is a touch scroll
         if (delta > 0) {
             // List is moving towards the top. Use first view as mMotionPosition
             mMotionPosition = mFirstPosition;
             View* firstView = getChildAt(0);
             mMotionViewOriginalTop = firstView->getTop();
             // Don't fling more than 1 screen
             delta = std::min(getHeight() - mPaddingBottom - mPaddingTop - 1, delta);
         } else {
             // List is moving towards the bottom. Use last view as mMotionPosition
             int offsetToLast = getChildCount() - 1;
             mMotionPosition = mFirstPosition + offsetToLast;
             View* lastView = getChildAt(offsetToLast);
             mMotionViewOriginalTop = lastView->getTop();
             // Don't fling more than 1 screen
             delta = std::max(-(getHeight() - mPaddingBottom - mPaddingTop - 1), delta);
        }

        // Check to see if we have bumped into the scroll limit
        View* motionView = getChildAt(mMotionPosition - mFirstPosition);
        int oldTop = 0;
        if (motionView != nullptr) {
            oldTop = motionView->getTop();
        }

        // Don't stop just because delta is zero (it could have been rounded)
        bool atEdge = trackMotionScroll(delta, delta);
        bool atEnd = atEdge && (delta != 0);
        if (atEnd) {
            if (motionView != nullptr) {
                // Tweak the scroll for how far we overshot
                int overshoot = -(delta - (motionView->getTop() - oldTop));
                overScrollBy(0, overshoot, 0, mScrollY, 0, 0, 0, mOverflingDistance, false);
            }
            if (more) {
                FLY_edgeReached(delta);
            }
            break;
        }

        if (more && !atEnd) {
            if (atEdge) invalidate();
            mLastFlingY = y;
            postOnAnimation(mFlingRunnable);
        } else {
            FLY_endFling();
            if (PROFILE_FLINGING) {
                if (mFlingProfilingStarted) {
                    mFlingProfilingStarted = false;
                }

                /*if (mFlingStrictSpan != nullptr) {
                    mFlingStrictSpan.finish();
                    mFlingStrictSpan = nullptr;
                }*/
            }
        }
        break;
    }
    case TOUCH_MODE_OVERFLING: {//6
         if (mScroller->computeScrollOffset()) {
            int scrollY = mScrollY;
            int currY = mScroller->getCurrY();
            int deltaY = currY - scrollY;
            if (overScrollBy(0, deltaY, 0, scrollY, 0, 0, 0, mOverflingDistance, false)) {
                bool crossDown = scrollY <= 0 && currY > 0;
                bool crossUp = scrollY >= 0 && currY < 0;
                if (crossDown || crossUp) {
                    int velocity = (int) mScroller->getCurrVelocity();
                    if (crossUp) velocity = -velocity;
                        // Don't flywheel from this; we're just continuing things.
                        mScroller->abortAnimation();
                        FLY_start(velocity);
                    } else {
                        FLY_startSpringback();
                    }
            } else {
                invalidate();
                postOnAnimation(mFlingRunnable);
            }
         } else {
             FLY_endFling();
         }
         break;
      }
   }
}

void AbsListView::FLY_CheckFlyWheelProc(){
    int activeId = mActivePointerId;
    VelocityTracker* vt = mVelocityTracker;
    OverScroller* scroller = mScroller;
    if (vt == nullptr || activeId == INVALID_POINTER) {
        return;
    }

    vt->computeCurrentVelocity(1000, mMaximumVelocity);
    float yvel = -vt->getYVelocity(activeId);

    if ( (std::abs(yvel) >= mMinimumVelocity) && scroller->isScrollingInDirection(0, yvel)) {
        // Keep the fling alive a little longer
        postDelayed(mCheckFlywheel, FLYWHEEL_TIMEOUT);
    } else {
        FLY_endFling();
        mTouchMode = TOUCH_MODE_SCROLL;
        reportScrollStateChange(OnScrollListener::SCROLL_STATE_TOUCH_SCROLL);
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////

AbsListView::AbsListView(int w,int h):AdapterView(w,h) {
    AttributeSet atts=mContext->obtainStyledAttributes("cdroid:attr/absListViewStyle");
    initAbsListView(atts);
}

AbsListView::AbsListView(Context*ctx,const AttributeSet&atts):AdapterView(ctx,atts) {
    initAbsListView(atts);
}

void AbsListView::initAbsListView(const AttributeSet&atts) {
    setClickable(true);
    setFocusableInTouchMode(true);
    setWillNotDraw(false);
    setAlwaysDrawnWithCacheEnabled(false);
    mDataSetObserver = nullptr;
    mScrollingCacheEnabled   = false;
    mFastScrollAlwaysVisible = false;
    setScrollingCacheEnabled(true);
    mListPadding.set(0,0,0,0);
    mVelocityTracker = nullptr;
    mPositionScroller= nullptr;
    mFastScroll = nullptr; 
    mStackFromBottom = false;
    mIsChildViewEnabled =false;
    mFiltered    = false;
    mIsDetaching = false;
    mPopupHidden = false;
    mTextFilterEnabled = false;
    mFastScrollEnabled = false;
    mSuppressIdleStateChangeCall =false;
    mVelocityScale = 1.0f;
    mLastScrollState = OnScrollListener::SCROLL_STATE_IDLE;
    mScrollConsumed[0]= mScrollConsumed[1] = 0;
    mScrollOffset[0] = mScrollOffset[1] = 0;
    mOnScrollListener.onScroll = nullptr;
    mOnScrollListener.onScrollStateChanged = nullptr;

    mScroller = new OverScroller(getContext());
    mEdgeGlowBottom = new EdgeEffect(mContext);
    mEdgeGlowTop = new EdgeEffect(mContext);

    mFlingRunnable = std::bind(&AbsListView::FLY_Proc,this);
    mCheckFlywheel = std::bind(&AbsListView::FLY_CheckFlyWheelProc,this);
    mSelector  = nullptr;
    mDirection = 0;
    mCacheColorHint = 0;
    mWidthMeasureSpec = 0;
    mResurrectToPosition = INVALID_POSITION;
    mLayoutMode = LAYOUT_FORCE_TOP;
    mTouchMode  = TOUCH_MODE_REST ;
    mDrawSelectorOnTop = true;
    mSmoothScrollbarEnabled = true;
    ViewConfiguration& configuration = ViewConfiguration::get(mContext);
    mTouchSlop = configuration.getScaledTouchSlop();
    mVerticalScrollFactor = configuration.getScaledVerticalScrollFactor();
    mMinimumVelocity = configuration.getScaledMinimumFlingVelocity();
    mMaximumVelocity = configuration.getScaledMaximumFlingVelocity();
    mOverscrollDistance = configuration.getScaledOverscrollDistance();
    mOverflingDistance  = configuration.getScaledOverflingDistance();
    mSelectionLeftPadding  = 0;
    mSelectionTopPadding   = 0;
    mSelectionRightPadding = 0;
    mSelectionBottomPadding= 0;
    mSelectorPosition = INVALID_POSITION;
    mSelectedTop = 0;
    mChoiceMode  = CHOICE_MODE_NONE;
    mLayoutMode  = LAYOUT_NORMAL;
    mActivePointerId = INVALID_POINTER;
    mTranscriptMode=TRANSCRIPT_MODE_DISABLED;
    mRecycler=new RecycleBin(this);
    mScrollUp = mScrollDown = nullptr;
    mCachingStarted = mCachingActive =false;
    mIsScrap[0]=mIsScrap[1]=0;
    mDensityScale=getContext()->getDisplayMetrics().density;

    mDrawSelectorOnTop = atts.getBoolean("drawSelectorOnTop",false);
    setStackFromBottom(atts.getBoolean("stackFromBottom",false));
    setScrollingCacheEnabled(atts.getBoolean("scrollingCache",true));
    setSmoothScrollbarEnabled(atts.getBoolean("smoothScrollbar",true));
    setChoiceMode(atts.getInt("choiceMode",std::map<const std::string,int>{
        {"none",CHOICE_MODE_NONE},
        {"singleChoice",CHOICE_MODE_SINGLE},
        {"multipleChoice",CHOICE_MODE_MULTIPLE}
    },CHOICE_MODE_NONE));
    setFastScrollEnabled(atts.getBoolean("fastScrollEnabled",false));
    setFastScrollStyle(atts.getString("fastScrollStyle"));
    setFastScrollAlwaysVisible(atts.getBoolean("fastScrollAlwaysVisible",false));
}

AbsListView::~AbsListView(){
    if(mVelocityTracker){
        mVelocityTracker->recycle();
        mVelocityTracker = nullptr;
    }
    delete mSelector;
    delete mFastScroll;
    delete mRecycler;
    delete mScroller;
    delete mEdgeGlowTop;
    delete mEdgeGlowBottom;
    delete mPositionScroller;
    delete mDataSetObserver;
}

void AbsListView::updateScrollIndicators(){
    if ( mScrollUp ) mScrollUp->setVisibility(canScrollUp() ? View::VISIBLE : View::INVISIBLE);
    if (mScrollDown) mScrollDown->setVisibility(canScrollDown() ? View::VISIBLE : View::INVISIBLE);
}

void AbsListView::setScrollIndicatorViews(View* up, View* down){
    mScrollUp = up;
    mScrollDown = down;
}

void AbsListView::setAdapter(Adapter*adapter) {
    if (adapter != nullptr) {
        mAdapterHasStableIds =adapter->hasStableIds();
    }
    clearChoices();
}

int AbsListView::getHeaderViewsCount()const {
    return 0;
}

int AbsListView::getFooterViewsCount()const {
    return 0;
}

int AbsListView::getHeightForPosition(int position) {
    int firstVisiblePosition = getFirstVisiblePosition();
    int childCount = getChildCount();
    int index = position - firstVisiblePosition;
    if (index >= 0 && index < childCount) {
        // Position is on-screen, use existing view.
        View* view = getChildAt(index);
        return view->getHeight();
    } else {
        // Position is off-screen, obtain & recycle view.
        View* view = obtainView(position, mIsScrap);
        view->measure(mWidthMeasureSpec, MeasureSpec::UNSPECIFIED);
        int height = view->getMeasuredHeight();
        mRecycler->addScrapView(view, position);
        return height;
    }
}

void AbsListView::setSmoothScrollbarEnabled(bool enable) {
    mSmoothScrollbarEnabled = enable;
}

bool AbsListView::isSmoothScrollbarEnabled()const {
    return mSmoothScrollbarEnabled;
}

void AbsListView::invokeOnItemScrollListener(){
    if (mFastScroll != nullptr) {
        mFastScroll->onScroll(mFirstPosition, getChildCount(), mItemCount);
    }
    if(mOnScrollListener.onScroll)
       mOnScrollListener.onScroll(*this,mFirstPosition,getChildCount(),mItemCount);
    onScrollChanged(0,0,0,0);
}

void AbsListView::setOnScrollListener(OnScrollListener l) {
    mOnScrollListener = l;
    invokeOnItemScrollListener();
}

void AbsListView::setScrollingCacheEnabled(bool enabled){
    if (mScrollingCacheEnabled && !enabled) {
        clearScrollingCache();
    }
    mScrollingCacheEnabled = enabled;
}

bool AbsListView::isScrollingCacheEnabled()const{
    return mScrollingCacheEnabled;
}

void AbsListView::setFastScrollerEnabledUiThread(bool enabled) {
    if (mFastScroll != nullptr) {
        mFastScroll->setEnabled(enabled);
    } else if (enabled) {
        mFastScroll = new FastScroller(this, mFastScrollStyle);
        mFastScroll->setEnabled(true);
    }

    resolvePadding();
    if (mFastScroll != nullptr) {
        mFastScroll->updateLayout();
    }
}

void AbsListView::setFastScrollerAlwaysVisibleUiThread(bool alwaysShow) {
     if (mFastScroll != nullptr) {
         mFastScroll->setAlwaysShow(alwaysShow);
     }
}

static bool isOwnerThread(){
   return true;
}

void AbsListView::setFastScrollEnabled(bool enabled){
    if (mFastScrollEnabled != enabled) {
        mFastScrollEnabled = enabled;
   
        if (isOwnerThread()) {
            setFastScrollerEnabledUiThread(enabled);
        } else {
            post([this,enabled](){
                setFastScrollerEnabledUiThread(enabled);
            });
        }
    }
}


void AbsListView::setFastScrollStyle(const std::string& styleResId){
    if (mFastScroll == nullptr) {
        mFastScrollStyle = styleResId;
    } else {
        mFastScroll->setStyle(styleResId);
    }
}

void AbsListView::setFastScrollAlwaysVisible(bool alwaysShow){
    if (mFastScrollAlwaysVisible != alwaysShow) {
        if (alwaysShow && !mFastScrollEnabled) {
            setFastScrollEnabled(true);
        }

        mFastScrollAlwaysVisible = alwaysShow;

        if (isOwnerThread()) {
            setFastScrollerAlwaysVisibleUiThread(alwaysShow);
        } else {
            post([this,alwaysShow](){
                setFastScrollerAlwaysVisibleUiThread(alwaysShow);
            });
        }
    }
}

bool AbsListView::isFastScrollAlwaysVisible()const{
    if (mFastScroll == nullptr) {
        return mFastScrollEnabled && mFastScrollAlwaysVisible;
    } else {
        return mFastScroll->isEnabled() && mFastScroll->isAlwaysShowEnabled();
    }
}

int AbsListView::getVerticalScrollbarWidth()const{
    if (mFastScroll && mFastScroll->isEnabled()) {
        return std::max(AdapterView::getVerticalScrollbarWidth(), mFastScroll->getWidth());
    }
    return AdapterView::getVerticalScrollbarWidth();
}

bool AbsListView::isFastScrollEnabled()const{
    if (mFastScroll == nullptr) {
        return mFastScrollEnabled;
    } else {
        return mFastScroll->isEnabled();
    }
}

View& AbsListView::setVerticalScrollbarPosition(int position) {
    AdapterView::setVerticalScrollbarPosition(position);
    if (mFastScroll) {
        mFastScroll->setScrollbarPosition(position);
    }
    return *this;
}

void AbsListView::setScrollBarStyle(int style) {
    AdapterView::setScrollBarStyle(style);
    if (mFastScroll != nullptr) {
        mFastScroll->setScrollBarStyle(style);
    }
}

int AbsListView::getListPaddingTop()const {
    return mListPadding.top;
}

int AbsListView::getListPaddingBottom()const {
    return mListPadding.height;
}

int AbsListView::getListPaddingLeft()const {
    return mListPadding.left;
}

int AbsListView::getListPaddingRight()const {
    return mListPadding.width;
}

void AbsListView::setTranscriptMode(int mode){
    mTranscriptMode=mode;
}

int AbsListView::getTranscriptMode()const{
    return mTranscriptMode;
}

int AbsListView::getCacheColorHint()const {
    return mCacheColorHint;
}

void AbsListView::setCacheColorHint(int color) {
    if (color != mCacheColorHint) {
        mCacheColorHint = color;
        int count = getChildCount();
        for (int i = 0; i < count; i++) {
            getChildAt(i)->setDrawingCacheBackgroundColor(color);
        }
        mRecycler->setCacheColorHint(color);
    }
}

ViewGroup::LayoutParams*AbsListView::generateDefaultLayoutParams()const {
    return new AbsListView::LayoutParams(LayoutParams::MATCH_PARENT,LayoutParams::WRAP_CONTENT, 0);
}

ViewGroup::LayoutParams*AbsListView::generateLayoutParams(const ViewGroup::LayoutParams* p)const {
    return new AbsListView::LayoutParams(*p);
}

ViewGroup::LayoutParams* AbsListView::generateLayoutParams(const AttributeSet& attrs)const {
    return new AbsListView::LayoutParams(getContext(), attrs);
}

bool AbsListView::checkLayoutParams(const ViewGroup::LayoutParams* p)const {
    return dynamic_cast<const AbsListView::LayoutParams*>(p)!=nullptr;
}

void AbsListView::setSelectionFromTop(int position, int y) {
    if (mAdapter == nullptr)return;

    if (!isInTouchMode()) {
        position = lookForSelectablePosition(position, true);
        if (position >= 0) {
            setNextSelectedPositionInt(position);
        }
    } else {
        mResurrectToPosition = position;
    }

    if (position >= 0) {
        mLayoutMode = LAYOUT_SPECIFIC;
        mSpecificTop = mListPadding.top + y;

        if (mNeedSync) {
            mSyncPosition = position;
            mSyncRowId = mAdapter->getItemId(position);
        }

        if (mPositionScroller) mPositionScroller->stop();
        requestLayout();
    }
}

int AbsListView::getCheckedItemCount()const {
    return mCheckedItemCount;
}

bool AbsListView::isItemChecked(int position)const {
    if (mChoiceMode != CHOICE_MODE_NONE) {
        return mCheckStates.get(position);
    }
    return false;
}

int AbsListView::getCheckedItemPosition()const {
    if (mChoiceMode == CHOICE_MODE_SINGLE && mCheckStates.size() == 1) {
        return mCheckStates.keyAt(0);
    }
    return INVALID_POSITION;
}

int AbsListView::getCheckedItemPositions(SparseBooleanArray&array){
    if (mChoiceMode != CHOICE_MODE_NONE) {
        return mCheckStates.size();
    }
    return 0;
}

int AbsListView::getCheckedItemIds(std::vector<long>&ids)const{
    if (mChoiceMode == CHOICE_MODE_NONE || mCheckedIdStates.size() == 0 || mAdapter == nullptr) {
        return 0;
    }
    const int count = mCheckedIdStates.size();
    for (int i = 0; i < count; i++) {
        ids.push_back(mCheckedIdStates.keyAt(i));
    }
    return ids.size();
}


void AbsListView::positionSelector(int position, View* sel, bool manageHotspot, float x, float y) {
    bool positionChanged = position != mSelectorPosition;
    if (position != INVALID_POSITION) {
        mSelectorPosition = position;
    }

    Rect&selectorRect = mSelectorRect;
    selectorRect.set(sel->getLeft(), sel->getTop(), sel->getWidth(), sel->getHeight());
    /*if (sel instanceof SelectionBoundsAdjuster) {
        ((SelectionBoundsAdjuster)sel).adjustListItemSelectionBounds(selectorRect);
    }*/

    // Adjust for selection padding.
    selectorRect.left -= mSelectionLeftPadding;
    selectorRect.top -= mSelectionTopPadding;
    selectorRect.width += mSelectionLeftPadding+mSelectionRightPadding;
    selectorRect.height+= mSelectionTopPadding+mSelectionBottomPadding;

    // Update the child enabled state prior to updating the selector.
    bool isChildViewEnabled = sel->isEnabled();
    if (mIsChildViewEnabled != isChildViewEnabled) {
        mIsChildViewEnabled = isChildViewEnabled;
    }

    // Update the selector drawable's state and position.
    Drawable* selector = mSelector;
    if (selector != nullptr) {
        if (positionChanged) {
            // Wipe out the current selector state so that we can start
            // over in the new position with a fresh state.
            selector->setVisible(false, false);
            selector->setState(StateSet::NOTHING);
        }
        selector->setBounds(selectorRect);
        if (positionChanged) {
            if (getVisibility() == VISIBLE) {
                selector->setVisible(true, false);
            }
            updateSelectorState();
        }
        if (manageHotspot) selector->setHotspot(x, y);
    }
}

void AbsListView::positionSelector(int position, View* sel) {
    positionSelector(position, sel, false, -1, -1);
}

void AbsListView::positionSelectorLikeTouch(int position, View* sel, float x, float y) {
    positionSelector(position, sel, true, x, y);
}

void AbsListView::positionSelectorLikeFocus(int position, View* sel) {
    if (mSelector != nullptr && mSelectorPosition != position && position != INVALID_POSITION) {
        Rect bounds = mSelectorRect;
        float x = (bounds.left+bounds.width)/2;//exactCenterX();
        float y = (bounds.top +bounds.height)/2;//exactCenterY();
        positionSelector(position, sel, true, x, y);
    } else {
        positionSelector(position, sel);
    }
}

void AbsListView::setItemChecked(int position, bool value) {
    if (mChoiceMode == CHOICE_MODE_NONE)  return;

    // Start selection mode if needed. We don't need to if we're unchecking something.
    if (value && mChoiceMode == CHOICE_MODE_MULTIPLE_MODAL /*&& mChoiceActionMode == nullptr*/) {
        if (mMultiChoiceModeCallback == nullptr /*|| !mMultiChoiceModeCallback.hasWrappedCallback()*/) {
            LOGE("AbsListView: attempted to start selection mode "
                    "for CHOICE_MODE_MULTIPLE_MODAL but no choice mode callback was "
                    "supplied. Call setMultiChoiceModeListener to set a callback.");
        }
        //mChoiceActionMode = startActionMode(mMultiChoiceModeCallback);
    }

    bool itemCheckChanged;
    if (mChoiceMode == CHOICE_MODE_MULTIPLE || mChoiceMode == CHOICE_MODE_MULTIPLE_MODAL) {
        bool oldValue = mCheckStates.get(position);
        mCheckStates.put(position,value);
        if (mCheckedIdStates.size() && mAdapter->hasStableIds()) {
            if (value) {
                mCheckedIdStates.put(mAdapter->getItemId(position), position);
            } else {
                mCheckedIdStates.remove(mAdapter->getItemId(position));
            }
        }
        itemCheckChanged = oldValue != value;
        if (itemCheckChanged) {
            if (value) {
                mCheckedItemCount++;
            } else {
                mCheckedItemCount--;
            }
        }
        if (mMultiChoiceModeCallback/*mChoiceActionMode != nullptr*/) {
            long id = mAdapter->getItemId(position);
            mMultiChoiceModeCallback(/*mChoiceActionMode*/ position, id, value);
            //mMultiChoiceModeCallback.onItemCheckedStateChanged
        }
    } else {
        bool updateIds = mCheckedIdStates.size() && mAdapter->hasStableIds();
        // Clear all values if we're checking something, or unchecking the currently selected item
        itemCheckChanged = isItemChecked(position) != value;
        if (value || isItemChecked(position)) {
            mCheckStates.clear();
            if (updateIds) {
                mCheckedIdStates.clear();
            }
        }
        // this may end up selecting the value we just cleared but this way
        // we ensure length of mCheckStates is 1, a fact getCheckedItemPosition relies on
        if (value) {
            mCheckStates.put(position, true);
            if (updateIds) {
                mCheckedIdStates.put(mAdapter->getItemId(position), position);
            }
            mCheckedItemCount = 1;
        } else if (mCheckStates.size() == 0 || !mCheckStates.valueAt(0)) {
            mCheckedItemCount = 0;
        }
    }

    // Do not generate a data change while we are in the layout phase or data has not changed
    if (!mInLayout && !mBlockLayoutRequests && itemCheckChanged) {
        mDataChanged = true;
        rememberSyncState();
        requestLayout();
    }
}

View* AbsListView::getSelectedView() {
    if (mItemCount > 0 && mSelectedPosition >= 0) {
        return getChildAt(mSelectedPosition - mFirstPosition);
    } else {
        return nullptr;
    }
}

int AbsListView::getChoiceMode()const {
    return mChoiceMode;
}

void AbsListView::setChoiceMode(int choiceMode) {
    mChoiceMode = choiceMode;
    /*if (mChoiceActionMode != null) {
        mChoiceActionMode.finish();
        mChoiceActionMode = null;
    }*/
    if (mChoiceMode != CHOICE_MODE_NONE) {
        /*if (mCheckStates == null) {
            mCheckStates = new SparseboolArray(0);
        }
        if (mCheckedIdStates == null && mAdapter != null && mAdapter.hasStableIds()) {
            mCheckedIdStates = new LongSparseArray<Integer>(0);
        }*/
        // Modal multi-choice mode only has choices when the mode is active. Clear them.
        if (mChoiceMode == CHOICE_MODE_MULTIPLE_MODAL) {
            clearChoices();
            setLongClickable(true);
        }
    }
}

void AbsListView::setMultiChoiceModeListener(MultiChoiceModeListener listener){
    mMultiChoiceModeCallback=listener;
}

void AbsListView::resetList() {
    removeAllViewsInLayout();
    mFirstPosition = 0;
    mDataChanged = false;
    mPositionScrollAfterLayout = nullptr;
    mNeedSync = false;
    //mPendingSync = null;
    mOldSelectedPosition = INVALID_POSITION;
    mOldSelectedRowId = INVALID_ROW_ID;
    setSelectedPositionInt(INVALID_POSITION);
    setNextSelectedPositionInt(INVALID_POSITION);
    mSelectedTop = 0;
    mSelectorPosition = INVALID_POSITION;
    mSelectorRect.setEmpty();
    invalidate();
}

void AbsListView::clearChoices() {
    mCheckStates.clear();
    mCheckedIdStates.clear();
    mCheckedItemCount = 0;
}

int AbsListView::computeVerticalScrollExtent() {
    int count = getChildCount();
    if (count ==0)return 0;
    if (mSmoothScrollbarEnabled) {
        int extent = count * 100;
        View* view = getChildAt(0);
        int top = view->getTop();
        int height = view->getHeight();
        if (height > 0) {
            extent += (top * 100) / height;
        }

        view = getChildAt(count - 1);
        int bottom = view->getBottom();
        height = view->getHeight();
        if (height > 0) {
            extent -= ((bottom - getHeight()) * 100) / height;
        }
        return extent;
    } else {
        return 1;
    }
}

int AbsListView::computeVerticalScrollOffset() {
    int firstPosition = mFirstPosition;
    int childCount = getChildCount();
    if (firstPosition >= 0 && childCount > 0) {
        if (mSmoothScrollbarEnabled) {
            View* view = getChildAt(0);
            int top = view->getTop();
            int height = view->getHeight();
            if (height > 0) {
                return std::max(firstPosition * 100 - (top * 100) / height +
                                (int)((float)mScrollY / getHeight() * mItemCount * 100), 0);
            }
        } else {
            int index;
            int count = mItemCount;
            if (firstPosition == 0) {
                index = 0;
            } else if (firstPosition + childCount == count) {
                index = count;
            } else {
                index = firstPosition + childCount / 2;
            }
            return (int) (firstPosition + childCount * (index / (float) count));
        }
    }
    return 0;
}

int AbsListView::computeVerticalScrollRange() {
    int result;
    if (mSmoothScrollbarEnabled) {
        result = std::max(mItemCount * 100, 0);
        if (mScrollY != 0) {
            // Compensate for overscroll
            result += std::abs((int) ((float) mScrollY / getHeight() * mItemCount * 100));
        }
    } else {
        result = mItemCount;
    }
    return result;
}

void AbsListView::useDefaultSelector() {
    //setSelector(new CorlorDrawable(0xFFFF0000));
}

bool AbsListView::isStackFromBottom()const {
    return mStackFromBottom;
}

void AbsListView::setStackFromBottom(bool stackFromBottom) {
    if (mStackFromBottom != stackFromBottom) {
        mStackFromBottom = stackFromBottom;
        requestLayoutIfNecessary();
    }
}
void AbsListView::onFocusChanged(bool gainFocus, int direction,Rect* previouslyFocusedRect) {
    AdapterView::onFocusChanged(gainFocus, direction, previouslyFocusedRect);
    if (gainFocus && mSelectedPosition < 0 && !isInTouchMode()) {
        if (!isAttachedToWindow() && mAdapter != nullptr) {
            // Data may have changed while we were detached and it's valid
            // to change focus while detached. Refresh so we don't die.
            mDataChanged = true;
            mOldItemCount = mItemCount;
            mItemCount = mAdapter->getCount();
        }
        resurrectSelection();
   }
}

void AbsListView::requestLayoutIfNecessary() {
    if (getChildCount() > 0) {
        resetList();
        requestLayout();
        invalidate();
    }
}

void AbsListView::setDrawSelectorOnTop(bool onTop) {
    mDrawSelectorOnTop=onTop;
}

Drawable*AbsListView::getSelector() {
    return mSelector;
}

void AbsListView::setSelector(Drawable*sel) {
    if (mSelector != nullptr) {
        mSelector->setCallback(nullptr);
        unscheduleDrawable(*mSelector);
    }
    mSelector = sel;
    Rect padding;
    sel->getPadding(padding);
    mSelectionLeftPadding  = padding.left;
    mSelectionTopPadding   = padding.top;
    mSelectionRightPadding = padding.width;
    mSelectionBottomPadding= padding.height;
    sel->setCallback(this);
    updateSelectorState();
}

void AbsListView::setSelector(const std::string&resid) {
    setSelector(mContext->getDrawable(resid));
}

void AbsListView::getFocusedRect(Rect& r){
    View* view = getSelectedView();
    if (view && view->getParent() == this) {
        // the focused rectangle of the selected view offset into the
        // coordinate space of this view.
        view->getFocusedRect(r);
        offsetDescendantRectToMyCoords(view, r);
    } else {
        // otherwise, just the norm
        AdapterView::getFocusedRect(r);
    }
}

bool AbsListView::touchModeDrawsInPressedState() {
    switch (mTouchMode) {
    case TOUCH_MODE_TAP:
    case TOUCH_MODE_DONE_WAITING:
        return true;
    default:
        return false;
    }
}

void AbsListView::internalSetPadding(int left, int top, int width, int height){
    AdapterView::internalSetPadding(left, top, width, height);
    if (isLayoutRequested()) {
         handleBoundsChange();
    }
}

void AbsListView::onSizeChanged(int w, int h, int oldw, int oldh){
    handleBoundsChange();
    if (mFastScroll) {
        mFastScroll->onSizeChanged(w, h, oldw, oldh);
    }
}

void AbsListView::handleBoundsChange(){
    if (mInLayout) {
        return;
    }
    const int childCount = getChildCount();
    if (childCount > 0) {
        mDataChanged = true;
        rememberSyncState();
        for (int i = 0; i < childCount; i++) {
             View* child = getChildAt(i);
             ViewGroup::LayoutParams* lp = child->getLayoutParams();
             // force layout child unless it has exact specs
             if (lp == nullptr || lp->width < 1 || lp->height < 1) {
                 child->forceLayout();
             }
        }
    }
}

bool AbsListView::shouldShowSelector() {
    return (isFocused() && !isInTouchMode()) || (touchModeDrawsInPressedState() && isPressed());
}

bool AbsListView::shouldDrawSelector() {
    return !mSelectorRect.empty();
}

void AbsListView::drawSelector(Canvas&canvas) {
    if (shouldDrawSelector() && mSelector) {
        mSelector->setBounds(mSelectorRect);
        mSelector->draw(canvas);
    }
}

std::vector<int>AbsListView::getDrawableStateForSelector() {
    if (mIsChildViewEnabled) {
        // Common case
        return AdapterView::getDrawableState();
    }

    // The selector uses this View's drawable state. The selected child view
    // is disabled, so we need to remove the enabled state from the drawable states.
    int enabledState = -1;//ENABLED_STATE_SET[0];

    // If we don't have any extra space, it will return one of the static
    // state arrays, and clearing the enabled state on those arrays is a
    // bad thing! If we specify we need extra space, it will create+copy
    // into a new array that is safely mutable.
    std::vector<int>state = onCreateDrawableState();

    int enabledPos = -1;
    for (int i = state.size() - 1; i >= 0; i--) {
        if (state[i] == enabledState) {
            enabledPos = i;
            break;
        }
    }

    // Remove the enabled state
    if (enabledPos >= 0) {
        state.erase(state.begin()+enabledPos);
        //System.arraycopy(state, enabledPos + 1, state, enabledPos,
        //        state.length - enabledPos - 1);
    }

    return state;
}

void AbsListView::updateSelectorState() {
    if (mSelector && mSelector->isStateful()) {
        if (shouldShowSelector()) {
            if (mSelector->setState(getDrawableStateForSelector())) {
                invalidateDrawable(*mSelector);
            }
        } else {
            mSelector->setState(StateSet::NOTHING);
        }
    }
}

void AbsListView::layoutChildren() {
}

void AbsListView::drawableStateChanged() {
    AdapterView::drawableStateChanged();
    updateSelectorState();
}

void AbsListView::onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
    if (mSelector == nullptr) {
        useDefaultSelector();
    }
    Rect& listPadding= mListPadding;
    listPadding.left = mSelectionLeftPadding + mPaddingLeft;
    listPadding.top  = mSelectionTopPadding + mPaddingTop;
    listPadding.width = mSelectionRightPadding + mPaddingRight;
    listPadding.height= mSelectionBottomPadding + mPaddingBottom;

    // Check if our previous measured size was at a point where we should scroll later.
    if (mTranscriptMode == TRANSCRIPT_MODE_NORMAL) {
        int childCount = getChildCount();
        int listBottom = getHeight() - getPaddingBottom();
        View* lastChild = getChildAt(childCount - 1);
        int lastBottom = lastChild ? lastChild->getBottom() : listBottom;
        mForceTranscriptScroll = mFirstPosition + childCount >= mLastHandledItemCount &&
                                 lastBottom <= listBottom;
    }
}

void AbsListView::onLayout(bool changed, int l, int t, int w, int h) {
    AdapterView::onLayout(changed, l, t, w, h);
    int childCount = getChildCount();
    mInLayout = true;
    if (changed) {
        for (int i = 0; i < childCount; i++) {
            getChildAt(i)->forceLayout();
        }
        mRecycler->markChildrenDirty();
    }
    layoutChildren();

    mOverscrollMax = h / OVERSCROLL_LIMIT_DIVISOR;

    // TODO: Move somewhere sane. This doesn't belong in onLayout().
    if (mFastScroll)
        mFastScroll->onItemCountChanged(getChildCount(), mItemCount);
    mInLayout = false;
}

void AbsListView::setItemViewLayoutParams(View* child, int position) {
    ViewGroup::LayoutParams* vlp = child->getLayoutParams();
    LayoutParams* lp;
    if (vlp == nullptr) {
        lp = (LayoutParams*) generateDefaultLayoutParams();
    } else if (!checkLayoutParams(vlp)) {
        lp = (LayoutParams*) generateLayoutParams(vlp);
    } else {
        lp = (LayoutParams*) vlp;
    }

    if (mAdapterHasStableIds) {
        lp->itemId = mAdapter->getItemId(position);
    }
    lp->viewType = mAdapter->getItemViewType(position);
    lp->isEnabled = mAdapter->isEnabled(position);
    if (lp != vlp) {
        child->setLayoutParams(lp);
    }
}

void AbsListView::confirmCheckedPositionsById() {
    // Clear out the positional check states, we'll rebuild it below from IDs.
    mCheckStates.clear();

    bool checkedCountChanged = false;
    for (int checkedIndex = 0; checkedIndex < mCheckedIdStates.size(); checkedIndex++) {
        long id = mCheckedIdStates.keyAt(checkedIndex);
        int lastPos = mCheckedIdStates.valueAt(checkedIndex);

        long lastPosId = mAdapter->getItemId(lastPos);
        if (id != lastPosId) {
            // Look around to see if the ID is nearby. If not, uncheck it.
            int start = std::max(0, lastPos - CHECK_POSITION_SEARCH_DISTANCE);
            int end = std::min(lastPos + CHECK_POSITION_SEARCH_DISTANCE, mItemCount);
            bool found = false;
            for (int searchPos = start; searchPos < end; searchPos++) {
                long searchId = mAdapter->getItemId(searchPos);
                if (id == searchId) {
                    found = true;
                    mCheckStates.put(searchPos, true);
                    mCheckedIdStates.put(checkedIndex, searchPos);
                    break;
                }
            }

            if (!found) {
                mCheckedIdStates.remove(id);//delete(id);
                checkedIndex--;
                mCheckedItemCount--;
                checkedCountChanged = true;
                if (/*mChoiceActionMode  && */mMultiChoiceModeCallback ) {
                    mMultiChoiceModeCallback(lastPos,id,false);
                    //mMultiChoiceModeCallback.onItemCheckedStateChanged(mChoiceActionMode,lastPos, id, false);
                }
            }
        } else {
            mCheckStates.put(lastPos, true);
        }
    }

    /*if (checkedCountChanged && mChoiceActionMode != nullptr) {
        mChoiceActionMode.invalidate();
    }*/
}

bool AbsListView::resurrectSelectionIfNeeded() {
    if (mSelectedPosition < 0 && resurrectSelection()) {
        updateSelectorState();
        return true;
    }
    return false;
}

bool AbsListView::resurrectSelection() {
    int childCount = getChildCount();

    if (childCount <= 0) {
        return false;
    }

    int selectedTop = 0;
    int selectedPos;
    int childrenTop = mListPadding.top;
    int childrenBottom = mBottom-mTop - mListPadding.height;
    int firstPosition = mFirstPosition;
    int toPosition = mResurrectToPosition;
    bool down = true;

    if (toPosition >= firstPosition && toPosition < firstPosition + childCount) {
        selectedPos = toPosition;

        View* selected = getChildAt(selectedPos - mFirstPosition);
        selectedTop = selected->getTop();
        int selectedBottom = selected->getBottom();

        // We are scrolled, don't get in the fade
        if (selectedTop < childrenTop) {
            selectedTop = childrenTop + getVerticalFadingEdgeLength();
        } else if (selectedBottom > childrenBottom) {
            selectedTop = childrenBottom - selected->getMeasuredHeight()
                          - getVerticalFadingEdgeLength();
        }
    } else {
        if (toPosition < firstPosition) {
            // Default to selecting whatever is first
            selectedPos = firstPosition;
            for (int i = 0; i < childCount; i++) {
                View* v = getChildAt(i);
                int top = v->getTop();

                if (i == 0) {
                    // Remember the position of the first item
                    selectedTop = top;
                    // See if we are scrolled at all
                    if (firstPosition > 0 || top < childrenTop) {
                        // If we are scrolled, don't select anything that is
                        // in the fade region
                        childrenTop += getVerticalFadingEdgeLength();
                    }
                }
                if (top >= childrenTop) {
                    // Found a view whose top is fully visisble
                    selectedPos = firstPosition + i;
                    selectedTop = top;
                    break;
                }
            }
        } else {
            int itemCount = mItemCount;
            down = false;
            selectedPos = firstPosition + childCount - 1;

            for (int i = childCount - 1; i >= 0; i--) {
                View* v = getChildAt(i);
                int top = v->getTop();
                int bottom = v->getBottom();

                if (i == childCount - 1) {
                    selectedTop = top;
                    if (firstPosition + childCount < itemCount || bottom > childrenBottom) {
                        childrenBottom -= getVerticalFadingEdgeLength();
                    }
                }

                if (bottom <= childrenBottom) {
                    selectedPos = firstPosition + i;
                    selectedTop = top;
                    break;
                }
            }
        }
    }

    mResurrectToPosition = INVALID_POSITION;
    removeCallbacks(mFlingRunnable);
    if (mPositionScroller) mPositionScroller->stop();

    mTouchMode = TOUCH_MODE_REST;
    clearScrollingCache();
    mSpecificTop = selectedTop;
    selectedPos = lookForSelectablePosition(selectedPos, down);
    if (selectedPos >= firstPosition && selectedPos <= getLastVisiblePosition()) {
        mLayoutMode = LAYOUT_SPECIFIC;
        updateSelectorState();
        setSelectionInt(selectedPos);
        invokeOnItemScrollListener();
    } else {
        selectedPos = INVALID_POSITION;
    }
    reportScrollStateChange(OnScrollListener::SCROLL_STATE_IDLE);

    return selectedPos >= 0;
}

bool AbsListView::performLongPress(View* child,int longPressPosition,long longPressId){
    return performLongPress(child,longPressPosition,longPressId,-1,-1);
}

bool AbsListView::performLongPress(View* child,int longPressPosition,long longPressId,int x,int y){
    if (mChoiceMode == CHOICE_MODE_MULTIPLE_MODAL) {
        /*if (mChoiceActionMode == nullptr &&
            (mChoiceActionMode = startActionMode(mMultiChoiceModeCallback)) != null)*/ {
            setItemChecked(longPressPosition, true);
            //performHapticFeedback(HapticFeedbackConstants.LONG_PRESS);
        }
        return true;
    }

    bool handled = false;
    if (mOnItemLongClickListener != nullptr) {
        handled = mOnItemLongClickListener(*this,*child,longPressPosition, longPressId);
    }
    /*if (!handled) {
        mContextMenuInfo = createContextMenuInfo(child, longPressPosition, longPressId);
        if (x != CheckForLongPress.INVALID_COORD && y != CheckForLongPress.INVALID_COORD) {
            handled = AdapterView::showContextMenuForChild(this, x, y);
        } else {
            handled = AdapterView::showContextMenuForChild(this);
        }
    }*/
    //if (handled) performHapticFeedback(HapticFeedbackConstants.LONG_PRESS);
    return handled;
}

void AbsListView::doClick(int motionPosition ){
    if (mDataChanged) return;
    ListAdapter* adapter = mAdapter;
    if (adapter && mItemCount > 0 &&
            motionPosition != INVALID_POSITION &&
            motionPosition < adapter->getCount() /*&& sameWindow()*/ &&
            adapter->isEnabled(motionPosition)) {
        View* view = getChildAt(motionPosition - mFirstPosition);
        // If there is no view, something bad happened (the view scrolled off the
        // screen, etc.) and we should cancel the click
        if (view )
            performItemClick(view, motionPosition, adapter->getItemId(motionPosition));
    }
}

void AbsListView::checkTap(int x,int y) {
    if (mTouchMode == TOUCH_MODE_DOWN) {
        mTouchMode = TOUCH_MODE_TAP;
        View* child = getChildAt(mMotionPosition - mFirstPosition);
        if (child  && !child->hasExplicitFocusable()) {
            mLayoutMode = LAYOUT_NORMAL;

            if (!mDataChanged) {
                float point[2]={(float)x,(float)y};
                transformPointToViewLocal(point,*child);
                //child->drawableHotspotChanged(point[0], point[1]);
                child->setPressed(true);
                setPressed(true);
                layoutChildren();
                positionSelector(mMotionPosition, child);
                refreshDrawableState();

                int longPressTimeout = ViewConfiguration::getLongPressTimeout();
                bool longClickable = isLongClickable();

                if (mSelector) {
                    Drawable* d = mSelector->getCurrent();
                    if (d && dynamic_cast<TransitionDrawable*>(d)) {
                        if (longClickable) {
                            ((TransitionDrawable*) d)->startTransition(longPressTimeout);
                        } else {
                            ((TransitionDrawable*) d)->resetTransition();
                        }
                    }
                    mSelector->setHotspot(x, y);
                }

                if (longClickable) {
                    if (mPendingCheckForLongPress == nullptr) {
                        mPendingCheckForLongPress=std::bind(&AbsListView::checkLongPress,this,x,y);
                        //mPendingCheckForLongPress = new CheckForLongPress();
                    }
                    //mPendingCheckForLongPress.rememberWindowAttachCount();
                    postDelayed(mPendingCheckForLongPress, longPressTimeout);
                } else {
                    mTouchMode = TOUCH_MODE_DONE_WAITING;
                }
            } else {
                mTouchMode = TOUCH_MODE_DONE_WAITING;
            }
        }
    }
}

void AbsListView::checkLongPress(int mX,int mY){
    #define INVALID_COORD -1
    int motionPosition = mMotionPosition;
    View* child = getChildAt(motionPosition - mFirstPosition);
    if (child) {
        int longPressPosition = mMotionPosition;
        long longPressId = mAdapter->getItemId(mMotionPosition);

        bool handled = false;
        if (/*sameWindow() &&*/ !mDataChanged) {
            if (mX != INVALID_COORD && mY != INVALID_COORD) {
                handled = performLongPress(child, longPressPosition, longPressId, mX, mY);
            } else {
                handled = performLongPress(child, longPressPosition, longPressId);
            }
        }

        if (handled) {
            mHasPerformedLongPress = true;
            mTouchMode = TOUCH_MODE_REST;
            setPressed(false);
            child->setPressed(false);
        } else {
            mTouchMode = TOUCH_MODE_DONE_WAITING;
        }
    }
}

void AbsListView::CheckForKeyLongPress(){
    if (isPressed() && mSelectedPosition >= 0) {
        int index = mSelectedPosition - mFirstPosition;
        View* v = getChildAt(index);

        if (!mDataChanged) {
            bool handled = false;
            //if (sameWindow()) {
                handled = performLongPress(v, mSelectedPosition, mSelectedRowId);
            //}
            if (handled) {
                setPressed(false);
                v->setPressed(false);
            }
        } else {
            setPressed(false);
            if (v) v->setPressed(false);
        }
    }
}

void AbsListView::keyPressed() {
    if (!isEnabled() || !isClickable()) {
        return;
    }

    Drawable* selector = mSelector;
    Rect selectorRect = mSelectorRect;
    LOGD("focused=%d touchModeDrawsInPressedState=%d",isFocused(),touchModeDrawsInPressedState());
    if (selector != nullptr && (isFocused() || touchModeDrawsInPressedState())
            && !selectorRect.empty()) {

        View* v = getChildAt(mSelectedPosition - mFirstPosition);

        if (v != nullptr) {
            if (v->hasExplicitFocusable()) return;
            v->setPressed(true);
        }
        setPressed(true);

        bool longClickable = isLongClickable();
        Drawable* d = selector->getCurrent();
        if (d != nullptr && dynamic_cast<TransitionDrawable*>(d)) {
            if (longClickable) {
                ((TransitionDrawable*) d)->startTransition(ViewConfiguration::getLongPressTimeout());
            } else {
                ((TransitionDrawable*) d)->resetTransition();
            }
        }
        if (longClickable && !mDataChanged) {
            postDelayed(std::bind(&AbsListView::CheckForKeyLongPress,this),ViewConfiguration::getLongPressTimeout());
        }
    }
}

void AbsListView::handleDataChanged() {
    int count = mItemCount;
    int lastHandledItemCount = mLastHandledItemCount;
    mLastHandledItemCount = mItemCount;

    if (mChoiceMode != CHOICE_MODE_NONE && mAdapter  && mAdapter->hasStableIds()) {
        confirmCheckedPositionsById();
    }
    // TODO: In the future we can recycle these views based on stable ID instead.
    mRecycler->clearTransientStateViews();

    if (count > 0) {
        int newPos;
        int selectablePos;

        // Find the row we are supposed to sync to
        if (mNeedSync) {
            // Update this first, since setNextSelectedPositionInt inspects it
            mNeedSync = false;
            //mPendingSync = nullptr;

            if (mTranscriptMode == TRANSCRIPT_MODE_ALWAYS_SCROLL) {
                mLayoutMode = LAYOUT_FORCE_BOTTOM;
                return;
            } else if (mTranscriptMode == TRANSCRIPT_MODE_NORMAL) {
                if (mForceTranscriptScroll) {
                    mForceTranscriptScroll = false;
                    mLayoutMode = LAYOUT_FORCE_BOTTOM;
                    return;
                }
                int childCount = getChildCount();
                int listBottom = getHeight() - getPaddingBottom();
                View* lastChild = getChildAt(childCount - 1);
                int lastBottom = lastChild ? lastChild->getBottom() : listBottom;
                if (mFirstPosition + childCount >= lastHandledItemCount &&
                        lastBottom <= listBottom) {
                    mLayoutMode = LAYOUT_FORCE_BOTTOM;
                    return;
                }
                // Something new came in and we didn't scroll; give the user a clue that
                // there's something new.
                awakenScrollBars();
            }

            switch (mSyncMode) {
            case SYNC_SELECTED_POSITION:
                if (isInTouchMode()) {
                    // We saved our state when not in touch mode. (We know this because
                    // mSyncMode is SYNC_SELECTED_POSITION.) Now we are trying to
                    // restore in touch mode. Just leave mSyncPosition as it is (possibly
                    // adjusting if the available range changed) and return.
                    mLayoutMode = LAYOUT_SYNC;
                    mSyncPosition = std::min(std::max(0, mSyncPosition), count - 1);

                    return;
                } else {
                    // See if we can find a position in the new data with the same
                    // id as the old selection. This will change mSyncPosition.
                    newPos = findSyncPosition();
                    if (newPos >= 0) {
                        // Found it. Now verify that new selection is still selectable
                        selectablePos = lookForSelectablePosition(newPos, true);
                        if (selectablePos == newPos) {
                            // Same row id is selected
                            mSyncPosition = newPos;

                            if (mSyncHeight == getHeight()) {
                                // If we are at the same height as when we saved state, try
                                // to restore the scroll position too.
                                mLayoutMode = LAYOUT_SYNC;
                            } else {
                                // We are not the same height as when the selection was saved, so
                                // don't try to restore the exact position
                                mLayoutMode = LAYOUT_SET_SELECTION;
                            }

                            // Restore selection
                            setNextSelectedPositionInt(newPos);
                            return;
                        }
                    }
                }
                break;
            case SYNC_FIRST_POSITION:
                // Leave mSyncPosition as it is -- just pin to available range
                mLayoutMode = LAYOUT_SYNC;
                mSyncPosition = std::min(std::max(0, mSyncPosition), count - 1);

                return;
            }
        }

        if (!isInTouchMode()) {
            // We couldn't find matching data -- try to use the same position
            newPos = getSelectedItemPosition();

            // Pin position to the available range
            if (newPos >= count) {
                newPos = count - 1;
            }
            if (newPos < 0) {
                newPos = 0;
            }

            // Make sure we select something selectable -- first look down
            selectablePos = lookForSelectablePosition(newPos, true);

            if (selectablePos >= 0) {
                setNextSelectedPositionInt(selectablePos);
                return;
            } else {
                // Looking down didn't work -- try looking up
                selectablePos = lookForSelectablePosition(newPos, false);
                if (selectablePos >= 0) {
                    setNextSelectedPositionInt(selectablePos);
                    return;
                }
            }
        } else {

            // We already know where we want to resurrect the selection
            if (mResurrectToPosition >= 0) {
                return;
            }
        }

    }

    // Nothing is selected. Give up and reset everything.
    mLayoutMode = mStackFromBottom ? LAYOUT_FORCE_BOTTOM : LAYOUT_FORCE_TOP;
    mSelectedPosition = INVALID_POSITION;
    mSelectedRowId = INVALID_ROW_ID;
    mNextSelectedPosition = INVALID_POSITION;
    mNextSelectedRowId = INVALID_ROW_ID;
    mNeedSync = false;
    //mPendingSync = nullptr;
    mSelectorPosition = INVALID_POSITION;
    checkSelectionChanged();
}

int AbsListView::getDistance(const Rect& source,const Rect& dest, int direction){
    int sX, sY; // source x, y
    int dX, dY; // dest x, y
    switch (direction) {
    case View::FOCUS_RIGHT:
        sX = source.right();
        sY = source.top + source.height / 2;
        dX = dest.left;
        dY = dest.top + dest.height / 2;
        break;
    case View::FOCUS_DOWN:
        sX = source.left + source.width / 2;
        sY = source.bottom();
        dX = dest.left + dest.width / 2;
        dY = dest.top;
        break;
    case View::FOCUS_LEFT:
        sX = source.left;
        sY = source.top + source.height / 2;
        dX = dest.right();
        dY = dest.top + dest.height / 2;
        break;
    case View::FOCUS_UP:
        sX = source.left + source.width / 2;
        sY = source.top;
        dX = dest.left + dest.width / 2;
        dY = dest.top;
        break;
    case View::FOCUS_FORWARD:
    case View::FOCUS_BACKWARD:
        sX = source.right() + source.width / 2;
        sY = source.top + source.height / 2;
        dX = dest.left + dest.width / 2;
        dY = dest.top + dest.height / 2;
        break;
    default:
        throw "direction must be one of {FOCUS_UP, FOCUS_DOWN, FOCUS_LEFT, FOCUS_RIGHT, "
                    "FOCUS_FORWARD, FOCUS_BACKWARD}.";
    }
    int deltaX = dX - sX;
    int deltaY = dY - sY;
    return deltaY * deltaY + deltaX * deltaX;
}

void AbsListView::reclaimViews(std::vector<View*>& views){
    const int childCount = getChildCount();
    RecycleBin::RecyclerListener listener = mRecycler->mRecyclerListener;

    // Reclaim views on screen
    for (int i = 0; i < childCount; i++) {
        View* child = getChildAt(i);
        AbsListView::LayoutParams* lp = (AbsListView::LayoutParams*) child->getLayoutParams();
        // Don't reclaim header or footer views, or views that should be ignored
        if (lp  && mRecycler->shouldRecycleViewType(lp->viewType)) {
            views.push_back(child);
            //child.setAccessibilityDelegate(null);
            if (listener != nullptr) {
                // Pretend they went through the scrap heap
                listener(*child);//.onMovedToScrapHeap(child);
            }
        }
    }
    mRecycler->reclaimScrapViews(views);
    removeAllViewsInLayout();
}

View*AbsListView::obtainView(int position, bool*outMetadata) {

    outMetadata[0] = false;
    // Check whether we have a transient state view. Attempt to re-bind the
    // data and discard the view if we fail.
    View* transientView = mRecycler->getTransientStateView(position);
    if (transientView != nullptr) {
        LayoutParams* params = (LayoutParams*) transientView->getLayoutParams();

        // If the view type hasn't changed, attempt to re-bind the data.
        if (params->viewType == mAdapter->getItemViewType(position)) {
            View* updatedView = mAdapter->getView(position, transientView, this);

            // If we failed to re-bind the data, scrap the obtained view.
            if (updatedView != transientView) {
                setItemViewLayoutParams(updatedView, position);
                mRecycler->addScrapView(updatedView, position);
            }
        }
        outMetadata[0] = true;
        // Finish the temporary detach started in addScrapView().
        transientView->dispatchFinishTemporaryDetach();
        return transientView;
    }
    View* scrapView = mRecycler->getScrapView(position);
    View* child = mAdapter->getView(position, scrapView, this);
    if (scrapView != nullptr) {
        if (child != scrapView) {
            // Failed to re-bind the data, return scrap to the heap.
            mRecycler->addScrapView(scrapView, position);
        } else if (child->isTemporarilyDetached()) {
            outMetadata[0] = true;
            // Finish the temporary detach started in addScrapView().
            child->dispatchFinishTemporaryDetach();
        }
    }

    if (mCacheColorHint != 0) child->setDrawingCacheBackgroundColor(mCacheColorHint);

    /*if (child->getImportantForAccessibility() == IMPORTANT_FOR_ACCESSIBILITY_AUTO) {
        child->setImportantForAccessibility(IMPORTANT_FOR_ACCESSIBILITY_YES);
    }*/
    setItemViewLayoutParams(child, position);

    /*if (AccessibilityManager.getInstance(mContext).isEnabled()) {
        if (mAccessibilityDelegate == null) {
            mAccessibilityDelegate = new ListItemAccessibilityDelegate();
        }
        if (child.getAccessibilityDelegate() == null) {
            child.setAccessibilityDelegate(mAccessibilityDelegate);
        }
    }*/
    return child;
}

void AbsListView::draw(Canvas& canvas) {
    AdapterView::draw(canvas);
    if (shouldDisplayEdgeEffects()) {
        int scrollY = mScrollY;
        bool clipToPadding = getClipToPadding();
        int width;
        int height;
        int translateX;
        int translateY;

        if (clipToPadding) {
            width = getWidth() - mPaddingLeft - mPaddingRight;
            height = getHeight() - mPaddingTop - mPaddingBottom;
            translateX = mPaddingLeft;
            translateY = mPaddingTop;
        } else {
            width = getWidth();
            height = getHeight();
            translateX = 0;
            translateY = 0;
        }
        if (!mEdgeGlowTop->isFinished()) {
            const int edgeY = std::min(0, scrollY + mFirstPositionDistanceGuess) + translateY;
            canvas.save();
            canvas.rectangle(translateX, translateY,  width ,mEdgeGlowTop->getMaxHeight());
            canvas.clip();
            canvas.translate(translateX, edgeY);
            mEdgeGlowTop->setSize(width, height);
            if (mEdgeGlowTop->draw(canvas)) {
                invalidateTopGlow();
            }
            canvas.restore();
        }
        if (!mEdgeGlowBottom->isFinished()) {
            const int edgeX = -width + translateX;
            const int edgeY = std::max(getHeight(), scrollY + mLastPositionDistanceGuess)
                  - (clipToPadding ? mPaddingBottom : 0);
            canvas.save();
            canvas.rectangle(translateX, translateY + height - mEdgeGlowBottom->getMaxHeight(),
                    width, mEdgeGlowBottom->getMaxHeight());
            canvas.clip();
            canvas.translate(width, edgeY);
            canvas.rotate_degrees(180);//, width, 0);
            mEdgeGlowBottom->setSize(width, height);
            if (mEdgeGlowBottom->draw(canvas)) {
                invalidateBottomGlow();
            }
            canvas.restore();
        }
    }
}
void AbsListView::initOrResetVelocityTracker() {
    if (mVelocityTracker == nullptr) {
        mVelocityTracker = VelocityTracker::obtain();
    } else {
        mVelocityTracker->clear();
    }
}

void AbsListView::initVelocityTrackerIfNotExists() {
    if (mVelocityTracker == nullptr) {
        mVelocityTracker = VelocityTracker::obtain();
    }
}

void AbsListView::recycleVelocityTracker() {
    if (mVelocityTracker != nullptr) {
        mVelocityTracker->recycle();
        mVelocityTracker = nullptr;
    }
}

bool AbsListView::canScrollUp() {
    bool canScrollUp=mFirstPosition > 0;
    // 0th element is not visible
    // ... Or top of 0th element is not visible
    if (!canScrollUp) {
        if (getChildCount() > 0) {
            View* child = getChildAt(0);
            canScrollUp = child->getTop() < mListPadding.top;
        }
    }
    return canScrollUp;
}

bool AbsListView::canScrollDown() {
    bool canScrollDown;
    int count = getChildCount();
    // Last item is not visible
    canScrollDown = (mFirstPosition + count) < mItemCount;
    // ... Or bottom of the last element is not visible
    if (!canScrollDown && count > 0) {
        View* child = getChildAt(count - 1);
        canScrollDown = child->getBottom() > getBottom() - mListPadding.bottom();
    }

    return canScrollDown;
}

void AbsListView::scrollListBy(int y){
    trackMotionScroll(-y,-y);
}

bool  AbsListView::canScrollList(int direction){
    const int childCount = getChildCount();
    if (childCount == 0) return false;

    int firstPosition = mFirstPosition;
    Rect listPadding = mListPadding;
    if (direction > 0) {
       const int lastBottom = getChildAt(childCount - 1)->getBottom();
       const int lastPosition = firstPosition + childCount;
       return lastPosition < mItemCount || lastBottom > getHeight() - listPadding.height;//bottom;
   } else {
       const int firstTop = getChildAt(0)->getTop();
       return firstPosition > 0 || firstTop < listPadding.top;
   }
}

bool AbsListView::trackMotionScroll(int deltaY, int incrementalDeltaY) {
    int childCount = getChildCount();
    if (childCount == 0) {
        return true;
    }

    int firstTop = getChildAt(0)->getTop();
    int lastBottom = getChildAt(childCount - 1)->getBottom();

    Rect listPadding = mListPadding;

    // "effective padding" In this case is the amount of padding that affects
    // how much space should not be filled by items. If we don't clip to padding
    // there is no effective padding.
    int effectivePaddingTop = 0;
    int effectivePaddingBottom = 0;
    if ((mGroupFlags & CLIP_TO_PADDING_MASK) == CLIP_TO_PADDING_MASK) {
        effectivePaddingTop = listPadding.top;
        effectivePaddingBottom = listPadding.bottom();
    }

    // FIXME account for grid vertical spacing too?
    int spaceAbove = effectivePaddingTop - firstTop;
    int end = getHeight() - effectivePaddingBottom;
    int spaceBelow = lastBottom - end;

    int height = getHeight() - mPaddingBottom - mPaddingTop;
    if (deltaY < 0) {
        deltaY = std::max(-(height - 1), deltaY);
    } else {
        deltaY = std::min(height - 1, deltaY);
    }

    if (incrementalDeltaY < 0) {
        incrementalDeltaY = std::max(-(height - 1), incrementalDeltaY);
    } else {
        incrementalDeltaY = std::min(height - 1, incrementalDeltaY);
    }

    int firstPosition = mFirstPosition;

    // Update our guesses for where the first and last views are
    if (firstPosition == 0) {
        mFirstPositionDistanceGuess = firstTop - listPadding.top;
    } else {
        mFirstPositionDistanceGuess += incrementalDeltaY;
    }
    if (firstPosition + childCount == mItemCount) {
        mLastPositionDistanceGuess = lastBottom + listPadding.bottom();
    } else {
        mLastPositionDistanceGuess += incrementalDeltaY;
    }

    bool cannotScrollDown = (firstPosition == 0 &&
                firstTop >= listPadding.top && incrementalDeltaY >= 0);
    bool cannotScrollUp = (firstPosition + childCount == mItemCount &&
                lastBottom <= getHeight() - listPadding.bottom() && incrementalDeltaY <= 0);
    if (cannotScrollDown || cannotScrollUp) {
        return incrementalDeltaY != 0;
    }

    bool down = incrementalDeltaY < 0;

    bool inTouchMode = isInTouchMode();
    if (inTouchMode) {
        hideSelector();
    }

    int headerViewsCount = getHeaderViewsCount();
    int footerViewsStart = mItemCount - getFooterViewsCount();

    int start = 0;
    int count = 0;

    if (down) {
        int top = -incrementalDeltaY;
        if ((mGroupFlags & CLIP_TO_PADDING_MASK) == CLIP_TO_PADDING_MASK) {
            top += listPadding.top;
        }
        for (int i = 0; i < childCount; i++) {
            View* child = getChildAt(i);
            if (child->getBottom() >= top) {
                break;
            } else {
                count++;
                int position = firstPosition + i;
                if (position >= headerViewsCount && position < footerViewsStart) {
                    // The view will be rebound to new data, clear any
                    // system-managed transient state.
                    //child->clearAccessibilityFocus();
                    mRecycler->addScrapView(child, position);
                }
            }
        }
    } else {
        int bottom = getHeight() - incrementalDeltaY;
        if ((mGroupFlags & CLIP_TO_PADDING_MASK) == CLIP_TO_PADDING_MASK) {
            bottom -= listPadding.bottom();
        }
        for (int i = childCount - 1; i >= 0; i--) {
            View* child = getChildAt(i);
            if (child->getTop() <= bottom) {
                break;
            } else {
                start = i;
                count++;
                int position = firstPosition + i;
                if (position >= headerViewsCount && position < footerViewsStart) {
                    // The view will be rebound to new data, clear any
                    // system-managed transient state.
                    //child->clearAccessibilityFocus();
                    mRecycler->addScrapView(child, position);
                }
            }
        }
    }

    mMotionViewNewTop = mMotionViewOriginalTop + deltaY;

    mBlockLayoutRequests = true;

    if (count > 0) {
        detachViewsFromParent(start, count);
        mRecycler->removeSkippedScrap();
    }

    // invalidate before moving the children to avoid unnecessary invalidate
    // calls to bubble up from the children all the way to the top
    if (!awakenScrollBars()) invalidate();

    offsetChildrenTopAndBottom(incrementalDeltaY);

    if (down) {
        mFirstPosition += count;
    }
    int absIncrementalDeltaY = std::abs(incrementalDeltaY);
    if (spaceAbove < absIncrementalDeltaY || spaceBelow < absIncrementalDeltaY) {
        fillGap(down);
    }

    mRecycler->fullyDetachScrapViews();
    bool selectorOnScreen = false;
    if (!inTouchMode && mSelectedPosition != INVALID_POSITION) {
        int childIndex = mSelectedPosition - mFirstPosition;
        if (childIndex >= 0 && childIndex < getChildCount()) {
            positionSelector(mSelectedPosition, getChildAt(childIndex));
            selectorOnScreen = true;
        }
    } else if (mSelectorPosition != INVALID_POSITION) {
        int childIndex = mSelectorPosition - mFirstPosition;
        if (childIndex >= 0 && childIndex < getChildCount()) {
            positionSelector(mSelectorPosition, getChildAt(childIndex));
            selectorOnScreen = true;
        }
    }
    if (!selectorOnScreen) {
        mSelectorRect.setEmpty();
    }

    mBlockLayoutRequests = false;
    invokeOnItemScrollListener();

    return false;
}

void AbsListView::dispatchDraw(Canvas& canvas) {
    bool clipToPadding = (mGroupFlags & CLIP_TO_PADDING_MASK) == CLIP_TO_PADDING_MASK;
    if (clipToPadding) {
        canvas.save();
        int scrollX = mScrollX;
        int scrollY = mScrollY;
        canvas.rectangle(scrollX + mPaddingLeft, scrollY + mPaddingTop,
                         scrollX + getWidth() - mPaddingRight,
                         scrollY + getHeight() - mPaddingBottom);
        mGroupFlags &= ~CLIP_TO_PADDING_MASK;
        canvas.clip();
    }

    if (!mDrawSelectorOnTop)  drawSelector(canvas);

    AdapterView::dispatchDraw(canvas);

    if (mDrawSelectorOnTop) drawSelector(canvas);

    if (clipToPadding) {
        canvas.restore();
        mGroupFlags |= CLIP_TO_PADDING_MASK;
    }
}

bool AbsListView::verifyDrawable(Drawable* dr)const {
    return (mSelector == dr) || AdapterView::verifyDrawable(dr);
}

void AbsListView::jumpDrawablesToCurrentState() {
    AdapterView::jumpDrawablesToCurrentState();
    if (mSelector) mSelector->jumpToCurrentState();
}

void AbsListView::onAttachedToWindow() {
    AdapterView::onAttachedToWindow();

    /*ViewTreeObserver treeObserver = getViewTreeObserver();
    treeObserver.addOnTouchModeChangeListener(this);
    if (mTextFilterEnabled && mPopup != null && !mGlobalLayoutListenerAddedFilter) {
        treeObserver.addOnGlobalLayoutListener(this);
    }*/

    if (mAdapter && mDataSetObserver==nullptr) {
        mDataSetObserver = new AdapterDataSetObserver(this);
        mAdapter->registerDataSetObserver(mDataSetObserver);

        // Data may have changed while we were detached. Refresh.
        mDataChanged = true;
        mOldItemCount = mItemCount;
        mItemCount = mAdapter->getCount();
    }
}

void AbsListView::onDetachedFromWindow() {
    AdapterView::onDetachedFromWindow();

    mIsDetaching = true;

    // Dismiss the popup in case onSaveInstanceState() was not invoked
    dismissPopup();

    // Detach any view left in the scrap heap
    mRecycler->clear();

    /*ViewTreeObserver treeObserver = getViewTreeObserver();
    treeObserver.removeOnTouchModeChangeListener(this);
    if (mTextFilterEnabled && mPopup != nullptr) {
        treeObserver.removeOnGlobalLayoutListener(this);
        mGlobalLayoutListenerAddedFilter = false;
    }*/

    if (mAdapter && mDataSetObserver) {
        mAdapter->unregisterDataSetObserver(mDataSetObserver);
        delete mDataSetObserver;
        mDataSetObserver = nullptr;
    }

    /*if (mScrollStrictSpan != nullptr) {
        mScrollStrictSpan.finish();
        mScrollStrictSpan = nullptr;
    }

    if (mFlingStrictSpan != nullptr) {
        mFlingStrictSpan.finish();
        mFlingStrictSpan = nullptr;
    }*/

    removeCallbacks(mFlingRunnable);

    /*if (mPositionScroller != nullptr) {
        mPositionScroller->stop();
    }

    if (mClearScrollingCache != nullptr) {
        removeCallbacks(mClearScrollingCache);
    }

    if (mPerformClick != nullptr) {
        removeCallbacks(mPerformClick);
    }

    if (mTouchModeReset != nullptr) {
        removeCallbacks(mTouchModeReset);
        mTouchModeReset->run();
    }*/

    mIsDetaching = false;
}

void AbsListView::onWindowFocusChanged(bool hasWindowFocus){
    AdapterView::onWindowFocusChanged(hasWindowFocus);

    int touchMode = isInTouchMode() ? TOUCH_MODE_ON : TOUCH_MODE_OFF;

    if (!hasWindowFocus) {
        setChildrenDrawingCacheEnabled(false);
        if (mFlingRunnable != nullptr) {
            removeCallbacks(mFlingRunnable);
            // let the fling runnable report its new state which
            // should be idle
            mSuppressIdleStateChangeCall = false;//mFlingRunnable.mSuppressIdleStateChangeCall = false;
            FLY_endFling();//mFlingRunnable.endFling();
            if (mPositionScroller != nullptr) {
                mPositionScroller->stop();
            }
            if (mScrollY != 0) {
                mScrollY = 0;
                invalidateParentCaches();
                finishGlows();
                invalidate();
            }
        }
        // Always hide the type filter
        dismissPopup();

        if (touchMode == TOUCH_MODE_OFF) {
            // Remember the last selected element
            mResurrectToPosition = mSelectedPosition;
        }
    } else {
        if (mFiltered && !mPopupHidden) {
            // Show the type filter only if a filter is in effect
            showPopup();
        }

        // If we changed touch mode since the last time we had focus
        if (touchMode != mLastTouchMode && mLastTouchMode != TOUCH_MODE_UNKNOWN) {
            // If we come back in trackball mode, we bring the selection back
            if (touchMode == TOUCH_MODE_OFF) {
                // This will trigger a layout
                resurrectSelection();
               // If we come back in touch mode, then we want to hide the selector
            } else {
                hideSelector();
                mLayoutMode = LAYOUT_NORMAL;
                layoutChildren();
            }
        }
    }

    mLastTouchMode = touchMode;
}

void AbsListView::onRtlPropertiesChanged(int layoutDirection) {
    AdapterView::onRtlPropertiesChanged(layoutDirection);
    if (mFastScroll != nullptr) {
        mFastScroll->setScrollbarPosition(getVerticalScrollbarPosition());
    }
}

void AbsListView::dismissPopup(){
}

void AbsListView::showPopup(){
}

void AbsListView::updateOnScreenCheckedViews() {
    int firstPos = mFirstPosition;
    int count = getChildCount();
    for (int i = 0; i < count; i++) {
        View* child = getChildAt(i);
        int position = firstPos + i;
        if (dynamic_cast<Checkable*>(child)) {
            const bool checked=mCheckStates.get(position);
            dynamic_cast<Checkable*>(child)->setChecked(mCheckStates.get(position));
            LOGV("setChecked %p[%d] ->%d",child,position,checked);
        } else {
            child->setActivated(mCheckStates.get(position));
        }
    }
}


bool AbsListView::performItemClick(View* view, int position, long id) {
    bool handled = false;
    bool dispatchItemClick = true;

    LOGV("view=%p,posid=%d,%ld",view,position,id);
    if (mChoiceMode != CHOICE_MODE_NONE) {
        handled = true;
        bool checkedStateChanged = false;

        if (mChoiceMode == CHOICE_MODE_MULTIPLE ||
                (mChoiceMode == CHOICE_MODE_MULTIPLE_MODAL && mChoiceActionMode != nullptr)) {
            bool checked = !mCheckStates.get(position, false);
            mCheckStates.put(position, checked);
            if (mCheckedIdStates.size() && mAdapter->hasStableIds()) {
                if (checked) {
                    mCheckedIdStates.put(mAdapter->getItemId(position), position);
                } else {
                    mCheckedIdStates.remove(mAdapter->getItemId(position));
                }
            }
            if (checked) {
                mCheckedItemCount++;
            } else {
                mCheckedItemCount--;
            }
            if (mMultiChoiceModeCallback){//mChoiceActionMode != null) {
                mMultiChoiceModeCallback(position,id,checked);
                //mMultiChoiceModeCallback.onItemCheckedStateChanged(mChoiceActionMode,position, id, checked);
                dispatchItemClick = false;
            }
            checkedStateChanged = true;
        } else if (mChoiceMode == CHOICE_MODE_SINGLE) {
            bool checked = !mCheckStates.get(position, false);
            if (checked) {
                mCheckStates.clear();
                mCheckStates.put(position, true);
                if (mCheckedIdStates.size() && mAdapter->hasStableIds()) {
                    mCheckedIdStates.clear();
                    mCheckedIdStates.put(mAdapter->getItemId(position), position);
                }
                mCheckedItemCount = 1;
            } else if (mCheckStates.size() == 0 || !mCheckStates.valueAt(0)) {
                mCheckedItemCount = 0;
            }
            checkedStateChanged = true;
        }

        if (checkedStateChanged) {
            updateOnScreenCheckedViews();
        }
    }

    if (dispatchItemClick) {
        handled |= AdapterView::performItemClick(view, position, id);
    }
    return handled;
}

bool AbsListView::onKeyDown(int keyCode, KeyEvent& event) {
    return false;
}

bool AbsListView::onKeyUp(int keyCode, KeyEvent& event) {
    if (KeyEvent::isConfirmKey(keyCode)) {
        LOGV("%p:%d isClickable=%d pressed=%d mSelectedPosition=%d",this,mID,isClickable(),isPressed(),mSelectedPosition);
        if (!isEnabled()) return true;
        if (isClickable() && isPressed() &&  (mSelectedPosition >= 0)
                && mAdapter && (mSelectedPosition < mAdapter->getCount()) ) {
            View* view = getChildAt(mSelectedPosition - mFirstPosition);
            if (view) {
                performItemClick(view, mSelectedPosition, mSelectedRowId);
                view->setPressed(false);
            }
            setPressed(false);
            return true;
        }
    }
    return AdapterView::onKeyUp(keyCode, event);
}

void AbsListView::dispatchSetPressed(bool pressed) {
    // Don't dispatch setPressed to our children. We call setPressed on ourselves to
    // get the selector in the right state, but we don't want to press each child.
}

void AbsListView::dispatchDrawableHotspotChanged(float x, float y){
    // Don't dispatch hotspot changes to children. We'll manually handle
    // calling drawableHotspotChanged on the correct child.
}

int AbsListView::pointToPosition(int x, int y) {
    Rect frame;
    int count = getChildCount();
    for (int i = count - 1; i >= 0; i--) {
        View* child = getChildAt(i);
        if (child->getVisibility() == View::VISIBLE) {
            child->getHitRect(frame);
            if (frame.contains(x, y)) {
                return mFirstPosition + i;
            }
        }
    }
    return INVALID_POSITION;
}

long AbsListView::pointToRowId(int x, int y) {
    int position = pointToPosition(x, y);
    if (position >= 0) {
        return mAdapter->getItemId(position);
    }
    return INVALID_ROW_ID;
}

int AbsListView::findClosestMotionRow(int y) {
    int childCount = getChildCount();
    if (childCount == 0) {
        return INVALID_POSITION;
    }

    int motionRow = findMotionRow(y);
    return motionRow != INVALID_POSITION ? motionRow : mFirstPosition + childCount - 1;
}

bool AbsListView::onInterceptTouchEvent(MotionEvent& ev) {
    View* v;
    int x,y;
    int actionMasked = ev.getActionMasked();
    if (mPositionScroller)  mPositionScroller->stop();

    if (mIsDetaching || !isAttachedToWindow()) {
        // Something isn't right.
        // Since we rely on being attached to get data set change notifications,
        // don't risk doing anything where we might try to resync and find things
        // in a bogus state.
        return false;
    }

    if (mFastScroll && mFastScroll->onInterceptTouchEvent(ev)) return true;
    switch (actionMasked) {
    case MotionEvent::ACTION_DOWN: {
        if (mTouchMode == TOUCH_MODE_OVERFLING || mTouchMode == TOUCH_MODE_OVERSCROLL) {
            mMotionCorrection = 0;
            return true;
        }

        x = (int) ev.getX();
        y = (int) ev.getY();
        mActivePointerId = ev.getPointerId(0);

        int motionPosition = findMotionRow(y);
        if (mTouchMode != TOUCH_MODE_FLING && motionPosition >= 0) {
            // User clicked on an actual view (and was not stopping a fling).
            // Remember where the motion event started
            v = getChildAt(motionPosition - mFirstPosition);
            mMotionViewOriginalTop = v->getTop();
            mMotionX = x;
            mMotionY = y;
            mMotionPosition = motionPosition;
            mTouchMode = TOUCH_MODE_DOWN;
            clearScrollingCache();
        }
        mLastY = INT_MIN;
        initOrResetVelocityTracker();
        mVelocityTracker->addMovement(ev);
        mNestedYOffset = 0;
        startNestedScroll(SCROLL_AXIS_VERTICAL);
        if (mTouchMode == TOUCH_MODE_FLING) {
            return true;
        }
        break;
    }

    case MotionEvent::ACTION_MOVE: {
        switch (mTouchMode) {
        case TOUCH_MODE_DOWN:
            int pointerIndex = ev.findPointerIndex(mActivePointerId);
            if (pointerIndex == -1) {
                pointerIndex = 0;
                mActivePointerId = ev.getPointerId(pointerIndex);
            }
            y = (int) ev.getY(pointerIndex);
            initVelocityTrackerIfNotExists();
            mVelocityTracker->addMovement(ev);
            if (startScrollIfNeeded((int) ev.getX(pointerIndex), y, nullptr)) {
                return true;
            }
            break;
        }
        break;
    }

    case MotionEvent::ACTION_CANCEL:
    case MotionEvent::ACTION_UP: {
        mTouchMode = TOUCH_MODE_REST;
        mActivePointerId = INVALID_POINTER;
        recycleVelocityTracker();
        reportScrollStateChange(OnScrollListener::SCROLL_STATE_IDLE);
        stopNestedScroll();
        break;
    }

    case MotionEvent::ACTION_POINTER_UP:
        onSecondaryPointerUp(ev);
        break;
    }
    return false;
}

void AbsListView::onSecondaryPointerUp(MotionEvent& ev) {
    int pointerIndex = (ev.getAction() & MotionEvent::ACTION_POINTER_INDEX_MASK) >>
                       MotionEvent::ACTION_POINTER_INDEX_SHIFT;
    int pointerId = ev.getPointerId(pointerIndex);
    if (pointerId == mActivePointerId) {
        // This was our active pointer going up. Choose a new
        // active pointer and adjust accordingly.
        // TODO: Make this decision more intelligent.
        int newPointerIndex = pointerIndex == 0 ? 1 : 0;
        mMotionX = (int) ev.getX(newPointerIndex);
        mMotionY = (int) ev.getY(newPointerIndex);
        mMotionCorrection = 0;
        mActivePointerId = ev.getPointerId(newPointerIndex);
    }
}

void AbsListView::onTouchModeChanged(bool isInTouchMode){
    if (isInTouchMode) {
        // Get rid of the selection when we enter touch mode
        hideSelector();
        // Layout, but only if we already have done so previously.
        // (Otherwise may clobber a LAYOUT_SYNC layout that was requested to restore
        // state.)
        if (getHeight() > 0 && getChildCount() > 0) {
            // We do not lose focus initiating a touch (since AbsListView is focusable in
            // touch mode). Force an initial layout to get rid of the selection.
            layoutChildren();
        }
        updateSelectorState();
    } else {
        int touchMode = mTouchMode;
        if (touchMode == TOUCH_MODE_OVERSCROLL || touchMode == TOUCH_MODE_OVERFLING) {
            FLY_endFling();
            if (mPositionScroller)mPositionScroller->stop();

            if (mScrollY != 0) {
                mScrollY = 0;
                invalidateParentCaches();
                finishGlows();
                invalidate();
            }
        }
    }
}

bool AbsListView::onTouchEvent(MotionEvent& ev) {
    if (!isEnabled()) {
        // A disabled view that is clickable still consumes the touch
        // events, it just doesn't respond to them.
        return isClickable() || isLongClickable();
    }

    if (mPositionScroller)  mPositionScroller->stop();


    if (mIsDetaching || !isAttachedToWindow()) {
        // Something isn't right.
        // Since we rely on being attached to get data set change notifications,
        // don't risk doing anything where we might try to resync and find things
        // in a bogus state.
        return false;
    }

    startNestedScroll(SCROLL_AXIS_VERTICAL);

    if (mFastScroll && mFastScroll->onTouchEvent(ev)) return true;

    initVelocityTrackerIfNotExists();
    MotionEvent* vtev = MotionEvent::obtain(ev);

    int actionMasked = ev.getActionMasked();
    if (actionMasked == MotionEvent::ACTION_DOWN) {
        mNestedYOffset = 0;
    }
    vtev->offsetLocation(0, mNestedYOffset);
    switch (actionMasked) {
    case MotionEvent::ACTION_DOWN:
        onTouchDown(ev);        break;
    case MotionEvent::ACTION_MOVE:
        onTouchMove(ev, *vtev); break;
    case MotionEvent::ACTION_UP:
        onTouchUp(ev);          break;
    case MotionEvent::ACTION_CANCEL:
        onTouchCancel();        break;
    case MotionEvent::ACTION_POINTER_UP: {
        onSecondaryPointerUp(ev);
        int motionPosition = pointToPosition(mMotionX, mMotionY);
        if (motionPosition >= 0) {
            // Remember where the motion event started
            View* child = getChildAt(motionPosition - mFirstPosition);
            mMotionViewOriginalTop = child->getTop();
            mMotionPosition = motionPosition;
        }
        mLastY = mMotionY;
        break;
    }
    case MotionEvent::ACTION_POINTER_DOWN: {
        // New pointers take over dragging duties
        int index = ev.getActionIndex();
        int id = ev.getPointerId(index);
        int x = (int) ev.getX(index);
        int y = (int) ev.getY(index);
        mMotionCorrection = 0;
        mActivePointerId = id;
        mMotionX = x;
        mMotionY = y;
        int motionPosition = pointToPosition(x, y);
        if (motionPosition >= 0) {
            // Remember where the motion event started
            View* child = getChildAt(motionPosition - mFirstPosition);
            mMotionViewOriginalTop = child->getTop();
            mMotionPosition = motionPosition;
        }
        mLastY = y;
        break;
    }
    }

    vtev->recycle();
    if (mVelocityTracker) mVelocityTracker->addMovement(*vtev);
    return true;
}

void AbsListView::reportScrollStateChange(int newState) {
    if (newState != mLastScrollState) {
        if (mOnScrollListener.onScrollStateChanged) {
            mLastScrollState = newState;
            mOnScrollListener.onScrollStateChanged(*this, newState);
        }
    }    
}

bool AbsListView::contentFits() {
    int childCount = getChildCount();
    if (childCount == 0) return true;
    if (childCount != mItemCount) return false;

    return getChildAt(0)->getTop() >= mListPadding.top &&
           getChildAt(childCount - 1)->getBottom() <= getHeight() - mListPadding.height;
}

void AbsListView::hideSelector() {
    if (mSelectedPosition != INVALID_POSITION) {
        if (mLayoutMode != LAYOUT_SPECIFIC) {
            mResurrectToPosition = mSelectedPosition;
        }
        if (mNextSelectedPosition >= 0 && mNextSelectedPosition != mSelectedPosition) {
            mResurrectToPosition = mNextSelectedPosition;
        }
        setSelectedPositionInt(INVALID_POSITION);
        setNextSelectedPositionInt(INVALID_POSITION);
        mSelectedTop = 0;
    }
}

int AbsListView::reconcileSelectedPosition() {
    int position = mSelectedPosition;
    if (position < 0) {
        position = mResurrectToPosition;
    }
    position = std::max(0, position);
    position = std::min(position, mItemCount - 1);
    return position;
}

bool AbsListView::startScrollIfNeeded(int x, int y, MotionEvent* vtev) {
    int deltaY = y - mMotionY;
    int distance = std::abs(deltaY);
    bool overscroll = mScrollY != 0;
    if ((overscroll || distance > mTouchSlop) &&
            (getNestedScrollAxes() & SCROLL_AXIS_VERTICAL) == 0) {
        createScrollingCache();
        if (overscroll) {
            mTouchMode = TOUCH_MODE_OVERSCROLL;
            mMotionCorrection = 0;
        } else {
            mTouchMode = TOUCH_MODE_SCROLL;
            mMotionCorrection = deltaY > 0 ? mTouchSlop : -mTouchSlop;
        }
        removeCallbacks(mPendingCheckForLongPress);
        setPressed(false);
        View* motionView = getChildAt(mMotionPosition - mFirstPosition);
        if (motionView) motionView->setPressed(false);

        reportScrollStateChange(OnScrollListener::SCROLL_STATE_TOUCH_SCROLL);
        // Time to start stealing events! Once we've stolen them, don't let anyone
        // steal from us
        ViewGroup* parent = getParent();
        if (parent) parent->requestDisallowInterceptTouchEvent(true);
        scrollIfNeeded(x, y, vtev);
        return true;
    }
    return false;
}

void AbsListView::scrollIfNeeded(int x, int y, MotionEvent* vtev) {
    int rawDeltaY = y - mMotionY;
    int scrollOffsetCorrection = 0;
    if (mLastY == INT_MIN) {
        rawDeltaY -= mMotionCorrection;
    }
    int incrementalDeltaY = mLastY != INT_MIN ? y - mLastY : rawDeltaY;
    incrementalDeltaY = releaseGlow(incrementalDeltaY,x);
    if (dispatchNestedPreScroll(0,-incrementalDeltaY, mScrollConsumed, mScrollOffset)) {
        rawDeltaY += mScrollConsumed[1];
        scrollOffsetCorrection = -mScrollOffset[1];
        incrementalDeltaY += mScrollConsumed[1];
        if (vtev != nullptr) {
            vtev->offsetLocation(0, mScrollOffset[1]);
            mNestedYOffset += mScrollOffset[1];
        }
    }
    int deltaY = rawDeltaY;
    int lastYCorrection = 0;

    if (mTouchMode == TOUCH_MODE_SCROLL/*3*/) {
        if (PROFILE_SCROLLING) {
            if (!mScrollProfilingStarted) {
                mScrollProfilingStarted = true;
            }
        }

        /*if (mScrollStrictSpan == nullptr) {
            // If it's non-null, we're already in a scroll.
            mScrollStrictSpan = StrictMode.enterCriticalSpan("AbsListView-scroll");
        }*/

        if (y != mLastY) {
            // We may be here after stopping a fling and continuing to scroll.
            // If so, we haven't disallowed intercepting touch events yet.
            // Make sure that we do so in case we're in a parent that can intercept.
            if ((mGroupFlags & FLAG_DISALLOW_INTERCEPT) == 0 &&
                    std::abs(rawDeltaY) > mTouchSlop) {
                ViewGroup* parent = getParent();
                if (parent)parent->requestDisallowInterceptTouchEvent(true);
            }

            int motionIndex;
            if (mMotionPosition >= 0) {
                motionIndex = mMotionPosition - mFirstPosition;
            } else {
                // If we don't have a motion position that we can reliably track,
                // pick something in the middle to make a best guess at things below.
                motionIndex = getChildCount() / 2;
            }

            int motionViewPrevTop = 0;
            View* motionView = getChildAt(motionIndex);
            if (motionView != nullptr) {
                motionViewPrevTop = motionView->getTop();
            }

            // No need to do all this work if we're not going to move anyway
            bool atEdge = false;
            if (incrementalDeltaY != 0) {
                atEdge = trackMotionScroll(deltaY, incrementalDeltaY);
            }

            // Check to see if we have bumped into the scroll limit
            motionView = getChildAt(motionIndex);
            if (motionView != nullptr) {
                // Check if the top of the motion view is where it is
                // supposed to be
                int motionViewRealTop = motionView->getTop();
                if (atEdge) {
                    // Apply overscroll

                    int overscroll = -incrementalDeltaY - (motionViewRealTop - motionViewPrevTop);
                    if (dispatchNestedScroll(0, overscroll - incrementalDeltaY, 0, overscroll,
                                             mScrollOffset)) {
                        lastYCorrection -= mScrollOffset[1];
                        if (vtev) {
                            vtev->offsetLocation(0, mScrollOffset[1]);
                            mNestedYOffset += mScrollOffset[1];
                        }
                    } else {
                        bool atOverscrollEdge =overScrollBy(0, overscroll, 0, mScrollY, 0, 0, 0, mOverscrollDistance, true);
                        if (atOverscrollEdge && mVelocityTracker != nullptr) {
                            // Don't allow overfling if we're at the edge
                            mVelocityTracker->clear();
                        }

                        int overscrollMode = getOverScrollMode();
                        if (overscrollMode == OVER_SCROLL_ALWAYS ||
                                (overscrollMode == OVER_SCROLL_IF_CONTENT_SCROLLS && !contentFits())) {
                            if (!atOverscrollEdge) {
                                mDirection = 0; // Reset when entering overscroll.
                                mTouchMode = TOUCH_MODE_OVERSCROLL;
                            }
                            if (incrementalDeltaY > 0) {
                                mEdgeGlowTop->onPullDistance((float) -overscroll / getHeight(), (float) x / getWidth());
                                if (!mEdgeGlowBottom->isFinished()) {
                                    mEdgeGlowBottom->onRelease();
                                }
                                invalidateTopGlow();
                            } else if (incrementalDeltaY < 0) {
                                mEdgeGlowBottom->onPullDistance((float) overscroll / getHeight(), 1.f - (float) x / getWidth());
                                if (!mEdgeGlowTop->isFinished()) {
                                    mEdgeGlowTop->onRelease();
                                }
                                invalidateBottomGlow();
                            }
                        }
                    }
                }
                mMotionY = y + lastYCorrection + scrollOffsetCorrection;
            }
            mLastY = y + lastYCorrection + scrollOffsetCorrection;
        }
    } else if (mTouchMode == TOUCH_MODE_OVERSCROLL/*5*/) {
        if (y != mLastY) {
            int oldScroll = mScrollY;
            int newScroll = oldScroll - incrementalDeltaY;
            int newDirection = y > mLastY ? 1 : -1;

            if (mDirection == 0) {
                mDirection = newDirection;
            }

            int overScrollDistance = -incrementalDeltaY;
            if ((newScroll < 0 && oldScroll >= 0) || (newScroll > 0 && oldScroll <= 0)) {
                overScrollDistance = -oldScroll;
                incrementalDeltaY += overScrollDistance;
            } else {
                incrementalDeltaY = 0;
            }

            if (overScrollDistance != 0) {
                overScrollBy(0, overScrollDistance, 0, mScrollY, 0, 0,
                             0, mOverscrollDistance, true);
                int overscrollMode = getOverScrollMode();
                if (overscrollMode == OVER_SCROLL_ALWAYS ||
                        (overscrollMode == OVER_SCROLL_IF_CONTENT_SCROLLS &&
                         !contentFits())) {
                    if (rawDeltaY > 0) {
                        mEdgeGlowTop->onPullDistance((float) overScrollDistance / getHeight(),
                                            (float) x / getWidth());
                        if (!mEdgeGlowBottom->isFinished()) {
                            mEdgeGlowBottom->onRelease();
                        }
                        invalidateTopGlow();
                    } else if (rawDeltaY < 0) {
                        mEdgeGlowBottom->onPullDistance((float) overScrollDistance / getHeight(),
                                               1.f - (float) x / getWidth());
                        if (!mEdgeGlowTop->isFinished()) {
                            mEdgeGlowTop->onRelease();
                        }
                        invalidateBottomGlow();
                    }
                }
            }

            if (incrementalDeltaY != 0) {
                // Coming back to 'real' list scrolling
                if (mScrollY != 0) {
                    mScrollY = 0;
                    invalidateParentIfNeeded();
                }
                
                trackMotionScroll(incrementalDeltaY, incrementalDeltaY);

                mTouchMode = TOUCH_MODE_SCROLL;

                // We did not scroll the full amount. Treat this essentially like the
                // start of a new touch scroll
                int motionPosition = findClosestMotionRow(y);

                mMotionCorrection = 0;
                View* motionView = getChildAt(motionPosition - mFirstPosition);
                mMotionViewOriginalTop = motionView ? motionView->getTop() : 0;
                mMotionY =  y + scrollOffsetCorrection;
                mMotionPosition = motionPosition;
            }
            mLastY = y + lastYCorrection + scrollOffsetCorrection;
            mDirection = newDirection;
        }//if (y != mLastY)
    }
}

int AbsListView::releaseGlow(int deltaY, int x) {
        // First allow releasing existing overscroll effect:
    float consumed = .0f;
    if (mEdgeGlowTop->getDistance() != .0f) {
        consumed = mEdgeGlowTop->onPullDistance((float) deltaY / getHeight(),
                (float) x / getWidth());
        if (consumed != .0f) {
            invalidateTopGlow();
        }
    } else if (mEdgeGlowBottom->getDistance() != .0f) {
        consumed = -mEdgeGlowBottom->onPullDistance((float) -deltaY / getHeight(),
                1.f - (float) x / getWidth());
        if (consumed != .0f) {
            invalidateBottomGlow();
        }
    }
    int pixelsConsumed = round(consumed * getHeight());
    return deltaY - pixelsConsumed;
}

/**
 * @return <code>true</code> if either the top or bottom edge glow is currently active or
 * <code>false</code> if it has no value to release.
 */
bool AbsListView::isGlowActive()const{
    return mEdgeGlowBottom->getDistance() != 0 || mEdgeGlowTop->getDistance() != 0;
}

void AbsListView::invalidateTopGlow() {
    if (mEdgeGlowTop == nullptr) return;

    bool clipToPadding = getClipToPadding();
    int top = clipToPadding ? mPaddingTop : 0;
    int left = clipToPadding ? mPaddingLeft : 0;
    int width = clipToPadding ? getWidth() - mPaddingRight-mPaddingLeft : getWidth();
    Rect rect=Rect::Make(left,top,width,mEdgeGlowTop->getMaxHeight());
    invalidate(&rect);
}

void AbsListView::invalidateBottomGlow() {
    if (mEdgeGlowBottom == nullptr) return;

    bool clipToPadding = getClipToPadding();
    int bottom = clipToPadding ? getHeight() - mPaddingBottom : getHeight();
    int left = clipToPadding ? mPaddingLeft : 0;
    int width = clipToPadding ? getWidth() - mPaddingRight-mPaddingLeft : getWidth();
    Rect rect=Rect::Make(left,bottom-mEdgeGlowBottom->getMaxHeight(),width,mEdgeGlowBottom->getMaxHeight());
    invalidate(&rect);
}

void AbsListView::finishGlows() {
    if (shouldDisplayEdgeEffects()) {
        mEdgeGlowTop->finish();
        mEdgeGlowBottom->finish();
    }
}

void AbsListView::onTouchDown(MotionEvent& ev) {
    mHasPerformedLongPress = false;
    mActivePointerId = ev.getPointerId(0);
    hideSelector();
    
    if (mTouchMode == TOUCH_MODE_OVERFLING) {
        // Stopped the fling. It is a scroll.
        FLY_endFling();
        if (mPositionScroller)  mPositionScroller->stop();

        mTouchMode = TOUCH_MODE_OVERSCROLL;
        mMotionX = (int) ev.getX();
        mMotionY = (int) ev.getY();
        mLastY = mMotionY;
        mMotionCorrection = 0;
        mDirection = 0;
    } else {
        int x = (int) ev.getX();
        int y = (int) ev.getY();
        int motionPosition = pointToPosition(x, y);

        if (!mDataChanged) {
            if (mTouchMode == TOUCH_MODE_FLING) {
                // Stopped a fling. It is a scroll.
                createScrollingCache();
                mTouchMode = TOUCH_MODE_SCROLL;
                mMotionCorrection = 0;
                motionPosition = findMotionRow(y);
                FLY_wheelTouch();//mFlingRunnable.flywheelTouch();
            } else if ((motionPosition >= 0) && getAdapter()->isEnabled(motionPosition)) {
                // User clicked on an actual view (and was not stopping a
                // fling). It might be a click or a scroll. Assume it is a
                // click until proven otherwise.
                mTouchMode = TOUCH_MODE_DOWN;

                // FIXME Debounce
                mPendingCheckForTap = std::bind(&AbsListView::checkTap,this,ev.getX(),ev.getY());
                postDelayed(mPendingCheckForTap,ViewConfiguration::getTapTimeout());
            }
        }

        if (motionPosition >= 0) {
            // Remember where the motion event started
            View* v = getChildAt(motionPosition - mFirstPosition);
            mMotionViewOriginalTop = v->getTop();
        }

        mMotionX = x;
        mMotionY = y;
        mMotionPosition = motionPosition;
        mLastY = INT_MIN;
    }
    if (mTouchMode == TOUCH_MODE_DOWN && mMotionPosition != INVALID_POSITION
            && performButtonActionOnTouchDown(ev)) {
        removeCallbacks(mPendingCheckForTap);
    }
}

void AbsListView::onTouchMove(MotionEvent&ev, MotionEvent&vtev) {
    if (mHasPerformedLongPress) {
        // Consume all move events following a successful long press.
        return;
    }

    int pointerIndex = ev.findPointerIndex(mActivePointerId);
    if (pointerIndex == -1) {
        pointerIndex = 0;
        mActivePointerId = ev.getPointerId(pointerIndex);
    }

    if (mDataChanged) {
        // Re-sync everything if data has been changed
        // since the scroll operation can query the adapter.
        layoutChildren();
    }
    View* motionView;
    int x;
    int y = (int) ev.getY(pointerIndex);
    switch (mTouchMode) {
    case TOUCH_MODE_DOWN:/*0*/
    case TOUCH_MODE_TAP :/*1*/ 
    case TOUCH_MODE_DONE_WAITING:/*2*/
        // Check if we have moved far enough that it looks more like a
        // scroll than a tap. If so, we'll enter scrolling mode.
        if (startScrollIfNeeded((int) ev.getX(pointerIndex), y, &vtev)) {
            break;
        }
        // Otherwise, check containment within list bounds. If we're
        // outside bounds, cancel any active presses.
        motionView = getChildAt(mMotionPosition - mFirstPosition);
        x = ev.getX(pointerIndex);
        if (!pointInView(x, y, mTouchSlop)) {
            setPressed(false);
            if (motionView) motionView->setPressed(false);
            removeCallbacks(mTouchMode == TOUCH_MODE_DOWN ? mPendingCheckForTap : mPendingCheckForLongPress);
            mTouchMode = TOUCH_MODE_DONE_WAITING;
            updateSelectorState();
        } else if (motionView != nullptr) {
            // Still within bounds, update the hotspot.
            float point[2]= {(float)x,(float)y};
            transformPointToViewLocal(point, *motionView);
            //motionView->drawableHotspotChanged(point[0], point[1]);
        }
        break;
    case TOUCH_MODE_SCROLL:/*3*/
    case TOUCH_MODE_OVERSCROLL:/*5*/
        scrollIfNeeded((int) ev.getX(pointerIndex), y, &vtev);
        break;
    }
}

void AbsListView::setVisibleRangeHint(int start,int end) {
    //nothing,for android's remote view
}

//Sets the edge effect color for both top and bottom edge effects.
void AbsListView::setEdgeEffectColor(int color) {
    setTopEdgeEffectColor(color);
    setBottomEdgeEffectColor(color);
}

/**
 * Sets the bottom edge effect color.
 */
void AbsListView::setBottomEdgeEffectColor( int color) {
    mEdgeGlowBottom->setColor(color);
    invalidateBottomGlow();
}

/**
 * Sets the top edge effect color.
 */
void AbsListView::setTopEdgeEffectColor(int color) {
    mEdgeGlowTop->setColor(color);
    invalidateTopGlow();
}

/**
 * Returns the top edge effect color.
 */
int AbsListView::getTopEdgeEffectColor()const{
    return mEdgeGlowTop->getColor();
}

/**
 * @return The bottom edge effect color.
 */
int AbsListView::getBottomEdgeEffectColor()const{
    return mEdgeGlowBottom->getColor();
}

void AbsListView::onTouchUp(MotionEvent&ev) {
    View*child;
    int childCount;
    switch (mTouchMode) {
    case TOUCH_MODE_DOWN:/*0*/
    case TOUCH_MODE_TAP :/*1*/
    case TOUCH_MODE_DONE_WAITING:/*2*/
        child = getChildAt(mMotionPosition - mFirstPosition);
        if (child != nullptr) {
            if (mTouchMode != TOUCH_MODE_DOWN) {
                child->setPressed(false);
            }

            float x = ev.getX();
            bool inList = x > mListPadding.left && x < getWidth() - mListPadding.width;
            if (inList && !child->hasExplicitFocusable()) {

                mResurrectToPosition = mMotionPosition;

                if (mTouchMode == TOUCH_MODE_DOWN || mTouchMode == TOUCH_MODE_TAP) {
                    removeCallbacks(mTouchMode == TOUCH_MODE_DOWN ? mPendingCheckForTap : mPendingCheckForLongPress);
                    mLayoutMode = LAYOUT_NORMAL;
                    if (!mDataChanged && mAdapter->isEnabled(mMotionPosition)) {
                        mTouchMode = TOUCH_MODE_TAP;
                        setSelectedPositionInt(mMotionPosition);
                        layoutChildren();
                        child->setPressed(true);
                        positionSelector(mMotionPosition, child);
                        setPressed(true);
                        if (mSelector != nullptr) {
                            Drawable* d = mSelector->getCurrent();
                            if (d  && dynamic_cast<TransitionDrawable*>(d)) {
                                ((TransitionDrawable*) d)->resetTransition();
                            }
                            mSelector->setHotspot(x, ev.getY());
                        }
                        if (mTouchModeReset != nullptr) {
                            removeCallbacks(mTouchModeReset);
                        }
                        mTouchModeReset =[this,child](){
                             mTouchModeReset = nullptr;
                             mTouchMode = TOUCH_MODE_REST;
                             child->setPressed(false);
                             setPressed(false);
                             if (!mDataChanged && !mIsDetaching && isAttachedToWindow()) {
                                 doClick(mMotionPosition);//performClick.run();
                             }
                        };
                        postDelayed(mTouchModeReset,ViewConfiguration::getPressedStateDuration());
                    } else {
                        mTouchMode = TOUCH_MODE_REST;
                        updateSelectorState();
                    }
                    return;
                } else if (!mDataChanged && mAdapter->isEnabled(mMotionPosition)) {
                    doClick(mMotionPosition);//performClick.run();
                }
            }
        }
        mTouchMode = TOUCH_MODE_REST;
        updateSelectorState();
        break;
    case TOUCH_MODE_SCROLL:/*3*/
        childCount = getChildCount();
        if (childCount > 0) {
            int firstChildTop = getChildAt(0)->getTop();
            int lastChildBottom = getChildAt(childCount - 1)->getBottom();
            int contentTop = mListPadding.top;
            int contentBottom = getHeight() - mListPadding.width;
            if (mFirstPosition == 0 && firstChildTop >= contentTop &&
                    mFirstPosition + childCount < mItemCount &&
                    lastChildBottom <= getHeight() - contentBottom) {
                mTouchMode = TOUCH_MODE_REST;
                reportScrollStateChange(OnScrollListener::SCROLL_STATE_IDLE);
            } else {
                mVelocityTracker->computeCurrentVelocity(1000, mMaximumVelocity);

                int initialVelocity = (int)(mVelocityTracker->getYVelocity(mActivePointerId) * mVelocityScale);
                // Fling if we have enough velocity and we aren't at a boundary.
                // Since we can potentially overfling more than we can overscroll, don't
                // allow the weird behavior where you can scroll to a boundary then
                // fling further.
                LOGV("Velocity [%d]%d:(%d,%d)",mActivePointerId,initialVelocity,mMinimumVelocity,mMaximumVelocity);
                bool flingVelocity = std::abs(initialVelocity) > mMinimumVelocity;
                if (flingVelocity && !((mFirstPosition == 0 &&
                        firstChildTop == contentTop - mOverscrollDistance) ||
                        (mFirstPosition + childCount == mItemCount &&
                        lastChildBottom == contentBottom + mOverscrollDistance))) {
                    if (!dispatchNestedPreFling(0, -initialVelocity)) {
                        reportScrollStateChange(OnScrollListener::SCROLL_STATE_FLING);
                        FLY_start(-initialVelocity);
                        dispatchNestedFling(0, -initialVelocity, true);
                    } else {
                        mTouchMode = TOUCH_MODE_REST;
                        reportScrollStateChange(OnScrollListener::SCROLL_STATE_IDLE);
                    }
                } else {
                    mTouchMode = TOUCH_MODE_REST;
                    reportScrollStateChange(OnScrollListener::SCROLL_STATE_IDLE);
                    if (mFlingRunnable) {
                        FLY_endFling();
                    }
                    if (mPositionScroller) mPositionScroller->stop();
                    if (flingVelocity && !dispatchNestedPreFling(0, -initialVelocity)) {
                        dispatchNestedFling(0, -initialVelocity, false);
                    }
                }
            }
        } else {
            mTouchMode = TOUCH_MODE_REST;
            reportScrollStateChange(OnScrollListener::SCROLL_STATE_IDLE);
        }
        break;

    case TOUCH_MODE_OVERSCROLL:/*5*/
        mVelocityTracker->computeCurrentVelocity(1000, mMaximumVelocity);
        int initialVelocity = (int) mVelocityTracker->getYVelocity(mActivePointerId);

        reportScrollStateChange(OnScrollListener::SCROLL_STATE_FLING);
        if (std::abs(initialVelocity) > mMinimumVelocity) {
            FLY_startOverfling(-initialVelocity);
        } else {
            FLY_startSpringback();
        }

        break;
    }

    setPressed(false);

    if (shouldDisplayEdgeEffects()) {
        mEdgeGlowTop->onRelease();
        mEdgeGlowBottom->onRelease();
    }
    finishGlows();
    // Need to redraw since we probably aren't drawing the selector anymore
    invalidate();
    removeCallbacks(mPendingCheckForLongPress);
    recycleVelocityTracker();

    mActivePointerId = INVALID_POINTER;

    if (PROFILE_SCROLLING) {
        if (mScrollProfilingStarted) {
            mScrollProfilingStarted = false;
        }
    }

    /*if (mScrollStrictSpan != nullptr) {
        mScrollStrictSpan.finish();
        mScrollStrictSpan = nullptr;
    }*/
}

bool AbsListView::shouldDisplayEdgeEffects()const{
    return getOverScrollMode() != OVER_SCROLL_NEVER;
}

void AbsListView::onTouchCancel() {
    switch (mTouchMode) {
    case TOUCH_MODE_OVERSCROLL:
        FLY_startSpringback();
        break;

    case TOUCH_MODE_OVERFLING:// Do nothing - let it play out.
        break;

    default:
        mTouchMode = TOUCH_MODE_REST;
        setPressed(false);
        View* motionView = getChildAt(mMotionPosition - mFirstPosition);
        if (motionView != nullptr) {
            motionView->setPressed(false);
        }
        clearScrollingCache();
        removeCallbacks(mPendingCheckForLongPress);
        recycleVelocityTracker();
    }
    if (shouldDisplayEdgeEffects()) {
        mEdgeGlowTop->onRelease();
        mEdgeGlowBottom->onRelease();
    }
    mActivePointerId = INVALID_POINTER;
}

void AbsListView::onOverScrolled(int scrollX, int scrollY, bool clampedX, bool clampedY) {
    if (mScrollY != scrollY) {
        onScrollChanged(mScrollX, scrollY, mScrollX, mScrollY);
        mScrollY = scrollY;
        if(mParent)mParent->invalidate(true);
        awakenScrollBars();
    }
}

void AbsListView::setFriction(float friction){
    mScroller->setFriction(friction);
}

void AbsListView::setVelocityScale(float scale) {
    mVelocityScale = scale;
}

void AbsListView::smoothScrollToPosition(int position){
    if (mPositionScroller == nullptr) {
        mPositionScroller = createPositionScroller();
    }
    mPositionScroller->start(position);
}

AbsListView::AbsPositionScroller* AbsListView::createPositionScroller(){
    return new PositionScroller(this);
}

void AbsListView::smoothScrollToPositionFromTop(int position, int offset, int duration){
    if (mPositionScroller == nullptr) {
        mPositionScroller = createPositionScroller();
    }
    mPositionScroller->startWithOffset(position, offset, duration);
}

void AbsListView::smoothScrollToPositionFromTop(int position, int offset){
    if (mPositionScroller == nullptr) {
        mPositionScroller = createPositionScroller();
    }
    mPositionScroller->startWithOffset(position, offset);
}

void AbsListView::smoothScrollToPosition(int position, int boundPosition){
    if (mPositionScroller == nullptr) {
        mPositionScroller = createPositionScroller();
    }
    mPositionScroller->start(position, boundPosition);
}

void AbsListView::smoothScrollBy(int distance, int duration){
    smoothScrollBy(distance, duration, false, false);
}

void AbsListView::smoothScrollBy(int distance, int duration, bool linear,bool suppressEndFlingStateChangeCall){
    // No sense starting to scroll if we're not going anywhere
    int firstPos = mFirstPosition;
    int childCount = getChildCount();
    int lastPos = firstPos + childCount;
    int topLimit = getPaddingTop();
    int bottomLimit = getHeight() - getPaddingBottom();

    if (distance == 0 || mItemCount == 0 || childCount == 0 ||
            (firstPos == 0 && getChildAt(0)->getTop() == topLimit && distance < 0) ||
            (lastPos == mItemCount &&
                    getChildAt(childCount - 1)->getBottom() == bottomLimit && distance > 0)) {
        FLY_endFling();
        if (mPositionScroller != nullptr) {
            mPositionScroller->stop();
        }
    } else {
        reportScrollStateChange(OnScrollListener::SCROLL_STATE_FLING);
        FLY_startScroll(distance, duration, linear, suppressEndFlingStateChangeCall);
    }
}

void AbsListView::smoothScrollByOffset(int position){
    int index = -1;
    if (position < 0) {
        index = getFirstVisiblePosition();
    } else if (position > 0) {
        index = getLastVisiblePosition();
    }

    if (index > -1) {
        View* child = getChildAt(index - getFirstVisiblePosition());
        if (child != nullptr) {
            Rect visibleRect;
            if (1){//child->getGlobalVisibleRect(visibleRect)) {
                // the child is partially visible
                int childRectArea = child->getWidth() * child->getHeight();
                int visibleRectArea = visibleRect.width * visibleRect.height;
                float visibleArea = (visibleRectArea / (float) childRectArea);
                float visibleThreshold = 0.75f;
                if ((position < 0) && (visibleArea < visibleThreshold)) {
                    // the top index is not perceivably visible so offset
                    // to account for showing that top index as well
                    ++index;
                } else if ((position > 0) && (visibleArea < visibleThreshold)) {
                    // the bottom index is not perceivably visible so offset
                    // to account for showing that bottom index as well
                    --index;
                }
            }
            smoothScrollToPosition(std::max(0, std::min(getCount(), index + position)));
        }
    }
}

void AbsListView::createScrollingCache(){
    if (mScrollingCacheEnabled && !mCachingStarted /*&& !isHardwareAccelerated()*/) {
        setChildrenDrawnWithCacheEnabled(true);
        setChildrenDrawingCacheEnabled(true);
        mCachingStarted = mCachingActive = true;
    }
}

void AbsListView::clearScrollingCache(){
    if (mClearScrollingCache == nullptr) {
        mClearScrollingCache =[this]() {
            if (mCachingStarted) {
                mCachingStarted = mCachingActive = false;
                setChildrenDrawnWithCacheEnabled(false);
                if ((mPersistentDrawingCache & PERSISTENT_SCROLLING_CACHE) == 0) {
                    setChildrenDrawingCacheEnabled(false);
                }
                if (!isAlwaysDrawnWithCacheEnabled()) {
                    invalidate();
                }
            }
       };
    }
    post(mClearScrollingCache);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

AbsListView::PositionScroller::PositionScroller(AbsListView*lv) {
    mExtraScroll = ViewConfiguration::get(lv->getContext()).getScaledFadingEdgeLength();
    mLV=lv;
}

void AbsListView::PositionScroller::start(int position) {
    stop();

    if (mLV->mDataChanged) {
        // Wait until we're back in a stable state to try this.
        mLV->mPositionScrollAfterLayout =[this,position](){
            start(position);
        };
        return;
    }

    int childCount = mLV->getChildCount();
    if (childCount == 0) {
        // Can't scroll without children.
        return;
    }

    int firstPos = mLV->mFirstPosition;
    int lastPos = firstPos + childCount - 1;

    int viewTravelCount;
    int clampedPosition = std::max(0, std::min(mLV->getCount() - 1, position));
    if (clampedPosition < firstPos) {
        viewTravelCount = firstPos - clampedPosition + 1;
        mMode = MOVE_UP_POS;
    } else if (clampedPosition > lastPos) {
        viewTravelCount = clampedPosition - lastPos + 1;
        mMode = MOVE_DOWN_POS;
    } else {
        scrollToVisible(clampedPosition, INVALID_POSITION, SCROLL_DURATION);
        return;
    }

    if (viewTravelCount > 0) {
        mScrollDuration = SCROLL_DURATION / viewTravelCount;
    } else {
        mScrollDuration = SCROLL_DURATION;
    }
    mTargetPos = clampedPosition;
    mBoundPos = INVALID_POSITION;
    mLastSeenPos = INVALID_POSITION;

    mLV->postOnAnimation(*this);
}

void AbsListView::PositionScroller::start(int position, int boundPosition) {
    stop();

    if (boundPosition == INVALID_POSITION) {
        start(position);
        return;
    }

    if (mLV->mDataChanged) {
        // Wait until we're back in a stable state to try this.
        mLV->mPositionScrollAfterLayout =[this,position,boundPosition](){
            start(position, boundPosition);
        };
        return;
    }

    int childCount = mLV->getChildCount();
    if (childCount == 0) {
        // Can't scroll without children.
        return;
    }

    int firstPos = mLV->mFirstPosition;
    int lastPos = firstPos + childCount - 1;

    int viewTravelCount;
    int clampedPosition = std::max(0, std::min(mLV->getCount() - 1, position));
    if (clampedPosition < firstPos) {
        int boundPosFromLast = lastPos - boundPosition;
        if (boundPosFromLast < 1) {
            // Moving would shift our bound position off the screen. Abort.
            return;
        }

        int posTravel = firstPos - clampedPosition + 1;
        int boundTravel = boundPosFromLast - 1;
        if (boundTravel < posTravel) {
            viewTravelCount = boundTravel;
            mMode = MOVE_UP_BOUND;
        } else {
            viewTravelCount = posTravel;
            mMode = MOVE_UP_POS;
        }
    } else if (clampedPosition > lastPos) {
        int boundPosFromFirst = boundPosition - firstPos;
        if (boundPosFromFirst < 1) {
            // Moving would shift our bound position off the screen. Abort.
            return;
        }

        int posTravel = clampedPosition - lastPos + 1;
        int boundTravel = boundPosFromFirst - 1;
        if (boundTravel < posTravel) {
            viewTravelCount = boundTravel;
            mMode = MOVE_DOWN_BOUND;
        } else {
            viewTravelCount = posTravel;
            mMode = MOVE_DOWN_POS;
        }
    } else {
        scrollToVisible(clampedPosition, boundPosition, SCROLL_DURATION);
        return;
    }

    if (viewTravelCount > 0) {
        mScrollDuration = SCROLL_DURATION / viewTravelCount;
    } else {
        mScrollDuration = SCROLL_DURATION;
    }
    mTargetPos = clampedPosition;
    mBoundPos = boundPosition;
    mLastSeenPos = INVALID_POSITION;

    mLV->postOnAnimation(*this);
}

void AbsListView::PositionScroller::startWithOffset(int position, int offset) {
    startWithOffset(position, offset, SCROLL_DURATION);
}

void AbsListView::PositionScroller::startWithOffset(int position, int offset, int duration) {
    stop();

    if (mLV->mDataChanged) {
        // Wait until we're back in a stable state to try this.
        mLV->mPositionScrollAfterLayout =[this,position,offset,duration](){
            startWithOffset(position, offset, duration);
        };
        return;
    }

    int childCount = mLV->getChildCount();
    if (childCount == 0) {
        // Can't scroll without children.
        return;
    }

    offset += mLV->getPaddingTop();

    mTargetPos = std::max(0, std::min(mLV->getCount() - 1, position));
    mOffsetFromTop = offset;
    mBoundPos = INVALID_POSITION;
    mLastSeenPos = INVALID_POSITION;
    mMode = MOVE_OFFSET;

    int firstPos = mLV->mFirstPosition;
    int lastPos = firstPos + childCount - 1;

    int viewTravelCount;
    if (mTargetPos < firstPos) {
        viewTravelCount = firstPos - mTargetPos;
    } else if (mTargetPos > lastPos) {
        viewTravelCount = mTargetPos - lastPos;
    } else {
        // On-screen, just scroll.
        int targetTop = mLV->getChildAt(mTargetPos - firstPos)->getTop();
        mLV->smoothScrollBy(targetTop - offset, duration, true, false);
        return;
    }

    // Estimate how many screens we should travel
    float screenTravelCount = (float) viewTravelCount / childCount;
    mScrollDuration = screenTravelCount < 1 ?
            duration : (int) (duration / screenTravelCount);
    mLastSeenPos = INVALID_POSITION;

    mLV->postOnAnimation(*this);
}

/**
 * Scroll such that targetPos is in the visible padded region without scrolling
 * boundPos out of view. Assumes targetPos is onscreen.
 */
void AbsListView::PositionScroller::scrollToVisible(int targetPos, int boundPos, int duration) {
    int firstPos = mLV->mFirstPosition;
    int childCount = mLV->getChildCount();
    int lastPos = firstPos + childCount - 1;
    int paddedTop = mLV->mListPadding.top;
    int paddedBottom = mLV->getHeight() - mLV->mListPadding.bottom();

    if (targetPos < firstPos || targetPos > lastPos) {
        LOGW("scrollToVisible called with targetPos %d not visible[%d,%d]",targetPos,firstPos,lastPos);
    }
    if (boundPos < firstPos || boundPos > lastPos) {
        // boundPos doesn't matter, it's already offscreen.
        boundPos = INVALID_POSITION;
    }

    View* targetChild = mLV->getChildAt(targetPos - firstPos);
    int targetTop = targetChild->getTop();
    int targetBottom = targetChild->getBottom();
    int scrollBy = 0;

    if (targetBottom > paddedBottom) {
        scrollBy = targetBottom - paddedBottom;
    }
    if (targetTop < paddedTop) {
        scrollBy = targetTop - paddedTop;
    }

    if (scrollBy == 0) {
        return;
    }

    if (boundPos >= 0) {
        View* boundChild = mLV->getChildAt(boundPos - firstPos);
        int boundTop = boundChild->getTop();
        int boundBottom = boundChild->getBottom();
        int absScroll = std::abs(scrollBy);

        if (scrollBy < 0 && boundBottom + absScroll > paddedBottom) {
            // Don't scroll the bound view off the bottom of the screen.
            scrollBy = std::max(0, boundBottom - paddedBottom);
        } else if (scrollBy > 0 && boundTop - absScroll < paddedTop) {
            // Don't scroll the bound view off the top of the screen.
            scrollBy = std::min(0, boundTop - paddedTop);
        }
    }

    mLV->smoothScrollBy(scrollBy, duration);
}

void AbsListView::PositionScroller::stop() {
    mLV->removeCallbacks(*this);
}

void AbsListView::PositionScroller::operator()() {
    int listHeight = mLV->getHeight();
    int firstPos = mLV->mFirstPosition;
    switch (mMode) {
    case MOVE_DOWN_POS: {
        int lastViewIndex = mLV->getChildCount() - 1;
        int lastPos = firstPos + lastViewIndex;

        if (lastViewIndex < 0) {
            return;
        }

        if (lastPos == mLastSeenPos) {
            // No new views, let things keep going.
            mLV->postOnAnimation(*this);
            return;
        }

        View* lastView = mLV->getChildAt(lastViewIndex);
        int lastViewHeight = lastView->getHeight();
        int lastViewTop = lastView->getTop();
        int lastViewPixelsShowing = listHeight - lastViewTop;
        int extraScroll = lastPos < mLV->mItemCount - 1 ?
             std::max(mLV->mListPadding.bottom(), mExtraScroll) : mLV->mListPadding.bottom();

        int scrollBy = lastViewHeight - lastViewPixelsShowing + extraScroll;
        mLV->smoothScrollBy(scrollBy, mScrollDuration, true, lastPos < mTargetPos);

        mLastSeenPos = lastPos;
        if (lastPos < mTargetPos) {
            mLV->postOnAnimation(*this);
        }
        break;
    }

    case MOVE_DOWN_BOUND: {
        int nextViewIndex = 1;
        int childCount = mLV->getChildCount();

        if (firstPos == mBoundPos || childCount <= nextViewIndex
                || firstPos + childCount >= mLV->mItemCount) {
            mLV->reportScrollStateChange(OnScrollListener::SCROLL_STATE_IDLE);
            return;
        }
        int nextPos = firstPos + nextViewIndex;

        if (nextPos == mLastSeenPos) {
            // No new views, let things keep going.
            mLV->postOnAnimation(*this);
            return;
        }

        View* nextView = mLV->getChildAt(nextViewIndex);
        int nextViewHeight = nextView->getHeight();
        int nextViewTop = nextView->getTop();
        int extraScroll = std::max(mLV->mListPadding.bottom(), mExtraScroll);
        if (nextPos < mBoundPos) {
            mLV->smoothScrollBy(std::max(0, nextViewHeight + nextViewTop - extraScroll),
                    mScrollDuration, true, true);

            mLastSeenPos = nextPos;

            mLV->postOnAnimation(*this);
        } else  {
            if (nextViewTop > extraScroll) {
                mLV->smoothScrollBy(nextViewTop - extraScroll, mScrollDuration, true, false);
            } else {
                mLV->reportScrollStateChange(OnScrollListener::SCROLL_STATE_IDLE);
            }
        }
        break;
    }

    case MOVE_UP_POS: {
        if (firstPos == mLastSeenPos) {
            // No new views, let things keep going.
            mLV->postOnAnimation(*this);
            return;
        }

        View* firstView = mLV->getChildAt(0);
        if (firstView == nullptr) {
            return;
        }
        int firstViewTop = firstView->getTop();
        int extraScroll = firstPos > 0 ?
              std::max(mExtraScroll, mLV->mListPadding.top) : mLV->mListPadding.top;

        mLV->smoothScrollBy(firstViewTop - extraScroll, mScrollDuration, true,
                firstPos > mTargetPos);

        mLastSeenPos = firstPos;

        if (firstPos > mTargetPos) {
            mLV->postOnAnimation(*this);
        }
        break;
    }

    case MOVE_UP_BOUND: {
        int lastViewIndex = mLV->getChildCount() - 2;
        if (lastViewIndex < 0) {
            return;
        }
        int lastPos = firstPos + lastViewIndex;

        if (lastPos == mLastSeenPos) {
            // No new views, let things keep going.
            mLV->postOnAnimation(*this);
            return;
        }

        View* lastView = mLV->getChildAt(lastViewIndex);
        int lastViewHeight = lastView->getHeight();
        int lastViewTop = lastView->getTop();
        int lastViewPixelsShowing = listHeight - lastViewTop;
        int extraScroll = std::max(mLV->mListPadding.top, mExtraScroll);
        mLastSeenPos = lastPos;
        if (lastPos > mBoundPos) {
            mLV->smoothScrollBy(-(lastViewPixelsShowing - extraScroll), mScrollDuration, true,
                    true);
            mLV->postOnAnimation(*this);
        } else {
            int bottom = listHeight - extraScroll;
            int lastViewBottom = lastViewTop + lastViewHeight;
            if (bottom > lastViewBottom) {
                mLV->smoothScrollBy(-(bottom - lastViewBottom), mScrollDuration, true, false);
            } else {
                mLV->reportScrollStateChange(OnScrollListener::SCROLL_STATE_IDLE);
            }
        }
        break;
    }

    case MOVE_OFFSET: {
        if (mLastSeenPos == firstPos) {
            // No new views, let things keep going.
            mLV->postOnAnimation(*this);
            return;
        }

        mLastSeenPos = firstPos;

        int childCount = mLV->getChildCount();
        int position = mTargetPos;
        int lastPos = firstPos + childCount - 1;

        // Account for the visible "portion" of the first / last child when we estimate
        // how many screens we should travel to reach our target
        View* firstChild = mLV->getChildAt(0);
        int firstChildHeight = firstChild->getHeight();
        View* lastChild = mLV->getChildAt(childCount - 1);
        int lastChildHeight = lastChild->getHeight();
        float firstPositionVisiblePart = (firstChildHeight == 0.0f) ? 1.0f
                : (float) (firstChildHeight + firstChild->getTop()) / firstChildHeight;
        float lastPositionVisiblePart = (lastChildHeight == 0.0f) ? 1.0f
                : (float) (lastChildHeight + mLV->getHeight() - lastChild->getBottom())
                        / lastChildHeight;

        float viewTravelCount = 0;
        if (position < firstPos) {
            viewTravelCount = firstPos - position + (1.0f - firstPositionVisiblePart) + 1;
        } else if (position > lastPos) {
            viewTravelCount = position - lastPos + (1.0f - lastPositionVisiblePart);
        }

        // Estimate how many screens we should travel
        float screenTravelCount = viewTravelCount / childCount;

        float modifier = std::min(std::abs(screenTravelCount), 1.f);
        if (position < firstPos) {
            int distance = (int) (-mLV->getHeight() * modifier);
            int duration = (int) (mScrollDuration * modifier);
            mLV->smoothScrollBy(distance, duration, true, true);
            mLV->postOnAnimation(*this);
        } else if (position > lastPos) {
            int distance = (int) (mLV->getHeight() * modifier);
            int duration = (int) (mScrollDuration * modifier);
            mLV->smoothScrollBy(distance, duration, true, true);
            mLV->postOnAnimation(*this);
        } else {
            // On-screen, just scroll.
            int targetTop = mLV->getChildAt(position - firstPos)->getTop();
            int distance = targetTop - mOffsetFromTop;
            int duration = (int) (mScrollDuration *
                        ((float) std::abs(distance) / mLV->getHeight()));
            mLV->smoothScrollBy(distance, duration, true, false);
        }
        break;
    }
    default:  break;
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////

AbsListView::LayoutParams::LayoutParams():ViewGroup::LayoutParams() {
    init();
}

void AbsListView::LayoutParams::init(){
    itemId=-1;
    viewType=0;
    recycledHeaderFooter=false;
    scrappedFromPosition=false;
    forceAdd=false;
}

AbsListView::LayoutParams::LayoutParams(const ViewGroup::LayoutParams&p):ViewGroup::LayoutParams(p) {
    init();
}

AbsListView::LayoutParams::LayoutParams(int w,int h):ViewGroup::LayoutParams(w,h) {
    init();
}

AbsListView::LayoutParams::LayoutParams(int w, int h, int vt):ViewGroup::LayoutParams(w,h) {
    init();
    viewType = vt;
}

AbsListView::LayoutParams::LayoutParams(Context*ctx,const AttributeSet&atts):ViewGroup::LayoutParams(ctx,atts){
    init();
}

}//namespace

