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
#include <preference/edittextpreferencedialogfragment.h>
#include <preference/edittextpreference.h>
#include <widget/edittext.h>
#include <widget/internal_R.h>
#include <core/bundle.h>
#include <stdexcept>

namespace cdroid {

EditTextPreferenceDialogFragment* EditTextPreferenceDialogFragment::newInstance(
        const std::string& key) {
    auto* fragment = new EditTextPreferenceDialogFragment();
    auto* b = new Bundle();
    b->putString(ARG_KEY, key);
    fragment->setArguments(b);
    return fragment;
}

void EditTextPreferenceDialogFragment::onCreate(Bundle* savedInstanceState) {
    PreferenceDialogFragment::onCreate(savedInstanceState);
    if (savedInstanceState == nullptr) {
        mText = getEditTextPreference()->getText();
    } else {
        mText = savedInstanceState->getString(SAVE_STATE_TEXT);
    }
}

void EditTextPreferenceDialogFragment::onSaveInstanceState(Bundle* outState) {
    PreferenceDialogFragment::onSaveInstanceState(outState);
    outState->putString(SAVE_STATE_TEXT, mText);
}

void EditTextPreferenceDialogFragment::onBindDialogView(View& view) {
    PreferenceDialogFragment::onBindDialogView(view);

    mEditText = dynamic_cast<EditText*>(view.findViewById((int)internal::R::id::edit));

    if (mEditText == nullptr) {
        throw std::logic_error("Dialog view must contain an EditText with id @android:id/edit");
    }

    mEditText->requestFocus();
    mEditText->setText(mText);
    // Place cursor at the end
    mEditText->setSelection((int)mEditText->getText().length());
    if (getEditTextPreference()->getOnBindEditTextListener() != nullptr) {
        getEditTextPreference()->getOnBindEditTextListener()(*mEditText);
    }
}

EditTextPreference* EditTextPreferenceDialogFragment::getEditTextPreference() {
    return static_cast<EditTextPreference*>(getPreference());
}

bool EditTextPreferenceDialogFragment::needInputMethod() {
    // We want the input method to show, if possible, when dialog is displayed
    return true;
}

void EditTextPreferenceDialogFragment::onDialogClosed(bool positiveResult) {
    if (positiveResult) {
        std::string value = mEditText->getText();
        EditTextPreference* preference = getEditTextPreference();
        if (preference->callChangeListener(value)) {
            preference->setText(value);
        }
    }
}

} // namespace cdroid
