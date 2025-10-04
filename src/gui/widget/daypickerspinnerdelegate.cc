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
#if 0
#include <widget/R.h>
#include <utils/textutils.h>
#include <widget/calendarview.h>
#include <widget/daypickerspinnerdelegate.h>
namespace cdroid{

DatePickerSpinnerDelegate::DatePickerSpinnerDelegate(DatePicker* delegator, Context* context,const AttributeSet& attrs)
    :AbstractDatePickerDelegate(delegator, context){
    mDelegator = delegator;
    mContext = context;

    // initialization based on locale
    //setCurrentLocale(Locale.getDefault());

    bool spinnersShown = attrs.getBoolean("spinnersShown",DEFAULT_SPINNERS_SHOWN);
    bool calendarViewShown = attrs.getBoolean("calendarViewShown", DEFAULT_CALENDAR_VIEW_SHOWN);
    int startYear = attrs.getInt("startYear", DEFAULT_START_YEAR);
    int endYear = attrs.getInt("endYear", DEFAULT_END_YEAR);
    std::string minDate = attrs.getString("minDate");
    std::string maxDate = attrs.getString("maxDate");
    std::string layoutResourceId = attrs.getString("legacyLayout");//, com.android.internal.R.layout.date_picker_legacy);

    LayoutInflater* inflater = LayoutInflater::from(context);
    View* view = inflater->inflate(layoutResourceId, mDelegator, true);
    view->setSaveFromParentEnabled(false);

    NumberPicker::OnValueChangeListener onChangeListener =[this](NumberPicker& picker, int oldVal, int newVal) {
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
        } else {
            throw std::invalid_argument("");
        }
        // now set the date to the adjusted one
        setDate(mTempDate.get(Calendar::YEAR), mTempDate.get(Calendar::MONTH),
                mTempDate.get(Calendar::DAY_OF_MONTH));
        updateSpinners();
        updateCalendarView();
        notifyDateChanged();
    };

    mSpinners = (LinearLayout*) mDelegator->findViewById(R::id::pickers);

    // calendar view day-picker
    mCalendarView = (CalendarView*) mDelegator->findViewById(R::id::calendar_view);
    mCalendarView->setOnDateChangeListener([this](CalendarView& view, int year, int month, int monthDay) {
        setDate(year, month, monthDay);
        updateSpinners();
        notifyDateChanged();
    });

    // day
    mDaySpinner = (NumberPicker*) mDelegator->findViewById(R::id::day);
    //mDaySpinner->setFormatter(NumberPicker::getTwoDigitFormatter());
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

    // show only what the user required but make sure we
    // show something and the spinners have higher priority
    if (!spinnersShown && !calendarViewShown) {
        setSpinnersShown(true);
    } else {
        setSpinnersShown(spinnersShown);
        setCalendarViewShown(calendarViewShown);
    }

    // set the min date giving priority of the minDate over startYear
    mTempDate.clear();
    if (!TextUtils::isEmpty(minDate)) {
        if (!parseDate(minDate, mTempDate)) {
            mTempDate.set(startYear, 0, 1);
        }
    } else {
        mTempDate.set(startYear, 0, 1);
    }
    setMinDate(mTempDate.getTimeInMillis());

    // set the max date giving priority of the maxDate over endYear
    mTempDate.clear();
    if (!TextUtils::isEmpty(maxDate)) {
        if (!parseDate(maxDate, mTempDate)) {
            mTempDate.set(endYear, 11, 31);
        }
    } else {
        mTempDate.set(endYear, 11, 31);
    }
    setMaxDate(mTempDate.getTimeInMillis());

    // initialize to current date
    mCurrentDate.setTimeInMillis(SystemClock::currentTimeMillis());
    init(mCurrentDate.get(Calendar::YEAR), mCurrentDate.get(Calendar::MONTH), mCurrentDate
            .get(Calendar::DAY_OF_MONTH), nullptr);

    // re-order the number spinners to match the current date format
    reorderSpinners();

    // accessibility
    setContentDescriptions();

