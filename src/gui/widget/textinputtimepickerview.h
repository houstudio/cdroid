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
#ifndef __TEXTINPUT_TIMEPICKER_VIEW_H__
#define __TEXTINPUT_TIMEPICKER_VIEW_H__

#include <widget/spinner.h>
#include <widget/edittext.h>
#include <widget/relativelayout.h>

namespace cdroid{
class TextInputTimePickerView:public RelativeLayout {
public:
    static constexpr int HOURS = 0;
    static constexpr int MINUTES = 1;
    static constexpr int AMPM = 2;
    DECLARE_UIEVENT(void,OnValueTypedListener,int/*inputType*/,int/*newValue*/);
private:
    static constexpr int AM = 0;
    static constexpr int PM = 1;

    EditText* mHourEditText;
    EditText* mMinuteEditText;
    TextView* mInputSeparatorView;
    Spinner* mAmPmSpinner;
    TextView* mErrorLabel;
    TextView* mHourLabel;
    TextView* mMinuteLabel;
    Adapter* mAdapter;
    bool mIs24Hour;
    bool mHourFormatStartsAtZero;
    bool mErrorShowing;
    bool mTimeSet;
    OnValueTypedListener mListener;

private:
    void setError(bool enabled);

    void setTimeSet(bool timeSet);
    bool isTimeSet()const;
    bool parseAndSetHourInternal(const std::string& input);
    bool parseAndSetMinuteInternal(const std::string& input);
    bool isValidLocalizedHour(int localizedHour)const;
    int getHourOfDayFromLocalizedHour(int localizedHour);
public:
    TextInputTimePickerView(Context* context,const AttributeSet& attrs);
    ~TextInputTimePickerView()override;
    void setListener(const OnValueTypedListener& listener);

    void setHourFormat(int maxCharLength);

    bool validateInput();

    void updateSeparator(const std::string& separatorText);

    /**
     * Computes the display value and updates the text of the view.
     * <p>
     * This method should be called whenever the current value or display
     * properties (leading zeroes, max digits) change.
     */
    void updateTextInputValues(int localizedHour, int minute, int amOrPm, bool is24Hour,
            bool hourFormatStartsAtZero);
};
}/*endof namespace*/
#endif/*__TEXTINPUT_TIMEPICKER_VIEW_H__*/
