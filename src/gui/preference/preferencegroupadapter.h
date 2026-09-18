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
#ifndef __CDROID_PREFERENCE_GROUP_ADAPTER_H__
#define __CDROID_PREFERENCE_GROUP_ADAPTER_H__

#include <vector>
#include <memory>
#include <widgetEx/recyclerview/recyclerview.h>
#include <preference/preference.h>
#include <preference/preferencegroup.h>

namespace cdroid {

class Drawable;
class Handler;
class PreferenceGroupAdapter;

/**
 * Port of androidx.preference.PreferenceGroupAdapter — an adapter that
 * connects a RecyclerView to the Preferences contained in an associated
 * PreferenceGroup.
 */
class PreferenceGroupAdapter
    : public RecyclerView::Adapter,
      public PreferenceGroup::PreferencePositionCallback {
public:
    explicit PreferenceGroupAdapter(PreferenceGroup& preferenceGroup);
    ~PreferenceGroupAdapter() override;

    /**
     * Updates mPreferences and mVisiblePreferences as well as notifying
     * RecyclerView of any changes.
     */
    void updatePreferences();

    /**
     * Recursively builds a list containing a flattened representation of a
     * given PreferenceGroup, which may itself contain nested PreferenceGroups
     * with their own Preferences.
     */
    void flattenPreferenceGroup(std::vector<Preference*>& preferences, PreferenceGroup& group);

    /**
     * Recursively generates a list of Preferences visible to the user.
     */
    std::vector<Preference*> createVisiblePreferencesList(PreferenceGroup& group);

    /**
     * Creates an ExpandButton for a given PreferenceGroup that will expand the
     * group when clicked, showing preferences previously collapsed by the group.
     */
    Preference* createExpandButton(PreferenceGroup& group,
            const std::vector<Preference*>& collapsedPreferences);

    /**
     * Helper method to return whether a group allows hiding some of its
     * preferences into an ExpandButton.
     */
    static bool isGroupExpandable(PreferenceGroup& preferenceGroup);

    /**
     * Returns the Preference at the given position.
     */
    Preference* getItem(int position) const;

    int getItemCount() override;
    long getItemId(int position) override;

    void onPreferenceChange(Preference& preference);
    void onPreferenceHierarchyChange(Preference& preference);
    void onPreferenceVisibilityChange(Preference& preference);

    int getItemViewType(int position) override;
    RecyclerView::ViewHolder* onCreateViewHolder(ViewGroup* parent, int viewType) override;
    void onBindViewHolder(RecyclerView::ViewHolder& holder, int position) override;

    int getPreferenceAdapterPosition(const std::string& key) override;
    int getPreferenceAdapterPosition(Preference* preference) override;

private:
    /**
     * Describes a unique combination of layout resource, widget layout
     * resource, and class name (PreferenceResourceDescriptor).
     */
    struct PreferenceResourceDescriptor {
        int mLayoutResId;
        int mWidgetLayoutResId;
        std::string mClassName;

        PreferenceResourceDescriptor(const Preference& preference);
        bool operator==(const PreferenceResourceDescriptor& other) const;
        bool operator!=(const PreferenceResourceDescriptor& other) const {
            return !(*this == other);
        }
    };

    /** The PreferenceGroup that we build a list of preferences from. */
    PreferenceGroup& mPreferenceGroup;

    /**
     * Contains a sorted list of all Preferences in this adapter regardless of
     * visibility. This is used to construct mVisiblePreferences.
     */
    std::vector<Preference*> mPreferences;

    /**
     * Contains a sorted list of all Preferences in this adapter that are
     * visible to the user and hence displayed in the attached RecyclerView.
     */
    std::vector<Preference*> mVisiblePreferences;

    /**
     * List of unique PreferenceResourceDescriptors, used to cache item view
     * types for RecyclerView.
     */
    std::vector<PreferenceResourceDescriptor> mPreferenceResourceDescriptors;

    // android: the adapter implements Preference.OnPreferenceChangeInternalListener and
    // registers `this`. Now an EventSet value member whose callbacks are wired with
    // lambdas in the ctor and set on the group and each flattened preference (copies
    // share EventSet mID). No subclass / no class pointer.
    Preference::OnPreferenceChangeInternalListener mInternalListener;

    std::unique_ptr<Handler> mHandler;
    Runnable mSyncRunnable;
};

} // namespace cdroid

#endif // __CDROID_PREFERENCE_GROUP_ADAPTER_H__
