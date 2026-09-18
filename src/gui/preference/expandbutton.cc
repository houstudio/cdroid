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
#include <preference/expandbutton.h>
#include <preference/preferencegroup.h>
#include <preference/preferenceviewholder.h>
#include <widget/internal_R.h>

namespace cdroid {

ExpandButton::ExpandButton(Context& context,
        const std::vector<Preference*>& collapsedPreferences, int64_t parentId)
    : Preference(context), mExpandId(parentId + 1000000) {
    initLayout();
    setSummary(collapsedPreferences);
    // Since IDs are unique, using the parentId as a reference ensures that this expand
    // button will have a unique ID and hence transitions will be correctly animated by
    // RecyclerView when there are multiple expand buttons.
}

void ExpandButton::initLayout() {
    setLayoutResource((int)internal::R::layout::expand_button);
    setIcon((int)internal::R::drawable::ic_arrow_down_24dp);
    setTitle((int)internal::R::string::expand_button_title);
    // Sets a high order so that the expand button will be placed at the bottom of the group
    setOrder(999);
}

void ExpandButton::setSummary(const std::vector<Preference*>& collapsedPreferences) {
    std::string summary;
    std::vector<PreferenceGroup*> parents;

    for (Preference* preference : collapsedPreferences) {
        const std::string title = preference->getTitle();
        PreferenceGroup* group = dynamic_cast<PreferenceGroup*>(preference);
        if (group != nullptr && !title.empty()) {
            parents.push_back(group);
        }
        if (std::find(parents.begin(), parents.end(), preference->getParent()) != parents.end()) {
            if (group != nullptr) {
                parents.push_back(group);
            }
            continue;
        }
        if (!title.empty()) {
            if (summary.empty()) {
                summary = title;
            } else {
                // AOSP: getString(R.string.summary_collapsed_preference_list, summary, title)
                // — CDROID has no formatted getString; the "%1$s", "%2$s"
                // pattern is inlined here.
                summary = "\"" + summary + "\", \"" + title + "\"";
            }
        }
    }
    Preference::setSummary(summary);
}

void ExpandButton::onBindViewHolder(PreferenceViewHolder& holder) {
    Preference::onBindViewHolder(holder);
    holder.setDividerAllowedAbove(false);
}

long ExpandButton::getId() const {
    return mExpandId;
}

} // namespace cdroid
