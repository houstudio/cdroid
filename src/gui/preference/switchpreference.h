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
#ifndef __CDROID_SWITCH_PREFERENCE_H__
#define __CDROID_SWITCH_PREFERENCE_H__

#include <preference/twostatepreference.h>
#include <widget/compoundbutton.h>

namespace cdroid {

class CompoundButton;
class Switch;
class View;

/**
 * Port of androidx.preference.SwitchPreference — a Preference that provides a
 * two-state toggleable option. This preference will save a boolean value to
 * SharedPreferences.
 */
class SwitchPreference : public TwoStatePreference {
public:
    SwitchPreference(Context& context);
    SwitchPreference(Context& context, const AttributeSet& attrs);
    SwitchPreference(Context& context, const AttributeSet& attrs, int defStyleAttr);
    SwitchPreference(Context& context, const AttributeSet& attrs,
            int defStyleAttr, int defStyleRes);

    void onBindViewHolder(PreferenceViewHolder& holder) override;
    void performClick(View* view) override;

    /** Set the text displayed on the switch widget in the on state. */
    void setSwitchTextOn(const std::string& onText);
    void setSwitchTextOn(int resId);
    std::string getSwitchTextOn() const;

    /** Set the text displayed on the switch widget in the off state. */
    void setSwitchTextOff(const std::string& offText);
    void setSwitchTextOff(int resId);
    std::string getSwitchTextOff() const;

    std::string getPreferenceClassName() const override { return "SwitchPreference"; }

private:
    void syncSwitchView(View* view);

    CompoundButton::OnCheckedChangeListener mListener;

    // Switch text for on and off states
    std::string mSwitchOn;
    std::string mSwitchOff;
};

} // namespace cdroid

#endif // __CDROID_SWITCH_PREFERENCE_H__
