#include <core/xmlpullparser.h>
#include <core/context.h>
#include <expat.h>
#include <iostream>
#include <fstream>
namespace cdroid{
struct EventData{
    XmlPullParser::XmlEvent event;
    int depth;
    int lineNumber;
    EventData(){
        depth=0;
        lineNumber=0;
    }
};
struct Private{
    XML_Parser parser;
    int depth;
    std::unique_ptr<std::istream>stream;
    std::queue<std::unique_ptr<EventData>> eventQueue;
    bool endDocument;
};
class XmlPullParser::AttrParser{
public:
    static void startElementHandler(void* userData, const XML_Char* name, const XML_Char** attrs){
        auto ed=std::make_unique<EventData>();
        XmlEvent& event=ed->event;
        Private*data=(Private*)userData;
        event.type = START_TAG;
        event.name = name;
        event.attributes.set(attrs);
        ed->depth=data->depth;
        data->depth++;
        data->eventQueue.push(std::move(ed));
    }
    static void endElementHandler(void* userData, const XML_Char* name){
        auto ed=std::make_unique<EventData>();
        XmlEvent& event=ed->event;;
        Private*data=(Private*)userData;
        event.type = END_TAG;
        event.name = name;
        ed->depth= --data->depth;
        data->eventQueue.push(std::move(ed));
    }
    static void characterDataHandler(void* userData, const XML_Char* s, int len){
        auto ed=std::make_unique<EventData>();
        XmlEvent& event=ed->event;
        Private*data=(Private*)userData;
        event.type = TEXT;
        event.text.assign(s, len);
        ed->depth= data->depth;
        data->eventQueue.push(std::move(ed));
    }
};

XmlPullParser::XmlPullParser(){
    mData = new Private;
    mData->endDocument = false;
    mData->parser = XML_ParserCreateNS(NULL,' ');
    XML_SetUserData(mData->parser, mData);
    XML_SetElementHandler(mData->parser, AttrParser::startElementHandler, AttrParser::endElementHandler);
    XML_SetCharacterDataHandler(mData->parser, AttrParser::characterDataHandler);
}

XmlPullParser::XmlPullParser(Context*ctx,const std::string&resid):XmlPullParser(){
    setContent(ctx,resid);
}

void XmlPullParser::setContent(Context*ctx,const std::string&resid){
    mData->stream = ctx? ctx->getInputStream(resid)
        :std::make_unique<std::ifstream>(resid);
}

XmlPullParser::~XmlPullParser() {
    XML_ParserFree(mData->parser);
    delete mData;
}

int XmlPullParser::getDepth()const{
    return mData->eventQueue.empty()?0:mData->eventQueue.front()->depth;
}

bool XmlPullParser::readChunk(){
    char buff[128];
    std::streamsize len;
    mData->stream->read(buff,sizeof(buff));
    len = mData->stream->gcount();
    if(len>0){
        const bool isEnd=mData->stream->eof();
        if(XML_Parse(mData->parser,buff,len,isEnd)==XML_STATUS_ERROR){
            throw std::runtime_error("XML parsing error: " + std::string(XML_ErrorString(XML_GetErrorCode(mData->parser))));
        }
        return true;
    }
    return false;
}

bool XmlPullParser::next(XmlEvent& event) {
    while(mData->eventQueue.empty() && !mData->endDocument){
        if(!readChunk()){
            mData->endDocument = true;
            auto ed=std::make_unique<EventData>();
            XmlEvent& endEvent=ed->event;
            endEvent.type=END_DOCUMENT;
            mData->eventQueue.push(std::move(ed));
        }
    }
    if(!mData->eventQueue.empty()){
        event = mData->eventQueue.front()->event;
        mData->eventQueue.pop();
        return true;
    }
    return false;
}

}/*endof namespace*/
