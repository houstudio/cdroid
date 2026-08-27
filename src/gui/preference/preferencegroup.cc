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
#include <content/typedarray.h>
#include <preference/preferencegroup.h>
#include <preference/preferencemanager.h>
#include <preference/preferencedatastore.h>
#include <preference/preferencescreen.h>
#include <core/handler.h>
#include <core/looper.h>
#include <core/parcel.h>
#include <widget/internal_R.h>
#include <widget/framework_styleable.h>
#include <algorithm>
#include <porting/cdlog.h>

namespace cdroid {

PreferenceGroup::PreferenceGroup(Context& context, const AttributeSet& attrs,
        int defStyleAttr, int defStyleRes)
    : Preference(context, attrs, defStyleAttr, defStyleRes) {
    namespace ns = internal::R::styleable;

    mHandler = std::make_unique<Handler>(Looper::getMainLooper());
    mClearRecycleCacheRunnable = [this]() {
        mIdRecycleCache.clear();
    };

    auto a = context.obtainStyledAttributes(attrs, ns::PreferenceGroup, defStyleAttr, defStyleRes);

    mOrderingAsAdded = a->getBoolean(ns::PreferenceGroup_orderingFromXml, true);

    if (a->hasValue(ns::PreferenceGroup_initialExpandedChildrenCount)) {
        setInitialExpandedChildrenCount(a->getInt(
                ns::PreferenceGroup_initialExpandedChildrenCount, DEFAULT_INITIAL_EXPANDED_COUNT));
    }
}

PreferenceGroup::PreferenceGroup(Context& context, const AttributeSet& attrs, int defStyleAttr)
    : PreferenceGroup(context, attrs, defStyleAttr, 0) {
}

PreferenceGroup::PreferenceGroup(Context& context, const AttributeSet& attrs)
    : PreferenceGroup(context, attrs, 0) {
}

void PreferenceGroup::setOrderingAsAdded(bool orderingAsAdded) {
    mOrderingAsAdded = orderingAsAdded;
}

bool PreferenceGroup::isOrderingAsAdded() const {
    return mOrderingAsAdded;
}

void PreferenceGroup::setInitialExpandedChildrenCount(int expandedCount) {
    if (expandedCount != DEFAULT_INITIAL_EXPANDED_COUNT && !hasKey()) {
        LOGE("PreferenceGroup should have a key defined if it contains an expandable preference");
    }
    mInitialExpandedChildrenCount = expandedCount;
}

int PreferenceGroup::getInitialExpandedChildrenCount() const {
    return mInitialExpandedChildrenCount;
}

void PreferenceGroup::addItemFromInflater(Preference* preference) {
    addPreference(preference);
}

int PreferenceGroup::getPreferenceCount() const {
    return (int)mPreferences.size();
}

Preference* PreferenceGroup::getPreference(int index) const {
    return mPreferences[index];
}

bool PreferenceGroup::addPreference(Preference* preference) {
    if (std::find(mPreferences.begin(), mPreferences.end(), preference) != mPreferences.end()) {
        return true;
    }
    if (!preference->getKey().empty()) {
        PreferenceGroup* root = this;
        while (root->getParent() != nullptr) {
            root = root->getParent();
        }
        const std::string& key = preference->getKey();
        if (root->findPreference(key) != nullptr) {
            LOGE("Found duplicated key: \"%s\". This can cause unintended behaviour,"
                 " please use unique keys for every preference.", key.c_str());
        }
    }

    if (preference->getOrder() == DEFAULT_ORDER) {
        if (mOrderingAsAdded) {
            preference->setOrder(mCurrentPreferenceOrder++);
        }

        if (dynamic_cast<PreferenceGroup*>(preference) != nullptr) {
            // TODO: fix (method is called tail recursively when inflating,
            // so we won't end up properly passing this flag down to children
            static_cast<PreferenceGroup*>(preference)->setOrderingAsAdded(mOrderingAsAdded);
        }
    }

    // Collections.binarySearch equivalent: find the insertion point in the
    // comparator-sorted list (Preference::operator< provides the ordering).
    auto it = std::lower_bound(mPreferences.begin(), mPreferences.end(), preference,
                               [](const Preference* a, const Preference* b) { return *a < *b; });
    const int insertionIndex = (int)(it - mPreferences.begin());

    if (!onPrepareAddPreference(preference)) {
        return false;
    }

    mPreferences.insert(mPreferences.begin() + insertionIndex, preference);

    PreferenceManager* preferenceManager = getPreferenceManager();
    const std::string& key = preference->getKey();
    int64_t id;
    auto cached = mIdRecycleCache.find(key);
    if (!key.empty() && cached != mIdRecycleCache.end()) {
        id = cached->second;
        mIdRecycleCache.erase(cached);
    } else {
        id = preferenceManager->getNextId();
    }
    preference->onAttachedToHierarchy(*preferenceManager, id);
    preference->assignParent(this);

    if (mAttachedToHierarchy) {
        preference->onAttached();
    }

    notifyHierarchyChanged();

    return true;
}

bool PreferenceGroup::removePreference(Preference* preference) {
    const bool returnValue = removePreferenceInt(preference);
    notifyHierarchyChanged();
    return returnValue;
}

bool PreferenceGroup::removePreferenceRecursively(const std::string& key) {
    Preference* preference = findPreference(key);
    if (preference == nullptr) {
        return false;
    }
    return preference->getParent()->removePreference(preference);
}

bool PreferenceGroup::removePreferenceInt(Preference* preference) {
    preference->onPrepareForRemoval();
    if (preference->getParent() == this) {
        preference->assignParent(nullptr);
    }
    auto it = std::find(mPreferences.begin(), mPreferences.end(), preference);
    const bool success = (it != mPreferences.end());
    if (success) {
        // If this preference, or another preference with the same key, gets re-added
        // immediately, we want it to have the same id so that it can be correctly tracked
        // in the adapter by RecyclerView, to make it appear as if it has only been
        // seamlessly updated. If the preference is not re-added by the time the handler
        // runs, we take that as a signal that the preference will not be re-added soon
        // in which case it does not need to retain the same id.
        const std::string& key = preference->getKey();
        if (!key.empty()) {
            mIdRecycleCache[key] = preference->getId();
            mHandler->removeCallbacks(mClearRecycleCacheRunnable);
            mHandler->post(mClearRecycleCacheRunnable);
        }
        if (mAttachedToHierarchy) {
            preference->onDetached();
        }
        mPreferences.erase(it);
    }

    return success;
}

void PreferenceGroup::removeAll() {
    std::vector<Preference*> preferences = mPreferences;
    for (size_t i = preferences.size(); i > 0; i--) {
        // AOSP removes get(0) from the live list each iteration
        // (PreferenceGroup.java:337-345).
        removePreferenceInt(mPreferences[0]);
    }
    notifyHierarchyChanged();
}

bool PreferenceGroup::onPrepareAddPreference(Preference* preference) {
    preference->onParentChanged(*this, shouldDisableDependents());
    return true;
}

Preference* PreferenceGroup::findPreference(const std::string& key) const {
    if (key.empty()) {
        throw std::invalid_argument("Key cannot be null");
    }
    if (getKey() == key) {
        return const_cast<PreferenceGroup*>(this);
    }
    const int preferenceCount = getPreferenceCount();
    for (int i = 0; i < preferenceCount; i++) {
        Preference* preference = getPreference(i);
        const std::string& curKey = preference->getKey();

        if (curKey == key) {
            return preference;
        }

        PreferenceGroup* group = dynamic_cast<PreferenceGroup*>(preference);
        if (group != nullptr) {
            Preference* returnedPreference = group->findPreference(key);
            if (returnedPreference != nullptr) {
                return returnedPreference;
            }
        }
    }
    return nullptr;
}

bool PreferenceGroup::isOnSameScreenAsChildren() const {
    return true;
}

bool PreferenceGroup::isAttached() const {
    return mAttachedToHierarchy;
}

void PreferenceGroup::setOnExpandButtonClickListener(
        const OnExpandButtonClickListener& onExpandButtonClickListener) {
    mOnExpandButtonClickListener = onExpandButtonClickListener;
}

PreferenceGroup::OnExpandButtonClickListener
PreferenceGroup::getOnExpandButtonClickListener() const {
    return mOnExpandButtonClickListener;
}

void PreferenceGroup::onAttached() {
    Preference::onAttached();

    // Mark as attached so if a preference is later added to this group, we
    // can tell it we are already attached
    mAttachedToHierarchy = true;

    // Dispatch to all contained preferences
    const int preferenceCount = getPreferenceCount();
    for (int i = 0; i < preferenceCount; i++) {
        getPreference(i)->onAttached();
    }
}

void PreferenceGroup::onDetached() {
    Preference::onDetached();

    // We won't be attached to the activity anymore
    mAttachedToHierarchy = false;

    // Dispatch to all contained preferences
    const int preferenceCount = getPreferenceCount();
    for (int i = 0; i < preferenceCount; i++) {
        getPreference(i)->onDetached();
    }
}

void PreferenceGroup::notifyDependencyChange(bool disableDependents) {
    Preference::notifyDependencyChange(disableDependents);

    // Child preferences have an implicit dependency on their containing
    // group. Dispatch dependency change to all contained preferences.
    const int preferenceCount = getPreferenceCount();
    for (int i = 0; i < preferenceCount; i++) {
        getPreference(i)->onParentChanged(*this, disableDependents);
    }
}

void PreferenceGroup::sortPreferences() {
    std::sort(mPreferences.begin(), mPreferences.end(),
              [](const Preference* a, const Preference* b) { return *a < *b; });
}

void PreferenceGroup::dispatchSaveInstanceState(Bundle& container) {
    Preference::dispatchSaveInstanceState(container);

    // Dispatch to all contained preferences
    const int preferenceCount = getPreferenceCount();
    for (int i = 0; i < preferenceCount; i++) {
        getPreference(i)->dispatchSaveInstanceState(container);
    }
}

void PreferenceGroup::dispatchRestoreInstanceState(Bundle& container) {
    Preference::dispatchRestoreInstanceState(container);

    // Dispatch to all contained preferences
    const int preferenceCount = getPreferenceCount();
    for (int i = 0; i < preferenceCount; i++) {
        getPreference(i)->dispatchRestoreInstanceState(container);
    }
}

Parcelable* PreferenceGroup::onSaveInstanceState() {
    Parcelable* superState = Preference::onSaveInstanceState();
    return new SavedState(superState, mInitialExpandedChildrenCount);
}

void PreferenceGroup::onRestoreInstanceState(Parcelable* state) {
    if (state == nullptr || dynamic_cast<SavedState*>(state) == nullptr) {
        // Didn't save state for us in saveInstanceState
        Preference::onRestoreInstanceState(state);
        return;
    }
    SavedState* groupState = static_cast<SavedState*>(state);
    mInitialExpandedChildrenCount = groupState->mInitialExpandedChildrenCount;
    Preference::onRestoreInstanceState(groupState->getSuperState());
}

PreferenceGroup::SavedState::SavedState(Parcel& source)
    : Preference::BaseSavedState(source) {
    mInitialExpandedChildrenCount = source.readInt();
}

PreferenceGroup::SavedState::SavedState(Parcelable* superState, int initialExpandedChildrenCount)
    : Preference::BaseSavedState(superState) {
    mInitialExpandedChildrenCount = initialExpandedChildrenCount;
}

void PreferenceGroup::SavedState::writeToParcel(Parcel& dest, int flags) {
    BaseSavedState::writeToParcel(dest, flags);
    dest.writeInt(mInitialExpandedChildrenCount);
}

} // namespace cdroid
