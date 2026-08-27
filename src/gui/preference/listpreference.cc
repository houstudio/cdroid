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
#include <content/typedarray.h>
#include <preference/listpreference.h>
#include <widget/internal_R.h>
#include <widget/framework_styleable.h>
#include <core/parcel.h>
#include <porting/cdlog.h>

#include <preference/preferenceinflater.h>

namespace cdroid {

ListPreference::SimpleSummaryProvider* ListPreference::SimpleSummaryProvider::sSimpleSummaryProvider = nullptr;

ListPreference::SimpleSummaryProvider* ListPreference::SimpleSummaryProvider::getInstance() {
    if (sSimpleSummaryProvider == nullptr) {
        sSimpleSummaryProvider = new SimpleSummaryProvider();
    }
    return sSimpleSummaryProvider;
}

std::string ListPreference::SimpleSummaryProvider::provideSummary(Preference& preference) {
    auto* listPreference = static_cast<ListPreference*>(&preference);
    if (listPreference->getEntry().empty()) {
        return listPreference->getContext().getString((int)internal::R::string::not_set);
    } else {
        return listPreference->getEntry();
    }
}

ListPreference::ListPreference(Context& context, const AttributeSet& attrs,
        int defStyleAttr, int defStyleRes)
    : DialogPreference(context, attrs, defStyleAttr, defStyleRes) {
    namespace ns = internal::R::styleable;

    auto a = context.obtainStyledAttributes(attrs, ns::ListPreference, defStyleAttr, defStyleRes);

    mEntries = a->getTextArray(ns::ListPreference_entries);
    mEntryValues = a->getTextArray(ns::ListPreference_entryValues);

    if (a->getBoolean(ns::ListPreference_useSimpleSummaryProvider, false)) {
        setSummaryProvider(SimpleSummaryProvider::getInstance());
    }

    // Retrieve the Preference summary attribute since it's private in the Preference class.
    auto pa = context.obtainStyledAttributes(attrs, ns::Preference, defStyleAttr, defStyleRes);
    mSummary = pa->getString(ns::Preference_summary);
}

ListPreference::ListPreference(Context& context, const AttributeSet& attrs, int defStyleAttr)
    : ListPreference(context, attrs, defStyleAttr, 0) {
}

ListPreference::ListPreference(Context& context, const AttributeSet& attrs)
    : ListPreference(context, attrs, (int)internal::R::attr::dialogPreferenceStyle) {
}

ListPreference::ListPreference(Context& context)
    : ListPreference(context, AttributeSet()) {
}

void ListPreference::setEntries(const std::vector<std::string>& entries) {
    mEntries = entries;
}

void ListPreference::setEntries(int entriesResId) {
    mEntries = getContext().getResources().getStringArray(entriesResId);
}

std::vector<std::string> ListPreference::getEntries() const {
    return mEntries;
}

void ListPreference::setEntryValues(const std::vector<std::string>& entryValues) {
    mEntryValues = entryValues;
}

void ListPreference::setEntryValues(int entryValuesResId) {
    mEntryValues = getContext().getResources().getStringArray(entryValuesResId);
}

std::vector<std::string> ListPreference::getEntryValues() const {
    return mEntryValues;
}

void ListPreference::setSummary(const std::string& summary) {
    DialogPreference::setSummary(summary);
    mSummary = summary;
}

std::string ListPreference::getSummary() const {
    if (getSummaryProvider() != nullptr) {
        return getSummaryProvider()->provideSummary(const_cast<ListPreference&>(*this));
    }
    const std::string entry = getEntry();
    const std::string summary = DialogPreference::getSummary();
    if (mSummary.empty()) {
        return summary;
    }
    // String.format(mSummary, entry): the deprecated %s formatting marker is
    // replaced with the current entry (kept for API parity, deprecated in AOSP).
    std::string formatted = mSummary;
    const size_t marker = formatted.find("%s");
    if (marker != std::string::npos) {
        formatted.replace(marker, 2, entry);
    }
    if (formatted == summary) {
        return summary;
    }
    LOGW("Setting a summary with a String formatting marker is no longer supported."
         " You should use a SummaryProvider instead.");
    return formatted;
}

void ListPreference::setValue(const std::string& value) {
    // Always persist/notify the first time.
    const bool changed = mValue != value;
    if (changed || !mValueSet) {
        mValue = value;
        mValueSet = true;
        persistString(value);
        if (changed) {
            notifyChanged();
        }
    }
}

std::string ListPreference::getValue() const {
    return mValue;
}

std::string ListPreference::getEntry() const {
    const int index = getValueIndex();
    return (index >= 0 && !mEntries.empty()) ? mEntries[index] : std::string();
}

int ListPreference::findIndexOfValue(const std::string& value) const {
    if (!value.empty() && !mEntryValues.empty()) {
        for (int i = (int)mEntryValues.size() - 1; i >= 0; i--) {
            if (mEntryValues[i] == value) {
                return i;
            }
        }
    }
    return -1;
}

void ListPreference::setValueIndex(int index) {
    if (!mEntryValues.empty()) {
        setValue(mEntryValues[index]);
    }
}

int ListPreference::getValueIndex() const {
    return findIndexOfValue(mValue);
}

any ListPreference::onGetDefaultValue(const TypedArray& a, int index) {
    return any(a.getString(index));
}

void ListPreference::onSetInitialValue(const any& defaultValue) {
    std::string def;
    if (defaultValue.has_value()) {
        def = any_cast<std::string>(defaultValue);
    }
    setValue(getPersistedString(def));
}

Parcelable* ListPreference::onSaveInstanceState() {
    Parcelable* superState = DialogPreference::onSaveInstanceState();
    if (isPersistent()) {
        // No need to save instance state since it's persistent
        return superState;
    }

    SavedState* myState = new SavedState(superState);
    myState->mValue = getValue();
    return myState;
}

void ListPreference::onRestoreInstanceState(Parcelable* state) {
    if (state == nullptr || dynamic_cast<SavedState*>(state) == nullptr) {
        // Didn't save state for us in onSaveInstanceState
        DialogPreference::onRestoreInstanceState(state);
        return;
    }

    SavedState* myState = static_cast<SavedState*>(state);
    DialogPreference::onRestoreInstanceState(myState->getSuperState());
    setValue(myState->mValue);
}

ListPreference::SavedState::SavedState(Parcel& source)
    : Preference::BaseSavedState(source) {
    mValue = source.readString();
}

ListPreference::SavedState::SavedState(Parcelable* superState)
    : Preference::BaseSavedState(superState) {
}

void ListPreference::SavedState::writeToParcel(Parcel& dest, int flags) {
    BaseSavedState::writeToParcel(dest, flags);
    dest.writeString(mValue);
}

DECLARE_PREFERENCE(ListPreference)

} // namespace cdroid

