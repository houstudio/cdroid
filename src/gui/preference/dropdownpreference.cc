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
#include <preference/dropdownpreference.h>
#include <preference/preferenceviewholder.h>
#include <widget/internal_R.h>
#include <widget/framework_styleable.h>
#include <preference/preferenceinflater.h>
#include <widget/spinner.h>
#include <widget/adapterview.h>
#include <widget/adapter.h>
#include <content/typedarray.h>

namespace cdroid {

DropDownPreference::DropDownPreference(Context& context, const AttributeSet& attrs,
        int defStyleAttr, int defStyleRes)
    : ListPreference(context, attrs, defStyleAttr, defStyleRes), mContext(context) {
    setWidgetLayoutResource((int)internal::R::layout::preference_dropdown_widget);

    mItemSelectedListener.onItemSelected =
        [this](AdapterView& /*parent*/, View& /*view*/, int position, long /*id*/) {
            if (position >= 0) {
                const std::string value = getEntryValues()[position];
                if (value != getValue() && callChangeListener(value)) {
                    setValue(value);
                }
            }
        };
    mItemSelectedListener.onNothingSelected = [](AdapterView&) {
        // noop
    };

    mAdapter = createAdapter();
    updateEntries();
}

DropDownPreference::DropDownPreference(Context& context, const AttributeSet& attrs,
        int defStyleAttr)
    : DropDownPreference(context, attrs, defStyleAttr, 0) {
}

DropDownPreference::DropDownPreference(Context& context, const AttributeSet& attrs)
    : DropDownPreference(context, attrs, (int)internal::R::attr::dropdownPreferenceStyle) {
}

DropDownPreference::DropDownPreference(Context& context)
    : DropDownPreference(context, AttributeSet()) {
}

void DropDownPreference::onClick() {
    if (mSpinner != nullptr) {
        mSpinner->performClick();
    }
}

void DropDownPreference::setEntries(const std::vector<std::string>& entries) {
    ListPreference::setEntries(entries);
    updateEntries();
}

Adapter* DropDownPreference::createAdapter() {
    // AOSP: new ArrayAdapter<>(context, android.R.layout.simple_spinner_dropdown_item)
    return new ArrayAdapter<std::string>(&mContext,
            (int)internal::R::layout::simple_spinner_dropdown_item, 0);
}

void DropDownPreference::updateEntries() {
    auto* adapter = dynamic_cast<ArrayAdapter<std::string>*>(mAdapter);
    if (adapter == nullptr) return;
    adapter->clear();
    for (const auto& entry : getEntries()) {
        adapter->add(entry);
    }
}

void DropDownPreference::setValueIndex(int index) {
    setValue(getEntryValues()[index]);
}

void DropDownPreference::notifyChanged() {
    Preference::notifyChanged();
    // When setting a SummaryProvider for this Preference, this method may be
    // called before mAdapter has been set (ListPreference ctor ordering).
    if (mAdapter != nullptr) {
        mAdapter->notifyDataSetChanged();
    }
}

DropDownPreference::~DropDownPreference() {
    delete mAdapter;
}

void DropDownPreference::onBindViewHolder(PreferenceViewHolder& holder) {
    mSpinner = dynamic_cast<Spinner*>(holder.findViewById((int)internal::R::id::spinner));
    if (mSpinner != nullptr) {
        mSpinner->setAdapter(mAdapter);
        mSpinner->setOnItemSelectedListener(mItemSelectedListener);
        mSpinner->setSelection(findSpinnerIndexOfValue(getValue()));
    }
    ListPreference::onBindViewHolder(holder);
}

int DropDownPreference::findSpinnerIndexOfValue(const std::string& value) const {
    const std::vector<std::string>& entryValues = getEntryValues();
    if (!value.empty() && !entryValues.empty()) {
        for (int i = (int)entryValues.size() - 1; i >= 0; i--) {
            if (entryValues[i] == value) {
                return i;
            }
        }
    }
    return AdapterView::INVALID_POSITION;
}

DECLARE_PREFERENCE(DropDownPreference)

} // namespace cdroid
