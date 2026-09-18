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
#ifndef __CDROID_MULTISELECT_LIST_PREFERENCE_H__
#define __CDROID_MULTISELECT_LIST_PREFERENCE_H__

#include <vector>
#include <set>
#include <preference/dialogpreference.h>

namespace cdroid {

class Parcel;

/**
 * Port of androidx.preference.MultiSelectListPreference — a Preference that
 * displays a list of entries as a dialog. This preference saves a set of
 * strings.
 */
class MultiSelectListPreference : public DialogPreference {
public:
    MultiSelectListPreference(Context& context);
    MultiSelectListPreference(Context& context, const AttributeSet& attrs);
    MultiSelectListPreference(Context& context, const AttributeSet& attrs, int defStyleAttr);
    MultiSelectListPreference(Context& context, const AttributeSet& attrs,
            int defStyleAttr, int defStyleRes);

    /** Sets the human-readable entries to be shown in the list. */
    void setEntries(const std::vector<std::string>& entries);
    void setEntries(int entriesResId);
    std::vector<std::string> getEntries() const;

    void setEntryValues(const std::vector<std::string>& entryValues);
    void setEntryValues(int entryValuesResId);
    std::vector<std::string> getEntryValues() const;

    /** Sets the values for the key. */
    void setValues(const std::set<std::string>& values);

    /** Retrieves the current values of the key. */
    const std::set<std::string>& getValues() const;

    /** Returns the index of the given value (in the entry values array). */
    int findIndexOfValue(const std::string& value) const;

    bool* getSelectedItems() const;

    std::string getPreferenceClassName() const override { return "MultiSelectListPreference"; }

    class SavedState : public Preference::BaseSavedState {
    public:
        std::set<std::string> mValues;
        SavedState(Parcel& source);
        SavedState(Parcelable* superState);
        void writeToParcel(Parcel& dest, int flags) override;
    };

protected:
    any onGetDefaultValue(const TypedArray& a, int index) override;
    void onSetInitialValue(const any& defaultValue) override;
    Parcelable* onSaveInstanceState() override;
    void onRestoreInstanceState(Parcelable* state) override;

private:
    std::vector<std::string> mEntries;
    std::vector<std::string> mEntryValues;
    std::set<std::string> mValues;
};

} // namespace cdroid

#endif // __CDROID_MULTISELECT_LIST_PREFERENCE_H__
