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
    if(strcmp(name,"section")){//root node is not in KVPARSER::attrs
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
    if(strcmp(name,"item")){//root node is not in KVPARSER::attrs
        TextUtils::trim(kvp->value);
	kvp->pref->setValue(kvp->section,kvp->key,kvp->value);
    }
}

void Preferences::load(const std::string&fname){
    std::ifstream fin(fname);
    if(fin.good()){
        mFileName=fname;
    }
    XML_Parser parser=XML_ParserCreate(nullptr);
    std::string curKey;
    std::string curValue;
    char buf[128];
    int len=0;
    PREFPARSER kvp;
    kvp.pref=this;
    XML_SetUserData(parser,&kvp);
    XML_SetElementHandler(parser, startElement, endElement);
    XML_SetCharacterDataHandler(parser,CharacterHandler);
    do {
        fin.read(buf,sizeof(buf));
        len=fin.gcount();
        if (XML_Parse(parser, buf,len,len==0) == XML_STATUS_ERROR) {
            const char*es=XML_ErrorString(XML_GetErrorCode(parser));
            LOGE("%s:%s at line %ld",fname.c_str(),es, XML_GetCurrentLineNumber(parser));
            XML_ParserFree(parser);
            return;
        }
    } while(len!=0);
    XML_ParserFree(parser);
    
    LOGV("parse=%s",fname.c_str());
}

void Preferences::save(const std::string&fname){
    std::ofstream ofs(fname);
    for(auto sec:mPrefs){
	ofs<<"<section name=\""<<sec.first<<"\">"<<std::endl;
	for(auto kv:sec.second){
	   ofs<<"<item name=\""<<kv.first<<"\">"<<kv.second<<"</item>"<<std::endl;
	}
	ofs<<"</section>"<<std::endl;
    }
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

int Preferences::getUpdates()const{
    return updates;
}

bool Preferences::getBool(const std::string&section,const std::string&key,bool def){
    auto sec=mPrefs.find(section);
    if(sec==mPrefs.end())return def;
    auto kv=sec->second.find(key);
    if(kv==sec->second.end())return def;
    std::stringstream ss(kv->second); 
    ss>>def;
    return def;
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

std::string Preferences::getString(const std::string&section,const std::string&key,const char*def){
    auto sec=mPrefs.find(section);
    if(sec==mPrefs.end())return def;
    auto kv=sec->second.find(key);
    if(kv==sec->second.end())return std::string(def);
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
	sec->second.insert({key,std::to_string(v)});
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
    LOGD("%s %s %d",section.c_str(),key.c_str(),v);
    updates++;
}

void Preferences::setValue(const std::string&section,const std::string&key,float v){
    auto sec=mPrefs.find(section);
    if(sec==mPrefs.end())
	sec=mPrefs.insert({section,std::map<std::string,std::string>()}).first;
    auto kv=sec->second.find(key);
    if(kv==sec->second.end())
	sec->second.insert({key,std::to_string(v)});
    LOGD("%s %s %d",section.c_str(),key.c_str(),v);
    updates++;
}

void Preferences::setValue(const std::string&section,const std::string&key,const std::string&v){
    auto sec=mPrefs.find(section);
    if(sec==mPrefs.end())
	sec=mPrefs.insert({section,std::map<std::string,std::string>()}).first;
    auto kv=sec->second.find(key);
    if(kv==sec->second.end())
	sec->second.insert({key,v});
    LOGD("%s %s %d",section.c_str(),key.c_str(),v);
    updates++;
}

void Preferences::setValue(const std::string&section,const std::string&key,double v){
    auto sec=mPrefs.find(section);
    if(sec==mPrefs.end())
	sec=mPrefs.insert({section,std::map<std::string,std::string>()}).first;
    auto kv=sec->second.find(key);
    if(kv==sec->second.end())
	sec->second.insert({key,std::to_string(v)});
    LOGD("%s %s %d",section.c_str(),key.c_str(),v);
    updates++;
}

}//namespace
