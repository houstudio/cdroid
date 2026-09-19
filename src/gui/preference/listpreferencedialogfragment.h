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
#ifndef __CDROID_LIST_PREFERENCE_DIALOG_FRAGMENT_H__
#define __CDROID_LIST_PREFERENCE_DIALOG_FRAGMENT_H__

#include <vector>
#include <preference/preferencedialogfragment.h>
#include <preference/listpreference.h>

namespace cdroid {

/**
 * Port of androidx.preference.ListPreferenceDialogFragmentCompat (Compat
 * suffix dropped).
 */
class ListPreferenceDialogFragment : public PreferenceDialogFragment {
public:
    static ListPreferenceDialogFragment* newInstance(const std::string& key);

    void onCreate(Bundle* savedInstanceState) override;
    void onSaveInstanceState(Bundle* outState) override;

    void onPrepareDialogBuilder(AlertDialog::Builder& builder) override;

    void onDialogClosed(bool positiveResult) override;

private:
    ListPreference* getListPreference();

    static constexpr const char* SAVE_STATE_INDEX = "ListPreferenceDialogFragment.index";
    static constexpr const char* SAVE_STATE_ENTRIES = "ListPreferenceDialogFragment.entries";
    static constexpr const char* SAVE_STATE_ENTRY_VALUES =
            "ListPreferenceDialogFragment.entryValues";

    int mClickedDialogEntryIndex = 0;
    std::vector<std::string> mEntries;
    std::vector<std::string> mEntryValues;
};

} // namespace cdroid

#endif // __CDROID_LIST_PREFERENCE_DIALOG_FRAGMENT_H__
