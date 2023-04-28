#ifndef __NUMBERPICKER_H__
#define __NUMBERPICKER_H__
#include <widget/linearlayout.h>
#include <widget/imagebutton.h>
#include <widget/edittext.h>
#include <widget/scroller.h>
#include <core/sparsearray.h>

namespace cdroid{

class NumberPicker:public LinearLayout{
public:
    DECLARE_UIEVENT(void,OnValueChangeListener,NumberPicker&,int,int);
    DECLARE_UIEVENT(const std::string,Formatter,int);
    struct OnScrollListener {
        static constexpr int SCROLL_STATE_IDLE =0;
        static constexpr int SCROLL_STATE_TOUCH_SCROLL =1;
        static constexpr int SCROLL_STATE_FLING =2;
        //virtual void onScrollStateChange(NumberPicker& view,int scrollState)=0;
        std::function<void(NumberPicker&,int)>onScrollStateChange;
    };
private:
    static constexpr int MODE_PRESS =1;
    static constexpr int MODE_TAPPED=2;
    static constexpr int DEFAULT_SELECTOR_WHEEL_ITEM_COUNT =3;
    static constexpr int DEFAULT_LONG_PRESS_UPDATE_INTERVAL =300;
    static constexpr int SELECTOR_MAX_FLING_VELOCITY_ADJUSTMENT =8;
    static constexpr int SELECTOR_ADJUSTMENT_DURATION_MILLIS =800;
    static constexpr int SNAP_SCROLL_DURATION =300;
    static constexpr float TOP_AND_BOTTOM_FADING_EDGE_STRENGTH =0.9f;
    static constexpr int UNSCALED_DEFAULT_SELECTION_DIVIDER_HEIGHT =2;
    static constexpr int UNSCALED_DEFAULT_SELECTION_DIVIDERS_DISTANCE =48;
    static constexpr int SIZE_UNSPECIFIED =-1;
    ImageButton* mIncrementButton;
    ImageButton* mDecrementButton;
    EditText* mInputText;
    Runnable mChangeCurrentByOneFromLongPressCommand;
    Runnable mBeginSoftInputOnLongPressCommand;
    int mSelectionDividersDistance;
    int mMinHeight;
    int mMaxHeight;
    int mMinWidth;
    int mMaxWidth;
    bool mComputeMaxWidth;
    int mTextSize;
    int mTextSize2;
    int mTextColor;
    int mTextColor2;
    int mSelectorTextGapHeight;
    std::vector<std::string> mDisplayedValues;
    int mMinValue;
    int mMaxValue;
    int mValue;
    OnValueChangeListener mOnValueChangeListener;
    Formatter mFormatter;
    OnScrollListener mOnScrollListener;
    int mMiddleItemIndex;
    int mMaxSelectorIndices;
    std::vector<int>mSelectorIndices;
    long mLongPressUpdateInterval;
    std::map<int,std::string> mSelectorIndexToStringCache;
    int mSelectorElementHeight;
    int mSelectorElementWidth;
    int mInitialScrollOffset=INT_MIN;
    int mCurrentScrollOffset;
    Scroller* mFlingScroller;
    Scroller* mAdjustScroller;
    int mPreviousScrollerY;
    float mLastDownEventY;
    long mLastDownEventTime;
    float mLastDownOrMoveEventY;
    VelocityTracker* mVelocityTracker;
    int mTouchSlop;
    int mMinimumFlingVelocity;
    int mMaximumFlingVelocity;
    bool mWrapSelectorWheel;
    int mSolidColor;
    bool mHasSelectorWheel;
    Drawable* mSelectionDivider;
    Drawable* mVirtualButtonPressedDrawable;
    int mSelectionDividerHeight;
    int mScrollState=OnScrollListener::SCROLL_STATE_IDLE;
    bool mIgnoreMoveEvents;
    bool mPerformClickOnTap;
    int mTopSelectionDividerTop;
    int mBottomSelectionDividerBottom;
    int mLastHoveredChildVirtualViewId;
    bool mIncrementVirtualButtonPressed;
    bool mDecrementVirtualButtonPressed;
    int mLastHandledDownDpadKeyCode = -1;
    bool mHideWheelUntilFocused; 
    bool mWrapSelectorWheelPreferred=true;


