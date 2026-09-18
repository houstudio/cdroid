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
#include <preference/preferencescreen.h>
#include <preference/preferencemanager.h>
#include <widget/internal_R.h>

#include <preference/preferenceinflater.h>

namespace cdroid {

PreferenceScreen::PreferenceScreen(Context& context, const AttributeSet& attrs)
    : PreferenceGroup(context, attrs, (int)internal::R::attr::preferenceScreenStyle) {
}

void PreferenceScreen::onClick() {
    if (getIntent() != nullptr || !getFragment().empty() || getPreferenceCount() == 0) {
        return;
    }
    auto listener = getPreferenceManager()->getOnNavigateToScreenListener();
    if (listener) {
        listener(*this);
    }
}

bool PreferenceScreen::isOnSameScreenAsChildren() const {
    return false;
}

bool PreferenceScreen::shouldUseGeneratedIds() const {
    return mShouldUseGeneratedIds;
}

void PreferenceScreen::setShouldUseGeneratedIds(bool shouldUseGeneratedIds) {
    if (isAttached()) {
        throw std::logic_error(
                "Cannot change the usage of generated IDs while attached to the preference hierarchy");
    }
    mShouldUseGeneratedIds = shouldUseGeneratedIds;
}

DECLARE_PREFERENCE(PreferenceScreen)

} // namespace cdroid

