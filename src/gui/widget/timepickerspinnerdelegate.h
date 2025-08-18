#ifndef __TIMEPICKER_SPINNER_DELEGATE_H__
#define __TIMEPICKER_SPINNER_DELEGATE_H__
#include <core/calendar.h>
#include <widget/button.h>
#include <widget/edittext.h>
#include <widget/timepicker.h>
#include <widget/numberpicker.h>
namespace cdroid{
class TimePickerSpinnerDelegate:public TimePicker::AbstractTimePickerDelegate {
private:
    static constexpr bool DEFAULT_ENABLED_STATE = true;
    static constexpr int HOURS_IN_HALF_DAY = 12;

    NumberPicker* mHourSpinner;
    NumberPicker* mMinuteSpinner;
    NumberPicker* mAmPmSpinner;
    EditText* mHourSpinnerInput;
    EditText* mMinuteSpinnerInput;
    EditText* mAmPmSpinnerInput;
    TextView* mDivider;

    // Note that the legacy implementation of the TimePicker is
    // using a button for toggling between AM/PM while the new
    // version uses a NumberPicker spinner. Therefore the code
    // accommodates these two cases to be backwards compatible.
    Button* mAmPmButton;

    std::vector<std::string> mAmPmStrings;

    Calendar mTempCalendar;

    bool mIsEnabled = DEFAULT_ENABLED_STATE;
    bool mHourWithTwoDigit;
    char mHourFormat;

    bool mIs24HourView;
    bool mIsAm;
private:
    void getHourFormatData();
    bool isAmPmAtStart();
    void setCurrentHour(int currentHour, bool notifyTimeChanged);
    void setDividerText();
    void setCurrentMinute(int minute, bool notifyTimeChanged);
    void updateInputState();
    void updateAmPmControl();
    void onTimeChanged();
    void updateHourControl();
    void updateMinuteControl();
    void setContentDescriptions();
    void trySetContentDescription(View* root, int viewId, int contDescResId);
public:
    TimePickerSpinnerDelegate(TimePicker* delegator, Context* context,const AttributeSet& attrs);
    bool validateInput() override;

    void setDate(int hour, int minute) override;

    void setHour(int hour) override;
    int getHour() override;
    void setMinute(int minute) override;
    int getMinute() override;

    void setIs24Hour(bool is24Hour);
    bool is24Hour()override;

    void setEnabled(bool enabled) override;
    bool isEnabled() const override;
    int getBaseline() override;

    Parcelable* onSaveInstanceState(Parcelable& superState) override;
    void onRestoreInstanceState(Parcelable& state) override;

    bool dispatchPopulateAccessibilityEvent(AccessibilityEvent& event)override;

    void onPopulateAccessibilityEvent(AccessibilityEvent& event)override;

    View* getHourView() override;
    View* getMinuteView() override;
    View* getAmView() override;
    View* getPmView()override;
};
}/*endof namespace*/
#endif/*__TIMEPICKER_SPINNER_DELEGATE_H__*/
