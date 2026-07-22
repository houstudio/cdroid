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
#include <iomanip>
#include <text/textwatcher.h>
#include <text/editable.h>
#include <widget/R.h>
#include <utils/textutils.h>
#include <utils/mathutils.h>
#include <widget/timepicker.h>
#include <widget/timepickerclockdelegate.h>
#include <widget/textinputtimepickerview.h>
namespace cdroid {

DECLARE_WIDGET(TextInputTimePickerView);
TextInputTimePickerView::TextInputTimePickerView(Context* context,const AttributeSet& attrs)
    :RelativeLayout(context, attrs){

    LayoutInflater::from(context)->inflate("cdroid:layout/time_picker_text_input_material", this);

    mHourEditText = (EditText*)findViewById(R::id::input_hour);
    mMinuteEditText = (EditText*)findViewById(R::id::input_minute);
    mInputSeparatorView = (TextView*)findViewById(R::id::input_separator);
    mErrorLabel = (TextView*)findViewById(R::id::label_error);
    mHourLabel = (TextView*)findViewById(R::id::label_hour);
    mMinuteLabel = (TextView*)findViewById(R::id::label_minute);

    // Wire text input -> parseAndSet*(Editable.toUTF8()), matching AOSP's two
    // TextWatchers (TextInputTimePickerView.java). The hour watcher hands focus
    // to the minute field once a valid >1-char hour is entered; AOSP gates that
    // on AccessibilityManager.isEnabled() — CDROID has no AccessibilityManager
    // wired, so the focus handoff is unconditional (a11y refinement deferred).
    TextWatcher hourWatcher;
    hourWatcher.afterTextChanged = [this](Editable& editable) {
        if (parseAndSetHourInternal(editable.toUTF8()) && editable.length() > 1) {
            mMinuteEditText->requestFocus();
        }
    };
    mHourEditText->addTextChangedListener(hourWatcher);

    TextWatcher minuteWatcher;
    minuteWatcher.afterTextChanged = [this](Editable& editable) {
        parseAndSetMinuteInternal(editable.toUTF8());
    };
    mMinuteEditText->addTextChangedListener(minuteWatcher);
    mAmPmSpinner = (Spinner*)findViewById(R::id::am_pm_spinner);
    std::vector<std::string> amPmStrings = TimePicker::getAmPmStrings(context);
    ArrayAdapter<std::string>* adapter = new ArrayAdapter<std::string>(context, "@cdroid:layout/simple_spinner_dropdown_item",0);
    adapter->add(TimePickerClockDelegate::obtainVerbatim(amPmStrings[0]));
    adapter->add(TimePickerClockDelegate::obtainVerbatim(amPmStrings[1]));
    mAmPmSpinner->setAdapter(adapter);
    mAdapter = adapter;
    AdapterView::OnItemSelectedListener sl;
    sl.onItemSelected=[this](AdapterView& adapterView, View& view, int position, long id){
        if (position == 0) {
            mListener/*.onValueChanged*/(AMPM, AM);
        } else {
            mListener/*.onValueChanged*/(AMPM, PM);
        }
    };
    mAmPmSpinner->setOnItemSelectedListener(sl);
}

TextInputTimePickerView::~TextInputTimePickerView(){
    delete mAdapter;
}

void TextInputTimePickerView::setListener(const OnValueTypedListener& listener) {
    mListener = listener;
}

void TextInputTimePickerView::setHourFormat(int maxCharLength) {
    mHourEditText->setFilters({ new InputFilter::LengthFilter(maxCharLength)});
    mMinuteEditText->setFilters({ new InputFilter::LengthFilter(maxCharLength)});
    /*final LocaleList locales = mContext.getResources().getConfiguration().getLocales();
    mHourEditText->setImeHintLocales(locales);
    mMinuteEditText->setImeHintLocales(locales);*/
}

bool TextInputTimePickerView::validateInput() {
    auto hint = mHourEditText->getHint();
    std::string hourText = mHourEditText->getText();
    if(TextUtils::isEmpty(hourText)&&hint){
        hourText = *hint;
    }
    std::string minuteText = mMinuteEditText->getText();
    hint = mMinuteEditText->getHint();
    if(TextUtils::isEmpty(minuteText)&&hint){
        minuteText = *hint;
    }
    const bool inputValid = parseAndSetHourInternal(hourText)&& parseAndSetMinuteInternal(minuteText);
    setError(!inputValid);
    return inputValid;
}

void TextInputTimePickerView::updateSeparator(const std::string& separatorText) {
    mInputSeparatorView->setText(separatorText);
}

void TextInputTimePickerView::setError(bool enabled) {
    mErrorShowing = enabled;
    mErrorLabel->setVisibility(enabled ? View::VISIBLE : View::INVISIBLE);
    mHourLabel->setVisibility(enabled ? View::INVISIBLE : View::VISIBLE);
    mMinuteLabel->setVisibility(enabled ? View::INVISIBLE : View::VISIBLE);
}

void TextInputTimePickerView::setTimeSet(bool timeSet) {
    mTimeSet = mTimeSet || timeSet;
}

bool TextInputTimePickerView::isTimeSet() const{
    return mTimeSet;
}

static std::string formatNumber(int mValue, int mCount) {
    std::ostringstream oss;
    if(mCount)
        oss << std::setw(mCount) << std::setfill('0') << mValue;
    else
        oss<<mValue;
    return oss.str();
}
void TextInputTimePickerView::updateTextInputValues(int localizedHour, int minute, int amOrPm, bool is24Hour,
        bool hourFormatStartsAtZero) {

    mIs24Hour = is24Hour;
    mHourFormatStartsAtZero = hourFormatStartsAtZero;

    mAmPmSpinner->setVisibility(is24Hour ? View::INVISIBLE : View::VISIBLE);

    if (amOrPm == AM) {
        mAmPmSpinner->setSelection(0);
    } else {
        mAmPmSpinner->setSelection(1);
    }

    if (isTimeSet()) {
        mHourEditText->setText(formatNumber(localizedHour,0));
        mMinuteEditText->setText(formatNumber(minute,2));
    } else {
        mHourEditText->setHint(formatNumber(localizedHour,0));
        mMinuteEditText->setHint(formatNumber(minute,2));
    }

    if (mErrorShowing) {
        validateInput();
    }
}

bool TextInputTimePickerView::parseAndSetHourInternal(const std::string& input) {
    try {
        const int hour = std::stoi(input);
        if (!isValidLocalizedHour(hour)) {
            const int minHour = mHourFormatStartsAtZero ? 0 : 1;
            const int maxHour = mIs24Hour ? 23 : 11 + minHour;
            mListener/*.onValueChanged*/(HOURS, getHourOfDayFromLocalizedHour(
                    MathUtils::constrain(hour, minHour, maxHour)));
            return false;
        }
        mListener/*.onValueChanged*/(HOURS, getHourOfDayFromLocalizedHour(hour));
        setTimeSet(true);
        return true;
    } catch (std::logic_error& e) {
        // std::stoi throws invalid_argument (non-numeric) or out_of_range
        // (overflow); both mean "unparseable input" — AOSP catches NumberFormatException.
        return false;
    }
}

bool TextInputTimePickerView::parseAndSetMinuteInternal(const std::string& input) {
    try {
        const int minutes = std::stoi(input);//Integer.parseInt(input);
        if (minutes < 0 || minutes > 59) {
            mListener/*.onValueChanged*/(MINUTES, MathUtils::constrain(minutes, 0, 59));
            return false;
        }
        mListener/*.onValueChanged*/(MINUTES, minutes);
        setTimeSet(true);
        return true;
    } catch (std::logic_error& e) {
        // std::stoi throws invalid_argument (non-numeric) or out_of_range
        // (overflow); both mean "unparseable input" — AOSP catches NumberFormatException.
        return false;
    }
}

bool TextInputTimePickerView::isValidLocalizedHour(int localizedHour) const{
    const int minHour = mHourFormatStartsAtZero ? 0 : 1;
    const int maxHour = (mIs24Hour ? 23 : 11) + minHour;
    return localizedHour >= minHour && localizedHour <= maxHour;
}

int TextInputTimePickerView::getHourOfDayFromLocalizedHour(int localizedHour) {
    int hourOfDay = localizedHour;
    if (mIs24Hour) {
        if (!mHourFormatStartsAtZero && localizedHour == 24) {
            hourOfDay = 0;
        }
    } else {
        if (!mHourFormatStartsAtZero && localizedHour == 12) {
            hourOfDay = 0;
        }
        if (mAmPmSpinner->getSelectedItemPosition() == 1) {
            hourOfDay += 12;
        }
    }
    return hourOfDay;
}
}/*endof namespace*/
