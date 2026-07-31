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
#ifndef __NAVARGUMENT_H__
#define __NAVARGUMENT_H__
/*********************************************************************************
 * Port of androidx.navigation.NavArgument (+ Builder) and NamedNavArgument.
 * Holds a NavTypeKind tag + nullable flag + optional default value (nonstd::any).
 *********************************************************************************/
#include <string>
#include <core/any.h>
#include <core/bundle.h>
#include <navigation/navtype.h>
namespace cdroid{

class NavArgument{
public:
    class Builder;
    NavArgument(NavTypeKind type, bool isNullable, bool hasDefault, const nonstd::any& defaultValue);
    NavTypeKind getType() const { return mType; }
    bool isNullable() const { return mIsNullable; }
    bool isDefaultValuePresent() const { return mHasDefault; }
    void putDefaultValue(const std::string& name, Bundle& bundle) const;
    bool verify(const std::string& name, const Bundle& bundle) const;
private:
    NavTypeKind mType;
    bool mIsNullable;
    bool mHasDefault;
    nonstd::any mDefaultValue;
};

class NavArgument::Builder{
public:
    Builder& setType(NavTypeKind t){ mType = t; return *this; }
    Builder& setIsNullable(bool n){ mIsNullable = n; return *this; }
    Builder& setDefaultValue(const nonstd::any& v){ mDefaultValue = v; mHasDefault = true; return *this; }
    NavArgument* build();
private:
    NavTypeKind mType = NavTypeKind::UNKNOWN;
    bool mIsNullable = false;
    bool mHasDefault = false;
    nonstd::any mDefaultValue;
};

struct NamedNavArgument{
    std::string name;
    NavArgument* argument = nullptr;
};

}//namespace cdroid
#endif
