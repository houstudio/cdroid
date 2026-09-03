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
#include <preference/preferencedialogfragment.h>
#include <app/alertdialog.h>
#include <view/view.h>
#include <view/layoutinflater.h>
#include <widget/textview.h>
#include <widget/internal_R.h>
#include <core/bundle.h>
#include <stdexcept>

namespace cdroid {

void PreferenceDialogFragment::onCreate(Bundle* savedInstanceState) {
    DialogFragment::onCreate(savedInstanceState);

    Fragment* rawFragment = getTargetFragment();
    auto* target = dynamic_cast<DialogPreference::TargetFragment*>(rawFragment);
    if (target == nullptr) {
        throw std::logic_error("Target fragment must implement TargetFragment interface");
    }

    const Bundle* args = getArguments();
    std::string key = args ? args->getString(ARG_KEY) : std::string();
    if (savedInstanceState == nullptr) {
        mPreference = static_cast<DialogPreference*>(target->findPreference(key));
        mDialogTitle = mPreference->getDialogTitle();
        mPositiveButtonText = mPreference->getPositiveButtonText();
        mNegativeButtonText = mPreference->getNegativeButtonText();
        mDialogMessage = mPreference->getDialogMessage();
        mDialogLayoutRes = mPreference->getDialogLayoutResource();
        mDialogIcon = mPreference->getDialogIcon();
    } else {
        mDialogTitle = savedInstanceState->getString(SAVE_STATE_TITLE);
        mPositiveButtonText = savedInstanceState->getString(SAVE_STATE_POSITIVE_TEXT);
        mNegativeButtonText = savedInstanceState->getString(SAVE_STATE_NEGATIVE_TEXT);
        mDialogMessage = savedInstanceState->getString(SAVE_STATE_MESSAGE);
        mDialogLayoutRes = savedInstanceState->getInt(SAVE_STATE_LAYOUT, 0);
    }
}

void PreferenceDialogFragment::onSaveInstanceState(Bundle* outState) {
    DialogFragment::onSaveInstanceState(outState);

    outState->putString(SAVE_STATE_TITLE, mDialogTitle);
    outState->putString(SAVE_STATE_POSITIVE_TEXT, mPositiveButtonText);
    outState->putString(SAVE_STATE_NEGATIVE_TEXT, mNegativeButtonText);
    outState->putString(SAVE_STATE_MESSAGE, mDialogMessage);
    outState->putInt(SAVE_STATE_LAYOUT, mDialogLayoutRes);
}

Dialog* PreferenceDialogFragment::onCreateDialog(Bundle* savedInstanceState) {
    mWhichButtonClicked = DialogInterface::BUTTON_NEGATIVE;

    auto* builder = new AlertDialog::Builder(requireContext());
    builder->setTitle(mDialogTitle)
           .setIcon(mDialogIcon)
           .setPositiveButton(mPositiveButtonText,
               [this](Dialog& d, int which) { onClick(*static_cast<DialogInterface*>(&d), which); })
           .setNegativeButton(mNegativeButtonText,
               [this](Dialog& d, int which) { onClick(*static_cast<DialogInterface*>(&d), which); });

    View* contentView = onCreateDialogView(*requireContext());
    if (contentView != nullptr) {
        onBindDialogView(*contentView);
        builder->setView(contentView);
    } else {
        builder->setMessage(mDialogMessage);
    }

    onPrepareDialogBuilder(*builder);

    // Create the dialog
    AlertDialog* dialog = builder->create();
    delete builder;  // the shell only: create() moved P into the dialog (GC in AOSP)
    // needInputMethod()/requestInputMethod(): CDROID has no soft-input
    // window service — the IME hooks are skipped (hardware keys drive input).
    (void)savedInstanceState;
    return dialog;
}

DialogPreference* PreferenceDialogFragment::getPreference() {
    if (mPreference == nullptr) {
        const Bundle* args = getArguments();
        std::string key = args ? args->getString(ARG_KEY) : std::string();
        auto* fragment = dynamic_cast<DialogPreference::TargetFragment*>(getTargetFragment());
        mPreference = fragment ? static_cast<DialogPreference*>(fragment->findPreference(key)) : nullptr;
    }
    return mPreference;
}

void PreferenceDialogFragment::onPrepareDialogBuilder(AlertDialog::Builder& /*builder*/) {
}

bool PreferenceDialogFragment::needInputMethod() {
    return false;
}

View* PreferenceDialogFragment::onCreateDialogView(Context& context) {
    const int resId = mDialogLayoutRes;
    if (resId == 0) {
        return nullptr;
    }

    // AOSP: getLayoutInflater().inflate(resId, null) — the CDROID fragment
    // has no getLayoutInflater accessor, so inflate straight from the context.
    return LayoutInflater::from(&context)->inflate(resId, nullptr);
}

void PreferenceDialogFragment::onBindDialogView(View& view) {
    View* dialogMessageView = view.findViewById((int)internal::R::id::message);

    if (dialogMessageView != nullptr) {
        const std::string& message = mDialogMessage;
        int newVisibility = View::GONE;

        if (!message.empty()) {
            auto* textView = dynamic_cast<TextView*>(dialogMessageView);
            if (textView != nullptr) {
                textView->setText(message);
            }
            newVisibility = View::VISIBLE;
        }

        if (dialogMessageView->getVisibility() != newVisibility) {
            dialogMessageView->setVisibility(newVisibility);
        }
    }
}

void PreferenceDialogFragment::onClick(DialogInterface& /*dialog*/, int which) {
    mWhichButtonClicked = which;
}

void PreferenceDialogFragment::onDismiss(DialogInterface* dialog) {
    DialogFragment::onDismiss(dialog);
    onDialogClosed(mWhichButtonClicked == DialogInterface::BUTTON_POSITIVE);
}

} // namespace cdroid
