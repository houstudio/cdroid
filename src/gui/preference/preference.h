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
#ifndef __CDROID_PREFERENCE_PREFERENCE_H__
#define __CDROID_PREFERENCE_PREFERENCE_H__

#include <string>
#include <vector>
#include <set>
#include <functional>
#include <core/any.h>
#include <core/context.h>
#include <core/attributeset.h>
#include <core/bundle.h>
#include <core/parcelable.h>
#include <view/abssavedstate.h>
#include <content/sharedpreferences.h>

namespace cdroid {

class Drawable;
class Intent;
class PreferenceManager;
class PreferenceDataStore;
class PreferenceGroup;
class PreferenceViewHolder;
class View;

using namespace nonstd;

/**
 * Port of androidx.preference.Preference — the basic building block that
 * represents an individual setting displayed to a user in the preference
 * hierarchy.
 */
class Preference {
public:
    /** Specify for {@link #setOrder(int)} if a specific order is not required. */
    static const int DEFAULT_ORDER = 0x7FFFFFFF;

    /**
     * Interface definition for a callback to be invoked when the value of this
     * Preference has been changed by the user and is about to be set and/or
     * persisted.
     */
    using OnPreferenceChangeListener = std::function<bool(Preference& preference, const any& newValue)>;

    /**
     * Interface definition for a callback to be invoked when a Preference is clicked.
     */
    using OnPreferenceClickListener = std::function<bool(Preference& preference)>;

    /**
     * Interface definition for a callback to be invoked when this Preference is
     * changed or, if this is a group, there is an addition/removal of
     * Preference(s). Used internally by the adapter.
     */
    class OnPreferenceChangeInternalListener {
    public:
        virtual ~OnPreferenceChangeInternalListener() = default;
        /** Called when this preference has changed. */
        virtual void onPreferenceChange(Preference& preference) = 0;
        /** Called when this group has added/removed Preference(s). */
        virtual void onPreferenceHierarchyChange(Preference& preference) = 0;
        /** Called when this preference has changed its visibility. */
        virtual void onPreferenceVisibilityChange(Preference& preference) = 0;
    };

    /**
     * Interface definition for a callback to be invoked when the summary of
     * this Preference is requested.
     */
    class SummaryProvider {
    public:
        virtual ~SummaryProvider() = default;
        virtual std::string provideSummary(Preference& preference) = 0;
    };

    /**
     * A base class for managing the instance state of a Preference.
     */
    class BaseSavedState : public AbsSavedState {
    public:
        BaseSavedState(Parcel& source) : AbsSavedState(source) {}
        explicit BaseSavedState(Parcelable* superState) : AbsSavedState(superState) {}
    };

    explicit Preference(Context& context, const AttributeSet& attrs);
    explicit Preference(Context& context);
    Preference(Context& context, const AttributeSet& attrs, int defStyleAttr);
    Preference(Context& context, const AttributeSet& attrs, int defStyleAttr, int defStyleRes);
    virtual ~Preference();

    /**
     * Called when a preference is being inflated and the default value
     * attribute needs to be read.
     */
    virtual any onGetDefaultValue(const TypedArray& a, int index);

    void setIntent(Intent* intent);
    Intent* getIntent() const;

    void setFragment(const std::string& fragment);
    const std::string& getFragment() const;

    /**
     * Sets a PreferenceDataStore to be used by this preference instead of
     * using SharedPreferences.
     */
    void setPreferenceDataStore(PreferenceDataStore* dataStore);
    PreferenceDataStore* getPreferenceDataStore() const;

    /** Return the extras Bundle object associated with this preference. */
    Bundle& getExtras();
    Bundle* peekExtras() const;

    void setLayoutResource(int layoutResId);
    int getLayoutResource() const;

    void setWidgetLayoutResource(int widgetLayoutResId);
    int getWidgetLayoutResource() const;

    /**
     * Binds the created View to the data for this preference.
     */
    virtual void onBindViewHolder(PreferenceViewHolder& holder);

    void setOrder(int order);
    int getOrder() const;

    void setViewId(int viewId);

    void setTitle(const std::string& title);
    void setTitle(int titleResId);
    std::string getTitle() const;

    void setIcon(Drawable* icon);
    void setIcon(int iconResId);
    Drawable* getIcon() const;

