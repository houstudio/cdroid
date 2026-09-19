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
#ifndef __CDROID_EDITTEXT_PREFERENCE_H__
#define __CDROID_EDITTEXT_PREFERENCE_H__

#include <preference/dialogpreference.h>

namespace cdroid {

class EditText;
class Parcel;

/**
 * Port of androidx.preference.EditTextPreference — a DialogPreference that
 * shows an EditText in the dialog. This preference saves a string value.
 */
class EditTextPreference : public DialogPreference {
public:
    /**
     * Interface definition for a callback to be invoked when the
     * corresponding dialog view for this preference is bound.
     *
     * CallbackBase value type — android's implementers implement the
     * interface; here it is assigned a lambda.
     */
    using OnBindEditTextListener = CallbackBase<void, EditText&>;

    /**
     * A simple SummaryProvider implementation for an EditTextPreference. If
     * no value has been set, the summary displayed will be 'Not set',
     * otherwise the summary displayed will be the value set for this
     * preference.
     *
     * android: a singleton subclass of Preference.SummaryProvider with
     * getInstance(); now a CallbackBase factory returning the provider by value.
     */
    static Preference::SummaryProvider SimpleSummaryProvider();

    EditTextPreference(Context& context);
    EditTextPreference(Context& context, const AttributeSet& attrs);
    EditTextPreference(Context& context, const AttributeSet& attrs, int defStyleAttr);
    EditTextPreference(Context& context, const AttributeSet& attrs,
            int defStyleAttr, int defStyleRes);

    /** Saves the text to the current data storage. */
    void setText(const std::string& text);

    /** Gets the text from the current data storage. */
    std::string getText() const;

    any onGetDefaultValue(const TypedArray& a, int index) override;
    void onSetInitialValue(const any& defaultValue) override;
    bool shouldDisableDependents() const override;

    void setOnBindEditTextListener(const OnBindEditTextListener& onBindEditTextListener);
    OnBindEditTextListener getOnBindEditTextListener() const;

    std::string getPreferenceClassName() const override { return "EditTextPreference"; }

    class SavedState : public Preference::BaseSavedState {
    public:
        std::string mText;
        SavedState(Parcel& source);
        SavedState(Parcelable* superState);
        void writeToParcel(Parcel& dest, int flags) override;
    };

protected:
    Parcelable* onSaveInstanceState() override;
    void onRestoreInstanceState(Parcelable* state) override;

private:
    std::string mText;
    OnBindEditTextListener mOnBindEditTextListener;
};

} // namespace cdroid

#endif // __CDROID_EDITTEXT_PREFERENCE_H__
