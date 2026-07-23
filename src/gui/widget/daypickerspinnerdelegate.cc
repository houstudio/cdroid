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
#include <widget/R.h>
#include <widget/numberpicker.h>
#include <widget/calendarview.h>
#include <view/layoutinflater.h>
#include <core/systemclock.h>
#include <widget/daypickerspinnerdelegate.h>
#include <cstdio>

namespace cdroid{

DatePickerSpinnerDelegate::DatePickerSpinnerDelegate(DatePicker* delegator, Context* context,
        const AttributeSet& attrs)
    : AbstractDatePickerDelegate(delegator, context) {
    mDelegator = delegator;
    mContext = context;

    // DEFERRED: setCurrentLocale(Locale.getDefault()) -- CDROID has no Locale.
    // Bootstrap the locale-dependent fields directly with numeric months.
    mNumberOfMonths = 12; // mTempDate.getActualMaximum(Calendar.MONTH) + 1
    mShortMonths.clear();
    for (int i = 0; i < mNumberOfMonths; ++i) {
        mShortMonths.push_back(std::to_string(i + 1)); // TODO: DateFormatSymbols.getShortMonths()
    }

    const bool spinnersShown = attrs.getBoolean("spinnersShown", DEFAULT_SPINNERS_SHOWN);
    const bool calendarViewShown = attrs.getBoolean("calendarViewShown", DEFAULT_CALENDAR_VIEW_SHOWN);
    const int startYear = attrs.getInt("startYear", DEFAULT_START_YEAR);
    const int endYear = attrs.getInt("endYear", DEFAULT_END_YEAR);
    const std::string minDate = attrs.getString("minDate");
    const std::string maxDate = attrs.getString("maxDate");

    LayoutInflater* inflater = LayoutInflater::from(mContext);
    View* content = inflater->inflate("cdroid:layout/date_picker_legacy", nullptr, false);
    mDelegator->addView(content);

    NumberPicker::OnValueChangeListener onChangeListener =
        [this](NumberPicker& picker, int oldVal, int newVal) {
            updateInputState();
            mTempDate.setTimeInMillis(mCurrentDate.getTimeInMillis());
            // take care of wrapping of days and months to update greater fields
            if (&picker == mDaySpinner) {
                int maxDayOfMonth = mTempDate.getActualMaximum(Calendar::DAY_OF_MONTH);
                if (oldVal == maxDayOfMonth && newVal == 1) {
                    mTempDate.add(Calendar::DAY_OF_MONTH, 1);
                } else if (oldVal == 1 && newVal == maxDayOfMonth) {
                    mTempDate.add(Calendar::DAY_OF_MONTH, -1);
                } else {
                    mTempDate.add(Calendar::DAY_OF_MONTH, newVal - oldVal);
                }
            } else if (&picker == mMonthSpinner) {
                if (oldVal == 11 && newVal == 0) {
                    mTempDate.add(Calendar::MONTH, 1);
                } else if (oldVal == 0 && newVal == 11) {
                    mTempDate.add(Calendar::MONTH, -1);
                } else {
                    mTempDate.add(Calendar::MONTH, newVal - oldVal);
                }
            } else if (&picker == mYearSpinner) {
                mTempDate.set(Calendar::YEAR, newVal);
            }
            setDate(mTempDate.get(Calendar::YEAR), mTempDate.get(Calendar::MONTH),
                    mTempDate.get(Calendar::DAY_OF_MONTH));
            updateSpinners();
            updateCalendarView();
            notifyDateChanged();
        };

    mSpinners = (LinearLayout*) mDelegator->findViewById(R::id::pickers);

    // calendar view day-picker
    mCalendarView = (CalendarView*) mDelegator->findViewById(R::id::calendar_view);
    mCalendarView->setOnDateChangeListener(
        CalendarView::OnDateChangeListener([this](CalendarView&, int year, int month, int monthDay) {
            setDate(year, month, monthDay);
            updateSpinners();
            notifyDateChanged();
        }));

    // day
    mDaySpinner = (NumberPicker*) mDelegator->findViewById(R::id::day);
    mDaySpinner->setFormatter(NumberPicker::getTwoDigitFormatter());
    mDaySpinner->setOnLongPressUpdateInterval(100);
    mDaySpinner->setOnValueChangedListener(onChangeListener);
    mDaySpinnerInput = (EditText*) mDaySpinner->findViewById(R::id::numberpicker_input);

    // month
    mMonthSpinner = (NumberPicker*) mDelegator->findViewById(R::id::month);
    mMonthSpinner->setMinValue(0);
    mMonthSpinner->setMaxValue(mNumberOfMonths - 1);
    mMonthSpinner->setDisplayedValues(mShortMonths);
    mMonthSpinner->setOnLongPressUpdateInterval(200);
    mMonthSpinner->setOnValueChangedListener(onChangeListener);
    mMonthSpinnerInput = (EditText*) mMonthSpinner->findViewById(R::id::numberpicker_input);

    // year
    mYearSpinner = (NumberPicker*) mDelegator->findViewById(R::id::year);
    mYearSpinner->setOnLongPressUpdateInterval(100);
    mYearSpinner->setOnValueChangedListener(onChangeListener);
    mYearSpinnerInput = (EditText*) mYearSpinner->findViewById(R::id::numberpicker_input);

    // show only what the user required but make sure we show something and the
    // spinners have higher priority
    if (!spinnersShown && !calendarViewShown) {
        setSpinnersShown(true);
    } else {
        setSpinnersShown(spinnersShown);
        setCalendarViewShown(calendarViewShown);
    }

    // set the min date giving priority of the minDate over startYear
    mTempDate.clear();
    if (!minDate.empty()) {
        if (!parseDate(minDate, mTempDate)) {
            mTempDate.set(startYear, 0, 1);
        }
    } else {
        mTempDate.set(startYear, 0, 1);
    }
    setMinDate(mTempDate.getTimeInMillis());

    // set the max date giving priority of the maxDate over endYear
    mTempDate.clear();
    if (!maxDate.empty()) {
        if (!parseDate(maxDate, mTempDate)) {
            mTempDate.set(endYear, 11, 31);
        }
    } else {
        mTempDate.set(endYear, 11, 31);
    }
    setMaxDate(mTempDate.getTimeInMillis());

    // initialize to current date
    mCurrentDate.setTimeInMillis(SystemClock::currentTimeMillis());
    init(mCurrentDate.get(Calendar::YEAR), mCurrentDate.get(Calendar::MONTH),
         mCurrentDate.get(Calendar::DAY_OF_MONTH), nullptr);

    // DEFERRED: reorderSpinners() needs android.text.format.DateFormat.
    reorderSpinners();
    // DEFERRED: accessibility content descriptions.
    setContentDescriptions();
}

void DatePickerSpinnerDelegate::init(int year, int monthOfYear, int dayOfMonth,
         const DatePicker::OnDateChangedListener& onDateChangedListener) {
    setDate(year, monthOfYear, dayOfMonth);
    updateSpinners();
    updateCalendarView();
    mOnDateChangedListener = onDateChangedListener;
}

void DatePickerSpinnerDelegate::updateDate(int year, int month, int dayOfMonth) {
    if (!isNewDate(year, month, dayOfMonth)) {
        return;
    }
    setDate(year, month, dayOfMonth);
    updateSpinners();
    updateCalendarView();
    notifyDateChanged();
}

int DatePickerSpinnerDelegate::getYear() { return mCurrentDate.get(Calendar::YEAR); }
int DatePickerSpinnerDelegate::getMonth() { return mCurrentDate.get(Calendar::MONTH); }
int DatePickerSpinnerDelegate::getDayOfMonth() { return mCurrentDate.get(Calendar::DAY_OF_MONTH); }

void DatePickerSpinnerDelegate::setFirstDayOfWeek(int firstDayOfWeek) {
    mCalendarView->setFirstDayOfWeek(firstDayOfWeek);
}

int DatePickerSpinnerDelegate::getFirstDayOfWeek() {
    return mCalendarView->getFirstDayOfWeek();
}

void DatePickerSpinnerDelegate::setMinDate(int64_t minDate) {
    mTempDate.setTimeInMillis(minDate);
    if (mTempDate.get(Calendar::YEAR) == mMinDate.get(Calendar::YEAR)
            && mTempDate.get(Calendar::DAY_OF_YEAR) == mMinDate.get(Calendar::DAY_OF_YEAR)) {
        return; // Same day, no-op.
    }
    mMinDate.setTimeInMillis(minDate);
    mCalendarView->setMinDate(minDate);
    if (mCurrentDate.before(mMinDate)) {
        mCurrentDate.setTimeInMillis(mMinDate.getTimeInMillis());
        updateCalendarView();
    }
    updateSpinners();
}

Calendar DatePickerSpinnerDelegate::getMinDate() {
    Calendar minDate;
    minDate.setTimeInMillis(mCalendarView->getMinDate());
    return minDate;
}

void DatePickerSpinnerDelegate::setMaxDate(int64_t maxDate) {
    mTempDate.setTimeInMillis(maxDate);
    if (mTempDate.get(Calendar::YEAR) == mMaxDate.get(Calendar::YEAR)
            && mTempDate.get(Calendar::DAY_OF_YEAR) == mMaxDate.get(Calendar::DAY_OF_YEAR)) {
        return; // Same day, no-op.
    }
    mMaxDate.setTimeInMillis(maxDate);
    mCalendarView->setMaxDate(maxDate);
    if (mCurrentDate.after(mMaxDate)) {
        mCurrentDate.setTimeInMillis(mMaxDate.getTimeInMillis());
        updateCalendarView();
    }
    updateSpinners();
}

Calendar DatePickerSpinnerDelegate::getMaxDate() {
    Calendar maxDate;
    maxDate.setTimeInMillis(mCalendarView->getMaxDate());
    return maxDate;
}

void DatePickerSpinnerDelegate::setEnabled(bool enabled) {
    mDaySpinner->setEnabled(enabled);
    mMonthSpinner->setEnabled(enabled);
    mYearSpinner->setEnabled(enabled);
    mCalendarView->setEnabled(enabled);
    mIsEnabled = enabled;
}

bool DatePickerSpinnerDelegate::isEnabled() const { return mIsEnabled; }

CalendarView* DatePickerSpinnerDelegate::getCalendarView() { return mCalendarView; }

void DatePickerSpinnerDelegate::setCalendarViewShown(bool shown) {
    mCalendarView->setVisibility(shown ? View::VISIBLE : View::GONE);
}

bool DatePickerSpinnerDelegate::getCalendarViewShown() {
    return mCalendarView->getVisibility() == View::VISIBLE;
}

void DatePickerSpinnerDelegate::setSpinnersShown(bool shown) {
    mSpinners->setVisibility(shown ? View::VISIBLE : View::GONE);
}

bool DatePickerSpinnerDelegate::getSpinnersShown() {
    return mSpinners->isShown();
}

Parcelable* DatePickerSpinnerDelegate::onSaveInstanceState(Parcelable& superState) {
    return new AbstractDatePickerDelegate::SavedState(superState, getYear(), getMonth(),
            getDayOfMonth(), getMinDate().getTimeInMillis(), getMaxDate().getTimeInMillis());
}

void DatePickerSpinnerDelegate::onRestoreInstanceState(Parcelable& state) {
    auto* ss = dynamic_cast<AbstractDatePickerDelegate::SavedState*>(&state);
    if (ss) {
        setDate(ss->getSelectedYear(), ss->getSelectedMonth(), ss->getSelectedDay());
        updateSpinners();
        updateCalendarView();
    }
}

bool DatePickerSpinnerDelegate::dispatchPopulateAccessibilityEvent(AccessibilityEvent&) {
    // DEFERRED: accessibility.
    return true;
}

bool DatePickerSpinnerDelegate::usingNumericMonths() const {
    // DEFERRED: real check is Character.isDigit(mShortMonths[0].charAt(0)).
    // No DateFormatSymbols -> months are numeric ("1".."12").
    return true;
}

void DatePickerSpinnerDelegate::reorderSpinners() {
    // DEFERRED: Android reorders spinners to the locale date format via
    // android.text.format.DateFormat. CDROID has no DateFormat -- the inflated
    // layout's default order is kept. TODO: accept a date-format order attr.
}

bool DatePickerSpinnerDelegate::parseDate(const std::string& date, Calendar& outDate) {
    // Android uses SimpleDateFormat("MM/dd/yyyy"); sscanf suffices here.
    int month, day, year;
    if (std::sscanf(date.c_str(), "%d/%d/%d", &month, &day, &year) == 3) {
        outDate.set(year, month - 1, day); // Calendar month is 0-based
        return true;
    }
    return false;
}

bool DatePickerSpinnerDelegate::isNewDate(int year, int month, int dayOfMonth) {
    return (mCurrentDate.get(Calendar::YEAR) != year
            || mCurrentDate.get(Calendar::MONTH) != month
            || mCurrentDate.get(Calendar::DAY_OF_MONTH) != dayOfMonth);
}

void DatePickerSpinnerDelegate::setDate(int year, int month, int dayOfMonth) {
    mCurrentDate.set(year, month, dayOfMonth);
    resetAutofilledValue();
    if (mCurrentDate.before(mMinDate)) {
        mCurrentDate.setTimeInMillis(mMinDate.getTimeInMillis());
    } else if (mCurrentDate.after(mMaxDate)) {
        mCurrentDate.setTimeInMillis(mMaxDate.getTimeInMillis());
    }
}

void DatePickerSpinnerDelegate::updateSpinners() {
    auto sameDay = [](Calendar& a, Calendar& b) {
        return a.get(Calendar::YEAR) == b.get(Calendar::YEAR)
            && a.get(Calendar::DAY_OF_YEAR) == b.get(Calendar::DAY_OF_YEAR);
    };
    // set the spinner ranges respecting the min and max dates
    if (sameDay(mCurrentDate, mMinDate)) {
        mDaySpinner->setMinValue(mCurrentDate.get(Calendar::DAY_OF_MONTH));
        mDaySpinner->setMaxValue(mCurrentDate.getActualMaximum(Calendar::DAY_OF_MONTH));
        mDaySpinner->setWrapSelectorWheel(false);
        mMonthSpinner->setDisplayedValues({});
        mMonthSpinner->setMinValue(mCurrentDate.get(Calendar::MONTH));
        mMonthSpinner->setMaxValue(mCurrentDate.getActualMaximum(Calendar::MONTH));
        mMonthSpinner->setWrapSelectorWheel(false);
    } else if (sameDay(mCurrentDate, mMaxDate)) {
        mDaySpinner->setMinValue(mCurrentDate.getActualMinimum(Calendar::DAY_OF_MONTH));
        mDaySpinner->setMaxValue(mCurrentDate.get(Calendar::DAY_OF_MONTH));
        mDaySpinner->setWrapSelectorWheel(false);
        mMonthSpinner->setDisplayedValues({});
        mMonthSpinner->setMinValue(mCurrentDate.getActualMinimum(Calendar::MONTH));
        mMonthSpinner->setMaxValue(mCurrentDate.get(Calendar::MONTH));
        mMonthSpinner->setWrapSelectorWheel(false);
    } else {
        mDaySpinner->setMinValue(1);
        mDaySpinner->setMaxValue(mCurrentDate.getActualMaximum(Calendar::DAY_OF_MONTH));
        mDaySpinner->setWrapSelectorWheel(true);
        mMonthSpinner->setDisplayedValues({});
        mMonthSpinner->setMinValue(0);
        mMonthSpinner->setMaxValue(11);
        mMonthSpinner->setWrapSelectorWheel(true);
    }

    // make sure the month names are a zero based array with the months in the
    // month spinner
    std::vector<std::string> displayedValues(
        mShortMonths.begin() + mMonthSpinner->getMinValue(),
        mShortMonths.begin() + mMonthSpinner->getMaxValue() + 1);
    mMonthSpinner->setDisplayedValues(displayedValues);

    // year spinner range does not change based on the current date
    mYearSpinner->setMinValue(mMinDate.get(Calendar::YEAR));
    mYearSpinner->setMaxValue(mMaxDate.get(Calendar::YEAR));
    mYearSpinner->setWrapSelectorWheel(false);

    // set the spinner values
    mYearSpinner->setValue(mCurrentDate.get(Calendar::YEAR));
    mMonthSpinner->setValue(mCurrentDate.get(Calendar::MONTH));
    mDaySpinner->setValue(mCurrentDate.get(Calendar::DAY_OF_MONTH));
}

void DatePickerSpinnerDelegate::updateCalendarView() {
    mCalendarView->setDate(mCurrentDate.getTimeInMillis(), false, false);
}

void DatePickerSpinnerDelegate::notifyDateChanged() {
    // DEFERRED: mDelegator->sendAccessibilityEvent(TYPE_VIEW_SELECTED).
    if (mOnDateChangedListener) {
        mOnDateChangedListener(*mDelegator, getYear(), getMonth(), getDayOfMonth());
    }
    if (mAutoFillChangeListener) {
        mAutoFillChangeListener(*mDelegator, getYear(), getMonth(), getDayOfMonth());
    }
}

void DatePickerSpinnerDelegate::setImeOptions(NumberPicker*, int, int) {
    // DEFERRED: IME options on the input (IME_ACTION_NEXT/DONE).
}

void DatePickerSpinnerDelegate::setContentDescriptions() {
    // DEFERRED: accessibility content descriptions.
}

void DatePickerSpinnerDelegate::trySetContentDescription(View*, int, int) {
    // DEFERRED: accessibility content descriptions.
}

void DatePickerSpinnerDelegate::updateInputState() {
    // DEFERRED: hide the IME when the user changes a value via the spinners.
}

} // namespace cdroid
