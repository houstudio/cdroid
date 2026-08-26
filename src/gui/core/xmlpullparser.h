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
#include <unordered_map>
#include <core/attributeset.h>
namespace cdroid{
// The expat-backed TEXT-XML pull parser (the org.kxml2.KXmlParser role).
// Binary AXML lives in XmlBlock::Parser; XmlPullParser::detectAndCreate is the
// one place that sniffs which to hand out.
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
protected:
    // Current element's attributes, name -> value (normalize()-qualified).
    // Re-pointed at each event; empty outside a START_TAG..END_TAG span.
    std::shared_ptr<std::unordered_map<std::string,std::string>>mAttrs;
    // Seeding ctors for the implementation hierarchy: the default seeds the
    // expat text engine; XmlPullParser(false) skips it (binary subclass).
    XmlPullParser();
    explicit XmlPullParser(bool initTextEngine);
public:
    // Text-XML entry: takes the stream. Resource-backed construction goes
    // through Resources::getXml / XmlPullParser::detectAndCreate.
    XmlPullParser(Context*,std::unique_ptr<std::istream>);
    ~XmlPullParser()override;
    // Single sniffing point (binary AXML vs text XML, first two bytes
    // 0x03 0x00): returns an XmlBlock::Parser for binary data, a text
    // parser otherwise. A failed stream still yields a usable parser primed
    // at END_DOCUMENT — never returns nullptr.
    static std::unique_ptr<XmlPullParser> detectAndCreate(Context*ctx,std::unique_ptr<std::istream>strm,
            const std::string&resourceId = std::string(),const std::string&pkg = std::string());
    virtual int getDepth()const;
    virtual std::string getName()const;
    virtual std::string getText()const;
    virtual std::string getPositionDescription()const;
    virtual int getEventType()const;
    virtual int getLineNumber()const;
    virtual int getColumnNumber()const;
    virtual int next();
    virtual operator bool()const;
    // Binary AXML state for TypedArray obtainStyledAttributes: always
    // false/nullptr on the text parser (XmlBlock::Parser overrides both).
    virtual bool isBinaryAXML() const;
    virtual const void* getBinaryAXMLTree() const;

    // --- AOSP android.util.AttributeSet — text-XML implementation -----------
    // Values are the normalize()-qualified strings the expat handler stored;
    // typed getters coerce them (the XmlUtils.convertValueTo* role). Index
    // walks the current element's map — unordered, and resolution matches by
    // resId/name, not position (real AXML order only on XmlBlock::Parser).
    // Every overload is declared here, so no base-name hiding can occur.
    size_t getAttributeCount()const override;
    std::string getAttributeNamespace(int index) const override;          // "" (expat keys are bare localnames)
    std::string getAttributeName(int index) const override;
    std::string getAttributeValue(int index) const override;
    std::string getAttributeValue(const std::string& namespace_,
                                  const std::string& name) const override;
    int getAttributeNameResource(int index) const override;               // arsc name->id bridge
    int getAttributeListValue(int index, const std::vector<std::string>& options,
                              int defaultValue) const override;
    int getAttributeListValue(const std::string& namespace_,
                              const std::string& attribute,
                              const std::vector<std::string>& options,
                              int defaultValue) const override;
    bool getAttributeBooleanValue(int index, bool defaultValue) const override;
    bool getAttributeBooleanValue(const std::string& namespace_,
                                  const std::string& attribute, bool defaultValue) const override;
    int getAttributeResourceValue(int index, int defaultValue) const override;
    int getAttributeResourceValue(const std::string& namespace_,
                                  const std::string& attribute, int defaultValue) const override;
    int getAttributeIntValue(int index, int defaultValue) const override;
    int getAttributeIntValue(const std::string& namespace_,
                             const std::string& attribute, int defaultValue) const override;
    int getAttributeUnsignedIntValue(int index, int defaultValue) const override;
    int getAttributeUnsignedIntValue(const std::string& namespace_,
                                     const std::string& attribute, int defaultValue) const override;
    float getAttributeFloatValue(int index, float defaultValue) const override;
    float getAttributeFloatValue(const std::string& namespace_,
                                 const std::string& attribute, float defaultValue) const override;
    std::string getIdAttribute() const override;
    std::string getClassAttribute() const override;
    int getIdAttributeResourceValue(int defaultValue) const override;
    int getStyleAttribute() const override;
};
}/*endof namespace*/
#endif /*__XML_PULLPARSER_H__*/
