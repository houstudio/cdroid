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
#include <preference/preferencegroupadapter.h>
#include <preference/preferenceviewholder.h>
#include <preference/expandbutton.h>
#include <preference/preferencemanager.h>
#include <preference/preferencescreen.h>
#include <core/handler.h>
#include <core/looper.h>
#include <view/layoutinflater.h>
#include <view/viewgroup.h>
#include <widget/internal_R.h>
#include <drawable/drawable.h>
#include <algorithm>
#include <porting/cdlog.h>

namespace cdroid {

PreferenceGroupAdapter::PreferenceResourceDescriptor::PreferenceResourceDescriptor(
        const Preference& preference)
    : mLayoutResId(preference.getLayoutResource()),
      mWidgetLayoutResId(preference.getWidgetLayoutResource()),
      mClassName(preference.getPreferenceClassName()) {
}

bool PreferenceGroupAdapter::PreferenceResourceDescriptor::operator==(
        const PreferenceResourceDescriptor& other) const {
    return mLayoutResId == other.mLayoutResId
            && mWidgetLayoutResId == other.mWidgetLayoutResId
            && mClassName == other.mClassName;
}

PreferenceGroupAdapter::PreferenceGroupAdapter(PreferenceGroup& preferenceGroup)
    : mPreferenceGroup(preferenceGroup) {
    mHandler = std::make_unique<Handler>(Looper::getMainLooper());
    mSyncRunnable = [this]() { updatePreferences(); };

    // This adapter should be notified when preferences are added or removed from the group
    mPreferenceGroup.setOnPreferenceChangeInternalListener(this);

    auto* screen = dynamic_cast<PreferenceScreen*>(&mPreferenceGroup);
    if (screen != nullptr) {
        setHasStableIds(screen->shouldUseGeneratedIds());
    } else {
        setHasStableIds(true);
    }
    // Initial sync to generate mPreferences and mVisiblePreferences and display the visible
    // preferences in the RecyclerView
    updatePreferences();
}

void PreferenceGroupAdapter::updatePreferences() {
    for (Preference* preference : mPreferences) {
        // Clear out the listeners in anticipation of some items being removed. This listener
        // will be set again on any remaining preferences when we flatten the group.
        preference->setOnPreferenceChangeInternalListener(nullptr);
    }
    mPreferences.clear();
    flattenPreferenceGroup(mPreferences, mPreferenceGroup);

    std::vector<Preference*> oldVisibleList = std::move(mVisiblePreferences);
    std::vector<Preference*> visiblePreferenceList = createVisiblePreferencesList(mPreferenceGroup);

    mVisiblePreferences = std::move(visiblePreferenceList);

    PreferenceManager* preferenceManager = mPreferenceGroup.getPreferenceManager();
    if (preferenceManager != nullptr
            && preferenceManager->getPreferenceComparisonCallback() != nullptr) {
        // CDROID seam: the recyclerview port has no DiffUtil yet, so the
        // DiffUtil.calculateDiff(dispatchUpdatesTo) path degrades to a full
        // notifyDataSetChanged() — same end state, coarser animations.
        // TODO(port): port androidx.recyclerview.widget.DiffUtil.
        notifyDataSetChanged();
    } else {
        notifyDataSetChanged();
    }

    for (Preference* preference : mPreferences) {
        preference->clearWasDetached();
    }
}

void PreferenceGroupAdapter::flattenPreferenceGroup(std::vector<Preference*>& preferences,
        PreferenceGroup& group) {
    group.sortPreferences();
    const int groupSize = group.getPreferenceCount();
    for (int i = 0; i < groupSize; i++) {
        Preference* preference = group.getPreference(i);

        preferences.push_back(preference);

        PreferenceResourceDescriptor descriptor(*preference);
        if (std::find(mPreferenceResourceDescriptors.begin(), mPreferenceResourceDescriptors.end(),
                      descriptor) == mPreferenceResourceDescriptors.end()) {
            mPreferenceResourceDescriptors.push_back(descriptor);
        }

        PreferenceGroup* nestedGroup = dynamic_cast<PreferenceGroup*>(preference);
        if (nestedGroup != nullptr && nestedGroup->isOnSameScreenAsChildren()) {
            flattenPreferenceGroup(preferences, *nestedGroup);
        }

        preference->setOnPreferenceChangeInternalListener(this);
    }
}

std::vector<Preference*> PreferenceGroupAdapter::createVisiblePreferencesList(
        PreferenceGroup& group) {
    int visiblePreferenceCount = 0;
    std::vector<Preference*> visiblePreferences;
    std::vector<Preference*> collapsedPreferences;

    const int groupSize = group.getPreferenceCount();
    for (int i = 0; i < groupSize; i++) {
        Preference* preference = group.getPreference(i);

        if (!preference->isVisible()) {
            continue;
        }

        if (!isGroupExpandable(group)
                || visiblePreferenceCount < group.getInitialExpandedChildrenCount()) {
            visiblePreferences.push_back(preference);
        } else {
            collapsedPreferences.push_back(preference);
        }

        // PreferenceGroups do not count towards the maximal number of preferences to show
        PreferenceGroup* innerGroup = dynamic_cast<PreferenceGroup*>(preference);
        if (innerGroup == nullptr) {
            visiblePreferenceCount++;
            continue;
        }

        if (!innerGroup->isOnSameScreenAsChildren()) {
            continue;
        }

        if (isGroupExpandable(group) && isGroupExpandable(*innerGroup)) {
            throw std::logic_error(
                    "Nesting an expandable group inside of another expandable group is not "
                    "supported!");
        }

        // Recursively generate nested list of visible preferences
        std::vector<Preference*> innerList = createVisiblePreferencesList(*innerGroup);

        for (Preference* inner : innerList) {
            if (!isGroupExpandable(group)
                    || visiblePreferenceCount < group.getInitialExpandedChildrenCount()) {
                visiblePreferences.push_back(inner);
            } else {
                collapsedPreferences.push_back(inner);
            }
            visiblePreferenceCount++;
        }
    }

    // If there are any visible preferences being hidden, add an expand button to show the rest
    // of the preferences. Clicking the expand button will show all the visible preferences.
    if (isGroupExpandable(group)
            && visiblePreferenceCount > group.getInitialExpandedChildrenCount()) {
        Preference* expandButton = createExpandButton(group, collapsedPreferences);
        visiblePreferences.push_back(expandButton);
    }
    return visiblePreferences;
}

Preference* PreferenceGroupAdapter::createExpandButton(PreferenceGroup& group,
        const std::vector<Preference*>& collapsedPreferences) {
    Preference* preference = new ExpandButton(group.getContext(), collapsedPreferences,
            group.getId());
    preference->setOnPreferenceClickListener(
        [&group, this](Preference& clicked) {
            group.setInitialExpandedChildrenCount(0x7FFFFFFF);
            onPreferenceHierarchyChange(clicked);
            PreferenceGroup::OnExpandButtonClickListener listener =
                    group.getOnExpandButtonClickListener();
            if (listener) {
                listener();
            }
            return true;
        });
    return preference;
}

bool PreferenceGroupAdapter::isGroupExpandable(PreferenceGroup& preferenceGroup) {
    return preferenceGroup.getInitialExpandedChildrenCount() != 0x7FFFFFFF;
}

Preference* PreferenceGroupAdapter::getItem(int position) const {
    if (position < 0 || position >= (int)mVisiblePreferences.size()) return nullptr;
    return mVisiblePreferences[position];
}

int PreferenceGroupAdapter::getItemCount() {
    return (int)mVisiblePreferences.size();
}

long PreferenceGroupAdapter::getItemId(int position) {
    if (!hasStableIds()) {
        return RecyclerView::NO_ID;
    }
    return this->getItem(position)->getId();
}

void PreferenceGroupAdapter::onPreferenceChange(Preference& preference) {
    auto it = std::find(mVisiblePreferences.begin(), mVisiblePreferences.end(), &preference);
    const int index = (int)(it - mVisiblePreferences.begin());
    // If we don't find the preference, we don't need to notify anyone
    if (index != -1) {
        // Send the preference as a placeholder to ensure the view holder is recycled in place
        notifyItemChanged(index, reinterpret_cast<Object*>(&preference));
    }
}

void PreferenceGroupAdapter::onPreferenceHierarchyChange(Preference& /*preference*/) {
    mHandler->removeCallbacks(mSyncRunnable);
    mHandler->post(mSyncRunnable);
}

void PreferenceGroupAdapter::onPreferenceVisibilityChange(Preference& preference) {
    onPreferenceHierarchyChange(preference);
}

int PreferenceGroupAdapter::getItemViewType(int position) {
    Preference* preference = this->getItem(position);

    PreferenceResourceDescriptor descriptor(*preference);

    auto it = std::find(mPreferenceResourceDescriptors.begin(), mPreferenceResourceDescriptors.end(),
                        descriptor);
    int viewType = (int)(it - mPreferenceResourceDescriptors.begin());
    if (it != mPreferenceResourceDescriptors.end()) {
        return viewType;
    } else {
        mPreferenceResourceDescriptors.push_back(descriptor);
        return (int)(mPreferenceResourceDescriptors.size() - 1);
    }
}

RecyclerView::ViewHolder* PreferenceGroupAdapter::onCreateViewHolder(ViewGroup* parent,
        int viewType) {
    const PreferenceResourceDescriptor& descriptor = mPreferenceResourceDescriptors[viewType];

    // AOSP resolves ?android:attr/selectableItemBackground through the
    // BackgroundStyle styleable; CDROID resolves the theme attr directly.
    Drawable* background = nullptr;
    TypedValue value;
    if (parent->getContext()->getTheme().resolveAttribute(
            (int)internal::R::attr::selectableItemBackground, &value, true)) {
        background = parent->getContext()->getDrawable(value.resourceId);
    }

    LayoutInflater* inflater = LayoutInflater::from(parent->getContext());
    View* view = inflater->inflate(descriptor.mLayoutResId, parent, false);
    if (view->getBackground() == nullptr) {
        view->setBackground(background);
    }

    ViewGroup* widgetFrame = dynamic_cast<ViewGroup*>(view->findViewById((int)internal::R::id::widget_frame));
    if (widgetFrame != nullptr) {
        if (descriptor.mWidgetLayoutResId != 0) {
            inflater->inflate(descriptor.mWidgetLayoutResId, widgetFrame);
        } else {
            widgetFrame->setVisibility(View::GONE);
        }
    }

    return new PreferenceViewHolder(view);
}

void PreferenceGroupAdapter::onBindViewHolder(RecyclerView::ViewHolder& holder, int position) {
    Preference* preference = getItem(position);
    static_cast<PreferenceViewHolder&>(holder).resetState();
    preference->onBindViewHolder(static_cast<PreferenceViewHolder&>(holder));
}

int PreferenceGroupAdapter::getPreferenceAdapterPosition(const std::string& key) {
    const int size = (int)mVisiblePreferences.size();
    for (int i = 0; i < size; i++) {
        Preference* candidate = mVisiblePreferences[i];
        if (key == candidate->getKey()) {
            return i;
        }
    }
    return RecyclerView::NO_POSITION;
}

int PreferenceGroupAdapter::getPreferenceAdapterPosition(Preference* preference) {
    const int size = (int)mVisiblePreferences.size();
    for (int i = 0; i < size; i++) {
        Preference* candidate = mVisiblePreferences[i];
        if (candidate != nullptr && candidate == preference) {
            return i;
        }
    }
    return RecyclerView::NO_POSITION;
}

} // namespace cdroid
