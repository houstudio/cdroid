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
#include <widget/numberpicker.h>
#include <view/accessibility/accessibilitymanager.h>
#include <widget/R.h>
#include <core/color.h>
#include <utils/textutils.h>
#include <utils/mathutils.h>
#include <porting/cdlog.h>

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
 
    mInputText =(EditText*)findViewById(R::id::numberpicker_input);
    if(mInputText){
        mInputText->setTextAlignment(View::TEXT_ALIGNMENT_CENTER);
        mInputTextSize = mInputText->getTextSize();
        mTextSize = mInputTextSize;
        mSelectorElementSize = mInputTextSize;
    }
    mIncrementButton =(ImageButton*)findViewById(cdroid::R::id::increment);
    mDecrementButton =(ImageButton*)findViewById(cdroid::R::id::decrement);
    View::OnClickListener onClick= [this](View& v) {
        hideSoftInput();
        mInputText->clearFocus();
        if (v.getId() == R::id::increment) {
            changeValueByOne(true);
        } else {
            changeValueByOne(false);
        }
    };
    View::OnLongClickListener onLongClick=[this](View& v) {
        hideSoftInput();
        mInputText->clearFocus();
        if (v.getId() == R::id::increment) {
            postChangeCurrentByOneFromLongPress(true, 0);
        } else {
            postChangeCurrentByOneFromLongPress(false, 0);
        }
        return true;
    };

    if(mIncrementButton){
        mIncrementButton->setOnClickListener(onClick);
        mIncrementButton->setOnLongClickListener(onLongClick);
    }
    if(mDecrementButton){
        mDecrementButton->setOnClickListener(onClick);
        mDecrementButton->setOnLongClickListener(onLongClick);
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
    mWrapSelectorWheelPreferred= atts.getBoolean("wrapSelectorWheel",mWrapSelectorWheelPreferred);
    mDividerDrawable = atts.getDrawable("selectionDivider");
    mInputTextSize = atts.getDimensionPixelSize("selectedTextSize",mInputTextSize);
    if (mDividerDrawable) {
        mDividerDrawable->setCallback(this);
        mDividerDrawable->setLayoutDirection(getLayoutDirection());
        if (mDividerDrawable->isStateful()) {
            mDividerDrawable->setState(getDrawableState());
        }
    }else{
        setDividerColor(atts.getColor("dividerColor",mDividerColor));
    }
    mItemBackground =  atts.getDrawable("itemBackground");
    if(mItemBackground){
        mItemBackground->setCallback(this);
        mItemBackground->setLayoutDirection(getLayoutDirection());
    }
    mOrder = ASCENDING;
    if(!isHorizontalMode()){
        mDividerThickness= atts.getDimensionPixelSize("selectionDividerHeight",UNSCALED_DEFAULT_SELECTION_DIVIDER_HEIGHT);
        mDividerDistance = atts.getDimensionPixelSize("selectionDividersDistance",UNSCALED_DEFAULT_SELECTION_DIVIDERS_DISTANCE);
    }else{
        mDividerThickness= atts.getDimensionPixelSize("selectionDividerWidth",UNSCALED_DEFAULT_SELECTION_DIVIDER_HEIGHT);
        mDividerDistance = atts.getDimensionPixelSize("selectionDividersDistance",UNSCALED_DEFAULT_SELECTION_DIVIDERS_DISTANCE);
    }
    mMinHeight = atts.getDimensionPixelSize("internalMinHeight",SIZE_UNSPECIFIED);
    mMaxHeight = atts.getDimensionPixelSize("internalMaxHeight",SIZE_UNSPECIFIED);
    
    mMinWidth = atts.getDimensionPixelSize("internalMinWidth", SIZE_UNSPECIFIED);
    mMaxWidth = atts.getDimensionPixelSize("internalMaxWidth", SIZE_UNSPECIFIED);

    if ((mMinWidth != SIZE_UNSPECIFIED) && (mMaxWidth != SIZE_UNSPECIFIED) && (mMinWidth > mMaxWidth) ){
        LOGE("minWidth(%d)  > maxWidth(%d)",mMinWidth,mMaxWidth);
    }
    const std::string defaultLayoutRes = (getOrientation()==LinearLayout::VERTICAL?DEFAULT_LAYOUT_VERT:DEFAULT_LAYOUT_HORZ);
    const std::string layoutRes = atts.getString("internalLayout",defaultLayoutRes);
    setWheelItemCount(atts.getInt("wheelItemCount",mWheelItemCount));
    mHasSelectorWheel = (defaultLayoutRes!=layoutRes)||(mWheelItemCount!=DEFAULT_WHEEL_ITEM_COUNT);
    LayoutInflater::from(mContext)->inflate(layoutRes,this);
    setWidthAndHeight();
    mComputeMaxWidth = (mMaxWidth == SIZE_UNSPECIFIED);
    mVirtualButtonPressedDrawable = atts.getDrawable("virtualButtonPressedDrawable");
    setWillNotDraw(false);

    mInputText =(EditText*)findViewById(cdroid::R::id::numberpicker_input);

    View::OnClickListener onClick= [this](View& v) {
        hideSoftInput();
        mInputText->clearFocus();
        if (v.getId() == R::id::increment) {
            changeValueByOne(true);
        } else {
            changeValueByOne(false);
        }
    };
    View::OnLongClickListener onLongClick=[this](View& v) {
        hideSoftInput();
        mInputText->clearFocus();
        if (v.getId() == R::id::increment) {
            postChangeCurrentByOneFromLongPress(true, 0);
        } else {
            postChangeCurrentByOneFromLongPress(false, 0);
        }
        return true;
    };
    if(!mHasSelectorWheel){
        mIncrementButton =(ImageButton*)findViewById(cdroid::R::id::increment);
        mDecrementButton =(ImageButton*)findViewById(cdroid::R::id::decrement);
        if(mIncrementButton){
            mIncrementButton->setOnClickListener(onClick);
            mIncrementButton->setOnLongClickListener(onLongClick);
        }
        if(mDecrementButton){
            mDecrementButton->setOnClickListener(onClick);
            mDecrementButton->setOnLongClickListener(onLongClick);
        }
    }else{
        mIncrementButton = nullptr;
        mDecrementButton = nullptr;
    }

    mInputText->setEnabled(false);
    mInputText->setFocusable(false);
    mUpdateInputTextInFling = atts.getBoolean("updateInputTextInFling",mUpdateInputTextInFling);
    mTextAlign = mInputText->getGravity();
    mInputTextSize = mInputText->getTextSize();
    mTypeface = Typeface::create(atts.getString("fontFamily"),Typeface::NORMAL);
    mSelectedTypeface = Typeface::create(atts.getString("selectedfontFamily"),Typeface::NORMAL);
    //ViewConfiguration configuration = ViewConfiguration::get(context);
    setTextSize(atts.getDimensionPixelSize("textSize",mTextSize));
    mTextSize2 = atts.getDimensionPixelSize("textSize2",mTextSize);
    if(atts.hasAttribute("selectedTextSize"))
        mInputTextSize = atts.getDimensionPixelSize("selectedTextSize");
    else if(!atts.hasAttribute("internalLayout"))
        mInputTextSize =std::max(mInputTextSize,mTextSize);
    setSelectedTextSize(mInputTextSize);
    setTextColor(atts.getColor("textColor"));
    setTextColor(mTextColor,atts.getColor("textColor2",mTextColor));
    setSelectedTextColor(atts.getColor("selectedTextColor"));
    const ColorStateList*colors = mInputText->getTextColors();
    if(colors&&colors->isStateful())
        setSelectedTextColor(colors->getColorForState(StateSet::get(StateSet::VIEW_STATE_ENABLED),mInputTextColor));
    else
        setSelectedTextColor(mInputText->getCurrentTextColor());
    updateInputTextView();

    //setWheelItemCount(atts.getInt("wheelItemCount",mWheelItemCount));
    setValue(atts.getInt("value",0));
    setMinValue(atts.getInt("min",0));
    setMaxValue(atts.getInt("max",0));

    std::vector<std::string>displayedValues;
    atts.getArray("displayedValues",displayedValues);
    const int valueCount = std::abs(getMinValue()-getMaxValue())+1;
    if(displayedValues.size()){
        if(displayedValues.size()==valueCount)
            setDisplayedValues(displayedValues);
        LOGW_IF(displayedValues.size()!=valueCount,"displayedValues %d,value(%d,%d)",displayedValues.size(),mMinValue,mMaxValue);
    }

    updateWrapSelectorWheel();
    LOGV("%p:%d textSize=%d,%d",this,mID,mInputTextSize,mTextSize);
    if(getFocusable()==View::FOCUSABLE_AUTO){
        setFocusable(int(View::FOCUSABLE));
        setFocusableInTouchMode(true);
    }
}

NumberPicker::~NumberPicker(){
    delete mPressedStateHelper;
    delete mDividerDrawable;
    delete mItemBackground;
    delete mVirtualButtonPressedDrawable;
    delete mBeginSoftInputOnLongPressCommand;
    delete mChangeCurrentByOneFromLongPressCommand;
    delete mFlingScroller;
    delete mAdjustScroller;
    delete mAccessibilityNodeProvider;
    for(auto d:mDisplayedDrawables)delete d;
}

bool NumberPicker::isHorizontalMode()const{
    return getOrientation() == HORIZONTAL;
}

