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
#include <preference/listpreferencedialogfragment.h>
#include <preference/listpreference.h>
#include <app/alertdialog.h>
#include <app/dialoginterface.h>
#include <core/bundle.h>
#include <stdexcept>

namespace cdroid {

ListPreferenceDialogFragment* ListPreferenceDialogFragment::newInstance(const std::string& key) {
    auto* fragment = new ListPreferenceDialogFragment();
    auto* b = new Bundle();
    b->putString(ARG_KEY, key);
    fragment->setArguments(b);
    return fragment;
}

void ListPreferenceDialogFragment::onCreate(Bundle* savedInstanceState) {
    PreferenceDialogFragment::onCreate(savedInstanceState);
    if (savedInstanceState == nullptr) {
        ListPreference* preference = getListPreference();

        if (preference->getEntries().empty() || preference->getEntryValues().empty()) {
            throw std::logic_error(
                    "ListPreference requires an entries array and an entryValues array.");
        }

        mClickedDialogEntryIndex = preference->findIndexOfValue(preference->getValue());
        mEntries = preference->getEntries();
        mEntryValues = preference->getEntryValues();
    } else {
        mClickedDialogEntryIndex = savedInstanceState->getInt(SAVE_STATE_INDEX, 0);
        mEntries = savedInstanceState->getStringArray(SAVE_STATE_ENTRIES);
        mEntryValues = savedInstanceState->getStringArray(SAVE_STATE_ENTRY_VALUES);
    }
}

void ListPreferenceDialogFragment::onSaveInstanceState(Bundle* outState) {
    PreferenceDialogFragment::onSaveInstanceState(outState);
    outState->putInt(SAVE_STATE_INDEX, mClickedDialogEntryIndex);
    outState->putStringArray(SAVE_STATE_ENTRIES, mEntries);
    outState->putStringArray(SAVE_STATE_ENTRY_VALUES, mEntryValues);
}

ListPreference* ListPreferenceDialogFragment::getListPreference() {
    return static_cast<ListPreference*>(getPreference());
}

void ListPreferenceDialogFragment::onPrepareDialogBuilder(AlertDialog::Builder& builder) {
    PreferenceDialogFragment::onPrepareDialogBuilder(builder);

    builder.setSingleChoiceItems(mEntries, mClickedDialogEntryIndex,
        [this](Dialog& dialog, int which) {
            mClickedDialogEntryIndex = which;

            // Clicking on an item simulates the positive button click, and dismisses
            // the dialog.
            onClick(*static_cast<DialogInterface*>(&dialog), DialogInterface::BUTTON_POSITIVE);
            dialog.dismiss();
        });

    // The typical interaction for list-based dialogs is to have click-on-an-item dismiss the
    // dialog instead of the user having to press 'Ok'.
    builder.setPositiveButton(std::string(), nullptr);
}

void ListPreferenceDialogFragment::onDialogClosed(bool positiveResult) {
    if (positiveResult && mClickedDialogEntryIndex >= 0) {
        std::string value = mEntryValues[mClickedDialogEntryIndex];
        ListPreference* preference = getListPreference();
        if (preference->callChangeListener(value)) {
            preference->setValue(value);
        }
    }
}

} // namespace cdroid
