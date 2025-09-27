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
#ifndef __ANIMATION_PROPERTY_H__
#define __ANIMATION_PROPERTY_H__
#include <string>
#include <core/variant.h>
#include <drawables/pathparser.h>
namespace cdroid{
typedef nonstd::variant<int,float,PathParser::PathData>AnimateValue;

#if variant_CPP17_OR_GREATER
   #define GET_VARIANT(vt,type) std::get<type>(vt)
#else
   #define GET_VARIANT(vt,type) vt.get<type>()
#endif

class Property{
public:
    static constexpr int UNDEFINED =-1;
    static constexpr int INT_TYPE = 0;
    static constexpr int COLOR_TYPE=1;
    static constexpr int FLOAT_TYPE=2;
    static constexpr int PATH_TYPE =3;
protected:
    int mType;
    std::string mName;
public:
    Property(const std::string&name);
    Property(const std::string&name,int type);
    virtual AnimateValue get(void* t)const;
    virtual void set(void* object,const AnimateValue& value)const;
    const std::string getName()const;
    int getType()const;
    static Property*fromName(const std::string&propertyName);
    static Property*fromName(const std::string&className,const std::string&propertyName);
    static bool reigsterProperty(const std::string&name,Property*prop);
};

class FloatProperty:public Property{
public:
    FloatProperty(const std::string& name):Property(name,FLOAT_TYPE){
    }

    virtual void setValue(void* object, float value)const{
        set(object,value);
    }

    float getValue(void*object)const{
        return GET_VARIANT(get(object),float);
    };
};

}/*endof namespace*/
#endif/*__ANIMATION_PROPERTY_H__*/
