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
#ifndef __CDROID_PREFERENCE_EXPAND_BUTTON_H__
#define __CDROID_PREFERENCE_EXPAND_BUTTON_H__

#include <vector>
#include <preference/preference.h>

namespace cdroid {

/**
 * Port of androidx.preference.ExpandButton — a Preference that visually wraps
 * preferences collapsed in a PreferenceGroup, and expands those preferences
 * into the group when tapped.
 */
class ExpandButton final : public Preference {
public:
    ExpandButton(Context& context, const std::vector<Preference*>& collapsedPreferences,
            int64_t parentId);

    void onBindViewHolder(PreferenceViewHolder& holder) override;

    long getId() const;

    std::string getPreferenceClassName() const override { return "ExpandButton"; }

private:
    void initLayout();
    void setSummary(const std::vector<Preference*>& collapsedPreferences);

    int64_t mExpandId;
};

} // namespace cdroid

#endif // __CDROID_PREFERENCE_EXPAND_BUTTON_H__
