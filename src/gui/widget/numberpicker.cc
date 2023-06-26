#include <widget/numberpicker.h>
#include <widget/R.h>
#include <color.h>
#include <textutils.h>
#include <cdlog.h>

//https://gitee.com/awang/WheelView/blob/master/src/com/wangjie/wheelview/WheelView.java

namespace cdroid{

DECLARE_WIDGET2(NumberPicker,"cdroid:attr/numberPickerStyle")
const std::string DEFAULT_LAYOUT_VERT="cdroid:layout/number_picker";
const std::string DEFAULT_LAYOUT_HORZ="cdroid:layout/number_picker_horz";

NumberPicker::NumberPicker(int w,int h):LinearLayout(w,h){
    initView();
    setOrientation(h>w?VERTICAL:HORIZONTAL);

    const std::string layoutres = (getOrientation()==VERTICAL)?DEFAULT_LAYOUT_VERT:DEFAULT_LAYOUT_HORZ;
    LayoutInflater::from(mContext)->inflate(layoutres,this,true);
 
    mSelectedText =(EditText*)findViewById(R::id::numberpicker_input);
    if(mSelectedText){
        mSelectedText->setTextAlignment(View::TEXT_ALIGNMENT_CENTER);
        mSelectedTextSize = mSelectedText->getTextSize();
        mTextSize = mSelectedTextSize;
        mSelectorElementSize = mSelectedTextSize;
    }
    setWidthAndHeight();
    mComputeMaxWidth = (mMaxWidth == SIZE_UNSPECIFIED);
    measure(MeasureSpec::makeMeasureSpec(w,MeasureSpec::EXACTLY),MeasureSpec::makeMeasureSpec(h,MeasureSpec::EXACTLY));
    layout(0,0,getMeasuredWidth(),getMeasuredHeight());
    updateInputTextView();
    setFocusable(int(View::FOCUSABLE));
    setFocusableInTouchMode(true);
}

NumberPicker::NumberPicker(Context* context,const AttributeSet& atts)
  :LinearLayout(context,atts){
    initView();
    mHideWheelUntilFocused = atts.getBoolean("hideWheelUntilFocused",false);
    mWrapSelectorWheel= atts.getBoolean("wrapSelectorWheel",false); 
    mDividerDrawable = atts.getDrawable("selectionDivider");
    mSelectedTextSize = atts.getDimensionPixelSize("selectedTextSize",mSelectedTextSize);
    if (mDividerDrawable) {
        mDividerDrawable->setCallback(this);
        mDividerDrawable->setLayoutDirection(getLayoutDirection());
        if (mDividerDrawable->isStateful()) {
            mDividerDrawable->setState(getDrawableState());
        }
    }else{
        setDividerColor(atts.getColor("dividerColor",mDividerColor));
    }
    mDividerDistance = atts.getDimensionPixelSize("dividerDistance",UNSCALED_DEFAULT_SELECTION_DIVIDERS_DISTANCE);
    mDividerLength = atts.getDimensionPixelSize("dividerLength",0);
    mOrder = ASCENDING;
    mSelectionDividerHeight = atts.getDimensionPixelSize("selectionDividerHeight",UNSCALED_DEFAULT_SELECTION_DIVIDER_HEIGHT);
    mSelectionDividersDistance=atts.getDimensionPixelSize("selectionDividersDistance",UNSCALED_DEFAULT_SELECTION_DIVIDERS_DISTANCE);
    mMinHeight = atts.getDimensionPixelSize("internalMinHeight",SIZE_UNSPECIFIED);
    mMaxHeight = atts.getDimensionPixelSize("internalMaxHeight",SIZE_UNSPECIFIED);
    
    mMinWidth = atts.getDimensionPixelSize("internalMinWidth", SIZE_UNSPECIFIED);
    mMaxWidth = atts.getDimensionPixelSize("internalMaxWidth", SIZE_UNSPECIFIED);

    if (mMinWidth != SIZE_UNSPECIFIED && mMaxWidth != SIZE_UNSPECIFIED
                && mMinWidth > mMaxWidth) {
        throw "minWidth  > maxWidth";
    }

    const std::string layoutres=atts.getString("internalLayout",(getOrientation()==LinearLayout::VERTICAL?DEFAULT_LAYOUT_VERT:DEFAULT_LAYOUT_HORZ));
    LayoutInflater::from(mContext)->inflate(layoutres,this);
    setWidthAndHeight();
    mComputeMaxWidth = (mMaxWidth == SIZE_UNSPECIFIED);
    setWillNotDraw(false);

    mSelectedText =(EditText*)findViewById(cdroid::R::id::numberpicker_input);
    mSelectedText->setEnabled(false);
    mSelectedText->setFocusable(false);
    mTextAlign = mSelectedText->getGravity();
    mSelectedTextSize = mSelectedText->getTextSize();
    mTypeface = Typeface::create(atts.getString("typeface"),Typeface::NORMAL);
    mSelectedTypeface = Typeface::create(atts.getString("selectedTypeface"),Typeface::NORMAL);
    ViewConfiguration configuration = ViewConfiguration::get(context);
    setTextSize(atts.getDimensionPixelSize("textSize",mTextSize));
    setSelectedTextSize(atts.getDimensionPixelSize("selectedTextSize",mSelectedTextSize));
    setTextColor(atts.getColor("textColor"));
    setSelectedTextColor(atts.getColor("selectedTextColor"));
    ColorStateList*colors=mSelectedText->getTextColors();
    mTextColor = colors->getColorForState(StateSet::get(StateSet::VIEW_STATE_ENABLED),Color::WHITE);
    updateInputTextView();

    setValue(atts.getInt("value",0));
    setMinValue(atts.getInt("min",0));
    setMaxValue(atts.getInt("max",0));
    setWheelItemCount(atts.getInt("wheelItemCount",mWheelItemCount));

    LOGV("%p:%d textSize=%d,%d",this,mID,mSelectedTextSize,mTextSize);
    if(getFocusable()==View::FOCUSABLE_AUTO){
        setFocusable(int(View::FOCUSABLE));
        setFocusableInTouchMode(true);
    }
}

NumberPicker::~NumberPicker(){
    delete mDividerDrawable;
}

bool NumberPicker::isHorizontalMode()const{
    return getOrientation() == HORIZONTAL;
}

bool NumberPicker::isAscendingOrder()const{
    return mOrder == ASCENDING;
}

static float pxToSp(float px){
    return px;
}

static float dpToPx(float dp){
    return dp;
}

void NumberPicker::setWidthAndHeight() {
    if (isHorizontalMode()) {
        mMinHeight = SIZE_UNSPECIFIED;
        mMaxHeight = (int) dpToPx(DEFAULT_MIN_WIDTH);
        mMinWidth = (int) dpToPx(DEFAULT_MAX_HEIGHT);
        mMaxWidth = SIZE_UNSPECIFIED;
    } else {
        mMinHeight = SIZE_UNSPECIFIED;
        mMaxHeight = (int) dpToPx(DEFAULT_MAX_HEIGHT);
        mMinWidth = (int) dpToPx(DEFAULT_MIN_WIDTH);
        mMaxWidth = SIZE_UNSPECIFIED;
    }
}

void NumberPicker::setOrientation(int orientation) {
    //if(orientation!=getOrientation())
    LinearLayout::setOrientation(orientation);
    setWidthAndHeight();
    requestLayout();
}

void NumberPicker::setWheelItemCount(int count) {
    LOGE_IF(count<1,"Wheel item count must be >= 1");
    mRealWheelItemCount = count;
    mWheelItemCount = std::max(count, (int)DEFAULT_WHEEL_ITEM_COUNT);
    mWheelMiddleItemIndex = mWheelItemCount / 2;
    mSelectorIndices.resize(mWheelItemCount);
}

void NumberPicker::initView(){
    ViewConfiguration&config=ViewConfiguration::get(mContext);
    mSelectedText = nullptr;
    mOnValueChangeListener = nullptr;
    mFormatter = nullptr;
    mOnScrollListener.onScrollStateChange = nullptr;
    mScrollState = OnScrollListener::SCROLL_STATE_IDLE;
    mTextSize   = 24;
    mItemSpacing= 0;
    mSelectedTextSize = 24;
    mSelectedTextColor = 0xFFFFFFFF;
    mSelectedTypeface = nullptr;
    mTypeface = nullptr;
    mDividerColor =DEFAULT_DIVIDER_COLOR;
    mWheelMiddleItemIndex = 0;
    mDividerDrawable  = nullptr;
    mDividerLength =2;
    mDividerThickness =2;
    mDividerType = SIDE_LINES;
    mLastHandledDownDpadKeyCode =-1;
    mFadingEdgeEnabled= true;
    mFadingEdgeStrength= DEFAULT_FADING_EDGE_STRENGTH;
    mWrapSelectorWheel= false;
    mWrapSelectorWheelPreferred =true;
    mSelectionDividerHeight = UNSCALED_DEFAULT_SELECTION_DIVIDER_HEIGHT;
    mPreviousScrollerY   =0;
    mCurrentScrollOffset =0;
    mInitialScrollOffset =0;
    mLongPressUpdateInterval = DEFAULT_LONG_PRESS_UPDATE_INTERVAL;
    mMinHeight = SIZE_UNSPECIFIED;
    mMaxHeight = SIZE_UNSPECIFIED;
    mMinWidth  = SIZE_UNSPECIFIED;
    mMaxWidth  = SIZE_UNSPECIFIED;
    mValue    = 0;
    mMinValue = 0;
    mMaxValue = 0;
    mSelectorTextGapHeight = 0;
    mSelectionDividersDistance =UNSCALED_DEFAULT_SELECTION_DIVIDERS_DISTANCE;
    mVelocityTracker = nullptr;

    mTouchSlop = config.getScaledTouchSlop();
    mMinimumFlingVelocity = config.getScaledMinimumFlingVelocity();
    mMaximumFlingVelocity = config.getScaledMaximumFlingVelocity()/ SELECTOR_MAX_FLING_VELOCITY_ADJUSTMENT;
    mFlingScroller  = new Scroller(getContext(), nullptr, true);
    mAdjustScroller = new Scroller(getContext(), new DecelerateInterpolator(2.5f));
    mComputeMaxWidth = (mMaxWidth == SIZE_UNSPECIFIED);
    mHideWheelUntilFocused=false;
    mWheelItemCount = DEFAULT_WHEEL_ITEM_COUNT;
    mRealWheelItemCount= DEFAULT_WHEEL_ITEM_COUNT;
    mSelectorIndices.resize(mWheelItemCount);
}
void NumberPicker::onLayout(bool changed, int left, int top, int width, int height){
    const int msrdWdth = getMeasuredWidth();
    const int msrdHght = getMeasuredHeight();

    // Input text centered horizontally.
    const int inptTxtMsrdWdth = mSelectedText->getMeasuredWidth();
    const int inptTxtMsrdHght = mSelectedText->getMeasuredHeight();
    const int inptTxtLeft= (msrdWdth - inptTxtMsrdWdth) /2;
    const int inptTxtTop = (msrdHght - inptTxtMsrdHght)/2;
    const int inptTxtRight= inptTxtLeft + inptTxtMsrdWdth;
    const int inptTxtBottom=inptTxtTop + inptTxtMsrdHght;
    mSelectedText->layout(inptTxtLeft, inptTxtTop, inptTxtMsrdWdth, inptTxtMsrdHght);
    mSelectedTextCenterX = mSelectedText->getX() + mSelectedText->getMeasuredWidth()/2.f -2.f;
    mSelectedTextCenterY = mSelectedText->getY() + mSelectedText->getMeasuredHeight()/2.f -5.f;
    if (changed) { // need to do all this when we know our size
        initializeSelectorWheel();
        initializeFadingEdges();
        const int dividerDistence = 2*mDividerThickness + mDividerDistance;
        if(isHorizontalMode()){
            mLeftDividerLeft = (getWidth()-mDividerDistance)/2 - mDividerThickness;
            mRightDividerRight= mLeftDividerLeft + dividerDistence;
            mBottomDividerBottom = getHeight();
        }else{
            mTopDividerTop = (getHeight() - mDividerDistance)/2 - mDividerThickness;
            mBottomDividerBottom = mTopDividerTop + dividerDistence;
        }
    }
}


void NumberPicker::onMeasure(int widthMeasureSpec, int heightMeasureSpec){
    // Try greedily to fit the max width and height.
    const int newWidthMeasureSpec = makeMeasureSpec(widthMeasureSpec, mMaxWidth);
    const int newHeightMeasureSpec = makeMeasureSpec(heightMeasureSpec, mMaxHeight);
    LinearLayout::onMeasure(newWidthMeasureSpec, newHeightMeasureSpec);
    // Flag if we are measured with width or height less than the respective min.
    const int widthSize = resolveSizeAndStateRespectingMinSize(mMinWidth, getMeasuredWidth(),
                widthMeasureSpec);
    const int heightSize = resolveSizeAndStateRespectingMinSize(mMinHeight, getMeasuredHeight(),
                heightMeasureSpec);
    setMeasuredDimension(widthSize, heightSize);
}


bool NumberPicker::moveToFinalScrollerPosition(Scroller* scroller) {
    scroller->forceFinished(true);
    if(isHorizontalMode()){
        int amountToScroll = scroller->getFinalX() - scroller->getCurrX();
        int futureScrollOffset = (mCurrentScrollOffset + amountToScroll) % mSelectorElementSize;
        int overshootAdjustment = mInitialScrollOffset - futureScrollOffset;
        if (overshootAdjustment != 0) {
            if (std::abs(overshootAdjustment) > mSelectorElementSize / 2) {
                if (overshootAdjustment > 0) {
                    overshootAdjustment -= mSelectorElementSize;
                } else {
                    overshootAdjustment += mSelectorElementSize;
                }
            }
            amountToScroll += overshootAdjustment;
            scrollBy(amountToScroll, 0);
            return true;
        }
    }else{
        int amountToScroll = scroller->getFinalY() - scroller->getCurrY();
        int futureScrollOffset = (mCurrentScrollOffset + amountToScroll) % mSelectorElementSize;
        int overshootAdjustment = mInitialScrollOffset - futureScrollOffset;
        if (overshootAdjustment != 0) {
            if (std::abs(overshootAdjustment) > mSelectorElementSize / 2) {
                if (overshootAdjustment > 0) {
                    overshootAdjustment -= mSelectorElementSize;
                } else {
                    overshootAdjustment += mSelectorElementSize;
                }
            }
            amountToScroll += overshootAdjustment;
            scrollBy(0, amountToScroll);
            return true;
        }
    }
    return false;
}
bool NumberPicker::onInterceptTouchEvent(MotionEvent& event){
    const int action = event.getActionMasked();
    if (!isEnabled() ||(action!=MotionEvent::ACTION_DOWN)) {
        return false;
    }
    if(isHorizontalMode()){
        mLastDownOrMoveEventX = mLastDownEventX = event.getX();
        if (!mFlingScroller->isFinished()) {
            mFlingScroller->forceFinished(true);
            mAdjustScroller->forceFinished(true);
            onScrollerFinished(mFlingScroller);
            onScrollStateChange(OnScrollListener::SCROLL_STATE_IDLE);
        } else if (!mAdjustScroller->isFinished()) {
            mFlingScroller->forceFinished(true);
            mAdjustScroller->forceFinished(true);
            onScrollerFinished(mAdjustScroller);
        } else if (mLastDownEventX >= mLeftDividerLeft
                && mLastDownEventX <= mRightDividerRight) {
            if (mOnClickListener) {
                mOnClickListener(*this);
            }
        } else if (mLastDownEventX < mLeftDividerLeft) {
            postChangeCurrentByOneFromLongPress(false);
        } else if (mLastDownEventX > mRightDividerRight) {
            postChangeCurrentByOneFromLongPress(true);
        }
    }else{
        mLastDownOrMoveEventY = mLastDownEventY = event.getY();
        if (!mFlingScroller->isFinished()) {
            mFlingScroller->forceFinished(true);
            mAdjustScroller->forceFinished(true);
            onScrollStateChange(OnScrollListener::SCROLL_STATE_IDLE);
        } else if (!mAdjustScroller->isFinished()) {
            mFlingScroller->forceFinished(true);
            mAdjustScroller->forceFinished(true);
        } else if (mLastDownEventY >= mTopDividerTop
                && mLastDownEventY <= mBottomDividerBottom) {
            if (mOnClickListener) {
                mOnClickListener(*this);
            }
        } else if (mLastDownEventY < mTopDividerTop) {
            postChangeCurrentByOneFromLongPress(false);
        } else if (mLastDownEventY > mBottomDividerBottom) {
            postChangeCurrentByOneFromLongPress(true);
        }        
    }//endif isHorizontalMode
    return true;
}

bool NumberPicker::onTouchEvent(MotionEvent& event){
    if (!isEnabled()) {
        return false;
    }
    if (mVelocityTracker==nullptr) mVelocityTracker = VelocityTracker::obtain();

    mVelocityTracker->addMovement(event);
    int action = event.getActionMasked();
    switch (action) {
    case MotionEvent::ACTION_CANCEL:LOGD("ACTION_CANCEL");break;
    case MotionEvent::ACTION_MOVE:
            if (isHorizontalMode()) {
                float currentMoveX = event.getX();
                if (mScrollState != OnScrollListener::SCROLL_STATE_TOUCH_SCROLL) {
                    int deltaDownX = (int) std::abs(currentMoveX - mLastDownEventX);
                    if (deltaDownX > mTouchSlop) {
                        removeAllCallbacks();
                        onScrollStateChange(OnScrollListener::SCROLL_STATE_TOUCH_SCROLL);
                    }
                } else {
                    int deltaMoveX = (int) ((currentMoveX - mLastDownOrMoveEventX));
                    scrollBy(deltaMoveX, 0);
                    invalidate();
                }
                mLastDownOrMoveEventX = currentMoveX;                
            }else{
                const float currentMoveY = event.getY();
                if (mScrollState != OnScrollListener::SCROLL_STATE_TOUCH_SCROLL) {
                    int deltaDownY = (int) std::abs(currentMoveY - mLastDownEventY);
                    if (deltaDownY > mTouchSlop) {
                        removeAllCallbacks();
                        onScrollStateChange(OnScrollListener::SCROLL_STATE_TOUCH_SCROLL);
                    }
                } else {
                    int deltaMoveY = (int) ((currentMoveY - mLastDownOrMoveEventY));
                    scrollBy(0, deltaMoveY);
                    invalidate();
                }
                mLastDownOrMoveEventY = currentMoveY;
            }   
          break;
    case MotionEvent::ACTION_UP: {
            removeBeginSoftInputCommand();
            removeChangeCurrentByOneFromLongPress();
            pshCancel();
            mVelocityTracker->computeCurrentVelocity(1000, mMaximumFlingVelocity);
            if(isHorizontalMode()){
                int initialVelocity = (int) mVelocityTracker->getXVelocity();
                if (std::abs(initialVelocity) > mMinimumFlingVelocity) {
                    fling(initialVelocity);
                    onScrollStateChange(OnScrollListener::SCROLL_STATE_FLING);
                } else {
                    const int eventX = (int) event.getX();
                    const int deltaMoveX = (int) std::abs(eventX - mLastDownEventX);
                    if (deltaMoveX <= mTouchSlop) {
                        int selectorIndexOffset = (eventX / mSelectorElementSize)
                                - mWheelMiddleItemIndex;
                        if (selectorIndexOffset > 0) {
                            changeValueByOne(true);
                        } else if (selectorIndexOffset < 0) {
                            changeValueByOne(false);
                        } else {
                            ensureScrollWheelAdjusted();
                        }
                    } else {
                        ensureScrollWheelAdjusted();
                    }
                    onScrollStateChange(OnScrollListener::SCROLL_STATE_IDLE);
                }
            }else{
                const int initialVelocity = (int) mVelocityTracker->getYVelocity();
                if (std::abs(initialVelocity) > mMinimumFlingVelocity) {
                    fling(initialVelocity);
                    onScrollStateChange(OnScrollListener::SCROLL_STATE_FLING);
                } else {
                    const int eventY = (int) event.getY();
                    const int deltaMoveY = (int) std::abs(eventY - mLastDownEventY);
                    if (deltaMoveY <= mTouchSlop){
                        int selectorIndexOffset = (eventY / mSelectorElementSize) - mWheelMiddleItemIndex;
                        if (selectorIndexOffset > 0) {
                            changeValueByOne(true);
                            pshButtonTapped(R::id::increment);
                        } else if (selectorIndexOffset < 0) {
                            changeValueByOne(false);
                            pshButtonTapped(R::id::decrement);
                        }else{
                            ensureScrollWheelAdjusted();
                        }
                    }else{
                        ensureScrollWheelAdjusted();
                    }
                    onScrollStateChange(OnScrollListener::SCROLL_STATE_IDLE);
                }
            }
            mVelocityTracker->recycle();
            mVelocityTracker = nullptr;
        } break;
    }//end switch
    return true;
}

bool NumberPicker::dispatchTouchEvent(MotionEvent& event){
    int action = event.getActionMasked();
    switch (action) {
    case MotionEvent::ACTION_CANCEL:LOGD("ACTION_CANCEL");
    case MotionEvent::ACTION_UP:
        removeAllCallbacks();
        break;
    }
    return LinearLayout::dispatchTouchEvent(event);
}

bool NumberPicker::dispatchKeyEvent(KeyEvent& event){
    int keyCode = event.getKeyCode();
    switch (keyCode) {
    case KEY_DPAD_CENTER:
    case KEY_ENTER:
        removeAllCallbacks();
        break;
    case KEY_DPAD_DOWN:
    case KEY_DPAD_UP:
        switch (event.getAction()) {
        case KeyEvent::ACTION_DOWN:
            if (mWrapSelectorWheel || ((keyCode == KEY_DPAD_DOWN)
                    ? getValue() < getMaxValue() : getValue() > getMinValue())) {
            requestFocus();
            mLastHandledDownDpadKeyCode = keyCode;
            removeAllCallbacks();
            if (mFlingScroller->isFinished()) {
                changeValueByOne(keyCode == KEY_DPAD_DOWN);
            }
            return true;
        }break;
        case KeyEvent::ACTION_UP:
            if (mLastHandledDownDpadKeyCode == keyCode) {
                mLastHandledDownDpadKeyCode = -1;
                return true;
            }break;
        }
    }
    return LinearLayout::dispatchKeyEvent(event);
}

void NumberPicker::computeScroll() {
    Scroller* scroller = mFlingScroller;
    if (scroller->isFinished()) {
        scroller = mAdjustScroller;
        if (scroller->isFinished()) {
            return;
        }
    }
    scroller->computeScrollOffset();
    if(isHorizontalMode()){
        int currentScrollerX = scroller->getCurrX();
        if (mPreviousScrollerX == 0) {
            mPreviousScrollerX = scroller->getStartX();
        }
        scrollBy(currentScrollerX - mPreviousScrollerX, 0);
        mPreviousScrollerX = currentScrollerX;
    }else{
        int currentScrollerY = scroller->getCurrY();
        if (mPreviousScrollerY == 0) {
            mPreviousScrollerY = scroller->getStartY();
        }
        scrollBy(0, currentScrollerY - mPreviousScrollerY);
        mPreviousScrollerY = currentScrollerY;
    }
    if (scroller->isFinished()) {
        onScrollerFinished(scroller);
    } else {
        invalidate();
    }
    
}

View& NumberPicker::setEnabled(bool enabled) {
    ViewGroup::setEnabled(enabled);
    mSelectedText->setEnabled(enabled);
    return *this;
}

void NumberPicker::scrollBy(int x, int y){
    std::vector<int>&selectorIndices = mSelectorIndices;
    const int startScrollOffset = mCurrentScrollOffset;
    const int gap = getMaxTextSize();
    if (isHorizontalMode()) {
        if (isAscendingOrder()) {
            if (!mWrapSelectorWheel && x > 0
                    && selectorIndices[mWheelMiddleItemIndex] <= mMinValue) {
                mCurrentScrollOffset = mInitialScrollOffset;
                return;
            }
            if (!mWrapSelectorWheel && x < 0
                    && selectorIndices[mWheelMiddleItemIndex] >= mMaxValue) {
                mCurrentScrollOffset = mInitialScrollOffset;
                return;
            }
        } else {
            if (!mWrapSelectorWheel && x > 0
                    && selectorIndices[mWheelMiddleItemIndex] >= mMaxValue) {
                mCurrentScrollOffset = mInitialScrollOffset;
                return;
            }
            if (!mWrapSelectorWheel && x < 0
                    && selectorIndices[mWheelMiddleItemIndex] <= mMinValue) {
                mCurrentScrollOffset = mInitialScrollOffset;
                return;
            }
        }

        mCurrentScrollOffset += x;
    } else {
        if (isAscendingOrder()) {
            if (!mWrapSelectorWheel && y > 0
                    && selectorIndices[mWheelMiddleItemIndex] <= mMinValue) {
                mCurrentScrollOffset = mInitialScrollOffset;
                return;
            }
            if (!mWrapSelectorWheel && y < 0
                    && selectorIndices[mWheelMiddleItemIndex] >= mMaxValue) {
                mCurrentScrollOffset = mInitialScrollOffset;
                return;
            }
        } else {
            if (!mWrapSelectorWheel && y > 0
                    && selectorIndices[mWheelMiddleItemIndex] >= mMaxValue) {
                mCurrentScrollOffset = mInitialScrollOffset;
                return;
            }
            if (!mWrapSelectorWheel && y < 0
                    && selectorIndices[mWheelMiddleItemIndex] <= mMinValue) {
                mCurrentScrollOffset = mInitialScrollOffset;
                return;
            }
        }

        mCurrentScrollOffset += y;
    }
    while (mCurrentScrollOffset - mInitialScrollOffset > gap) {
        mCurrentScrollOffset -= mSelectorElementSize;
        if(isAscendingOrder())
            decrementSelectorIndices(selectorIndices);
        else
            incrementSelectorIndices(selectorIndices);
        setValueInternal(selectorIndices[mWheelMiddleItemIndex], true);
        if (!mWrapSelectorWheel && selectorIndices[mWheelMiddleItemIndex] <= mMinValue) {
            mCurrentScrollOffset = mInitialScrollOffset;
        }
    }

    while (mCurrentScrollOffset - mInitialScrollOffset < -gap) {
        mCurrentScrollOffset += mSelectorElementSize;
        if(isAscendingOrder())
            incrementSelectorIndices(selectorIndices);
        else
            decrementSelectorIndices(selectorIndices);
        setValueInternal(selectorIndices[mWheelMiddleItemIndex], true);
        if (!mWrapSelectorWheel && selectorIndices[mWheelMiddleItemIndex] >= mMaxValue) {
            mCurrentScrollOffset = mInitialScrollOffset;
        }
    }
    if (startScrollOffset != mCurrentScrollOffset) {
        if(isHorizontalMode())
            onScrollChanged(mCurrentScrollOffset,0,startScrollOffset,0);
        else
            onScrollChanged(0, mCurrentScrollOffset, 0, startScrollOffset);
    }
}

int NumberPicker::computeHorizontalScrollOffset(){
    return isHorizontalMode()?mCurrentScrollOffset:0;
}
int NumberPicker::computeHorizontalScrollRange(){
    return isHorizontalMode()?(mMaxValue - mMinValue +1)*mSelectorElementSize:0;
}
int NumberPicker::computeHorizontalScrollExtent(){
    return isHorizontalMode()?getWidth():0;
}
int NumberPicker::computeVerticalScrollOffset() {
    return isHorizontalMode()?0:mCurrentScrollOffset;
}

int NumberPicker::computeVerticalScrollRange() {
    //return std::min(mMaxValue - mMinValue + 1,mMaxSelectorIndices) * mSelectorElementSize;
    return isHorizontalMode()?0:(mMaxValue - mMinValue + 1) * mSelectorElementSize;
}

int NumberPicker::computeVerticalScrollExtent() {
    return isHorizontalMode()?0:getHeight();
}

void NumberPicker::setOnClickListener(OnClickListener onClickListener){
    mOnClickListener = onClickListener;
}
void NumberPicker::setOnValueChangedListener(OnValueChangeListener onValueChangedListener){
    mOnValueChangeListener=onValueChangedListener;
}

void NumberPicker::setOnScrollListener(const OnScrollListener& onScrollListener){
    mOnScrollListener = onScrollListener;
}

void NumberPicker::setFormatter(Formatter formatter){
    mFormatter = formatter;
    initializeSelectorWheelIndices();
    updateInputTextView();
}

void NumberPicker::setFadingEdgeEnabled(bool fadingEdgeEnabled) {
    mFadingEdgeEnabled = fadingEdgeEnabled;
}

void NumberPicker::setFadingEdgeStrength(float strength) {
    mFadingEdgeStrength = strength;
}

void NumberPicker::setValue(int value) {
    setValueInternal(value, false);
}

float NumberPicker::getMaxTextSize()const {
    return std::max(mTextSize, mSelectedTextSize);
}

bool NumberPicker::performClick() {
    if (true/*!mHasSelectorWheel*/) {
        return ViewGroup::performClick();
    } else if (!ViewGroup::performClick()) {
        showSoftInput();
    }
    return true;
}

bool NumberPicker::performLongClick() {
    if (true/*!mHasSelectorWheel*/) {
        return ViewGroup::performLongClick();
    } else if (!ViewGroup::performLongClick()) {
        showSoftInput();
    }
    return true;
}

void NumberPicker::showSoftInput(){
    //if(mHasSelectorWheel)
	mSelectedText->setVisibility(View::VISIBLE);
}

void NumberPicker::hideSoftInput(){
    if (mSelectedText->getInputType() != EditText::TYPE_NONE) {
        mSelectedText->setVisibility(View::INVISIBLE);
    }
}

void NumberPicker::tryComputeMaxWidth(){
    if (!mComputeMaxWidth) {
        return;
    }
    int maxTextWidth = 0;
    if (mDisplayedValues.size() == 0) {
        float maxDigitWidth = 0;
        Layout l(mTextSize,-1);
        l.setTypeface(mSelectedText->getTypeface());
        for (int i = 0; i <= 9; i++) {
            l.setText(std::to_string(i));
            l.relayout();
            const float digitWidth = l.getMaxLineWidth();
            if (digitWidth > maxDigitWidth) {
                maxDigitWidth = digitWidth;
            }
        }
        int numberOfDigits = 0;
        int current = mMaxValue;
        while (current > 0) {
            numberOfDigits++;
            current = current / 10;
        }
        maxTextWidth = (int) (numberOfDigits * maxDigitWidth);
    } else {
        const int valueCount = mDisplayedValues.size();
        Layout l(mTextSize,-1);
        l.setTypeface(mSelectedText->getTypeface());
        for (int i = 0; i < valueCount; i++) {
            l.setText(mDisplayedValues[i]);
            l.relayout();
            const float textWidth = l.getMaxLineWidth();
            if (textWidth > maxTextWidth) {
                maxTextWidth = (int) textWidth;
            }
        }
    }
    maxTextWidth += mSelectedText->getPaddingLeft() + mSelectedText->getPaddingRight();
    if (mMaxWidth != maxTextWidth) {
        mMaxWidth = std::max(mMinWidth,maxTextWidth);
        invalidate();
    }
}

bool NumberPicker::getWrapSelectorWheel()const{
    return mWrapSelectorWheel;
}

void NumberPicker::setWrapSelectorWheel(bool wrapSelectorWheel) {
    mWrapSelectorWheelPreferred = wrapSelectorWheel;
    updateWrapSelectorWheel();
}

void NumberPicker::updateWrapSelectorWheel() {
    const bool wrappingAllowed = (mMaxValue - mMinValue + 1) >= mSelectorIndices.size();
    mWrapSelectorWheel = wrappingAllowed && mWrapSelectorWheelPreferred;
}

void NumberPicker::setOnLongPressUpdateInterval(long intervalMillis) {
    mLongPressUpdateInterval = intervalMillis;
}

Drawable*NumberPicker::getDivider()const{
    return mDividerDrawable;
}

void NumberPicker::setDivider(Drawable*d){
    delete mDividerDrawable;
    mDividerDrawable = d;
    if (mDividerDrawable) {
        mDividerDrawable->setCallback(this);
        mDividerDrawable->setLayoutDirection(getLayoutDirection());
        if (mDividerDrawable->isStateful()) {
            mDividerDrawable->setState(getDrawableState());
        }
    }
    invalidate();
}

void NumberPicker::setSelectionDivider(Drawable*d){
    setDivider(d);
}

Drawable*NumberPicker::getSelectionDivider()const{
    return mDividerDrawable;
}

int  NumberPicker::getDividerColor()const{
    return mDividerColor;
}

void NumberPicker::setDividerColor(int color){
    mDividerColor = color;
    mDividerDrawable = new ColorDrawable(color);
}

int NumberPicker::getDividerType()const{
    return mDividerType;
}

void NumberPicker::setDividerType(int type){
    mDividerType = type;
    invalidate();
}

int NumberPicker::getDividerThickness()const{
    return mDividerThickness;
}

void NumberPicker::setDividerThickness(int thickness) {
    mDividerThickness = thickness;
    invalidate();
}

int NumberPicker::getOrder()const{
    return mOrder;
}

void NumberPicker::setOrder(int order) {
    mOrder = order;
    invalidate();
}

int NumberPicker::getValue()const{
    return mValue;
}

int NumberPicker::getMinValue()const{
    return mMinValue;
}

void NumberPicker::setMinValue(int minValue){
    mMinValue = minValue;
    if (mMinValue > mValue) mValue = mMinValue;
    updateWrapSelectorWheel();
    initializeSelectorWheelIndices();
    updateInputTextView();
    tryComputeMaxWidth();
    invalidate();
}

int NumberPicker::getMaxValue()const{
    return mMaxValue;
}

void NumberPicker::setMaxValue(int maxValue) {
    mMaxValue = maxValue;
    if (mMaxValue < mValue) mValue = mMaxValue;
    updateWrapSelectorWheel();
    initializeSelectorWheelIndices();
    updateInputTextView();
    tryComputeMaxWidth();
    invalidate();
}

void NumberPicker::setMinHeight(int h){
    mMinHeight = h;
}

void NumberPicker::setMaxHeight(int h){
    mMaxHeight = h;
}

int NumberPicker::getMinHeight()const{
    return mMinHeight;
}

int NumberPicker::getMaxHeight()const{
    return mMaxHeight;
}

std::vector<std::string>  NumberPicker::getDisplayedValues()const{
    return mDisplayedValues;
}

void  NumberPicker::setDisplayedValues(const std::vector<std::string>&displayedValues){
    mDisplayedValues = displayedValues;
    mSelectorIndexToStringCache.clear();
    updateInputTextView();
    initializeSelectorWheelIndices();
    tryComputeMaxWidth();
}

float NumberPicker::getFadingEdgeStrength(bool isHorizontalMode)const{
    return isHorizontalMode && mFadingEdgeEnabled ? mFadingEdgeStrength : 0;
}

float NumberPicker::getTopFadingEdgeStrength() {
     return getFadingEdgeStrength(!isHorizontalMode());
}

float NumberPicker::getBottomFadingEdgeStrength() {
     return getFadingEdgeStrength(!isHorizontalMode());
}

float NumberPicker::getLeftFadingEdgeStrength() {
    return getFadingEdgeStrength(isHorizontalMode());
}

float NumberPicker::getRightFadingEdgeStrength() {
    return getFadingEdgeStrength(isHorizontalMode());
}

void NumberPicker::drawableStateChanged() {
    ViewGroup::drawableStateChanged();

    if (mDividerDrawable!=nullptr  && mDividerDrawable->isStateful()
        && mDividerDrawable->setState(getDrawableState())) {
        invalidateDrawable(*mDividerDrawable);
    }
}

void NumberPicker::jumpDrawablesToCurrentState() {
    ViewGroup::jumpDrawablesToCurrentState();

    if (mDividerDrawable != nullptr) {
        mDividerDrawable->jumpToCurrentState();
    }
}

void NumberPicker::onResolveDrawables(int layoutDirection){
    ViewGroup::onResolveDrawables(layoutDirection);
    if (mDividerDrawable) {
        mDividerDrawable->setLayoutDirection(layoutDirection);
    }
}

void NumberPicker::setTextColor(int color){
    mTextColor = color;
    invalidate();
}
void NumberPicker::setTextColor(int color,int color2){
    setSelectedTextColor(color);
    mTextColor = color2;
    invalidate();
}

int  NumberPicker::getTextColor()const{
    return mTextColor;
}

void NumberPicker::setTextSize(int size){
    mTextSize = size;
    invalidate();
}
void NumberPicker::setTextSize(int size,int size2){
    setSelectedTextSize(size);
    mTextSize = size2;
    requestLayout();
    invalidate();
}

int  NumberPicker::getTextSize()const{
    return mTextSize;
}

int  NumberPicker::getSelectedTextColor()const{
    return mSelectedTextColor;
}

void NumberPicker::setSelectedTextColor(int textColor){
    mSelectedTextColor = textColor;
    mSelectedText->setTextColor(textColor);
}

int  NumberPicker::getSelectedTextSize()const{
    return mSelectedTextSize;
}

void NumberPicker::setSelectedTextSize(int textSize) {
    mSelectedTextSize = textSize;
    mSelectedText->setTextSize(textSize);
    requestLayout();
    invalidate();
}

void NumberPicker::setSelectedTypeface(Typeface* typeface){
    mSelectedTypeface = typeface;
    if (mSelectedTypeface != nullptr) {
        //do nothing mSelectorWheelPaint.setTypeface(mSelectedTypeface);
    } else if (mTypeface != nullptr) {
        mSelectedTypeface = mTypeface;
    } else {
        mSelectedTypeface = Typeface::MONOSPACE;
    }
}

void NumberPicker::setSelectedTypeface(const std::string& string, int style){
    setSelectedTypeface(Typeface::create(string, style));
}

Typeface* NumberPicker::getSelectedTypeface()const{
    return mSelectedTypeface;
}

void NumberPicker::setTypeface(Typeface* typeface){
    mTypeface = typeface;
    if (mTypeface != nullptr) {
        mSelectedText->setTypeface(mTypeface);
        setSelectedTypeface(mSelectedTypeface);
    } else {
        mSelectedText->setTypeface(Typeface::MONOSPACE);
    }
}

void NumberPicker::setTypeface(const std::string& string, int style){
    setTypeface(Typeface::create(string, style));
}

Typeface* NumberPicker::getTypeface()const{
    return mTypeface;
}

void NumberPicker::onDraw(Canvas&canvas){
    const bool showSelectorWheel = !mHideWheelUntilFocused || hasFocus();
    float x, y;
    Rect recText;
    canvas.save();
    if (isHorizontalMode()) {
        x = mCurrentScrollOffset - mSelectorElementSize/2;
        y = 0;
        recText = Rect::Make(x,y,mSelectorElementSize,getHeight());
        if (mRealWheelItemCount < DEFAULT_WHEEL_ITEM_COUNT) {
            canvas.rectangle(mLeftDividerLeft, 0, mRightDividerRight-mLeftDividerLeft, getHeight());
            canvas.clip();
        }
    } else {
        x = 0;
        y = mCurrentScrollOffset - mSelectorElementSize/2;
        recText = Rect::Make(0,y,getWidth(),mSelectorElementSize);
        if (mRealWheelItemCount < DEFAULT_WHEEL_ITEM_COUNT) {
            canvas.rectangle(0, mTopDividerTop, getWidth(), mBottomDividerBottom-mTopDividerTop);
            canvas.clip();
        }
    }
    Cairo::RefPtr<Cairo::LinearGradient> pat;
    if(mSelectedTextColor!=mTextColor){
        Color c1(mSelectedTextColor), c2(mTextColor);
        pat=Cairo::LinearGradient::create(0,0,(isHorizontalMode()?getWidth():0),
                                          (isHorizontalMode()?0:getHeight()));
  	    pat->add_color_stop_rgba(.0f,c2.red(),c2.green(),c2.blue(),c2.alpha());
	    pat->add_color_stop_rgba(.5f,c1.red(),c1.green(),c1.blue(),c1.alpha());
	    pat->add_color_stop_rgba(1.f,c2.red(),c2.green(),c2.blue(),c2.alpha());
        canvas.set_source(pat);
    }else{
        canvas.set_color(mSelectedTextColor);
    }
    canvas.set_font_size(mSelectedTextSize);
    // draw the selector wheel
    std::vector<int>& selectorIndices = mSelectorIndices;
    for (int i = 0; i < selectorIndices.size(); i++) {
        float font_size = mSelectedTextSize;
        int selectedHeight = mSelectorElementSize;
        if(mSelectedTextSize!=mTextSize){
            if(isHorizontalMode()){
                const float harfWidth = getWidth()/2.f;
                const float fraction = std::abs(x-harfWidth)/harfWidth;
                font_size = lerp(mSelectedTextSize,mTextSize,fraction);
            }else{
                const float harfHeight = getHeight()/2.f;
                const float fraction = std::abs(y-harfHeight)/harfHeight;
                font_size = lerp(mSelectedTextSize,mTextSize,fraction);
            }
            canvas.set_font_size(font_size);
        }

        int selectorIndex = selectorIndices[isAscendingOrder() ? i : selectorIndices.size() - i - 1];
        std::string scrollSelectorValue = mSelectorIndexToStringCache.at(selectorIndex);
        if (scrollSelectorValue.empty()) {
            continue;
        }
        if(i==mWheelMiddleItemIndex)
            selectedHeight = std::max(mSelectorElementSize,mSelectedText->getHeight());
        recText.height = selectedHeight;
        // Do not draw the middle item if input is visible since the input
        // is shown only if the wheel is static and it covers the middle
        // item. Otherwise, if the user starts editing the text via the
        // IME he may see a dimmed version of the old value intermixed
        // with the new one.
        if ((showSelectorWheel && i != mWheelMiddleItemIndex)
                || (i == mWheelMiddleItemIndex && mSelectedText->getVisibility() != VISIBLE)) {
            int xOffset = 0;
            int yOffset = 0;
            if (i != mWheelMiddleItemIndex && mItemSpacing != 0) {
                if (isHorizontalMode()) {
                    if (i > mWheelMiddleItemIndex) {
                        xOffset = mItemSpacing;
                    } else {
                        xOffset = -mItemSpacing;
                    }
                } else {
                    if (i > mWheelMiddleItemIndex) {
                        yOffset = mItemSpacing;
                    } else {
                        yOffset = -mItemSpacing;
                    }
                }
            }
            canvas.draw_text(recText,scrollSelectorValue,Gravity::CENTER);//mTextAlign);
        }
        if (isHorizontalMode()) {
            x += mSelectorElementSize;
            recText.offset(mSelectorElementSize,0);
        } else {
            int selectedHeight = mSelectorElementSize;
            if(i==mWheelMiddleItemIndex)selectedHeight = std::max(mSelectorElementSize,mSelectedText->getHeight());
            y += selectedHeight;
            recText.offset(0,selectedHeight);
        }
    }

    // restore canvas
    canvas.restore();

    // draw the dividers
    if (showSelectorWheel && mDividerDrawable) {
        if (isHorizontalMode())
            drawHorizontalDividers(canvas);
        else
            drawVerticalDividers(canvas);
    }
}

void NumberPicker::drawHorizontalDividers(Canvas& canvas) {
    int top,bottom,left,right;
    int leftOfLeftDivider,rightOfLeftDivider;
    int bottomOfUnderlineDivider,topOfUnderlineDivider;
    int rightOfRightDivider,leftOfRightDivider;

    switch (mDividerType) {
    case SIDE_LINES:
        if (mDividerLength > 0 && mDividerLength <= mMaxHeight) {
            top = (mMaxHeight - mDividerLength) / 2;
            bottom = top + mDividerLength;
        } else {
            top = 0;
            bottom = getBottom();
        }
        // draw the left divider
        leftOfLeftDivider = mLeftDividerLeft;
        rightOfLeftDivider = leftOfLeftDivider + mDividerThickness;
        mDividerDrawable->setBounds(leftOfLeftDivider, top, mDividerThickness, bottom-top);
        mDividerDrawable->draw(canvas);
        // draw the right divider
        rightOfRightDivider = mRightDividerRight;
        leftOfRightDivider = rightOfRightDivider - mDividerThickness;
        mDividerDrawable->setBounds(leftOfRightDivider, top, mDividerThickness, bottom-top);
        mDividerDrawable->draw(canvas);
        break;
    case UNDERLINE:
        if (mDividerLength > 0 && mDividerLength <= mMaxWidth) {
            left = (mMaxWidth - mDividerLength) / 2;
            right = left + mDividerLength;
        } else {
            left = mLeftDividerLeft;
            right = mRightDividerRight;
        }
        bottomOfUnderlineDivider = mBottomDividerBottom;
        mDividerDrawable->setBounds(left,topOfUnderlineDivider,right - left,mDividerThickness);
        mDividerDrawable->draw(canvas);
        break;
   }
}

void NumberPicker::drawVerticalDividers(Canvas& canvas) {
    int left, right;
    int topOfTopDivider,bottomOfTopDivider;
    int bottomOfUnderlineDivider,topOfUnderlineDivider;
    int topOfBottomDivider,bottomOfBottomDivider;
    if (mDividerLength > 0 && mDividerLength <= mMaxWidth) {
        left = (mMaxWidth - mDividerLength) / 2;
        right = left + mDividerLength;
    } else {
        left = 0;
        right = getRight();
    }
    switch (mDividerType) {
    case SIDE_LINES:
        // draw the top divider
        topOfTopDivider = mTopDividerTop;
        bottomOfTopDivider = topOfTopDivider + mDividerThickness;
        mDividerDrawable->setBounds(left, topOfTopDivider, right-left, mDividerThickness);
        mDividerDrawable->draw(canvas);
        // draw the bottom divider
        bottomOfBottomDivider = mBottomDividerBottom;
        topOfBottomDivider = bottomOfBottomDivider - mDividerThickness;
        mDividerDrawable->setBounds(left,topOfBottomDivider,right-left, mDividerThickness);
        mDividerDrawable->draw(canvas);
        break;
    case UNDERLINE:
        bottomOfUnderlineDivider = mBottomDividerBottom;
        topOfUnderlineDivider = bottomOfUnderlineDivider - mDividerThickness;
        mDividerDrawable->setBounds(left,topOfUnderlineDivider,right-left, mDividerThickness);
        mDividerDrawable->draw(canvas);
        break;
    }
}

void NumberPicker::drawText(const std::string& text, float x, float y,Canvas& canvas) {
    /*if (text.contains("\n")) {
        std::string[] lines = text.split("\n");
        float height = Math.abs(paint.descent() + paint.ascent())
                * mLineSpacingMultiplier;
        float diff = (lines.length - 1) * height / 2;
        y -= diff;
        for (String line : lines) {
            canvas.drawText(line, x, y, paint);
            y += height;
        }
    } else */{
        canvas.move_to(x,y);
        canvas.show_text(text);
    }
}

int NumberPicker::makeMeasureSpec(int measureSpec, int maxSize){
    if (maxSize == SIZE_UNSPECIFIED) {
        return measureSpec;
    }
    int size = MeasureSpec::getSize(measureSpec);
    int mode = MeasureSpec::getMode(measureSpec);
    switch (mode) {
    case MeasureSpec::EXACTLY:     return measureSpec;
    case MeasureSpec::AT_MOST:     return MeasureSpec::makeMeasureSpec(std::min(size, maxSize), MeasureSpec::EXACTLY);
    case MeasureSpec::UNSPECIFIED: return MeasureSpec::makeMeasureSpec(maxSize, MeasureSpec::EXACTLY);
    default:        throw std::string("Unknown measure mode: ")+std::to_string(mode);
    }
}

int NumberPicker::resolveSizeAndStateRespectingMinSize(int minSize, int measuredSize, int measureSpec) {
    if (minSize != SIZE_UNSPECIFIED) {
        int desiredWidth = std::max(minSize, measuredSize);
        return resolveSizeAndState(desiredWidth, measureSpec, 0);
    } else {
        return measuredSize;
    }
}

void NumberPicker::initializeSelectorWheelIndices(){
    mSelectorIndexToStringCache.clear();
    const int current = getValue();
    for (int i = 0; i < mSelectorIndices.size(); i++) {
        int selectorIndex = current + (i - mWheelMiddleItemIndex);
        if (mWrapSelectorWheel) {
            selectorIndex = getWrappedSelectorIndex(selectorIndex);
        }
        mSelectorIndices[i] = selectorIndex;
        ensureCachedScrollSelectorValue(mSelectorIndices[i]);
    }
}

void NumberPicker::setValueInternal(int current, bool notifyChng){
    if (mValue == current) {
        return;
    }
    // Wrap around the values if we go past the start or end
    if (mWrapSelectorWheel) {
        current = getWrappedSelectorIndex(current);
    } else {
        current = std::max(current, mMinValue);
        current = std::min(current, mMaxValue);
    }
    int previous = mValue;
    mValue = current;
    // If we're flinging, we'll update the text view at the end when it becomes visible
    if (mScrollState != OnScrollListener::SCROLL_STATE_FLING)
        updateInputTextView();
    if (notifyChng)
        notifyChange(previous, current);

    initializeSelectorWheelIndices();
    invalidate();
}

void NumberPicker::changeValueByOne(bool increment){
    if (!moveToFinalScrollerPosition(mFlingScroller)) {
        moveToFinalScrollerPosition(mAdjustScroller);
    }
    smoothScroll(increment,1);
}

void NumberPicker::smoothScrollToPosition(int position) {
    const int currentPosition = mSelectorIndices[mWheelMiddleItemIndex];
    if (currentPosition == position) {
        return;
    }
    smoothScroll(position > currentPosition, std::abs(position - currentPosition));
}

void NumberPicker::smoothScroll(bool increment, int steps) {
    const int diffSteps = (increment ? -mSelectorElementSize : mSelectorElementSize) * steps;
    if (isHorizontalMode()) {
        mPreviousScrollerX = 0;
        mFlingScroller->startScroll(0, 0, diffSteps, 0, SNAP_SCROLL_DURATION);
    } else {
        mPreviousScrollerY = 0;
        mFlingScroller->startScroll(0, 0, 0, diffSteps, SNAP_SCROLL_DURATION);
    }
    invalidate();
}

void NumberPicker::initializeSelectorWheel(){
    initializeSelectorWheelIndices();
    std::vector<int>& selectorIndices = mSelectorIndices;
    const int selectedHeight= std::max(mSelectedTextSize,mSelectedText->getHeight());
    const int totalTextSize = int ((selectorIndices.size() - 1) * mSelectedTextSize + selectedHeight);
    const float textGapCount = selectorIndices.size();
    if (isHorizontalMode()) {
        float totalTextGapWidth = getWidth() - totalTextSize;
        mSelectorTextGapWidth = (int) (totalTextGapWidth / textGapCount);
        mSelectorElementSize = (int) getMaxTextSize() + mSelectorTextGapWidth;
        mInitialScrollOffset = (int) (mSelectedTextCenterX - mSelectorElementSize * mWheelMiddleItemIndex);
    } else {
        float totalTextGapHeight = getHeight() - totalTextSize;
        mSelectorTextGapHeight = (int) (totalTextGapHeight / textGapCount);
        mSelectorElementSize = (int) getMaxTextSize() + mSelectorTextGapHeight;
        mInitialScrollOffset = (int) (mSelectedTextCenterY - mSelectorElementSize * mWheelMiddleItemIndex-(selectedHeight-mSelectorElementSize)/2);
    }
    mCurrentScrollOffset = mInitialScrollOffset;
    updateInputTextView();
}

void NumberPicker::initializeFadingEdges(){
    if(isHorizontalMode()){
        setHorizontalFadingEdgeEnabled(true);
        setVerticalFadingEdgeEnabled(false);
        setFadingEdgeLength((getWidth() - mTextSize)/2);
    }else{
        setHorizontalFadingEdgeEnabled(false);
        setVerticalFadingEdgeEnabled(true);
        setFadingEdgeLength((getHeight() - mTextSize)/2);
    }
}

void NumberPicker::onScrollerFinished(Scroller* scroller) {
    if (scroller == mFlingScroller) {
        ensureScrollWheelAdjusted();
        updateInputTextView();
        onScrollStateChange(OnScrollListener::SCROLL_STATE_IDLE);
    } else {
        if (mScrollState != OnScrollListener::SCROLL_STATE_TOUCH_SCROLL) {
            updateInputTextView();
        }
    }
}

void NumberPicker::onScrollStateChange(int scrollState) {
    if (mScrollState == scrollState)  return;
    mScrollState = scrollState;
    if (mOnScrollListener.onScrollStateChange) mOnScrollListener.onScrollStateChange(*this, scrollState);
}

void NumberPicker::fling(int velocity) {
    if (isHorizontalMode()){
        mPreviousScrollerX = 0;
        if (velocity > 0) {
            mFlingScroller->fling(0, 0, velocity, 0, 0, INT_MAX, 0, 0);
        } else {
            mFlingScroller->fling(INT_MAX, 0, velocity, 0, 0, INT_MAX, 0, 0);
        }
    }else{
        mPreviousScrollerY = 0;
        if (velocity > 0) {
            mFlingScroller->fling(0, 0, 0, velocity, 0, 0, 0,INT_MAX);
        } else {
            mFlingScroller->fling(0, INT_MAX, 0, velocity, 0, 0, 0,INT_MAX);
        }
    }
}

int NumberPicker::getWrappedSelectorIndex(int selectorIndex){
   if (selectorIndex > mMaxValue) {
        return mMinValue + (selectorIndex - mMaxValue) % (mMaxValue - mMinValue) - 1;
    } else if (selectorIndex < mMinValue) {
        return mMaxValue - (mMinValue - selectorIndex) % (mMaxValue - mMinValue) + 1;
    }
    return selectorIndex;
}

void NumberPicker::incrementSelectorIndices(std::vector<int>&selectorIndices){
    for (int i = 0; i < selectorIndices.size() - 1; i++) {
        selectorIndices[i] = selectorIndices[i + 1];
    }
    int nextScrollSelectorIndex = selectorIndices[selectorIndices.size() - 2] + 1;
    if (mWrapSelectorWheel && nextScrollSelectorIndex > mMaxValue) {
        nextScrollSelectorIndex = mMinValue;
    }
    selectorIndices[selectorIndices.size() - 1] = nextScrollSelectorIndex;
    ensureCachedScrollSelectorValue(nextScrollSelectorIndex);
}

void NumberPicker::decrementSelectorIndices(std::vector<int>&selectorIndices) {
    for (int i = selectorIndices.size() - 1; i > 0; i--) {
        selectorIndices[i] = selectorIndices[i - 1];
    }
    int nextScrollSelectorIndex = selectorIndices[1] - 1;
    if (mWrapSelectorWheel && nextScrollSelectorIndex < mMinValue) {
        nextScrollSelectorIndex = mMaxValue;
    }
    selectorIndices[0] = nextScrollSelectorIndex;
    ensureCachedScrollSelectorValue(nextScrollSelectorIndex);
}

void NumberPicker::ensureCachedScrollSelectorValue(int selectorIndex) {
    std::string scrollSelectorValue;
    std::map<int,std::string>& cache = mSelectorIndexToStringCache;
    auto itr= cache.find(selectorIndex);

    if (itr != cache.end()) return;

    if (selectorIndex < mMinValue || selectorIndex > mMaxValue) {
        scrollSelectorValue = "";
    } else {
        if (mDisplayedValues.size()){
            const int displayedValueIndex = selectorIndex - mMinValue;
            if(displayedValueIndex >=mDisplayedValues.size()){
                cache.erase(itr);
                return;
            }
            scrollSelectorValue = mDisplayedValues[displayedValueIndex];
        } else {
            scrollSelectorValue = formatNumber(selectorIndex);
        }
    }
    cache[selectorIndex] = scrollSelectorValue;
}

std::string NumberPicker::formatNumber(int value){
    return (mFormatter != nullptr) ? mFormatter(value):std::to_string(value);
}

void NumberPicker::validateInputTextView(View* v){
    std::string str =((TextView*)v)->getText();// String.valueOf(((TextView*) v)->getText());
    if (str.empty()){ // Restore to the old value as we don't allow empty values
        updateInputTextView();
    } else {  // Check the new value and ensure it's in range
        int current = getSelectedPos(str);//.toString());
        setValueInternal(current, true);
    }
}

bool NumberPicker::updateInputTextView(){
    std::string text = (mDisplayedValues.size() == 0) ? formatNumber(mValue) : mDisplayedValues[mValue - mMinValue];
    if (!text.empty() ){
        std::string beforeText = mSelectedText->getText();
        if (text != beforeText){//!text.equals(beforeText.toString())) {
            mSelectedText->setText(text);
            return true;
        }
    }

    return false;
}

void NumberPicker::notifyChange(int previous, int current){
    if (mOnValueChangeListener) {
        mOnValueChangeListener(*this, previous, mValue);
    }
}

void NumberPicker::postChangeCurrentByOneFromLongPress(bool increment, long delayMillis){
    if(mChangeCurrentByOneFromLongPressCommand!=nullptr)
        removeCallbacks(mChangeCurrentByOneFromLongPressCommand);

        mChangeCurrentByOneFromLongPressCommand=[this,increment](){
        changeValueByOne(increment);
        postDelayed(mChangeCurrentByOneFromLongPressCommand, mLongPressUpdateInterval);
    };
    postDelayed(mChangeCurrentByOneFromLongPressCommand, delayMillis);
}

void NumberPicker::postChangeCurrentByOneFromLongPress(bool increment){
    postChangeCurrentByOneFromLongPress(increment, ViewConfiguration::getLongPressTimeout());
}
void NumberPicker::removeChangeCurrentByOneFromLongPress(){
    if (mChangeCurrentByOneFromLongPressCommand != nullptr) {
        removeCallbacks(mChangeCurrentByOneFromLongPressCommand);
        mChangeCurrentByOneFromLongPressCommand =nullptr;
    }
}

void NumberPicker::removeBeginSoftInputCommand(){
    if(mBeginSoftInputOnLongPressCommand!=nullptr){
        removeCallbacks(mBeginSoftInputOnLongPressCommand);
        mBeginSoftInputOnLongPressCommand=nullptr;
    }
}

void NumberPicker::postBeginSoftInputOnLongPressCommand(){
    if(mBeginSoftInputOnLongPressCommand!=nullptr)
        removeCallbacks(mBeginSoftInputOnLongPressCommand);
    mBeginSoftInputOnLongPressCommand=[this](){
        performLongClick();
    };
    postDelayed(mBeginSoftInputOnLongPressCommand,ViewConfiguration::getLongPressTimeout());
}


void NumberPicker::removeAllCallbacks(){
    if (mChangeCurrentByOneFromLongPressCommand != nullptr) {
        removeCallbacks(mChangeCurrentByOneFromLongPressCommand);
        mChangeCurrentByOneFromLongPressCommand =nullptr;
    }
    removeBeginSoftInputCommand();
    pshCancel();
}

int NumberPicker::getSelectedPos(const std::string& value){
    if (mDisplayedValues.size()==0){
        return std::strtol(value.c_str(),nullptr,10);
    } else {
        for (int i = 0; i < mDisplayedValues.size(); i++) {
            // Don't force the user to type in jan when ja will do
            //value = value.toLowerCase();
            //if (mDisplayedValues[i].toLowerCase().startsWith(value)) return mMinValue + i;
            if( TextUtils::startWith(mDisplayedValues[i],value))return mMinValue+i;
        }
        /* The user might have typed in a number into the month field i.e.
        * 10 instead of OCT so support that too.*/
        return std::strtol(value.c_str(),nullptr,10);//Integer.parseInt(value);
    }
    return mMinValue;
}

void  NumberPicker::ensureScrollWheelAdjusted() {
    // adjust to the closest value
    int delta = mInitialScrollOffset - mCurrentScrollOffset;
    if (delta == 0) return;

    if(std::abs(delta)>mSelectorElementSize/2){
        delta += (delta > 0) ? -mSelectorElementSize : mSelectorElementSize;
    }
    if(isHorizontalMode()){
        mPreviousScrollerX = 0;
        mAdjustScroller->startScroll(0, 0,delta,0, SELECTOR_ADJUSTMENT_DURATION_MILLIS);
    }else{
        mPreviousScrollerY = 0;
        mAdjustScroller->startScroll(0, 0, 0, delta, SELECTOR_ADJUSTMENT_DURATION_MILLIS);
        LOGV("delta=%d finished=%d time=%d",delta,mAdjustScroller->isFinished(),SELECTOR_ADJUSTMENT_DURATION_MILLIS);
    }
    invalidate();
}

void NumberPicker::pshCancel(){
    mPSHMode = 0; 
    mPSHManagedButton = 0;
    if(mPressedStateHelpers != nullptr){
        removeCallbacks(mPressedStateHelpers);
        invalidate(0, mBottomSelectionDividerBottom, mRight-mLeft, mBottom-mTop);
    }
    mPressedStateHelpers = [this](){
        pshRun();
    };
    mDecrementVirtualButtonPressed = false;
    if(mDecrementVirtualButtonPressed)
        invalidate(0,0,mRight-mLeft,mTopSelectionDividerTop);
}

void NumberPicker::pshButtonPressDelayed(int button){
    pshCancel();
    mPSHMode = MODE_PRESS;
    mPSHManagedButton = button;
    
    postDelayed(mPressedStateHelpers,ViewConfiguration::getTapTimeout());
}

void NumberPicker::pshButtonTapped(int button){
    pshCancel();
    mPSHMode = MODE_TAPPED;
    mPSHManagedButton = button;
    post(mPressedStateHelpers);
}

void NumberPicker::pshRun(){
    switch (mPSHMode) {
    case MODE_PRESS:
        switch (mPSHManagedButton) {
        case R::id::increment:
             mIncrementVirtualButtonPressed = true;
             invalidate(0, mBottomSelectionDividerBottom, mRight, mBottom);
             break;
        case R::id::decrement:
             mDecrementVirtualButtonPressed = true;
             invalidate(0, 0, mRight, mTopSelectionDividerTop);
             break;
        }
        break;
    case MODE_TAPPED:
        switch (mPSHManagedButton) {
        case R::id::increment:
            if (!mIncrementVirtualButtonPressed) {
                postDelayed(mPressedStateHelpers,ViewConfiguration::getPressedStateDuration());
            }
            mIncrementVirtualButtonPressed ^= true;
            invalidate(0, mBottomSelectionDividerBottom, mRight, mBottom);
             break;
        case R::id::decrement:
            if (!mDecrementVirtualButtonPressed) {
                postDelayed(mPressedStateHelpers,ViewConfiguration::getPressedStateDuration());
            }
            mDecrementVirtualButtonPressed ^= true;
            invalidate(0, 0, mRight, mTopSelectionDividerTop);
        }//endof switch (mManagedButton)
        break;/*endof case MODE_TAPPED*/
    }
}

}//namespace
