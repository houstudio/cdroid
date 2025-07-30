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

#ifndef __TIME_PICKER_H__
#define __TIME_PICKER_H__
#include <widget/framelayout.h>
namespace cdroid{
class TimePicker:public FrameLayout {
public:
    static constexpr int MODE_SPINNER = 1;
    static constexpr int MODE_CLOCK = 2;

    class TimePickerDelegate;
    class AbstractTimePickerDelegate;
    DECLARE_UIEVENT(void,OnTimeChangedListener,TimePicker&,int,int);
private:
    TimePickerDelegate* mDelegate;
    int mMode;
protected:
    Parcelable* onSaveInstanceState() override;
    void onRestoreInstanceState(Parcelable& state) override;
public:
    TimePicker(Context* context,const AttributeSet& attrs);
    ~TimePicker()override;
    int getMode() const{
        return mMode;
    }

    void setHour(int hour) {
        mDelegate->setHour(MathUtils.constrain(hour, 0, 23));
    }

    int getHour() {
        return mDelegate->getHour();
    }

    void setMinute(int minute) {
        mDelegate->setMinute(MathUtils.constrain(minute, 0, 59));
    }

    int getMinute() {
        return mDelegate->getMinute();
    }

    void setIs24HourView(bool is24HourView) {
        if (is24HourView == null) {
            return;
        }
        mDelegate->setIs24Hour(is24HourView);
    }

    bool is24HourView() {
        return mDelegate->is24Hour();
    }

    void setOnTimeChangedListener(const OnTimeChangedListener& onTimeChangedListener) {
        mDelegate->setOnTimeChangedListener(onTimeChangedListener);
    }

    void setEnabled(bool enabled) override{
        super.setEnabled(enabled);
        mDelegate->setEnabled(enabled);
    }

    bool isEnabled() override{
        return mDelegate->isEnabled();
    }

    int getBaseline() override{
        return mDelegate->getBaseline();
    }

    bool validateInput() {
        return mDelegate->validateInput();
    }

    std::string getAccessibilityClassName() override{
        return TimePicker.class.getName();
    }

    bool dispatchPopulateAccessibilityEventInternal(AccessibilityEvent& event) override{
        return mDelegate->dispatchPopulateAccessibilityEvent(event);
    }

    View* getHourView() {
        return mDelegate->getHourView();
    }

    View* getMinuteView() {
        return mDelegate->getMinuteView();
    }

    View* getAmView() {
        return mDelegate->getAmView();
    }

    View* getPmView() {
        return mDelegate->getPmView();
    }

    static String[] getAmPmStrings(Context context) {
        final Locale locale = context.getResources().getConfiguration().locale;
        DateFormatSymbols dfs = DateFormat.getIcuDateFormatSymbols(locale);
        String[] amPm = dfs.getAmPmStrings();
        String[] narrowAmPm = dfs.getAmpmNarrowStrings();

        final String[] result = new String[2];
        result[0] = amPm[0].length() > 4 ? narrowAmPm[0] : amPm[0];
        result[1] = amPm[1].length() > 4 ? narrowAmPm[1] : amPm[1];
        return result;
    }

    /*void dispatchProvideAutofillStructure(ViewStructure structure, int flags) override;
    void autofill(AutofillValue value) override;
    int getAutofillType() override;
    AutofillValue getAutofillValue() override;*/
};

class TimerPicker::TimePickerDelegate {
public:
    virtual ~TimePickerDelegate()=default;
    virtual void setHour(int hour)=0;
    virtual int getHour()=0;

    virtual void setMinute(int minute)=0;
    virtual int getMinute()=0;

    virtual void setDate( int hour,int minute)=0;

    //void autofill(AutofillValue value)=0;
    //AutofillValue getAutofillValue();

    virtual void setIs24Hour(bool is24Hour)=0;
    virtual bool is24Hour()=0;

    virtual bool validateInput()=0;

    virtual void setOnTimeChangedListener(const OnTimeChangedListener& onTimeChangedListener)=0;
    //virtual void setAutoFillChangeListener(const OnTimeChangedListener& autoFillChangeListener)=0;

    virtual void setEnabled(bool enabled)=0;
    virtual bool isEnabled()const =0;

    virtual int getBaseline()=0;

    virtual Parcelable* onSaveInstanceState(Parcelable& superState)=0;
    virtual void onRestoreInstanceState(Parcelable& state)=0;

    virtual bool dispatchPopulateAccessibilityEvent(AccessibilityEvent& event)=0;
    virtual void onPopulateAccessibilityEvent(AccessibilityEvent& event)=0;

    virtual View* getHourView()=0;
    virtual View* getMinuteView()=0;
    virtual View* getAmView()=0;
    virtual View* getPmView()=0;
};

class TimerPicker::AbstractTimePickerDelegate:public TimerPicker::TimePickerDelegate {
protected:
    TimePicker* mDelegator;
    Context* mContext;
    Locale mLocale;

    OnTimeChangedListener mOnTimeChangedListener;
    OnTimeChangedListener mAutoFillChangeListener;

    // The value that was passed to autofill() - it must be stored because it getAutofillValue()
    // must return the exact same value that was autofilled, otherwise the widget will not be
    // properly highlighted after autofill().
    long mAutofilledValue;
public:
    AbstractTimePickerDelegate(TimePicker* delegator, Context* context);

    void setOnTimeChangedListener(const OnTimeChangedListener& callback) override;

    void setAutoFillChangeListener(const OnTimeChangedListener& callback) override;

    //void autofill(AutofillValue value) override;
    //AutofillValue getAutofillValue() override;
    //void resetAutofilledValue() { mAutofilledValue = 0; }override

    class SavedState:public View:BaseSavedState {
    private:
        int mHour;
        int mMinute;
        int mCurrentItemShowing;
        bool mIs24HourMode;
    public:
        SavedState(Parcelable superState, int hour, int minute, bool is24HourMode);

        SavedState(Parcelable superState, int hour, int minute, bool is24HourMode,
            int currentItemShowing);

         SavedState(Parcel in);

        int getHour();

        int getMinute();

        bool is24HourMode();

        int getCurrentItemShowing();
        void writeToParcel(Parcel dest, int flags);
    };
};

}/*endof namespace*/
#endif/*__TIME_PICKER_H__*/