    virtual std::string getSummary() const;
    virtual void setSummary(const std::string& summary);
    void setSummary(int summaryResId);

    virtual void setEnabled(bool enabled);
    virtual bool isEnabled() const;

    void setSelectable(bool selectable);
    bool isSelectable() const;

    void setShouldDisableView(bool shouldDisableView);
    bool getShouldDisableView() const;

    void setVisible(bool visible);
    bool isVisible() const;

    /** Checks whether this preference is shown to the user in the hierarchy. */
    bool isShown() const;

    // Package-private in AOSP but virtually dispatched (Java semantics);
    // ExpandButton overrides it with a parent-derived stable id.
    virtual long getId() const;

    /** Processes a click on the preference. */
    virtual void onClick() {}

    void setKey(const std::string& key);
    const std::string& getKey() const;

    bool hasKey() const;

    bool isPersistent() const;

    /** Checks whether this preference should store/restore its value(s). */
    bool shouldPersist() const;

    void setPersistent(bool persistent);

    void setSingleLineTitle(bool singleLineTitle);
    bool isSingleLineTitle() const;

    void setIconSpaceReserved(bool iconSpaceReserved);
    bool isIconSpaceReserved() const;

    void setCopyingEnabled(bool enabled);
    bool isCopyingEnabled() const;

    void setSummaryProvider(SummaryProvider* summaryProvider);
    SummaryProvider* getSummaryProvider() const;

    /**
     * Call this method after the user changes the preference, but before the
     * internal state is set.
     */
    bool callChangeListener(const any& newValue);

    void setOnPreferenceChangeListener(const OnPreferenceChangeListener& onPreferenceChangeListener);
    OnPreferenceChangeListener getOnPreferenceChangeListener() const;

    void setOnPreferenceClickListener(const OnPreferenceClickListener& onPreferenceClickListener);
    OnPreferenceClickListener getOnPreferenceClickListener() const;

    /** Called when a click should be performed. */
    virtual void performClick(View* view);
    virtual void performClick();

    Context& getContext() const;

    /**
     * Returns the SharedPreferences where this preference can read its
     * value(s), or null when detached / a data store is set.
     */
    SharedPreferences* getSharedPreferences() const;

    /**
     * Compares preference objects based on order (if set), otherwise
     * alphabetically on the titles.
     */
    int compareTo(const Preference& another) const;
    bool operator<(const Preference& another) const { return compareTo(another) < 0; }
    bool operator==(const Preference& another) const { return this == &another; }

    void setOnPreferenceChangeInternalListener(OnPreferenceChangeInternalListener* listener);

    PreferenceManager* getPreferenceManager() const;

    /**
     * Called when this preference has been attached to a preference hierarchy.
     */
    virtual void onAttachedToHierarchy(PreferenceManager& preferenceManager);
    virtual void onAttachedToHierarchy(PreferenceManager& preferenceManager, long id);

    void assignParent(PreferenceGroup* parentGroup);

    /**
     * Called when the preference hierarchy has been attached to the list of
     * preferences.
     */
    virtual void onAttached();
    /** Called when the preference hierarchy has been detached. */
    virtual void onDetached();

    bool wasDetached() const;
    void clearWasDetached();

    /**
     * Finds a preference in the entire hierarchy with the given key.
     */
    Preference* findPreferenceInHierarchy(const std::string& key) const;

    virtual void notifyDependencyChange(bool disableDependents);

    /**
     * Called when the dependency changes.
     */
    virtual void onDependencyChanged(Preference& dependency, bool disableDependent);

    /**
     * Called when the implicit parent dependency changes.
     */
    virtual void onParentChanged(Preference& parent, bool disableChild);

    virtual bool shouldDisableDependents() const;

    void setDependency(const std::string& dependencyKey);
    std::string getDependency() const;

    PreferenceGroup* getParent() const;

    /** Called when this preference is being removed from the hierarchy. */
    virtual void onPrepareForRemoval();

    void setDefaultValue(const any& defaultValue);

    virtual void onSetInitialValue(bool restorePersistedValue, const any& defaultValue);
    virtual void onSetInitialValue(const any& defaultValue) {}

    bool persistString(const std::string& value);
    std::string getPersistedString(const std::string& defaultReturnValue) const;