    //PressedStateHelper's members
    Runnable mPressedStateHelpers;
    int mPSHManagedButton;
    int mPSHMode;
    void pshCancel();
    void pshButtonPressDelayed(int);
    void pshButtonTapped(int);
    void pshRun();
private:
    void initView();
    int makeMeasureSpec(int measureSpec, int maxSize);
    int resolveSizeAndStateRespectingMinSize(int minSize, int measuredSize, int measureSpec);
    void initializeSelectorWheelIndices();
    void setValueInternal(int current, bool notifyChange);
    void changeValueByOne(bool increment);
    void initializeSelectorWheel();
    void initializeFadingEdges();
    void onScrollerFinished(Scroller* scroller);
    bool moveToFinalScrollerPosition(Scroller* scroller);
    void onScrollStateChange(int scrollState);
    void fling(int velocityY);
    int getWrappedSelectorIndex(int selectorIndex);
    void incrementSelectorIndices(std::vector<int>&);
    void decrementSelectorIndices(std::vector<int>&);
    void ensureCachedScrollSelectorValue(int selectorIndex);
    std::string formatNumber(int value);
    void validateInputTextView(View* v);
    bool updateInputTextView();
    void notifyChange(int previous, int current);
    void postChangeCurrentByOneFromLongPress(bool increment, long delayMillis);
    void removeChangeCurrentByOneFromLongPress();
    void removeBeginSoftInputCommand();
    void postBeginSoftInputOnLongPressCommand();
    void removeAllCallbacks();
    int getSelectedPos(const std::string& value);
    void showSoftInput();
    void hideSoftInput();
    void tryComputeMaxWidth();
    void updateWrapSelectorWheel();
    bool ensureScrollWheelAdjusted();
    void onIncDecClick(View&v);
    bool onIncDecLongClick(View&v);
protected:
    void onLayout(bool changed, int left, int top, int right, int bottom)override;
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec)override;
    int computeVerticalScrollOffset();
    int computeVerticalScrollRange();
    int computeVerticalScrollExtent();
    void drawableStateChanged();
    void drawVertical(Canvas&);
    void drawHorizontal(Canvas&);
    void onDraw(Canvas&canvas)override;
public:
    NumberPicker(int w,int h);
    NumberPicker(Context* context,const AttributeSet& attrs);
    ~NumberPicker();
    bool onInterceptTouchEvent(MotionEvent& event)override;
    bool onTouchEvent(MotionEvent& event)override;
    bool dispatchTouchEvent(MotionEvent& event)override;
    bool dispatchKeyEvent(KeyEvent& event)override;
    void computeScroll()override;
    View& setEnabled(bool enabled)override;
    void scrollBy(int x, int y)override;
    int getSolidColor()const;
    void setOnValueChangedListener(OnValueChangeListener onValueChangedListener);
    void setOnScrollListener(const OnScrollListener& onScrollListener);
    void setFormatter(Formatter formatter);
    void setValue(int value);
    int getValue()const;
    void setMinValue(int value);
    int getMinValue()const;
    void setMaxValue(int maxValue);
    int getMaxValue()const;
    void setMinHeight(int h);
    void setMaxHeight(int h);
    int getMinHeight()const;
    int getMaxHeight()const;
    std::vector<std::string> getDisplayedValues()const;
    void setDisplayedValues(const std::vector<std::string>&);
    bool performClick()override;
    bool performLongClick()override;
    bool getWrapSelectorWheel()const;
    void setWrapSelectorWheel(bool);
    void setOnLongPressUpdateInterval(long);
    void setSelector(int items);

    void jumpDrawablesToCurrentState();
    void onResolveDrawables(int layoutDirection)override;
    void setTextColor(int color,int color2=0);
    int  getTextColor()const;
    void setTextSize(int size,int size2=0);
    int  getTextSize()const;
};
    
}//namespace
#endif
