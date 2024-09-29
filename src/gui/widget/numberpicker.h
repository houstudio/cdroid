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
    static constexpr int ASCENDING = 0;
    static constexpr int DESCENDING = 1;
    static constexpr int SIDE_LINES = 0;
    static constexpr int UNDERLINE = 1;
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
    static constexpr int DEFAULT_WHEEL_ITEM_COUNT = 3;
    static constexpr int DEFAULT_LONG_PRESS_UPDATE_INTERVAL =300;
    static constexpr int SELECTOR_MAX_FLING_VELOCITY_ADJUSTMENT =8;
    static constexpr int SELECTOR_ADJUSTMENT_DURATION_MILLIS =800;
    static constexpr int SNAP_SCROLL_DURATION =300;
    static constexpr float TOP_AND_BOTTOM_FADING_EDGE_STRENGTH =0.9f;
    static constexpr int UNSCALED_DEFAULT_SELECTION_DIVIDER_HEIGHT =2;
    static constexpr int UNSCALED_DEFAULT_SELECTION_DIVIDERS_DISTANCE =48;
    static constexpr int SIZE_UNSPECIFIED =-1;
    static constexpr int DEFAULT_DIVIDER_COLOR = 0x0;
    static constexpr int DEFAULT_MAX_HEIGHT= 180;
    static constexpr int DEFAULT_MIN_WIDTH = 64;
    static constexpr float DEFAULT_FADING_EDGE_STRENGTH = .9f;
    ImageButton* mIncrementButton;
    ImageButton* mDecrementButton;
    EditText* mInputText;
    Runnable mChangeCurrentByOneFromLongPressCommand;
    Runnable mBeginSoftInputOnLongPressCommand;
    float mInputTextCenter;
    int mSelectionDividersDistance;
    int mMinHeight;
    int mMaxHeight;
    int mMinWidth;
    int mMaxWidth;
    int mTextSize,mTextSize2;
    int mInputTextSize;
    int mTextColor,mTextColor2;
    int mInputTextColor;
    int mInputTextGapWidth;
    int mInputTextGapHeight;
    int mTextAlign;
    Typeface *mTypeface;
    Typeface *mSelectedTypeface; 
    std::vector<std::string> mDisplayedValues;
    std::vector<Drawable*> mDisplayedDrawables;
    int mDisplayedDrawableCount;
    int mDisplayedDrawableSize;
    int mMinValue;
    int mMaxValue;
    int mValue;
    OnClickListener mOnClickListener;
    OnValueChangeListener mOnValueChangeListener;
    Formatter mFormatter;
    OnScrollListener mOnScrollListener;
    std::vector<int>mSelectorIndices;
    long mLongPressUpdateInterval;
    std::map<int,std::string> mSelectorIndexToStringCache;
    int mSelectorElementSize;
    int mInitialScrollOffset=INT_MIN;
    int mCurrentScrollOffset;
    Scroller* mFlingScroller;
    Scroller* mAdjustScroller;
    int mPreviousScrollerX;
    int mPreviousScrollerY;
    float mLastDownEventX;
    float mLastDownEventY;
    long mLastDownEventTime;
    float mLastDownOrMoveEventX;
    float mLastDownOrMoveEventY;
    VelocityTracker* mVelocityTracker;
    int mTouchSlop;
    int mMinimumFlingVelocity;
    int mMaximumFlingVelocity;
    bool mComputeMaxWidth;
    bool mWrapSelectorWheel;
    bool mScrollerEnabled;
    bool mIncrementVirtualButtonPressed;
    bool mDecrementVirtualButtonPressed;
    bool mHideWheelUntilFocused; 
    bool mWrapSelectorWheelPreferred;
    bool mUpdateInputTextInFling;
    int mWheelItemCount;
    int mRealWheelItemCount;
    int mWheelMiddleItemIndex;
    Drawable* mDividerDrawable;
    Drawable* mItemBackground;
    Drawable* mVirtualButtonPressedDrawable;
    int mDividerColor;
    int mDividerType;
    int mDividerDistance;
    int mDividerLength;
    int mDividerThickness;
    int mTopDividerTop;
    int mBottomDividerBottom;
    int mLeftDividerLeft;
    int mRightDividerRight;
    int mOrder;
    int mItemSpacing;
    int mSelectionDividerHeight;
    int mScrollState=OnScrollListener::SCROLL_STATE_IDLE;
    int mTopSelectionDividerTop;
    int mBottomSelectionDividerBottom;
    int mLastHoveredChildVirtualViewId;
    int mLastHandledDownDpadKeyCode;
    Cairo::RefPtr<Cairo::LinearGradient>mPat;
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
    float getMaxTextSize()const;
    void setWidthAndHeight();
    bool isHorizontalMode()const;
    void drawHorizontalDividers(Canvas& canvas);
    void drawVerticalDividers(Canvas& canvas);
    void drawText(const std::string& text, float x, float y,Canvas& canvas);
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
    void postChangeCurrentByOneFromLongPress(bool increment);
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
    void ensureScrollWheelAdjusted();
