#include<theme.h>
#include<expat.h>
#include<cdtypes.h>
#include<cdlog.h>
#include<iostream>
#include <iomanip>
#include <sstream>
#include <string.h>


namespace cdroid{
static void startElement(void *userData, const XML_Char *name, const XML_Char **atts){
}
static void endElement(void *userData, const XML_Char *name){
}

Theme*Theme::mInst=nullptr;
Theme&Theme::getInstance(){
    if(mInst==nullptr)
       mInst=new Theme();
    return *mInst;
}

int Theme::parseStyles(std::istream&stream){
    int done=0;
    char buf[256];

    XML_Parser parser = XML_ParserCreate(NULL);
    XML_SetUserData(parser,nullptr);
    XML_SetElementHandler(parser, startElement, endElement);
    //XML_SetCharacterDataHandler(parser,dataHandler);

    do {
       int len=stream.readsome(buf,sizeof(buf));
       done=(len==0);
       if (XML_Parse(parser, buf,len,done) == XML_STATUS_ERROR) {
           const char*es=XML_ErrorString(XML_GetErrorCode(parser));
           LOGE("%s at line %ld",es, XML_GetCurrentLineNumber(parser));
           return 1;
       }
    } while(!done);
    XML_ParserFree(parser);
    return 0;
}

const AttributeSet Theme::getStyle(const std::string&name)const{
    return AttributeSet();
}

}//namespace
