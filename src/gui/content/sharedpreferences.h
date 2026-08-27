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
#ifndef __CDROID_SHARED_PREFERENCES_H__
#define __CDROID_SHARED_PREFERENCES_H__

#include <string>
#include <vector>
#include <set>
#include <memory>
#include <functional>

namespace cdroid {

class Preferences;

/**
 * Port of android.content.SharedPreferences. The interface face follows AOSP
 * verbatim; the default implementation (SharedPreferencesImpl) persists to a
 * Preferences INI file instead of Android's XML — the on-disk format is an
 * implementation detail of the storage backend (CDROID seam).
 */
class SharedPreferences {
public:
    /**
     * Interface definition for a callback to be invoked when a shared
     * preference is changed.
     */
    using OnSharedPreferenceChangeListener =
            std::function<void(SharedPreferences& sharedPreferences, const std::string& key)>;

    /**
     * Interface used for modifying values in a {@link SharedPreferences}
     * object. All changes you make in an editor are batched, and not copied
     * back to the original {@link SharedPreferences} until you call {@link Editor#commit}
     * or {@link Editor#apply}.
     */
    class Editor {
    public:
        virtual ~Editor() = default;
        /**
         * Set a String value in the preferences editor, to be written back once
         * {@link #commit} or {@link #apply} are called.
         */
        virtual Editor& putString(const std::string& key, const std::string& value) = 0;
        /**
         * Set a set of String values in the preferences editor, to be written back
         * once {@link #commit} is called.
         */
        virtual Editor& putStringSet(const std::string& key, const std::set<std::string>& values) = 0;
        /**
         * Set an int value in the preferences editor, to be written back once
         * {@link #commit} or {@link #apply} are called.
         */
        virtual Editor& putInt(const std::string& key, int value) = 0;
        /**
         * Set a long value in the preferences editor, to be written back once
         * {@link #commit} or {@link #apply} are called.
         */
        virtual Editor& putLong(const std::string& key, int64_t value) = 0;
        /**
         * Set a float value in the preferences editor, to be written back once
         * {@link #commit} or {@link #apply} are called.
         */
        virtual Editor& putFloat(const std::string& key, float value) = 0;
        /**
         * Set a boolean value in the preferences editor, to be written back once
         * {@link #commit} or {@link #apply} are called.
         */
        virtual Editor& putBoolean(const std::string& key, bool value) = 0;

        /**
         * Mark in the editor that a preference value should be removed, which
         * will be done in the actual preferences once {@link #commit} is
         * called.
         */
        virtual Editor& remove(const std::string& key) = 0;

        /**
         * Mark in the editor to remove <em>all</em> values from the
         * preferences.  Once commit is called, the only remaining preferences
         * will be any that you have defined in this editor.
         */
        virtual Editor& clear() = 0;

        /**
         * Commit your preferences changes back from this Editor to the
         * {@link SharedPreferences} object it is editing.  This atomically
         * performs the requested modifications, replacing whatever is currently
         * in the SharedPreferences.
         */
        virtual bool commit() = 0;

        /**
         * Commit your preferences changes back from this Editor to the
         * {@link SharedPreferences} object it is editing.  This atomically
         * performs the requested modifications, replacing whatever is currently
         * in the SharedPreferences.
         *
         * CDROID seam: unlike Android there is no background write thread, so
         * apply() writes synchronously (identical to commit()).
         */
        virtual void apply() = 0;
    };

    virtual ~SharedPreferences() = default;

    /**
     * Retrieve all values from the preferences.
     */
    virtual std::vector<std::pair<std::string, std::string>> getAll() = 0;

    /**
     * Retrieve a String value from the preferences.
     */
    virtual std::string getString(const std::string& key, const std::string& defValue) = 0;

    /**
     * Retrieve a set of String values from the preferences.
     */
    virtual std::set<std::string> getStringSet(const std::string& key,
            const std::set<std::string>& defValues) = 0;

    virtual int getInt(const std::string& key, int defValue) = 0;
    virtual int64_t getLong(const std::string& key, int64_t defValue) = 0;
    virtual float getFloat(const std::string& key, float defValue) = 0;
    virtual bool getBoolean(const std::string& key, bool defValue) = 0;

    /**
     * Checks whether the preferences contains a preference.
     */
    virtual bool contains(const std::string& key) = 0;

    /**
     * Create a new Editor for these preferences, through which you can make
     * modifications to the data in the preferences and atomically commit those
     * changes back to the SharedPreferences object.
     */
    virtual Editor& edit() = 0;

    /**
     * Registers a callback to be invoked when a preference is changed.
     */
    virtual void registerOnSharedPreferenceChangeListener(
            const OnSharedPreferenceChangeListener& listener) = 0;

    /**
     * Unregisters a previous callback.
     */
    virtual void unregisterOnSharedPreferenceChangeListener(
            const OnSharedPreferenceChangeListener& listener) = 0;
};

/**
 * Port of android.app.SharedPreferencesImpl — the backing implementation of
 * {@link SharedPreferences} used by {@link Context#getSharedPreferences}.
 * Storage is a {@link Preferences} INI file persisted under the CDROID data
 * directory ($HOME/.cdroid/prefs/&lt;name&gt;.ini). All value types are
 * stringified into the single flat namespace, exactly like Android's XML
 * shared_prefs files (one key namespace per file).
 */
class SharedPreferencesImpl : public SharedPreferences {
public:
    /**
     * Constructor: opens (loads) the file named {@code name} under the CDROID
     * prefs directory, creating an empty store when it does not exist yet.
     * {@code mode} mirrors AOSP's Context.MODE_PRIVATE (other file creation
     * modes have no meaning on the CDROID targets and are ignored).
     */
    SharedPreferencesImpl(const std::string& name, int mode);
    ~SharedPreferencesImpl() override;

    std::vector<std::pair<std::string, std::string>> getAll() override;
    std::string getString(const std::string& key, const std::string& defValue) override;
    std::set<std::string> getStringSet(const std::string& key,
            const std::set<std::string>& defValues) override;
    int getInt(const std::string& key, int defValue) override;
    int64_t getLong(const std::string& key, int64_t defValue) override;
    float getFloat(const std::string& key, float defValue) override;
    bool getBoolean(const std::string& key, bool defValue) override;
    bool contains(const std::string& key) override;
    Editor& edit() override;
    void registerOnSharedPreferenceChangeListener(
            const OnSharedPreferenceChangeListener& listener) override;
    void unregisterOnSharedPreferenceChangeListener(
            const OnSharedPreferenceChangeListener& listener) override;

    /**
     * Flush the current in-memory map to disk (called by the Editor on
     * commit/apply).
     */
    void persist();

private:
    class EditorImpl;
    std::string mFilePath;
    std::unique_ptr<Preferences> mPrefs;
    std::unique_ptr<Editor> mEditor;
    std::vector<OnSharedPreferenceChangeListener> mListeners;

    void notifyListeners(const std::string& key);
};

} // namespace cdroid

#endif // __CDROID_SHARED_PREFERENCES_H__
