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
#ifndef __DATE_PICKER_H__
#define __DATE_PICKER_H__
#include <core/calendar.h>
#include <widget/framelayout.h>
namespace cdroid{
class CalendarView;
class DatePicker:public FrameLayout {
public:
    static constexpr int MODE_SPINNER = 1;
    static constexpr int MODE_CALENDAR = 2;
    DECLARE_UIEVENT(void,OnDateChangedListener,DatePicker&,int/*year*/,int/*month*/,int/*dayOfMonth*/);
    DECLARE_UIEVENT(void,ValidationCallback,bool);
    class DatePickerDelegate;
    class AbstractDatePickerDelegate;
private:
    DatePickerDelegate* mDelegate;
    int mMode;
private:
    DatePickerDelegate* createSpinnerUIDelegate(Context*,const AttributeSet& attrs);
    DatePickerDelegate* createCalendarUIDelegate(Context*,const AttributeSet& attrs);
protected:
    //void onConfigurationChanged(Configuration newConfig)override;
    void dispatchRestoreInstanceState(SparseArray<Parcelable*>& container)override;
    Parcelable* onSaveInstanceState()override;
    void onRestoreInstanceState(Parcelable&state)override;
public:
    DatePicker(Context* context,const AttributeSet& attrs);
    int getMode();

    void init(int year, int monthOfYear, int dayOfMonth,const OnDateChangedListener& onDateChangedListener);

    void setOnDateChangedListener(const OnDateChangedListener& onDateChangedListener);

    void updateDate(int year, int month, int dayOfMonth);
    int getYear();
    int getMonth();
    int getDayOfMonth();

    int64_t getMinDate();
    void setMinDate(int64_t minDate);
    int64_t getMaxDate();
    void setMaxDate(int64_t maxDate);

    void setValidationCallback(const ValidationCallback& callback);

    void setEnabled(bool enabled)override;
    bool isEnabled()const override;

    bool dispatchPopulateAccessibilityEventInternal(AccessibilityEvent& event)override;

    void onPopulateAccessibilityEventInternal(AccessibilityEvent& event);

    std::string getAccessibilityClassName()const override;

    void setFirstDayOfWeek(int firstDayOfWeek);

    int getFirstDayOfWeek();

    bool getCalendarViewShown();

    CalendarView* getCalendarView();

    void setCalendarViewShown(bool shown);

    bool getSpinnersShown();
    void setSpinnersShown(bool shown);

    //void dispatchProvideAutofillStructure(ViewStructure structure, int flags) override;
    //void autofill(AutofillValue value) override;
    //int getAutofillType() override;
    //AutofillValue getAutofillValue() override;
};/*endof DatePicker*/

class DatePicker::DatePickerDelegate {
public:
    virtual ~DatePickerDelegate()=default;
    virtual void init(int year, int monthOfYear, int dayOfMonth,
              const OnDateChangedListener& onDateChangedListener)=0;

    virtual void setOnDateChangedListener(const OnDateChangedListener& onDateChangedListener)=0;
    virtual void setAutoFillChangeListener(const OnDateChangedListener& onDateChangedListener)=0;

    virtual void updateDate(int year, int month, int dayOfMonth)=0;

    virtual int getYear()=0;
    virtual int getMonth()=0;
    virtual int getDayOfMonth()=0;

    //void autofill(AutofillValue value);
    //AutofillValue getAutofillValue();

    virtual void setFirstDayOfWeek(int firstDayOfWeek)=0;
    virtual int getFirstDayOfWeek()=0;

    virtual void setMinDate(int64_t minDate)=0;
    virtual Calendar getMinDate()=0;

    virtual void setMaxDate(int64_t maxDate)=0;
    virtual Calendar getMaxDate()=0;

    virtual void setEnabled(bool enabled)=0;
    virtual bool isEnabled()const=0;

    virtual CalendarView* getCalendarView()=0;

    virtual void setCalendarViewShown(bool shown)=0;
    virtual bool getCalendarViewShown()=0;

    virtual void setSpinnersShown(bool shown)=0;
    virtual bool getSpinnersShown()=0;

    virtual void setValidationCallback(const ValidationCallback& callback)=0;

    //void onConfigurationChanged(Configuration newConfig);

    virtual Parcelable* onSaveInstanceState(Parcelable& superState)=0;
    virtual void onRestoreInstanceState(Parcelable& state)=0;

    virtual bool dispatchPopulateAccessibilityEvent(AccessibilityEvent& event)=0;
    virtual void onPopulateAccessibilityEvent(AccessibilityEvent& event)=0;
};

/**
 * An abstract class which can be used as a start for DatePicker implementations
 */
class DatePicker::AbstractDatePickerDelegate:public DatePickerDelegate {
protected:
    DatePicker* mDelegator;
    Context* mContext;
    Calendar mCurrentDate;
    //Locale mCurrentLocale;

    OnDateChangedListener mOnDateChangedListener;
    OnDateChangedListener mAutoFillChangeListener;
    ValidationCallback mValidationCallback;
    long mAutofilledValue;
    //void setCurrentLocale(Locale& locale);
    void resetAutofilledValue();
    void onValidationChanged(bool valid);
    //void onLocaleChanged(Locale& locale);
    std::string getFormattedCurrentDate();
public:
    AbstractDatePickerDelegate(DatePicker* delegator, Context* context);

    void setOnDateChangedListener(const OnDateChangedListener& callback)override;
    void setAutoFillChangeListener(const OnDateChangedListener& callback) override;
    void setValidationCallback(const ValidationCallback& callback)override;

    //void autofill(AutofillValue value)override;
    //AutofillValue getAutofillValue()override;
    void onPopulateAccessibilityEvent(AccessibilityEvent& event)override;

    class SavedState:public View::BaseSavedState {
    private:
        int mSelectedYear;
        int mSelectedMonth;
        int mSelectedDay;
        int64_t mMinDate;
        int64_t mMaxDate;
        int mCurrentView;
        int mListPosition;
        int mListPositionOffset;
        SavedState(Parcel& in);
    public:
        SavedState(Parcelable& superState, int year, int month, int day, int64_t minDate,int64_t maxDate);
        SavedState(Parcelable& superState, int year, int month, int day, int64_t minDate,
            int64_t maxDate, int currentView, int listPosition, int listPositionOffset);
        void writeToParcel(Parcel& dest, int flags) override;

        int getSelectedDay();
        int getSelectedMonth();
        int getSelectedYear();
        int64_t getMinDate();
        int64_t getMaxDate();
        int getCurrentView();
        int getListPosition();
        int getListPositionOffset();
    };
};/*endof AbstractDatePickerDelegate*/
}/*endof namespace*/
#endif/*__DATE_PICKER_H__*/
