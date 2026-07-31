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
#ifndef __NAVTYPE_H__
#define __NAVTYPE_H__
/*********************************************************************************
 * Port of androidx.navigation.NavType<T>. Type-system for navigation arguments:
 * parseValue/serializeAsValue convert between strings and typed values, put/get
 * move them in/out of a Bundle. ParcelableType/SerializableType/EnumType
 * (androidMain-only) are omitted. Array/List variants can be added later.
 *********************************************************************************/
#include <string>
#include <core/bundle.h>

namespace cdroid{

// Type tag (used by NavArgument to avoid a type-erased NavType<T>* in C++).
enum class NavTypeKind{
    INT, LONG, FLOAT, BOOL, STRING,
    REFERENCE,
    INT_ARRAY, LONG_ARRAY, FLOAT_ARRAY, BOOL_ARRAY, STRING_ARRAY,
    UNKNOWN
};

template<typename T>
class NavType{
protected:
    bool mNullableAllowed;
public:
    explicit NavType(bool isNullableAllowed = false) : mNullableAllowed(isNullableAllowed){}
    virtual ~NavType() = default;
    bool isNullableAllowed() const { return mNullableAllowed; }
    virtual void put(Bundle& bundle, const std::string& key, const T& value) = 0;
    virtual T get(const Bundle& bundle, const std::string& key) = 0;
    virtual T parseValue(const std::string& value) = 0;
    virtual std::string serializeAsValue(const T& value) = 0;
    virtual const char* name() const = 0;
    virtual bool valueEquals(const T& a, const T& b){ return a == b; }
    T parseAndPut(Bundle& bundle, const std::string& key, const std::string& value){
        T v = parseValue(value); put(bundle, key, v); return v;
    }
};

class IntNavType    : public NavType<int>{ public: void put(Bundle&,const std::string&,const int&)override; int get(const Bundle&,const std::string&)override; int parseValue(const std::string&)override; std::string serializeAsValue(const int&)override; const char* name()const override{return "integer";} };
class LongNavType   : public NavType<long>{ public: void put(Bundle&,const std::string&,const long&)override; long get(const Bundle&,const std::string&)override; long parseValue(const std::string&)override; std::string serializeAsValue(const long&)override; const char* name()const override{return "long";} };
class FloatNavType  : public NavType<float>{ public: void put(Bundle&,const std::string&,const float&)override; float get(const Bundle&,const std::string&)override; float parseValue(const std::string&)override; std::string serializeAsValue(const float&)override; const char* name()const override{return "float";} };
class BoolNavType   : public NavType<bool>{ public: void put(Bundle&,const std::string&,const bool&)override; bool get(const Bundle&,const std::string&)override; bool parseValue(const std::string&)override; std::string serializeAsValue(const bool&)override; const char* name()const override{return "boolean";} };
class StringNavType : public NavType<std::string>{ public: void put(Bundle&,const std::string&,const std::string&)override; std::string get(const Bundle&,const std::string&)override; std::string parseValue(const std::string&)override; std::string serializeAsValue(const std::string&)override; const char* name()const override{return "string";} };
// Reference type: NavType<int> for resource ids (androidx NavType.ReferenceType). name="reference".
class ReferenceNavType: public NavType<int>{ public: void put(Bundle&,const std::string&,const int&)override; int get(const Bundle&,const std::string&)override; int parseValue(const std::string&)override; std::string serializeAsValue(const int&)override; const char* name()const override{return "reference";} };

// Singleton accessors (androidx NavType.IntType etc.).
inline NavType<int>&         IntType(){    static IntNavType s;    return s; }
inline NavType<long>&        LongType(){   static LongNavType s;   return s; }
inline NavType<float>&       FloatType(){  static FloatNavType s;  return s; }
inline NavType<bool>&        BoolType(){   static BoolNavType s;   return s; }
inline NavType<std::string>& StringType(){ static StringNavType s; return s; }
inline NavType<int>&         ReferenceType(){ static ReferenceNavType s; return s; }

// androidx NavType.fromArgType(type, packageName) -> resolved as a kind tag.
NavTypeKind navTypeKindFromName(const std::string& type);

}//namespace cdroid
#endif
