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
#ifndef __CDROID_PREFERENCE_DIALOG_FRAGMENT_H__
#define __CDROID_PREFERENCE_DIALOG_FRAGMENT_H__

#include <string>
#include <fragment/dialogfragment.h>
#include <app/alertdialog.h>
#include <preference/dialogpreference.h>

namespace cdroid {

class AlertDialog;
class Drawable;
class View;

/**
 * Port of androidx.preference.PreferenceDialogFragmentCompat (the Compat
 * suffix is dropped) — abstract base class which presents a dialog associated
 * with a DialogPreference.
 */
class PreferenceDialogFragment : public fragment::DialogFragment {
public:
    static constexpr const char* ARG_KEY = "key";

    void onCreate(Bundle* savedInstanceState) override;
    void onSaveInstanceState(Bundle* outState) override;
    Dialog* onCreateDialog(Bundle* savedInstanceState) override;

    /** Get the preference that requested this dialog. */
    DialogPreference* getPreference();

    /**
     * Prepares the dialog builder to be shown when the preference is clicked.
     */
    virtual void onPrepareDialogBuilder(class AlertDialog::Builder& builder);

    /**
     * Returns whether the preference needs to display a soft input method
     * when the dialog is displayed. Default is false.
     */
    virtual bool needInputMethod();

    /**
     * Creates the content view for the dialog (if a custom content view is
     * required). By default, it inflates the dialog layout resource if it is
     * set.
     */
    virtual View* onCreateDialogView(Context& context);

    /**
     * Binds views in the content view of the dialog to data.
     */
    virtual void onBindDialogView(View& view);

    virtual void onClick(DialogInterface& dialog, int which);
    void onDismiss(DialogInterface* dialog) override;

    virtual void onDialogClosed(bool positiveResult) = 0;

protected:
    DialogPreference* mPreference = nullptr;

    std::string mDialogTitle;
    std::string mPositiveButtonText;
    std::string mNegativeButtonText;
    std::string mDialogMessage;
    int mDialogLayoutRes = 0;

    Drawable* mDialogIcon = nullptr;

    /** Which button was clicked. */
    int mWhichButtonClicked = 0;

private:
    static constexpr const char* SAVE_STATE_TITLE = "PreferenceDialogFragment.title";
    static constexpr const char* SAVE_STATE_POSITIVE_TEXT = "PreferenceDialogFragment.positiveText";
    static constexpr const char* SAVE_STATE_NEGATIVE_TEXT = "PreferenceDialogFragment.negativeText";
    static constexpr const char* SAVE_STATE_MESSAGE = "PreferenceDialogFragment.message";
    static constexpr const char* SAVE_STATE_LAYOUT = "PreferenceDialogFragment.layout";
};

} // namespace cdroid

#endif // __CDROID_PREFERENCE_DIALOG_FRAGMENT_H__
