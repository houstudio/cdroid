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
#include <array>
#include <fstream>
#include <iostream>
#include <sstream>
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
        event->columnNumber=XML_GetCurrentColumnNumber(parser);
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
        auto event = data->acquire(START_TAG,name);
        data->mText.clear();
        event->depth = data->depth++;
        for(int i = 0;attrs[i];i+=2){
            const char* key = strrchr(attrs[i],' ');
            if(key) key++;
            else key = attrs[i];
            event->atts->insert({std::string(key),AttributeSet::normalize(parser->mPackage,std::string(attrs[i+1]))});
        }
        data->eventQueue.push(event);
    }
    static void endElementHandler(void* userData, const XML_Char* name){
        Private*data = static_cast<XmlPullParser*>(userData)->mData;
        auto event = data->acquire(END_TAG,name);
        const int depth = --data->depth;
        event->depth= depth;
        data->eventQueue.push(event);
    }
    static void characterDataHandler(void* userData, const XML_Char* s, int len){
        Private*data = static_cast<XmlPullParser*>(userData)->mData;
        auto event = data->acquire(TEXT,"");
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
    mData->stream = std::move(strm);
    auto event = mData->acquire(mData->stream->good()?START_DOCUMENT:END_DOCUMENT);
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
    mData->resourceId = resid;
    auto event = mData->acquire(mData->stream?START_DOCUMENT:END_DOCUMENT);
    event->depth= mData->depth++;
    event->lineNumber = 0;
    mAttrs = event->atts;
    mData->eventQueue.push(event);
}

XmlPullParser::operator bool()const{
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

}/*endof namespace*/
