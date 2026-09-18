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
#include <preference/checkboxpreference.h>
#include <preference/preferenceviewholder.h>
#include <widget/internal_R.h>
#include <widget/framework_styleable.h>
#include <widget/compoundbutton.h>
#include <widget/checkable.h>
#include <view/view.h>

#include <preference/preferenceinflater.h>

namespace cdroid {

CheckBoxPreference::CheckBoxPreference(Context& context, const AttributeSet& attrs,
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

    auto a = context.obtainStyledAttributes(attrs, ns::CheckBoxPreference, defStyleAttr, defStyleRes);

    setSummaryOn(a->getString(ns::CheckBoxPreference_summaryOn));
    setSummaryOff(a->getString(ns::CheckBoxPreference_summaryOff));
    setDisableDependentsState(a->getBoolean(ns::CheckBoxPreference_disableDependentsState, false));
}

CheckBoxPreference::CheckBoxPreference(Context& context, const AttributeSet& attrs,
        int defStyleAttr)
    : CheckBoxPreference(context, attrs, defStyleAttr, 0) {
}

CheckBoxPreference::CheckBoxPreference(Context& context, const AttributeSet& attrs)
    : CheckBoxPreference(context, attrs, (int)internal::R::attr::checkBoxPreferenceStyle) {
}

CheckBoxPreference::CheckBoxPreference(Context& context)
    : CheckBoxPreference(context, AttributeSet()) {
}

void CheckBoxPreference::onBindViewHolder(PreferenceViewHolder& holder) {
    TwoStatePreference::onBindViewHolder(holder);

    syncCheckboxView(holder.findViewById((int)internal::R::id::checkbox));

    syncSummaryView(holder);
}

void CheckBoxPreference::performClick(View* view) {
    TwoStatePreference::performClick(view);
    // syncViewIfAccessibilityEnabled omitted: CDROID has no accessibility
    // manager (a11y is out of scope for this port).
}

void CheckBoxPreference::syncCheckboxView(View* view) {
    if (dynamic_cast<CompoundButton*>(view) != nullptr) {
        static_cast<CompoundButton*>(view)->setOnCheckedChangeListener(nullptr);
    }
    auto* checkable = dynamic_cast<Checkable*>(view);
    if (checkable != nullptr) {
        checkable->setChecked(mChecked);
    }
    if (dynamic_cast<CompoundButton*>(view) != nullptr) {
        static_cast<CompoundButton*>(view)->setOnCheckedChangeListener(mListener);
    }
}

DECLARE_PREFERENCE(CheckBoxPreference)

} // namespace cdroid

