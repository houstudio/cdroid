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
#include <core/xmlpullparser.h>
#include <core/xmlblock.h>             // XmlBlock::Parser (detectAndCreate product)
#include <porting/cdlog.h>
#include <core/context.h>
#include <core/color.h>
#include <expat.h>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
#include <vector>

namespace cdroid{

struct XmlEvent {
    XmlPullParser::EventType type;
    int depth;
    int lineNumber;
    int columnNumber;
    std::string name;
    std::string text;
    std::shared_ptr<std::unordered_map<std::string,std::string>>atts;
    XmlEvent(){
        depth = -1;
        lineNumber = -1;
        columnNumber = -1;
        atts = std::make_shared<std::unordered_map<std::string,std::string>>();
    }
    XmlEvent(XmlPullParser::EventType tp):XmlEvent(){type =tp;}
    XmlEvent(XmlPullParser::EventType tp,const std::string&name_):XmlEvent(tp){
       name= name_;
    }
};

struct Private{
    XML_Parser parser;
    int depth;
    std::string resourceId;
    std::string mText;
    std::array <char,512> buffer;
    std::unique_ptr <std::istream> stream;
    std::queue <XmlEvent*> eventQueue;
    std::queue <XmlEvent*> eventPool;
    ~Private(){
        while(eventQueue.size()){
            delete eventQueue.front();
            eventQueue.pop();
        }
        while(eventPool.size()){
            delete eventPool.front();
            eventPool.pop();
        }
    }
    XmlEvent*acquire(XmlPullParser::EventType type,const std::string&text = std::string()){
        if(eventPool.size()==0) eventPool.push(new XmlEvent());
        auto event  = eventPool.front();
        event->name = text;
        event->type = type;
        event->atts->clear();
        event->text.clear();
        event->lineNumber = XML_GetCurrentLineNumber(parser);
        event->columnNumber= XML_GetCurrentColumnNumber(parser);
        eventPool.pop();
        return event;
    }
    void release(XmlEvent*event){
        eventPool.push(event);
    }
};

class XmlPullParser::AttrParser{
public:
    static void startElementHandler(void* userData, const XML_Char* name, const XML_Char** attrs){
        XmlPullParser*parser = static_cast<XmlPullParser*>(userData);
        Private*data = parser->mData;
        auto event = data->acquire(XmlPullParser::START_TAG,name);
        data->mText.clear();
        event->depth = data->depth++;
        for(int i = 0;attrs[i];i+=2){
            const char* nmsp= strrchr(attrs[i],' ');
            const char* attr= attrs[i+1];
            const char* key = nmsp?(nmsp+1):attrs[i];
            event->atts->insert({std::string(key),std::string(attr)});
        }
        data->eventQueue.push(event);
    }
    static void endElementHandler(void* userData, const XML_Char* name){
        Private*data = static_cast<XmlPullParser*>(userData)->mData;
        auto event = data->acquire(XmlPullParser::END_TAG,name);
        const int depth = --data->depth;
        event->depth= depth;
        data->eventQueue.push(event);
    }
    static void characterDataHandler(void* userData, const XML_Char* s, int len){
        Private*data = static_cast<XmlPullParser*>(userData)->mData;
        auto event = data->acquire(XmlPullParser::TEXT,"");
        event->text.append(s,len);
        event->depth = data->depth;
        data->eventQueue.push(event);
    }
};

XmlPullParser::XmlPullParser():XmlPullParser(true){
}

XmlPullParser::XmlPullParser(bool initTextEngine){
    mData = new Private;
    mData->depth = 0;
    mData->parser = nullptr;
    if(initTextEngine){
        mData->parser = XML_ParserCreateNS(nullptr,' ');
        XML_SetUserData(mData->parser, this);
        XML_SetElementHandler(mData->parser, AttrParser::startElementHandler, AttrParser::endElementHandler);
        XML_SetCharacterDataHandler(mData->parser, AttrParser::characterDataHandler);
    }
}

XmlPullParser::XmlPullParser(Context*ctx,std::unique_ptr<std::istream>strm):XmlPullParser(){
    mContext = ctx;
    mData->stream = std::move(strm);
    auto event = mData->acquire((mData->stream&&mData->stream->good())?START_DOCUMENT:END_DOCUMENT);
    event->depth= mData->depth++;
    event->lineNumber = 0;
    mAttrs = event->atts;
    mData->eventQueue.push(event);
}

XmlPullParser::operator bool()const{
    return (mData->stream!=nullptr)&&(*mData->stream);
}

XmlPullParser::~XmlPullParser() {
    if(mData->parser) XML_ParserFree(mData->parser);
    delete mData;
}

int XmlPullParser::getDepth()const{
    return mData->eventQueue.front()->depth;
}

int XmlPullParser::getEventType()const{
    return mData->eventQueue.front()->type;
}

int XmlPullParser::getLineNumber()const{
    return mData->eventQueue.front()->lineNumber;
}

int XmlPullParser::getColumnNumber()const{
    return mData->eventQueue.front()->columnNumber;
}

std::string XmlPullParser::getName()const{
    return mData->eventQueue.front()->name;
}

std::string XmlPullParser::getText()const{
    return mData->eventQueue.front()->text;
}

int XmlPullParser::next(){
    const EventType currentEvent = mData->eventQueue.front()->type;
    if((currentEvent==BAD_DOCUMENT)||(currentEvent==END_DOCUMENT)){
        return currentEvent;
    }
    mData->release(mData->eventQueue.front());
    mData->eventQueue.pop();
    while(mData->eventQueue.empty()){
        std::streamsize len;
        mData->stream->read(mData->buffer.data(),mData->buffer.size());
        len = mData->stream->gcount();
        const bool done = mData->stream->eof();
        if(XML_Parse(mData->parser,mData->buffer.data(),len,done)==XML_STATUS_ERROR){
            const XML_Error xmlError = XML_GetErrorCode(mData->parser);
            const char*errMsg = XML_ErrorString(xmlError);
            LOGE("%d:%s %s:%s",xmlError,errMsg,mData->resourceId.c_str(),getPositionDescription().c_str());
            mData->eventQueue.push(mData->acquire(BAD_DOCUMENT));
            break;
        }
        if(done){
            mData->eventQueue.push(mData->acquire(END_DOCUMENT));
        }
    }
    mAttrs = mData->eventQueue.front()->atts;
    return mData->eventQueue.front()->type;
}

std::string XmlPullParser::getPositionDescription()const{
    std::ostringstream oss;
    oss<<XML_GetCurrentLineNumber(mData->parser)<<":"<<XML_GetCurrentColumnNumber(mData->parser);
    return oss.str();
}

// Single text/binary sniffing point. Slurps the stream once and dispatches on
// the RES_XML_TYPE magic; a failed stream still yields a text parser primed at
// END_DOCUMENT (the object must stay usable — many callers next() blindly).
std::unique_ptr<XmlPullParser> XmlPullParser::detectAndCreate(Context*ctx,
        std::unique_ptr<std::istream> strm,const std::string&resourceId,const std::string&pkg){
    if(strm && *strm){
        std::string data((std::istreambuf_iterator<char>(*strm)),
                         std::istreambuf_iterator<char>());
        strm.reset();
        if(data.size() >= 2 && (uint8_t)data[0] == 0x03 && (uint8_t)data[1] == 0x00){
            return XmlBlock::newParser(ctx,pkg,resourceId,
                    std::vector<uint8_t>(data.begin(),data.end()));
        }
        auto parser = std::unique_ptr<XmlPullParser>(new XmlPullParser(ctx,
                std::make_unique<std::istringstream>(std::move(data))));
        // resourceId/pkg feed the dev-aid logging (same inputs the
        // resource-id ctors kept in Private).
        parser->mData->resourceId = resourceId;
        if(!pkg.empty()) parser->mPackage = pkg;
        return parser;
    }
    return std::unique_ptr<XmlPullParser>(new XmlPullParser(ctx,std::move(strm)));
}

// ----------------------------------------------------------------------------
// AOSP android.util.AttributeSet — text-XML implementation. The expat handler
// stores attribute values verbatim; these coerce them (the
// XmlUtils.convertValueTo* role). Index walks mAttrs (small N; resolution
// matches by resId/name, not position, so the unordered order is fine).
// ----------------------------------------------------------------------------
namespace {
bool keyAt(const std::unordered_map<std::string,std::string>& m, size_t idx, std::string* out) {
    if (idx >= m.size()) return false;
    size_t i = 0;
    for (const auto& kv : m) {
        if (i == idx) {
            *out = kv.first;
            return true;
        }
        i++;
    }
    return false;
}
}

size_t XmlPullParser::getAttributeCount()const{
    return mAttrs->size();
}

std::string XmlPullParser::getAttributeNamespace(int /*index*/) const {
    return std::string();   // text carries no namespace (bare localname keys)
}

std::string XmlPullParser::getAttributeName(int index) const {
    std::string k;
    return keyAt(*mAttrs, (size_t)index, &k) ? k : std::string();
}

std::string XmlPullParser::getAttributeValue(int index) const {
    std::string k;
    return keyAt(*mAttrs, (size_t)index, &k) ? getAttributeValue(std::string(), k) : std::string();
}

std::string XmlPullParser::getAttributeValue(const std::string& /*namespace_*/,
                        const std::string& name) const {
    auto it = mAttrs->find(name);   // namespace-agnostic for text (bare localname keys)
    return it != mAttrs->end() ? it->second : std::string();
}

int XmlPullParser::getAttributeNameResource(int index) const {
    // Text-built sets carry no attr resource ids natively (binary AXML does).
    // Resolve the attribute NAME through the arsc attr table instead —
    // obtainStyledAttributes matches element attributes BY RESOURCE ID, so a
    // text XML (e.g. res/color/ selectors packed as text) otherwise never
    // matches and its items fall to defaults (the MAGENTA ColorStateList bug).
    // Name form: "prefix:name" (prefix is a package) or bare "name".
    std::string key;
    if (!keyAt(*mAttrs, (size_t)index, &key)) return 0;
    std::string pkg, name = key;
    const size_t colon = key.rfind(':');
    if (colon != std::string::npos) {
        pkg = key.substr(0, colon);
        name = key.substr(colon + 1);
    }
    if (name.empty() || mContext == nullptr) return 0;
    return mContext->getResources().getIdentifier(name, "attr", pkg);
}

int XmlPullParser::getAttributeListValue(int index,
        const std::vector<std::string>& options, int defaultValue) const {
    const std::string v = getAttributeValue(index);
    for (size_t i = 0; i < options.size(); i++){
        if (options[i] == v) return (int)i;
    }
    return defaultValue;
}

int XmlPullParser::getAttributeListValue(const std::string& /*namespace_*/,const std::string& attribute,
            const std::vector<std::string>& options, int defaultValue) const {
    const std::string v = getAttributeValue(std::string(), attribute);
    for (size_t i = 0; i < options.size(); i++) if (options[i] == v) return (int)i;
    return defaultValue;
}

bool XmlPullParser::getAttributeBooleanValue(int index, bool defaultValue) const {
    std::string k;
    return keyAt(*mAttrs, (size_t)index, &k) ? getAttributeBooleanValue(std::string(), k, defaultValue) : defaultValue;
}

int XmlPullParser::getAttributeResourceValue(int index, int defaultValue) const {
    std::string k;
    return keyAt(*mAttrs, (size_t)index, &k) ? getAttributeResourceValue(std::string(), k, defaultValue) : defaultValue;
}

int XmlPullParser::getAttributeIntValue(int index, int defaultValue) const {
    std::string k;
    return keyAt(*mAttrs, (size_t)index, &k) ? getAttributeIntValue(std::string(), k, defaultValue) : defaultValue;
}

int XmlPullParser::getAttributeUnsignedIntValue(int index, int defaultValue) const {
    std::string k;
    if (!keyAt(*mAttrs, (size_t)index, &k)) return defaultValue;
    const std::string v = getAttributeValue(std::string(), k);
    if (!v.empty()) {
        if (v[0] == '#') return (int)Color::parseColor(v);
        if (v.size() >= 2 && v[0] == '0' && (v[1] == 'x' || v[1] == 'X'))
            return (int)strtoul(v.c_str() + 2, nullptr, 16);
    }
    return getAttributeIntValue(std::string(), k, defaultValue);
}

float XmlPullParser::getAttributeFloatValue(int index, float defaultValue) const {
    std::string k;
    return keyAt(*mAttrs, (size_t)index, &k) ? getAttributeFloatValue(std::string(), k, defaultValue) : defaultValue;
}

bool XmlPullParser::getAttributeBooleanValue(const std::string& /*namespace_*/,
            const std::string& attribute, bool defaultValue) const {
    const std::string v = getAttributeValue(std::string(), attribute);
    if (v.empty()) return defaultValue;
    return v.compare("true") == 0;
}

int XmlPullParser::getAttributeResourceValue(const std::string& /*namespace_*/,
            const std::string& attribute,int defaultValue) const {
    const std::string v = getAttributeValue(std::string(), attribute);
    if (v.empty()) return defaultValue;
    // "parent" is the ConstraintLayout/RelativeLayout anchor sentinel meaning
    // the parent view (id 0) — NOT a named resource; resolving it hits an
    // unrelated arsc entry named "parent" and breaks parent anchors.
    if (v == "parent") return 0;
    if (v.find_first_of("@+/") != std::string::npos) {
        std::string name = v;
        const size_t slash = name.rfind('/');
        if (slash != std::string::npos) name = name.substr(slash + 1);
        size_t at = 0;
        while (at < name.size() && (name[at]=='@'||name[at]=='+')) at++;
        if (at > 0) name = name.substr(at);
        const int value = mContext ? mContext->getResources().getIdentifier(name, "id", "") : 0;
        return value ? value : defaultValue;
    }
    return (int)std::strtoul(v.c_str(), nullptr, 10);
}

int XmlPullParser::getAttributeIntValue(const std::string& /*namespace_*/,
            const std::string& attribute, int defaultValue) const {
    const std::string v = getAttributeValue(std::string(), attribute);
    if (v.empty() || ((v[0] >= 'a') && (v[0] <= 'z'))) return defaultValue;
    const int base = (((v.length() > 2) && (v[1]=='x'||v[1]=='X')) || (v[0]=='#')) ? 16 : 10;
    return (int)std::strtol(v.c_str(), nullptr, base);
}

int XmlPullParser::getAttributeUnsignedIntValue(const std::string& /*namespace_*/,
            const std::string& attribute, int defaultValue) const {
    const std::string v = getAttributeValue(std::string(), attribute);
    if (!v.empty()) {
        if (v[0] == '#')
            return (int)Color::parseColor(v);
        if (v.size() >= 2 && v[0] == '0' && (v[1] == 'x' || v[1] == 'X'))
            return (int)strtoul(v.c_str() + 2, nullptr, 16);
    }
    return getAttributeIntValue(std::string(), attribute, defaultValue);
}

float XmlPullParser::getAttributeFloatValue(const std::string& /*namespace_*/,
            const std::string& attribute,float defaultValue) const {
    const std::string v = getAttributeValue(std::string(), attribute);
    if (v.empty()) return defaultValue;
    return std::strtof(v.c_str(), nullptr);
}

std::string XmlPullParser::getIdAttribute() const {
    return getAttributeValue(std::string(), "id");
}

std::string XmlPullParser::getClassAttribute() const {
    return getAttributeValue(std::string(), "class");
}

int XmlPullParser::getIdAttributeResourceValue(int defaultValue) const {
    return getAttributeResourceValue(std::string(), "id", defaultValue);
}

int XmlPullParser::getStyleAttribute() const {
    return getAttributeResourceValue(std::string(), "style", 0);
}

}/*endof namespace*/
