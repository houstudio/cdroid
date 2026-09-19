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
#include <widget/internal_R.h>
#include <utils/mathutils.h>
#include <widget/timepicker.h>
#include <widget/timepickerclockdelegate.h>
#include <widget/timepickerspinnerdelegate.h>
#include <widget/framework_styleable.h>
#include <content/typedarray.h>
#include <content/dateformatsymbols.h>
#include <text/textutils.h>
namespace cdroid{
using namespace cdroid::internal;
TimePicker::TimePicker(Context*ctx)
    :TimePicker(ctx,nullptr){}

TimePicker::TimePicker(Context* context,const AttributeSet* attrs):TimePicker(context,attrs,cdroid::internal::R::attr::timePickerStyle){}

TimePicker::TimePicker(Context* context,const AttributeSet* pAttrs,int defStyleAttr)
    :TimePicker(context,pAttrs,defStyleAttr,0){}

TimePicker::TimePicker(Context* context,const AttributeSet* pAttrs,int defStyleAttr,int defStyleRes)
    :FrameLayout(context, pAttrs, defStyleAttr, defStyleRes){

    // DatePicker is important by default, unless app developer overrode attribute.
    if (getImportantForAutofill() == IMPORTANT_FOR_AUTOFILL_AUTO) {
        setImportantForAutofill(IMPORTANT_FOR_AUTOFILL_YES);
    }

    auto a = context->obtainStyledAttributes(pAttrs, R::styleable::TimePicker, defStyleAttr, defStyleRes);
    const bool isDialogMode = a->getBoolean(R::styleable::TimePicker_dialogMode, false);
    const int requestedMode = a->getInt(R::styleable::TimePicker_timePickerMode, MODE_SPINNER);

    if (requestedMode == MODE_CLOCK && isDialogMode) {
        // You want MODE_CLOCK? YOU CAN'T HANDLE MODE_CLOCK! Well, maybe
        // you can depending on your screen size. Let's check...
        mMode = context->getInteger(R::integer::time_picker_mode);
    } else {
        mMode = requestedMode;
    }

    switch (mMode) {
    case MODE_CLOCK:
        mDelegate = new TimePickerClockDelegate(this, context, pAttrs, defStyleAttr, defStyleRes);
        break;
    case MODE_SPINNER:
    default:
        mDelegate = new TimePickerSpinnerDelegate(this, context, pAttrs, defStyleAttr, defStyleRes);
        break;
    }
    /*mDelegate->setAutoFillChangeListener((v, h, m) -> {
        final AutofillManager afm = context.getSystemService(AutofillManager.class);
        if (afm != null) {
            afm.notifyValueChanged(this);
        }
    });*/
}

void TimePicker::onConfigurationChanged(Configuration& newConfig) {
    FrameLayout::onConfigurationChanged(newConfig);
    // CDROID runtime-locale extension: re-localize the delegate in place
    // (AOSP android-36 rebuilds the activity instead).
    mDelegate->onConfigurationChanged(newConfig);
}

TimePicker::~TimePicker(){
    delete mDelegate;
}

int TimePicker::getMode() const{
    return mMode;
}

void TimePicker::setHour(int hour) {
    mDelegate->setHour(MathUtils::constrain(hour, 0, 23));
}

int TimePicker::getHour() {
    return mDelegate->getHour();
}

void TimePicker::setMinute(int minute) {
    mDelegate->setMinute(MathUtils::constrain(minute, 0, 59));
}

int TimePicker::getMinute() {
    return mDelegate->getMinute();
}

void TimePicker::setIs24HourView(bool is24HourView) {
    mDelegate->setIs24Hour(is24HourView);
}

bool TimePicker::is24HourView() const{
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
    Parcelable* superState = FrameLayout::onSaveInstanceState();
    return mDelegate->onSaveInstanceState(*superState);
}

void TimePicker::onRestoreInstanceState(Parcelable& state) {
    BaseSavedState& ss = (BaseSavedState&) state;
    FrameLayout::onRestoreInstanceState(*ss.getSuperState());
    mDelegate->onRestoreInstanceState(ss);
}

std::string TimePicker::getAccessibilityClassName() const{
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

std::vector<std::string> TimePicker::getAmPmStrings(Context* context) {
    // AOSP: DateFormat.getIcuDateFormatSymbols(locale), then pick wide vs narrow
    // by the marker's UTF-16 length (Java String.length(); "上午" is 2 units,
    // so it stays whole). The symbols object must outlive the references
    // (getters return refs into it; binding to a temporary would dangle).
    const Locale locale = context->getResources().getConfiguration().getLocales().get(0);
    const DateFormatSymbols dfs(locale);
    const auto& amPm = dfs.getAmPmStrings();
    const auto& narrowAmPm = dfs.getAmpmNarrowStrings();

    auto utf16Length = [](const std::string& s) {
        return TextUtils::utf8_utf16(s).length();
    };

    std::vector<std::string> result;
    result.push_back(utf16Length(amPm[0]) > 4 ? narrowAmPm[0] : amPm[0]);
    result.push_back(utf16Length(amPm[1]) > 4 ? narrowAmPm[1] : amPm[1]);
    return result;
}

/**
 * An abstract class which can be used as a start for TimePicker implementations
 */
TimePicker::AbstractTimePickerDelegate::AbstractTimePickerDelegate(TimePicker* delegator, Context* context) {
    mDelegator = delegator;
    mContext = context;
    mLocale = context->getResources().getConfiguration().getLocales().get(0);
}

void TimePicker::AbstractTimePickerDelegate::setOnTimeChangedListener(const OnTimeChangedListener& callback) {
    mOnTimeChangedListener = callback;
}
#if 0
void TimePicker::AbstractTimePickerDelegate::setAutoFillChangeListener(const OnTimeChangedListener& callback) {
    mAutoFillChangeListener = callback;
}

void TimePicker::AbstractTimePickerDelegate::autofill(AutofillValue value) {
    if (value == null || !value.isDate()) {
        LOGW("%d could not be autofilled into ",value,this);
        return;
    }

    final long time = value.getDateValue();

    final Calendar cal = Calendar.getInstance(mLocale);
    cal.setTimeInMillis(time);
    setDate(cal.get(Calendar::HOUR_OF_DAY), cal.get(Calendar::MINUTE));

    // Must set mAutofilledValue *after* calling subclass method to make sure the value
    // returned by getAutofillValue() matches it.
    mAutofilledValue = time;
}

AutofillValue TimePicker::AbstractTimePickerDelegate::getAutofillValue() {
    if (mAutofilledValue != 0) {
        return AutofillValue.forDate(mAutofilledValue);
    }

    final Calendar cal = Calendar.getInstance(mLocale);
    cal.set(Calendar::HOUR_OF_DAY, getHour());
    cal.set(Calendar::MINUTE, getMinute());
    return AutofillValue.forDate(cal.getTimeInMillis());
}

void TimePicker::AbstractTimePickerDelegate::resetAutofilledValue() {
    mAutofilledValue = 0;
}
#endif

TimePicker::AbstractTimePickerDelegate::SavedState::SavedState(Parcelable* superState, int hour, int minute, bool is24HourMode)
    :SavedState(superState, hour, minute, is24HourMode, 0){
}

TimePicker::AbstractTimePickerDelegate::SavedState::SavedState(Parcelable* superState, int hour, int minute, bool is24HourMode,
        int currentItemShowing):BaseSavedState(superState){
    mHour = hour;
    mMinute = minute;
    mIs24HourMode = is24HourMode;
    mCurrentItemShowing = currentItemShowing;
}

TimePicker::AbstractTimePickerDelegate::SavedState::SavedState(Parcel& in)
    :BaseSavedState(in){
    mHour = in.readInt();
    mMinute = in.readInt();
    mIs24HourMode = (in.readInt() == 1);
    mCurrentItemShowing = in.readInt();
}

int TimePicker::AbstractTimePickerDelegate::SavedState::getHour() const{
    return mHour;
}

int TimePicker::AbstractTimePickerDelegate::SavedState::getMinute() const{
    return mMinute;
}

bool TimePicker::AbstractTimePickerDelegate::SavedState::is24HourMode() const{
    return mIs24HourMode;
}

int TimePicker::AbstractTimePickerDelegate::SavedState::getCurrentItemShowing() const{
    return mCurrentItemShowing;
}

void TimePicker::AbstractTimePickerDelegate::SavedState::writeToParcel(Parcel& dest, int flags) {
    BaseSavedState::writeToParcel(dest, flags);
    dest.writeInt(mHour);
    dest.writeInt(mMinute);
    dest.writeInt(mIs24HourMode ? 1 : 0);
    dest.writeInt(mCurrentItemShowing);
}
#if 0
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
#endif
DECLARE_WIDGET2(TimePicker, "android.widget.TimePicker");
}/*endof namespace*/

