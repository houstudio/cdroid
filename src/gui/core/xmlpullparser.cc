#include <core/xmlpullparser.h>
#include <core/context.h>
#include <expat.h>
#include <iostream>
#include <fstream>
namespace cdroid{
struct EventData{
    XmlPullParser::XmlEvent event;
    int depth;
    int type;
    int lineNumber;
    EventData(){
        depth = 0;
        lineNumber = 0;
    }
};
struct Private{
    XML_Parser parser;
    int depth;
    int parsedDepth;
    Context*context;
    std::string package;
    std::unique_ptr<std::istream>stream;
    std::queue<std::unique_ptr<EventData>> eventQueue;
    std::string mTagName;
    bool endDocument;
};

class XmlPullParser::AttrParser{
public:
    static void startElementHandler(void* userData, const XML_Char* name, const XML_Char** attrs){
        auto ed=std::make_unique<EventData>();
        XmlEvent& event=ed->event;
        Private*data=(Private*)userData;
        event.name = name;
        event.attributes.setContext(data->context,data->package);
        event.attributes.set(attrs);
        ed->type = START_TAG;
        ed->depth=data->depth;
        data->depth++;
        data->eventQueue.push(std::move(ed));
    }
    static void endElementHandler(void* userData, const XML_Char* name){
        auto ed=std::make_unique<EventData>();
        XmlEvent& event=ed->event;;
        Private*data=(Private*)userData;
        event.name = name;
        ed->type = END_TAG;
        ed->depth= --data->depth;
        data->eventQueue.push(std::move(ed));
    }
    static void characterDataHandler(void* userData, const XML_Char* s, int len){
        auto ed=std::make_unique<EventData>();
        XmlEvent& event=ed->event;
        Private*data=(Private*)userData;
        event.text.assign(s, len);
        ed->type = TEXT;
        ed->depth= data->depth;
        data->eventQueue.push(std::move(ed));
    }
};

XmlPullParser::XmlPullParser(){
    mData = new Private;
    mData->depth =0;
    mData->parsedDepth=0;
    mData->context = nullptr;
    mData->endDocument = false;
    mData->parser = XML_ParserCreateNS(NULL,' ');
    XML_SetUserData(mData->parser, mData);
    XML_SetElementHandler(mData->parser, AttrParser::startElementHandler, AttrParser::endElementHandler);
    XML_SetCharacterDataHandler(mData->parser, AttrParser::characterDataHandler);
}

XmlPullParser::XmlPullParser(const std::string&content):XmlPullParser(){
    setContent(content);
}

XmlPullParser::XmlPullParser(Context*ctx,const std::string&resid):XmlPullParser(){
    setContent(ctx,resid);
}

void XmlPullParser::setContent(Context*ctx,const std::string&resid){
    if(ctx){
        mData->stream = ctx->getInputStream(resid,&mData->package);
    }
    mData->context = ctx;
    if(((mData->stream==nullptr)||(!*mData->stream))&&resid.size()){
        mData->stream = std::make_unique<std::ifstream>(resid);
    }
}

void XmlPullParser::setContent(const std::string&content){
    mData->stream = std::make_unique<std::istringstream>(content);
}

XmlPullParser::operator bool()const{
   return (mData->stream!=nullptr)&&(*mData->stream);
}

XmlPullParser::~XmlPullParser() {
    XML_ParserFree(mData->parser);
    delete mData;
}

int XmlPullParser::getDepth()const{
    //return mData->eventQueue.empty()?0:mData->eventQueue.front()->depth;
    return mData->parsedDepth;
}

std::string XmlPullParser::getName()const{
    return mData->mTagName;
}

bool XmlPullParser::readChunk(){
    char buff[128];
    std::streamsize len;
    mData->stream->read(buff,sizeof(buff));
    len = mData->stream->gcount();
    if(len>0){
        const bool isEnd = mData->stream->eof();
        if(XML_Parse(mData->parser,buff,len,isEnd)==XML_STATUS_ERROR){
            throw std::runtime_error("XML parsing error: " + std::string(XML_ErrorString(XML_GetErrorCode(mData->parser))));
        }
        return true;
    }
    return false;
}

int XmlPullParser::next(XmlEvent& event){
    int depth;
    return next(event,depth);
}

int XmlPullParser::next(XmlEvent& event,int &depth) {
    while(mData->eventQueue.empty() && !mData->endDocument){
        if(!readChunk()){
            mData->endDocument = true;
            auto ed = std::make_unique<EventData>();
            XmlEvent& endEvent = ed->event;
            endEvent.type = END_DOCUMENT;
            mData->eventQueue.push(std::move(ed));
        }
    }
    if(!mData->eventQueue.empty()){
        auto front =mData->eventQueue.front().get();
        event = front->event;
        const int type= front->type;
        mData->mTagName = event.name;
        depth = front->depth;
        if(type==START_TAG)mData->parsedDepth++;
        else if(type==END_TAG)mData->parsedDepth--;
        mData->eventQueue.pop();
        return type;
    }
    mData->parsedDepth =0;
    return END_DOCUMENT;
}

std::string XmlPullParser::getPositionDescription()const{
    std::ostringstream oss;
    oss<<XML_GetCurrentLineNumber(mData->parser)<<":"<<XML_GetCurrentColumnNumber(mData->parser);
    return oss.str();
}

}/*endof namespace*/
