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
#include <preference/preference.h>
#include <preference/preferencemanager.h>
#include <preference/preferencedatastore.h>
#include <preference/preferencegroup.h>
#include <preference/preferencescreen.h>
#include <preference/preferenceviewholder.h>
#include <preference/androidresources.h>
#include <content/sharedpreferences.h>
#include <widget/internal_R.h>
#include <widget/framework_styleable.h>
#include <view/view.h>
#include <view/viewgroup.h>
#include <widget/textview.h>
#include <widget/imageview.h>
#include <drawable/drawable.h>
#include <core/intent.h>
#include <porting/cdlog.h>

#include <preference/preferenceinflater.h>

namespace cdroid {

Preference::Preference(Context& context, const AttributeSet& attrs, int defStyleAttr, int defStyleRes)
    : mContext(context) {
    namespace ns = internal::R::styleable;

    auto a = context.obtainStyledAttributes(attrs, ns::Preference, defStyleAttr, defStyleRes);

    mIconResId = (int)a->getResourceId(ns::Preference_icon, 0);
    mKey = a->getString(ns::Preference_key);
    mTitle = a->getString(ns::Preference_title);
    mSummary = a->getString(ns::Preference_summary);
    mOrder = a->getInt(ns::Preference_order, DEFAULT_ORDER);
    mFragment = a->getString(ns::Preference_fragment);
    mLayoutResId = (int)a->getResourceId(ns::Preference_layout, (uint32_t)internal::R::layout::preference);
    mWidgetLayoutResId = (int)a->getResourceId(ns::Preference_widgetLayout, 0);
    mEnabled = a->getBoolean(ns::Preference_enabled, true);
    mSelectable = a->getBoolean(ns::Preference_selectable, true);
    mPersistent = a->getBoolean(ns::Preference_persistent, true);
    mDependencyKey = a->getString(ns::Preference_dependency);

    mAllowDividerAbove = a->getBoolean(ns::Preference_allowDividerAbove, mSelectable);
    mAllowDividerBelow = a->getBoolean(ns::Preference_allowDividerBelow, mSelectable);

    if (a->hasValue(ns::Preference_defaultValue)) {
        mDefaultValue = onGetDefaultValue(*a, ns::Preference_defaultValue);
    }

    mShouldDisableView = a->getBoolean(ns::Preference_shouldDisableView, true);

    mHasSingleLineTitleAttr = a->hasValue(ns::Preference_singleLineTitle);
    if (mHasSingleLineTitleAttr) {
        mSingleLineTitle = a->getBoolean(ns::Preference_singleLineTitle, true);
    }

    mIconSpaceReserved = a->getBoolean(ns::Preference_iconSpaceReserved, false);
    mVisible = a->getBoolean(ns::Preference_isPreferenceVisible, true);
    mCopyingEnabled = a->getBoolean(ns::Preference_enableCopying, false);
}

Preference::Preference(Context& context, const AttributeSet& attrs, int defStyleAttr)
    : Preference(context, attrs, defStyleAttr, 0) {
}

Preference::Preference(Context& context, const AttributeSet& attrs)
    : Preference(context, attrs, (int)internal::R::attr::preferenceStyle) {
}

Preference::Preference(Context& context)
    : Preference(context, AttributeSet()) {
}

Preference::~Preference() = default;

any Preference::onGetDefaultValue(const TypedArray& /*a*/, int /*index*/) {
    return any();
}

void Preference::setIntent(Intent* intent) {
    mIntent = intent;
}

Intent* Preference::getIntent() const {
    return mIntent;
}

void Preference::setFragment(const std::string& fragment) {
    mFragment = fragment;
}

const std::string& Preference::getFragment() const {
    return mFragment;
}

void Preference::setPreferenceDataStore(PreferenceDataStore* dataStore) {
    mPreferenceDataStore = dataStore;
}

PreferenceDataStore* Preference::getPreferenceDataStore() const {
    if (mPreferenceDataStore != nullptr) {
        return mPreferenceDataStore;
    } else if (mPreferenceManager != nullptr) {
        return mPreferenceManager->getPreferenceDataStore();
    }
    return nullptr;
}

Bundle& Preference::getExtras() {
    if (!mExtras) {
        mExtras = std::make_unique<Bundle>();
    }
    return *mExtras;
}

Bundle* Preference::peekExtras() const {
    return mExtras.get();
}

void Preference::setLayoutResource(int layoutResId) {
    mLayoutResId = layoutResId;
}

int Preference::getLayoutResource() const {
    return mLayoutResId;
}

void Preference::setWidgetLayoutResource(int widgetLayoutResId) {
    mWidgetLayoutResId = widgetLayoutResId;
}

int Preference::getWidgetLayoutResource() const {
    return mWidgetLayoutResId;
}

void Preference::onBindViewHolder(PreferenceViewHolder& holder) {
    View& itemView = *holder.itemView;

    // The click listener routes to performClick (AOSP mClickListener).
    itemView.setOnClickListener([&itemView, this](View&) { performClick(&itemView); });
    itemView.setId(mViewId);

    int summaryTextColor = 0;
    bool hasSummaryTextColor = false;

    auto summaryView = static_cast<TextView*>(holder.findViewById((int)internal::R::id::summary));
    if (summaryView) {
        const std::string summary = getSummary();
        if (!summary.empty()) {
            summaryView->setText(summary);
            summaryView->setVisibility(View::VISIBLE);
            summaryTextColor = summaryView->getCurrentTextColor();
            hasSummaryTextColor = true;
        } else {
            summaryView->setVisibility(View::GONE);
        }
    }

    auto titleView = static_cast<TextView*>(holder.findViewById((int)internal::R::id::title));
    if (titleView) {
        const std::string title = getTitle();
        if (!title.empty()) {
            titleView->setText(title);
            titleView->setVisibility(View::VISIBLE);
            if (mHasSingleLineTitleAttr) {
                titleView->setSingleLine(mSingleLineTitle);
            }
            // If this Preference is not selectable, but still enabled, we should set the
            // title text colour to the same colour used for the summary text
            if (!isSelectable() && isEnabled() && hasSummaryTextColor) {
                titleView->setTextColor(summaryTextColor);
            }
        } else {
            titleView->setVisibility(View::GONE);
        }
    }

    auto imageView = static_cast<ImageView*>(holder.findViewById((int)internal::R::id::icon));
    if (imageView) {
        if (mIconResId != 0 || mIcon != nullptr) {
            if (mIcon == nullptr) {
                mIcon = mContext.getDrawable(mIconResId);
            }
            if (mIcon != nullptr) {
                imageView->setImageDrawable(mIcon);
            }
        }
        if (mIcon != nullptr) {
            imageView->setVisibility(View::VISIBLE);
        } else {
            imageView->setVisibility(mIconSpaceReserved ? View::INVISIBLE : View::GONE);
        }
    }

    // The framework layout has no separate icon_frame container; the CDROID
    // seam keeps the lookup so custom layouts with one keep working.
    View* imageFrame = holder.findViewById(AndroidResources::ANDROID_R_ICON_FRAME);
    if (imageFrame) {
        if (mIcon != nullptr) {
            imageFrame->setVisibility(View::VISIBLE);
        } else {
            imageFrame->setVisibility(mIconSpaceReserved ? View::INVISIBLE : View::GONE);
        }
    }

    if (mShouldDisableView) {
        setEnabledStateOnViews(itemView, isEnabled());
    } else {
        setEnabledStateOnViews(itemView, true);
    }

    const bool selectable = isSelectable();
    itemView.setFocusable(selectable);
    itemView.setClickable(selectable);

    holder.setDividerAllowedAbove(mAllowDividerAbove);
    holder.setDividerAllowedBelow(mAllowDividerBelow);

    // enableCopying requires a clipboard service, which CDROID does not
    // provide; the long-press context-menu hook is kept for layout parity.
    const bool copyingEnabled = isCopyingEnabled();
    itemView.setLongClickable(copyingEnabled);
}

void Preference::setEnabledStateOnViews(View& v, bool enabled) {
    v.setEnabled(enabled);

    ViewGroup* vg = dynamic_cast<ViewGroup*>(&v);
    if (vg) {
        for (int i = vg->getChildCount() - 1; i >= 0; i--) {
            setEnabledStateOnViews(*vg->getChildAt(i), enabled);
        }
    }
}

void Preference::setOrder(int order) {
    if (order != mOrder) {
        mOrder = order;
        // Reorder the list
        notifyHierarchyChanged();
    }
}

int Preference::getOrder() const {
    return mOrder;
}

void Preference::setViewId(int viewId) {
    mViewId = viewId;
}

void Preference::setTitle(const std::string& title) {
    if (title != mTitle) {
        mTitle = title;
        notifyChanged();
    }
}

void Preference::setTitle(int titleResId) {
    setTitle(mContext.getString(titleResId));
}

std::string Preference::getTitle() const {
    return mTitle;
}

void Preference::setIcon(Drawable* icon) {
    if (mIcon != icon) {
        mIcon = icon;
        mIconResId = 0;
        notifyChanged();
    }
}

void Preference::setIcon(int iconResId) {
    setIcon(mContext.getDrawable(iconResId));
    mIconResId = iconResId;
}

Drawable* Preference::getIcon() const {
    if (mIcon == nullptr && mIconResId != 0) {
        mIcon = mContext.getDrawable(mIconResId);
    }
    return mIcon;
}

std::string Preference::getSummary() const {
    if (getSummaryProvider() != nullptr) {
        return getSummaryProvider()->provideSummary(const_cast<Preference&>(*this));
    }
    return mSummary;
}

void Preference::setSummary(const std::string& summary) {
    if (summary != mSummary) {
        mSummary = summary;
        notifyChanged();
    }
}

void Preference::setSummary(int summaryResId) {
    setSummary(mContext.getString(summaryResId));
}

void Preference::setEnabled(bool enabled) {
    if (mEnabled != enabled) {
        mEnabled = enabled;
        // Enabled state can change dependent preferences' states, so notify
        notifyDependencyChange(shouldDisableDependents());
        notifyChanged();
    }
}

bool Preference::isEnabled() const {
    return mEnabled && mDependencyMet && mParentDependencyMet;
}

void Preference::setSelectable(bool selectable) {
    if (mSelectable != selectable) {
        mSelectable = selectable;
        notifyChanged();
    }
}

bool Preference::isSelectable() const {
    return mSelectable;
}

void Preference::setShouldDisableView(bool shouldDisableView) {
    if (mShouldDisableView != shouldDisableView) {
        mShouldDisableView = shouldDisableView;
        notifyChanged();
    }
}

bool Preference::getShouldDisableView() const {
    return mShouldDisableView;
}

void Preference::setVisible(bool visible) {
    if (mVisible != visible) {
        mVisible = visible;
        if (mListener != nullptr) {
            mListener->onPreferenceVisibilityChange(*this);
        }
    }
}

bool Preference::isVisible() const {
    return mVisible;
}

bool Preference::isShown() const {
    if (!isVisible()) {
        return false;
    }

    if (getPreferenceManager() == nullptr) {
        // We are not attached to the hierarchy
        return false;
    }

    if (static_cast<const Preference*>(getPreferenceManager()->getPreferenceScreen()) == this) {
        // We are at the root preference, so this preference and its ancestors are visible
        return true;
    }

    PreferenceGroup* parent = getParent();
    if (parent == nullptr) {
        // We are not attached to the hierarchy
        return false;
    }

    return parent->isShown();
}

long Preference::getId() const {
    return mId;
}

void Preference::setKey(const std::string& key) {
    mKey = key;

    if (mRequiresKey && !hasKey()) {
        requireKey();
    }
}

const std::string& Preference::getKey() const {
    return mKey;
}

void Preference::requireKey() {
    if (mKey.empty()) {
        throw std::logic_error("Preference does not have a key assigned.");
    }
    mRequiresKey = true;
}

bool Preference::hasKey() const {
    return !mKey.empty();
}

bool Preference::isPersistent() const {
    return mPersistent;
}

bool Preference::shouldPersist() const {
    return mPreferenceManager != nullptr && isPersistent() && hasKey();
}

void Preference::setPersistent(bool persistent) {
    mPersistent = persistent;
}

void Preference::setSingleLineTitle(bool singleLineTitle) {
    mHasSingleLineTitleAttr = true;
    mSingleLineTitle = singleLineTitle;
}

bool Preference::isSingleLineTitle() const {
    return mSingleLineTitle;
}

void Preference::setIconSpaceReserved(bool iconSpaceReserved) {
    if (mIconSpaceReserved != iconSpaceReserved) {
        mIconSpaceReserved = iconSpaceReserved;
        notifyChanged();
    }
}

bool Preference::isIconSpaceReserved() const {
    return mIconSpaceReserved;
}

void Preference::setCopyingEnabled(bool enabled) {
    if (mCopyingEnabled != enabled) {
        mCopyingEnabled = enabled;
        notifyChanged();
    }
}

bool Preference::isCopyingEnabled() const {
    return mCopyingEnabled;
}

void Preference::setSummaryProvider(Preference::SummaryProvider* summaryProvider) {
    mSummaryProvider = summaryProvider;
    notifyChanged();
}

Preference::SummaryProvider* Preference::getSummaryProvider() const {
    return mSummaryProvider;
}

bool Preference::callChangeListener(const any& newValue) {
    return !mOnChangeListener || mOnChangeListener(*this, newValue);
}

void Preference::setOnPreferenceChangeListener(const OnPreferenceChangeListener& onPreferenceChangeListener) {
    mOnChangeListener = onPreferenceChangeListener;
}

Preference::OnPreferenceChangeListener Preference::getOnPreferenceChangeListener() const {
    return mOnChangeListener;
}

void Preference::setOnPreferenceClickListener(const OnPreferenceClickListener& onPreferenceClickListener) {
    mOnClickListener = onPreferenceClickListener;
}

Preference::OnPreferenceClickListener Preference::getOnPreferenceClickListener() const {
    return mOnClickListener;
}

void Preference::performClick(View* /*view*/) {
    performClick();
}

void Preference::performClick() {
    if (!isEnabled() || !isSelectable()) {
        return;
    }

    onClick();

    if (mOnClickListener && mOnClickListener(*this)) {
        return;
    }

    PreferenceManager* preferenceManager = getPreferenceManager();
    if (preferenceManager != nullptr) {
        auto listener = preferenceManager->getOnPreferenceTreeClickListener();
        if (listener && listener(*this)) {
            return;
        }
    }

    if (mIntent != nullptr) {
        mContext.startActivity(*mIntent);
    }
}

Context& Preference::getContext() const {
    return mContext;
}

SharedPreferences* Preference::getSharedPreferences() const {
    if (mPreferenceManager == nullptr || getPreferenceDataStore() != nullptr) {
        return nullptr;
    }
    return mPreferenceManager->getSharedPreferences();
}

int Preference::compareTo(const Preference& another) const {
    if (mOrder != another.mOrder) {
        // Do order comparison
        return mOrder - another.mOrder;
    } else if (mTitle == another.mTitle) {
        // If titles are empty or the same string comparison
        return 0;
    } else if (mTitle.empty()) {
        return 1;
    } else if (another.mTitle.empty()) {
        return -1;
    } else {
        // Do name comparison (case-insensitive, mirroring compareToIgnoreCase)
        std::string a = mTitle;
        std::string b = another.mTitle;
        std::transform(a.begin(), a.end(), a.begin(), ::tolower);
        std::transform(b.begin(), b.end(), b.begin(), ::tolower);
        return a.compare(b);
    }
}

void Preference::setOnPreferenceChangeInternalListener(OnPreferenceChangeInternalListener* listener) {
    mListener = listener;
}

void Preference::notifyChanged() {
    if (mListener != nullptr) {
        mListener->onPreferenceChange(*this);
    }
}

void Preference::notifyHierarchyChanged() {
    if (mListener != nullptr) {
        mListener->onPreferenceHierarchyChange(*this);
    }
}

PreferenceManager* Preference::getPreferenceManager() const {
    return mPreferenceManager;
}

void Preference::onAttachedToHierarchy(PreferenceManager& preferenceManager) {
    mPreferenceManager = &preferenceManager;

    if (!mHasId) {
        mId = preferenceManager.getNextId();
    }

    dispatchSetInitialValue();
}

void Preference::onAttachedToHierarchy(PreferenceManager& preferenceManager, long id) {
    mId = id;
    mHasId = true;
    onAttachedToHierarchy(preferenceManager);
    mHasId = false;
}

void Preference::assignParent(PreferenceGroup* parentGroup) {
    if (parentGroup != nullptr && mParentGroup != nullptr) {
        throw std::logic_error(
                "This preference already has a parent. You must remove the existing parent "
                "before assigning a new one.");
    }
    mParentGroup = parentGroup;
}

void Preference::onAttached() {
    // At this point, the hierarchy that this preference is in is connected
    // with all other preferences.
    registerDependency();
}

void Preference::onDetached() {
    unregisterDependency();
    mWasDetached = true;
}

bool Preference::wasDetached() const {
    return mWasDetached;
}

void Preference::clearWasDetached() {
    mWasDetached = false;
}

void Preference::registerDependency() {
    if (mDependencyKey.empty()) return;

    Preference* preference = findPreferenceInHierarchy(mDependencyKey);
    if (preference != nullptr) {
        preference->registerDependent(this);
    } else {
        throw std::logic_error("Dependency \"" + mDependencyKey
                + "\" not found for preference \"" + mKey + "\" (title: \"" + mTitle + "\"");
    }
}

void Preference::unregisterDependency() {
    if (!mDependencyKey.empty()) {
        Preference* oldDependency = findPreferenceInHierarchy(mDependencyKey);
        if (oldDependency != nullptr) {
            oldDependency->unregisterDependent(this);
        }
    }
}

Preference* Preference::findPreferenceInHierarchy(const std::string& key) const {
    if (mPreferenceManager == nullptr) {
        return nullptr;
    }
    return mPreferenceManager->findPreference(key);
}

void Preference::registerDependent(Preference* dependent) {
    mDependents.push_back(dependent);
    dependent->onDependencyChanged(*this, shouldDisableDependents());
}

void Preference::unregisterDependent(Preference* dependent) {
    mDependents.erase(std::remove(mDependents.begin(), mDependents.end(), dependent),
                      mDependents.end());
}

void Preference::notifyDependencyChange(bool disableDependents) {
    const size_t dependentsCount = mDependents.size();
    for (size_t i = 0; i < dependentsCount; i++) {
        mDependents[i]->onDependencyChanged(*this, disableDependents);
    }
}

void Preference::onDependencyChanged(Preference& /*dependency*/, bool disableDependent) {
    if (mDependencyMet == disableDependent) {
        mDependencyMet = !disableDependent;
        // Enabled state can change dependent preferences' states, so notify
        notifyDependencyChange(shouldDisableDependents());
        notifyChanged();
    }
}

void Preference::onParentChanged(Preference& /*parent*/, bool disableChild) {
    if (mParentDependencyMet == disableChild) {
        mParentDependencyMet = !disableChild;
        // Enabled state can change dependent preferences' states, so notify
        notifyDependencyChange(shouldDisableDependents());
        notifyChanged();
    }
}

bool Preference::shouldDisableDependents() const {
    return !isEnabled();
}

void Preference::setDependency(const std::string& dependencyKey) {
    // Unregister the old dependency, if we had one
    unregisterDependency();

    // Register the new
    mDependencyKey = dependencyKey;
    registerDependency();
}

std::string Preference::getDependency() const {
    return mDependencyKey;
}

PreferenceGroup* Preference::getParent() const {
    return mParentGroup;
}

void Preference::onPrepareForRemoval() {
    unregisterDependency();
}

void Preference::setDefaultValue(const any& defaultValue) {
    mDefaultValue = defaultValue;
}

void Preference::dispatchSetInitialValue() {
    if (getPreferenceDataStore() != nullptr) {
        onSetInitialValue(true, mDefaultValue);
        return;
    }

    // By now, we know if we are persistent.
    const bool shouldPersist = this->shouldPersist();
    if (!shouldPersist || !getSharedPreferences()->contains(mKey)) {
        if (mDefaultValue.has_value()) {
            onSetInitialValue(false, mDefaultValue);
        }
    } else {
        onSetInitialValue(true, any());
    }
}

void Preference::onSetInitialValue(bool /*restorePersistedValue*/, const any& defaultValue) {
    // Deprecated AOSP form delegates to onSetInitialValue(Object) (Preference.java:1612).
    onSetInitialValue(defaultValue);
}

void Preference::tryCommit(SharedPreferences::Editor& editor) {
    if (mPreferenceManager->shouldCommit()) {
        editor.apply();
    }
}

bool Preference::persistString(const std::string& value) {
    if (!shouldPersist()) {
        return false;
    }

    // Shouldn't store null
    if (value == getPersistedString(std::string())) {
        // It's already there, so the same as persisting
        return true;
    }

    PreferenceDataStore* dataStore = getPreferenceDataStore();
    if (dataStore != nullptr) {
        dataStore->putString(mKey, value);
    } else {
        auto* editor = mPreferenceManager->getEditor();
        editor->putString(mKey, value);
        tryCommit(*editor);
    }
    return true;
}

std::string Preference::getPersistedString(const std::string& defaultReturnValue) const {
    if (!shouldPersist()) {
        return defaultReturnValue;
    }

    PreferenceDataStore* dataStore = getPreferenceDataStore();
    if (dataStore != nullptr) {
        return dataStore->getString(mKey, defaultReturnValue);
    }

    return mPreferenceManager->getSharedPreferences()->getString(mKey, defaultReturnValue);
}

bool Preference::persistStringSet(const std::set<std::string>& values) {
    if (!shouldPersist()) {
        return false;
    }

    // Shouldn't store null
    if (values == getPersistedStringSet(std::set<std::string>())) {
        // It's already there, so the same as persisting
        return true;
    }

    PreferenceDataStore* dataStore = getPreferenceDataStore();
    if (dataStore != nullptr) {
        dataStore->putStringSet(mKey, values);
    } else {
        auto* editor = mPreferenceManager->getEditor();
        editor->putStringSet(mKey, values);
        tryCommit(*editor);
    }
    return true;
}

std::set<std::string> Preference::getPersistedStringSet(
        const std::set<std::string>& defaultReturnValue) const {
    if (!shouldPersist()) {
        return defaultReturnValue;
    }

    PreferenceDataStore* dataStore = getPreferenceDataStore();
    if (dataStore != nullptr) {
        return dataStore->getStringSet(mKey, defaultReturnValue);
    }

    return mPreferenceManager->getSharedPreferences()->getStringSet(mKey, defaultReturnValue);
}

bool Preference::persistInt(int value) {
    if (!shouldPersist()) {
        return false;
    }

    if (value == getPersistedInt(~value)) {
        // It's already there, so the same as persisting
        return true;
    }

    PreferenceDataStore* dataStore = getPreferenceDataStore();
    if (dataStore != nullptr) {
        dataStore->putInt(mKey, value);
    } else {
        auto* editor = mPreferenceManager->getEditor();
        editor->putInt(mKey, value);
        tryCommit(*editor);
    }
    return true;
}

int Preference::getPersistedInt(int defaultReturnValue) const {
    if (!shouldPersist()) {
        return defaultReturnValue;
    }

    PreferenceDataStore* dataStore = getPreferenceDataStore();
    if (dataStore != nullptr) {
        return dataStore->getInt(mKey, defaultReturnValue);
    }

    return mPreferenceManager->getSharedPreferences()->getInt(mKey, defaultReturnValue);
}

bool Preference::persistFloat(float value) {
    if (!shouldPersist()) {
        return false;
    }

    if (value == getPersistedFloat(std::numeric_limits<float>::quiet_NaN())) {
        // It's already there, so the same as persisting
        return true;
    }

    PreferenceDataStore* dataStore = getPreferenceDataStore();
    if (dataStore != nullptr) {
        dataStore->putFloat(mKey, value);
    } else {
        auto* editor = mPreferenceManager->getEditor();
        editor->putFloat(mKey, value);
        tryCommit(*editor);
    }
    return true;
}

float Preference::getPersistedFloat(float defaultReturnValue) const {
    if (!shouldPersist()) {
        return defaultReturnValue;
    }

    PreferenceDataStore* dataStore = getPreferenceDataStore();
    if (dataStore != nullptr) {
        return dataStore->getFloat(mKey, defaultReturnValue);
    }

    return mPreferenceManager->getSharedPreferences()->getFloat(mKey, defaultReturnValue);
}

bool Preference::persistLong(int64_t value) {
    if (!shouldPersist()) {
        return false;
    }

    if (value == getPersistedLong(~value)) {
        // It's already there, so the same as persisting
        return true;
    }

    PreferenceDataStore* dataStore = getPreferenceDataStore();
    if (dataStore != nullptr) {
        dataStore->putLong(mKey, value);
    } else {
        auto* editor = mPreferenceManager->getEditor();
        editor->putLong(mKey, value);
        tryCommit(*editor);
    }
    return true;
}

int64_t Preference::getPersistedLong(int64_t defaultReturnValue) const {
    if (!shouldPersist()) {
        return defaultReturnValue;
    }

    PreferenceDataStore* dataStore = getPreferenceDataStore();
    if (dataStore != nullptr) {
        return dataStore->getLong(mKey, defaultReturnValue);
    }

    return mPreferenceManager->getSharedPreferences()->getLong(mKey, defaultReturnValue);
}

bool Preference::persistBoolean(bool value) {
    if (!shouldPersist()) {
        return false;
    }

    if (value == getPersistedBoolean(!value)) {
        // It's already there, so the same as persisting
        return true;
    }

    PreferenceDataStore* dataStore = getPreferenceDataStore();
    if (dataStore != nullptr) {
        dataStore->putBoolean(mKey, value);
    } else {
        auto* editor = mPreferenceManager->getEditor();
        editor->putBoolean(mKey, value);
        tryCommit(*editor);
    }
    return true;
}

bool Preference::getPersistedBoolean(bool defaultReturnValue) const {
    if (!shouldPersist()) {
        return defaultReturnValue;
    }

    PreferenceDataStore* dataStore = getPreferenceDataStore();
    if (dataStore != nullptr) {
        return dataStore->getBoolean(mKey, defaultReturnValue);
    }

    return mPreferenceManager->getSharedPreferences()->getBoolean(mKey, defaultReturnValue);
}

std::string Preference::toString() const {
    return getFilterableStringBuilder();
}

std::string Preference::getFilterableStringBuilder() const {
    std::string sb;
    const std::string title = getTitle();
    if (!title.empty()) {
        sb += title;
        sb += ' ';
    }
    const std::string summary = getSummary();
    if (!summary.empty()) {
        sb += summary;
        sb += ' ';
    }
    if (!sb.empty()) {
        // Drop the last space
        sb.pop_back();
    }
    return sb;
}

void Preference::saveHierarchyState(Bundle& container) {
    dispatchSaveInstanceState(container);
}

void Preference::dispatchSaveInstanceState(Bundle& container) {
    if (hasKey()) {
        mBaseMethodCalled = false;
        Parcelable* state = onSaveInstanceState();
        if (!mBaseMethodCalled) {
            throw std::logic_error("Derived class did not call super.onSaveInstanceState()");
        }
        if (state != nullptr) {
            container.putParcelable(mKey, state);
        }
    }
}

Parcelable* Preference::onSaveInstanceState() {
    mBaseMethodCalled = true;
    return &AbsSavedState::EMPTY_STATE;
}

void Preference::restoreHierarchyState(Bundle& container) {
    dispatchRestoreInstanceState(container);
}

void Preference::dispatchRestoreInstanceState(Bundle& container) {
    if (hasKey()) {
        Parcelable* state = container.getParcelable(mKey);
        if (state != nullptr) {
            mBaseMethodCalled = false;
            onRestoreInstanceState(state);
            if (!mBaseMethodCalled) {
                throw std::logic_error("Derived class did not call super.onRestoreInstanceState()");
            }
        }
    }
}

void Preference::onRestoreInstanceState(Parcelable* state) {
    mBaseMethodCalled = true;
    if (state != &AbsSavedState::EMPTY_STATE && state != nullptr) {
        throw std::invalid_argument("Wrong state class -- expecting Preference State");
    }
}

DECLARE_PREFERENCE(Preference)

} // namespace cdroid

