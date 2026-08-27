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
#ifndef __CDROID_CHECKBOX_PREFERENCE_H__
#define __CDROID_CHECKBOX_PREFERENCE_H__

#include <preference/twostatepreference.h>
#include <widget/compoundbutton.h>

namespace cdroid {

class CompoundButton;
class View;

/**
 * Port of androidx.preference.CheckBoxPreference — a Preference that provides
 * checkbox widget functionality. This preference saves a boolean value.
 */
class CheckBoxPreference : public TwoStatePreference {
public:
    CheckBoxPreference(Context& context);
    CheckBoxPreference(Context& context, const AttributeSet& attrs);
    CheckBoxPreference(Context& context, const AttributeSet& attrs, int defStyleAttr);
    CheckBoxPreference(Context& context, const AttributeSet& attrs,
            int defStyleAttr, int defStyleRes);

    void onBindViewHolder(PreferenceViewHolder& holder) override;
    void performClick(View* view) override;

    std::string getPreferenceClassName() const override { return "CheckBoxPreference"; }

private:
    void syncCheckboxView(View* view);

    CompoundButton::OnCheckedChangeListener mListener;
};

} // namespace cdroid

#endif // __CDROID_CHECKBOX_PREFERENCE_H__