    bool persistStringSet(const std::set<std::string>& values);
    std::set<std::string> getPersistedStringSet(const std::set<std::string>& defaultReturnValue) const;

    bool persistInt(int value);
    int getPersistedInt(int defaultReturnValue) const;

    bool persistFloat(float value);
    float getPersistedFloat(float defaultReturnValue) const;

    bool persistLong(int64_t value);
    int64_t getPersistedLong(int64_t defaultReturnValue) const;

    bool persistBoolean(bool value);
    bool getPersistedBoolean(bool defaultReturnValue) const;

    std::string toString() const;
    std::string getFilterableStringBuilder() const;

    /**
     * Store this preference hierarchy's frozen state into the given container.
     */
    void saveHierarchyState(Bundle& container);
    /** Restore this preference hierarchy's previously saved state. */
    void restoreHierarchyState(Bundle& container);

    // Package-private in AOSP (package-visible; PreferenceGroup dispatches
    // into child preferences) — public virtual here so a group can invoke
    // these on children it does not derive from.
    virtual void dispatchSaveInstanceState(Bundle& container);
    virtual void dispatchRestoreInstanceState(Bundle& container);

    virtual Parcelable* onSaveInstanceState();
    virtual void onRestoreInstanceState(Parcelable* state);

    /** Returns the class name used for view-type discrimination by the adapter. */
    virtual std::string getPreferenceClassName() const { return "Preference"; }

protected:
    /** Should be called when the data of this Preference has changed. */
    virtual void notifyChanged();
    /** Should be called when the ordering should be re-evaluated. */
    void notifyHierarchyChanged();

    void requireKey();

    Context& mContext;

private:
    void dispatchSetInitialValue();
    void tryCommit(SharedPreferences::Editor& editor);
    void registerDependency();
    void unregisterDependency();
    void registerDependent(Preference* dependent);
    void unregisterDependent(Preference* dependent);
    void setEnabledStateOnViews(View& v, bool enabled);

    static constexpr const char* CLIPBOARD_ID = "Preference";

    PreferenceManager* mPreferenceManager = nullptr;

    /**
     * The data store that should be used by this preference to store /
     * retrieve data. If null then PreferenceManager#getPreferenceDataStore()
     * needs to be checked; if that is null too, SharedPreferences are used.
     */
    PreferenceDataStore* mPreferenceDataStore = nullptr;

    /** Set when added to hierarchy since we need a unique ID within it. */
    long mId = 0;
    /** Set true temporarily to keep onAttachedToHierarchy from overwriting mId. */
    bool mHasId = false;

    OnPreferenceChangeListener mOnChangeListener;
    OnPreferenceClickListener mOnClickListener;

    int mOrder = DEFAULT_ORDER;
    int mViewId = 0;
    std::string mTitle;
    std::string mSummary;
    /** mIconResId is overridden by mIcon, if mIcon is specified. */
    int mIconResId = 0;
    mutable Drawable* mIcon = nullptr;
    std::string mKey;
    Intent* mIntent = nullptr;
    std::string mFragment;
    std::unique_ptr<Bundle> mExtras;
    bool mEnabled = true;
    bool mSelectable = true;
    bool mRequiresKey = false;
    bool mPersistent = true;
    std::string mDependencyKey;
    any mDefaultValue;
    bool mDependencyMet = true;
    bool mParentDependencyMet = true;
    bool mVisible = true;

    bool mAllowDividerAbove = true;
    bool mAllowDividerBelow = true;
    bool mHasSingleLineTitleAttr = false;
    bool mSingleLineTitle = true;
    bool mIconSpaceReserved = false;
    bool mCopyingEnabled = false;
    bool mShouldDisableView = true;

    int mLayoutResId = 0 /* internal::R::layout::preference, set in the .cc after R.h */;
    int mWidgetLayoutResId = 0;

    OnPreferenceChangeInternalListener* mListener = nullptr;

    std::vector<Preference*> mDependents;
    PreferenceGroup* mParentGroup = nullptr;

    bool mWasDetached = false;
    bool mBaseMethodCalled = false;

    SummaryProvider* mSummaryProvider = nullptr;

    std::function<void(View&)> mClickListener;
};

} // namespace cdroid

#endif // __CDROID_PREFERENCE_PREFERENCE_H__
