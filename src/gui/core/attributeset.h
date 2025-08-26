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
#ifndef __ATTRIBUTESET_H__
#define __ATTRIBUTESET_H__
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <core/displaymetrics.h>

namespace cdroid{
class Drawable;
class ColorStateList;
class Context;
class XmlPullParser;
class AttributeSet{
protected:
    std::string mPackage;
    Context*mContext;
    std::shared_ptr<std::unordered_map<std::string,std::string>>mAttrs;
public:
    AttributeSet();
    AttributeSet(const AttributeSet&);
    AttributeSet(Context*ctx,const std::string&package);
    virtual ~AttributeSet()=default;
    Context*getContext()const;
    void setContext(Context*,const std::string&package);
    bool add(const std::string&,const std::string&value);
    bool hasAttribute(const std::string&key)const;
    size_t getAttributeCount()const;
    int set(const char*atts[],int size=0);
    static std::string normalize(const std::string&pkg,const std::string&property);
    int inherit(const AttributeSet&other);
    int Override(const AttributeSet&other);
    const std::string getAttributeValue(const std::string&key)const;
    bool getBoolean(const std::string&key,bool def=false)const;
    int getInt(const std::string&key,int def=0)const;
    int getInt(const std::string&key,const std::unordered_map<std::string,int>&keyvaluemaps,int def=0)const;
    int getResourceId(const std::string&key,int def=0)const;
    int getColor(const std::string&key,int def=0xFFFFFFFF)const;
    int getColorWithException(const std::string&key)const;
    float getFloat(const std::string&key,float def=.0)const;
    const std::string getString(const std::string&key,const std::string&def=std::string())const;
    int getGravity(const std::string&key,int defvalue=0)const;

    int getDimension(const std::string&key,int def=0)const;
    int getDimensionPixelSize(const std::string&key,int def=0)const;
    int getDimensionPixelOffset(const std::string&key,int def=0)const;
    int getLayoutDimension(const std::string&key,int def)const;
    float getFraction(const std::string&key,int base,int pbase,float def=.0)const;

    ColorStateList*getColorStateList(const std::string&key)const;
    Drawable*getDrawable(const std::string&key)const;
    int getArray(const std::string&key,std::vector<std::string>&array)const;
    int getArray(const std::string&key,std::vector<int>&array)const;
    AttributeSet& operator =(const AttributeSet&other);
    void dump()const;
};
}
#endif
