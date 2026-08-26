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
#include <androidfw/resourcetypes.h>   // Res_value/ResXMLTree
#include <core/typedvalue.h>           // TypedValue (typed currency)
#include <core/xmlblock.h>
#include <porting/cdlog.h>
#include <core/context.h>
#include <core/app.h>
#include <core/assets.h>
#include <core/resources.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace cdroid{

// androidfw glue (same seam as xmlpullparser.cc): fill a TypedValue
// from the raw Res_value the AXML tree hands out.
static TypedValue tvOf(const Res_value& rv) {
    TypedValue tv; tv.type = rv.dataType; tv.data = rv.data; return tv;
}

// Decode a TYPE_DIMENSION complex value to its float magnitude.
static float axmlComplexToFloat(uint32_t data) {
    const uint32_t radix = (data >> TypedValue::COMPLEX_RADIX_SHIFT) & TypedValue::COMPLEX_RADIX_MASK;
    const uint32_t mantissa = (data >> TypedValue::COMPLEX_MANTISSA_SHIFT) & TypedValue::COMPLEX_MANTISSA_MASK;
    switch (radix) {
        case TypedValue::COMPLEX_RADIX_23p0: return (float)(int32_t)mantissa;
        case TypedValue::COMPLEX_RADIX_16p7: return mantissa * (1.0f / (1 << 7));
        case TypedValue::COMPLEX_RADIX_8p15: return mantissa * (1.0f / (1 << 15));
        default: return mantissa * (1.0f / (1 << 23));
    }
}

static std::string u16toUtf8(const char16_t* s, size_t len){
    std::string out;
    for(size_t i = 0; s && i < len; i++){
        uint32_t c = s[i];
        if(c >= 0xD800 && c <= 0xDBFF && i + 1 < len && s[i+1] >= 0xDC00)
            c = 0x10000 + ((c - 0xD800) << 10) + (s[++i] - 0xDC00);
        if(c < 0x80) out += (char)c;
        else if(c < 0x800){ out += (char)(0xC0|(c>>6)); out += (char)(0x80|(c&0x3F)); }
        else if(c < 0x10000){ out += (char)(0xE0|(c>>12)); out += (char)(0x80|((c>>6)&0x3F)); out += (char)(0x80|(c&0x3F)); }
        else { out += (char)(0xF0|(c>>18)); out += (char)(0x80|((c>>12)&0x3F)); out += (char)(0x80|((c>>6)&0x3F)); out += (char)(0x80|(c&0x3F)); }
    }
    return out;
}

XmlBlock::XmlBlock(std::vector<uint8_t>data)
    :mBytes(std::move(data)),mTree(new ResXMLTree()){
    mTree->setTo(mBytes.data(),mBytes.size());
}

XmlBlock::~XmlBlock(){
}

bool XmlBlock::valid()const{
    return mTree->getError()==0;
}

std::unique_ptr<XmlBlock::Parser>XmlBlock::newParser(Context*ctx,const std::string&pkg,
        const std::string&resourceId,std::vector<uint8_t>data){
    return std::make_unique<Parser>(ctx,pkg,resourceId,
            std::unique_ptr<XmlBlock>(new XmlBlock(std::move(data))));
}

XmlBlock::Parser::Parser(Context*ctx,const std::string&pkg,const std::string&resourceId,
        std::unique_ptr<XmlBlock>block)
    :XmlPullParser(false),mBlock(std::move(block)),mTree(mBlock ? mBlock->mTree.get() : nullptr),
     mEventDepth(0),mDepth(1),mEventType(XmlPullParser::START_DOCUMENT),
     mLineNumber(0),mResourceId(resourceId){
    mContext = ctx;
    mPackage = pkg;
}

XmlBlock::Parser::~Parser(){
}

int XmlBlock::Parser::getDepth()const{
    return mEventDepth;
}

std::string XmlBlock::Parser::getName()const{
    return mName;
}

std::string XmlBlock::Parser::getText()const{
    return mText;
}

