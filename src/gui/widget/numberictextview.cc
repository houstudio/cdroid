#include <iomanip>
#include <widget/numberictextview.h>
namespace cdroid{

DECLARE_WIDGET(NumericTextView);
NumericTextView::NumericTextView(Context* context,const AttributeSet& attrs)
    :TextView(context, attrs){
    // Generate the hint text color based on disabled state.
    const int textColorDisabled = getTextColors()->getColorForState(StateSet::get(0), 0);
    setHintTextColor(textColorDisabled);
    setFocusable(true);
}

void NumericTextView::onFocusChanged(bool focused, int direction,Rect* previouslyFocusedRect) {
    TextView::onFocusChanged(focused, direction, previouslyFocusedRect);

    if (focused) {
        mPreviousValue = mValue;
        mValue = 0;
        mCount = 0;
        // Transfer current text to hint.
        setHint(getText());
        setText("");
    } else {
        if (mCount == 0) {
            // No digits were entered, revert to previous value.
            mValue = mPreviousValue;
            setText(getHint());
            setHint("");
        }
        // Ensure the committed value is within range.
        if (mValue < mMinValue) {
            mValue = mMinValue;
        }
        setValue(mValue);
        if (mListener != nullptr) {
            mListener/*.onValueChanged*/(*this, mValue, true, true);
        }
    }
}

void NumericTextView::setValue(int value) {
    if (mValue != value) {
        mValue = value;
        updateDisplayedValue();
    }
}

int NumericTextView::getValue() const{
    return mValue;
}

void NumericTextView::setRange(int minValue, int maxValue) {
    if (mMinValue != minValue) {
        mMinValue = minValue;
    }

    if (mMaxValue != maxValue) {
        mMaxValue = maxValue;
        mMaxCount = 1 + (int) (std::log(maxValue) / LOG_RADIX);

        updateMinimumWidth();
        updateDisplayedValue();
    }
}

int NumericTextView::getRangeMinimum() const{
    return mMinValue;
}

int NumericTextView::getRangeMaximum() const{
    return mMaxValue;
}

/**
 * Sets whether this view shows leading zeroes.
 * <p>
 * When leading zeroes are shown, the displayed value will be padded
 * with zeroes to the width of the maximum value as specified by
 * {@link #setRange(int, int)} (see also {@link #getRangeMaximum()}.
 * <p>
 * For example, with leading zeroes shown, a maximum of 99 and value of
 * 9 would display "09". A maximum of 100 and a value of 9 would display
 * "009". With leading zeroes hidden, both cases would show "9".
 *
 * @param showLeadingZeroes {@code true} to show leading zeroes,
 *                          {@code false} to hide them
 */
void NumericTextView::setShowLeadingZeroes(bool showLeadingZeroes) {
    if (mShowLeadingZeroes != showLeadingZeroes) {
        mShowLeadingZeroes = showLeadingZeroes;
        updateDisplayedValue();
    }
}

bool NumericTextView::getShowLeadingZeroes() const{
    return mShowLeadingZeroes;
}

static std::string formatNumber(int mValue, int mCount) {
    std::ostringstream oss;
    if(mCount)
        oss << std::setw(mCount) << std::setfill('0') << mValue;
    else
        oss<<mValue;
    return oss.str();
}

void NumericTextView::updateDisplayedValue() {
    // Always use String.format() rather than Integer.toString()
    // to obtain correctly localized values.
    setText(formatNumber(mValue,mShowLeadingZeroes?mMaxCount:0));
}

void NumericTextView::updateMinimumWidth() {
    const std::string previousText = getText();
    int maxWidth = 0;

    for (int i = 0; i < mMaxValue; i++) {
        setText(formatNumber(i,mMaxCount));//String.format("%0" + mMaxCount + "d", i));
        measure(MeasureSpec::UNSPECIFIED, MeasureSpec::UNSPECIFIED);

        const int width = getMeasuredWidth();
        if (width > maxWidth) {
            maxWidth = width;
        }
    }
    setText(previousText);
    setMinWidth(maxWidth);
    setMinimumWidth(maxWidth);
}

void NumericTextView::setOnDigitEnteredListener(const OnValueChangedListener& listener) {
    mListener = listener;
}

NumericTextView::OnValueChangedListener NumericTextView::getOnDigitEnteredListener() const{
    return mListener;
}

bool NumericTextView::onKeyDown(int keyCode, KeyEvent& event) {
    return isKeyCodeNumeric(keyCode) || (keyCode == KeyEvent::KEYCODE_DEL)
            || TextView::onKeyDown(keyCode, event);
}

bool NumericTextView::onKeyMultiple(int keyCode, int repeatCount, KeyEvent& event) {
    return isKeyCodeNumeric(keyCode) || (keyCode == KeyEvent::KEYCODE_DEL)
            || TextView::onKeyMultiple(keyCode, repeatCount, event);
}

bool NumericTextView::onKeyUp(int keyCode, KeyEvent& event) {
    return handleKeyUp(keyCode) || TextView::onKeyUp(keyCode, event);
}

bool NumericTextView::handleKeyUp(int keyCode) {
    if (keyCode == KeyEvent::KEYCODE_DEL) {
        // Backspace removes the least-significant digit, if available.
        if (mCount > 0) {
            mValue /= RADIX;
            mCount--;
        }
    } else if (isKeyCodeNumeric(keyCode)) {
        if (mCount < mMaxCount) {
            const int keyValue = numericKeyCodeToInt(keyCode);
            const int newValue = mValue * RADIX + keyValue;
            if (newValue <= mMaxValue) {
                mValue = newValue;
                mCount++;
            }
        }
    } else {
        return false;
    }
    std::string formattedValue;
    if (mCount > 0) {
        // If the user types 01, we should always show the leading 0 even if
        // getShowLeadingZeroes() is false. Preserve typed leading zeroes by
        // using the number of digits entered as the format width.
        formattedValue = formatNumber(mValue,mCount);
    } else {
        formattedValue = "";
    }
    setText(formattedValue);
    if (mListener != nullptr) {
        const bool isValid = mValue >= mMinValue;
        const bool isFinished = mCount >= mMaxCount || mValue * RADIX > mMaxValue;
        mListener/*.onValueChanged*/(*this, mValue, isValid, isFinished);
    }
    return true;
}

bool NumericTextView::isKeyCodeNumeric(int keyCode) {
    return keyCode == KeyEvent::KEYCODE_0 || keyCode == KeyEvent::KEYCODE_1
            || keyCode == KeyEvent::KEYCODE_2 || keyCode == KeyEvent::KEYCODE_3
            || keyCode == KeyEvent::KEYCODE_4 || keyCode == KeyEvent::KEYCODE_5
            || keyCode == KeyEvent::KEYCODE_6 || keyCode == KeyEvent::KEYCODE_7
            || keyCode == KeyEvent::KEYCODE_8 || keyCode == KeyEvent::KEYCODE_9;
}

int NumericTextView::numericKeyCodeToInt(int keyCode) {
    return keyCode - KeyEvent::KEYCODE_0;
}
}
