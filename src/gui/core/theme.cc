#include <theme.h>
#include <expat.h>
#include <cdtypes.h>
#include <cdlog.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string.h>

namespace cdroid{

Theme*Theme::mInst=nullptr;
Theme&Theme::getInstance(){
    if(mInst==nullptr)
       mInst=new Theme();
    return *mInst;
}

typedef struct StyleData{
    std::map<const std::string,AttributeSet>*maps;
    AttributeSet*cur;
    std::string key;
    std::string value;
}STYLEDATA;

static void startElement(void *userData, const XML_Char *name, const XML_Char **satts){
    AttributeSet atts(satts);
    STYLEDATA*sd=(STYLEDATA*)userData;
    std::map<const std::string,AttributeSet>*maps=sd->maps;

    if(0==strcmp(name,"style")){
        const std::string stylename=atts.getString("name");
        const std::string parent=atts.getString("parent");
        auto it=maps->find(stylename);
        LOGD("<%s> parent=%s",stylename.c_str(),parent.c_str());
        if(it!=maps->end()){
            sd->cur=&it->second;
        }else{
            auto it2=maps->insert(std::make_pair<const std::string,AttributeSet>(stylename.c_str(),AttributeSet()));
            sd->cur=&(it2.first->second);
            if(!parent.empty())sd->cur->add("parent",parent);
        }
    }else if(0==strcmp(name,"item")){
        sd->key  = atts.getString("name");
        sd->value=std::string();
    }
}

static void CharacterHandler(void *userData,const XML_Char *s, int len){
    STYLEDATA*sd=(STYLEDATA*)userData;
    sd->value+=std::string(s,len);
}

static void endElement(void *userData, const XML_Char *name){
    STYLEDATA*sd=(STYLEDATA*)userData;
    std::map<const std::string,AttributeSet>*maps=sd->maps;
    if(0==strcmp(name,"style")){
       sd->cur=nullptr;
    }else if(0==strcmp(name,"item")){
       LOGV("\t%s=%s",sd->key.c_str(),sd->value.c_str());
       sd->cur->add(sd->key,sd->value);
    }
}

int Theme::loadStyles(std::istream&stream){
    int len;
    char buf[256];
    XML_Parser parser=XML_ParserCreate(nullptr);
    std::string curKey;
    std::string curValue;
    STYLEDATA sd={&mStyles,nullptr};
    XML_SetUserData(parser,&sd);
    XML_SetElementHandler(parser, startElement, endElement);
    XML_SetCharacterDataHandler(parser,CharacterHandler);
    do {
        stream.read(buf,sizeof(buf));
        len=stream.gcount();
        if (XML_Parse(parser, buf,len,len==0) == XML_STATUS_ERROR) {
            const char*es=XML_ErrorString(XML_GetErrorCode(parser));
            LOGE("%s at line %ld",es, XML_GetCurrentLineNumber(parser));
            XML_ParserFree(parser);
            return 0;
        }
    } while(len!=0);
    XML_ParserFree(parser);
}

const AttributeSet Theme::getStyle(const std::string&name)const{
    auto it=mStyles.find(name);
    if(it!=mStyles.end())
       return it->second;
    return AttributeSet();
}

}//namespace
