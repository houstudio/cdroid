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
#include <widget/datepicker.h>
namespace cdroid{
DatePicker::DatePicker(Context* context,const AttributeSet& attrs)
    :FrameLayout(context, attrs){

    if (getImportantForAutofill() == IMPORTANT_FOR_AUTOFILL_AUTO) {
        setImportantForAutofill(IMPORTANT_FOR_AUTOFILL_YES);
    }

    const bool isDialogMode = attrs.getBoolean("dialogMode", false);
    const int requestedMode = attrs.getInt("datePickerMode", MODE_SPINNER);
    const int firstDayOfWeek = attrs.getInt("firstDayOfWeek", 0);

    if (requestedMode == MODE_CALENDAR && isDialogMode) {
        // You want MODE_CALENDAR? YOU CAN'T HANDLE MODE_CALENDAR! Well,
        // maybe you can depending on your screen size. Let's check...
        mMode = requestedMode;//context.getResources().getInteger(R.integer.date_picker_mode);
    } else {
        mMode = requestedMode;
    }

    switch (mMode) {
    case MODE_CALENDAR:
        mDelegate = createCalendarUIDelegate(context, attrs);
        break;
    case MODE_SPINNER:
    default:
        mDelegate = createSpinnerUIDelegate(context, attrs);
        break;
    }

    if (firstDayOfWeek != 0) {
        setFirstDayOfWeek(firstDayOfWeek);
    }

    /*mDelegate->setAutoFillChangeListener((v, y, m, d) -> {
        final AutofillManager afm = context.getSystemService(AutofillManager.class);
        if (afm != null) {
            afm.notifyValueChanged(this);
        }
    });*/
}

DatePicker::DatePickerDelegate* DatePicker::createSpinnerUIDelegate(Context* context,const AttributeSet& attrs) {
    return nullptr;//new DatePickerSpinnerDelegate(this, context, attrs);
}

DatePicker::DatePickerDelegate* DatePicker::createCalendarUIDelegate(Context* context,const AttributeSet& attrs) {
    return nullptr;//new DatePickerCalendarDelegate(this, context, attrs);
}

int DatePicker::getMode() {
    return mMode;
}

void DatePicker::init(int year, int monthOfYear, int dayOfMonth,const OnDateChangedListener& onDateChangedListener) {
    mDelegate->init(year, monthOfYear, dayOfMonth, onDateChangedListener);
}

void DatePicker::setOnDateChangedListener(const OnDateChangedListener& onDateChangedListener) {
    mDelegate->setOnDateChangedListener(onDateChangedListener);
}

void DatePicker::updateDate(int year, int month, int dayOfMonth) {
    mDelegate->updateDate(year, month, dayOfMonth);
}

int DatePicker::getYear() {
    return mDelegate->getYear();
}

int DatePicker::getMonth() {
    return mDelegate->getMonth();
}

int DatePicker::getDayOfMonth() {
    return mDelegate->getDayOfMonth();
}

int64_t DatePicker::getMinDate() {
    return mDelegate->getMinDate().getTimeInMillis();
}

void DatePicker::setMinDate(int64_t minDate) {
    mDelegate->setMinDate(minDate);
}

int64_t DatePicker::getMaxDate() {
    return mDelegate->getMaxDate().getTimeInMillis();
}

void DatePicker::setMaxDate(int64_t maxDate) {
    mDelegate->setMaxDate(maxDate);
}

void DatePicker::setValidationCallback(const ValidationCallback& callback) {
    mDelegate->setValidationCallback(callback);
}

void DatePicker::setEnabled(bool enabled) {
    if (mDelegate->isEnabled() == enabled) {
        return;
    }
    FrameLayout::setEnabled(enabled);
    mDelegate->setEnabled(enabled);
}

bool DatePicker::isEnabled() const{
    return mDelegate->isEnabled();
}

bool DatePicker::dispatchPopulateAccessibilityEventInternal(AccessibilityEvent& event) {
    return mDelegate->dispatchPopulateAccessibilityEvent(event);
}

void DatePicker::onPopulateAccessibilityEventInternal(AccessibilityEvent& event) {
    FrameLayout::onPopulateAccessibilityEventInternal(event);
    mDelegate->onPopulateAccessibilityEvent(event);
}

std::string DatePicker::getAccessibilityClassName() const{
    return "DatePicker";
}

/*void DatePicker::onConfigurationChanged(Configuration newConfig) {
    FrameLayout::onConfigurationChanged(newConfig);
    mDelegate->onConfigurationChanged(newConfig);
}*/

void DatePicker::setFirstDayOfWeek(int firstDayOfWeek) {
    if (firstDayOfWeek < Calendar::SUNDAY || firstDayOfWeek > Calendar::SATURDAY) {
        throw std::invalid_argument("firstDayOfWeek must be between 1 and 7");
    }
    mDelegate->setFirstDayOfWeek(firstDayOfWeek);
}

int DatePicker::getFirstDayOfWeek() {
    return mDelegate->getFirstDayOfWeek();
}

bool DatePicker::getCalendarViewShown() {
    return mDelegate->getCalendarViewShown();
}

CalendarView* DatePicker::getCalendarView() {
    return mDelegate->getCalendarView();
}

void DatePicker::setCalendarViewShown(bool shown) {
    mDelegate->setCalendarViewShown(shown);
}

bool DatePicker::getSpinnersShown() {
    return mDelegate->getSpinnersShown();
}

void DatePicker::setSpinnersShown(bool shown) {
    mDelegate->setSpinnersShown(shown);
}

void DatePicker::dispatchRestoreInstanceState(SparseArray<Parcelable*>& container) {
    dispatchThawSelfOnly(container);
}

Parcelable* DatePicker::onSaveInstanceState() {
    Parcelable* superState = FrameLayout::onSaveInstanceState();
    return mDelegate->onSaveInstanceState(*superState);
}

void DatePicker::onRestoreInstanceState(Parcelable& state) {
    BaseSavedState& ss = (BaseSavedState&) state;
    FrameLayout::onRestoreInstanceState(*ss.getSuperState());
    mDelegate->onRestoreInstanceState(ss);
}

/////////////// DatePicker::AbstractDatePickerDelegate implements DatePickerDelegate /////////////////

DatePicker::AbstractDatePickerDelegate::AbstractDatePickerDelegate(DatePicker* delegator, Context* context) {
    mDelegator = delegator;
    mContext = context;
    //setCurrentLocale(Locale.getDefault());
}

/*void DatePicker::AbstractDatePickerDelegate::setCurrentLocale(Locale& locale) {
    if (!locale.equals(mCurrentLocale)) {
        mCurrentLocale = locale;
        onLocaleChanged(locale);
    }
}*/

void DatePicker::AbstractDatePickerDelegate::setOnDateChangedListener(const OnDateChangedListener& callback) {
    mOnDateChangedListener = callback;
}

void DatePicker::AbstractDatePickerDelegate::setAutoFillChangeListener(const OnDateChangedListener& callback) {
    mAutoFillChangeListener = callback;
}

void DatePicker::AbstractDatePickerDelegate::setValidationCallback(const ValidationCallback& callback) {
    mValidationCallback = callback;
}

/*void DatePicker::AbstractDatePickerDelegate::autofill(AutofillValue value) {
    if (value == null || !value.isDate()) {
        Log.w(LOG_TAG, value + " could not be autofilled into " + this);
        return;
    }
    const int64_t time = value.getDateValue();
    const Calendar cal = Calendar.getInstance(mCurrentLocale);
    cal.setTimeInMillis(time);
    updateDate(cal.get(Calendar::YEAR), cal.get(Calendar::MONTH),
            cal.get(Calendar::DAY_OF_MONTH));

    // Must set mAutofilledValue *after* calling subclass method to make sure the value
    // returned by getAutofillValue() matches it.
    mAutofilledValue = time;
}
AutofillValue DatePicker::AbstractDatePickerDelegate::getAutofillValue() {
    const int64_t time=(mAutofilledValue!=0)? mAutofilledValue:mCurrentDate.getTimeInMillis();
    return AutofillValue.forDate(time);
}
void DatePicker::AbstractDatePickerDelegate::resetAutofilledValue() {
    mAutofilledValue = 0;
}*/

void DatePicker::AbstractDatePickerDelegate::onValidationChanged(bool valid) {
    if (mValidationCallback != nullptr) {
        mValidationCallback(valid);
    }
}

/*void DatePicker::AbstractDatePickerDelegate::onLocaleChanged(Locale& locale) {
    // Stub.
}*/

void DatePicker::AbstractDatePickerDelegate::onPopulateAccessibilityEvent(AccessibilityEvent& event) {
    //event.getText().add(getFormattedCurrentDate());
}

std::string DatePicker::AbstractDatePickerDelegate::getFormattedCurrentDate() {
   return "";/*DateUtils::formatDateTime(mContext, mCurrentDate.getTimeInMillis(),
           DateUtils::FORMAT_SHOW_DATE | DateUtils::FORMAT_SHOW_YEAR
                   | DateUtils::FORMAT_SHOW_WEEKDAY);*/
}

/////////////////DatePicker::AbstractDatePickerDelegate::SavedState///////////////////////

DatePicker::AbstractDatePickerDelegate::SavedState::SavedState(Parcelable& superState,
        int year, int month, int day, int64_t minDate,int64_t maxDate)
    :SavedState(superState, year, month, day, minDate, maxDate, 0, 0, 0){
}

DatePicker::AbstractDatePickerDelegate::SavedState::SavedState(Parcelable& superState, int year, int month, int day, int64_t minDate,
        int64_t maxDate, int currentView, int listPosition, int listPositionOffset)
  :View::BaseSavedState(&superState){
    mSelectedYear = year;
    mSelectedMonth = month;
    mSelectedDay = day;
    mMinDate = minDate;
    mMaxDate = maxDate;
    mCurrentView = currentView;
    mListPosition = listPosition;
    mListPositionOffset = listPositionOffset;
}

DatePicker::AbstractDatePickerDelegate::SavedState::SavedState(Parcel& in)
  :BaseSavedState(in){
    mSelectedYear = in.readInt();
    mSelectedMonth = in.readInt();
    mSelectedDay = in.readInt();
    mMinDate = in.readLong();
    mMaxDate = in.readLong();
    mCurrentView = in.readInt();
    mListPosition = in.readInt();
    mListPositionOffset = in.readInt();
}

void DatePicker::AbstractDatePickerDelegate::SavedState::writeToParcel(Parcel& dest, int flags) {
    BaseSavedState::writeToParcel(dest, flags);
    dest.writeInt(mSelectedYear);
    dest.writeInt(mSelectedMonth);
    dest.writeInt(mSelectedDay);
    dest.writeLong(mMinDate);
    dest.writeLong(mMaxDate);
    dest.writeInt(mCurrentView);
    dest.writeInt(mListPosition);
    dest.writeInt(mListPositionOffset);
}

int DatePicker::AbstractDatePickerDelegate::SavedState::getSelectedDay() {
    return mSelectedDay;
}

int DatePicker::AbstractDatePickerDelegate::SavedState::getSelectedMonth() {
    return mSelectedMonth;
}

int DatePicker::AbstractDatePickerDelegate::SavedState::getSelectedYear() {
    return mSelectedYear;
}

int64_t DatePicker::AbstractDatePickerDelegate::SavedState::getMinDate() {
    return mMinDate;
}

int64_t DatePicker::AbstractDatePickerDelegate::SavedState::getMaxDate() {
    return mMaxDate;
}

int DatePicker::AbstractDatePickerDelegate::SavedState::getCurrentView() {
    return mCurrentView;
}

int DatePicker::AbstractDatePickerDelegate::SavedState::getListPosition() {
    return mListPosition;
}

int DatePicker::AbstractDatePickerDelegate::SavedState::getListPositionOffset() {
    return mListPositionOffset;
}

///////////////////////////////////////////////////////////////////////////////////////
/*
void DatePicker::dispatchProvideAutofillStructure(ViewStructure structure, int flags) {
    structure.setAutofillId(getAutofillId());
    onProvideAutofillStructure(structure, flags);
}

void DatePicker::autofill(AutofillValue value) {
    if (!isEnabled()) return;
    mDelegate.autofill(value);
}

int DatePicker::getAutofillType() {
    return isEnabled() ? AUTOFILL_TYPE_DATE : AUTOFILL_TYPE_NONE;
}

AutofillValue DatePicker::getAutofillValue() {
    return isEnabled() ? mDelegate.getAutofillValue() : null;
}*/
}/*endof namespace*/
