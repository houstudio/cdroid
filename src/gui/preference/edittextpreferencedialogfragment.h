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
#ifndef __CDROID_EDITTEXT_PREFERENCE_DIALOG_FRAGMENT_H__
#define __CDROID_EDITTEXT_PREFERENCE_DIALOG_FRAGMENT_H__

#include <preference/preferencedialogfragment.h>
#include <preference/edittextpreference.h>

namespace cdroid {

class EditText;

/**
 * Port of androidx.preference.EditTextPreferenceDialogFragmentCompat (Compat
 * suffix dropped).
 */
class EditTextPreferenceDialogFragment : public PreferenceDialogFragment {
public:
    static EditTextPreferenceDialogFragment* newInstance(const std::string& key);

    void onCreate(Bundle* savedInstanceState) override;
    void onSaveInstanceState(Bundle* outState) override;

    void onBindDialogView(View& view) override;

    bool needInputMethod() override;

    void onDialogClosed(bool positiveResult) override;

private:
    EditTextPreference* getEditTextPreference();

    static constexpr const char* SAVE_STATE_TEXT = "EditTextPreferenceDialogFragment.text";

    EditText* mEditText = nullptr;
    std::string mText;
};

} // namespace cdroid

#endif // __CDROID_EDITTEXT_PREFERENCE_DIALOG_FRAGMENT_H__
