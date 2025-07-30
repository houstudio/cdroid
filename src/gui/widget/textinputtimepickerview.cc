#include <iomanip>
#include <widget/R.h>
#include <core/textutils.h>
#include <core/mathutils.h>
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

    /*mHourEditText->addTextChangedListener(new TextWatcher() {
        @Override
        public void beforeTextChanged(CharSequence charSequence, int i, int i1, int i2) {}

        @Override
        public void onTextChanged(CharSequence charSequence, int i, int i1, int i2) {}

        @Override
        public void afterTextChanged(Editable editable) {
            if (parseAndSetHourInternal(editable.toString()) && editable.length() > 1) {
                AccessibilityManager am = (AccessibilityManager) context.getSystemService(
                        context.ACCESSIBILITY_SERVICE);
                if (!am.isEnabled()) {
                    mMinuteEditText.requestFocus();
                }
            }
        }
    });

    mMinuteEditText->addTextChangedListener(new TextWatcher() {
        @Override
        public void beforeTextChanged(CharSequence charSequence, int i, int i1, int i2) {}

        @Override
        public void onTextChanged(CharSequence charSequence, int i, int i1, int i2) {}

        @Override
        public void afterTextChanged(Editable editable) {
            parseAndSetMinuteInternal(editable.toString());
        }
    });

    mAmPmSpinner = (Spinner*)findViewById(R::id::am_pm_spinner);
    final String[] amPmStrings = TimePicker.getAmPmStrings(context);
    ArrayAdapter<CharSequence> adapter = new ArrayAdapter<CharSequence>(context, R.layout.simple_spinner_dropdown_item);
    adapter->add(TimePickerClockDelegate::obtainVerbatim(amPmStrings[0]));
    adapter->add(TimePickerClockDelegate::obtainVerbatim(amPmStrings[1]));
    mAmPmSpinner->setAdapter(adapter);
    mAmPmSpinner->setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
        @Override
        public void onItemSelected(AdapterView& adapterView, View& view, int position, long id) {
            if (position == 0) {
                mListener.onValueChanged(AMPM, AM);
            } else {
                mListener.onValueChanged(AMPM, PM);
            }
        }

        @Override
        public void onNothingSelected(AdapterView& adapterView) {}
    });*/
}

void TextInputTimePickerView::setListener(const OnValueTypedListener& listener) {
    mListener = listener;
}

void TextInputTimePickerView::setHourFormat(int maxCharLength) {
    /*mHourEditText->setFilters(new InputFilter[] { new InputFilter.LengthFilter(maxCharLength)});
    mMinuteEditText->setFilters(new InputFilter[] { new InputFilter.LengthFilter(maxCharLength)});
    final LocaleList locales = mContext.getResources().getConfiguration().getLocales();
    mHourEditText->setImeHintLocales(locales);
    mMinuteEditText->setImeHintLocales(locales);*/
}

bool TextInputTimePickerView::validateInput() {
    const std::string hourText = TextUtils::isEmpty(mHourEditText->getText())
            ? mHourEditText->getHint() : mHourEditText->getText();
    const std::string minuteText = TextUtils::isEmpty(mMinuteEditText->getText())
            ? mMinuteEditText->getHint() : mMinuteEditText->getText();

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
    } catch (std::invalid_argument& e) {
        // Do nothing since we cannot parse the input.
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
    } catch (std::invalid_argument& e) {
        // Do nothing since we cannot parse the input.
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