bool NumberPicker::isAscendingOrder()const{
    return mOrder == ASCENDING;
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

const DecelerateInterpolator sDecelerateInterpolator(2.5);

void NumberPicker::initView(){
    ViewConfiguration&config= ViewConfiguration::get(mContext);
    mDisplayedDrawableCount = 0;
    mDisplayedDrawableSize = 0;
    mInputText = nullptr;
    mIncrementButton= nullptr;
    mDecrementButton= nullptr;
    mOnValueChangeListener = nullptr;
    mFormatter = nullptr;
    mOnScrollListener.onScrollStateChange = nullptr;
    mScrollState = OnScrollListener::SCROLL_STATE_IDLE;
    mTextSize   = 24;
    mItemSpacing= 0;
    mInputTextSize = 24;
    mEndDividerEnd = 0;
    mStartDividerStart= 0;
    mInputTextColor = 0xFFFFFFFF;
    mSelectedTypeface = nullptr;
    mComputeMaxWidth  = false;
    mHasSelectorWheel = false;
    mTypeface = nullptr;
    mItemBackground = nullptr;
    mVirtualButtonPressedDrawable = nullptr;
    mBeginSoftInputOnLongPressCommand = nullptr;
    mChangeCurrentByOneFromLongPressCommand = nullptr;
    mDividerColor = DEFAULT_DIVIDER_COLOR;
    mWheelMiddleItemIndex = 0;
    mDividerDrawable  = nullptr;
    mDividerThickness =2;
    mDividerType = SIDE_LINES;
    mLastHandledDownDpadKeyCode = -1;
    mWrapSelectorWheel= false;
    mWrapSelectorWheelPreferred = true;
    mUpdateInputTextInFling = false;
    mIgnoreMoveEvents  = false;
    mPerformClickOnTap = false;
    mIncrementVirtualButtonPressed = false;
    mDecrementVirtualButtonPressed = false;
    mPreviousScrollerY   = 0;
    mCurrentScrollOffset = 0;
    mInitialScrollOffset = INT_MIN;
    mLongPressUpdateInterval = DEFAULT_LONG_PRESS_UPDATE_INTERVAL;
    mMinHeight = SIZE_UNSPECIFIED;
    mMaxHeight = SIZE_UNSPECIFIED;
    mMinWidth  = SIZE_UNSPECIFIED;
    mMaxWidth  = SIZE_UNSPECIFIED;
    mValue    = 0;
    mMinValue = 0;
    mMaxValue = 0;
    mPressedStateHelper = new PressedStateHelper(this);
    mInputTextGapHeight = 0;
    mSelectorElementSize = 1;//avoid divide by zero.
    mDividerDistance =UNSCALED_DEFAULT_SELECTION_DIVIDERS_DISTANCE;
    mVelocityTracker = nullptr;
    mAccessibilityNodeProvider = nullptr;

    mTouchSlop = config.getScaledTouchSlop();
    mMinimumFlingVelocity = config.getScaledMinimumFlingVelocity();
    mMaximumFlingVelocity = config.getScaledMaximumFlingVelocity()/ SELECTOR_MAX_FLING_VELOCITY_ADJUSTMENT;
    mFlingScroller  = new Scroller(getContext(), nullptr, true);
    mAdjustScroller = new Scroller(getContext(), &sDecelerateInterpolator);
    mComputeMaxWidth = (mMaxWidth == SIZE_UNSPECIFIED);
    mHideWheelUntilFocused = false;
    mWheelItemCount = DEFAULT_WHEEL_ITEM_COUNT;
    mRealWheelItemCount= DEFAULT_WHEEL_ITEM_COUNT;
    mSelectorIndices.resize(mWheelItemCount);
}

void NumberPicker::onLayout(bool changed, int left, int top, int width, int height){
    if (!mHasSelectorWheel&&mIncrementButton&&mDecrementButton) {
        if(!isHorizontalMode()){
            const int btnh = mIncrementButton->getMeasuredHeight();
            mIncrementButton->layout( 0, 0, width, btnh);
            mDecrementButton->layout( 0, height - btnh, width, btnh);
        }else{
            const int btnw = mIncrementButton->getMeasuredWidth();
            if(!isLayoutRtl()){
                mIncrementButton->layout( 0, 0, btnw, height);
                mDecrementButton->layout( width - btnw, 0, btnw, height);
            }else{
                mDecrementButton->layout( 0, 0, btnw, height);
                mIncrementButton->layout( width - btnw, 0, btnw, height);
            }
        }
    }
    const int msrdWdth = getMeasuredWidth();
    const int msrdHght = getMeasuredHeight();

    // Input text centered horizontally.
    const int inptTxtMsrdWdth = mInputText->getMeasuredWidth();
    const int inptTxtMsrdHght = mInputText->getMeasuredHeight();
    const int inptTxtLeft= (msrdWdth - inptTxtMsrdWdth) /2;
    const int inptTxtTop = (msrdHght - inptTxtMsrdHght)/2;

    mInputText->layout(inptTxtLeft, inptTxtTop, inptTxtMsrdWdth, inptTxtMsrdHght);
    mInputTextCenter = (isHorizontalMode()?getWidth():getHeight())/2;
    if (changed) { // need to do all this when we know our size
        initializeSelectorWheel();
        initializeFadingEdges();
        if(isHorizontalMode()){
            const int w = std::max(inptTxtMsrdWdth,mSelectorElementSize);
            if(w > mDividerDistance) mDividerDistance = w;
            mStartDividerStart = (getWidth() - mDividerDistance)/2 - mDividerThickness;
            mEndDividerEnd= mStartDividerStart + mDividerDistance + 2*mDividerThickness;
            mInputText->layout((msrdWdth - w) /2, inptTxtTop,w, inptTxtMsrdHght);
        }else{
            const int h = std::max(inptTxtMsrdHght,mSelectorElementSize);
            if(h > mDividerDistance) mDividerDistance = h;
            mStartDividerStart = (getHeight() - mDividerDistance)/2 - mDividerThickness;
            mEndDividerEnd = mStartDividerStart + mDividerDistance+2*mDividerThickness;
        }
    }
}

void NumberPicker::onMeasure(int widthMeasureSpec, int heightMeasureSpec){
    if (!mHasSelectorWheel) {
        LinearLayout::onMeasure(widthMeasureSpec, heightMeasureSpec);
    }
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

        int reduced = amountToScroll%mSelectorElementSize;
        if(reduced < 0) reduced += mSelectorElementSize;
        if (overshootAdjustment != 0) {
            if (std::abs(overshootAdjustment) > mSelectorElementSize / 2) {
                if (overshootAdjustment > 0) {
                    overshootAdjustment -= mSelectorElementSize;
                } else {
                    overshootAdjustment += mSelectorElementSize;
                }
            }
            amountToScroll += overshootAdjustment;
            scrollBy(reduced + overshootAdjustment/*amountToScroll*/, 0);
            return true;
        }
    }else{
        int amountToScroll = scroller->getFinalY() - scroller->getCurrY();
        int futureScrollOffset = (mCurrentScrollOffset + amountToScroll) % mSelectorElementSize;
        int overshootAdjustment = mInitialScrollOffset - futureScrollOffset;

        int reduced = amountToScroll%mSelectorElementSize;
        if(reduced < 0) reduced += mSelectorElementSize;
        if (overshootAdjustment != 0) {
            if (std::abs(overshootAdjustment) > mSelectorElementSize / 2) {
                if (overshootAdjustment > 0) {
                    overshootAdjustment -= mSelectorElementSize;
                } else {
                    overshootAdjustment += mSelectorElementSize;
                }
            }
            amountToScroll += overshootAdjustment;
            scrollBy(0, reduced + overshootAdjustment/*amountToScroll*/);
            return true;
        }
    }
    return false;
}

bool NumberPicker::onInterceptTouchEvent(MotionEvent& event){
    const int action = event.getActionMasked();
    if (!mHasSelectorWheel || !isEnabled() ||(action!=MotionEvent::ACTION_DOWN)) {
        return false;
    }
    mIgnoreMoveEvents = false;
    mPerformClickOnTap = false;
    removeAllCallbacks();
    if(isHorizontalMode()){
        mLastDownOrMoveEventX = mLastDownEventX = event.getX();
        mLastDownEventTime = event.getEventTime();

        // Handle pressed state before any state change.
        if (mLastDownEventX < mStartDividerStart) {
            if (mScrollState == OnScrollListener::SCROLL_STATE_IDLE) {
                mPressedStateHelper->buttonPressDelayed(PressedStateHelper::BUTTON_DECREMENT);
            }
        } else if (mLastDownEventX > mEndDividerEnd) {
            if (mScrollState == OnScrollListener::SCROLL_STATE_IDLE) {
                mPressedStateHelper->buttonPressDelayed(PressedStateHelper::BUTTON_INCREMENT);
            }
        }
        // Make sure we support flinging inside scrollables.
        getParent()->requestDisallowInterceptTouchEvent(true);
        if (!mFlingScroller->isFinished()) {
            mFlingScroller->forceFinished(true);
            mAdjustScroller->forceFinished(true);
            onScrollerFinished(mFlingScroller);
            onScrollStateChange(OnScrollListener::SCROLL_STATE_IDLE);
        } else if (!mAdjustScroller->isFinished()) {
            mFlingScroller->forceFinished(true);
            mAdjustScroller->forceFinished(true);
            onScrollerFinished(mAdjustScroller);
        } else if (mLastDownEventX< mStartDividerStart) {
            postChangeCurrentByOneFromLongPress(false, ViewConfiguration::getLongPressTimeout());
        } else if (mLastDownEventX > mEndDividerEnd) {
            postChangeCurrentByOneFromLongPress(true, ViewConfiguration::getLongPressTimeout());
        } else{
            mPerformClickOnTap = true;
            postBeginSoftInputOnLongPressCommand();
        }
    }else{
        mLastDownOrMoveEventY = mLastDownEventY = event.getY();
        // Handle pressed state before any state change.
        if (mLastDownEventY < mStartDividerStart) {
            if (mScrollState == OnScrollListener::SCROLL_STATE_IDLE) {
                mPressedStateHelper->buttonPressDelayed(PressedStateHelper::BUTTON_DECREMENT);
            }
        } else if (mLastDownEventY > mEndDividerEnd) {
            if (mScrollState == OnScrollListener::SCROLL_STATE_IDLE) {
                mPressedStateHelper->buttonPressDelayed(PressedStateHelper::BUTTON_INCREMENT);
            }
        }
        // Make sure we support flinging inside scrollables.
        getParent()->requestDisallowInterceptTouchEvent(true);
        if (!mFlingScroller->isFinished()) {
            mFlingScroller->forceFinished(true);
            mAdjustScroller->forceFinished(true);
            onScrollerFinished(mFlingScroller);
            onScrollStateChange(OnScrollListener::SCROLL_STATE_IDLE);
        } else if (!mAdjustScroller->isFinished()) {
            mFlingScroller->forceFinished(true);
            mAdjustScroller->forceFinished(true);
            onScrollerFinished(mAdjustScroller);
        } else if (mLastDownEventY < mStartDividerStart) {
            postChangeCurrentByOneFromLongPress(false, ViewConfiguration::getLongPressTimeout());
        } else if (mLastDownEventY > mEndDividerEnd) {
            postChangeCurrentByOneFromLongPress(true, ViewConfiguration::getLongPressTimeout());
        } else{
            mPerformClickOnTap = true;
            postBeginSoftInputOnLongPressCommand();
        }        
    }//endif isHorizontalMode
    return true;
}

