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
#include <core/displaymetrics.h>

namespace cdroid{
class Drawable;
class ColorStateList;
class Context;
class XmlPullParser;
template <typename T>
using RefPtr = std::shared_ptr<T>;

// Port of android.util.AttributeSet — the attribute-view interface over an XML
// element, implemented by the parsers (the text XmlPullParser and the binary
// XmlBlock::Parser) exactly like AOSP's XmlPullAttributes / XmlBlock.Parser
// pair. The base carries no storage: every lookup returns the empty default,
// which also serves synthetic empty sets (AOSP constructs action views with a
// null AttributeSet; CDROID passes one of these).
class AttributeSet{
protected:
    std::string mPackage;
    Context*mContext;
public:
    AttributeSet();
    AttributeSet(Context*ctx,const std::string&package);
    virtual ~AttributeSet()=default;
    // The AOSP interface has no value semantics — copying is deleted so a
    // parser-backed set can never be silently sliced into an empty shell.
    AttributeSet(const AttributeSet&) = delete;
    AttributeSet& operator =(const AttributeSet&) = delete;
    Context*getContext()const;
    void setContext(Context*,const std::string&package);
    // Qualify a bare XML value ("@mipmap/x", "?attr/x") into "pkg:type/name"
    // form. Used by the text-XML paths building string attribute sets.
    static std::string normalize(const std::string&pkg,const std::string&property);

    // --- AOSP android.util.AttributeSet interface ----------------------------
    // Ported verbatim from frameworks/base/core/java/android/util/AttributeSet.java.
    // Empty defaults here; the parser subclasses provide the real answers.
    // String return values use std::string (empty == AOSP null).

    virtual size_t getAttributeCount()const;                              // 0
    virtual bool hasAttribute(const std::string&key)const;                // false
    virtual std::string getAttributeNamespace(int index) const;           // default ""
    virtual std::string getAttributeName(int index) const;                // "" if not found
    virtual std::string getAttributeValue(int index) const;               // "" if not found
    virtual std::string getAttributeValue(const std::string& namespace_,
                                          const std::string& name) const;
    virtual std::string getPositionDescription() const;
    // Resource id associated with the attribute NAME (the attr's own id), 0 if none.
    virtual int getAttributeNameResource(int index) const;                // 0
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
                                     const std::string& attribute,
                                     int defaultValue) const;
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
    // Debug print of the present attributes (loops the virtual index API, so
    // each parser subclass renders its own view).
    virtual void dump()const;
};
}
#endif
