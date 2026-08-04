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
#include <navigation/navargument.h>
#include <typeinfo>
#include <stdexcept>

namespace cdroid{
namespace {
NavTypeKind inferFromAny(const nonstd::any& v){
    if(!v.has_value()) return NavTypeKind::STRING;
    const std::type_info& t = v.type();
    if(t == typeid(int))         return NavTypeKind::INT;
    if(t == typeid(long))        return NavTypeKind::LONG;
    if(t == typeid(float))       return NavTypeKind::FLOAT;
    if(t == typeid(bool))        return NavTypeKind::BOOL;
    if(t == typeid(std::string)) return NavTypeKind::STRING;
    return NavTypeKind::UNKNOWN;
}
}//anonymous

NavArgument::NavArgument(NavTypeKind type, bool isNullable, bool hasDefault, const nonstd::any& defaultValue)
    : mType(type), mIsNullable(isNullable), mHasDefault(hasDefault), mDefaultValue(defaultValue){
}

void NavArgument::putDefaultValue(const std::string& name, Bundle& bundle) const{
    if(!mHasDefault) return;
    try{
        switch(mType){
            case NavTypeKind::INT:    bundle.putInt(name,    nonstd::any_cast<int>(mDefaultValue)); break;
            case NavTypeKind::REFERENCE: bundle.putInt(name, nonstd::any_cast<int>(mDefaultValue)); break;
            case NavTypeKind::LONG:   bundle.putLong(name,   nonstd::any_cast<long>(mDefaultValue)); break;
            case NavTypeKind::FLOAT:  bundle.putFloat(name,  nonstd::any_cast<float>(mDefaultValue)); break;
            case NavTypeKind::BOOL:   bundle.putBoolean(name,nonstd::any_cast<bool>(mDefaultValue)); break;
            case NavTypeKind::STRING: bundle.putString(name, nonstd::any_cast<std::string>(mDefaultValue)); break;
            default: break;
        }
    }catch(const nonstd::bad_any_cast&){}
}

bool NavArgument::verify(const std::string& name, const Bundle& bundle) const{
    if(bundle.containsKey(name)) return true;
    return mIsNullable || mHasDefault;
}

NavArgument* NavArgument::Builder::build(){
    NavTypeKind type = mType;
    if(type == NavTypeKind::UNKNOWN && mHasDefault){
        type = inferFromAny(mDefaultValue);
    }
    if(type == NavTypeKind::UNKNOWN){
        throw std::runtime_error("NavArgument requires a NavType or a defaultValue to infer from");
    }
    return new NavArgument(type, mIsNullable, mHasDefault, mDefaultValue);
}

}//namespace cdroid
