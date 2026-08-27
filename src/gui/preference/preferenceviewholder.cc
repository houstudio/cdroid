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
#include <preference/preferenceviewholder.h>
#include <preference/androidresources.h>
#include <widget/internal_R.h>
#include <widget/textview.h>
#include <drawable/drawable.h>

namespace cdroid {

PreferenceViewHolder::PreferenceViewHolder(View* itemView)
    : RecyclerView::ViewHolder(itemView), mBackground(itemView->getBackground()) {
    // Pre-cache the views that we know in advance we'll want to find
    mCachedViews[(int)internal::R::id::title] = itemView->findViewById((int)internal::R::id::title);
    mCachedViews[(int)internal::R::id::summary] = itemView->findViewById((int)internal::R::id::summary);
    mCachedViews[(int)internal::R::id::icon] = itemView->findViewById((int)internal::R::id::icon);
    mCachedViews[(int)AndroidResources::ANDROID_R_ICON_FRAME] =
            itemView->findViewById(AndroidResources::ANDROID_R_ICON_FRAME);

    auto titleView = dynamic_cast<TextView*>(mCachedViews[(int)internal::R::id::title]);
    if (titleView != nullptr) {
        mTitleTextColors = titleView->getTextColors();
    }
}

View* PreferenceViewHolder::findViewById(int id) {
    auto cached = mCachedViews.find(id);
    if (cached != mCachedViews.end()) {
        return cached->second;
    }
    View* v = itemView->findViewById(id);
    if (v != nullptr) {
        mCachedViews[id] = v;
    }
    return v;
}

bool PreferenceViewHolder::isDividerAllowedAbove() const {
    return mDividerAllowedAbove;
}

void PreferenceViewHolder::setDividerAllowedAbove(bool allowed) {
    mDividerAllowedAbove = allowed;
}

bool PreferenceViewHolder::isDividerAllowedBelow() const {
    return mDividerAllowedBelow;
}

void PreferenceViewHolder::setDividerAllowedBelow(bool allowed) {
    mDividerAllowedBelow = allowed;
}

void PreferenceViewHolder::resetState() {
    if (itemView->getBackground() != mBackground) {
        itemView->setBackground(mBackground);
    }

    auto titleView = dynamic_cast<TextView*>(findViewById((int)internal::R::id::title));
    if (titleView != nullptr && mTitleTextColors != nullptr) {
        if (!(titleView->getTextColors() == mTitleTextColors)) {
            titleView->setTextColor(mTitleTextColors);
        }
    }
}

} // namespace cdroid
