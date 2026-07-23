#ifndef __NUMERIC_TEXT_VIEW_H__
#define __NUMERIC_TEXT_VIEW_H__
#include <widget/textview.h>
namespace cdroid{
class NumericTextView:public TextView {
public:
    DECLARE_UIEVENT(void,OnValueChangedListener,NumericTextView&,int/*value*/, bool/*isValid*/, bool/*isFinished*/);
private:
    static constexpr int RADIX = 10;
    static constexpr double LOG_RADIX = std::log(RADIX);

    int mMinValue = 0;
    int mMaxValue = 99;
    int mMaxCount = 2;
    int mValue;
    int mCount;
    int mPreviousValue;
    OnValueChangedListener mListener;
    bool mShowLeadingZeroes = true;
private:
    void updateDisplayedValue();
    void updateMinimumWidth();
    bool handleKeyUp(int keyCode);
    static bool isKeyCodeNumeric(int keyCode);
    static int numericKeyCodeToInt(int keyCode);
protected:
    void onFocusChanged(bool focused, int direction,Rect* previouslyFocusedRect)override;
public:
    NumericTextView(Context* context,const AttributeSet& attrs);

    /**
     * Sets the currently displayed value.
     * <p>
     * The specified {@code value} must be within the range specified by
     * {@link #setRange(int, int)} (e.g. between {@link #getRangeMinimum()}
     * and {@link #getRangeMaximum()}).
     *
     * @param value the value to display
     */
    void setValue(int value);

    /**
     * Returns the currently displayed value.
     * <p>
     * If the value is currently being edited, returns the live value which may
     * not be within the range specified by {@link #setRange(int, int)}.
     *
     * @return the currently displayed value
     */
    int getValue() const;

    /**
     * Sets the valid range (inclusive).
     *
     * @param minValue the minimum valid value (inclusive)
     * @param maxValue the maximum valid value (inclusive)
     */
    void setRange(int minValue, int maxValue);

    /**
     * @return the minimum value value (inclusive)
     */
    int getRangeMinimum() const;

    /**
     * @return the maximum value value (inclusive)
     */
    int getRangeMaximum() const;

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
    void setShowLeadingZeroes(bool showLeadingZeroes);
    bool getShowLeadingZeroes()const;

    void setOnDigitEnteredListener(const OnValueChangedListener& listener);
    OnValueChangedListener getOnDigitEnteredListener() const;

    bool onKeyDown(int keyCode, KeyEvent& event)override;
    bool onKeyMultiple(int keyCode, int repeatCount, KeyEvent& event)override;
    bool onKeyUp(int keyCode, KeyEvent& event)override;
};
}/*endof namespace*/
#endif/*__NUMERIC_TEXT_VIEW_H__*/
