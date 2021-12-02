#include <widget/numberpicker.h>
#include <widget/layoutinflater.h>
#include <color.h>
#include <textutils.h>
#include <cdlog.h>

//https://gitee.com/awang/WheelView/blob/master/src/com/wangjie/wheelview/WheelView.java

namespace cdroid{

DECLARE_WIDGET(NumberPicker)

NumberPicker::NumberPicker(int w,int h):LinearLayout(w,h){
    initView();
    setOrientation(VERTICAL);//HORIZONTAL);

    LayoutInflater*inflater=LayoutInflater::from(mContext);
    inflater->inflate("cdroid:layout/number_picker.xml",this);
    delete inflater;
   
    if(!mHasSelectorWheel){
        mDecrementButton=(ImageButton*)findViewById(BUTTON_DECREMENT);
        mDecrementButton->setMinimumHeight(20);
        addView(mDecrementButton,new LayoutParams(LayoutParams::WRAP_CONTENT,LayoutParams::WRAP_CONTENT));
        mDecrementButton->setOnClickListener(std::bind(&NumberPicker::onIncDecClick,this,std::placeholders::_1));
        mDecrementButton->setOnLongClickListener(std::bind(&NumberPicker::onIncDecLongClick,this,std::placeholders::_1));
    }
   
    mInputText =(EditText*)findViewById(0);//new EditText("123",20,20);
    if(mInputText){
        mInputText->setTextAlignment(View::TEXT_ALIGNMENT_CENTER);
        mInputText->setText("123");
        mInputText->setTextSize(24);
        addView(mInputText,new LayoutParams(LayoutParams::MATCH_PARENT,LayoutParams::WRAP_CONTENT));
    }

    if(!mHasSelectorWheel){
        mIncrementButton=(ImageButton*)findViewById(BUTTON_INCREMENT);
        mIncrementButton->setMinimumHeight(20);
        addView(mIncrementButton,new LayoutParams(LayoutParams::WRAP_CONTENT,LayoutParams::WRAP_CONTENT));
        mIncrementButton->setOnClickListener(std::bind(&NumberPicker::onIncDecClick,this,std::placeholders::_1));
        mIncrementButton->setOnLongClickListener(std::bind(&NumberPicker::onIncDecLongClick,this,std::placeholders::_1));
    }
    measure(MeasureSpec::makeMeasureSpec(w,MeasureSpec::EXACTLY),MeasureSpec::makeMeasureSpec(h,MeasureSpec::EXACTLY));
    layout(0,0,getMeasuredWidth(),getMeasuredHeight());
    LOGD("%d,%d-%d,%d (%dx%d)",mInputText->getLeft(),mInputText->getTop(),mInputText->getWidth(),mInputText->getHeight(),w,h);
    updateInputTextView();
}

NumberPicker::NumberPicker(Context* context,const AttributeSet& atts)
  :LinearLayout(context,atts){
    initView();
    mHideWheelUntilFocused = atts.getBoolean("hideWheelUntilFocused",false);
    mSolidColor =atts.getColor("solidColor",0);
    mSelectionDivider =context->getDrawable(atts.getString("selectionDivider"));
    if (mSelectionDivider) {
        mSelectionDivider->setCallback(this);
        mSelectionDivider->setLayoutDirection(getLayoutDirection());
        if (mSelectionDivider->isStateful()) {
            mSelectionDivider->setState(getDrawableState());
        }
    }
    mSelectionDividerHeight = atts.getDimensionPixelSize("selectionDividerHeight",UNSCALED_DEFAULT_SELECTION_DIVIDER_HEIGHT);
    mSelectionDividersDistance=atts.getDimensionPixelSize("selectionDividersDistance",UNSCALED_DEFAULT_SELECTION_DIVIDERS_DISTANCE);
    mMinHeight = atts.getDimensionPixelSize("internalMinHeight",SIZE_UNSPECIFIED);
    mMaxHeight = atts.getDimensionPixelSize("internalMaxHeight",SIZE_UNSPECIFIED);
    
    mMinWidth = atts.getDimensionPixelSize("internalMinWidth", SIZE_UNSPECIFIED);
    mMaxWidth = atts.getDimensionPixelSize("internalMaxWidth", SIZE_UNSPECIFIED);

    if (mMinWidth != SIZE_UNSPECIFIED && mMaxWidth != SIZE_UNSPECIFIED
                && mMinWidth > mMaxWidth) {
        throw "minWidth > maxWidth";
    }
    LayoutInflater*inflater=LayoutInflater::from(mContext);
    inflater->inflate("cdroid:layout/number_picker.xml",this);
    delete inflater;
    mComputeMaxWidth = (mMaxWidth == SIZE_UNSPECIFIED);
    setWillNotDraw(!mHasSelectorWheel);

    if (!mHasSelectorWheel) {
        mIncrementButton = (ImageButton*)findViewById(BUTTON_INCREMENT);//R.id.increment);
        mIncrementButton->setOnClickListener(std::bind(&NumberPicker::onIncDecClick,this,std::placeholders::_1));
        mIncrementButton->setOnLongClickListener(std::bind(&NumberPicker::onIncDecLongClick,this,std::placeholders::_1));
    } else {
        mIncrementButton = nullptr;
    }

    // decrement button
    if (!mHasSelectorWheel) {
        mDecrementButton = (ImageButton*)findViewById(BUTTON_DECREMENT);//R.id.decrement);
        mDecrementButton->setOnClickListener(std::bind(&NumberPicker::onIncDecClick,this,std::placeholders::_1));
        mDecrementButton->setOnLongClickListener(std::bind(&NumberPicker::onIncDecLongClick,this,std::placeholders::_1));
    } else {
        mDecrementButton = nullptr;
    }

    mInputText =(EditText*)findViewById(0);//R.id.numberpicker_input
    ViewConfiguration configuration = ViewConfiguration::get(context);
    mTextSize = (int) mInputText->getTextSize();

    updateInputTextView();

}

NumberPicker::~NumberPicker(){
    LOGD("mChildren.size=%d",mChildren.size());
}

void NumberPicker::onIncDecClick(View&v){
    hideSoftInput();
    mInputText->clearFocus();
    if (&v == mIncrementButton) {
        changeValueByOne(true);
    } else {
        changeValueByOne(false);
    }
}

bool NumberPicker::onIncDecLongClick(View&v){
    hideSoftInput();
    mInputText->clearFocus();
    if (&v == mIncrementButton){
        postChangeCurrentByOneFromLongPress(true, 0);
    } else {
        postChangeCurrentByOneFromLongPress(false, 0);
    }
    return true;
}

void NumberPicker::initView(){
    ViewConfiguration&config=ViewConfiguration::get(mContext);
    mIncrementButton=nullptr;
    mDecrementButton=nullptr;
    mInputText=nullptr;
    mOnValueChangeListener=nullptr;
    mFormatter=nullptr;
    mOnScrollListener.onScrollStateChange = nullptr;
    mScrollState = OnScrollListener::SCROLL_STATE_IDLE;
    mTextSize = 24;
    mSolidColor =0xFF222222;
    mSelectionDivider = nullptr;
    mVirtualButtonPressedDrawable =nullptr;
    mLastHandledDownDpadKeyCode =-1;
    mHasSelectorWheel = true;
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
    mSelectionDividersDistance =UNSCALED_DEFAULT_SELECTION_DIVIDERS_DISTANCE;
    mVelocityTracker=nullptr;

    mTouchSlop = config.getScaledTouchSlop();
    mMinimumFlingVelocity = config.getScaledMinimumFlingVelocity();
    mMaximumFlingVelocity = config.getScaledMaximumFlingVelocity()/ SELECTOR_MAX_FLING_VELOCITY_ADJUSTMENT;
    mFlingScroller = new Scroller(getContext(), nullptr, true);
    mAdjustScroller = new Scroller(getContext(), new DecelerateInterpolator(2.5f));
    mComputeMaxWidth = (mMaxWidth == SIZE_UNSPECIFIED);
    setSelector(DEFAULT_SELECTOR_WHEEL_ITEM_COUNT,-1);
}
void NumberPicker::onLayout(bool changed, int left, int top, int width, int height){
    if (!mHasSelectorWheel) {
        LinearLayout::onLayout(changed, left, top, width, height);
        return;
    }
    const int msrdWdth = getMeasuredWidth();
    const int msrdHght = getMeasuredHeight();

    // Input text centered horizontally.
    const int inptTxtMsrdWdth = mInputText->getMeasuredWidth();
    const int inptTxtMsrdHght = mInputText->getMeasuredHeight();
    if(getOrientation()==LinearLayout::HORIZONTAL){
        int inptTxtLeft= (msrdWdth - inptTxtMsrdWdth) / 2;
        mInputText->layout(inptTxtLeft, 0, inptTxtMsrdWdth, inptTxtMsrdHght);
    }else{
        int inptTxtTop= (msrdHght - inptTxtMsrdHght)/2;
        mInputText->layout(0,inptTxtTop,inptTxtMsrdWdth,inptTxtMsrdHght);
    }

    if (changed) { // need to do all this when we know our size
        initializeSelectorWheel();
        initializeFadingEdges();
        mTopSelectionDividerTop = (getHeight() - mSelectionDividersDistance) / 2
                - mSelectionDividerHeight;
        mBottomSelectionDividerBottom = mTopSelectionDividerTop + 2 * mSelectionDividerHeight
                + mSelectionDividersDistance;
    }
}


void NumberPicker::onMeasure(int widthMeasureSpec, int heightMeasureSpec){
    if (!mHasSelectorWheel) {
        LinearLayout::onMeasure(widthMeasureSpec, heightMeasureSpec);
        return;
    }
    // Try greedily to fit the max width and height.
    int newWidthMeasureSpec = makeMeasureSpec(widthMeasureSpec, mMaxWidth);
    int newHeightMeasureSpec = makeMeasureSpec(heightMeasureSpec, mMaxHeight);
    LinearLayout::onMeasure(newWidthMeasureSpec, newHeightMeasureSpec);
    // Flag if we are measured with width or height less than the respective min.
    int widthSize = resolveSizeAndStateRespectingMinSize(mMinWidth, getMeasuredWidth(),
                widthMeasureSpec);
    int heightSize = resolveSizeAndStateRespectingMinSize(mMinHeight, getMeasuredHeight(),
                heightMeasureSpec);
    setMeasuredDimension(widthSize, heightSize);
}


bool NumberPicker::moveToFinalScrollerPosition(Scroller* scroller) {
    scroller->forceFinished(true);
    int amountToScroll = scroller->getFinalY() - scroller->getCurrY();
    int futureScrollOffset = (mCurrentScrollOffset + amountToScroll) % mSelectorElementHeight;
    int overshootAdjustment = mInitialScrollOffset - futureScrollOffset;
    if (overshootAdjustment != 0) {
        if (std::abs(overshootAdjustment) > mSelectorElementHeight / 2) {
            if (overshootAdjustment > 0) {
                overshootAdjustment -= mSelectorElementHeight;
            } else {
                overshootAdjustment += mSelectorElementHeight;
            }
        }
        amountToScroll += overshootAdjustment;
        scrollBy(0, amountToScroll);
        return true;
    }
    return false;
}
bool NumberPicker::onInterceptTouchEvent(MotionEvent& event){
    if (!mHasSelectorWheel || !isEnabled()) {
        return false;
    }
    int action = event.getActionMasked();
    switch (action) {
    case MotionEvent::ACTION_DOWN:
        removeAllCallbacks();
        hideSoftInput();
        mLastDownOrMoveEventY = mLastDownEventY = event.getY();
        mLastDownEventTime = event.getEventTime();
        mIgnoreMoveEvents  = false;
        mPerformClickOnTap = false;
        // Handle pressed state before any state change.
        if (mLastDownEventY < mTopSelectionDividerTop) {
            if (mScrollState == OnScrollListener::SCROLL_STATE_IDLE) {
                pshButtonPressDelayed(BUTTON_DECREMENT);
            }
        } else if (mLastDownEventY > mBottomSelectionDividerBottom) {
            if (mScrollState == OnScrollListener::SCROLL_STATE_IDLE) {
                pshButtonPressDelayed(BUTTON_INCREMENT);
            }
        }
        // Make sure we support flinging inside scrollables.
        getParent()->requestDisallowInterceptTouchEvent(true);
        if (!mFlingScroller->isFinished()) {
            mFlingScroller->forceFinished(true);
            mAdjustScroller->forceFinished(true);
            onScrollStateChange(OnScrollListener::SCROLL_STATE_IDLE);
        } else if (!mAdjustScroller->isFinished()) {
            mFlingScroller->forceFinished(true);
            mAdjustScroller->forceFinished(true);
        } else if (mLastDownEventY < mTopSelectionDividerTop) {
            postChangeCurrentByOneFromLongPress(
                false, ViewConfiguration::getLongPressTimeout());
        } else if (mLastDownEventY > mBottomSelectionDividerBottom) {
            postChangeCurrentByOneFromLongPress(
                true, ViewConfiguration::getLongPressTimeout());
        } else {
            mPerformClickOnTap = true;
            postBeginSoftInputOnLongPressCommand();
        }
        return true;
    }//endswitch action
    return false;
}

bool NumberPicker::onTouchEvent(MotionEvent& event){
    if (!isEnabled() || !mHasSelectorWheel) {
        return false;
    }
    if (mVelocityTracker==nullptr) mVelocityTracker = VelocityTracker::obtain();

    mVelocityTracker->addMovement(event);
    int action = event.getActionMasked();
    switch (action) {
    case MotionEvent::ACTION_CANCEL:LOGD("ACTION_CANCEL");break;
    case MotionEvent::ACTION_MOVE: {
            if (mIgnoreMoveEvents) {
                 break;
            }
            float currentMoveY = event.getY();
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
        } break;
    case MotionEvent::ACTION_UP: {
            removeBeginSoftInputCommand();
            removeChangeCurrentByOneFromLongPress();
            pshCancel();//mPressedStateHelper.cancel();
            //VelocityTracker* velocityTracker = mVelocityTracker;
            mVelocityTracker->computeCurrentVelocity(1000, mMaximumFlingVelocity);
            int initialVelocity = (int) mVelocityTracker->getYVelocity();
            if (std::abs(initialVelocity) > mMinimumFlingVelocity) {
                fling(initialVelocity);
                onScrollStateChange(OnScrollListener::SCROLL_STATE_FLING);
            } else {
                int eventY = (int) event.getY();
                int deltaMoveY = (int) std::abs(eventY - mLastDownEventY);
                long deltaTime = event.getEventTime() - mLastDownEventTime;
                if (deltaMoveY <= mTouchSlop && deltaTime < ViewConfiguration::getTapTimeout()) {
                    if (mPerformClickOnTap) {
                        mPerformClickOnTap = false;
                        performClick();
                    } else {
                        int selectorIndexOffset = (eventY / mSelectorElementHeight) - mMiddleItemIndex;
                        if (selectorIndexOffset > 0) {
                            changeValueByOne(true);
                            pshButtonTapped(BUTTON_INCREMENT);
                        } else if (selectorIndexOffset < 0) {
                            changeValueByOne(false);
                            pshButtonTapped(BUTTON_DECREMENT);
                        }
                    }
                } else {
                    ensureScrollWheelAdjusted();
                }
                onScrollStateChange(OnScrollListener::SCROLL_STATE_IDLE);
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
        if (!mHasSelectorWheel) {
            break;
        }
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
    int currentScrollerY = scroller->getCurrY();
    if (mPreviousScrollerY == 0) {
        mPreviousScrollerY = scroller->getStartY();
    }
    scrollBy(0, currentScrollerY - mPreviousScrollerY);
    mPreviousScrollerY = currentScrollerY;
    if (scroller->isFinished()) {
        onScrollerFinished(scroller);
    } else {
        invalidate();
    }
}

View& NumberPicker::setEnabled(bool enabled) {
    ViewGroup::setEnabled(enabled);
    if (!mHasSelectorWheel) {
        mIncrementButton->setEnabled(enabled);
    }
    if (!mHasSelectorWheel) {
        mDecrementButton->setEnabled(enabled);
    }
    mInputText->setEnabled(enabled);
    return *this;
}

void NumberPicker::scrollBy(int x, int y){
    std::vector<int>&selectorIndices = mSelectorIndices;
    int startScrollOffset = mCurrentScrollOffset;
    if (!mWrapSelectorWheel && y > 0
            && selectorIndices[mMiddleItemIndex] <= mMinValue) {
        mCurrentScrollOffset = mInitialScrollOffset;
        return;
    }
    if (!mWrapSelectorWheel && y < 0
            && selectorIndices[mMiddleItemIndex] >= mMaxValue) {
        mCurrentScrollOffset = mInitialScrollOffset;
        return;
    }
    mCurrentScrollOffset += y;
    while (mCurrentScrollOffset - mInitialScrollOffset > mSelectorTextGapHeight) {
        mCurrentScrollOffset -= mSelectorElementHeight;
        decrementSelectorIndices(selectorIndices);
        setValueInternal(selectorIndices[mMiddleItemIndex], true);
        if (!mWrapSelectorWheel && selectorIndices[mMiddleItemIndex] <= mMinValue) {
            mCurrentScrollOffset = mInitialScrollOffset;
        }
    }

    while (mCurrentScrollOffset - mInitialScrollOffset < -mSelectorTextGapHeight) {
        mCurrentScrollOffset += mSelectorElementHeight;
        incrementSelectorIndices(selectorIndices);
        setValueInternal(selectorIndices[mMiddleItemIndex], true);
        if (!mWrapSelectorWheel && selectorIndices[mMiddleItemIndex] >= mMaxValue) {
            mCurrentScrollOffset = mInitialScrollOffset;
        }
    }
    if (startScrollOffset != mCurrentScrollOffset) {
        onScrollChanged(0, mCurrentScrollOffset, 0, startScrollOffset);
    }
}

int NumberPicker::computeVerticalScrollOffset() {
    return mCurrentScrollOffset;
}

int NumberPicker::computeVerticalScrollRange() {
    return (mMaxValue - mMinValue + 1) * mSelectorElementHeight;
}

int NumberPicker::computeVerticalScrollExtent() {
    return getHeight();
}

int NumberPicker::getSolidColor()const{
    return mSolidColor;
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

void NumberPicker::setValue(int value) {
    setValueInternal(value, false);
}

bool NumberPicker::performClick() {
    if (!mHasSelectorWheel) {
        return ViewGroup::performClick();
    } else if (!ViewGroup::performClick()) {
        showSoftInput();
    }
    return true;
}

bool NumberPicker::performLongClick() {
    if (!mHasSelectorWheel) {
        return ViewGroup::performLongClick();
    } else if (!ViewGroup::performLongClick()) {
        showSoftInput();
        mIgnoreMoveEvents = true;
    }
    return true;
}

void NumberPicker::showSoftInput(){
}

void NumberPicker::hideSoftInput(){
}

void NumberPicker::tryComputeMaxWidth(){

}

bool NumberPicker::getWrapSelectorWheel()const{
    return mWrapSelectorWheel;
}

void NumberPicker::setWrapSelectorWheel(bool wrapSelectorWheel) {
    mWrapSelectorWheelPreferred = wrapSelectorWheel;
    updateWrapSelectorWheel();
}

void NumberPicker::updateWrapSelectorWheel() {
    bool wrappingAllowed = (mMaxValue - mMinValue) >= mSelectorIndices.size();
    mWrapSelectorWheel = wrappingAllowed && mWrapSelectorWheelPreferred;
}

void NumberPicker::setOnLongPressUpdateInterval(long intervalMillis) {
    mLongPressUpdateInterval = intervalMillis;
}

void NumberPicker::setSelector(int items,int focused){
    mSelectorIndices.resize(items);
    if(focused<0)focused=items/2;
    mMiddleItemIndex=focused;
    updateWrapSelectorWheel();
    initializeSelectorWheelIndices();
    //updateInputTextView();
    invalidate();
}

int NumberPicker::getValue()const{
    return mValue;
}

int NumberPicker::getMinValue()const{
    return mMinValue;
}

void NumberPicker::setMinValue(int minValue){

    if (mMinValue == minValue)return;

    if (minValue < 0) throw "minValue must be >= 0";

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

    if (mMaxValue == maxValue) return;

    if (maxValue < 0) throw "maxValue must be >= 0";

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
    updateInputTextView();
    initializeSelectorWheelIndices();
    tryComputeMaxWidth();
}

void NumberPicker::drawableStateChanged() {
    ViewGroup::drawableStateChanged();

    Drawable* selectionDivider = mSelectionDivider;
    if (selectionDivider!=nullptr  && selectionDivider->isStateful()
        && selectionDivider->setState(getDrawableState())) {
        invalidateDrawable(*selectionDivider);
    }
}

void NumberPicker::jumpDrawablesToCurrentState() {
    ViewGroup::jumpDrawablesToCurrentState();

    if (mSelectionDivider != nullptr) {
        mSelectionDivider->jumpToCurrentState();
    }
}

void NumberPicker::onDraw(Canvas&canvas){
    if (!mHasSelectorWheel) {
        LinearLayout::onDraw(canvas);
        return;
    }
    bool showSelectorWheel = mHideWheelUntilFocused ? hasFocus() : true;
    float x = (mRight-mLeft) / 2;
    float y = mCurrentScrollOffset;
    Rect rc=mInputText->getBound();
    // draw the virtual buttons pressed state if needed
    if (showSelectorWheel && mVirtualButtonPressedDrawable != nullptr
            && mScrollState == OnScrollListener::SCROLL_STATE_IDLE) {
        if (mDecrementVirtualButtonPressed) {
            mVirtualButtonPressedDrawable->setState(StateSet::get(StateSet::VIEW_STATE_PRESSED));
            mVirtualButtonPressedDrawable->setBounds(0, 0, mRight-mLeft, mTopSelectionDividerTop);
            mVirtualButtonPressedDrawable->draw(canvas);
        }
        if (mIncrementVirtualButtonPressed) {
            mVirtualButtonPressedDrawable->setState(StateSet::get(StateSet::VIEW_STATE_PRESSED));
            mVirtualButtonPressedDrawable->setBounds(0, mBottomSelectionDividerBottom, mRight-mLeft,mBottom-mTop);
            mVirtualButtonPressedDrawable->draw(canvas);
        }
    }

    // draw the selector wheel
    std::vector<int>& selectorIndices = mSelectorIndices;
    ColorStateList* colors = mInputText->getTextColors();
    int selectorWheelColor = colors->getColorForState(StateSet::get(StateSet::VIEW_STATE_ENABLED), Color::WHITE);
    canvas.set_color(selectorWheelColor);
    canvas.set_font_size(mTextSize);
    Rect rctxt={0,mCurrentScrollOffset,mRight-mLeft,mSelectorElementHeight-mSelectorTextGapHeight/2};
    Layout txtlayout(mTextSize,mRight-mLeft);
    //txtlayout.setAlignment(mInputText->getLayoutAlignment());
    for (int i = 0; i < selectorIndices.size(); i++) {
        int selectorIndex = selectorIndices[i];
        std::string scrollSelectorValue = mSelectorIndexToStringCache[selectorIndex];
        // Do not draw the middle item if input is visible since the input is shown only if the wheel
        // is static and it covers the middle item. Otherwise, if the user starts editing the text 
        // via the/ IME he may see a dimmed version of the old value intermixed with the new one.
        if ((showSelectorWheel && i != mMiddleItemIndex) ||
            (i == mMiddleItemIndex && mInputText->getVisibility() != VISIBLE)) {
            canvas.draw_text(rctxt,scrollSelectorValue,DT_CENTER|DT_VCENTER);
        }
        rctxt.offset(0,mSelectorElementHeight);
        // draw the selector dividers
        if(mSelectionDivider && (mSelectorTextGapHeight>0) && (i!=selectorIndices.size()-1)){
             canvas.save();
             int alpha=255;
             if((i!=mMiddleItemIndex-1)&&(i!=mMiddleItemIndex))
                 alpha=(255-500.*std::abs(i-mMiddleItemIndex+(i>mMiddleItemIndex?1:0))/(selectorIndices.size()+1));
             mSelectionDivider->setAlpha(alpha);
             mSelectionDivider->setBounds(0,y+mTextSize-mCurrentScrollOffset+mSelectorTextGapHeight,mRight-mLeft,std::min(mSelectorTextGapHeight,mSelectionDividerHeight));
             mSelectionDivider->draw(canvas);
             canvas.restore();
        }
        y += mSelectorElementHeight;
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
    std::vector<int>& selectorIndices = mSelectorIndices;
    int current = getValue();
    for (int i = 0; i < mSelectorIndices.size(); i++) {
        int selectorIndex = current + (i - mMiddleItemIndex);
        if (mWrapSelectorWheel) {
            selectorIndex = getWrappedSelectorIndex(selectorIndex);
        }
        selectorIndices[i] = selectorIndex;
        ensureCachedScrollSelectorValue(selectorIndices[i]);
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
    if (mScrollState != OnScrollListener::SCROLL_STATE_FLING)updateInputTextView();
    if (notifyChng) notifyChange(previous, current);

    initializeSelectorWheelIndices();
    invalidate();
}

void NumberPicker::changeValueByOne(bool increment){
    if (mHasSelectorWheel) {
        hideSoftInput();
        if (!moveToFinalScrollerPosition(mFlingScroller)) {
            moveToFinalScrollerPosition(mAdjustScroller);
        }
        mPreviousScrollerY = 0;
        if (increment) {
            mFlingScroller->startScroll(0, 0, 0, -mSelectorElementHeight, SNAP_SCROLL_DURATION);
        } else {
            mFlingScroller->startScroll(0, 0, 0, mSelectorElementHeight, SNAP_SCROLL_DURATION);
        }
        invalidate();
    } else {
        if (increment) {
            setValueInternal(mValue + 1, true);
        } else {
            setValueInternal(mValue - 1, true);
        }
    }
}

void NumberPicker::initializeSelectorWheel(){
    initializeSelectorWheelIndices();
    std::vector<int>&selectorIndices = mSelectorIndices;
    int totalTextHeight = selectorIndices.size() * mTextSize;
    float totalTextGapHeight = mBottom-mTop - totalTextHeight;
    float textGapCount = selectorIndices.size();
    mSelectorTextGapHeight = (int) (totalTextGapHeight / textGapCount + 0.5f);
    mSelectorElementHeight = mTextSize + mSelectorTextGapHeight;
    // Ensure that the middle item is positioned the same as the text in mInputText
    int editTextTextPosition = mInputText->getBaseline() + mInputText->getTop();
    mInitialScrollOffset = editTextTextPosition  - (mSelectorElementHeight * mMiddleItemIndex);
    mCurrentScrollOffset = mInitialScrollOffset;
    updateInputTextView();
}

void NumberPicker::initializeFadingEdges(){
    setVerticalFadingEdgeEnabled(true);
    setFadingEdgeLength((mRight-mLeft - mTextSize) / 2);
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

void NumberPicker::fling(int velocityY) {
    mPreviousScrollerY = 0;
    if (velocityY > 0) {
        mFlingScroller->fling(0, 0, 0, velocityY, 0, 0, 0,INT_MAX);
    } else {
        mFlingScroller->fling(0, INT_MAX, 0, velocityY, 0, 0, 0,INT_MAX);
    }
    invalidate();
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
    std::map<int,std::string>& cache = mSelectorIndexToStringCache;
    auto itr= cache.find(selectorIndex);

    std::string scrollSelectorValue;
    if (itr != cache.end()) return;

    if (selectorIndex < mMinValue || selectorIndex > mMaxValue) {
        scrollSelectorValue = "";
    } else {
        if (mDisplayedValues.size()){
            int displayedValueIndex = selectorIndex - mMinValue;
            scrollSelectorValue = mDisplayedValues[displayedValueIndex];
        } else {
            scrollSelectorValue = formatNumber(selectorIndex);
        }
    }
    cache[selectorIndex]=scrollSelectorValue;
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
        std::string beforeText = mInputText->getText();
        if (text != beforeText){//!text.equals(beforeText.toString())) {
            mInputText->setText(text);
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

bool NumberPicker::ensureScrollWheelAdjusted() {
    // adjust to the closest value
    int deltaY = mInitialScrollOffset - mCurrentScrollOffset;
    if (deltaY != 0) {
        mPreviousScrollerY = 0;
        if (std::abs(deltaY) > mSelectorElementHeight / 2) {
            deltaY += (deltaY > 0) ? -mSelectorElementHeight : mSelectorElementHeight;
        }
        mAdjustScroller->startScroll(0, 0, 0, deltaY, SELECTOR_ADJUSTMENT_DURATION_MILLIS);
        invalidate();
        return true;
    }
    return false;
}

void NumberPicker::pshCancel(){
    mPSHMode =0; 
    mPSHManagedButton=0;
    if(mPressedStateHelpers!=nullptr){
         removeCallbacks(mPressedStateHelpers);
         invalidate(0, mBottomSelectionDividerBottom, mRight-mLeft, mBottom-mTop);
    }
    mPressedStateHelpers =[this](){
        pshRun();
    };
    mDecrementVirtualButtonPressed =false;
    if(mDecrementVirtualButtonPressed)
         invalidate(0,0,mRight-mLeft,mTopSelectionDividerTop);
}

void NumberPicker::pshButtonPressDelayed(int button){
    pshCancel();
    mPSHMode =MODE_PRESS;
    mPSHManagedButton =button;
    
    postDelayed(mPressedStateHelpers,ViewConfiguration::getTapTimeout());
}

void NumberPicker::pshButtonTapped(int button){
    pshCancel();
    mPSHMode =MODE_TAPPED;
    mPSHManagedButton=button;
    post(mPressedStateHelpers);
}

void NumberPicker::pshRun(){
    switch (mPSHMode) {
    case MODE_PRESS:
        switch (mPSHManagedButton) {
        case BUTTON_INCREMENT:
             mIncrementVirtualButtonPressed = true;
             invalidate(0, mBottomSelectionDividerBottom, mRight, mBottom);
             break;
        case BUTTON_DECREMENT:
             mDecrementVirtualButtonPressed = true;
             invalidate(0, 0, mRight, mTopSelectionDividerTop);
             break;
        }
        break;
    case MODE_TAPPED:
        switch (mPSHManagedButton) {
        case BUTTON_INCREMENT:
            if (!mIncrementVirtualButtonPressed) {
                postDelayed(mPressedStateHelpers,ViewConfiguration::getPressedStateDuration());
            }
            mIncrementVirtualButtonPressed ^= true;
            invalidate(0, mBottomSelectionDividerBottom, mRight, mBottom);
             break;
        case BUTTON_DECREMENT:
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
