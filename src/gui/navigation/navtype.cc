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
#include <navigation/navtype.h>
#include <stdexcept>

namespace cdroid{

void IntNavType::put(Bundle& b, const std::string& k, const int& v){ b.putInt(k, v); }
int IntNavType::get(const Bundle& b, const std::string& k){ return b.getInt(k); }
int IntNavType::parseValue(const std::string& v){
    if(v.size() > 2 && v[0] == '0' && (v[1] == 'x' || v[1] == 'X'))
        return std::stoi(v.substr(2), nullptr, 16); // androidx IntNavType supports 0x hex
    return std::stoi(v);
}
std::string IntNavType::serializeAsValue(const int& v){ return std::to_string(v); }

void LongNavType::put(Bundle& b, const std::string& k, const long& v){ b.putLong(k, v); }
long LongNavType::get(const Bundle& b, const std::string& k){ return b.getLong(k); }
long LongNavType::parseValue(const std::string& v){
    std::string s = v;
    if(!s.empty() && s.back() == 'L') s.pop_back(); // strip trailing L suffix (androidx)
    if(s.size() > 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
        return std::stol(s.substr(2), nullptr, 16);
    return std::stol(s);
}
std::string LongNavType::serializeAsValue(const long& v){ return std::to_string(v); }

void FloatNavType::put(Bundle& b, const std::string& k, const float& v){ b.putFloat(k, v); }
float FloatNavType::get(const Bundle& b, const std::string& k){ return b.getFloat(k); }
float FloatNavType::parseValue(const std::string& v){ return std::stof(v); }
std::string FloatNavType::serializeAsValue(const float& v){ return std::to_string(v); }

void BoolNavType::put(Bundle& b, const std::string& k, const bool& v){ b.putBoolean(k, v); }
bool BoolNavType::get(const Bundle& b, const std::string& k){ return b.getBoolean(k); }
bool BoolNavType::parseValue(const std::string& v){ return v == "true"; }
std::string BoolNavType::serializeAsValue(const bool& v){ return v ? "true" : "false"; }

void StringNavType::put(Bundle& b, const std::string& k, const std::string& v){ b.putString(k, v); }
std::string StringNavType::get(const Bundle& b, const std::string& k){ return b.getString(k); }
std::string StringNavType::parseValue(const std::string& v){ return v; }
std::string StringNavType::serializeAsValue(const std::string& v){ return v; }

void ReferenceNavType::put(Bundle& b, const std::string& k, const int& v){ b.putInt(k, v); }
int ReferenceNavType::get(const Bundle& b, const std::string& k){ return b.getInt(k); }
int ReferenceNavType::parseValue(const std::string& v){
    if(v.size() > 2 && v[0] == '0' && (v[1] == 'x' || v[1] == 'X'))
        return std::stoi(v.substr(2), nullptr, 16); // androidx ReferenceType supports 0x hex
    return std::stoi(v);
}
std::string ReferenceNavType::serializeAsValue(const int& v){ return std::to_string(v); }

NavTypeKind navTypeKindFromName(const std::string& type){
    if(type == "integer" || type == "int") return NavTypeKind::INT;
    if(type == "long")                    return NavTypeKind::LONG;
    if(type == "float")                   return NavTypeKind::FLOAT;
    if(type == "boolean" || type == "bool") return NavTypeKind::BOOL;
    if(type == "string")                  return NavTypeKind::STRING;
    if(type == "reference")               return NavTypeKind::REFERENCE;
    if(type == "integer[]" || type == "int[]") return NavTypeKind::INT_ARRAY;
    if(type == "long[]")                  return NavTypeKind::LONG_ARRAY;
    if(type == "float[]")                 return NavTypeKind::FLOAT_ARRAY;
    if(type == "boolean[]")               return NavTypeKind::BOOL_ARRAY;
    if(type == "string[]")                return NavTypeKind::STRING_ARRAY;
    return NavTypeKind::UNKNOWN;
}

}//namespace cdroid