bool NumberPicker::onTouchEvent(MotionEvent& event){
    if (!isEnabled() || !mHasSelectorWheel) {
        return false;
    }
    if (mVelocityTracker == nullptr) mVelocityTracker = VelocityTracker::obtain();

    mVelocityTracker->addMovement(event);
    int action = event.getActionMasked();
    switch (action) {
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
    case MotionEvent::ACTION_UP:
    case MotionEvent::ACTION_CANCEL:
        removeBeginSoftInputCommand();
        removeChangeCurrentByOneFromLongPress();
        mPressedStateHelper->cancel();
        mVelocityTracker->computeCurrentVelocity(1000, mMaximumFlingVelocity);
        if(isHorizontalMode()){
            const int initialVelocity = (int) mVelocityTracker->getXVelocity();
            if (std::abs(initialVelocity) > mMinimumFlingVelocity) {
                fling(initialVelocity);
                onScrollStateChange(OnScrollListener::SCROLL_STATE_FLING);
            } else {
                const int eventX = (int) event.getX();
                const int deltaMoveX = (int) std::abs(eventX - mLastDownEventX);
                if (deltaMoveX <= mTouchSlop) {
                    if( mPerformClickOnTap ){
                        mPerformClickOnTap = false;
                        performClick();
                        if ( (mScrollState != OnScrollListener::SCROLL_STATE_TOUCH_SCROLL) && mOnClickListener &&
                             (mLastDownEventX >= mStartDividerStart && mLastDownEventX <= mEndDividerEnd)) {
                            mOnClickListener(*this);
                        }
                    }else{
                        int selectorIndexOffset = (eventX / mSelectorElementSize) - mWheelMiddleItemIndex;
                        if (selectorIndexOffset > 0) {
                            changeValueByOne(true);
                        } else if (selectorIndexOffset < 0) {
                            changeValueByOne(false);
                        } else {
                            ensureScrollWheelAdjusted();
                        }
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
                    if( mPerformClickOnTap ){
                        mPerformClickOnTap = false;
                        performClick();
                        if ( (mScrollState != OnScrollListener::SCROLL_STATE_TOUCH_SCROLL) && mOnClickListener &&
                             (mLastDownEventY >= mStartDividerStart && mLastDownEventY <= mEndDividerEnd)) {
                            mOnClickListener(*this);
                        }
                    }else{
                        int selectorIndexOffset = (eventY / mSelectorElementSize) - mWheelMiddleItemIndex;
                        if (selectorIndexOffset > 0) {
                            changeValueByOne(true);
                            mPressedStateHelper->buttonTapped(PressedStateHelper::BUTTON_INCREMENT);
                        } else if (selectorIndexOffset < 0) {
                            changeValueByOne(false);
                            mPressedStateHelper->buttonTapped(PressedStateHelper::BUTTON_DECREMENT);
                        }else{
                            ensureScrollWheelAdjusted();
                        }
                    }
                }else{
                    ensureScrollWheelAdjusted();
                }
                onScrollStateChange(OnScrollListener::SCROLL_STATE_IDLE);
            }
        }
        mVelocityTracker->recycle();
        mVelocityTracker = nullptr;
        break;
    }//end switch
    return true;
}

bool NumberPicker::dispatchTouchEvent(MotionEvent& event){
    const int action = event.getActionMasked();
    switch (action) {
    case MotionEvent::ACTION_CANCEL:
    case MotionEvent::ACTION_UP:
        removeAllCallbacks();
        break;
    }
    return LinearLayout::dispatchTouchEvent(event);
}

bool NumberPicker::dispatchKeyEvent(KeyEvent& event){
    const int keyCode = event.getKeyCode();
    switch (keyCode) {
    case KeyEvent::KEYCODE_DPAD_CENTER:
    case KeyEvent::KEYCODE_ENTER:
        removeAllCallbacks();
        break;
    case KeyEvent::KEYCODE_DPAD_DOWN:
    case KeyEvent::KEYCODE_DPAD_UP:
        if(!mHasSelectorWheel){
            break;
        }
        switch (event.getAction()) {
        case KeyEvent::ACTION_DOWN:
            if (mWrapSelectorWheel || ((keyCode == KeyEvent::KEYCODE_DPAD_DOWN)
                    ? getValue() < getMaxValue() : getValue() > getMinValue())) {
            requestFocus();
            mLastHandledDownDpadKeyCode = keyCode;
            removeAllCallbacks();
            changeValueByOne(keyCode == KeyEvent::KEYCODE_DPAD_DOWN);
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

bool NumberPicker::dispatchHoverEvent(MotionEvent& event) {
    if (!mHasSelectorWheel) {
        return LinearLayout::dispatchHoverEvent(event);
    }

    if (AccessibilityManager::getInstance(mContext).isEnabled()) {
        const int eventY = (int) event.getY();
        int hoveredVirtualViewId;
        if (eventY < mStartDividerStart) {
            hoveredVirtualViewId = AccessibilityNodeProviderImpl::VIRTUAL_VIEW_ID_DECREMENT;
        } else if (eventY > mEndDividerEnd) {
            hoveredVirtualViewId = AccessibilityNodeProviderImpl::VIRTUAL_VIEW_ID_INCREMENT;
        } else {
            hoveredVirtualViewId = AccessibilityNodeProviderImpl::VIRTUAL_VIEW_ID_INPUT;
        }
        const int action = event.getActionMasked();
        AccessibilityNodeProviderImpl* provider =(AccessibilityNodeProviderImpl*) getAccessibilityNodeProvider();
        switch (action) {
            case MotionEvent::ACTION_HOVER_ENTER: {
                provider->sendAccessibilityEventForVirtualView(hoveredVirtualViewId, AccessibilityEvent::TYPE_VIEW_HOVER_ENTER);
                mLastHoveredChildVirtualViewId = hoveredVirtualViewId;
                provider->performAction(hoveredVirtualViewId, AccessibilityNodeInfo::ACTION_ACCESSIBILITY_FOCUS, nullptr);
            } break;
            case MotionEvent::ACTION_HOVER_MOVE: {
                if (mLastHoveredChildVirtualViewId != hoveredVirtualViewId
                        && mLastHoveredChildVirtualViewId != View::NO_ID) {
                    provider->sendAccessibilityEventForVirtualView( mLastHoveredChildVirtualViewId, AccessibilityEvent::TYPE_VIEW_HOVER_EXIT);
                    provider->sendAccessibilityEventForVirtualView(hoveredVirtualViewId, AccessibilityEvent::TYPE_VIEW_HOVER_ENTER);
                    mLastHoveredChildVirtualViewId = hoveredVirtualViewId;
                    provider->performAction(hoveredVirtualViewId, AccessibilityNodeInfo::ACTION_ACCESSIBILITY_FOCUS, nullptr);
                }
            } break;
            case MotionEvent::ACTION_HOVER_EXIT: {
                provider->sendAccessibilityEventForVirtualView(hoveredVirtualViewId, AccessibilityEvent::TYPE_VIEW_HOVER_EXIT);
                mLastHoveredChildVirtualViewId = View::NO_ID;
            } break;
        }
    }

    return false;
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
        const int currentScrollerX = scroller->getCurrX();
        if (mPreviousScrollerX == 0) {
            mPreviousScrollerX = scroller->getStartX();
        }
        scrollBy(currentScrollerX - mPreviousScrollerX, 0);
        mPreviousScrollerX = currentScrollerX;
    }else{
        const int currentScrollerY = scroller->getCurrY();
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

void NumberPicker::setEnabled(bool enabled) {
    ViewGroup::setEnabled(enabled);
    if(!mHasSelectorWheel){
        if(mIncrementButton)mIncrementButton->setEnabled(enabled);
        if(mDecrementButton)mDecrementButton->setEnabled(enabled);
    }
    mInputText->setEnabled(enabled);
}

void NumberPicker::scrollBy(int x, int y){
    std::vector<int>&selectorIndices = mSelectorIndices;
    const int startScrollOffset = mCurrentScrollOffset;
    const int gap = mSelectorElementSize/2;
    const int xy = isHorizontalMode() ? x : y;
    if (isAscendingOrder()) {
        if (!mWrapSelectorWheel && xy > 0
                && selectorIndices[mWheelMiddleItemIndex] <= mMinValue) {//changed from <=--><,make items wrapable
            mCurrentScrollOffset = mInitialScrollOffset;
            return;
        }
        if (!mWrapSelectorWheel && xy < 0
                && selectorIndices[mWheelMiddleItemIndex] >= mMaxValue) {
            mCurrentScrollOffset = mInitialScrollOffset;
            return;
        }
    } else {
        if (!mWrapSelectorWheel && xy > 0
                && selectorIndices[mWheelMiddleItemIndex] >= mMaxValue) {//changed from >=-->>,make items wrapable
            mCurrentScrollOffset = mInitialScrollOffset;
            return;
        }
        if (!mWrapSelectorWheel && xy < 0
                && selectorIndices[mWheelMiddleItemIndex] <= mMinValue) {
            mCurrentScrollOffset = mInitialScrollOffset;
            return;
        }
    }
    mCurrentScrollOffset += xy;
    int loop1=0,loop2=0;
    int oldCurrentScrollOffset= mCurrentScrollOffset;
    while (mCurrentScrollOffset - mInitialScrollOffset >  gap) {
        mCurrentScrollOffset -= mSelectorElementSize;
        if(isAscendingOrder())
            decrementSelectorIndices(selectorIndices);
        else
            incrementSelectorIndices(selectorIndices);loop1++;
        setValueInternal(selectorIndices[mWheelMiddleItemIndex], true);
        if (!mWrapSelectorWheel && (selectorIndices[mWheelMiddleItemIndex] <= mMinValue)) {
            mCurrentScrollOffset = mInitialScrollOffset;
        }
    }

    while (mCurrentScrollOffset - mInitialScrollOffset < - gap) {
        mCurrentScrollOffset += mSelectorElementSize;
        if(isAscendingOrder())
            incrementSelectorIndices(selectorIndices);
        else
            decrementSelectorIndices(selectorIndices);loop2++;
        setValueInternal(selectorIndices[mWheelMiddleItemIndex], true);
        if (!mWrapSelectorWheel && (selectorIndices[mWheelMiddleItemIndex] >= mMaxValue)) {
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

void NumberPicker::setOnClickListener(const OnClickListener& onClickListener){
    mOnClickListener = onClickListener;
}
void NumberPicker::setOnValueChangedListener(const OnValueChangeListener& onValueChangedListener){
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

float NumberPicker::getMaxTextSize()const {
    return std::max(std::max(mTextSize,mDisplayedDrawableSize), mInputTextSize);
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
    }
    return true;
}

void NumberPicker::showSoftInput(){
    if(mHasSelectorWheel){
	   mInputText->setVisibility(View::VISIBLE);
    }
}

void NumberPicker::hideSoftInput(){
    if (mInputText->getInputType() != EditText::TYPE_NONE) {
        if(mHasSelectorWheel){
            mInputText->setVisibility(View::INVISIBLE);
        }
    }
}

void NumberPicker::tryComputeMaxWidth(){
    if (!mComputeMaxWidth) {
        return;
    }
    int maxTextWidth = 0;
    Layout l(mTextSize,-1);
    l.setTypeface(mInputText->getTypeface());
    if (mDisplayedValues.size() == 0) {
        float maxDigitWidth = 0;
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
        for (int i = 0; i < valueCount; i++) {
            l.setText(mDisplayedValues[i]);
            l.relayout();
            const float textWidth = l.getMaxLineWidth();
            if (textWidth > maxTextWidth) {
                maxTextWidth = (int) textWidth;
            }
        }
    }
    maxTextWidth += mInputText->getPaddingLeft() + mInputText->getPaddingRight();
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

EditText*NumberPicker::getSelectedText()const{
    return mInputText;
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
    for(auto d:mDisplayedDrawables)delete d;
    mDisplayedDrawables.clear();
    mDisplayedDrawableCount = 0;
    mDisplayedDrawableSize = 0;
    int drsize=0;
    for(auto s:mDisplayedValues){
        Drawable*dr = mContext->getDrawable(s);
        mDisplayedDrawables.push_back(dr);
        if(dr){
            drsize += (isHorizontalMode()?dr->getIntrinsicWidth():dr->getIntrinsicHeight());
            mDisplayedDrawableCount++;
        }
    }
    if(mDisplayedDrawableCount==mDisplayedValues.size())
        mInputText->setVisibility(View::INVISIBLE); 
    if(mDisplayedDrawableCount)
        mDisplayedDrawableSize = drsize/mDisplayedDrawableCount;
}
/**
 * Set the height for the divider that separates the currently selected value from the others.
 * @param height The height to be set
 */
void NumberPicker::setSelectionDividerHeight(int height) {
    mDividerThickness = height;
    invalidate();
}

/**
 * Retrieve the height for the divider that separates the currently selected value from the
 * others.
 * @return The height of the divider
 */
int NumberPicker::getSelectionDividerHeight()const{
    return mDividerThickness;
}

float NumberPicker::getTopFadingEdgeStrength(){
    return TOP_AND_BOTTOM_FADING_EDGE_STRENGTH;
}

float NumberPicker::getBottomFadingEdgeStrength(){
    return TOP_AND_BOTTOM_FADING_EDGE_STRENGTH;
}

void NumberPicker::drawableStateChanged() {
    ViewGroup::drawableStateChanged();

    if (mDividerDrawable  && mDividerDrawable->isStateful()
        && mDividerDrawable->setState(getDrawableState())) {
        invalidateDrawable(*mDividerDrawable);
    }

    if (mItemBackground  && mItemBackground->isStateful()
        && mItemBackground->setState(getDrawableState())) {
        invalidateDrawable(*mItemBackground);
    }
}

void NumberPicker::jumpDrawablesToCurrentState() {
    ViewGroup::jumpDrawablesToCurrentState();

    if (mDividerDrawable)
        mDividerDrawable->jumpToCurrentState();

    if (mItemBackground)
        mItemBackground->jumpToCurrentState();
}

void NumberPicker::onResolveDrawables(int layoutDirection){
    ViewGroup::onResolveDrawables(layoutDirection);

    if (mDividerDrawable)
        mDividerDrawable->setLayoutDirection(layoutDirection);

    if (mItemBackground)
        mItemBackground->setLayoutDirection(layoutDirection);
}

void NumberPicker::setTextColor(int color){
    mTextColor = color;
    mTextColor2= color;
    invalidate();
}

void NumberPicker::setTextColor(int color,int color2){
    mTextColor  = color;
    mTextColor2 = color2;
    invalidate();
}

int  NumberPicker::getTextColor()const{
    return mTextColor;
}

void NumberPicker::setTextSize(int size){
    mTextSize  = size;
    mTextSize2 = size;
    invalidate();
}
void NumberPicker::setTextSize(int size,int size2){
    mTextSize  = size;
    mTextSize2 = size2;
    requestLayout();
    invalidate();
}

int  NumberPicker::getTextSize()const{
    return mTextSize;
}

int  NumberPicker::getSelectedTextColor()const{
    return mInputTextColor;
}

void NumberPicker::setSelectedTextColor(int textColor){
    mInputTextColor = textColor;
    mInputText->setTextColor(textColor);
}

int  NumberPicker::getSelectedTextSize()const{
    return mInputTextSize;
}

void NumberPicker::setSelectedTextSize(int textSize) {
    mInputTextSize = textSize;
    mInputText->setTextSize(textSize);
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
        mInputText->setTypeface(mTypeface);
        setSelectedTypeface(mSelectedTypeface);
    } else {
        mInputText->setTypeface(Typeface::MONOSPACE);
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
    float x=0, y=0;
    Rect recText;
    const int textGravity = mInputText?mInputText->getGravity():Gravity::CENTER;
    canvas.save();
    if (isHorizontalMode()) {
        x = mCurrentScrollOffset - mSelectorElementSize/2;
        recText = Rect::Make(x,y,mSelectorElementSize,getHeight());
        if (mRealWheelItemCount < DEFAULT_WHEEL_ITEM_COUNT) {
            canvas.rectangle(mStartDividerStart, 0, mEndDividerEnd - mStartDividerStart + mDividerThickness, getHeight());
            canvas.clip();
        }
    } else {
        y = mCurrentScrollOffset - mSelectorElementSize/2;
        recText = Rect::Make(0,y,getWidth(),mSelectorElementSize);
        if (mRealWheelItemCount < DEFAULT_WHEEL_ITEM_COUNT) {
            canvas.rectangle(0, mStartDividerStart, getWidth(), mEndDividerEnd - mStartDividerStart + mDividerThickness);
            canvas.clip();
        }
    }
    if (showSelectorWheel && mVirtualButtonPressedDrawable && (mScrollState == OnScrollListener::SCROLL_STATE_IDLE)){
        if (mDecrementVirtualButtonPressed) {
            mVirtualButtonPressedDrawable->setState(StateSet::PRESSED_STATE_SET);
            if(!isHorizontalMode())
                mVirtualButtonPressedDrawable->setBounds(0, 0, getWidth() , mStartDividerStart);
            else
                mVirtualButtonPressedDrawable->setBounds(0, 0, mStartDividerStart , getHeight());
            mVirtualButtonPressedDrawable->draw(canvas);
        }
        if (mIncrementVirtualButtonPressed) {
            mVirtualButtonPressedDrawable->setState(StateSet::PRESSED_STATE_SET);
            if(!isHorizontalMode())
                mVirtualButtonPressedDrawable->setBounds(0, mEndDividerEnd, getWidth(), getHeight() - mEndDividerEnd);
            else
                mVirtualButtonPressedDrawable->setBounds(mEndDividerEnd, 0, getWidth() - mEndDividerEnd, getHeight());
            mVirtualButtonPressedDrawable->draw(canvas);
        }
    }
    if( mTextColor != mTextColor2 ){
        if( mPat == nullptr ) {
            Color c1(mTextColor), c2(mTextColor2);
            CycleInterpolator ci(0.5f);
            if(isHorizontalMode())
                mPat = Cairo::LinearGradient::create( 0 , 0 , getWidth() , 0);
            else
                mPat = Cairo::LinearGradient::create( 0 , 0 , 0, getHeight());
            const int cStops = mSelectorIndices.size()*3;
            for(int i = 0; i < cStops ;i++){
                const float offset = float(i)/cStops;
                const float fraction = ci.getInterpolation(offset);
                mPat->add_color_stop_rgba(offset,MathUtils::lerp(c2.red(),c1.red(),fraction),MathUtils::lerp(c2.green(),c1.green(),fraction),
                                 MathUtils::lerp(c2.blue(),c1.blue(),fraction), std::abs(MathUtils::lerp(c2.alpha(),c1.alpha(),fraction)));
            }
        }
        canvas.set_source(mPat);
    }else{
        canvas.set_color(mTextColor);
    }
    canvas.set_font_face(mInputText->getTypeface()->getFontFace()->get_font_face());
    canvas.set_font_size(mTextSize);
    // draw the selector wheel
    for (int i = 0; i < mSelectorIndices.size(); i++) {
        float font_size  = mTextSize;
        int selectedSize = mSelectorElementSize;
        const float harfSize = (isHorizontalMode()?getWidth():getHeight())/2.f;
        const float fraction = std::abs( (isHorizontalMode()?x:y) - harfSize + mSelectorElementSize/2)/harfSize;
        if(mTextSize != mTextSize2){
            font_size = MathUtils::lerp(float(mTextSize),float(mTextSize2),fraction);
            canvas.set_font_size(font_size);
        }

        const int selectorIndex = mSelectorIndices[isAscendingOrder() ? i : mSelectorIndices.size() - i - 1];
        std::string scrollSelectorValue = mSelectorIndexToStringCache.at(selectorIndex);
        if (scrollSelectorValue.empty()) {
            if(isHorizontalMode()){
                x += selectedSize;
                recText.offset(selectedSize,0);
            }else{
                y += selectedSize;
                recText.offset(0,selectedSize);
            }
            continue;
        }
        if(mInputText->getVisibility()==View::VISIBLE){
            if(isHorizontalMode()==false){
                if(i==mWheelMiddleItemIndex)
                    selectedSize = std::max(mSelectorElementSize,mInputText->getHeight());
                recText.height = selectedSize;
            }else{
                if(i==mWheelMiddleItemIndex)
                    selectedSize = std::max(mSelectorElementSize,mInputText->getWidth());
                recText.width = selectedSize;
            }
        }
        // Do not draw the middle item if input is visible since the input
        // is shown only if the wheel is static and it covers the middle item.
        // Otherwise, if the user starts editing the text via the IME he may 
        // see a dimmed version of the old value intermixed with the new one.
        if ((showSelectorWheel && i != mWheelMiddleItemIndex) || (i == mWheelMiddleItemIndex && mInputText->getVisibility() != VISIBLE)) {
            Drawable*dr = nullptr;
            if(selectorIndex<mDisplayedDrawables.size() && (dr = mDisplayedDrawables.at(selectorIndex))){
                Rect outRect;
                const ColorStateList*cl = getForegroundTintList();
                Gravity::apply(textGravity,dr->getIntrinsicWidth(),dr->getIntrinsicHeight(),recText,outRect,getLayoutDirection());
                dr->setBounds(outRect);
                if(cl){
                    const int color =cl->getColorForState((i != mWheelMiddleItemIndex?StateSet::NOTHING:StateSet::SELECTED_STATE_SET),0xFFFFFFFF);
                    dr->setTint(color);
                }
                dr->draw(canvas);
            }else{
                if(mItemBackground){
                    mItemBackground->setBounds(recText);
                    if(mTextColor != mTextColor2)
                        mItemBackground->setAlpha(255*(1.0-fraction));
                    if(mItemBackground->isStateful()){
                        std::vector<int>state = getDrawableState();
                        if(i==mWheelMiddleItemIndex)
                            state.push_back(StateSet::SELECTED);
                        mItemBackground->setState(state);
                    }
                    mItemBackground->draw(canvas);
                }
                canvas.draw_text(recText,scrollSelectorValue,textGravity);
            }
        }
        if (isHorizontalMode()) {
            x += selectedSize;
            recText.offset(selectedSize,0);
        } else {
            y += selectedSize;
            recText.offset(0,selectedSize);
        }
    }
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

    switch (mDividerType) {
    case SIDE_LINES:
        if (mDividerThickness > 0 && mDividerDistance <= mMaxHeight) {
            top = (mMaxHeight - mDividerDistance) / 2;
            bottom = top + mDividerDistance;
        } else {
            top = 0;
            bottom = getBottom();
        }
        // draw the left divider
        mDividerDrawable->setBounds(mStartDividerStart, top, mDividerThickness, bottom-top);
        mDividerDrawable->draw(canvas);
        // draw the right divider
        mDividerDrawable->setBounds(mEndDividerEnd - mDividerThickness, top, mDividerThickness, bottom-top);
        mDividerDrawable->draw(canvas);
        break;
    case UNDERLINE:
        if (mDividerDistance > 0 && mDividerDistance <= mMaxWidth) {
            left = (mMaxWidth - mDividerDistance) / 2;
            right = left + mDividerDistance;
        } else {
            left = mStartDividerStart;
            right = mEndDividerEnd;
        }
        mDividerDrawable->setBounds(left,0,mDividerThickness,mBottom);
        mDividerDrawable->draw(canvas);
        break;
   }
}

void NumberPicker::drawVerticalDividers(Canvas& canvas) {
    const int left = getPaddingLeft();
    const int right= getWidth()-getPaddingRight();
    switch (mDividerType) {
    case SIDE_LINES:
        // draw the top divider
        mDividerDrawable->setBounds(0, mStartDividerStart, right-left, mDividerThickness);
        mDividerDrawable->draw(canvas);
        // draw the bottom divider
        mDividerDrawable->setBounds(left,mEndDividerEnd - mDividerThickness,right - left, mDividerThickness);
        mDividerDrawable->draw(canvas);
        break;
    case UNDERLINE:
        mDividerDrawable->setBounds(left,mEndDividerEnd - mDividerThickness,right-left, mDividerThickness);
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
    //const int count = (std::abs(mMaxValue - mMinValue) + 1);
    for (int i = 0; i < mSelectorIndices.size(); i++) {
        int selectorIndex = current + (i - mWheelMiddleItemIndex);
        if (mWrapSelectorWheel) {
            selectorIndex = getWrappedSelectorIndex(selectorIndex);
        }
        //if(mSelectorIndices.size() > count)
        //    selectorIndex = (selectorIndex + count)%count;
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
    if ( (mScrollState != OnScrollListener::SCROLL_STATE_FLING) || mUpdateInputTextInFling){
        updateInputTextView();
    }
	if(notifyChng)notifyChange(previous, current);
    initializeSelectorWheelIndices();
    invalidate();
}

void NumberPicker::changeValueByOne(bool increment){
    if (!moveToFinalScrollerPosition(mFlingScroller)) {
        moveToFinalScrollerPosition(mAdjustScroller);
    }
    if(mFlingScroller->isFinished()){
        smoothScroll(increment,1);
    }else{
        setValueInternal(mValue+(increment?1:-1),true);
    }
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
    const float textGapCount = selectorIndices.size();
    const int inputEdit_Size = isHorizontalMode()?mInputText->getWidth():mInputText->getHeight();
    if (isHorizontalMode()) {
        int selectedWidth= (mInputText->getVisibility()==View::VISIBLE)?std::max(mInputTextSize,inputEdit_Size):inputEdit_Size;//mInputTextSize;
        const int totalTextSize = int ((selectorIndices.size() - 1) * mTextSize + selectedWidth);
        const float totalTextGapWidth = getWidth() - totalTextSize;
        mInputTextGapWidth = (int) (totalTextGapWidth / (textGapCount-1));
        mSelectorElementSize = std::min(getMaxTextSize() + mInputTextGapWidth,getWidth()/textGapCount);
        if( (mInputText->getVisibility()==View::INVISIBLE) || (selectedWidth-mSelectorElementSize<0))
            selectedWidth = mSelectorElementSize;
        mInitialScrollOffset = (int) (mInputTextCenter - mSelectorElementSize * mWheelMiddleItemIndex-(selectedWidth-mSelectorElementSize)/2);
    } else {
        int selectedHeight= std::max(mSelectorElementSize,inputEdit_Size);//:mInputTextSize;
        const int totalTextSize = int ((selectorIndices.size() - 1) * mTextSize + selectedHeight);
        const float totalTextGapHeight= getHeight() - totalTextSize;
        mInputTextGapHeight = (int) (totalTextGapHeight / (textGapCount-1));
        mSelectorElementSize = std::min(getMaxTextSize() + mInputTextGapHeight,getHeight()/textGapCount);
        if( (mInputText->getVisibility()==View::INVISIBLE) || (selectedHeight-mSelectorElementSize<0) )
            selectedHeight = mSelectorElementSize;
        mInitialScrollOffset = (int) (mInputTextCenter - mSelectorElementSize * mWheelMiddleItemIndex-(selectedHeight-mSelectorElementSize)/2);
    }
    LOGV("mInitialScrollOffset=%d %d/%d textsize=%d,%d",mInitialScrollOffset,mSelectorElementSize,mInputText->getHeight(),mInputTextSize,mTextSize);
    mCurrentScrollOffset = mInitialScrollOffset;
    updateInputTextView();
}

void NumberPicker::initializeFadingEdges(){
    if(mTextColor!=mTextColor2)
	    return;
    const int size = isHorizontalMode()?getWidth():getHeight();
    setFadingEdgeLength((size - mTextSize)/2);
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
    std::unordered_map<int,std::string>& cache = mSelectorIndexToStringCache;
    auto itr= cache.find(selectorIndex);

    if (cache.size()&&(itr != cache.end())) return;

    if ((selectorIndex < mMinValue)||(selectorIndex > mMaxValue)||(mMinValue==mMaxValue)) {
        scrollSelectorValue = "";
    } else {
        if (mDisplayedValues.size()){
            const int displayedValueIndex = selectorIndex - mMinValue;
            /*if(cache.size()&&(displayedValueIndex >=mDisplayedValues.size())){
                cache.erase(itr);
                return;
            }*/
            if((displayedValueIndex>=0)&&(displayedValueIndex<mDisplayedValues.size()))
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
    std::string text = mDisplayedValues.empty() ? formatNumber(mValue) : mDisplayedValues[mValue - mMinValue];
    if (!text.empty() ){
        std::string beforeText = mInputText->getText();
        if (text != beforeText){//!text.equals(beforeText.toString())) {
            mInputText->setText(text);
            if (AccessibilityManager::getInstance(mContext).isEnabled()) {
                AccessibilityEvent* event = AccessibilityEvent::obtain(AccessibilityEvent::TYPE_VIEW_TEXT_CHANGED);
                mInputText->onInitializeAccessibilityEvent(*event);
                mInputText->onPopulateAccessibilityEvent(*event);
                event->setFromIndex(0);
                event->setRemovedCount(beforeText.length());
                event->setAddedCount(text.length());
                event->setBeforeText(beforeText);
                event->setSource(this, AccessibilityNodeProviderImpl::VIRTUAL_VIEW_ID_INPUT);
                requestSendAccessibilityEvent(this, *event);
            }
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
    if(mChangeCurrentByOneFromLongPressCommand==nullptr)
        mChangeCurrentByOneFromLongPressCommand = new ChangeCurrentByOneFromLongPressCommand(this);
    else
        mChangeCurrentByOneFromLongPressCommand->removeCallbacks();
    mChangeCurrentByOneFromLongPressCommand->setStep(increment);
    mChangeCurrentByOneFromLongPressCommand->postDelayed(delayMillis);
}

void NumberPicker::removeChangeCurrentByOneFromLongPress(){
    if (mChangeCurrentByOneFromLongPressCommand != nullptr) {
        mChangeCurrentByOneFromLongPressCommand->removeCallbacks();
    }
}

void NumberPicker::removeBeginSoftInputCommand(){
    if(mBeginSoftInputOnLongPressCommand!=nullptr){
        mBeginSoftInputOnLongPressCommand->removeCallbacks();
    }
}

void NumberPicker::postBeginSoftInputOnLongPressCommand(){
    if(mBeginSoftInputOnLongPressCommand==nullptr){
        mBeginSoftInputOnLongPressCommand = new BeginSoftInputOnLongPressCommand(this);
    }else{
        mBeginSoftInputOnLongPressCommand->removeCallbacks();
    }
    mBeginSoftInputOnLongPressCommand->postDelayed(ViewConfiguration::getLongPressTimeout());
}


void NumberPicker::removeAllCallbacks(){
    if (mChangeCurrentByOneFromLongPressCommand != nullptr) {
        mChangeCurrentByOneFromLongPressCommand->removeCallbacks();
    }
    /*if (mSetSelectionCommand != nullptr) {
        mSetSelectionCommand->cancel();
    }*/
    removeBeginSoftInputCommand();
    if (mBeginSoftInputOnLongPressCommand != nullptr) {
        mBeginSoftInputOnLongPressCommand->removeCallbacks();
    }
    mPressedStateHelper->cancel();
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

void NumberPicker::ensureScrollWheelAdjusted() {
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
        LOGV("delta=%d scrollOffset=%d/%d ",delta,mInitialScrollOffset,mCurrentScrollOffset);
    }
    invalidate();
}

NumberPicker::PressedStateHelper::PressedStateHelper(NumberPicker*np)
    :ViewRunnable(np),mNP(np){
}

void NumberPicker::PressedStateHelper::cancel(){
    mMode = 0; 
    mManagedButton = 0;
    removeCallbacks();
    if(mNP->mIncrementVirtualButtonPressed){
        mNP->mIncrementVirtualButtonPressed = false;
        if(!mNP->isHorizontalMode())
            mNP->invalidate(0, mNP->mEndDividerEnd, mNP->mRight, mNP->mBottom-mNP->mEndDividerEnd);
        else
            mNP->invalidate(mNP->mEndDividerEnd,0,mNP->mRight-mNP->mEndDividerEnd,mNP->mBottom);
    }
    if(mNP->mDecrementVirtualButtonPressed){
        mNP->mDecrementVirtualButtonPressed = false;
        if(!mNP->isHorizontalMode())
            mNP->invalidate(0,0,mNP->mRight,mNP->mStartDividerStart);
        else
            mNP->invalidate(0,0,mNP->mStartDividerStart,mNP->mBottom);
    }
}

void NumberPicker::PressedStateHelper::buttonPressDelayed(int button){
    cancel();
    mMode = MODE_PRESS;
    mManagedButton = button;
    
    postDelayed(ViewConfiguration::getTapTimeout());
}

void NumberPicker::PressedStateHelper::buttonTapped(int button){
    cancel();
    mMode = MODE_TAPPED;
    mManagedButton = button;
    post();
}

void NumberPicker::PressedStateHelper::run(){
    switch (mMode) {
    case MODE_PRESS:
        switch (mManagedButton) {
        case BUTTON_INCREMENT:
             mNP->mIncrementVirtualButtonPressed = true;
             if(!mNP->isHorizontalMode())
                 mNP->invalidate(0, mNP->mEndDividerEnd, mNP->mRight, mNP->mBottom-mNP->mEndDividerEnd);
             else
                 mNP->invalidate(mNP->mEndDividerEnd,0,mNP->mRight-mNP->mEndDividerEnd,mNP->mBottom);
             break;
        case BUTTON_DECREMENT:
             mNP->mDecrementVirtualButtonPressed = true;
             if(!mNP->isHorizontalMode())
                 mNP->invalidate(0,0, mNP->mRight, mNP->mStartDividerStart);
             else
                 mNP->invalidate(0,0,mNP->mStartDividerStart,mNP->mBottom);
             break;
        }
        break;
    case MODE_TAPPED:
        switch (mManagedButton) {
        case BUTTON_INCREMENT:
            if (!mNP->mIncrementVirtualButtonPressed) {
                postDelayed(ViewConfiguration::getPressedStateDuration());
            }
            mNP->mIncrementVirtualButtonPressed ^= true;
            if(!mNP->isHorizontalMode())
                mNP->invalidate(0, mNP->mEndDividerEnd, mNP->mRight, mNP->mBottom-mNP->mEndDividerEnd);
            else
                mNP->invalidate(mNP->mEndDividerEnd,0,mNP->mRight-mNP->mEndDividerEnd,mNP->mBottom);
            break;
        case BUTTON_DECREMENT:
            if (!mNP->mDecrementVirtualButtonPressed) {
                postDelayed(ViewConfiguration::getPressedStateDuration());
            }
            mNP->mDecrementVirtualButtonPressed ^= true;
            if(!mNP->isHorizontalMode())
                mNP->invalidate(0, 0, mNP->mRight, mNP->mStartDividerStart);
            else
                mNP->invalidate(0,0,mNP->mStartDividerStart,mNP->mBottom);
        }//endof switch (mManagedButton)
        break;/*endof case MODE_TAPPED*/
    }
}

void NumberPicker::ChangeCurrentByOneFromLongPressCommand::run(){
    ((NumberPicker*)mView)->changeValueByOne(mIncrement);
    postDelayed(((NumberPicker*)mView)->mLongPressUpdateInterval);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
std::string NumberPicker::getAccessibilityClassName()const{
    return "NumberPicker";
}

void NumberPicker::onInitializeAccessibilityEventInternal(AccessibilityEvent& event){
    LinearLayout::onInitializeAccessibilityEventInternal(event);
    event.setClassName("NumberPicker");
    event.setScrollable(true);
    event.setScrollY((mMinValue + mValue) * mSelectorElementSize);
    event.setMaxScrollY((mMaxValue - mMinValue) * mSelectorElementSize);
}

AccessibilityNodeProvider* NumberPicker::getAccessibilityNodeProvider(){
    if (!mHasSelectorWheel) {
        return LinearLayout::getAccessibilityNodeProvider();
    }
    if (mAccessibilityNodeProvider == nullptr) {
        mAccessibilityNodeProvider = new AccessibilityNodeProviderImpl((NumberPicker*)this);
    }
    return mAccessibilityNodeProvider;
}

////////////////////////////////////////////////////////////////////////////////////
NumberPicker::AccessibilityNodeProviderImpl::AccessibilityNodeProviderImpl(NumberPicker*np):mNP(np){
}

AccessibilityNodeInfo* NumberPicker::AccessibilityNodeProviderImpl::createAccessibilityNodeInfo(int virtualViewId) {
    switch (virtualViewId) {
        case View::NO_ID:
            return createAccessibilityNodeInfoForNumberPicker( mNP->mScrollX, mNP->mScrollY,
                    mNP->mScrollX + mNP->getWidth(), mNP->mScrollY + mNP->getHeight());
        case VIRTUAL_VIEW_ID_DECREMENT:
            return createAccessibilityNodeInfoForVirtualButton(VIRTUAL_VIEW_ID_DECREMENT,
                    getVirtualDecrementButtonText(), mNP->mScrollX, mNP->mScrollY, mNP->mScrollX + mNP->getWidth(),
                    mNP->mStartDividerStart + mNP->mDividerThickness);
        case VIRTUAL_VIEW_ID_INPUT:
            return createAccessibiltyNodeInfoForInputText(mNP->mScrollX,
                    mNP->mStartDividerStart + mNP->mDividerThickness, mNP->mScrollX + mNP->getWidth(),
                    mNP->mEndDividerEnd - mNP->mDividerThickness);
        case VIRTUAL_VIEW_ID_INCREMENT:
            return createAccessibilityNodeInfoForVirtualButton(VIRTUAL_VIEW_ID_INCREMENT,
                    getVirtualIncrementButtonText(), mNP->mScrollX, mNP->mEndDividerEnd - mNP->mDividerThickness,
                    mNP->mScrollX + mNP->getWidth(), mNP->mScrollY + mNP->getHeight());
    }
    return AccessibilityNodeProvider::createAccessibilityNodeInfo(virtualViewId);
}

std::vector<AccessibilityNodeInfo*> NumberPicker::AccessibilityNodeProviderImpl::findAccessibilityNodeInfosByText(const std::string& searched,
        int virtualViewId) {
    if (TextUtils::isEmpty(searched)) {
        return std::vector<AccessibilityNodeInfo*>();
    }
    std::string searchedLowerCase = searched;//toLowerCase();
    std::vector<AccessibilityNodeInfo*> result;
    switch (virtualViewId) {
        case View::NO_ID: {
            findAccessibilityNodeInfosByTextInChild(searchedLowerCase, VIRTUAL_VIEW_ID_DECREMENT, result);
            findAccessibilityNodeInfosByTextInChild(searchedLowerCase, VIRTUAL_VIEW_ID_INPUT, result);
            findAccessibilityNodeInfosByTextInChild(searchedLowerCase, VIRTUAL_VIEW_ID_INCREMENT, result);
            return result;
        }
        case VIRTUAL_VIEW_ID_DECREMENT:
        case VIRTUAL_VIEW_ID_INCREMENT:
        case VIRTUAL_VIEW_ID_INPUT:
            findAccessibilityNodeInfosByTextInChild(searchedLowerCase, virtualViewId, result);
            return result;
    }
    return AccessibilityNodeProvider::findAccessibilityNodeInfosByText(searched, virtualViewId);
}

bool NumberPicker::AccessibilityNodeProviderImpl::performAction(int virtualViewId, int action, Bundle* arguments) {
    switch (virtualViewId) {
    case View::NO_ID: {
        switch (action) {
            case AccessibilityNodeInfo::ACTION_ACCESSIBILITY_FOCUS: {
                if (mAccessibilityFocusedView != virtualViewId) {
                    mAccessibilityFocusedView = virtualViewId;
                    mNP->requestAccessibilityFocus();
                    return true;
                }
            } return false;
            case AccessibilityNodeInfo::ACTION_CLEAR_ACCESSIBILITY_FOCUS: {
                if (mAccessibilityFocusedView == virtualViewId) {
                    mAccessibilityFocusedView = UNDEFINED;
                    mNP->clearAccessibilityFocus();
                    return true;
                }
                return false;
            }
            case AccessibilityNodeInfo::ACTION_SCROLL_FORWARD: {
                if (mNP->isEnabled() && (mNP->getWrapSelectorWheel() || mNP->getValue() < mNP->getMaxValue())) {
                    mNP->changeValueByOne(true);
                    return true;
                }
            } return false;
            case AccessibilityNodeInfo::ACTION_SCROLL_BACKWARD: {
                if (mNP->isEnabled() && (mNP->getWrapSelectorWheel() || mNP->getValue() > mNP->getMinValue())) {
                    mNP->changeValueByOne(false);
                    return true;
                }
            } return false;
        }
    } break;
    case VIRTUAL_VIEW_ID_INPUT: {
        switch (action) {
            case AccessibilityNodeInfo::ACTION_FOCUS: {
                if (mNP->isEnabled() && !mNP->mInputText->isFocused()) {
                    return mNP->mInputText->requestFocus();
                }
            } break;
            case AccessibilityNodeInfo::ACTION_CLEAR_FOCUS: {
                if (mNP->isEnabled() && mNP->mInputText->isFocused()) {
                    mNP->mInputText->clearFocus();
                    return true;
                }
                return false;
            }
            case AccessibilityNodeInfo::ACTION_CLICK: {
                if (mNP->isEnabled()) {
                    mNP->performClick();
                    return true;
                }
                return false;
            }
            case AccessibilityNodeInfo::ACTION_LONG_CLICK: {
                if (mNP->isEnabled()) {
                    mNP->performLongClick();
                    return true;
                }
                return false;
            }
            case AccessibilityNodeInfo::ACTION_ACCESSIBILITY_FOCUS: {
                if (mAccessibilityFocusedView != virtualViewId) {
                    mAccessibilityFocusedView = virtualViewId;
                    sendAccessibilityEventForVirtualView(virtualViewId, AccessibilityEvent::TYPE_VIEW_ACCESSIBILITY_FOCUSED);
                    mNP->mInputText->invalidate();
                    return true;
                }
            } return false;
            case  AccessibilityNodeInfo::ACTION_CLEAR_ACCESSIBILITY_FOCUS: {
                if (mAccessibilityFocusedView == virtualViewId) {
                    mAccessibilityFocusedView = UNDEFINED;
                    sendAccessibilityEventForVirtualView(virtualViewId, AccessibilityEvent::TYPE_VIEW_ACCESSIBILITY_FOCUS_CLEARED);
                    mNP->mInputText->invalidate();
                    return true;
                }
            } return false;
            default: {
                return mNP->mInputText->performAccessibilityAction(action, arguments);
            }
        }
    } return false;
    case VIRTUAL_VIEW_ID_INCREMENT: {
        switch (action) {
            case AccessibilityNodeInfo::ACTION_CLICK: {
                if (mNP->isEnabled()) {
                    mNP->changeValueByOne(true);
                    sendAccessibilityEventForVirtualView(virtualViewId,AccessibilityEvent::TYPE_VIEW_CLICKED);
                    return true;
                }
            } return false;
            case AccessibilityNodeInfo::ACTION_ACCESSIBILITY_FOCUS: {
                if (mAccessibilityFocusedView != virtualViewId) {
                    mAccessibilityFocusedView = virtualViewId;
                    sendAccessibilityEventForVirtualView(virtualViewId, AccessibilityEvent::TYPE_VIEW_ACCESSIBILITY_FOCUSED);
                    mNP->invalidate(0, mNP->mEndDividerEnd, mNP->mRight, mNP->mBottom-mNP->mEndDividerEnd);
                    return true;
                }
            } return false;
            case  AccessibilityNodeInfo::ACTION_CLEAR_ACCESSIBILITY_FOCUS: {
                if (mAccessibilityFocusedView == virtualViewId) {
                    mAccessibilityFocusedView = UNDEFINED;
                    sendAccessibilityEventForVirtualView(virtualViewId, AccessibilityEvent::TYPE_VIEW_ACCESSIBILITY_FOCUS_CLEARED);
                    mNP->invalidate(0, mNP->mEndDividerEnd, mNP->mRight, mNP->mBottom-mNP->mEndDividerEnd);
                    return true;
                }
            } return false;
        }
    } return false;
    case VIRTUAL_VIEW_ID_DECREMENT: {
        switch (action) {
            case AccessibilityNodeInfo::ACTION_CLICK: {
                if (mNP->isEnabled()) {
                    bool increment = (virtualViewId == VIRTUAL_VIEW_ID_INCREMENT);
                    mNP->changeValueByOne(increment);
                    sendAccessibilityEventForVirtualView(virtualViewId, AccessibilityEvent::TYPE_VIEW_CLICKED);
                    return true;
                }
            } return false;
            case AccessibilityNodeInfo::ACTION_ACCESSIBILITY_FOCUS: {
                if (mAccessibilityFocusedView != virtualViewId) {
                    mAccessibilityFocusedView = virtualViewId;
                    sendAccessibilityEventForVirtualView(virtualViewId, AccessibilityEvent::TYPE_VIEW_ACCESSIBILITY_FOCUSED);
                    mNP->invalidate(0, 0, mNP->mRight, mNP->mStartDividerStart);
                    return true;
                }
            } return false;
            case  AccessibilityNodeInfo::ACTION_CLEAR_ACCESSIBILITY_FOCUS: {
                if (mAccessibilityFocusedView == virtualViewId) {
                    mAccessibilityFocusedView = UNDEFINED;
                    sendAccessibilityEventForVirtualView(virtualViewId, AccessibilityEvent::TYPE_VIEW_ACCESSIBILITY_FOCUS_CLEARED);
                    mNP->invalidate(0, 0, mNP->mRight, mNP->mStartDividerStart);
                    return true;
                }
            } return false;
        }
    } return false;
    }
    return AccessibilityNodeProvider::performAction(virtualViewId, action, arguments);
}

void NumberPicker::AccessibilityNodeProviderImpl::sendAccessibilityEventForVirtualView(int virtualViewId, int eventType) {
    switch (virtualViewId) {
    case VIRTUAL_VIEW_ID_DECREMENT:
        if (hasVirtualDecrementButton()) {
            sendAccessibilityEventForVirtualButton(virtualViewId, eventType,
                    getVirtualDecrementButtonText());
        }
        break;
    case VIRTUAL_VIEW_ID_INPUT:
        sendAccessibilityEventForVirtualText(eventType);
        break;
    case VIRTUAL_VIEW_ID_INCREMENT:
        if (hasVirtualIncrementButton()) {
            sendAccessibilityEventForVirtualButton(virtualViewId, eventType,
                    getVirtualIncrementButtonText());
        }
        break;
    }
}

void NumberPicker::AccessibilityNodeProviderImpl::sendAccessibilityEventForVirtualText(int eventType) {
    if (AccessibilityManager::getInstance(mNP->mContext).isEnabled()) {
        AccessibilityEvent* event = AccessibilityEvent::obtain(eventType);
        mNP->mInputText->onInitializeAccessibilityEvent(*event);
        mNP->mInputText->onPopulateAccessibilityEvent(*event);
        event->setSource(mNP, VIRTUAL_VIEW_ID_INPUT);
        mNP->requestSendAccessibilityEvent(mNP, *event);
    }
}

void NumberPicker::AccessibilityNodeProviderImpl::sendAccessibilityEventForVirtualButton(int virtualViewId, int eventType,const std::string& text) {
    if (AccessibilityManager::getInstance(mNP->mContext).isEnabled()) {
        AccessibilityEvent* event = AccessibilityEvent::obtain(eventType);
        event->setClassName("Button");
        event->setPackageName("cdroid");
        event->getText().push_back(text);
        event->setEnabled(mNP->isEnabled());
        event->setSource(mNP, virtualViewId);
        mNP->requestSendAccessibilityEvent(mNP, *event);
    }
}

void NumberPicker::AccessibilityNodeProviderImpl::findAccessibilityNodeInfosByTextInChild(const std::string& searchedLowerCase,
        int virtualViewId, std::vector<AccessibilityNodeInfo*>& outResult) {
    switch (virtualViewId) {
        case VIRTUAL_VIEW_ID_DECREMENT: {
            std::string text = getVirtualDecrementButtonText();
            if (!TextUtils::isEmpty(text) && text.find(searchedLowerCase)!=std::string::npos){//.toLowerCase().contains(searchedLowerCase)) {
                outResult.push_back(createAccessibilityNodeInfo(VIRTUAL_VIEW_ID_DECREMENT));
            }
        } return;
        case VIRTUAL_VIEW_ID_INPUT: {
            std::string text = mNP->mInputText->getText();
            if (!TextUtils::isEmpty(text) && text.find(searchedLowerCase)!=std::string::npos){//toLowerCase().contains(searchedLowerCase)) {
                outResult.push_back(createAccessibilityNodeInfo(VIRTUAL_VIEW_ID_INPUT));
                return;
            }
            std::string contentDesc = mNP->mInputText->getText();
            if (!TextUtils::isEmpty(contentDesc) && contentDesc.find(searchedLowerCase)!=std::string::npos){//toLowerCase().contains(searchedLowerCase)) {
                outResult.push_back(createAccessibilityNodeInfo(VIRTUAL_VIEW_ID_INPUT));
                return;
            }
        } break;
        case VIRTUAL_VIEW_ID_INCREMENT: {
            std::string text = getVirtualIncrementButtonText();
            if (!TextUtils::isEmpty(text) && text.find(searchedLowerCase)!=std::string::npos){//toLowerCase().contains(searchedLowerCase)) {
                outResult.push_back(createAccessibilityNodeInfo(VIRTUAL_VIEW_ID_INCREMENT));
            }
        } return;
    }
}

AccessibilityNodeInfo* NumberPicker::AccessibilityNodeProviderImpl::createAccessibiltyNodeInfoForInputText(
        int left, int top, int right, int bottom) {
    AccessibilityNodeInfo* info = mNP->mInputText->createAccessibilityNodeInfo();
    info->setSource(mNP, VIRTUAL_VIEW_ID_INPUT);
    if (mAccessibilityFocusedView != VIRTUAL_VIEW_ID_INPUT) {
        info->addAction(AccessibilityNodeInfo::ACTION_ACCESSIBILITY_FOCUS);
    }
    if (mAccessibilityFocusedView == VIRTUAL_VIEW_ID_INPUT) {
        info->addAction(AccessibilityNodeInfo::ACTION_CLEAR_ACCESSIBILITY_FOCUS);
    }
    Rect boundsInParent;
    boundsInParent.set(left, top, right, bottom);
    info->setVisibleToUser(mNP->isVisibleToUser(&boundsInParent));
    info->setBoundsInParent(boundsInParent);
    Rect boundsInScreen = boundsInParent;
    int locationOnScreen[2];
    mNP->getLocationOnScreen(locationOnScreen);
    boundsInScreen.offset(locationOnScreen[0], locationOnScreen[1]);
    info->setBoundsInScreen(boundsInScreen);
    return info;
}

AccessibilityNodeInfo* NumberPicker::AccessibilityNodeProviderImpl::createAccessibilityNodeInfoForVirtualButton(int virtualViewId,
       const std::string& text, int left, int top, int right, int bottom) {
    AccessibilityNodeInfo* info = AccessibilityNodeInfo::obtain();
    info->setClassName("Button");
    info->setPackageName("cdroid");
    info->setSource(mNP, virtualViewId);
    info->setParent(mNP);
    info->setText(text);
    info->setClickable(true);
    info->setLongClickable(true);
    info->setEnabled(mNP->isEnabled());
    Rect boundsInParent;
    boundsInParent.set(left, top, right, bottom);
    info->setVisibleToUser(mNP->isVisibleToUser(&boundsInParent));
    info->setBoundsInParent(boundsInParent);
    Rect boundsInScreen = boundsInParent;
    int locationOnScreen[2];
    mNP->getLocationOnScreen(locationOnScreen);
    boundsInScreen.offset(locationOnScreen[0], locationOnScreen[1]);
    info->setBoundsInScreen(boundsInScreen);

    if (mAccessibilityFocusedView != virtualViewId) {
        info->addAction(AccessibilityNodeInfo::ACTION_ACCESSIBILITY_FOCUS);
    }
    if (mAccessibilityFocusedView == virtualViewId) {
        info->addAction(AccessibilityNodeInfo::ACTION_CLEAR_ACCESSIBILITY_FOCUS);
    }
    if (mNP->isEnabled()) {
        info->addAction(AccessibilityNodeInfo::ACTION_CLICK);
    }

    return info;
}

AccessibilityNodeInfo* NumberPicker::AccessibilityNodeProviderImpl::createAccessibilityNodeInfoForNumberPicker(int left, int top,int right, int bottom) {
    AccessibilityNodeInfo* info = AccessibilityNodeInfo::obtain();
    info->setClassName("NumberPicker");
    info->setPackageName("cdroid");
    info->setSource(mNP);

    if (hasVirtualDecrementButton()) {
        info->addChild(mNP, VIRTUAL_VIEW_ID_DECREMENT);
    }
    info->addChild(mNP, VIRTUAL_VIEW_ID_INPUT);
    if (hasVirtualIncrementButton()) {
        info->addChild(mNP, VIRTUAL_VIEW_ID_INCREMENT);
    }

    info->setParent((View*) mNP->getParentForAccessibility());
    info->setEnabled(mNP->isEnabled());
    info->setScrollable(true);

    const float applicationScale =1.f;//getContext().getResources().getCompatibilityInfo().applicationScale;

    Rect boundsInParent;
    boundsInParent.set(left, top, right, bottom);
    //boundsInParent.scale(applicationScale);
    info->setBoundsInParent(boundsInParent);

    info->setVisibleToUser(mNP->isVisibleToUser());

    Rect boundsInScreen = boundsInParent;
    int locationOnScreen[2];
    mNP->getLocationOnScreen(locationOnScreen);
    boundsInScreen.offset(locationOnScreen[0], locationOnScreen[1]);
    //boundsInScreen.scale(applicationScale);
    info->setBoundsInScreen(boundsInScreen);

    if (mAccessibilityFocusedView != View::NO_ID) {
        info->addAction(AccessibilityNodeInfo::ACTION_ACCESSIBILITY_FOCUS);
    }
    if (mAccessibilityFocusedView == View::NO_ID) {
        info->addAction(AccessibilityNodeInfo::ACTION_CLEAR_ACCESSIBILITY_FOCUS);
    }
    if (mNP->isEnabled()) {
        if (mNP->getWrapSelectorWheel() || mNP->getValue() < mNP->getMaxValue()) {
            info->addAction(AccessibilityNodeInfo::ACTION_SCROLL_FORWARD);
        }
        if (mNP->getWrapSelectorWheel() || mNP->getValue() > mNP->getMinValue()) {
            info->addAction(AccessibilityNodeInfo::ACTION_SCROLL_BACKWARD);
        }
    }

    return info;
}

bool NumberPicker::AccessibilityNodeProviderImpl::hasVirtualDecrementButton() {
    return mNP->getWrapSelectorWheel() || mNP->getValue() > mNP->getMinValue();
}

bool NumberPicker::AccessibilityNodeProviderImpl::hasVirtualIncrementButton() {
    return mNP->getWrapSelectorWheel() || mNP->getValue() < mNP->getMaxValue();
}

std::string NumberPicker::AccessibilityNodeProviderImpl::getVirtualDecrementButtonText()const{
    int value = mNP->mValue - 1;
    if (mNP->mWrapSelectorWheel) {
        value = mNP->getWrappedSelectorIndex(value);
    }
    if (value >= mNP->mMinValue) {
        return (mNP->mDisplayedValues.empty()) ? mNP->formatNumber(value)
                : mNP->mDisplayedValues[value - mNP->mMinValue];
    }
    return "";
}

std::string NumberPicker::AccessibilityNodeProviderImpl::getVirtualIncrementButtonText()const{
    int value = mNP->mValue + 1;
    if (mNP->mWrapSelectorWheel) {
        value = mNP->getWrappedSelectorIndex(value);
    }
    if (value <= mNP->mMaxValue) {
        return (mNP->mDisplayedValues.empty()) ? mNP->formatNumber(value)
                : mNP->mDisplayedValues[value - mNP->mMinValue];
    }
    return "";
}
}//namespace