    // If not explicitly specified this view is important for accessibility.
    if (mDelegator->getImportantForAccessibility() == View::IMPORTANT_FOR_ACCESSIBILITY_AUTO) {
        mDelegator->setImportantForAccessibility(View::IMPORTANT_FOR_ACCESSIBILITY_YES);
    }
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

int DatePickerSpinnerDelegate::getYear() {
    return mCurrentDate.get(Calendar::YEAR);
}

int DatePickerSpinnerDelegate::getMonth() {
    return mCurrentDate.get(Calendar::MONTH);
}

int DatePickerSpinnerDelegate::getDayOfMonth() {
    return mCurrentDate.get(Calendar::DAY_OF_MONTH);
}

void DatePickerSpinnerDelegate::setFirstDayOfWeek(int firstDayOfWeek) {
    mCalendarView->setFirstDayOfWeek(firstDayOfWeek);
}

int DatePickerSpinnerDelegate::getFirstDayOfWeek() {
    return mCalendarView->getFirstDayOfWeek();
}

void DatePickerSpinnerDelegate::DatePickerSpinnerDelegate::setMinDate(long minDate) {
    mTempDate.setTimeInMillis(minDate);
    if (mTempDate.get(Calendar::YEAR) == mMinDate.get(Calendar::YEAR)
            && mTempDate.get(Calendar::DAY_OF_YEAR) == mMinDate.get(Calendar::DAY_OF_YEAR)) {
        // Same day, no-op.
        return;
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
    Calendar minDate;// = Calendar.getInstance();
    minDate.setTimeInMillis(mCalendarView->getMinDate());
    return minDate;
}

void DatePickerSpinnerDelegate::setMaxDate(long maxDate) {
    mTempDate.setTimeInMillis(maxDate);
    if (mTempDate.get(Calendar::YEAR) == mMaxDate.get(Calendar::YEAR)
            && mTempDate.get(Calendar::DAY_OF_YEAR) == mMaxDate.get(Calendar::DAY_OF_YEAR)) {
        // Same day, no-op.
        return;
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
    Calendar maxDate;// = Calendar.getInstance();
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

bool DatePickerSpinnerDelegate::isEnabled() const{
    return mIsEnabled;
}

CalendarView* DatePickerSpinnerDelegate::getCalendarView() {
    return mCalendarView;
}

void DatePickerSpinnerDelegate::setCalendarViewShown(bool shown) {
    mCalendarView->setVisibility(shown ? View::VISIBLE : View::GONE);
}

bool DatePickerSpinnerDelegate::getCalendarViewShown() {
    return (mCalendarView->getVisibility() == View::VISIBLE);
}

void DatePickerSpinnerDelegate::setSpinnersShown(bool shown) {
    mSpinners->setVisibility(shown ? View::VISIBLE : View::GONE);
}

bool DatePickerSpinnerDelegate::getSpinnersShown() {
    return mSpinners->isShown();
}

/*void DatePickerSpinnerDelegate::onConfigurationChanged(Configuration newConfig) {
    setCurrentLocale(newConfig.locale);
}*/

Parcelable* DatePickerSpinnerDelegate::onSaveInstanceState(Parcelable& superState) {
    return new SavedState(superState, getYear(), getMonth(), getDayOfMonth(),
            getMinDate().getTimeInMillis(), getMaxDate().getTimeInMillis());
}

void DatePickerSpinnerDelegate::onRestoreInstanceState(Parcelable& state) {
    if (dynamic_cast<SavedState&>(state)) {
        SavedState& ss = (SavedState&) state;
        setDate(ss.getSelectedYear(), ss.getSelectedMonth(), ss.getSelectedDay());
        updateSpinners();
        updateCalendarView();
    }
}

bool DatePickerSpinnerDelegate::dispatchPopulateAccessibilityEvent(AccessibilityEvent& event) {
    onPopulateAccessibilityEvent(event);
    return true;
}
#if 0
void DatePickerSpinnerDelegate::setCurrentLocale(Locale& locale) {
    AbstractDatePickerDelegate::setCurrentLocale(locale);

    mTempDate = getCalendarForLocale(mTempDate, locale);
    mMinDate = getCalendarForLocale(mMinDate, locale);
    mMaxDate = getCalendarForLocale(mMaxDate, locale);
    mCurrentDate = getCalendarForLocale(mCurrentDate, locale);

    mNumberOfMonths = mTempDate.getActualMaximum(Calendar::MONTH) + 1;
    mShortMonths = new DateFormatSymbols().getShortMonths();

    if (usingNumericMonths()) {
        // We're in a locale where a date should either be all-numeric, or all-text.
        // All-text would require custom NumberPicker formatters for day and year.
        mShortMonths = new String[mNumberOfMonths];
        for (int i = 0; i < mNumberOfMonths; ++i) {
            mShortMonths[i] = String.format("%d", i + 1);
        }
    }
}
#endif
bool DatePickerSpinnerDelegate::usingNumericMonths() const{
    return std::isdigit(mShortMonths[Calendar::JANUARY].at(0));
}
#if 0
Calendar DatePickerSpinnerDelegate::getCalendarForLocale(Calendar& oldCalendar, Locale& locale) {
    /*if (oldCalendar == null) {
        return Calendar.getInstance(locale);
    } else */{
        const int64_t currentTimeMillis = oldCalendar.getTimeInMillis();
        Calendar newCalendar;// = Calendar.getInstance(locale);
        newCalendar.setTimeInMillis(currentTimeMillis);
        return newCalendar;
    }
}
#endif
void DatePickerSpinnerDelegate::reorderSpinners() {
    mSpinners->removeAllViews();
    // We use numeric spinners for year and day, but textual months. Ask icu4c what
    // order the user's locale uses for that combination. http://b/7207103.
    std::string pattern = DateFormat.getBestDateTimePattern(Locale.getDefault(), "yyyyMMMdd");
    char[] order = DateFormat.getDateFormatOrder(pattern);
    const int spinnerCount = order.length;
    for (int i = 0; i < spinnerCount; i++) {
        switch (order[i]) {
        case 'd':
            mSpinners->addView(mDaySpinner);
            setImeOptions(mDaySpinner, spinnerCount, i);
            break;
        case 'M':
            mSpinners->addView(mMonthSpinner);
            setImeOptions(mMonthSpinner, spinnerCount, i);
            break;
        case 'y':
            mSpinners->addView(mYearSpinner);
            setImeOptions(mYearSpinner, spinnerCount, i);
            break;
        default:
            throw std::invalid_argument("Arrays.toString(order)");
        }
    }
}

bool DatePickerSpinnerDelegate::parseDate(const std::string& date, Calendar& outDate) {
    try {
        outDate.setTime(mDateFormat.parse(date));
        return true;
    } catch (std::exception& e) {
        return false;
    }
}

bool DatePickerSpinnerDelegate::isNewDate(int year, int month, int dayOfMonth) {
    return (mCurrentDate.get(Calendar::YEAR) != year) || (mCurrentDate.get(Calendar::MONTH) != month)
            || (mCurrentDate.get(Calendar::DAY_OF_MONTH) != dayOfMonth);
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
    // set the spinner ranges respecting the min and max dates
    std::vector<std::string>EMPTY;
    if (mCurrentDate.equals(mMinDate)) {
        mDaySpinner->setMinValue(mCurrentDate.get(Calendar::DAY_OF_MONTH));
        mDaySpinner->setMaxValue(mCurrentDate.getActualMaximum(Calendar::DAY_OF_MONTH));
        mDaySpinner->setWrapSelectorWheel(false);
        mMonthSpinner->setDisplayedValues(EMPTY);
        mMonthSpinner->setMinValue(mCurrentDate.get(Calendar::MONTH));
        mMonthSpinner->setMaxValue(mCurrentDate.getActualMaximum(Calendar::MONTH));
        mMonthSpinner->setWrapSelectorWheel(false);
    } else if (mCurrentDate.equals(mMaxDate)) {
        mDaySpinner->setMinValue(mCurrentDate.getActualMinimum(Calendar::DAY_OF_MONTH));
        mDaySpinner->setMaxValue(mCurrentDate.get(Calendar::DAY_OF_MONTH));
        mDaySpinner->setWrapSelectorWheel(false);
        mMonthSpinner->setDisplayedValues(EMPTY);
        mMonthSpinner->setMinValue(mCurrentDate.getActualMinimum(Calendar::MONTH));
        mMonthSpinner->setMaxValue(mCurrentDate.get(Calendar::MONTH));
        mMonthSpinner->setWrapSelectorWheel(false);
    } else {
        mDaySpinner->setMinValue(1);
        mDaySpinner->setMaxValue(mCurrentDate.getActualMaximum(Calendar::DAY_OF_MONTH));
        mDaySpinner->setWrapSelectorWheel(true);
        mMonthSpinner->setDisplayedValues(EMPTY);
        mMonthSpinner->setMinValue(0);
        mMonthSpinner->setMaxValue(11);
        mMonthSpinner->setWrapSelectorWheel(true);
    }

    // make sure the month names are a zero based array
    // with the months in the month spinner
    std::vector<std::string> displayedValues;std::copy(mShortMonths.begin()+mMonthSpinner->getMinValue(),
            mShortMonths.begin()+mMonthSpinner->getMaxValue() + 1,std::back_inserter(displayedValues));
    //Arrays.copyOfRange(mShortMonths, mMonthSpinner->getMinValue(), mMonthSpinner->getMaxValue() + 1);
    mMonthSpinner->setDisplayedValues(displayedValues);

    // year spinner range does not change based on the current date
    mYearSpinner->setMinValue(mMinDate.get(Calendar::YEAR));
    mYearSpinner->setMaxValue(mMaxDate.get(Calendar::YEAR));
    mYearSpinner->setWrapSelectorWheel(false);

    // set the spinner values
    mYearSpinner->setValue(mCurrentDate.get(Calendar::YEAR));
    mMonthSpinner->setValue(mCurrentDate.get(Calendar::MONTH));
    mDaySpinner->setValue(mCurrentDate.get(Calendar::DAY_OF_MONTH));

    if (usingNumericMonths()) {
        //mMonthSpinnerInput->setRawInputType(InputType::TYPE_CLASS_NUMBER);
    }
}

void DatePickerSpinnerDelegate::updateCalendarView() {
    mCalendarView->setDate(mCurrentDate.getTimeInMillis(), false, false);
}


void DatePickerSpinnerDelegate::notifyDateChanged() {
    mDelegator->sendAccessibilityEvent(AccessibilityEvent::TYPE_VIEW_SELECTED);
    if (mOnDateChangedListener != nullptr) {
        mOnDateChangedListener(*mDelegator, getYear(), getMonth(), getDayOfMonth());
    }
    /*if (mAutoFillChangeListener != null) {
        mAutoFillChangeListener.onDateChanged(*mDelegator, getYear(), getMonth(),getDayOfMonth());
    }*/
}

void DatePickerSpinnerDelegate::setImeOptions(NumberPicker* spinner, int spinnerCount, int spinnerIndex) {
    /*int imeOptions;
    if (spinnerIndex < spinnerCount - 1) {
        imeOptions = EditorInfo.IME_ACTION_NEXT;
    } else {
        imeOptions = EditorInfo.IME_ACTION_DONE;
    }
    TextView* input = (TextView*) spinner->findViewById(R::id::numberpicker_input);
    input.setImeOptions(imeOptions);*/
}

void DatePickerSpinnerDelegate::setContentDescriptions() {
    // Day
    trySetContentDescription(mDaySpinner, R::id::increment,
            com.android.internal.R.string.date_picker_increment_day_button);
    trySetContentDescription(mDaySpinner, R::id::decrement,
            com.android.internal.R.string.date_picker_decrement_day_button);
    // Month
    trySetContentDescription(mMonthSpinner, R::id::increment,
            com.android.internal.R.string.date_picker_increment_month_button);
    trySetContentDescription(mMonthSpinner, R::id::decrement,
            com.android.internal.R.string.date_picker_decrement_month_button);
    // Year
    trySetContentDescription(mYearSpinner, R::id::increment,
            com.android.internal.R.string.date_picker_increment_year_button);
    trySetContentDescription(mYearSpinner, R::id::decrement,
            com.android.internal.R.string.date_picker_decrement_year_button);
}

void DatePickerSpinnerDelegate::trySetContentDescription(View* root, int viewId, int contDescResId) {
    View* target = root->findViewById(viewId);
    if (target != nullptr) {
        target->setContentDescription(mContext.getString(contDescResId));
    }
}

void DatePickerSpinnerDelegate::updateInputState() {
    // Make sure that if the user changes the value and the IME is active
    // for one of the inputs if this widget, the IME is closed. If the user
    // changed the value via the IME and there is a next input the IME will
    // be shown, otherwise the user chose another means of changing the
    // value and having the IME up makes no sense.
    /*InputMethodManager inputMethodManager = mContext.getSystemService(InputMethodManager.class);
    if (inputMethodManager != nullptr) {
        if (mYearSpinnerInput->hasFocus()) {
            inputMethodManager.hideSoftInputFromView(mYearSpinnerInput, 0);
            mYearSpinnerInput->clearFocus();
        } else if (mMonthSpinnerInput->hasFocus()) {
            inputMethodManager->hideSoftInputFromView(mMonthSpinnerInput, 0);
            mMonthSpinnerInput->clearFocus();
        } else if (mDaySpinnerInput->hasFocus()) {
            inputMethodManager->hideSoftInputFromView(mDaySpinnerInput, 0);
            mDaySpinnerInput->clearFocus();
        }
    }*/
}
}
#endif
