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
#ifndef __CDROID_PREFERENCE_DATA_STORE_H__
#define __CDROID_PREFERENCE_DATA_STORE_H__

#include <string>
#include <set>

namespace cdroid {

/**
 * Port of androidx.preference.PreferenceDataStore — a data store interface to
 * be implemented and provided to the Preference framework. This can be used to
 * replace the default SharedPreferences.
 *
 * By default, all "put" methods throw std::logic_error (the C++ counterpart
 * of AOSP's UnsupportedOperationException).
 */
class PreferenceDataStore {
public:
    virtual ~PreferenceDataStore() = default;

    virtual void putString(const std::string& key, const std::string& value);
    virtual void putStringSet(const std::string& key, const std::set<std::string>& values);
    virtual void putInt(const std::string& key, int value);
    virtual void putLong(const std::string& key, int64_t value);
    virtual void putFloat(const std::string& key, float value);
    virtual void putBoolean(const std::string& key, bool value);

    virtual std::string getString(const std::string& key, const std::string& defValue) const;
    virtual std::set<std::string> getStringSet(const std::string& key,
            const std::set<std::string>& defValues) const;
    virtual int getInt(const std::string& key, int defValue) const;
    virtual int64_t getLong(const std::string& key, int64_t defValue) const;
    virtual float getFloat(const std::string& key, float defValue) const;
    virtual bool getBoolean(const std::string& key, bool defValue) const;
};

} // namespace cdroid

#endif // __CDROID_PREFERENCE_DATA_STORE_H__
