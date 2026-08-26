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
#ifndef __XMLBLOCK_H__
#define __XMLBLOCK_H__
#include <string>
#include <vector>
#include <memory>
#include <core/xmlpullparser.h>

class ResXMLTree;   // androidfw — kept out of the installed public headers

namespace cdroid{

// Port of android.content.res.XmlBlock: owns the binary-AXML bytes and their
// ResXMLTree, and hands out parsers over them (XmlBlock.newParser()).
class XmlBlock{
public:
    // AOSP XmlBlock(byte[] data). data: raw binary AXML bytes (first two
    // bytes 0x03 0x00 = RES_XML_TYPE).
    XmlBlock(std::vector<uint8_t>data);
    ~XmlBlock();
    // True when the ResXMLTree parsed clean (AOSP surfaces this via
    // nativeCreate failing / getError).
    bool valid()const;

    class Parser;
    // AOSP XmlBlock.newParser() → XmlResourceParser. C++ adaptation: no GC /
    // mOpenCount here, and a ResXMLTree is a single cursor, so the ownership
    // is handed to the parser — one parser per block, never shared. The tree
    // must also outlive any TypedArray built from the parser (TypedArray
    // keeps the const ResXMLTree*), which this ownership guarantees.
    static std::unique_ptr<Parser>newParser(Context*ctx,
            const std::string&resourceId,std::vector<uint8_t>data);

private:
    friend class Parser;
    // mBytes must precede mTree: ResXMLTree::setTo borrows the buffer.
    std::vector<uint8_t> mBytes;
    std::unique_ptr<ResXMLTree> mTree;
};

// Port of android.content.res.XmlBlock.Parser — the binary-AXML pull parser.
// Drives the ResXMLParser directly from next(), tracking event/depth state in
// members (AOSP member shape, no event queue).
class XmlBlock::Parser:public XmlPullParser{
public:
    // AOSP's Parser(long parseState, XmlBlock block) is package-private; the
    // C++ shape takes block ownership (see XmlBlock::newParser).
    Parser(Context*ctx,const std::string&resourceId,
            std::unique_ptr<XmlBlock>block);
    ~Parser()override;

    int getDepth()const override;
    std::string getName()const override;
    std::string getText()const override;
    std::string getPositionDescription()const override;
    int getEventType()const override;
    int getLineNumber()const override;
    int getColumnNumber()const override;
    int next()override;
    operator bool()const override;

    // The binary parse state (AOSP keeps mParseState package-visible for
    // AssetManager.applyStyle; CDROID's resolver reaches it the same way,
    // by downcasting to this concrete class). Typed, unlike the old base
    // void* seam. Null when the tree did not parse clean.
    const ResXMLTree* getResXMLTree()const;

    // AOSP android.util.AttributeSet interface (index/id-based). The
    // using-declarations keep the name-keyed overloads visible (the (int)
    // overrides would otherwise hide them — C++ name hiding), mirroring
    // XmlPullParser.
    using XmlPullParser::getAttributeValue;
    using XmlPullParser::getAttributeBooleanValue;
    using XmlPullParser::getAttributeIntValue;
    using XmlPullParser::getAttributeResourceValue;
    using XmlPullParser::getAttributeUnsignedIntValue;
    using XmlPullParser::getAttributeFloatValue;
    std::string getAttributeName(int index)const override;
    std::string getAttributeValue(int index)const override;
    int getAttributeNameResource(int index)const override;
    bool getAttributeBooleanValue(int index,bool defaultValue)const override;
    int getAttributeResourceValue(int index,int defaultValue)const override;
    int getAttributeIntValue(int index,int defaultValue)const override;
    int getAttributeUnsignedIntValue(int index,int defaultValue)const override;
    float getAttributeFloatValue(int index,float defaultValue)const override;
    int getStyleAttribute()const override;
    bool getAttributeBooleanValue(const std::string&namespace_,
            const std::string&attribute,bool defaultValue)const override;
    int getAttributeResourceValue(const std::string&namespace_,
            const std::string&attribute,int defaultValue)const override;
    int getAttributeIntValue(const std::string&namespace_,
            const std::string&attribute,int defaultValue)const override;
    std::string getAttributeValue(const std::string&namespace_,
            const std::string&name)const override;
    size_t getAttributeCount()const override;
    void dump()const override;
private:
    // Find an attribute by bare localname; returns its index or -1.
    int binaryAttrIndex(const std::string&name)const;
    // Render a typed Res_value to a string when no rawValue is available.
    std::string renderTypedValue(size_t attrIdx)const;
    // Dev aid: flag attributes aapt2 could not resolve to a resource id.
    void warnUnnamedAttributes()const;
private:
    // The context that opened the resource (impl state: reference rendering
    // and the dev-aid logging resolve through it).
    Context* mContext;
    std::unique_ptr<XmlBlock> mBlock;
    // Cached tree cursor — AOSP's Parser caches its native mParseState the
    // same way instead of reaching through mBlock on every call.
    ResXMLTree* mTree;
    // Event state (AOSP XmlBlock.Parser member shape; the reported depths
    // reproduce the old event-queue numbers exactly: root tag reports 1, its
    // END_TAG 1, children one deeper, ...).
    int mEventDepth;
    int mDepth;
    int mEventType;
    std::string mName;
    std::string mText;
    int mLineNumber;
    std::string mResourceId;
};
}/*endof namespace*/
#endif /*__XMLBLOCK_H__*/
