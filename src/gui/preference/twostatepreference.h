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
#ifndef __CDROID_TWO_STATE_PREFERENCE_H__
#define __CDROID_TWO_STATE_PREFERENCE_H__

#include <preference/preference.h>

namespace cdroid {

class View;
class TextView;
class Parcel;

/**
 * Port of androidx.preference.TwoStatePreference — common base class for
 * preferences that have two selectable states, save a boolean value, and may
 * have dependent preferences that are enabled/disabled based on the current
 * state.
 */
class TwoStatePreference : public Preference {
public:
    TwoStatePreference(Context& context);
    TwoStatePreference(Context& context, const AttributeSet& attrs);
    TwoStatePreference(Context& context, const AttributeSet& attrs, int defStyleAttr);
    TwoStatePreference(Context& context, const AttributeSet& attrs,
            int defStyleAttr, int defStyleRes);

    void onClick() override;

    /** Returns the checked state. */
    bool isChecked() const;

    /** Sets the checked state and saves it. */
    void setChecked(bool checked);

    bool shouldDisableDependents() const override;

    void setSummaryOn(const std::string& summary);
    std::string getSummaryOn() const;
    void setSummaryOn(int summaryResId);

    void setSummaryOff(const std::string& summary);
    std::string getSummaryOff() const;
    void setSummaryOff(int summaryResId);

    bool getDisableDependentsState() const;
    void setDisableDependentsState(bool disableDependentsState);

    any onGetDefaultValue(const TypedArray& a, int index) override;
    void onSetInitialValue(const any& defaultValue) override;

    /**
     * Sync a summary holder contained within holder's sub-hierarchy with the
     * correct summary text.
     */
    void syncSummaryView(PreferenceViewHolder& holder);
    void syncSummaryView(View* view);

    std::string getPreferenceClassName() const override { return "TwoStatePreference"; }

    class SavedState : public Preference::BaseSavedState {
    public:
        bool mChecked;
        SavedState(Parcel& source);
        SavedState(Parcelable* superState);
        void writeToParcel(Parcel& dest, int flags) override;
    };

protected:
    bool mChecked;

    Parcelable* onSaveInstanceState() override;
    void onRestoreInstanceState(Parcelable* state) override;

private:
    std::string mSummaryOn;
    std::string mSummaryOff;
    bool mCheckedSet = false;
    bool mDisableDependentsState = false;
};

} // namespace cdroid

#endif // __CDROID_TWO_STATE_PREFERENCE_H__
