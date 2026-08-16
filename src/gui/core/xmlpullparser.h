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
#ifndef __XML_PULLPARSER_H__
#define __XML_PULLPARSER_H__
#include <string>
#include <queue>
#include <memory>
#include <core/attributeset.h>
namespace cdroid{
class XmlPullParser:public AttributeSet{
public:
    enum EventType {
        BAD_DOCUMENT,
        START_DOCUMENT,
        END_DOCUMENT,

        START_TAG,
        END_TAG,
        TEXT,
        COMMENT
    };
private:
    class AttrParser;
    struct Private* mData;
    XmlPullParser();
public:
    XmlPullParser(Context*ctx,const std::string&resid);
    XmlPullParser(Context*ctx,int resid);
    XmlPullParser(Context*,std::unique_ptr<std::istream>);
    ~XmlPullParser()override;
    int getDepth()const;
    std::string getName()const;
    std::string getText()const;
    std::string getPositionDescription()const;
    int getEventType()const;
    int getLineNumber()const;
    int getColumnNumber()const;
    int next();
    operator bool()const;
    // Phase 2: expose binary AXML state for TypedArray obtainStyledAttributes.
    // Returns true if parsing binary AXML (has a ResXMLTree).
    bool isBinaryAXML() const;
    // Returns the ResXMLTree* (as void* to avoid the heavy androidfw include here).
    // Null for text XML. Caller (which has androidfw) casts to const ResXMLTree*.
    const void* getBinaryAXMLTree() const;

    // AOSP AttributeSet id-interface — binary overrides. For binary AXML the
    // index is the ResXMLTree attribute order and values are the typed Res_value
    // (aapt2-pre-resolved), so these return real attr resIds / typed data instead
    // of the base impl's string parsing. Non-binary falls through to AttributeSet.
    // The using-declarations keep AttributeSet's string-key overloads visible
    // (the (int) overrides would otherwise hide them — C++ name hiding).
    using AttributeSet::getAttributeValue;
    using AttributeSet::getAttributeBooleanValue;
    using AttributeSet::getAttributeIntValue;
    using AttributeSet::getAttributeResourceValue;
    using AttributeSet::getAttributeUnsignedIntValue;
    using AttributeSet::getAttributeFloatValue;
    std::string getAttributeName(int index) const override;
    std::string getAttributeValue(int index) const override;
    int getAttributeNameResource(int index) const override;
    bool getAttributeBooleanValue(int index, bool defaultValue) const override;
    int getAttributeResourceValue(int index, int defaultValue) const override;
    int getAttributeIntValue(int index, int defaultValue) const override;
    int getAttributeUnsignedIntValue(int index, int defaultValue) const override;
    float getAttributeFloatValue(int index, float defaultValue) const override;
    // Name-keyed typed lookups (AOSP AttributeSet.getAttributeXxxValue(ns, name, def)
    // and the style= special): binary resolves the attr index by name, then reads
    // the typed Res_value through the (int) overrides above.
    int getStyleAttribute() const override;
    bool getAttributeBooleanValue(const std::string& namespace_,
                                  const std::string& attribute, bool defaultValue) const override;
    int getAttributeResourceValue(const std::string& namespace_,
                                  const std::string& attribute, int defaultValue) const override;
    int getAttributeIntValue(const std::string& namespace_,
                             const std::string& attribute, int defaultValue) const override;
    // Name-based value / count / presence — resolve straight from ResXMLTree on
    // binary AXML (no mAttrs bridge). Lets the binary parser serve the AOSP
    // (namespace, name) lookups after the eager mAttrs populate is retired.
    std::string getAttributeValue(const std::string& namespace_,
                                  const std::string& name) const override;
    bool hasAttribute(const std::string& key) const override;
    size_t getAttributeCount() const override;
    // Debug dump: binary prints each attribute's raw typed Res_value (attr
    // resId + type + data, like the resources dump) plus the rendered text;
    // text XML falls through to AttributeSet::dump().
    void dump() const override;
private:
    // Find a binary-AXML attribute by bare localname; returns its index or -1.
    int binaryAttrIndex(const std::string& name) const;
};
}/*endof namespace*/
#endif /*__XML_PULLPARSER_H__*/
