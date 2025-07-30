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
# if 0
#include <widget/timepicker.h>
namespace cdroid{
TimePicker::TimePicker(Context* context,const AttributeSet& attrs)
    :FrameLayout(context, attrs, defStyleAttr, defStyleRes){

    // DatePicker is important by default, unless app developer overrode attribute.
    if (getImportantForAutofill() == IMPORTANT_FOR_AUTOFILL_AUTO) {
        setImportantForAutofill(IMPORTANT_FOR_AUTOFILL_YES);
    }

    const bool isDialogMode = a.getBoolean("dialogMode", false);
    const int requestedMode = a.getInt("timePickerMode", MODE_SPINNER);

    if (requestedMode == MODE_CLOCK && isDialogMode) {
        // You want MODE_CLOCK? YOU CAN'T HANDLE MODE_CLOCK! Well, maybe
        // you can depending on your screen size. Let's check...
        mMode = MODE_SPINNER;//context.getResources().getInteger(R.integer.time_picker_mode);
    } else {
        mMode = requestedMode;
    }

    switch (mMode) {
    case MODE_CLOCK:
        mDelegate = new TimePickerClockDelegate(this, context, attrs);
        break;
    case MODE_SPINNER:
    default:
        mDelegate = new TimePickerSpinnerDelegate( this, context, attrs);
        break;
    }
    mDelegate->setAutoFillChangeListener((v, h, m) -> {
        final AutofillManager afm = context.getSystemService(AutofillManager.class);
        if (afm != null) {
            afm.notifyValueChanged(this);
        }
    });
}

int TimePicker::getMode() {
    return mMode;
}

void TimePicker::setHour(int hour) {
    mDelegate->setHour(MathUtils.constrain(hour, 0, 23));
}

int TimePicker::getHour() {
    return mDelegate->getHour();
}

void TimePicker::setMinute(int minute) {
    mDelegate->setMinute(MathUtils.constrain(minute, 0, 59));
}

int TimePicker::getMinute() {
    return mDelegate->getMinute();
}

void TimePicker::setIs24HourView(boolis24HourView) {
    mDelegate->setIs24Hour(is24HourView);
}

bool TimePicker::is24HourView() {
    return mDelegate->is24Hour();
}

void TimePicker::setOnTimeChangedListener(const OnTimeChangedListener& onTimeChangedListener) {
    mDelegate->setOnTimeChangedListener(onTimeChangedListener);
}

void TimePicker::setEnabled(bool enabled) {
    FrameLayout::setEnabled(enabled);
    mDelegate->setEnabled(enabled);
}

bool TimePicker::isEnabled() const{
    return mDelegate->isEnabled();
}

int TimePicker::getBaseline() {
    return mDelegate->getBaseline();
}

bool TimePicker::validateInput() {
    return mDelegate->validateInput();
}

Parcelable* TimePicker::onSaveInstanceState() {
    Parcelable superState = super.onSaveInstanceState();
    return mDelegate.onSaveInstanceState(superState);
}

void TimePicker::onRestoreInstanceState(Parcelable& state) {
    BaseSavedState ss = (BaseSavedState) state;
    FrameLayout::onRestoreInstanceState(ss.getSuperState());
    mDelegate->onRestoreInstanceState(ss);
}

std::string TimePicker::getAccessibilityClassName() {
    return "TimePicker";
}

bool TimePicker::dispatchPopulateAccessibilityEventInternal(AccessibilityEvent& event) {
    return mDelegate->dispatchPopulateAccessibilityEvent(event);
}

View* TimePicker::getHourView() {
    return mDelegate->getHourView();
}

View* TimePicker::getMinuteView() {
    return mDelegate->getMinuteView();
}

View* TimePicker::getAmView() {
    return mDelegate->getAmView();
}

View* TimePicker::getPmView() {
    return mDelegate->getPmView();
}

String[] TimePicker::getAmPmStrings(Context context) {
    final Locale locale = context.getResources().getConfiguration().locale;
    DateFormatSymbols dfs = DateFormat.getIcuDateFormatSymbols(locale);
    String[] amPm = dfs.getAmPmStrings();
    String[] narrowAmPm = dfs.getAmpmNarrowStrings();

    final String[] result = new String[2];
    result[0] = amPm[0].length() > 4 ? narrowAmPm[0] : amPm[0];
    result[1] = amPm[1].length() > 4 ? narrowAmPm[1] : amPm[1];
    return result;
}

/**
 * An abstract class which can be used as a start for TimePicker implementations
 */
class AbstractTimePickerDelegate:public TimePickerDelegate {
    protected final TimePicker mDelegator;
    protected final Context mContext;
    protected final Locale mLocale;

