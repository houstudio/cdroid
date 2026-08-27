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
#include <preference/preferencedatastore.h>
#include <stdexcept>

namespace cdroid {

void PreferenceDataStore::putString(const std::string&, const std::string&) {
    throw std::logic_error("Not implemented on this data store");
}

void PreferenceDataStore::putStringSet(const std::string&, const std::set<std::string>&) {
    throw std::logic_error("Not implemented on this data store");
}

void PreferenceDataStore::putInt(const std::string&, int) {
    throw std::logic_error("Not implemented on this data store");
}

void PreferenceDataStore::putLong(const std::string&, int64_t) {
    throw std::logic_error("Not implemented on this data store");
}

void PreferenceDataStore::putFloat(const std::string&, float) {
    throw std::logic_error("Not implemented on this data store");
}

void PreferenceDataStore::putBoolean(const std::string&, bool) {
    throw std::logic_error("Not implemented on this data store");
}

std::string PreferenceDataStore::getString(const std::string& /*key*/,
        const std::string& defValue) const {
    return defValue;
}

std::set<std::string> PreferenceDataStore::getStringSet(const std::string& /*key*/,
        const std::set<std::string>& defValues) const {
    return defValues;
}

int PreferenceDataStore::getInt(const std::string& /*key*/, int defValue) const {
    return defValue;
}

int64_t PreferenceDataStore::getLong(const std::string& /*key*/, int64_t defValue) const {
    return defValue;
}

float PreferenceDataStore::getFloat(const std::string& /*key*/, float defValue) const {
    return defValue;
}

bool PreferenceDataStore::getBoolean(const std::string& /*key*/, bool defValue) const {
    return defValue;
}

} // namespace cdroid