std::string XmlBlock::Parser::getPositionDescription()const{
    return "Binary XML file line #"+std::to_string(getLineNumber());
}

int XmlBlock::Parser::getEventType()const{
    return mEventType;
}

int XmlBlock::Parser::getLineNumber()const{
    return mLineNumber;
}

int XmlBlock::Parser::getColumnNumber()const{
    return -1;
}

XmlBlock::Parser::operator bool()const{
    return mTree && mTree->getError()==0;
}

bool XmlBlock::Parser::isBinaryAXML()const{
    return mTree && mTree->getError()==0;
}

const void* XmlBlock::Parser::getBinaryAXMLTree()const{
    return isBinaryAXML() ? static_cast<const void*>(mTree) : nullptr;
}

// Drive the ResXMLParser one event at a time (the AOSP XmlBlock.Parser.next
// shape). Namespace and START_DOCUMENT events are skipped, empty TEXT is
// skipped, and parse failure collapses to END_DOCUMENT (CDROID's event-code
// contract; AOSP throws a XmlPullParserException instead). The first call
// advances past START_DOCUMENT and returns the first real event — the
// queue-fed parser's protocol.
int XmlBlock::Parser::next(){
    if(mEventType==END_DOCUMENT||mEventType==BAD_DOCUMENT) return mEventType;
    if(!mTree){ mEventType = END_DOCUMENT; return mEventType; }
    size_t iters = 0;
    while(true){
        ResXMLParser::event_code_t ev = mTree->next();
        if(++iters > 200){ LOGE("XmlBlock::Parser spin (iter>200) ev=%d — aborting element", (int)ev);
            mEventType = END_DOCUMENT; return mEventType; }
        switch(ev){
            case ResXMLParser::START_TAG:{
                size_t nl = 0;
                const char16_t* n16 = mTree->getElementName(&nl);
                mName = u16toUtf8(n16, nl);
                mText.clear();
                mEventDepth = mDepth++;
                mLineNumber = mTree->getLineNumber();
                warnUnnamedAttributes();
                mEventType = START_TAG;
                return mEventType;
            }
            case ResXMLParser::END_TAG:{
                size_t nl = 0;
                const char16_t* n16 = mTree->getElementName(&nl);
                mName = u16toUtf8(n16, nl);
                mText.clear();
                mEventDepth = --mDepth;
                mLineNumber = mTree->getLineNumber();
                mEventType = END_TAG;
                return mEventType;
            }
            case ResXMLParser::TEXT:{
                size_t tl = 0;
                const char16_t* t16 = mTree->getText(&tl);
                if(tl > 0){
                    mName.clear();
                    mText = u16toUtf8(t16, tl);
                    mEventDepth = mDepth;
                    mLineNumber = mTree->getLineNumber();
                    mEventType = TEXT;
                    return mEventType;
                }
                break; // empty text — skip
            }
            case ResXMLParser::END_DOCUMENT:
            case ResXMLParser::BAD_DOCUMENT:
                mEventType = END_DOCUMENT;
                return mEventType;
            default: // START_DOCUMENT, START/END_NAMESPACE — skipped
                break;
        }
    }
}

