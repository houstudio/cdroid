/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
#include <iostream>
#include <sstream>
#include <fstream>
#include <core/preferences.h>
#include <porting/cdtypes.h>
#include <porting/cdlog.h>
#include <utils/textutils.h>
#include <core/attributeset.h>
#include <core/iostreams.h>
#include <core/xmlpullparser.h>

namespace cdroid{

Preferences::Preferences(){
   updates=0;
}

Preferences::~Preferences(){
   if(updates&&!mFileName.empty())
       save(mFileName);
}

void Preferences::load(const std::string&fname){
    std::ifstream fin(fname);
    if(fname.find("<")==std::string::npos){
        /*< cant used in filename,but must be used in xml content*/
        if(fin.is_open())load(fin);
    }else{
        std::istringstream istm(fname);
        load(istm);
    }
}

void Preferences::load(const char*buf,size_t len){
    if(buf&&len){
        std::istringstream sin(std::string(buf,len));
        load(sin);
    }
}

void Preferences::load(std::istream&istream){
    auto strm = std::make_unique<std::istream>(istream.rdbuf());
    XmlPullParser parser(nullptr,std::move(strm));
    const AttributeSet& attrs = parser;
    int type;
    std::string section,key,value;
    istream.rdbuf(nullptr);
    while(((type=parser.next())!=XmlPullParser::END_DOCUMENT)&&(type!=XmlPullParser::BAD_DOCUMENT)){
        std::string tagName = parser.getName();
        switch(type){
        case XmlPullParser::START_TAG:
            if(tagName.compare("item")==0){
                key = attrs.getString("name");
            }else if(tagName.compare("section")==0){
                section = attrs.getString("name");
            }
            break;
        case XmlPullParser::END_TAG:
            if(tagName.compare("item")==0){
                TextUtils::trim(value);
                setValue(section,key,value);
                value.clear();
            }
            break;
        case XmlPullParser::TEXT:
            value.append(parser.getText());
            break;
        }
    }
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

static std::vector<std::string> split(const std::string & path) {
    std::vector<std::string> vec;
    size_t begin;
    begin = path.find_first_not_of("|");
    while (begin != std::string::npos) {
        size_t end = path.find_first_of("|", begin);
        vec.push_back(path.substr(begin, end-begin));
        begin = path.find_first_not_of("|", end);
    }
    return vec;
}

int Preferences::getInt(const std::string&section,const std::string&key,const std::map<std::string,int>&kvs,int def){
    const std::string vstr = getString(section,key,"");
    if( vstr.size() && (vstr.find('|') != std::string::npos) ){
        std::vector<std::string> gs = split(vstr);
        int result= 0;
        int count = 0;
        for(std::string s:gs){
            auto it = kvs.find(s);
            if(it != kvs.end()){
                result |= it->second;
                count++;
            }
        }
        return count ? result : def;
    }else{
        auto it = kvs.find(vstr);
        return it == kvs.end() ? def : it->second;
    }    
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
    if(sec==mPrefs.end())
	    sec = mPrefs.insert({section,std::map<std::string,std::string>()}).first;
    auto kv = sec->second.find(key);
    if(kv == sec->second.end())
	    sec->second.insert({key,(v?"true":"false")});
    else kv->second = (v?"true":"false");
    updates++;
}

void Preferences::setValue(const std::string&section,const std::string&key,int v){
    auto sec = mPrefs.find(section);
    if(sec == mPrefs.end())
        sec = mPrefs.insert({section,std::map<std::string,std::string>()}).first;
    auto kv = sec->second.find(key);
    if(kv == sec->second.end())
	    sec->second.insert({key,std::to_string(v)});
    else kv->second = std::to_string(v);
    LOGV("%s %s %d",section.c_str(),key.c_str(),v);
    updates++;
}

void Preferences::setValue(const std::string&section,const std::string&key,float v){
    auto sec = mPrefs.find(section);
    if(sec == mPrefs.end())
        sec = mPrefs.insert({section,std::map<std::string,std::string>()}).first;
    auto kv = sec->second.find(key);
    if(kv == sec->second.end())
	    sec->second.insert({key,std::to_string(v)});
    else kv->second = std::to_string(v);
    LOGV("%s %s %f",section.c_str(),key.c_str(),v);
    updates++;
}

void Preferences::setValue(const std::string&section,const std::string&key,const std::string&v){
    auto sec = mPrefs.find(section);
    if(sec == mPrefs.end())
	    sec = mPrefs.insert({section,std::map<std::string,std::string>()}).first;
    auto kv = sec->second.find(key);
    if(kv == sec->second.end())
	    sec->second.insert({key,v});
    else kv->second = v;
    LOGV("%s %s %s",section.c_str(),key.c_str(),v.c_str());
    updates++;
}

void Preferences::setValue(const std::string&section,const std::string&key,double v){
    auto sec = mPrefs.find(section);
    if(sec==mPrefs.end())
	    sec = mPrefs.insert({section,std::map<std::string,std::string>()}).first;
    auto kv = sec->second.find(key);
    if(kv == sec->second.end())
	    sec->second.insert({key,std::to_string(v)});
    else kv->second = std::to_string(v);
    LOGV("%s %s %f",section.c_str(),key.c_str(),v);
    updates++;
}

}//namespace
