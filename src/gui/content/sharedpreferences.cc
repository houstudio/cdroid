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
#include <core/preferences.h>
#include <content/sharedpreferences.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cstdlib>
#include <fstream>
#include <unordered_map>

namespace cdroid {

// The single flat section backing the Preferences store. Android's
// shared_prefs XML has one key namespace per file; the Preferences backend is
// section-scoped, so every key lives under this fixed section.
static const char* PREFS_SECTION = "prefs";

// Sentinel for "key absent" told apart from an empty stored string.
static const std::string MISSING = "\x01__missing__";

// CDROID data directory for preference files. AOSP stores these under
// /data/data/<pkg>/shared_prefs/; CDROID has no per-app sandbox, so the
// convention is $HOME/.cdroid/prefs/.
static std::string prefsDirectory() {
    const char* home = getenv("HOME");
    std::string dir = (home && *home) ? std::string(home) : std::string("/tmp");
    dir += "/.cdroid/prefs";
    return dir;
}

class SharedPreferencesImpl::EditorImpl : public SharedPreferences::Editor {
public:
    explicit EditorImpl(SharedPreferencesImpl& parent) : mParent(parent) {}

    Editor& putString(const std::string& key, const std::string& value) override {
        mPending[key] = value;
        mRemoved.erase(key);
        return *this;
    }

    Editor& putStringSet(const std::string& key, const std::set<std::string>& values) override {
        // Serialize the set by joining with '\n' (values must not contain it).
        std::string joined;
        for (const auto& v : values) {
            if (!joined.empty()) joined += '\n';
            joined += v;
        }
        mPending[key] = joined;
        mRemoved.erase(key);
        return *this;
    }

    Editor& putInt(const std::string& key, int value) override {
        mPending[key] = std::to_string(value);
        mRemoved.erase(key);
        return *this;
    }

    Editor& putLong(const std::string& key, int64_t value) override {
        mPending[key] = std::to_string(value);
        mRemoved.erase(key);
        return *this;
    }

    Editor& putFloat(const std::string& key, float value) override {
        mPending[key] = std::to_string(value);
        mRemoved.erase(key);
        return *this;
    }

    Editor& putBoolean(const std::string& key, bool value) override {
        mPending[key] = value ? "true" : "false";
        mRemoved.erase(key);
        return *this;
    }

    Editor& remove(const std::string& key) override {
        mRemoved.insert(key);
        mPending.erase(key);
        return *this;
    }

    Editor& clear() override {
        mCleared = true;
        mPending.clear();
        mRemoved.clear();
        return *this;
    }

    bool commit() override {
        if (mCleared) {
            mParent.mPrefs->removeSection(PREFS_SECTION);
        }
        for (const auto& key : mRemoved) {
            mParent.mPrefs->remove(PREFS_SECTION, key);
        }
        for (const auto& kv : mPending) {
            mParent.mPrefs->setValue(PREFS_SECTION, kv.first, kv.second);
        }
        mParent.persist();
        // Notify after the state is visible to readers (AOSP fires on commit).
        std::vector<std::string> changed;
        for (const auto& kv : mPending) changed.push_back(kv.first);
        for (const auto& key : mRemoved) changed.push_back(key);
        for (const auto& key : changed) {
            mParent.notifyListeners(key);
        }
        mPending.clear();
        mRemoved.clear();
        mCleared = false;
        return true;
    }

