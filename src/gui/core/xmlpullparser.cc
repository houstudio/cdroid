#include <core/xmlpullparser.h>
#include <core/context.h>
#include <core/app.h>
#include <expat.h>
#include <iostream>
#include <fstream>
namespace cdroid{
struct XmlEvent {
    XmlPullParser::EventType type;
    int depth;
    int lineNumber;
    std::string name;
    std::string text;
    std::shared_ptr<std::unordered_map<std::string,std::string>>atts;
    XmlEvent(){
        atts = std::make_shared<std::unordered_map<std::string,std::string>>();
    }
    XmlEvent(XmlPullParser::EventType tp):XmlEvent(){type =tp;depth=-1;}
    XmlEvent(XmlPullParser::EventType tp,const std::string&name_):XmlEvent(tp){
       name= name_;
    }
};

struct Private{
    XML_Parser parser;
    int depth;
    std::string resourceId;
    std::string mText;
    std::array<char,1024>buffer;
    std::unique_ptr<std::istream>stream;
    std::queue<std::unique_ptr<XmlEvent>> eventQueue;
    std::string mTagName;
    bool endDocument;
};

class XmlPullParser::AttrParser{
public:
    static void startElementHandler(void* userData, const XML_Char* name, const XML_Char** attrs){
        auto event = std::make_unique<XmlEvent>(START_TAG,name);
        XmlPullParser*parser = (XmlPullParser*)userData;
        Private*data = parser->mData;
        data->mText.clear();
        event->depth = data->depth++;
        event->lineNumber = XML_GetCurrentLineNumber(data->parser);
        for(int i = 0;attrs[i];i+=2){
            const char* key = strrchr(attrs[i],' ');
            if(key) key++;
            else key = attrs[i];
            event->atts->insert({std::string(key),AttributeSet::normalize(parser->mPackage,std::string(attrs[i+1]))});
        }

        data->eventQueue.push(std::move(event));
    }
    static void endElementHandler(void* userData, const XML_Char* name){
        auto event = std::make_unique<XmlEvent>(END_TAG,name);
        Private*data =((XmlPullParser*)userData)->mData;
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
        data->eventQueue.push(std::move(event));
    }
    static void characterDataHandler(void* userData, const XML_Char* s, int len){
        auto event = std::make_unique<XmlEvent>(TEXT);
        Private*data = ((XmlPullParser*)userData)->mData;
        data->mText.append(s,len);
    }
};

XmlPullParser::XmlPullParser(){
    mData = new Private;
    mData->depth = 0;
    mData->endDocument = false;
    mData->parser = XML_ParserCreateNS(NULL,' ');
    XML_SetUserData(mData->parser, this);
    XML_SetElementHandler(mData->parser, AttrParser::startElementHandler, AttrParser::endElementHandler);
    XML_SetCharacterDataHandler(mData->parser, AttrParser::characterDataHandler);
}

XmlPullParser::XmlPullParser(const std::string&content):XmlPullParser(){
    mData->stream = std::make_unique<std::istringstream>(content);
    auto event = std::make_unique<XmlEvent>(START_DOCUMENT);
    mContext = &App::getInstance();
    event->depth= mData->depth++;
    event->lineNumber = 0;
    mAttrs = event->atts;
    mData->eventQueue.push(std::move(event));}

XmlPullParser::XmlPullParser(Context*ctx,std::unique_ptr<std::istream>strm):XmlPullParser(){
    mContext = ctx;
    mData->stream = std::move(strm);
    auto event = std::make_unique<XmlEvent>(mData->stream->good()?START_DOCUMENT:END_DOCUMENT);
    event->depth= mData->depth++;
    event->lineNumber = 0;
    mAttrs = event->atts;
    mData->eventQueue.push(std::move(event));
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
    auto event = std::make_unique<XmlEvent>(mData->stream?START_DOCUMENT:END_DOCUMENT);
    event->depth= mData->depth++;
    event->lineNumber = 0;
    mAttrs = event->atts;
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
    mData->eventQueue.pop();
    while(mData->eventQueue.empty()){
        std::streamsize len;
        mData->stream->read(mData->buffer.data(),mData->buffer.size());
        len = mData->stream->gcount();
        const bool done = mData->stream->eof();
        if(XML_Parse(mData->parser,mData->buffer.data(),len,done)==XML_STATUS_ERROR){
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
    mAttrs = mData->eventQueue.front()->atts;
    return mData->eventQueue.front()->type;
}

std::string XmlPullParser::getPositionDescription()const{
    std::ostringstream oss;
    oss<<XML_GetCurrentLineNumber(mData->parser)<<":"<<XML_GetCurrentColumnNumber(mData->parser);
    return oss.str();
}

}/*endof namespace*/