    protected OnTimeChangedListener mOnTimeChangedListener;
    protected OnTimeChangedListener mAutoFillChangeListener;

    // The value that was passed to autofill() - it must be stored because it getAutofillValue()
    // must return the exact same value that was autofilled, otherwise the widget will not be
    // properly highlighted after autofill().
    private long mAutofilledValue;

    public AbstractTimePickerDelegate(TimePicker* delegator, Context* context) {
        mDelegator = delegator;
        mContext = context;
        mLocale = context.getResources().getConfiguration().locale;
    }

    @Override
    public void setOnTimeChangedListener(const OnTimeChangedListener& callback) {
        mOnTimeChangedListener = callback;
    }

    @Override
    public void setAutoFillChangeListener(const OnTimeChangedListener& callback) {
        mAutoFillChangeListener = callback;
    }

    @Override
    public final void autofill(AutofillValue value) {
        if (value == null || !value.isDate()) {
            Log.w(LOG_TAG, value + " could not be autofilled into " + this);
            return;
        }

        final long time = value.getDateValue();

        final Calendar cal = Calendar.getInstance(mLocale);
        cal.setTimeInMillis(time);
        setDate(cal.get(Calendar.HOUR_OF_DAY), cal.get(Calendar.MINUTE));

        // Must set mAutofilledValue *after* calling subclass method to make sure the value
        // returned by getAutofillValue() matches it.
        mAutofilledValue = time;
    }

    @Override
    public final AutofillValue getAutofillValue() {
        if (mAutofilledValue != 0) {
            return AutofillValue.forDate(mAutofilledValue);
        }

        final Calendar cal = Calendar.getInstance(mLocale);
        cal.set(Calendar.HOUR_OF_DAY, getHour());
        cal.set(Calendar.MINUTE, getMinute());
        return AutofillValue.forDate(cal.getTimeInMillis());
    }

    protected void resetAutofilledValue() {
        mAutofilledValue = 0;
    }

    protected static class SavedState extends View.BaseSavedState {
        private final int mHour;
        private final int mMinute;
        private final bool mIs24HourMode;
        private final int mCurrentItemShowing;

        public SavedState(Parcelable superState, int hour, int minute, bool is24HourMode) {
            this(superState, hour, minute, is24HourMode, 0);
        }

        public SavedState(Parcelable superState, int hour, int minute, bool is24HourMode,
                int currentItemShowing) {
            super(superState);
            mHour = hour;
            mMinute = minute;
            mIs24HourMode = is24HourMode;
            mCurrentItemShowing = currentItemShowing;
        }

        private SavedState(Parcel in) {
            super(in);
            mHour = in.readInt();
            mMinute = in.readInt();
            mIs24HourMode = (in.readInt() == 1);
            mCurrentItemShowing = in.readInt();
        }

        public int getHour() {
            return mHour;
        }

        public int getMinute() {
            return mMinute;
        }

        public bool is24HourMode() {
            return mIs24HourMode;
        }

        public int getCurrentItemShowing() {
            return mCurrentItemShowing;
        }

        @Override
        public void writeToParcel(Parcel dest, int flags) {
            super.writeToParcel(dest, flags);
            dest.writeInt(mHour);
            dest.writeInt(mMinute);
            dest.writeInt(mIs24HourMode ? 1 : 0);
            dest.writeInt(mCurrentItemShowing);
        }

        @SuppressWarnings({"unused", "hiding"})
        public static final @android.annotation.NonNull Creator<SavedState> CREATOR = new Creator<SavedState>() {
            public SavedState createFromParcel(Parcel in) {
                return new SavedState(in);
            }

            public SavedState[] newArray(int size) {
                return new SavedState[size];
            }
        };
    }
};

void TimePicker::dispatchProvideAutofillStructure(ViewStructure structure, int flags) {
    // This view is self-sufficient for autofill, so it needs to call
    // onProvideAutoFillStructure() to fill itself, but it does not need to call
    // dispatchProvideAutoFillStructure() to fill its children.
    structure.setAutofillId(getAutofillId());
    onProvideAutofillStructure(structure, flags);
}

void TimePicker::autofill(AutofillValue value) {
    if (!isEnabled()) return;

    mDelegate->autofill(value);
}

int TimePicker::getAutofillType() {
    return isEnabled() ? AUTOFILL_TYPE_DATE : AUTOFILL_TYPE_NONE;
}

AutofillValue TimePicker::getAutofillValue() {
    return isEnabled() ? mDelegate->getAutofillValue() : null;
}
}
#endif