protected:
    void onLayout(bool changed, int left, int top, int right, int bottom)override;
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec)override;
    int computeHorizontalScrollOffset()override;
    int computeHorizontalScrollRange()override;
    int computeHorizontalScrollExtent()override;
    int computeVerticalScrollOffset()override;
    int computeVerticalScrollRange()override;
    int computeVerticalScrollExtent()override;
    void drawableStateChanged();
    void onDraw(Canvas&canvas)override;
public:
    NumberPicker(int w,int h);
    NumberPicker(Context* context,const AttributeSet& attrs);
    ~NumberPicker();
    void setOrientation(int orientation)override;
    void setWheelItemCount(int count);
    void setSelector(int count){setWheelItemCount(count);}
    bool isAscendingOrder()const;
    int  getSelectedTextColor()const;
    void setSelectedTextColor(int);
    int  getSelectedTextSize()const;
    void setSelectedTextSize(int);
    bool onInterceptTouchEvent(MotionEvent& event)override;
    bool onTouchEvent(MotionEvent& event)override;
    bool dispatchTouchEvent(MotionEvent& event)override;
    bool dispatchKeyEvent(KeyEvent& event)override;
    void computeScroll()override;
    View& setEnabled(bool enabled)override;
    void scrollBy(int x, int y)override;
    void setOnClickListener(OnClickListener onClickListener);
    void setOnValueChangedListener(OnValueChangeListener onValueChangedListener);
    void setOnScrollListener(const OnScrollListener& onScrollListener);
    void setFormatter(Formatter formatter);

    EditText*getSelectedText()const;
    Drawable* getDivider()const;
    void setDivider(Drawable*d);
    void setSelectionDivider(Drawable*d);
    Drawable* getSelectionDivider()const;

    int  getDividerColor()const;
    void setDividerColor(int);
    int  getDividerType()const;
    void setDividerType(int);
    int  getDividerThickness()const;
    void setDividerThickness(int thickness);
    int  getOrder()const;
    void setOrder(int);
    void setValue(int value);
    int  getValue()const;
    void setMinValue(int value);
    int  getMinValue()const;
    void setMaxValue(int maxValue);
    int  getMaxValue()const;
    void setMinHeight(int h);
    void setMaxHeight(int h);
    int  getMinHeight()const;
    int  getMaxHeight()const;
    std::vector<std::string> getDisplayedValues()const;
    void setDisplayedValues(const std::vector<std::string>&);
    bool performClick()override;
    bool performLongClick()override;
    bool getWrapSelectorWheel()const;
    void setWrapSelectorWheel(bool);
    void setOnLongPressUpdateInterval(long);

    void jumpDrawablesToCurrentState();
    void onResolveDrawables(int layoutDirection)override;
    void setTextColor(int color);
    virtual void setTextColor(int color,int color2);
    int  getTextColor()const;
    void setTextSize(int);
    virtual void setTextSize(int size,int size2);
    int  getTextSize()const;
    void setSelectedTypeface(Typeface* typeface);
    void setSelectedTypeface(const std::string& string, int style);
    Typeface* getSelectedTypeface()const;
    void setTypeface(Typeface* typeface);
    void setTypeface(const std::string& string, int style);
    Typeface* getTypeface()const;
    void smoothScrollToPosition(int position);
    void smoothScroll(bool increment, int steps);
};
    
}//namespace
#endif
