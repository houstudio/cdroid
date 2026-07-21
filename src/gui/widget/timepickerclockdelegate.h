#ifndef __TIMEPICKER_CLOCK_DELEGATE_H__
#define __TIMEPICKER_CLOCK_DELEGATE_H__
#include <core/calendar.h>
#include <widget/imagebutton.h>
#include <widget/radiobutton.h>
#include <widget/timepicker.h>
#include <widget/numberictextview.h>
#include <widget/radialtimepickerview.h>
#include <widget/textinputtimepickerview.h>
namespace cdroid{
class TimePickerClockDelegate:public TimePicker::AbstractTimePickerDelegate {
private:
    static constexpr long DELAY_COMMIT_MILLIS = 2000;

    static constexpr int FROM_EXTERNAL_API = 0;
    static constexpr int FROM_RADIAL_PICKER = 1;
    static constexpr int FROM_INPUT_PICKER = 2;

    // Index used by RadialPickerLayout
    static constexpr int HOUR_INDEX  = RadialTimePickerView::HOURS;
    static constexpr int MINUTE_INDEX= RadialTimePickerView::MINUTES;

    //static final int[] ATTRS_TEXT_COLOR = new int[] {R.attr.textColor};
    //static final int[] ATTRS_DISABLED_ALPHA = new int[] {R.attr.disabledAlpha};

    static constexpr int AM = 0;
    static constexpr int PM = 1;

    static constexpr int HOURS_IN_HALF_DAY = 12;

    NumericTextView* mHourView;
    NumericTextView* mMinuteView;
    View* mAmPmLayout;
    RadioButton* mAmLabel;
    RadioButton* mPmLabel;
    RadialTimePickerView* mRadialTimePickerView;
    TextView* mSeparatorView;

    bool mRadialPickerModeEnabled = true;
    ImageButton* mRadialTimePickerModeButton;
    std::string mRadialTimePickerModeEnabledDescription;
    std::string mTextInputPickerModeEnabledDescription;
    View* mRadialTimePickerHeader;
    View* mTextInputPickerHeader;

    TextInputTimePickerView* mTextInputPickerView;

    Calendar mTempCalendar;

    // Accessibility strings.
    std::string mSelectHours;
    std::string mSelectMinutes;

    int mCurrentHour;
    int mCurrentMinute;
    bool mIsEnabled = true;
    bool mAllowAutoAdvance;
    bool mIs24Hour;

    // The portrait layout puts AM/PM at the right by default.
    bool mIsAmPmAtLeft = false;
    // The landscape layouts put AM/PM at the bottom by default.
    bool mIsAmPmAtTop = false;

    // Localization data.
    bool mHourFormatShowLeadingZero;
    bool mHourFormatStartsAtZero;
    Runnable mCommitHour;
    Runnable mCommitMinute;

    // Listeners (ported from Java anonymous inner classes).
    View::OnClickListener mClickListener;
    View::OnFocusChangeListener mFocusListener;
    RadialTimePickerView::OnValueSelectedListener mOnValueSelectedListener;
    TextInputTimePickerView::OnValueTypedListener mOnValueTypedListener;
    NumericTextView::OnValueChangedListener mDigitEnteredListener;
private:
    void toggleRadialPickerMode();
    static void ensureMinimumTextWidth(TextView* v);
    void updateHourFormat();
    ColorStateList* applyLegacyColorFixes(ColorStateList* color);
    int  multiplyAlphaComponent(int color, float alphaMod);
    void initialize(int hourOfDay, int minute, bool is24HourView, int index);
    void updateUI(int index);
    void updateTextInputPicker();
    void updateRadialPicker(int index);
    void updateHeaderAmPm();
    void setAmPmStart(bool isAmPmAtStart);
    void setHourInternal(int hour, int source, bool announce,bool notify);
    void setMinuteInternal(int minute,int source, bool notify);
    int getCurrentItemShowing();
    void onTimeChanged();
    void tryVibrate();
    void updateAmPmLabelStates(int amOrPm);
    int getLocalizedHour(int hourOfDay);
    void updateHeaderHour(int hourOfDay, bool announce);
    void updateHeaderMinute(int minuteOfHour, bool announce);
    void updateHeaderSeparator();
    static std::string getHourMinSeparatorFromPattern(const std::string& dateTimePattern);
    static int lastIndexOfAny(const std::string& str, const std::string& any);
    void setCurrentItemShowing(int index, bool animateCircle);
    void setAmOrPm(int amOrPm);
    void onViewClick(View& v);
    void onViewFocusChange(View& v, bool focused);
public:
    TimePickerClockDelegate(TimePicker* delegator, Context* context,const AttributeSet& attrs);


    bool validateInput() override;


    static std::string obtainVerbatim(const std::string& text);


    void setDate(int hour, int minute) override;

    void setHour(int hour) override;
    int getHour() override;

    void setMinute(int minute) override;
    int getMinute() override;

    void setIs24Hour(bool is24Hour);

    bool is24Hour() override;

    void setEnabled(bool enabled) override;
    bool isEnabled() const;

    int getBaseline() override;

    Parcelable* onSaveInstanceState(Parcelable& superState) override;
    void onRestoreInstanceState(Parcelable& state) override;
    bool dispatchPopulateAccessibilityEvent(AccessibilityEvent& event) override;

    void onPopulateAccessibilityEvent(AccessibilityEvent& event)override;

    View* getHourView() override;
    View* getMinuteView() override;
    View* getAmView() override;
    View* getPmView() override;
};

// Ported from Java private static class NearestTouchDelegate implements View.OnTouchListener.
// Routes a touch on a transparent container (e.g. the AM/PM layout) to the nearest child,
// so tapping between two radio buttons hits the closer one.
class TimePickerClockDelegateNearestTouchDelegate:public View::OnTouchListener {
private:
    View* mInitialTouchTarget = nullptr;
    static View* findNearestChild(ViewGroup* v, int x, int y);
public:
    bool operator()(View& view, MotionEvent& motionEvent) override;
};
}/*endof namespace*/
#endif/*__TIMEPICKER_CLOCK_DELEGATE_H__*/
