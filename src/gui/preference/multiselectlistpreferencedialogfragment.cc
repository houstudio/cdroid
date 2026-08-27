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
#include <preference/multiselectlistpreferencedialogfragment.h>
#include <preference/multiselectlistpreference.h>
#include <app/alertdialog.h>
#include <app/dialoginterface.h>
#include <core/bundle.h>
#include <stdexcept>

namespace cdroid {

MultiSelectListPreferenceDialogFragment*
MultiSelectListPreferenceDialogFragment::newInstance(const std::string& key) {
    auto* fragment = new MultiSelectListPreferenceDialogFragment();
    auto* b = new Bundle();
    b->putString(ARG_KEY, key);
    fragment->setArguments(b);
    return fragment;
}

void MultiSelectListPreferenceDialogFragment::onCreate(Bundle* savedInstanceState) {
    PreferenceDialogFragment::onCreate(savedInstanceState);
    if (savedInstanceState == nullptr) {
        MultiSelectListPreference* preference = getMultiSelectListPreference();

        if (preference->getEntries().empty() || preference->getEntryValues().empty()) {
            throw std::logic_error(
                    "MultiSelectListPreference requires an entries array and an entryValues array.");
        }

        mNewValues = preference->getValues();
        mEntries = preference->getEntries();
        mEntryValues = preference->getEntryValues();
    } else {
        mNewValues.clear();
        for (const auto& v : savedInstanceState->getStringArray(SAVE_STATE_VALUES)) {
            mNewValues.insert(v);
        }
        mPreferenceChanged = savedInstanceState->getBoolean(SAVE_STATE_CHANGED, false);
        mEntries = savedInstanceState->getStringArray(SAVE_STATE_ENTRIES);
        mEntryValues = savedInstanceState->getStringArray(SAVE_STATE_ENTRY_VALUES);
    }
}

void MultiSelectListPreferenceDialogFragment::onSaveInstanceState(Bundle* outState) {
    PreferenceDialogFragment::onSaveInstanceState(outState);
    outState->putStringArray(SAVE_STATE_VALUES,
            std::vector<std::string>(mNewValues.begin(), mNewValues.end()));
    outState->putBoolean(SAVE_STATE_CHANGED, mPreferenceChanged);
    outState->putStringArray(SAVE_STATE_ENTRIES, mEntries);
    outState->putStringArray(SAVE_STATE_ENTRY_VALUES, mEntryValues);
}

MultiSelectListPreference* MultiSelectListPreferenceDialogFragment::getMultiSelectListPreference() {
    return static_cast<MultiSelectListPreference*>(getPreference());
}

void MultiSelectListPreferenceDialogFragment::onPrepareDialogBuilder(
        AlertDialog::Builder& builder) {
    PreferenceDialogFragment::onPrepareDialogBuilder(builder);

    MultiSelectListPreference* preference = getMultiSelectListPreference();
    const std::vector<std::string>& entryValues = mEntryValues;
    const int entryCount = (int)entryValues.size();
    std::vector<bool> selectedItems(entryCount);
    for (int i = 0; i < entryCount; i++) {
        selectedItems[i] = mNewValues.count(entryValues[i]) > 0;
    }

    builder.setMultiChoiceItems(mEntries, selectedItems,
        [this, entryValues](Dialog&, int which, bool isChecked) {
            (void)entryValues;
            if (isChecked) {
                mNewValues.insert(mEntryValues[which]);
            } else {
                mNewValues.erase(mEntryValues[which]);
            }
            mPreferenceChanged = true;
        });
}

void MultiSelectListPreferenceDialogFragment::onDialogClosed(bool positiveResult) {
    MultiSelectListPreference* preference = getMultiSelectListPreference();
    if (positiveResult && mPreferenceChanged) {
        if (preference->callChangeListener(mNewValues)) {
            preference->setValues(mNewValues);
        }
    }
}

} // namespace cdroid
