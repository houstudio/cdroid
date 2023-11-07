#include <preferences.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cdtypes.h>
#include <cdlog.h>
#include <expat.h>
#include <string.h>
#include <textutils.h>
#include <core/attributeset.h>
#include <core/iostreams.h>

namespace cdroid{

Preferences::Preferences(){
   updates=0;
}

Preferences::~Preferences(){
   if(updates&&!mFileName.empty())
       save(mFileName);
}
typedef struct{
   Preferences*pref;
   std::string section;
   std::string key;
   std::string value;
}PREFPARSER;

static void startElement(void *userData, const XML_Char *name, const XML_Char **satts){
    PREFPARSER*kvp=(PREFPARSER*)userData;
    if(strcmp(name,"section")==0){//root node is not in KVPARSER::attrs
        AttributeSet atts;
        atts.set(satts);
        kvp->section=atts.getString("name");
    }else if(strcmp(name,"item")==0){
        AttributeSet atts;
        atts.set(satts);
	kvp->key=atts.getString("name");
        kvp->value=std::string();
    }
}

static void CharacterHandler(void *userData,const XML_Char *s, int len){
    PREFPARSER*kvp=(PREFPARSER*)userData;
    kvp->value+=std::string(s,len);
}

static void endElement(void *userData, const XML_Char *name){
    PREFPARSER*kvp=(PREFPARSER*)userData;
    if(strcmp(name,"item")==0){//root node is not in KVPARSER::attrs
        TextUtils::trim(kvp->value);
	kvp->pref->setValue(kvp->section,kvp->key,kvp->value);
    }
}

void Preferences::load(const std::string&fname){
    std::ifstream fin(fname);
    if(fin.good()){
        mFileName=fname;
        load(fin);
    }else{
        load(fname.c_str(),fname.length());
    }
}

void Preferences::load(const char*buf,size_t len){
    MemoryInputStream stream(buf,len);
    load(stream);
}

void Preferences::load(std::istream&istream){
    XML_Parser parser=XML_ParserCreate(nullptr);
    std::string curKey;
    std::string curValue;
    int len=0;
    PREFPARSER kvp;
    kvp.pref=this;
    XML_SetUserData(parser,&kvp);
    XML_SetElementHandler(parser, startElement, endElement);
    XML_SetCharacterDataHandler(parser,CharacterHandler);
    if(!istream.good())return;
    do {
        std::string str;
        std::getline(istream,str);
        len=str.length();
        if (XML_Parse(parser, str.c_str(),len,len==0) == XML_STATUS_ERROR) {
            const char*es=XML_ErrorString(XML_GetErrorCode(parser));
            LOGE("%s at line %ld",es, XML_GetCurrentLineNumber(parser));
            XML_ParserFree(parser);
            return;
        }
    } while(len!=0);
    XML_ParserFree(parser);
    updates = 0; 
}

void Preferences::save(const std::string&fname){
    std::ofstream ofs(fname);
    save(ofs);
}

void Preferences::save(std::ostream&os){
    os<<"<sections>"<<std::endl;
    for(auto sec:mPrefs){
	    os<<"<section name=\""<<sec.first<<"\">"<<std::endl;
	    for(auto kv:sec.second){
	        os<<"<item name=\""<<kv.first<<"\">"<<kv.second<<"</item>"<<std::endl;
	    }
	    os<<"</section>"<<std::endl;
    }
    os<<"</sections>"<<std::endl;
    updates=0;
}

int Preferences::getSectionCount()const{
    return mPrefs.size();
}

int Preferences::getSections(std::vector<std::string>&mbs){
    mbs.clear();
    for(auto s:mPrefs)
	mbs.push_back(s.first);
    return mbs.size();
}

void Preferences::removeSection(const std::string&section){
    auto sec = mPrefs.find(section);
    if(sec!=mPrefs.end()){
        mPrefs.erase(sec);
        updates++;
    }
}

bool Preferences::hasSection(const std::string&section)const{
    return mPrefs.find(section) != mPrefs.end();
}

int Preferences::getUpdates()const{
    return updates;
}

bool Preferences::getBool(const std::string&section,const std::string&key,bool def){
    auto sec=mPrefs.find(section);
    if(sec==mPrefs.end())return def;
    auto kv=sec->second.find(key);
    if(kv==sec->second.end())return def;
    std::string s=kv->second;
    LOGV("%s:%s=%s",section.c_str(),key.c_str(),s.c_str());
    return (s[0]=='t')||(s[0]=='T');
}

int Preferences::getInt(const std::string&section,const std::string&key,int def){
    auto sec=mPrefs.find(section);
    if(sec==mPrefs.end())return def;
    auto kv=sec->second.find(key);
    if(kv==sec->second.end())return def;
    std::stringstream ss(kv->second);
    ss>>def;
    return def;
}

float Preferences::getFloat(const std::string&section,const std::string&key,float def){
    auto sec=mPrefs.find(section);
    if(sec==mPrefs.end())return def;
    auto kv=sec->second.find(key);
    if(kv==sec->second.end())return def;
    std::stringstream ss(kv->second);
    ss>>def;
    return def;
}

double Preferences::getDouble(const std::string&section,const std::string&key,double def){
    auto sec=mPrefs.find(section);
    if(sec==mPrefs.end())return def;
    auto kv=sec->second.find(key);
    if(kv==sec->second.end())return def;
    std::stringstream ss(kv->second);
    ss>>def;
    return def;
}

std::string Preferences::getString(const std::string&section,const std::string&key,const std::string&def){
    auto sec=mPrefs.find(section);
    if(sec==mPrefs.end())return def;
    auto kv=sec->second.find(key);
    if(kv==sec->second.end())return def;
    LOGV("%s:%s=%s",section.c_str(),key.c_str(),kv->second.c_str());
    return kv->second;
}

void Preferences::setValue(const std::string&section,const std::string&key,bool v){
    auto sec=mPrefs.find(section);
    if(sec==mPrefs.end()){
	std::map<std::string,std::string>ss;
	sec=mPrefs.insert({section,std::map<std::string,std::string>()}).first;
    }
    auto kv=sec->second.find(key);
    if(kv==sec->second.end())
	sec->second.insert({key,(v?"true":"false")});
    else kv->second=(v?"true":"false");
    updates++;
}

void Preferences::setValue(const std::string&section,const std::string&key,int v){
    auto sec=mPrefs.find(section);
    if(sec==mPrefs.end()){
	std::string vs=std::to_string(v);
	sec=mPrefs.insert({section,std::map<std::string,std::string>()}).first;
    }
    auto kv=sec->second.find(key);
    if(kv==sec->second.end())
	sec->second.insert({key,std::to_string(v)});
    else kv->second=std::to_string(v);
    LOGV("%s %s %d",section.c_str(),key.c_str(),v);
    updates++;
}

void Preferences::setValue(const std::string&section,const std::string&key,float v){
    auto sec=mPrefs.find(section);
    if(sec==mPrefs.end())
	sec=mPrefs.insert({section,std::map<std::string,std::string>()}).first;
    auto kv=sec->second.find(key);
    if(kv==sec->second.end())
	sec->second.insert({key,std::to_string(v)});
    else kv->second=std::to_string(v);
    LOGV("%s %s %f",section.c_str(),key.c_str(),v);
    updates++;
}

void Preferences::setValue(const std::string&section,const std::string&key,const std::string&v){
    auto sec=mPrefs.find(section);
    if(sec==mPrefs.end())
	sec=mPrefs.insert({section,std::map<std::string,std::string>()}).first;
    auto kv=sec->second.find(key);
    if(kv==sec->second.end())
	sec->second.insert({key,v});
    else kv->second=v;
    LOGV("%s %s %s",section.c_str(),key.c_str(),v.c_str());
    updates++;
}

void Preferences::setValue(const std::string&section,const std::string&key,double v){
    auto sec=mPrefs.find(section);
    if(sec==mPrefs.end())
	sec=mPrefs.insert({section,std::map<std::string,std::string>()}).first;
    auto kv=sec->second.find(key);
    if(kv==sec->second.end())
	sec->second.insert({key,std::to_string(v)});
    else kv->second=std::to_string(v);
    LOGV("%s %s %f",section.c_str(),key.c_str(),v);
    updates++;
}

}//namespace
