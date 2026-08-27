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
#ifndef __CDROID_DIALOG_PREFERENCE_H__
#define __CDROID_DIALOG_PREFERENCE_H__

#include <preference/preference.h>

namespace cdroid {

class Drawable;

/**
 * Port of androidx.preference.DialogPreference — a base class for Preferences
 * that are dialog-based. When clicked, these preferences will open a dialog
 * showing the actual preference controls.
 */
class DialogPreference : public Preference {
public:
    /**
     * Interface for PreferenceFragments to implement to allow
     * DialogPreferences to find the preference that launched the dialog.
     */
    class TargetFragment {
    public:
        virtual ~TargetFragment() = default;
        virtual Preference* findPreference(const std::string& key) = 0;
    };

    DialogPreference(Context& context);
    DialogPreference(Context& context, const AttributeSet& attrs);
    DialogPreference(Context& context, const AttributeSet& attrs, int defStyleAttr);
    DialogPreference(Context& context, const AttributeSet& attrs,
            int defStyleAttr, int defStyleRes);

    void setDialogTitle(const std::string& dialogTitle);
    void setDialogTitle(int dialogTitleResId);
    std::string getDialogTitle() const;

    void setDialogMessage(const std::string& dialogMessage);
    void setDialogMessage(int dialogMessageResId);
    std::string getDialogMessage() const;

    void setDialogIcon(Drawable* dialogIcon);
    void setDialogIcon(int dialogIconRes);
    Drawable* getDialogIcon() const;

    void setPositiveButtonText(const std::string& positiveButtonText);
    void setPositiveButtonText(int positiveButtonTextResId);
    std::string getPositiveButtonText() const;

    void setNegativeButtonText(const std::string& negativeButtonText);
    void setNegativeButtonText(int negativeButtonTextResId);
    std::string getNegativeButtonText() const;

    void setDialogLayoutResource(int dialogLayoutResId);
    int getDialogLayoutResource() const;

    std::string getPreferenceClassName() const override { return "DialogPreference"; }

protected:
    void onClick() override;

private:
    std::string mDialogTitle;
    std::string mDialogMessage;
    Drawable* mDialogIcon = nullptr;
    std::string mPositiveButtonText;
    std::string mNegativeButtonText;
    int mDialogLayoutResId = 0;
};

} // namespace cdroid

#endif // __CDROID_DIALOG_PREFERENCE_H__
