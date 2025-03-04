#include <core/xmlpullparser.h>
#include <core/context.h>
#include <expat.h>
#include <iostream>
#include <fstream>
namespace cdroid{

struct Private{
    XML_Parser parser;
    std::unique_ptr<std::istream>stream;
    std::queue<XmlPullParser::XmlEvent> eventQueue;
    bool endDocument;
};
class XmlPullParser::AttrParser{
public:
    static void startElementHandler(void* userData, const XML_Char* name, const XML_Char** attrs){
        XmlEvent event;
        event.type = START_TAG;
        event.name = name;
        event.attributes.set(attrs);
        XmlPullParser*pull = static_cast<XmlPullParser*>(userData);
        pull->mData->eventQueue.push(event);
    }
    static void endElementHandler(void* userData, const XML_Char* name){
        XmlEvent event;
        event.type = END_TAG;
        event.name = name;
        XmlPullParser*pull = static_cast<XmlPullParser*>(userData);
        pull->mData->eventQueue.push(event);
    }
    static void characterDataHandler(void* userData, const XML_Char* s, int len){
        XmlEvent event;
        event.type = TEXT;
        event.text.assign(s, len);
        XmlPullParser*pull = static_cast<XmlPullParser*>(userData);
        pull->mData->eventQueue.push(event);
    }
};

XmlPullParser::XmlPullParser(){
    mData = new Private;
    mData->endDocument = false;
    mData->parser = XML_ParserCreateNS(NULL,' ');
    XML_SetUserData(mData->parser, this);
    XML_SetElementHandler(mData->parser, AttrParser::startElementHandler, AttrParser::endElementHandler);
    XML_SetCharacterDataHandler(mData->parser, AttrParser::characterDataHandler);
}

XmlPullParser::XmlPullParser(Context*ctx,const std::string&resid):XmlPullParser(){
    setContent(ctx,resid);
}

void XmlPullParser::setContent(Context*ctx,const std::string&resid){
    if(ctx)
        mData->stream = ctx->getInputStream(resid);
    else mData->stream= std::make_unique<std::ifstream>(resid);
}

XmlPullParser::~XmlPullParser() {
    XML_ParserFree(mData->parser);
    delete mData;
}

int XmlPullParser::getDepth()const{
    return (int)mData->eventQueue.size();
}

bool XmlPullParser::readChunk(){
    char buff[128];
    if(mData->stream->read(buff,sizeof(buff))||mData->stream->gcount()>0){
        if(XML_Parse(mData->parser,buff,mData->stream->gcount(),mData->stream->eof())!=XML_STATUS_ERROR){
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
            XmlEvent endEvent;
            endEvent.type=END_DOCUMENT;
            mData->eventQueue.push(endEvent);
        }
    }
    if(!mData->eventQueue.empty()){
        event = mData->eventQueue.front();
        mData->eventQueue.pop();
        return true;
    }
    return false;
}

}/*endof namespace*/
