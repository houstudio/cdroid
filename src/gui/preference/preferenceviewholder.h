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
#ifndef __CDROID_PREFERENCE_VIEW_HOLDER_H__
#define __CDROID_PREFERENCE_VIEW_HOLDER_H__

#include <map>
#include <widgetEx/recyclerview/recyclerview.h>
#include <drawable/colorstatelist.h>

namespace cdroid {

class Drawable;

/**
 * Port of androidx.preference.PreferenceViewHolder — a RecyclerView.ViewHolder
 * class which caches views associated with the default Preference layouts.
 * Cached views can be retrieved by calling findViewById(int).
 */
class PreferenceViewHolder : public RecyclerView::ViewHolder {
public:
    explicit PreferenceViewHolder(View* itemView);

    /**
     * Returns a cached reference to a subview managed by this object.
     */
    View* findViewById(int id);

    /**
     * Dividers are only drawn between items if both items allow it, or above
     * the first and below the last item if that item allows it.
     */
    bool isDividerAllowedAbove() const;
    void setDividerAllowedAbove(bool allowed);

    bool isDividerAllowedBelow() const;
    void setDividerAllowedBelow(bool allowed);

    /**
     * Resets the state of properties modified by
     * Preference#onBindViewHolder to ensure that we don't keep stale state
     * for a different Preference around.
     */
    void resetState();

private:
    Drawable* mBackground;
    RefPtr<ColorStateList> mTitleTextColors;
    std::map<int, View*> mCachedViews;
    bool mDividerAllowedAbove = false;
    bool mDividerAllowedBelow = false;
};

} // namespace cdroid

#endif // __CDROID_PREFERENCE_VIEW_HOLDER_H__
