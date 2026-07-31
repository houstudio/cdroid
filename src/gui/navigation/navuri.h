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
#ifndef __NAVURI_H__
#define __NAVURI_H__
/*********************************************************************************
 * Port of androidx.navigation.NavUri. Platform-agnostic URI (path?query#fragment)
 * with segment/query-parameter access. Used by NavDeepLink matching.
 *********************************************************************************/
#include <string>
#include <vector>
#include <map>
namespace cdroid{

class NavUri{
public:
    explicit NavUri(const std::string& uriString);
    const std::string& getFragment() const { return mFragment; }
    const std::string& getQuery() const { return mQuery; }
    const std::vector<std::string>& getPathSegments() const { return mPathSegments; }
    std::vector<std::string> getQueryParameters(const std::string& key) const;
    const std::string& toString() const { return mUri; }
    static NavUri* parse(const std::string& s){ return new NavUri(s); }
private:
    std::string mUri;
    std::string mPath;
    std::string mQuery;
    std::string mFragment;
    std::vector<std::string> mPathSegments;
    std::map<std::string, std::vector<std::string>> mQueryParams;
};

}//namespace cdroid
#endif
