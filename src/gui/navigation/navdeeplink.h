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
#ifndef __NAV_DEEPLINK_H__
#define __NAV_DEEPLINK_H__
/*********************************************************************************
 * Port of androidx.navigation.NavDeepLink (rewritten). Compiles a uriPattern
 * with {arg} placeholders into a regex; matches() tests a uri, getMatchingArguments()
 * extracts placeholder values (typed via NavArgument when provided).
 * Keeps the legacy (string)-only overloads so existing navdestination.cc compiles.
 *********************************************************************************/
#include <string>
#include <vector>
#include <map>
#include <regex>
#include <core/bundle.h>
namespace cdroid{

class NavArgument;

class NavDeepLink{
public:
    explicit NavDeepLink(const std::string& uriPattern);
    const std::string& getUriPattern() const { return mUriPattern; }
    bool isExactDeepLink() const;
    bool matches(const std::string& deepLink) const;
    // Legacy: no type map; placeholders returned as strings.
    Bundle* getMatchingArguments(const std::string& deepLink);
    // Modern: typed extraction via the destination's arguments map.
    Bundle* getMatchingArguments(const std::string& deepLink,
                                 const std::map<std::string, NavArgument*>& arguments) const;
private:
    void buildRegex(const std::string& pattern);
    std::string mUriPattern;
    std::regex mRegex;
    std::vector<std::string> mArgNames;
};

}//namespace cdroid
#endif/*__NAV_DEEPLINK_H__*/
