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
#include <preference/preferencecategory.h>
#include <widget/internal_R.h>

#include <preference/preferenceinflater.h>

namespace cdroid {

PreferenceCategory::PreferenceCategory(Context& context, const AttributeSet& attrs,
        int defStyleAttr, int defStyleRes)
    : PreferenceGroup(context, attrs, defStyleAttr, defStyleRes) {
}

PreferenceCategory::PreferenceCategory(Context& context, const AttributeSet& attrs,
        int defStyleAttr)
    : PreferenceGroup(context, attrs, defStyleAttr) {
}

PreferenceCategory::PreferenceCategory(Context& context, const AttributeSet& attrs)
    : PreferenceGroup(context, attrs, (int)internal::R::attr::preferenceCategoryStyle) {
}

PreferenceCategory::PreferenceCategory(Context& context)
    : PreferenceCategory(context, AttributeSet()) {
}

bool PreferenceCategory::isEnabled() const {
    return false;
}

bool PreferenceCategory::shouldDisableDependents() const {
    return !PreferenceGroup::isEnabled();
}

void PreferenceCategory::onBindViewHolder(PreferenceViewHolder& holder) {
    PreferenceGroup::onBindViewHolder(holder);
    // setAccessibilityHeading (API 28) has no CDROID counterpart — skipped
    // (a11y is out of scope for this port).
}

DECLARE_PREFERENCE(PreferenceCategory)

} // namespace cdroid

