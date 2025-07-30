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
#ifndef __DAYPICKER_SPINNER_DELEGATE_H__
#define __DAYPICKER_SPINNER_DELEGATE_H__
#include <widget/datepicker.h>
#include <widget/numberpicker.h>
#include <widget/linearlayout.h>
#include <widget/edittext.h>
namespace cdroid{
class DatePickerSpinnerDelegate:public DatePicker::AbstractDatePickerDelegate {
private:
    static constexpr const char* DATE_FORMAT = "MM/dd/yyyy";

    static constexpr int DEFAULT_START_YEAR = 1900;
    static constexpr int DEFAULT_END_YEAR = 2100;
    static constexpr bool DEFAULT_CALENDAR_VIEW_SHOWN = true;
    static constexpr bool DEFAULT_SPINNERS_SHOWN = true;
    static constexpr bool DEFAULT_ENABLED_STATE = true;

    LinearLayout* mSpinners;
    NumberPicker* mDaySpinner;
    NumberPicker* mMonthSpinner;
    NumberPicker* mYearSpinner;
    EditText* mDaySpinnerInput;
    EditText* mMonthSpinnerInput;
    EditText* mYearSpinnerInput;
    CalendarView* mCalendarView;

    std::vector<std::string> mShortMonths;

    //final java.text.DateFormat mDateFormat = new SimpleDateFormat(DATE_FORMAT);

    int mNumberOfMonths;
    bool mIsEnabled = DEFAULT_ENABLED_STATE;
    Calendar mTempDate;
    Calendar mMinDate;
    Calendar mMaxDate;
public:
    DatePickerSpinnerDelegate(DatePicker* delegator, Context* context,const AttributeSet& attrs);
    void init(int year, int monthOfYear, int dayOfMonth,
              const DatePicker::OnDateChangedListener& onDateChangedListener)override;
    void updateDate(int year, int month, int dayOfMonth) override;
    int getYear()override;

    int getMonth() override;

    int getDayOfMonth()override;

    void setFirstDayOfWeek(int firstDayOfWeek)override;
    int getFirstDayOfWeek()override;

    void setMinDate(long minDate)override;
    Calendar getMinDate() override;

    void setMaxDate(long maxDate)override;
    Calendar getMaxDate() override;

    void setEnabled(bool enabled) override;
    bool isEnabled() const override;

    CalendarView* getCalendarView() override;

    void setCalendarViewShown(bool shown) override;
    bool getCalendarViewShown() override;

    void setSpinnersShown(bool shown) override;
    bool getSpinnersShown() override;

    //void onConfigurationChanged(Configuration newConfig)override;

    Parcelable* onSaveInstanceState(Parcelable& superState);
    void onRestoreInstanceState(Parcelable& state)override;

    bool dispatchPopulateAccessibilityEvent(AccessibilityEvent& event)override;
protected:
    //void setCurrentLocale(Locale& locale)override;
private:
    bool usingNumericMonths()const;

    //Calendar getCalendarForLocale(Calendar& oldCalendar, Locale& locale);
    void reorderSpinners();
    bool parseDate(const std::string& date, Calendar& outDate);
    bool isNewDate(int year, int month, int dayOfMonth);

    void setDate(int year, int month, int dayOfMonth);

    void updateSpinners();
    void updateCalendarView();
    void notifyDateChanged();
    void setImeOptions(NumberPicker* spinner, int spinnerCount, int spinnerIndex);
    void setContentDescriptions();
    void trySetContentDescription(View* root, int viewId, int contDescResId);
    void updateInputState();
};
}/*endof namespace*/
#endif/*__DAYPICKER_SPINNER_DELEGATE_H__*/