// Dev aid: an attribute aapt2 could not resolve to a resource id (typically a
// missing android:/app: prefix — unprefixed names get no id baked into the
// binary AXML) is invisible to every id-based lookup (obtainStyledAttributes /
// getAttributeNameResource) and is silently dropped. Android behaves the same,
// but it breaks layouts in confusing ways (e.g. an unprefixed layout_width in
// a MotionScene <Constraint> collapses the view to 0dp), so flag it here.
// Directive tags (<merge>/<requestFocus>/<tag>) carry no view attributes at
// all — nothing to warn about there.
void XmlBlock::Parser::warnUnnamedAttributes()const{
    if(mName == "merge" || mName == "requestFocus" || mName == "tag") return;
    const size_t ac = mTree->getAttributeCount();
    for(size_t i = 0; i < ac; i++){
        if(mTree->getAttributeNameResID(i) != 0) continue;
        size_t anLen = 0;
        const char16_t* an = mTree->getAttributeName(i, &anLen);
        const std::string attrName = u16toUtf8(an, anLen);
        // Namespace-less system attributes are read BY NAME in AOSP
        // (getAttributeValue(null, ...)) and legitimately carry no resource
        // id: style, <view>/<fragment> class, <include> layout.
        if(attrName == "style" || attrName == "class" || attrName == "layout") continue;
        // Best-effort source name: the resource-id ctor stores a numeric id
        // string; resolve it to pkg:type/name for the log.
        std::string src = mResourceId;
        if(mContext != nullptr && !mResourceId.empty()
                && mResourceId.find_first_not_of("0123456789") == std::string::npos){
            std::string resName;
            if(mContext->getResources().getResourceName(
                    atoi(mResourceId.c_str()), &resName)) src = resName;
        }
        LOGD("binary AXML '%s' line %d: attribute '%s' on <%s> has no "
             "resource id (missing android:/app: prefix?) — id-based "
             "lookups will ignore it",
             src.c_str(), mLineNumber, attrName.c_str(), mName.c_str());
    }
}

// AOSP android.util.AttributeSet — index = ResXMLTree attribute order; values
// come straight from the typed Res_value (aapt2 already resolved enums/refs).
std::string XmlBlock::Parser::getAttributeName(int index) const {
    size_t len = 0;
    const char16_t* n = mTree->getAttributeName((size_t)index, &len);
    return n ? u16toUtf8(n, len) : std::string();
}

std::string XmlBlock::Parser::getAttributeValue(int index) const {
    return renderTypedValue((size_t)index);
}

int XmlBlock::Parser::getAttributeNameResource(int index) const {
    return (int)mTree->getAttributeNameResID((size_t)index);
}

bool XmlBlock::Parser::getAttributeBooleanValue(int index, bool defaultValue) const {
    Res_value v;
    if(mTree->getAttributeValue((size_t)index, &v) == sizeof(Res_value)
        && v.dataType == TypedValue::TYPE_INT_BOOLEAN) return v.data != 0;
    return defaultValue;
}

int XmlBlock::Parser::getAttributeResourceValue(int index, int defaultValue) const {
    Res_value v;
    if(mTree->getAttributeValue((size_t)index, &v) == sizeof(Res_value)
        && (v.dataType == TypedValue::TYPE_REFERENCE || v.dataType == TypedValue::TYPE_ATTRIBUTE
            || v.dataType == TypedValue::TYPE_DYNAMIC_REFERENCE)) return (int)v.data;
    return defaultValue;
}

int XmlBlock::Parser::getAttributeIntValue(int index, int defaultValue) const {
    Res_value v;
    if(mTree->getAttributeValue((size_t)index, &v) == sizeof(Res_value)
        && (v.dataType == TypedValue::TYPE_INT_DEC || v.dataType == TypedValue::TYPE_INT_HEX)) return (int)v.data;
    return defaultValue;
}

int XmlBlock::Parser::getAttributeUnsignedIntValue(int index, int defaultValue) const {
    Res_value v;
    if(mTree->getAttributeValue((size_t)index, &v) == sizeof(Res_value)
        && (v.dataType == TypedValue::TYPE_INT_DEC || v.dataType == TypedValue::TYPE_INT_HEX)) return (int)v.data;
    return defaultValue;
}

float XmlBlock::Parser::getAttributeFloatValue(int index, float defaultValue) const {
    Res_value v;
    if(mTree->getAttributeValue((size_t)index, &v) == sizeof(Res_value)
        && v.dataType == TypedValue::TYPE_FLOAT) {
        float f; memcpy(&f, &v.data, sizeof(f)); return f;
    }
    return defaultValue;
}

// Find an attribute by bare localname (iterate the ResXMLTree attrs).
int XmlBlock::Parser::binaryAttrIndex(const std::string& name) const {
    const size_t ac = mTree->getAttributeCount();
    for(size_t i = 0; i < ac; i++){
        size_t nl = 0;
        const char16_t* n = mTree->getAttributeName(i, &nl);
        if(n && u16toUtf8(n, nl) == name) return (int)i;
    }
    return -1;
}

