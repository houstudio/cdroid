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
#ifndef __CDROID_PREFERENCE_CATEGORY_H__
#define __CDROID_PREFERENCE_CATEGORY_H__

#include <preference/preferencegroup.h>

namespace cdroid {

/**
 * Port of androidx.preference.PreferenceCategory — a container that is used to
 * group similar Preferences. A PreferenceCategory displays a category title
 * and visually separates groups of Preferences.
 */
class PreferenceCategory : public PreferenceGroup {
public:
    PreferenceCategory(Context& context);
    PreferenceCategory(Context& context, const AttributeSet& attrs);
    PreferenceCategory(Context& context, const AttributeSet& attrs, int defStyleAttr);
    PreferenceCategory(Context& context, const AttributeSet& attrs,
            int defStyleAttr, int defStyleRes);

    bool isEnabled() const override;
    bool shouldDisableDependents() const override;
    void onBindViewHolder(PreferenceViewHolder& holder) override;

    std::string getPreferenceClassName() const override { return "PreferenceCategory"; }
};

} // namespace cdroid

#endif // __CDROID_PREFERENCE_CATEGORY_H__
