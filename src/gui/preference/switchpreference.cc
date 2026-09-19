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
#include <preference/switchpreference.h>
#include <preference/preferenceviewholder.h>
#include <preference/androidresources.h>
#include <widget/internal_R.h>
#include <widget/framework_styleable.h>
#include <widget/compoundbutton.h>
#include <widget/switch.h>
#include <widget/checkable.h>
#include <view/view.h>

#include <preference/preferenceinflater.h>

namespace cdroid {

SwitchPreference::SwitchPreference(Context& context, const AttributeSet& attrs,
        int defStyleAttr, int defStyleRes)
    : TwoStatePreference(context, attrs, defStyleAttr, defStyleRes) {
    namespace ns = internal::R::styleable;

    mListener = [this](CompoundButton& buttonView, bool isChecked) {
        if (!callChangeListener(isChecked)) {
            // Listener didn't like it, change it back.
            // CompoundButton will make sure we don't recurse.
            buttonView.setChecked(!isChecked);
            return;
        }
        setChecked(isChecked);
    };

    auto a = context.obtainStyledAttributes(attrs, ns::SwitchPreference, defStyleAttr, defStyleRes);

    setSummaryOn(a->getString(ns::SwitchPreference_summaryOn));
    setSummaryOff(a->getString(ns::SwitchPreference_summaryOff));
    setSwitchTextOn(a->getString(ns::SwitchPreference_switchTextOn));
    setSwitchTextOff(a->getString(ns::SwitchPreference_switchTextOff));
    setDisableDependentsState(a->getBoolean(ns::SwitchPreference_disableDependentsState, false));
}

SwitchPreference::SwitchPreference(Context& context, const AttributeSet& attrs, int defStyleAttr)
    : SwitchPreference(context, attrs, defStyleAttr, 0) {
}

SwitchPreference::SwitchPreference(Context& context, const AttributeSet& attrs)
    : SwitchPreference(context, attrs, (int)internal::R::attr::switchPreferenceStyle) {
}

SwitchPreference::SwitchPreference(Context& context)
    : SwitchPreference(context, AttributeSet()) {
}

void SwitchPreference::onBindViewHolder(PreferenceViewHolder& holder) {
    TwoStatePreference::onBindViewHolder(holder);
    View* switchView = holder.findViewById(AndroidResources::ANDROID_R_SWITCH_WIDGET);
    syncSwitchView(switchView);
    syncSummaryView(holder);
}

void SwitchPreference::setSwitchTextOn(const std::string& onText) {
    mSwitchOn = onText;
    notifyChanged();
}

void SwitchPreference::setSwitchTextOn(int resId) {
    setSwitchTextOn(getContext().getString(resId));
}

std::string SwitchPreference::getSwitchTextOn() const {
    return mSwitchOn;
}

void SwitchPreference::setSwitchTextOff(const std::string& offText) {
    mSwitchOff = offText;
    notifyChanged();
}

void SwitchPreference::setSwitchTextOff(int resId) {
    setSwitchTextOff(getContext().getString(resId));
}

std::string SwitchPreference::getSwitchTextOff() const {
    return mSwitchOff;
}

void SwitchPreference::performClick(View* view) {
    TwoStatePreference::performClick(view);
    // syncViewIfAccessibilityEnabled omitted: CDROID has no accessibility
    // manager (a11y is out of scope for this port).
}

void SwitchPreference::syncSwitchView(View* view) {
    Switch* switchView = dynamic_cast<Switch*>(view);
    if (switchView != nullptr) {
        switchView->setOnCheckedChangeListener(nullptr);
    }
    auto* checkable = dynamic_cast<Checkable*>(view);
    if (checkable != nullptr) {
        checkable->setChecked(mChecked);
    }
    if (switchView != nullptr) {
        switchView->setTextOn(mSwitchOn);
        switchView->setTextOff(mSwitchOff);
        switchView->setOnCheckedChangeListener(mListener);
    }
}

DECLARE_PREFERENCE(SwitchPreference)

} // namespace cdroid

