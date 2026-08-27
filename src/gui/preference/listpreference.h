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
#ifndef __CDROID_LIST_PREFERENCE_H__
#define __CDROID_LIST_PREFERENCE_H__

#include <vector>
#include <preference/dialogpreference.h>

namespace cdroid {

class Parcel;

/**
 * Port of androidx.preference.ListPreference — a Preference that displays a
 * list of entries as a dialog. This preference saves a string value.
 */
class ListPreference : public DialogPreference {
public:
    /**
     * A simple SummaryProvider implementation for a ListPreference. If no
     * value has been set, the summary displayed will be 'Not set', otherwise
     * the summary displayed will be the entry set for this preference.
     */
    class SimpleSummaryProvider : public Preference::SummaryProvider {
    public:
        static SimpleSummaryProvider* getInstance();
        std::string provideSummary(Preference& preference) override;
    private:
        SimpleSummaryProvider() = default;
        static SimpleSummaryProvider* sSimpleSummaryProvider;
    };

    ListPreference(Context& context);
    ListPreference(Context& context, const AttributeSet& attrs);
    ListPreference(Context& context, const AttributeSet& attrs, int defStyleAttr);
    ListPreference(Context& context, const AttributeSet& attrs,
            int defStyleAttr, int defStyleRes);

    /** Sets the human-readable entries to be shown in the list. */
    void setEntries(const std::vector<std::string>& entries);
    void setEntries(int entriesResId);
    std::vector<std::string> getEntries() const;

    /**
     * The array to find the value to save for a preference when an entry from
     * entries is selected.
     */
    void setEntryValues(const std::vector<std::string>& entryValues);
    void setEntryValues(int entryValuesResId);
    std::vector<std::string> getEntryValues() const;

    void setSummary(const std::string& summary) override;
    std::string getSummary() const override;

    /** Sets the value of the key. */
    void setValue(const std::string& value);
    std::string getValue() const;

    /** Returns the entry corresponding to the current value. */
    std::string getEntry() const;

    /** Returns the index of the given value (in the entry values array). */
    int findIndexOfValue(const std::string& value) const;

    /** Sets the value to the given index from the entry values. */
    void setValueIndex(int index);

    std::string getPreferenceClassName() const override { return "ListPreference"; }

    class SavedState : public Preference::BaseSavedState {
    public:
        std::string mValue;
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
    int getValueIndex() const;

    std::vector<std::string> mEntries;
    std::vector<std::string> mEntryValues;
    std::string mValue;
    std::string mSummary;
    bool mValueSet = false;
};

} // namespace cdroid

#endif // __CDROID_LIST_PREFERENCE_H__
