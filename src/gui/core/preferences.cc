#include <preferences.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cdtypes.h>
#include <cdlog.h>


namespace cdroid{
Preferences::Preferences(){
   updates=0;
}

Preferences::~Preferences(){
   if(updates&&!pref_file.empty())
       save(pref_file);
}
void Preferences::load(const std::string&fname){
    Json::CharReaderBuilder builder;
    Json::String errs;
    std::ifstream fin(fname);
    bool rc=Json::parseFromStream(builder,fin, &d, &errs);
    if(fin.good()){
        pref_file=fname;
    }
    LOGV("parse=%s",fname.c_str());
}
void Preferences::save(const std::string&fname){
    Json::StreamWriterBuilder builder;
    builder.settings_["indentation"] = "  ";
    std::unique_ptr<Json::StreamWriter>writer(builder.newStreamWriter());
    std::ofstream ofs(fname);
    writer->write(d,&ofs);
    updates=0;
}
int Preferences::getSections(std::vector<std::string>*mbs){
    Json::Value::Members membs=d.getMemberNames();
    if(mbs)*mbs=membs;
    return membs.size();
}
int Preferences::getUpdates()const{
    return updates;
}

bool Preferences::getBool(const std::string&section,const std::string&key,bool def){
    if(!d.isObject()||!d.isMember(section))return def;
    if(!d[section].isMember(key))return def;
    if(!d[section][key].isBool())return def;
    return d[section][key].asBool();
}

int Preferences::getInt(const std::string&section,const std::string&key,int def){
    if(!d.isObject()||!d.isMember(section))return def;
    if(!d[section].isMember(key))return def;
    if(!d[section][key].isInt())return def;
    LOGV("%s.%s=%d",section.c_str(),key.c_str(),d[section][key].asInt());
    return d[section][key].asInt();
}

float Preferences::getFloat(const std::string&section,const std::string&key,float def){
    if(!d.isObject()||!d.isMember(section))return def;
    if(!d[section].isMember(key))return def;
    if(!d[section][key].isDouble())return def;
    return d[section][key].asFloat();
}

double Preferences::getDouble(const std::string&section,const std::string&key,double def){
    if(!d.isObject()||!d.isMember(section))return def;
    if(!d[section].isMember(key))return def;
    if(!d[section][key].isDouble())return def;
    return d[section][key].asDouble();
}

const char* Preferences::getString(const std::string&section,const std::string&key,const char*def){
    if(!d.isObject()||!d.isMember(section))return def;
    if(!d[section].isMember(key))return def;
    if(!d[section][key].isString())return def;
    return d[section][key].asString().data();
}

void Preferences::setValue(const std::string&section,const std::string&key,bool v){
    d[section][key]=v;
    updates++;
}
void Preferences::setValue(const std::string&section,const std::string&key,int v){
    d[section][key]=v;
    LOGD("%s %s %d",section.c_str(),key.c_str(),v);
    updates++;
}
void Preferences::setValue(const std::string&section,const std::string&key,float v){
    d[section][key]=v;
    LOGD("%s %s %d",section.c_str(),key.c_str(),v);
    updates++;
}

void Preferences::setValue(const std::string&section,const std::string&key,const char*v){
    d[section][key]=v;
    LOGD("%s %s %d",section.c_str(),key.c_str(),v);
    updates++;
}
void Preferences::setValue(const std::string&section,const std::string&key,double v){
    d[section][key]=v;
    LOGD("%s %s %d",section.c_str(),key.c_str(),v);
    updates++;
}

}//namespace
