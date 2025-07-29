#if 0
#include <widget/timepickerspinnerdelegate.h>
namespace cdroid{

TimePickerSpinnerDelegate::TimePickerSpinnerDelegate(TimePicker* delegator, Context* context,const AttributeSet& attrs)
    :AbstractTimePickerDelegate(delegator, context){

    // process style attributes
    final TypedArray a = mContext.obtainStyledAttributes(
            attrs, R.styleable.TimePicker, defStyleAttr, defStyleRes);
    final int layoutResourceId = a.getString("legacyLayout", R.layout.time_picker_legacy);
    a.recycle();

    final LayoutInflater inflater = LayoutInflater.from(mContext);
    final View view = inflater.inflate(layoutResourceId, mDelegator, true);
    view->setSaveFromParentEnabled(false);

    // hour
    mHourSpinner = delegator.findViewById(R.id.hour);
    mHourSpinner.setOnValueChangedListener(new NumberPicker.OnValueChangeListener() {
        public void onValueChange(NumberPicker spinner, int oldVal, int newVal) {
            updateInputState();
            if (!is24Hour()) {
                if ((oldVal == HOURS_IN_HALF_DAY - 1 && newVal == HOURS_IN_HALF_DAY) ||
                        (oldVal == HOURS_IN_HALF_DAY && newVal == HOURS_IN_HALF_DAY - 1)) {
                    mIsAm = !mIsAm;
                    updateAmPmControl();
                }
            }
            onTimeChanged();
        }
    });
    mHourSpinnerInput = mHourSpinner->findViewById(R::id::numberpicker_input);
    //mHourSpinnerInput->setImeOptions(EditorInfo.IME_ACTION_NEXT);

    // divider (only for the new widget style)
    mDivider = mDelegator->findViewById(R::id::divider);
    if (mDivider != null) {
        setDividerText();
    }

    // minute
    mMinuteSpinner = mDelegator->findViewById(R::id::minute);
    mMinuteSpinner->setMinValue(0);
    mMinuteSpinner->setMaxValue(59);
    mMinuteSpinner->setOnLongPressUpdateInterval(100);
    mMinuteSpinner->setFormatter(NumberPicker.getTwoDigitFormatter());
    mMinuteSpinner->setOnValueChangedListener(new NumberPicker.OnValueChangeListener() {
        public void onValueChange(NumberPicker spinner, int oldVal, int newVal) {
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
        }
    });
    mMinuteSpinnerInput = mMinuteSpinner->findViewById(R::id::numberpicker_input);
    mMinuteSpinnerInput->setImeOptions(EditorInfo.IME_ACTION_NEXT);

    // Get the localized am/pm strings and use them in the spinner.
    mAmPmStrings = TimePicker.getAmPmStrings(context);

    // am/pm
    View* amPmView = mDelegator->findViewById(R::id::amPm);
    if (amPmView instanceof Button) {
        mAmPmSpinner = nullptr;
        mAmPmSpinnerInput = nullptr;
        mAmPmButton = (Button*) amPmView;
        mAmPmButton->setOnClickListener(new View.OnClickListener() {
            public void onClick(View button) {
                button.requestFocus();
                mIsAm = !mIsAm;
                updateAmPmControl();
                onTimeChanged();
            }
        });
    } else {
        mAmPmButton = nullptr;
        mAmPmSpinner = (NumberPicker*) amPmView;
        mAmPmSpinner->setMinValue(0);
        mAmPmSpinner->setMaxValue(1);
        mAmPmSpinner->setDisplayedValues(mAmPmStrings);
        mAmPmSpinner->setOnValueChangedListener(new NumberPicker.OnValueChangeListener() {
            public void onValueChange(NumberPicker picker, int oldVal, int newVal) {
                updateInputState();
                picker.requestFocus();
                mIsAm = !mIsAm;
                updateAmPmControl();
                onTimeChanged();
            }
        });
        mAmPmSpinnerInput = mAmPmSpinner->findViewById(R::id::numberpicker_input);
        mAmPmSpinnerInput->setImeOptions(EditorInfo::IME_ACTION_DONE);
    }

    if (isAmPmAtStart()) {
        // Move the am/pm view to the beginning
        ViewGroup* amPmParent = (ViewGroup*)delegator->findViewById(R::id::timePickerLayout);
        amPmParent->removeView(amPmView);
        amPmParent->addView(amPmView, 0);
        // Swap layout margins if needed. They may be not symmetrical (Old Standard Theme
        // for example and not for Holo Theme)
        ViewGroup::MarginLayoutParams* lp =
                (ViewGroup::MarginLayoutParams*) amPmView->getLayoutParams();
        const int startMargin = lp->getMarginStart();
        const int endMargin = lp->getMarginEnd();
        if (startMargin != endMargin) {
            lp.setMarginStart(endMargin);
            lp.setMarginEnd(startMargin);
        }
    }

    getHourFormatData();

    // update controls to initial state
    updateHourControl();
    updateMinuteControl();
    updateAmPmControl();

    // set to current time
    mTempCalendar = Calendar.getInstance(mLocale);
    setHour(mTempCalendar.get(Calendar.HOUR_OF_DAY));
    setMinute(mTempCalendar.get(Calendar.MINUTE));

    if (!isEnabled()) {
        setEnabled(false);
    }

    // set the content descriptions
    setContentDescriptions();

    // If not explicitly specified this view is important for accessibility.
    if (mDelegator->getImportantForAccessibility() == IMPORTANT_FOR_ACCESSIBILITY_AUTO) {
        mDelegator->setImportantForAccessibility(IMPORTANT_FOR_ACCESSIBILITY_YES);
    }
}

bool TimePickerSpinnerDelegate::validateInput() {
    return true;
}

void TimePickerSpinnerDelegate::getHourFormatData() {
    std::string bestDateTimePattern = DateFormat.getBestDateTimePattern(mLocale,
            (mIs24HourView) ? "Hm" : "hm");
    final int lengthPattern = bestDateTimePattern.length();
    mHourWithTwoDigit = false;
    char hourFormat = '\0';
    // Check if the returned pattern is single or double 'H', 'h', 'K', 'k'. We also save
    // the hour format that we found.
    for (int i = 0; i < lengthPattern; i++) {
        final char c = bestDateTimePattern.charAt(i);
        if (c == 'H' || c == 'h' || c == 'K' || c == 'k') {
            mHourFormat = c;
            if (i + 1 < lengthPattern && c == bestDateTimePattern.charAt(i + 1)) {
                mHourWithTwoDigit = true;
            }
            break;
        }
    }
}

bool TimePickerSpinnerDelegate::isAmPmAtStart() {
    final String bestDateTimePattern = DateFormat.getBestDateTimePattern(mLocale,
            "hm" /* skeleton */);

    return bestDateTimePattern.startsWith("a");
}

void TimePickerSpinnerDelegate::setDividerText() {
    std::string skeleton = (mIs24HourView) ? "Hm" : "hm";
    std::string bestDateTimePattern = DateFormat.getBestDateTimePattern(mLocale,
            skeleton);
    std::string separatorText;
    int hourIndex = bestDateTimePattern.lastIndexOf('H');
    if (hourIndex == -1) {
        hourIndex = bestDateTimePattern.lastIndexOf('h');
    }
    if (hourIndex == -1) {
        // Default case
        separatorText = ":";
    } else {
        int minuteIndex = bestDateTimePattern.indexOf('m', hourIndex + 1);
        if  (minuteIndex == -1) {
            separatorText = Character.toString(bestDateTimePattern.charAt(hourIndex + 1));
        } else {
            separatorText = bestDateTimePattern.substring(hourIndex + 1, minuteIndex);
        }
    }
    mDivider->setText(separatorText);
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
    // why was Integer used in the first place?
    if (currentHour == getHour()) {
        return;
    }
    resetAutofilledValue();
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
    resetAutofilledValue();
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
    // Order is important here.
    mIs24HourView = is24Hour;
    getHourFormatData();
    updateHourControl();
    // set value after spinner range is updated
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

Parcelable* TimePickerSpinnerDelegate::onSaveInstanceState(Parcelable superState) {
    return new SavedState(superState, getHour(), getMinute(), is24Hour());
}

void TimePickerSpinnerDelegate::onRestoreInstanceState(Parcelable& state) {
    if (state instanceof SavedState) {
        final SavedState ss = (SavedState) state;
        setHour(ss.getHour());
        setMinute(ss.getMinute());
    }
}

bool TimePickerSpinnerDelegate::dispatchPopulateAccessibilityEvent(AccessibilityEvent& event) {
    onPopulateAccessibilityEvent(event);
    return true;
}

void TimePickerSpinnerDelegate::onPopulateAccessibilityEvent(AccessibilityEvent& event) {
    int flags = DateUtils.FORMAT_SHOW_TIME;
    if (mIs24HourView) {
        flags |= DateUtils.FORMAT_24HOUR;
    } else {
        flags |= DateUtils.FORMAT_12HOUR;
    }
    mTempCalendar.set(Calendar.HOUR_OF_DAY, getHour());
    mTempCalendar.set(Calendar.MINUTE, getMinute());
    String selectedDateUtterance = DateUtils.formatDateTime(mContext,
            mTempCalendar.getTimeInMillis(), flags);
    event.getText().add(selectedDateUtterance);
}

View* TimePickerSpinnerDelegate::getHourView() {
    return mHourSpinnerInput;
}

View* TimePickerSpinnerDelegate::getMinuteView() {
    return mMinuteSpinnerInput;
}

View* TimePickerSpinnerDelegate::getAmView() {
    return mAmPmSpinnerInput;
}

View* TimePickerSpinnerDelegate::getPmView() {
    return mAmPmSpinnerInput;
}

void TimePickerSpinnerDelegate::updateInputState() {
    // Make sure that if the user changes the value and the IME is active
    // for one of the inputs if this widget, the IME is closed. If the user
    // changed the value via the IME and there is a next input the IME will
    // be shown, otherwise the user chose another means of changing the
    // value and having the IME up makes no sense.
    InputMethodManager inputMethodManager = mContext.getSystemService(InputMethodManager.class);
    if (inputMethodManager != nullptr) {
        if (mHourSpinnerInput->hasFocus()) {
            inputMethodManager->hideSoftInputFromView(mHourSpinnerInput, 0);
            mHourSpinnerInput->clearFocus();
        } else if (mMinuteSpinnerInput->hasFocus()) {
            inputMethodManager->hideSoftInputFromView(mMinuteSpinnerInput, 0);
            mMinuteSpinnerInput->clearFocus();
        } else if (mAmPmSpinnerInput != nullptr && mAmPmSpinnerInput->hasFocus()) {
            inputMethodManager->hideSoftInputFromView(mAmPmSpinnerInput, 0);
            mAmPmSpinnerInput->clearFocus();
        }
    }
}

void TimePickerSpinnerDelegate::updateAmPmControl() {
    if (is24Hour()) {
        if (mAmPmSpinner != nullptr) {
            mAmPmSpinner->setVisibility(View::GONE);
        } else {
            mAmPmButton->setVisibility(View::GONE);
        }
    } else {
        const int index = mIsAm ? Calendar::AM : Calendar::PM;
        if (mAmPmSpinner != nullptr) {
            mAmPmSpinner->setValue(index);
            mAmPmSpinner->setVisibility(View::VISIBLE);
        } else {
            mAmPmButton->setText(mAmPmStrings[index]);
            mAmPmButton->setVisibility(View::VISIBLE);
        }
    }
    mDelegator->sendAccessibilityEvent(AccessibilityEvent::TYPE_VIEW_SELECTED);
}

void TimePickerSpinnerDelegate::onTimeChanged() {
    mDelegator->sendAccessibilityEvent(AccessibilityEvent::TYPE_VIEW_SELECTED);
    if (mOnTimeChangedListener != nullptr) {
        mOnTimeChangedListener.onTimeChanged(mDelegator, getHour(),
                getMinute());
    }
    if (mAutoFillChangeListener != nullptr) {
        mAutoFillChangeListener.onTimeChanged(mDelegator, getHour(), getMinute());
    }
}

void TimePickerSpinnerDelegate::updateHourControl() {
    if (is24Hour()) {
        // 'k' means 1-24 hour
        if (mHourFormat == 'k') {
            mHourSpinner->setMinValue(1);
            mHourSpinner->setMaxValue(24);
        } else {
            mHourSpinner->setMinValue(0);
            mHourSpinner->setMaxValue(23);
        }
    } else {
        // 'K' means 0-11 hour
        if (mHourFormat == 'K') {
            mHourSpinner->setMinValue(0);
            mHourSpinner->setMaxValue(11);
        } else {
            mHourSpinner->setMinValue(1);
            mHourSpinner->setMaxValue(12);
        }
    }
    mHourSpinner->setFormatter(mHourWithTwoDigit ? NumberPicker.getTwoDigitFormatter() : null);
}

void TimePickerSpinnerDelegate::updateMinuteControl() {
    if (is24Hour()) {
        mMinuteSpinnerInput->setImeOptions(EditorInfo.IME_ACTION_DONE);
    } else {
        mMinuteSpinnerInput->setImeOptions(EditorInfo.IME_ACTION_NEXT);
    }
}

void TimePickerSpinnerDelegate::setContentDescriptions() {
    // Minute
    trySetContentDescription(mMinuteSpinner, R.id.increment,
            R.string.time_picker_increment_minute_button);
    trySetContentDescription(mMinuteSpinner, R.id.decrement,
            R.string.time_picker_decrement_minute_button);
    // Hour
    trySetContentDescription(mHourSpinner, R.id.increment,
            R.string.time_picker_increment_hour_button);
    trySetContentDescription(mHourSpinner, R.id.decrement,
            R.string.time_picker_decrement_hour_button);
    // AM/PM
    if (mAmPmSpinner != null) {
        trySetContentDescription(mAmPmSpinner, R.id.increment,
                R.string.time_picker_increment_set_pm_button);
        trySetContentDescription(mAmPmSpinner, R.id.decrement,
                R.string.time_picker_decrement_set_am_button);
    }
}

void TimePickerSpinnerDelegate::trySetContentDescription(View* root, int viewId, int contDescResId) {
    View target = root->findViewById(viewId);
    if (target != nullptr) {
        target.setContentDescription(mContext.getString(contDescResId));
    }
}
}/*endof namespace*/
#endif
