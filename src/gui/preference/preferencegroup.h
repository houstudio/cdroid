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
#ifndef __CDROID_PREFERENCE_GROUP_H__
#define __CDROID_PREFERENCE_GROUP_H__

#include <map>
#include <string>
#include <vector>
#include <functional>
#include <preference/preference.h>
#include <core/handler.h>

namespace cdroid {

class Handler;
class PreferenceDataStore;

/**
 * Port of androidx.preference.PreferenceGroup — a container for multiple
 * Preferences. It is a base class for preference objects that are parents,
 * such as PreferenceCategory and PreferenceScreen.
 */
class PreferenceGroup : public Preference {
public:
    /**
     * Interface for PreferenceGroup adapters to implement so that
     * PreferenceFragment#scrollToPreference can determine the correct scroll
     * position to request.
     */
    class PreferencePositionCallback {
    public:
        virtual ~PreferencePositionCallback() = default;
        /** Returns the adapter position of the first Preference with the key. */
        virtual int getPreferenceAdapterPosition(const std::string& key) = 0;
        /** Returns the adapter position of the specified Preference object. */
        virtual int getPreferenceAdapterPosition(Preference* preference) = 0;
    };

    /**
     * Definition for a callback to be invoked when the expand button is clicked.
     */
    using OnExpandButtonClickListener = std::function<void()>;

    PreferenceGroup(Context& context, const AttributeSet& attrs);
    PreferenceGroup(Context& context, const AttributeSet& attrs, int defStyleAttr);
    PreferenceGroup(Context& context, const AttributeSet& attrs, int defStyleAttr, int defStyleRes);

    void setOrderingAsAdded(bool orderingAsAdded);
    bool isOrderingAsAdded() const;

    /**
     * Sets the maximal number of children that are shown when the preference
     * group is launched, collapsing the rest behind an expand button.
     */
    void setInitialExpandedChildrenCount(int expandedCount);
    int getInitialExpandedChildrenCount() const;

    /** Called by the inflater to add an item to this group. */
    void addItemFromInflater(Preference* preference);

    int getPreferenceCount() const;
    Preference* getPreference(int index) const;

    /**
     * Adds a Preference at the correct position based on the preference's order.
     */
    bool addPreference(Preference* preference);

    /**
     * Removes a Preference from this group (not recursive).
     */
    bool removePreference(Preference* preference);

    /**
     * Recursively finds and removes a Preference from this group or a nested
     * group lower down in the hierarchy.
     */
    bool removePreferenceRecursively(const std::string& key);

    /** Removes all Preferences from this group. */
    void removeAll();

    /**
     * Finds a Preference based on its key, searching nested PreferenceGroups
     * recursively.
     */
    Preference* findPreference(const std::string& key) const;

    /**
     * Whether this preference group should be shown on the same screen as its
     * contained preferences.
     */
    virtual bool isOnSameScreenAsChildren() const;

    bool isAttached() const;

    void setOnExpandButtonClickListener(const OnExpandButtonClickListener& listener);
    OnExpandButtonClickListener getOnExpandButtonClickListener() const;

    void onAttached() override;
    void onDetached() override;
    void notifyDependencyChange(bool disableDependents) override;

    void sortPreferences();

    std::string getPreferenceClassName() const override { return "PreferenceGroup"; }

    class SavedState : public Preference::BaseSavedState {
    public:
        int mInitialExpandedChildrenCount;
        SavedState(Parcel& source);
        SavedState(Parcelable* superState, int initialExpandedChildrenCount);
        void writeToParcel(Parcel& dest, int flags) override;
    };

protected:
    /**
     * Prepares a Preference to be added to the group.
     */
    virtual bool onPrepareAddPreference(Preference* preference);

    Parcelable* onSaveInstanceState() override;
    void onRestoreInstanceState(Parcelable* state) override;
    void dispatchSaveInstanceState(Bundle& container) override;
    void dispatchRestoreInstanceState(Bundle& container) override;

private:
    bool removePreferenceInt(Preference* preference);

    static constexpr int DEFAULT_INITIAL_EXPANDED_COUNT = 0x7FFFFFFF;

    // SimpleArrayMap<String, Long> — key recycle cache for stable RecyclerView ids.
    std::map<std::string, int64_t> mIdRecycleCache;
    std::unique_ptr<Handler> mHandler;
    Runnable mClearRecycleCacheRunnable;
    /**
     * The container for child Preferences. This is sorted based on the
     * ordering, please use #addPreference instead of adding to this directly.
     */
    std::vector<Preference*> mPreferences;
    bool mOrderingAsAdded = true;
    int mCurrentPreferenceOrder = 0;
    bool mAttachedToHierarchy = false;
    int mInitialExpandedChildrenCount = DEFAULT_INITIAL_EXPANDED_COUNT;
    OnExpandButtonClickListener mOnExpandButtonClickListener;
};

} // namespace cdroid

#endif // __CDROID_PREFERENCE_GROUP_H__
