#ifndef __RADIAL_TIMEPICKER_VIEW_H__
#define __RADIAL_TIMEPICKER_VIEW_H__
#include <view/view.h>
#include <widget/explorebytouchhelper.h>
namespace cdroid{

class RadialTimePickerView:public View {
public:
    static constexpr int HOURS = 0;
    static constexpr int MINUTES = 1;
    DECLARE_UIEVENT(void,OnValueSelectedListener,int/*pickerType*/, int/*newValue*/, bool/*autoAdvance*/);
    //void onValueSelected(int pickerType, int newValue, bool autoAdvance);
private:
    static constexpr int HOURS_INNER = 2;
    static constexpr int SELECTOR_CIRCLE = 0;
    static constexpr int SELECTOR_DOT = 1;
    static constexpr int SELECTOR_LINE = 2;

    static constexpr int AM = 0;
    static constexpr int PM = 1;

    static constexpr int HOURS_IN_CIRCLE = 12;
    static constexpr int MINUTES_IN_CIRCLE = 60;
    static constexpr int DEGREES_FOR_ONE_HOUR = 360 / HOURS_IN_CIRCLE;
    static constexpr int DEGREES_FOR_ONE_MINUTE = 360 / MINUTES_IN_CIRCLE;

    static constexpr int ANIM_DURATION_NORMAL = 500;
    static constexpr int ANIM_DURATION_TOUCH = 60;
    //static constexpr int SNAP_PREFER_30S_MAP[361];
    static constexpr int NUM_POSITIONS = 12;

    /** "Something is wrong" color used when a color attribute is missing. */
    static constexpr int MISSING_COLOR = Color::MAGENTA;
private:
    /*final String[] mHours12Texts = new String[12];
    final String[] mOuterHours24Texts = new String[12];
    final String[] mInnerHours24Texts = new String[12];
    final String[] mMinutesTexts = new String[12];

    final Paint[] mPaint = new Paint[2];
    final Paint mPaintCenter = new Paint();
    final Paint[] mPaintSelector = new Paint[3];
    final Paint mPaintBackground = new Paint();*/

    Typeface* mTypeface;

    ColorStateList* mTextColor[3];
    int mTextSize[3];
    int mTextInset[3];

    float mOuterTextX[2][12];
    float mOuterTextY[2][12];

    float mInnerTextX[12];
    float mInnerTextY[12];

    int mSelectionDegrees[2];

    RadialPickerTouchHelper* mTouchHelper;

    Path mSelectorPath = new Path();

    bool mIs24HourMode;
    bool mShowHours;
    bool mIsOnInnerCircle;
    bool mInputEnabled = true;
    bool mChangedDuringTouch = false;

    ObjectAnimator* mHoursToMinutesAnimator;
    float mHoursToMinutes;

    int mSelectorRadius;
    int mSelectorStroke;
    int mSelectorDotRadius;
    int mCenterDotRadius;

    int mSelectorColor;
    int mSelectorDotColor;

    int mXCenter;
    int mYCenter;
    int mCircleRadius;

    int mMinDistForInnerNumber;
    int mMaxDistForOuterNumber;
    int mHalfwayDist;
    int mAmOrPm;

    String[] mOuterTextHours;
    String[] mInnerTextHours;
    String[] mMinutesText;

    float mDisabledAlpha;