// AOSP getAttributeValue(ns, name): resolve by name, then render.
std::string XmlBlock::Parser::getAttributeValue(const std::string& /*namespace_*/,
                                             const std::string& name) const {
    const int i = binaryAttrIndex(name);
    return i >= 0 ? getAttributeValue(i) : std::string();
}

// Debug dump — prints the raw typed data (attr resId + Res_value type/data,
// same shape as the resources dump) plus the rendered text value.
void XmlBlock::Parser::dump() const {
    const size_t ac = mTree->getAttributeCount();
    for(size_t i = 0; i < ac; i++){
        Res_value v;
        const bool have = mTree->getAttributeValue(i, &v) == sizeof(Res_value);
        LOGD("[%zu] %s (attr 0x%08x): type=0x%x data=0x%x  \"%s\"", i,
             getAttributeName((int)i).c_str(),
             mTree->getAttributeNameResID(i),
             have ? v.dataType : 0, have ? v.data : 0u,
             getAttributeValue((int)i).c_str());
    }
}

// Name-keyed typed lookups: resolve the attr index by name, then read the
// typed Res_value through the (int) overrides above.
int XmlBlock::Parser::getStyleAttribute() const {
    const int i = binaryAttrIndex("style");
    return i >= 0 ? getAttributeResourceValue(i, 0) : 0;
}

bool XmlBlock::Parser::getAttributeBooleanValue(const std::string& /*namespace_*/,
        const std::string& attribute, bool defaultValue) const {
    const int i = binaryAttrIndex(attribute);
    return i >= 0 ? getAttributeBooleanValue(i, defaultValue) : defaultValue;
}

int XmlBlock::Parser::getAttributeResourceValue(const std::string& /*namespace_*/,
        const std::string& attribute, int defaultValue) const {
    const int i = binaryAttrIndex(attribute);
    return i >= 0 ? getAttributeResourceValue(i, defaultValue) : defaultValue;
}

int XmlBlock::Parser::getAttributeIntValue(const std::string& /*namespace_*/,
        const std::string& attribute, int defaultValue) const {
    const int i = binaryAttrIndex(attribute);
    return i >= 0 ? getAttributeIntValue(i, defaultValue) : defaultValue;
}

size_t XmlBlock::Parser::getAttributeCount() const {
    return mTree->getAttributeCount();
}

