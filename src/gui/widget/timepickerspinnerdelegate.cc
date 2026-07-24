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
#include <widget/timepickerspinnerdelegate.h>
#include <widget/numberpicker.h>
#include <widget/button.h>
#include <widget/edittext.h>
#include <widget/textview.h>
#include <view/viewgroup.h>
#include <view/layoutinflater.h>
#include <widget/timepicker.h>

namespace cdroid{

TimePickerSpinnerDelegate::TimePickerSpinnerDelegate(TimePicker* delegator, Context* context,const AttributeSet& attrs)
    :AbstractTimePickerDelegate(delegator, context) {

    const std::string layoutResourceId = attrs.getString("legacyLayout", "cdroid:layout/time_picker_legacy");

    LayoutInflater* inflater = LayoutInflater::from(mContext);
    View* view = inflater->inflate(layoutResourceId, mDelegator, true);
    view->setSaveFromParentEnabled(false);

    // hour
    mHourSpinner = (NumberPicker*) delegator->findViewById(R::id::hour);
    mHourSpinner->setOnValueChangedListener([this](NumberPicker& spinner, int oldVal, int newVal) {
        updateInputState();
        if (!is24Hour()) {
            if ((oldVal == HOURS_IN_HALF_DAY - 1 && newVal == HOURS_IN_HALF_DAY) ||
                    (oldVal == HOURS_IN_HALF_DAY && newVal == HOURS_IN_HALF_DAY - 1)) {
                mIsAm = !mIsAm;
                updateAmPmControl();
            }
        }
        onTimeChanged();
    });
    mHourSpinnerInput = (EditText*)mHourSpinner->findViewById(R::id::numberpicker_input);
    // DEFERRED: mHourSpinnerInput->setImeOptions(EditorInfo::IME_ACTION_NEXT);

    // divider (only for the new widget style)
    mDivider = (TextView*)mDelegator->findViewById(R::id::divider);
    if (mDivider != nullptr) {
        setDividerText();
    }

    // minute
    mMinuteSpinner = (NumberPicker*) mDelegator->findViewById(R::id::minute);
    mMinuteSpinner->setMinValue(0);
    mMinuteSpinner->setMaxValue(59);
    mMinuteSpinner->setOnLongPressUpdateInterval(100);
    mMinuteSpinner->setOnValueChangedListener([this](NumberPicker& spinner, int oldVal, int newVal) {
        updateInputState();
        int minValue = mMinuteSpinner->getMinValue();
        int maxValue = mMinuteSpinner->getMaxValue();
        if (oldVal == maxValue && newVal == minValue) {
            int newHour = mHourSpinner->getValue() + 1;
            if (!is24Hour() && newHour == HOURS_IN_HALF_DAY) {
                mIsAm = !mIsAm;
                updateAmPmControl();
            }
            mHourSpinner->setValue(newHour);
        } else if (oldVal == minValue && newVal == maxValue) {
            int newHour = mHourSpinner->getValue() - 1;
            if (!is24Hour() && newHour == HOURS_IN_HALF_DAY - 1) {
                mIsAm = !mIsAm;
                updateAmPmControl();
            }
            mHourSpinner->setValue(newHour);
        }
        onTimeChanged();
    });
    mMinuteSpinnerInput = (EditText*)mMinuteSpinner->findViewById(R::id::numberpicker_input);
    // DEFERRED: mMinuteSpinnerInput->setImeOptions(EditorInfo::IME_ACTION_NEXT);

    // Get the am/pm strings and use them in the spinner.
    mAmPmStrings = TimePicker::getAmPmStrings(context);

    // am/pm
    View* amPmView = mDelegator->findViewById(R::id::amPm);
    if (dynamic_cast<Button*>(amPmView)) {
        mAmPmSpinner = nullptr;
        mAmPmSpinnerInput = nullptr;
        mAmPmButton = (Button*) amPmView;
        mAmPmButton->setOnClickListener([this](View& button) {
            button.requestFocus();
            mIsAm = !mIsAm;
            updateAmPmControl();
            onTimeChanged();
        });
    } else {
        mAmPmButton = nullptr;
        mAmPmSpinner = (NumberPicker*) amPmView;
        mAmPmSpinner->setMinValue(0);
        mAmPmSpinner->setMaxValue(1);
        mAmPmSpinner->setDisplayedValues(mAmPmStrings);
        mAmPmSpinner->setOnValueChangedListener([this](NumberPicker& picker, int /*oldVal*/, int /*newVal*/) {
            updateInputState();
            picker.requestFocus();
            mIsAm = !mIsAm;
            updateAmPmControl();
            onTimeChanged();
        });
        mAmPmSpinnerInput = (EditText*)mAmPmSpinner->findViewById(R::id::numberpicker_input);
        // DEFERRED: mAmPmSpinnerInput->setImeOptions(EditorInfo::IME_ACTION_DONE);
    }

    // DEFERRED: am/pm reorder for locale (isAmPmAtStart). Default keeps layout order.

    getHourFormatData();

    // update controls to initial state
    updateHourControl();
    updateMinuteControl();
    updateAmPmControl();

    // set to current time (mTempCalendar is default-constructed to now)
    // AOSP uses Calendar.getInstance(locale) which carries the local time zone;
    // CDROID has no locale dispatch, so mirror the local-TZ part here (the
    // default ctor leaves zone=0/UTC, which would show the hour in UTC).
    mTempCalendar.setTimeZone(Calendar::getInstance()->getTimeZone());
    setHour(mTempCalendar.get(Calendar::HOUR_OF_DAY));
    setMinute(mTempCalendar.get(Calendar::MINUTE));

    if (!isEnabled()) {
        setEnabled(false);
    }

    // DEFERRED: accessibility content descriptions.
    setContentDescriptions();
}

bool TimePickerSpinnerDelegate::validateInput() {
    return true;
}

void TimePickerSpinnerDelegate::getHourFormatData() {
    // DEFERRED: android.text.format.DateFormat.getBestDateTimePattern not ported.
    // Default to a 24-hour, two-digit style; updateHourControl uses mIs24HourView too.
    mHourFormat = 'H';
    mHourWithTwoDigit = true;
}

bool TimePickerSpinnerDelegate::isAmPmAtStart() {
    // DEFERRED: needs DateFormat; assume am/pm at end.
    return false;
}

void TimePickerSpinnerDelegate::setDividerText() {
    // DEFERRED: locale time separator via DateFormat; default ':'.
    if (mDivider != nullptr) {
        mDivider->setText(":");
    }
}

void TimePickerSpinnerDelegate::setDate(int hour, int minute) {
    setCurrentHour(hour, false);
    setCurrentMinute(minute, false);
    onTimeChanged();
}

void TimePickerSpinnerDelegate::setHour(int hour) {
    setCurrentHour(hour, true);
}

void TimePickerSpinnerDelegate::setCurrentHour(int currentHour, bool notifyTimeChanged) {
    if (currentHour == getHour()) {
        return;
    }
    if (!is24Hour()) {
        // convert [0,23] ordinal to wall clock display
        if (currentHour >= HOURS_IN_HALF_DAY) {
            mIsAm = false;
            if (currentHour > HOURS_IN_HALF_DAY) {
                currentHour = currentHour - HOURS_IN_HALF_DAY;
            }
        } else {
            mIsAm = true;
            if (currentHour == 0) {
                currentHour = HOURS_IN_HALF_DAY;
            }
        }
        updateAmPmControl();
    }
    mHourSpinner->setValue(currentHour);
    if (notifyTimeChanged) {
        onTimeChanged();
    }
}

int TimePickerSpinnerDelegate::getHour() {
    int currentHour = mHourSpinner->getValue();
    if (is24Hour()) {
        return currentHour;
    } else if (mIsAm) {
        return currentHour % HOURS_IN_HALF_DAY;
    } else {
        return (currentHour % HOURS_IN_HALF_DAY) + HOURS_IN_HALF_DAY;
    }
}

void TimePickerSpinnerDelegate::setMinute(int minute) {
    setCurrentMinute(minute, true);
}

void TimePickerSpinnerDelegate::setCurrentMinute(int minute, bool notifyTimeChanged) {
    if (minute == getMinute()) {
        return;
    }
    mMinuteSpinner->setValue(minute);
    if (notifyTimeChanged) {
        onTimeChanged();
    }
}

int TimePickerSpinnerDelegate::getMinute() {
    return mMinuteSpinner->getValue();
}

void TimePickerSpinnerDelegate::setIs24Hour(bool is24Hour) {
    if (mIs24HourView == is24Hour) {
        return;
    }
    // cache the current hour since spinner range changes and BEFORE changing mIs24HourView!!
    int currentHour = getHour();
    mIs24HourView = is24Hour;
    getHourFormatData();
    updateHourControl();
    setCurrentHour(currentHour, false);
    updateMinuteControl();
    updateAmPmControl();
}

bool TimePickerSpinnerDelegate::is24Hour() {
    return mIs24HourView;
}

void TimePickerSpinnerDelegate::setEnabled(bool enabled) {
    mMinuteSpinner->setEnabled(enabled);
    if (mDivider != nullptr) {
        mDivider->setEnabled(enabled);
    }
    mHourSpinner->setEnabled(enabled);
    if (mAmPmSpinner != nullptr) {
        mAmPmSpinner->setEnabled(enabled);
    } else {
        mAmPmButton->setEnabled(enabled);
    }
    mIsEnabled = enabled;
}

bool TimePickerSpinnerDelegate::isEnabled() const{
    return mIsEnabled;
}

int TimePickerSpinnerDelegate::getBaseline() {
    return mHourSpinner->getBaseline();
}

Parcelable* TimePickerSpinnerDelegate::onSaveInstanceState(Parcelable& superState) {
    return new AbstractTimePickerDelegate::SavedState(&superState, getHour(), getMinute(), is24Hour());
}

void TimePickerSpinnerDelegate::onRestoreInstanceState(Parcelable& state) {
    auto* ss = dynamic_cast<AbstractTimePickerDelegate::SavedState*>(&state);
    if (ss) {
        setHour(ss->getHour());
        setMinute(ss->getMinute());
    }
}

bool TimePickerSpinnerDelegate::dispatchPopulateAccessibilityEvent(AccessibilityEvent&) {
    // DEFERRED: accessibility.
    return true;
}

void TimePickerSpinnerDelegate::onPopulateAccessibilityEvent(AccessibilityEvent&) {
    // DEFERRED: DateUtils.formatDateTime not ported.
}

View* TimePickerSpinnerDelegate::getHourView() { return mHourSpinnerInput; }
View* TimePickerSpinnerDelegate::getMinuteView() { return mMinuteSpinnerInput; }
View* TimePickerSpinnerDelegate::getAmView() { return mAmPmSpinnerInput; }
View* TimePickerSpinnerDelegate::getPmView() { return mAmPmSpinnerInput; }

void TimePickerSpinnerDelegate::updateInputState() {
    // DEFERRED: hide the IME when the user changes a value via the spinners.
}

void TimePickerSpinnerDelegate::updateAmPmControl() {
    if (is24Hour()) {
        if (mAmPmSpinner != nullptr) {
            mAmPmSpinner->setVisibility(View::GONE);
        } else {
            mAmPmButton->setVisibility(View::GONE);
        }
    } else {
        int index = mIsAm ? Calendar::AM : Calendar::PM;
        if (mAmPmSpinner != nullptr) {
            mAmPmSpinner->setValue(index);
            mAmPmSpinner->setVisibility(View::VISIBLE);
        } else {
            mAmPmButton->setText(mAmPmStrings[index]);
            mAmPmButton->setVisibility(View::VISIBLE);
        }
    }
}

void TimePickerSpinnerDelegate::onTimeChanged() {
    if (mOnTimeChangedListener) {
        mOnTimeChangedListener(*mDelegator, getHour(), getMinute());
    }
    if (mAutoFillChangeListener) {
        mAutoFillChangeListener(*mDelegator, getHour(), getMinute());
    }
}

void TimePickerSpinnerDelegate::updateHourControl() {
    if (is24Hour()) {
        if (mHourFormat == 'k') {
            mHourSpinner->setMinValue(1);
            mHourSpinner->setMaxValue(24);
        } else {
            mHourSpinner->setMinValue(0);
            mHourSpinner->setMaxValue(23);
        }
    } else {
        if (mHourFormat == 'K') {
            mHourSpinner->setMinValue(0);
            mHourSpinner->setMaxValue(11);
        } else {
            mHourSpinner->setMinValue(1);
            mHourSpinner->setMaxValue(12);
        }
    }
    mHourSpinner->setFormatter(mHourWithTwoDigit ? NumberPicker::getTwoDigitFormatter()
                                                 : NumberPicker::Formatter());
}

void TimePickerSpinnerDelegate::updateMinuteControl() {
    // DEFERRED: IME options (IME_ACTION_DONE/NEXT) on the minute input.
}

void TimePickerSpinnerDelegate::setContentDescriptions() {
    // DEFERRED: accessibility content descriptions (R.string.* not generated).
}

void TimePickerSpinnerDelegate::trySetContentDescription(View*, int, int) {
    // DEFERRED: accessibility content descriptions.
}

}/*endof namespace*/