    OnValueSelectedListener mListener;
private:
    static int snapPrefer30s(int degrees);
    static int snapOnly30s(int degrees, int forceHigherOrLower);
    void setCurrentHourInternal(int hour, bool callback, bool autoAdvance);
    int getMinuteForDegrees(int degrees);
    int getDegreesForMinute(int minute);
    void initHoursAndMinutesText();
    void initData();
    void showPicker(bool hours, bool animate);
    void animatePicker(bool hoursToMinutes, long duration);
    void drawCircleBackground(Canvas& canvas);
    void drawHours(Canvas& canvas, Path selectorPath, float alphaMod);
    void drawHoursClipped(Canvas& canvas, int hoursAlpha, bool showActivated);
    void drawMinutes(Canvas& canvas, Path selectorPath, float alphaMod);
    void drawMinutesClipped(Canvas& canvas, int minutesAlpha, bool showActivated);
    void drawCenter(Canvas& canvas, float alphaMod);
    int getMultipliedAlpha(int argb, int alpha)const;
    void drawSelector(Canvas& canvas, Path selectorPath);
    void calculatePositionsHours();
    void calculatePositionsMinutes();
    static void calculatePositions(Paint paint, float radius, float xCenter, float yCenter,
    float textSize, float[] x, float[] y);
    void drawTextElements(Canvas canvas, float textSize, Typeface typeface,
    ColorStateList textColor, String[] texts, float[] textX, float[] textY, Paint paint,
    int alpha, bool showActivated, int activatedDegrees, bool activatedOnly);
    int getDegreesFromXY(float x, float y, bool constrainOutside);
    bool getInnerCircleFromXY(float x, float y);
    bool handleTouchInput(float x, float y, bool forceSelection, bool autoAdvance);
protected:
    void onLayout(bool changed, int left, int top, int right, int bottom)override;
    void onDraw(Canvas& canvas)override;
public:
    RadialTimePickerView(Context* context,const AttributeSet& attrs);

    void applyAttributes(const AttributeSet& attrs);
    void initialize(int hour, int minute, bool is24HourMode);

    void setCurrentItemShowing(int item, bool animate);
    int getCurrentItemShowing()const;

    void setOnValueSelectedListener(OnValueSelectedListener listener);

    void setCurrentHour(int hour);
    int getCurrentHour()const;
    int getCurrentMinute()const;

    bool setAmOrPm(int amOrPm);
    int getAmOrPm() const;

    void showHours(bool animate);
    void showMinutes(bool animate);

    bool onTouchEvent(MotionEvent& event)override;
    bool dispatchHoverEvent(MotionEvent& event) override;

    void setInputEnabled(bool inputEnabled);

    PointerIcon* onResolvePointerIcon(MotionEvent& event, int pointerIndex) override;
};

class RadialTimePickerView::RadialPickerTouchHelper:public ExploreByTouchHelper {
private:
    Rect mTempRect;

    int TYPE_HOUR = 1;
    int TYPE_MINUTE = 2;

    int SHIFT_TYPE = 0;
    int MASK_TYPE = 0xF;

    int SHIFT_VALUE = 8;
    int MASK_VALUE = 0xFF;

    /** Increment in which virtual views are exposed for minutes. */
    int MINUTE_INCREMENT = 5;
private:
    void adjustPicker(int step);
    int getCircularDiff(int first, int second, int max);
    int getVirtualViewIdAfter(int type, int value)
    int hour12To24(int hour12, int amOrPm);
    int hour24To12(int hour24);
    void getBoundsForVirtualView(int virtualViewId, Rect bounds);

    CharSequence getVirtualViewDescription(int type, int value);

    bool isVirtualViewSelected(int type, int value);

    int makeId(int type, int value)const;
    int getTypeFromId(int id)const;
    int getValueFromId(int id)const;
protected:
    int getVirtualViewAt(float x, float y)override;
    void getVisibleVirtualViews(IntArray virtualViewIds)override;
    void onPopulateEventForVirtualView(int virtualViewId, AccessibilityEvent& event)override;
    void onPopulateNodeForVirtualView(int virtualViewId, AccessibilityNodeInfo& node)override;
    bool onPerformActionForVirtualView(int virtualViewId, int action,Bundle arguments)override;
public:
    RadialPickerTouchHelper();
    void onInitializeAccessibilityNodeInfo(View host, AccessibilityNodeInfo info)override;
    bool performAccessibilityAction(View host, int action, Bundle arguments)override;
};
}/*endof namespace*/
#endif/*__RADIAL_TIMEPICKER_VIEW_H__*/