    void apply() override {
        // No background write thread in CDROID — identical to commit().
        commit();
    }

private:
    SharedPreferencesImpl& mParent;
    std::unordered_map<std::string, std::string> mPending;
    std::set<std::string> mRemoved;
    bool mCleared = false;
};

SharedPreferencesImpl::SharedPreferencesImpl(const std::string& name, int /*mode*/) {
    const std::string dir = prefsDirectory();
    mkdir(dir.c_str(), 0755); // best-effort; an existing dir is fine
    mFilePath = dir + "/" + name + ".xml";
    mPrefs = std::make_unique<Preferences>();
    std::ifstream in(mFilePath);
    if (in.good()) {
        mPrefs->load(in);
    }
    mEditor = std::make_unique<EditorImpl>(*this);
}

SharedPreferencesImpl::~SharedPreferencesImpl() {
    persist();
}

void SharedPreferencesImpl::persist() {
    std::ofstream out(mFilePath, std::ios::trunc);
    if (out.good()) {
        mPrefs->save(out);
    }
}

std::vector<std::pair<std::string, std::string>> SharedPreferencesImpl::getAll() {
    std::vector<std::pair<std::string, std::string>> all;
    if (!mPrefs->hasSection(PREFS_SECTION)) return all;
    std::vector<std::string> keys;
    mPrefs->getKeys(PREFS_SECTION, keys);
    for (const auto& key : keys) {
        all.emplace_back(key, mPrefs->getString(PREFS_SECTION, key, std::string()));
    }
    return all;
}

std::string SharedPreferencesImpl::getString(const std::string& key, const std::string& defValue) {
    const std::string v = mPrefs->getString(PREFS_SECTION, key, MISSING);
    return (v == MISSING) ? defValue : v;
}

std::set<std::string> SharedPreferencesImpl::getStringSet(const std::string& key,
        const std::set<std::string>& defValues) {
    const std::string joined = getString(key, MISSING);
    if (joined == MISSING || joined.empty()) return defValues;
    std::set<std::string> values;
    size_t start = 0;
    while (true) {
        const size_t nl = joined.find('\n', start);
        if (nl == std::string::npos) {
            values.insert(joined.substr(start));
            break;
        }
        values.insert(joined.substr(start, nl - start));
        start = nl + 1;
    }
    return values;
}

int SharedPreferencesImpl::getInt(const std::string& key, int defValue) {
    const std::string v = getString(key, MISSING);
    if (v == MISSING) return defValue;
    try { return std::stoi(v); } catch (...) { return defValue; }
}

int64_t SharedPreferencesImpl::getLong(const std::string& key, int64_t defValue) {
    const std::string v = getString(key, MISSING);
    if (v == MISSING) return defValue;
    try { return std::stoll(v); } catch (...) { return defValue; }
}

float SharedPreferencesImpl::getFloat(const std::string& key, float defValue) {
    const std::string v = getString(key, MISSING);
    if (v == MISSING) return defValue;
    try { return std::stof(v); } catch (...) { return defValue; }
}

bool SharedPreferencesImpl::getBoolean(const std::string& key, bool defValue) {
    const std::string v = getString(key, MISSING);
    if (v == MISSING) return defValue;
    return (v == "true" || v == "1");
}

bool SharedPreferencesImpl::contains(const std::string& key) {
    return mPrefs->getString(PREFS_SECTION, key, MISSING) != MISSING;
}

SharedPreferences::Editor& SharedPreferencesImpl::edit() {
    return *mEditor;
}

void SharedPreferencesImpl::registerOnSharedPreferenceChangeListener(
        const OnSharedPreferenceChangeListener& listener) {
    mListeners.push_back(listener);
}

void SharedPreferencesImpl::unregisterOnSharedPreferenceChangeListener(
        const OnSharedPreferenceChangeListener& listener) {
    for (auto it = mListeners.begin(); it != mListeners.end(); ++it) {
        // std::function target identity: erase the first callable registered
        // with the same target address (sufficient for the listener pattern
        // used by the preference framework and apps).
        if (*it && listener &&
            it->template target<void(SharedPreferences&, const std::string&)>() ==
            listener.template target<void(SharedPreferences&, const std::string&)>()) {
            mListeners.erase(it);
            return;
        }
    }
}

void SharedPreferencesImpl::notifyListeners(const std::string& key) {
    // Copy: listeners may unregister themselves while iterating (AOSP uses a
    // snapshot of the listener set too).
    std::vector<OnSharedPreferenceChangeListener> listeners = mListeners;
    for (auto& l : listeners) {
        if (l) l(*this, key);
    }
}

} // namespace cdroid
