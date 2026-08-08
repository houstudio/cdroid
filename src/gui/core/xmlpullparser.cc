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
#include <porting/cdlog.h>
#include <core/context.h>
#include <core/app.h>
#include <expat.h>
#include <androidfw/resourcetypes.h>   // ResXMLTree: binary AXML pull parser
#include <array>
#include <fstream>
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
    // Binary AXML support (additive; expat path unchanged when isBinary is false).
    bool isBinary = false;
    ResXMLTree* axmlTree = nullptr;
    std::vector<uint8_t> axmlData;
    ~Private(){
        while(eventQueue.size()){
            delete eventQueue.front();
            eventQueue.pop();
        }
        while(eventPool.size()){
            delete eventPool.front();
            eventPool.pop();
        }
        delete axmlTree;
    }
    XmlEvent*acquire(XmlPullParser::EventType type,const std::string&text = std::string()){
        if(eventPool.size()==0) eventPool.push(new XmlEvent());
        auto event  = eventPool.front();
        event->name = text;
        event->type = type;
        event->atts->clear();
        event->text.clear();
        event->lineNumber = XML_GetCurrentLineNumber(parser);
        event->columnNumber=XML_GetCurrentColumnNumber(parser);
        eventPool.pop();
        return event;
    }
    void release(XmlEvent*event){
        eventPool.push(event);
    }
    // Sniff the istream for binary AXML (first byte 0x03 = RES_XML_TYPE). If
    // detected, slurp into axmlData and create a ResXMLTree; otherwise re-wrap
    // the data in a new istringstream for the expat path.
    void detectBinary(std::unique_ptr<std::istream>& strm){
        if(!strm || !*strm) return;
        std::string data((std::istreambuf_iterator<char>(*strm)),
                         std::istreambuf_iterator<char>());
        strm.reset();
        if(data.size() >= 2 && (uint8_t)data[0] == 0x03 && (uint8_t)data[1] == 0x00){
            isBinary = true;
            axmlData.assign(data.begin(), data.end());
            axmlTree = new ResXMLTree();
            axmlTree->setTo(axmlData.data(), axmlData.size());
        } else {
            strm = std::make_unique<std::istringstream>(std::move(data));
        }
    }
    // Drive ResXMLParser to produce one XmlEvent (skip namespace events that
    // CDROID's pull model doesn't use). Returns false at END_DOCUMENT.
    bool feedFromAxml(const std::string& pkg){
        if(!axmlTree) return false;
        while(true){
            ResXMLParser::event_code_t ev = axmlTree->next();
            switch(ev){
                case ResXMLParser::START_TAG:{
                    size_t nl = 0;
                    const char16_t* n16 = axmlTree->getElementName(&nl);
                    auto event = acquire(XmlPullParser::START_TAG, u16toUtf8(n16, nl));
                    event->depth = depth++;
                    event->lineNumber = axmlTree->getLineNumber();
                    const size_t ac = axmlTree->getAttributeCount();
                    for(size_t i = 0; i < ac; i++){
                        size_t anl = 0;
                        const char16_t* an = axmlTree->getAttributeName(i, &anl);
                        size_t avl = 0;
                        const char16_t* av = axmlTree->getAttributeStringValue(i, &avl);
                        std::string attrName = u16toUtf8(an, anl);
                        std::string attrValue = av ? u16toUtf8(av, avl) : renderTypedValue(i);
                        event->atts->insert({attrName, AttributeSet::normalize(pkg, attrValue)});
                    }
                    eventQueue.push(event);
                    return true;
                }
                case ResXMLParser::END_TAG:{
                    size_t nl = 0;
                    const char16_t* n16 = axmlTree->getElementName(&nl);
                    auto event = acquire(XmlPullParser::END_TAG, u16toUtf8(n16, nl));
                    event->depth = --depth;
                    event->lineNumber = axmlTree->getLineNumber();
                    eventQueue.push(event);
                    return true;
                }
                case ResXMLParser::TEXT:{
                    size_t tl = 0;
                    const char16_t* t16 = axmlTree->getText(&tl);
                    if(tl > 0){
                        auto event = acquire(XmlPullParser::TEXT);
                        event->text = u16toUtf8(t16, tl);
                        event->depth = depth;
                        event->lineNumber = axmlTree->getLineNumber();
                        eventQueue.push(event);
                        return true;
                    }
                    break; // empty text — skip
                }
                case ResXMLParser::END_DOCUMENT:
                case ResXMLParser::BAD_DOCUMENT:
                    return false;
                default: // START_DOCUMENT, START/END_NAMESPACE — CDROID skips
                    break;
            }
        }
    }
    // Render a typed Res_value to a string when no rawValue is available.
    std::string renderTypedValue(size_t attrIdx){
        Res_value v;
        if(axmlTree->getAttributeValue(attrIdx, &v) != sizeof(Res_value)) return "";
        char buf[32];
        switch(v.dataType){
            case Res_value::TYPE_STRING:{
                size_t len = 0;
                const char16_t* s = axmlTree->getStrings().stringAt(v.data, &len);
                return s ? u16toUtf8(s, len) : "";
            }
            case Res_value::TYPE_REFERENCE:
            case Res_value::TYPE_ATTRIBUTE:
                snprintf(buf, sizeof(buf), "@0x%08x", v.data);
                return buf;
            case Res_value::TYPE_INT_DEC:
                snprintf(buf, sizeof(buf), "%d", (int)v.data);
                return buf;
            case Res_value::TYPE_INT_HEX:
                snprintf(buf, sizeof(buf), "0x%x", v.data);
                return buf;
            case Res_value::TYPE_INT_BOOLEAN:
                return v.data ? "true" : "false";
            default:
                snprintf(buf, sizeof(buf), "0x%08x", v.data);
                return buf;
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
            event->atts->insert({std::string(key),AttributeSet::normalize(parser->mPackage,std::string(attr))});
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

XmlPullParser::XmlPullParser(){
    mData = new Private;
    mData->depth = 0;
    mData->parser = XML_ParserCreateNS(nullptr,' ');
    XML_SetUserData(mData->parser, this);
    XML_SetElementHandler(mData->parser, AttrParser::startElementHandler, AttrParser::endElementHandler);
    XML_SetCharacterDataHandler(mData->parser, AttrParser::characterDataHandler);
}

XmlPullParser::XmlPullParser(Context*ctx,std::unique_ptr<std::istream>strm):XmlPullParser(){
    mContext = ctx;
    mData->detectBinary(strm);
    mData->stream = std::move(strm);
    auto event = mData->acquire((mData->isBinary||(mData->stream&&mData->stream->good()))?START_DOCUMENT:END_DOCUMENT);
    event->depth= mData->depth++;
    event->lineNumber = 0;
    mAttrs = event->atts;
    mData->eventQueue.push(event);
}

XmlPullParser::XmlPullParser(Context*ctx,const std::string&resid):XmlPullParser(){
    if(ctx){
        mContext = ctx;
        mData->stream = ctx->getInputStream(resid,&mPackage);
    }
    if(((mData->stream==nullptr)||(!*mData->stream))&&resid.size()){
        auto fs = std::make_unique<std::ifstream>(resid);
        if(fs->is_open()){
            mData->stream= std::move(fs);
        }
    }
    mData->detectBinary(mData->stream);
    mData->resourceId = resid;
    auto event = mData->acquire((mData->isBinary||(mData->stream&&mData->stream->good()))?START_DOCUMENT:END_DOCUMENT);
    event->depth= mData->depth++;
    event->lineNumber = 0;
    mAttrs = event->atts;
    mData->eventQueue.push(event);
}

XmlPullParser::operator bool()const{
   if(mData->isBinary) return mData->axmlTree && mData->axmlTree->getError()==0;
   return (mData->stream!=nullptr)&&(*mData->stream);
}

XmlPullParser::~XmlPullParser() {
    XML_ParserFree(mData->parser);
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
        if(mData->isBinary){
            // Binary AXML: drive ResXMLParser to produce events.
            if(!mData->feedFromAxml(mPackage)){
                mData->eventQueue.push(mData->acquire(END_DOCUMENT));
            }
        } else {
            // Existing expat text-XML path (unchanged).
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
    }
    mAttrs = mData->eventQueue.front()->atts;
    return mData->eventQueue.front()->type;
}

std::string XmlPullParser::getPositionDescription()const{
    std::ostringstream oss;
    oss<<XML_GetCurrentLineNumber(mData->parser)<<":"<<XML_GetCurrentColumnNumber(mData->parser);
    return oss.str();
}

}/*endof namespace*/
