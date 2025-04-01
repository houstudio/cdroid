#include <core/xmlpullparser.h>
#include <core/context.h>
#include <expat.h>
#include <iostream>
#include <fstream>
namespace cdroid{
struct Private{
    XML_Parser parser;
    Context*context;
    int depth;
    std::string package;
    std::string resourceId;
    std::string mText;
    std::unique_ptr<std::istream>stream;
    std::queue<std::unique_ptr<XmlPullParser::XmlEvent>> eventQueue;
    std::unordered_map<std::string,std::string>attributes;
    std::string mTagName;
    bool endDocument;
};

XmlPullParser::XmlEvent::XmlEvent(){}

XmlPullParser::XmlEvent::XmlEvent(EventType tp):type(tp),depth(-1){
}

XmlPullParser::XmlEvent::XmlEvent(EventType tp,const std::string&name_):XmlEvent(tp){
    name = name_;
}

class XmlPullParser::AttrParser{
public:
    static void startElementHandler(void* userData, const XML_Char* name, const XML_Char** attrs){
        auto event = std::make_unique<XmlEvent>(START_TAG,name);
        Private*data =(Private*)userData;
        event->depth = data->depth++;
        event->lineNumber = XML_GetCurrentLineNumber(data->parser);
        event->attributes.setContext(data->context,data->package);
        event->attributes.set(attrs);
        data->attributes.clear();
        for(int i = 0;attrs[i];i+=2){
            const char* key = strrchr(attrs[i],' ');
            if(key) key++;
            else key = attrs[i];
            data->attributes.insert({std::string(key),AttributeSet::normalize(data->package,std::string(attrs[i+1]))});
        }

        data->eventQueue.push(std::move(event));
    }
    static void endElementHandler(void* userData, const XML_Char* name){
        auto event = std::make_unique<XmlEvent>(END_TAG,name);
        Private*data =(Private*)userData;
        const int depth = --data->depth;
        if(!data->mText.empty()){
            auto te = std::make_unique<XmlEvent>(TEXT,name);
            te->lineNumber = XML_GetCurrentLineNumber(data->parser);
            te->text = data->mText;
            te->depth= depth;
            data->eventQueue.push(std::move(te));
        }
        event->depth= depth;
        event->text = data->mText;
        event->lineNumber = XML_GetCurrentLineNumber(data->parser);
        data->mText.clear();
        for(auto a:data->attributes)event->attributes.add(a.first,a.second);
        data->eventQueue.push(std::move(event));
    }
    static void characterDataHandler(void* userData, const XML_Char* s, int len){
        auto event = std::make_unique<XmlEvent>(TEXT);
        Private*data = (Private*)userData;
        data->mText.append(s,len);
        event->text.assign(s,len);
        event->depth = data->depth;
    }
};

XmlPullParser::XmlPullParser(){
    mData = new Private;
    mData->depth = 0;
    mData->context = nullptr;
    mData->endDocument = false;
    mData->parser = XML_ParserCreateNS(NULL,' ');
    XML_SetUserData(mData->parser, mData);
    XML_SetElementHandler(mData->parser, AttrParser::startElementHandler, AttrParser::endElementHandler);
    XML_SetCharacterDataHandler(mData->parser, AttrParser::characterDataHandler);
}

XmlPullParser::XmlPullParser(const std::string&content):XmlPullParser(){
    mData->stream = std::make_unique<std::istringstream>(content);
}

XmlPullParser::XmlPullParser(Context*ctx,std::unique_ptr<std::istream>strm):XmlPullParser(){
    mData->context= ctx;
    mData->stream = std::move(strm);
    auto event = std::make_unique<XmlEvent>(mData->stream->good()?START_DOCUMENT:END_DOCUMENT);
    event->depth= mData->depth++;
    event->lineNumber = 0;
    mData->eventQueue.push(std::move(event));
}

XmlPullParser::XmlPullParser(Context*ctx,const std::string&resid):XmlPullParser(){
    if(ctx){
        mData->stream = ctx->getInputStream(resid,&mData->package);
    }
    mData->context = ctx;
    if(((mData->stream==nullptr)||(!*mData->stream))&&resid.size()){
        auto fs = std::make_unique<std::ifstream>(resid);
        if(fs->is_open()){
            mData->stream= std::move(fs);
        }
    }
    mData->resourceId = resid;
    auto event = std::make_unique<XmlEvent>(mData->stream?START_DOCUMENT:END_DOCUMENT);
    event->depth= mData->depth++;
    event->lineNumber = 0;
    mData->eventQueue.push(std::move(event));
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
    return 0;
}

const AttributeSet& XmlPullParser::asAttributeSet()const{
    return mData->eventQueue.front()->attributes;
}

std::string XmlPullParser::getName()const{
    return mData->eventQueue.front()->name;
}

int XmlPullParser::next(XmlEvent& event) {
    const EventType currentEvent = mData->eventQueue.front()->type;
    if((currentEvent==BAD_DOCUMENT)||(currentEvent==END_DOCUMENT)){
        event.type = currentEvent;
        event.name.clear();
        return currentEvent;
    }
    mData->eventQueue.pop();
    while(mData->eventQueue.empty()){
        char buff[128];
        std::streamsize len;
        mData->stream->read(buff,sizeof(buff));
        len = mData->stream->gcount();
        const bool done = mData->stream->eof();
        if(XML_Parse(mData->parser,buff,len,done)==XML_STATUS_ERROR){
            const XML_Error xmlError = XML_GetErrorCode(mData->parser);
            const char*errMsg=XML_ErrorString(xmlError);
            mData->endDocument = true;
            LOGE("%d:%s %s:%s",xmlError,errMsg,mData->resourceId.c_str(),getPositionDescription().c_str());
            mData->eventQueue.push(std::make_unique<XmlEvent>(BAD_DOCUMENT));
            break;
        }
        if(done){
            mData->eventQueue.push(std::make_unique<XmlEvent>(END_DOCUMENT));
        }
    }
    event =*mData->eventQueue.front();
    return mData->eventQueue.front()->type;
}

std::string XmlPullParser::getPositionDescription()const{
    std::ostringstream oss;
    oss<<XML_GetCurrentLineNumber(mData->parser)<<":"<<XML_GetCurrentColumnNumber(mData->parser);
    return oss.str();
}

}/*endof namespace*/
