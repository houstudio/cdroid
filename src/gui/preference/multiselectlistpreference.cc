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
#include <preference/multiselectlistpreference.h>
#include <widget/internal_R.h>
#include <widget/framework_styleable.h>
#include <core/parcel.h>

#include <preference/preferenceinflater.h>

namespace cdroid {

MultiSelectListPreference::MultiSelectListPreference(Context& context, const AttributeSet& attrs,
        int defStyleAttr, int defStyleRes)
    : DialogPreference(context, attrs, defStyleAttr, defStyleRes) {
    namespace ns = internal::R::styleable;

    auto a = context.obtainStyledAttributes(attrs, ns::MultiSelectListPreference,
            defStyleAttr, defStyleRes);

    mEntries = a->getTextArray(ns::MultiSelectListPreference_entries);
    mEntryValues = a->getTextArray(ns::MultiSelectListPreference_entryValues);
}

MultiSelectListPreference::MultiSelectListPreference(Context& context, const AttributeSet& attrs,
        int defStyleAttr)
    : MultiSelectListPreference(context, attrs, defStyleAttr, 0) {
}

MultiSelectListPreference::MultiSelectListPreference(Context& context, const AttributeSet& attrs)
    : MultiSelectListPreference(context, attrs, (int)internal::R::attr::dialogPreferenceStyle) {
}

MultiSelectListPreference::MultiSelectListPreference(Context& context)
    : MultiSelectListPreference(context, AttributeSet()) {
}

void MultiSelectListPreference::setEntries(const std::vector<std::string>& entries) {
    mEntries = entries;
}

void MultiSelectListPreference::setEntries(int entriesResId) {
    mEntries = getContext().getResources().getStringArray(entriesResId);
}

std::vector<std::string> MultiSelectListPreference::getEntries() const {
    return mEntries;
}

void MultiSelectListPreference::setEntryValues(const std::vector<std::string>& entryValues) {
    mEntryValues = entryValues;
}

void MultiSelectListPreference::setEntryValues(int entryValuesResId) {
    mEntryValues = getContext().getResources().getStringArray(entryValuesResId);
}

std::vector<std::string> MultiSelectListPreference::getEntryValues() const {
    return mEntryValues;
}

void MultiSelectListPreference::setValues(const std::set<std::string>& values) {
    mValues.clear();
    mValues.insert(values.begin(), values.end());

    persistStringSet(values);
    notifyChanged();
}

const std::set<std::string>& MultiSelectListPreference::getValues() const {
    return mValues;
}

int MultiSelectListPreference::findIndexOfValue(const std::string& value) const {
    if (!value.empty() && !mEntryValues.empty()) {
        for (int i = (int)mEntryValues.size() - 1; i >= 0; i--) {
            if (mEntryValues[i] == value) {
                return i;
            }
        }
    }
    return -1;
}

bool* MultiSelectListPreference::getSelectedItems() const {
    const int entryCount = (int)mEntryValues.size();
    bool* result = new bool[entryCount];

    for (int i = 0; i < entryCount; i++) {
        result[i] = mValues.count(mEntryValues[i]) > 0;
    }

    return result;
}

any MultiSelectListPreference::onGetDefaultValue(const TypedArray& a, int index) {
    const std::vector<std::string> defaultValues = a.getTextArray(index);
    auto* result = new std::set<std::string>();
    for (const auto& defaultValue : defaultValues) {
        result->insert(defaultValue);
    }
    return any(result);
}

void MultiSelectListPreference::onSetInitialValue(const any& defaultValue) {
    std::set<std::string> def;
    if (defaultValue.has_value()) {
        def = *any_cast<std::set<std::string>*>(defaultValue);
    }
    setValues(getPersistedStringSet(def));
}

Parcelable* MultiSelectListPreference::onSaveInstanceState() {
    Parcelable* superState = DialogPreference::onSaveInstanceState();
    if (isPersistent()) {
        // No need to save instance state
        return superState;
    }

    SavedState* myState = new SavedState(superState);
    myState->mValues = getValues();
    return myState;
}

void MultiSelectListPreference::onRestoreInstanceState(Parcelable* state) {
    if (state == nullptr || dynamic_cast<SavedState*>(state) == nullptr) {
        // Didn't save state for us in onSaveInstanceState
        DialogPreference::onRestoreInstanceState(state);
        return;
    }

    SavedState* myState = static_cast<SavedState*>(state);
    DialogPreference::onRestoreInstanceState(myState->getSuperState());
    setValues(myState->mValues);
}

MultiSelectListPreference::SavedState::SavedState(Parcel& source)
    : Preference::BaseSavedState(source) {
    const int size = source.readInt();
    for (int i = 0; i < size; i++) {
        mValues.insert(source.readString());
    }
}

MultiSelectListPreference::SavedState::SavedState(Parcelable* superState)
    : Preference::BaseSavedState(superState) {
}

void MultiSelectListPreference::SavedState::writeToParcel(Parcel& dest, int flags) {
    BaseSavedState::writeToParcel(dest, flags);
    dest.writeInt((int)mValues.size());
    for (const auto& v : mValues) {
        dest.writeString(v);
    }
}

DECLARE_PREFERENCE(MultiSelectListPreference)

} // namespace cdroid

