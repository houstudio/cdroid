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
template <typename T>
using RefPtr = std::shared_ptr<T>;
class AttributeSet{
protected:
    std::string mPackage;
    Context*mContext;
    std::shared_ptr<std::unordered_map<std::string,std::string>>mAttrs;
public:
    AttributeSet();
    AttributeSet(const AttributeSet&);
    // Shared empty AttributeSet — non-null stand-in for a widget ctor's null
    // AttributeSet* (programmatic styled construction), so attribute reads return
    // defaults instead of dereferencing null.
    static const AttributeSet& empty();
    AttributeSet(Context*ctx,const std::string&package);
    virtual ~AttributeSet()=default;
    Context*getContext()const;
    void setContext(Context*,const std::string&package);
    bool add(const std::string&,const std::string&value);
    virtual bool hasAttribute(const std::string&key)const;
    virtual size_t getAttributeCount()const;
    // Qualify a bare XML value ("@mipmap/x", "?attr/x") into "pkg:type/name"
    // form. Used by the text-XML paths building string attribute sets.
    static std::string normalize(const std::string&pkg,const std::string&property);
    // String-key value lookup. Virtual so a binary XmlPullParser can resolve by
    // name straight from its ResXMLTree (no mAttrs bridge). Call sites pass a
    // std::string (not const char*) so the virtual dispatch is not bypassed.
    virtual const std::string getAttributeValue(const std::string&key)const;



    // --- AOSP android.util.AttributeSet interface (index/id-based) -------------
    // Ported verbatim from frameworks/base/core/java/android/util/AttributeSet.java.
    // These are the index/namespace-based, typed, AOSP-faithful methods (the old
    // CDROID string-key getters are retired). Virtual so XmlPullParser
    // (binary AXML) overrides them via ResXMLTree; the base impl works off mAttrs.
    // String return values use std::string (empty == AOSP null).
    virtual std::string getAttributeNamespace(int index) const;          // default ""
    virtual std::string getAttributeName(int index) const;               // "" if not found
    virtual std::string getAttributeValue(int index) const;              // "" if not found
    virtual std::string getAttributeValue(const std::string& namespace_,
                                          const std::string& name) const;
    virtual std::string getPositionDescription() const;
    // Resource id associated with the attribute NAME (the attr's own id), 0 if none.
    virtual int getAttributeNameResource(int index) const;
    virtual int getAttributeListValue(int index, const std::vector<std::string>& options,
                                      int defaultValue) const;
    virtual bool getAttributeBooleanValue(int index, bool defaultValue) const;
    // The attribute's VALUE as a resource id ("@type/key"), 0 if none.
    virtual int getAttributeResourceValue(int index, int defaultValue) const;
    virtual int getAttributeIntValue(int index, int defaultValue) const;
    virtual int getAttributeUnsignedIntValue(int index, int defaultValue) const;
    virtual float getAttributeFloatValue(int index, float defaultValue) const;
    virtual int getAttributeListValue(const std::string& namespace_,
                                      const std::string& attribute,
                                      const std::vector<std::string>& options,
                                      int defaultValue) const;
    virtual bool getAttributeBooleanValue(const std::string& namespace_,
                                          const std::string& attribute,
                                          bool defaultValue) const;
    virtual int getAttributeResourceValue(const std::string& namespace_,
                                          const std::string& attribute,
                                          int defaultValue) const;
    virtual int getAttributeIntValue(const std::string& namespace_,
                                     const std::string& attribute, int defaultValue) const;
    virtual int getAttributeUnsignedIntValue(const std::string& namespace_,
                                             const std::string& attribute,
                                             int defaultValue) const;
    virtual float getAttributeFloatValue(const std::string& namespace_,
                                         const std::string& attribute,
                                         float defaultValue) const;
    // The "id"/"class"/"style" special attributes (AOSP semantics).
    virtual std::string getIdAttribute() const;             // == getAttributeValue("id")
    virtual std::string getClassAttribute() const;          // == getAttributeValue("class")
    virtual int getIdAttributeResourceValue(int defaultValue) const;
    virtual int getStyleAttribute() const;                  // getAttributeResourceValue("style")
    AttributeSet& operator =(const AttributeSet&other);
    // Debug print of the present attributes as text (text sets: name = value;
    // a binary XmlPullParser overrides this to print each attribute's raw
    // typed Res_value — attr resId + type + data — alongside the rendered text).
    virtual void dump()const;
};
}
#endif
