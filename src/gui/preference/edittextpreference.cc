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
#include <preference/edittextpreference.h>
#include <widget/internal_R.h>
#include <widget/framework_styleable.h>
#include <widget/edittext.h>
#include <core/parcel.h>

#include <preference/preferenceinflater.h>

namespace cdroid {

Preference::SummaryProvider EditTextPreference::SimpleSummaryProvider() {
    return [](Preference& preference) {
        auto* editTextPreference = static_cast<EditTextPreference*>(&preference);
        if (editTextPreference->getText().empty()) {
            return editTextPreference->getContext().getString((int)internal::R::string::not_set);
        } else {
            return editTextPreference->getText();
        }
    };
}

EditTextPreference::EditTextPreference(Context& context, const AttributeSet& attrs,
        int defStyleAttr, int defStyleRes)
    : DialogPreference(context, attrs, defStyleAttr, defStyleRes) {
    namespace ns = internal::R::styleable;

    auto a = context.obtainStyledAttributes(attrs, ns::EditTextPreference, defStyleAttr, defStyleRes);

    if (a->getBoolean(ns::EditTextPreference_useSimpleSummaryProvider, false)) {
        setSummaryProvider(SimpleSummaryProvider());
    }
}

EditTextPreference::EditTextPreference(Context& context, const AttributeSet& attrs,
        int defStyleAttr)
    : EditTextPreference(context, attrs, defStyleAttr, 0) {
}

EditTextPreference::EditTextPreference(Context& context, const AttributeSet& attrs)
    : EditTextPreference(context, attrs, (int)internal::R::attr::editTextPreferenceStyle) {
}

EditTextPreference::EditTextPreference(Context& context)
    : EditTextPreference(context, AttributeSet()) {
}

void EditTextPreference::setText(const std::string& text) {
    const bool wasBlocking = shouldDisableDependents();

    mText = text;

    persistString(text);

    const bool isBlocking = shouldDisableDependents();
    if (isBlocking != wasBlocking) {
        notifyDependencyChange(isBlocking);
    }

    notifyChanged();
}

std::string EditTextPreference::getText() const {
    return mText;
}

any EditTextPreference::onGetDefaultValue(const TypedArray& a, int index) {
    return any(a.getString(index));
}

void EditTextPreference::onSetInitialValue(const any& defaultValue) {
    std::string def;
    if (defaultValue.has_value()) {
        def = any_cast<std::string>(defaultValue);
    }
    setText(getPersistedString(def));
}

bool EditTextPreference::shouldDisableDependents() const {
    return mText.empty() || DialogPreference::shouldDisableDependents();
}

Parcelable* EditTextPreference::onSaveInstanceState() {
    Parcelable* superState = DialogPreference::onSaveInstanceState();
    if (isPersistent()) {
        // No need to save instance state since it's persistent
        return superState;
    }

    SavedState* myState = new SavedState(superState);
    myState->mText = getText();
    return myState;
}

void EditTextPreference::onRestoreInstanceState(Parcelable* state) {
    if (state == nullptr || dynamic_cast<SavedState*>(state) == nullptr) {
        // Didn't save state for us in onSaveInstanceState
        DialogPreference::onRestoreInstanceState(state);
        return;
    }

    SavedState* myState = static_cast<SavedState*>(state);
    DialogPreference::onRestoreInstanceState(myState->getSuperState());
    setText(myState->mText);
}

void EditTextPreference::setOnBindEditTextListener(const OnBindEditTextListener& onBindEditTextListener) {
    mOnBindEditTextListener = onBindEditTextListener;
}

EditTextPreference::OnBindEditTextListener EditTextPreference::getOnBindEditTextListener() const {
    return mOnBindEditTextListener;
}

EditTextPreference::SavedState::SavedState(Parcel& source)
    : Preference::BaseSavedState(source) {
    mText = source.readString();
}

EditTextPreference::SavedState::SavedState(Parcelable* superState)
    : Preference::BaseSavedState(superState) {
}

void EditTextPreference::SavedState::writeToParcel(Parcel& dest, int flags) {
    BaseSavedState::writeToParcel(dest, flags);
    dest.writeString(mText);
}

DECLARE_PREFERENCE(EditTextPreference)

} // namespace cdroid