// Render a typed Res_value to a string when no rawValue is available.
std::string XmlBlock::Parser::renderTypedValue(size_t attrIdx) const {
    Context* ctx = mContext;
    Res_value rv;
    if(mTree->getAttributeValue(attrIdx, &rv) != sizeof(Res_value)) return "";
    const TypedValue v = tvOf(rv);
    char buf[32];
    switch(v.type){
        case TypedValue::TYPE_STRING:{
            size_t len = 0;
            const char16_t* s = mTree->getStrings().stringAt(v.data, &len);
            return s ? u16toUtf8(s, len) : "";
        }
        case TypedValue::TYPE_INT_DEC:
            snprintf(buf, sizeof(buf), "%d", (int)v.data);
            return buf;
        case TypedValue::TYPE_INT_HEX:
            snprintf(buf, sizeof(buf), "0x%x", v.data);
            return buf;
        case TypedValue::TYPE_INT_BOOLEAN:
            return v.data ? "true" : "false";
        case TypedValue::TYPE_INT_COLOR_ARGB8:
        case TypedValue::TYPE_INT_COLOR_RGB8:
        case TypedValue::TYPE_INT_COLOR_ARGB4:
        case TypedValue::TYPE_INT_COLOR_RGB4:
            snprintf(buf, sizeof(buf), "#%08x", v.data);
            return buf;
        case TypedValue::TYPE_DIMENSION:{
            float mag = axmlComplexToFloat(v.data);
            int unit = (v.data >> TypedValue::COMPLEX_UNIT_SHIFT) & TypedValue::COMPLEX_UNIT_MASK;
            const char* u = unit == TypedValue::COMPLEX_UNIT_SP ? "sp"
                          : unit == TypedValue::COMPLEX_UNIT_DIP ? "dp" : "px";
            snprintf(buf, sizeof(buf), "%d%s", (int)mag, u);
            return buf;
        }
        case TypedValue::TYPE_FLOAT:{
            snprintf(buf, sizeof(buf), "%f", v.getFloat());
            return buf;
        }
        case TypedValue::TYPE_REFERENCE:
        case TypedValue::TYPE_DYNAMIC_REFERENCE:
            // Render as an "@type/key" reference string (e.g. "@drawable/bg",
            // "@string/hello", "@android:color/holo_orange") — the same form
            // text XML uses — so the consuming widget's resolver
            // (getDrawable/getString/getColor/...) handles it unchanged.
            // Falls back to "@0xRESID" if the arsc can't name the resource.
            if(ctx && v.data != 0 && v.data != 0xFFFFFFFF){
                Assets* assets = dynamic_cast<Assets*>(ctx);
                if(assets){
                    std::string ref = ctx->getResourceName(v.data);
                    if(!ref.empty()) return ref;
                }
            }
            snprintf(buf, sizeof(buf), "@0x%08x", v.data);
            return buf;
        case TypedValue::TYPE_ATTRIBUTE:
        case TypedValue::TYPE_DYNAMIC_ATTRIBUTE:{
            // A theme-attribute reference "?type/key" (e.g. "?android:attr/
            // colorPrimary"). Rendered with '?' so AttributeSet routes it to
            // obtainStyledAttributes (theme lookup) instead of treating it as
            // a plain resource reference and handing it to getInputStream.
            if(ctx && v.data != 0 && v.data != 0xFFFFFFFF){
                Assets* assets = dynamic_cast<Assets*>(ctx);
                if(assets){
                    TypedValue tv;
                    if(assets->arscThemeAttribute(v.data, &tv)){
                        switch(tv.type){
                            case TypedValue::TYPE_INT_COLOR_ARGB8:
                            case TypedValue::TYPE_INT_COLOR_RGB8:
                            case TypedValue::TYPE_INT_COLOR_ARGB4:
                            case TypedValue::TYPE_INT_COLOR_RGB4:
                                snprintf(buf, sizeof(buf), "#%08x", tv.data); return buf;
                            case TypedValue::TYPE_INT_DEC:
                                snprintf(buf, sizeof(buf), "%d", (int)tv.data); return buf;
                            case TypedValue::TYPE_INT_HEX:
                                snprintf(buf, sizeof(buf), "0x%x", tv.data); return buf;
                            case TypedValue::TYPE_INT_BOOLEAN:
                                return tv.data ? "true" : "false";
                            case TypedValue::TYPE_DIMENSION:{
                                float mag = axmlComplexToFloat(tv.data);
                                int unit = (tv.data >> TypedValue::COMPLEX_UNIT_SHIFT) & TypedValue::COMPLEX_UNIT_MASK;
                                const char* u = unit == TypedValue::COMPLEX_UNIT_SP ? "sp"
                                              : unit == TypedValue::COMPLEX_UNIT_DIP ? "dp" : "px";
                                snprintf(buf, sizeof(buf), "%d%s", (int)mag, u); return buf;
                            }
                            case TypedValue::TYPE_REFERENCE:
                            case TypedValue::TYPE_DYNAMIC_REFERENCE:{
                                std::string ref = ctx->getResourceName(tv.data);
                                if(!ref.empty()) return ref;
                                break;
                            }
                            default: break;  // STRING etc. — fall through to ?type/key
                        }
                    }
                    std::string ref = ctx->getResourceName(v.data);
                    if(!ref.empty()){ if(ref[0] == '@') ref[0] = '?'; return ref; }
                }
            }
            snprintf(buf, sizeof(buf), "?0x%08x", v.data);
            return buf;
        }
        default:
            snprintf(buf, sizeof(buf), "0x%08x", v.data);
            return buf;
    }
}

}/*endof namespace*/
