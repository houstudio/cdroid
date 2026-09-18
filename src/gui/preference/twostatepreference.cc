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
#include <preference/twostatepreference.h>
#include <preference/preferenceviewholder.h>
#include <widget/internal_R.h>
#include <widget/framework_styleable.h>
#include <view/view.h>
#include <widget/textview.h>
#include <core/parcel.h>

namespace cdroid {

TwoStatePreference::TwoStatePreference(Context& context, const AttributeSet& attrs,
        int defStyleAttr, int defStyleRes)
    : Preference(context, attrs, defStyleAttr, defStyleRes), mChecked(false) {
}

TwoStatePreference::TwoStatePreference(Context& context, const AttributeSet& attrs,
        int defStyleAttr)
    : TwoStatePreference(context, attrs, defStyleAttr, 0) {
}

TwoStatePreference::TwoStatePreference(Context& context, const AttributeSet& attrs)
    : TwoStatePreference(context, attrs, 0) {
}

TwoStatePreference::TwoStatePreference(Context& context)
    : TwoStatePreference(context, AttributeSet()) {
}

void TwoStatePreference::onClick() {
    Preference::onClick();

    const bool newValue = !isChecked();
    if (callChangeListener(newValue)) {
        setChecked(newValue);
    }
}

bool TwoStatePreference::isChecked() const {
    return mChecked;
}

void TwoStatePreference::setChecked(bool checked) {
    // Always persist/notify the first time; don't assume the field's default of false.
    const bool changed = mChecked != checked;
    if (changed || !mCheckedSet) {
        mChecked = checked;
        mCheckedSet = true;
        persistBoolean(checked);
        if (changed) {
            notifyDependencyChange(shouldDisableDependents());
            notifyChanged();
        }
    }
}

bool TwoStatePreference::shouldDisableDependents() const {
    const bool shouldDisable = mDisableDependentsState ? mChecked : !mChecked;
    return shouldDisable || Preference::shouldDisableDependents();
}

void TwoStatePreference::setSummaryOn(const std::string& summary) {
    mSummaryOn = summary;
    if (isChecked()) {
        notifyChanged();
    }
}

std::string TwoStatePreference::getSummaryOn() const {
    return mSummaryOn;
}

void TwoStatePreference::setSummaryOn(int summaryResId) {
    setSummaryOn(getContext().getString(summaryResId));
}

void TwoStatePreference::setSummaryOff(const std::string& summary) {
    mSummaryOff = summary;
    if (!isChecked()) {
        notifyChanged();
    }
}

std::string TwoStatePreference::getSummaryOff() const {
    return mSummaryOff;
}

void TwoStatePreference::setSummaryOff(int summaryResId) {
    setSummaryOff(getContext().getString(summaryResId));
}

bool TwoStatePreference::getDisableDependentsState() const {
    return mDisableDependentsState;
}

void TwoStatePreference::setDisableDependentsState(bool disableDependentsState) {
    mDisableDependentsState = disableDependentsState;
}

any TwoStatePreference::onGetDefaultValue(const TypedArray& a, int index) {
    return any(a.getBoolean(index, false));
}

void TwoStatePreference::onSetInitialValue(const any& defaultValue) {
    bool def = false;
    if (defaultValue.has_value()) {
        def = any_cast<bool>(defaultValue);
    }
    setChecked(getPersistedBoolean(def));
}

void TwoStatePreference::syncSummaryView(PreferenceViewHolder& holder) {
    // Sync the summary holder
    View* view = holder.findViewById((int)internal::R::id::summary);
    syncSummaryView(view);
}

void TwoStatePreference::syncSummaryView(View* view) {
    TextView* summaryView = dynamic_cast<TextView*>(view);
    if (summaryView == nullptr) {
        return;
    }
    bool useDefaultSummary = true;
    if (mChecked && !mSummaryOn.empty()) {
        summaryView->setText(mSummaryOn);
        useDefaultSummary = false;
    } else if (!mChecked && !mSummaryOff.empty()) {
        summaryView->setText(mSummaryOff);
        useDefaultSummary = false;
    }
    if (useDefaultSummary) {
        const std::string summary = getSummary();
        if (!summary.empty()) {
            summaryView->setText(summary);
            useDefaultSummary = false;
        }
    }
    int newVisibility = View::GONE;
    if (!useDefaultSummary) {
        // Someone has written to it
        newVisibility = View::VISIBLE;
    }
    if (newVisibility != summaryView->getVisibility()) {
        summaryView->setVisibility(newVisibility);
    }
}

Parcelable* TwoStatePreference::onSaveInstanceState() {
    Parcelable* superState = Preference::onSaveInstanceState();
    if (isPersistent()) {
        // No need to save instance state since it's persistent
        return superState;
    }

    SavedState* myState = new SavedState(superState);
    myState->mChecked = isChecked();
    return myState;
}

void TwoStatePreference::onRestoreInstanceState(Parcelable* state) {
    if (state == nullptr || dynamic_cast<SavedState*>(state) == nullptr) {
        // Didn't save state for us in onSaveInstanceState
        Preference::onRestoreInstanceState(state);
        return;
    }

    SavedState* myState = static_cast<SavedState*>(state);
    Preference::onRestoreInstanceState(myState->getSuperState());
    setChecked(myState->mChecked);
}

TwoStatePreference::SavedState::SavedState(Parcel& source)
    : Preference::BaseSavedState(source) {
    mChecked = source.readInt() == 1;
}

TwoStatePreference::SavedState::SavedState(Parcelable* superState)
    : Preference::BaseSavedState(superState), mChecked(false) {
}

void TwoStatePreference::SavedState::writeToParcel(Parcel& dest, int flags) {
    BaseSavedState::writeToParcel(dest, flags);
    dest.writeInt(mChecked ? 1 : 0);
}

} // namespace cdroid
