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
#ifndef __CDROID_PREFERENCE_MANAGER_H__
#define __CDROID_PREFERENCE_MANAGER_H__

#include <string>
#include <memory>
#include <functional>
#include <core/context.h>
#include <content/sharedpreferences.h>

namespace cdroid {

class Preference;
class PreferenceScreen;
class PreferenceDataStore;
class TwoStatePreference;
class DropDownPreference;

/**
 * Port of androidx.preference.PreferenceManager — used to help create
 * Preference hierarchies from activities or XML.
 */
class PreferenceManager {
public:
    static constexpr const char* KEY_HAS_SET_DEFAULT_VALUES = "_has_set_default_values";

    using OnPreferenceTreeClickListener = std::function<bool(Preference& preference)>;
    using OnDisplayPreferenceDialogListener = std::function<void(Preference& preference)>;
    using OnNavigateToScreenListener = std::function<void(PreferenceScreen& preferenceScreen)>;

    /**
     * Callback class to be used by the RecyclerView Adapter associated with
     * the PreferenceScreen, used to determine when two Preference objects are
     * semantically and visually the same.
     */
    class PreferenceComparisonCallback {
    public:
        virtual ~PreferenceComparisonCallback() = default;
        virtual bool arePreferenceItemsTheSame(const Preference& p1, const Preference& p2) = 0;
        virtual bool arePreferenceContentsTheSame(const Preference& p1, const Preference& p2) = 0;
    };

    /**
     * A basic implementation of PreferenceComparisonCallback suitable for use
     * with the default Preference classes.
     */
    class SimplePreferenceComparisonCallback : public PreferenceComparisonCallback {
    public:
        bool arePreferenceItemsTheSame(const Preference& p1, const Preference& p2) override;
        bool arePreferenceContentsTheSame(const Preference& p1, const Preference& p2) override;
    };

    explicit PreferenceManager(Context& context);

    /**
     * Gets a SharedPreferences instance that points to the default file that
     * is used by the preference framework in the given context.
     */
    static std::shared_ptr<SharedPreferences> getDefaultSharedPreferences(Context& context);

    /**
     * Sets the default values from an XML preference file by reading the
     * values defined by each Preference item's defaultValue attribute.
     */
    static void setDefaultValues(Context& context, int resId, bool readAgain);
    static void setDefaultValues(Context& context, const std::string& sharedPreferencesName,
            int sharedPreferencesMode, int resId, bool readAgain);

    /**
     * Inflates a preference hierarchy from XML. If a preference hierarchy is
     * given, the new preference hierarchies will be merged in.
     */
    PreferenceScreen* inflateFromResource(Context& context, int resId,
            PreferenceScreen* rootPreferences);

    PreferenceScreen* createPreferenceScreen(Context& context);

    /** Called by a preference to get a unique ID in its hierarchy. */
    int64_t getNextId();

    const std::string& getSharedPreferencesName() const;
    void setSharedPreferencesName(const std::string& sharedPreferencesName);

    int getSharedPreferencesMode() const;
    void setSharedPreferencesMode(int sharedPreferencesMode);

    void setStorageDefault();
    void setStorageDeviceProtected();
    bool isStorageDefault() const;
    bool isStorageDeviceProtected() const;

    void setPreferenceDataStore(PreferenceDataStore* dataStore);
    PreferenceDataStore* getPreferenceDataStore() const;

    /**
     * Gets a SharedPreferences instance that preferences managed by this will
     * use. Returns null if a PreferenceDataStore has been set.
     */
    SharedPreferences* getSharedPreferences() const;

    /** Returns the root of the preference hierarchy managed by this class. */
    PreferenceScreen* getPreferenceScreen() const;

    /** Sets the root of the preference hierarchy. */
    bool setPreferences(PreferenceScreen* preferenceScreen);

    /**
     * Finds a Preference with the given key. Returns null if no Preference
     * could be found with the given key.
     */
    Preference* findPreference(const std::string& key) const;

    /**
     * Returns an editor to use when modifying the shared preferences.
     * Do NOT commit unless shouldCommit() returns true.
     */
    SharedPreferences::Editor* getEditor() const;

    /** Whether it is the client's responsibility to commit on the editor. */
    bool shouldCommit() const;

    Context& getContext() const;

    PreferenceComparisonCallback* getPreferenceComparisonCallback() const;
    void setPreferenceComparisonCallback(PreferenceComparisonCallback* preferenceComparisonCallback);

    OnDisplayPreferenceDialogListener getOnDisplayPreferenceDialogListener() const;
    void setOnDisplayPreferenceDialogListener(
            const OnDisplayPreferenceDialogListener& onDisplayPreferenceDialogListener);

    /** Called when a preference requests that a dialog be shown. */
    void showDialog(Preference& preference);

    void setOnPreferenceTreeClickListener(const OnPreferenceTreeClickListener& listener);
    OnPreferenceTreeClickListener getOnPreferenceTreeClickListener() const;

    void setOnNavigateToScreenListener(const OnNavigateToScreenListener& listener);
    OnNavigateToScreenListener getOnNavigateToScreenListener() const;

private:
    void setNoCommit(bool noCommit);

    static std::string getDefaultSharedPreferencesName(Context& context);
    static int getDefaultSharedPreferencesMode();

    static constexpr int STORAGE_DEFAULT = 0;
    static constexpr int STORAGE_DEVICE_PROTECTED = 1;

    /** The context to use. This should always be set. */
    Context& mContext;
    /** The counter for unique IDs. */
    int64_t mNextId = 0;
    /** Cached shared preferences. */
    mutable std::shared_ptr<SharedPreferences> mSharedPreferences;
    /** Data store to be used by the preferences or null if SharedPreferences should be used. */
    PreferenceDataStore* mPreferenceDataStore = nullptr;
    /**
     * If in no-commit mode, the shared editor to give out (which will be
     * committed when exiting no-commit mode). Points at the editor owned by
     * the cached SharedPreferences instance (kept alive by mSharedPreferences).
     */
    mutable SharedPreferences::Editor* mEditor = nullptr;
    /**
     * Blocks commits from happening on the shared editor. This is used when
     * inflating the hierarchy.
     */
    bool mNoCommit = false;
    /** The SharedPreferences name that will be used for all Preferences managed by this instance. */
    std::string mSharedPreferencesName;
    /** The SharedPreferences mode that will be used. */
    int mSharedPreferencesMode = 0;
    int mStorage = STORAGE_DEFAULT;

    /** The PreferenceScreen at the root of the preference hierarchy. */
    PreferenceScreen* mPreferenceScreen = nullptr;

    PreferenceComparisonCallback* mPreferenceComparisonCallback = nullptr;
    OnPreferenceTreeClickListener mOnPreferenceTreeClickListener;
    OnDisplayPreferenceDialogListener mOnDisplayPreferenceDialogListener;
    OnNavigateToScreenListener mOnNavigateToScreenListener;
};

} // namespace cdroid

#endif // __CDROID_PREFERENCE_MANAGER_H__
