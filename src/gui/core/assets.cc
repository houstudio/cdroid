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
#include <assets.h>
#include <algorithm>
#include <cdtypes.h>
#include <cdlog.h>
#include <ziparchive.h>
#include <iostreams.h>
#include <iostream>
#include <fstream>
#include <drawables.h>
#include <textutils.h>
#include <limits.h>
#include <unistd.h>
#include <core/systemclock.h>
#include <drawables/drawableinflater.h>
#include <image-decoders/imagedecoder.h>

using namespace Cairo;
namespace cdroid {

Assets::Assets() {
    mNextAutofillViewId=100000;
}

Assets::Assets(const std::string&path):Assets() {
    addResource(path);
}

Assets::~Assets() {
    for(auto cls:mStateColors){
        delete cls.second;
    }
    mStateColors.clear();

    for(auto it=mResources.begin(); it!=mResources.end(); it++) {
        delete it->second;
    }
    for(auto d:mDrawables) {
        LOGD_IF(d.second.use_count(),"%s reference=%d",d.first.c_str(),d.second.use_count());
    }
    mDrawables.clear();
    mIDS.clear();
    mResources.clear();
    mStrings.clear();
    mStyles.clear();
    std::cout<<" Assets destroied!"<<std::endl;
}

const DisplayMetrics& Assets::getDisplayMetrics()const{
    return mDisplayMetrics;
}

const std::string Assets::getPackageName()const {
    return mName;
}

const std::string Assets::getTheme()const {
    return mThemeName;
}

void Assets::setTheme(const std::string&theme) {
    auto it = mStyles.find(theme);
    if(it!=mStyles.end()) {
        std::string pkg;
        mThemeName= theme;
        mTheme = it->second;
        parseResource(theme,nullptr,&pkg);
        LOGD("set Theme to %s",theme.c_str());
    } else {
        LOGE("Theme %s not found,[cdroid.pak %s] must be copied to your work directory!",theme.c_str(),
             mName.empty()?"":(mName+".pak").c_str());
    }
}


static std::string convertXmlToCString(const std::string& xml) {
    static std::unordered_map<std::string, std::string> escapeMap = {
        {"\\n", "\n"},     {"\\'", "\'"},   {"\\\"", "\""}
    };
    std::string result;
    result.reserve(xml.length());

    for (size_t i = 0; i < xml.length(); ++i) {
        bool replaced = false;
        for (const auto& pair : escapeMap) {
            if (xml.compare(i, pair.first.length(), pair.first) == 0) {
                result.append(pair.second);
                i += pair.first.length() - 1;
                replaced = true;
                break;
            }
        }
        if (!replaced) {
            result += xml[i];
        }
    }
    return result;
}

typedef struct{
    std::unordered_map<std::string,const std::string>colors;
    std::unordered_map<std::string,const std::string>dimens;
    std::unordered_map<std::string,std::vector<AttributeSet>>colorStateList;
}PENDINGRESOURCE;

static std::string getTrimedValue(XmlPullParser&parser){
    int type;
    std::string value;
    while((type=parser.next())!=XmlPullParser::END_TAG){
        if(type==XmlPullParser::TEXT){
            value.append(parser.getText());
        }
    }
    TextUtils::trim(value);
    return value;
}

int Assets::loadKeyValues(const std::string&package,const std::string&resid,void*params){
    int type,depth;
    XmlPullParser parser(this,resid);
    const AttributeSet& attrs=(AttributeSet&)parser;
    PENDINGRESOURCE*pending=(PENDINGRESOURCE*)params;
    while((type=parser.next())!=XmlPullParser::END_DOCUMENT){
        const std::string tag = parser.getName();
        if(type!=XmlPullParser::START_TAG)continue;
        if(tag.compare("id")==0){
            std::string key = package +":id/"+attrs.getString("name");
            std::string value= getTrimedValue(parser);
            mIDS[key] = TextUtils::strtol(value);
        }else if((tag.compare("dimen")==0)||(tag.compare("integer")==0)||(tag.compare("bool")==0)){
            const std::string resUri = package+":"+tag+"/"+attrs.getString("name");
            std::string value = getTrimedValue(parser);
            const std::string dimenRes = AttributeSet::normalize(package,value);
            auto itc = mDimensions.find(dimenRes);
            if(value.find("/")==std::string::npos){
                char*endP;
                int v = std::strtol(value.c_str(),&endP,10);
                if(*endP){
                    const DisplayMetrics& dm = getDisplayMetrics();
                    if(*endP=='s'/*sp*/) v = int(dm.scaledDensity * v /*+0.5f*/);
                    else if(*endP=='d'/*dp dip*/)v =int(dm.density * v /*+0.5f*/);
                }
                if(tag.compare("bool")==0){
                    v = value[0]=='t'?true:false;
                }
                mDimensions.insert({resUri,v});
            }else if(itc!=mDimensions.end()){
                mDimensions.insert({resUri,itc->second});
            }else{
                pending->dimens.insert({resUri,dimenRes});
            }
        }else if(tag.compare("color")==0){
            std::string colorUri = package+":color/"+attrs.getString("name");
            std::string value = getTrimedValue(parser);
            const std::string colorRef = AttributeSet::normalize(package,value);
            auto itc = mColors.find(colorRef);
            if((value[0]=='#')||(itc!=mColors.end())){
                const uint32_t color = (value[0]=='#')?Color::parseColor(value):itc->second;
                mColors.insert({colorUri,color});
            }else if (itc==mColors.end()){
                pending->colors.insert({colorUri,colorRef});
            }
        }else if(tag.compare("string")==0){
            std::string key = package+":string/"+attrs.getString("name");
            std::string value = getTrimedValue(parser);
            mStrings[key] = convertXmlToCString(value);
        }else if(tag.compare("item")==0){
            const std::string type = attrs.getString("type");
            if(type.compare("dimen")==0||type.compare("integer")==0||type.compare("bool")==0||type.compare("fraction")==0){
                const std::string resUri = package+":dimen/"+attrs.getString("name");
                const std::string format = attrs.getString("format");
                std::string value = getTrimedValue(parser);
                if((format.compare("float")==0)||(type[0]=='f')){
                    float fv =std::strtof(value.c_str(),nullptr);
                    if(type[0]=='f') fv/=100.f;
                    mDimensions.insert({resUri,*(int32_t*)&fv});
                }else{
                    const int32_t v = std::stol(value);
                    mDimensions.insert({resUri,v});
                }
            }else if(type.compare("id")){
                LOGD("CANT REACHED---------%s depth=%d %s",type.c_str(),parser.getDepth(),attrs.getString("name").c_str());
            }
        }else if(tag.compare("selector")==0){//for colorstatelist
            std::string key = attrs.getString("name");
            depth = parser.getDepth()+1;
            std::string resUri = resid.substr(0,resid.find(".xml"));
            std::unordered_map<std::string,std::vector<AttributeSet>>::iterator it;
            it = pending->colorStateList.end();
            while(((type=parser.next())!=XmlPullParser::END_DOCUMENT) && (parser.getDepth()>=depth) ){
                if(type!=XmlPullParser::START_TAG)continue;
                AttributeSet itemAtts(attrs);
                //itemAtts = attrs;
                if(it==pending->colorStateList.end()){
                    it = pending->colorStateList.insert({resUri,{itemAtts}}).first;
                }else
                    it->second.push_back(itemAtts);
            }
        }else if(tag.compare("style")==0){
            const std::string styleName = package+":style/"+attrs.getString("name");
            auto its =mStyles.find(styleName);
            if(its==mStyles.end()){
                const std::string styleParent = attrs.getString("parent");
                its =mStyles.insert(its,{styleName,AttributeSet(this,package)});
                if(styleParent.size())its->second.add("parent",styleParent);
            }
            depth = parser.getDepth()+1;
            while(((type=parser.next())!=XmlPullParser::END_DOCUMENT) && (parser.getDepth()>=depth) ){
                if(type!=XmlPullParser::START_TAG)continue;
                std::string key  = attrs.getString("name");
                std::string value= getTrimedValue(parser);
                value = AttributeSet::normalize(package,value);
                size_t pos =key.find(':');
                if(pos!=std::string::npos)key=key.substr(pos+1);
                its->second.add(key,value);
            }
        }else if(tag.find("array")!=std::string::npos){
            const std::string key = package+":array/"+attrs.getString("name");
            std::vector<std::string>array;
            depth = parser.getDepth()+1;
            while(((type=parser.next())!=XmlPullParser::END_DOCUMENT) && (parser.getDepth()>=depth) ){
                if(type!=XmlPullParser::START_TAG)continue;
                std::string value= getTrimedValue(parser);
                array.push_back(value);
            }
            mArraies.emplace(key,array);
        }
    }
    return 0;
}

int Assets::addResource(const std::string&path,const std::string&name) {
    ZIPArchive*pak = new ZIPArchive(path);
    std::string package = name;
    if(name.empty()) {
        size_t pos=path.find_last_of('/');
        if(pos != std::string::npos)
            package = path.substr(pos+1);
        pos = package.find('.');
        if( pos != std::string::npos)
            package = package.substr(0,pos);
    }
    mResources.insert({package,pak});

    int count=0;
    PENDINGRESOURCE pending;
    auto sttm = SystemClock::uptimeMillis();
    pak->forEachEntry([this,package,&count,&pending](const std::string&res) {
        count++;
        if((res.size()>6)&&(TextUtils::startWith(res,"values")||TextUtils::startWith(res,"color"))) {
            LOGV("LoadKeyValues from:%s",res.c_str());
            std::string resid = AttributeSet::normalize(package,res);//package+":"+res;
            resid = resid.substr(0,resid.find(".xml"));
            loadKeyValues(package,package+":"+res,&pending);
        }
        return 0;
    });
    if(name.compare("cdroid")==0)
        setTheme("cdroid:style/Theme");
    for(auto c:pending.colors){
        auto it = mColors.find(c.second);
        LOGD_IF(it==mColors.end(),"%s-->%s [X]",c.first.c_str(),c.second.c_str());
        if( it != mColors.end() ) mColors.insert({c.first,it->second});
    }
    for(auto d:pending.dimens){
        auto it = mDimensions.find(d.second);
        LOGD_IF(it==mDimensions.end(),"dimen %s losting refto %s",d.first.c_str(),d.second.c_str());
        if(it != mDimensions.end()) mDimensions.insert({d.first,it->second});
    }
    for(auto cs:pending.colorStateList){
        ColorStateList*cls = new ColorStateList();
        for(auto attr:cs.second){
            cls->addStateColor(this,attr);
        }
        mStateColors.insert({cs.first,cls});
    }
    const size_t preloadCount = mColors.size()+mDimensions.size()+mStateColors.size()+mArraies.size()+mStyles.size()+mStrings.size();
    LOGI("[%s] load %d assets from %d files [%d id,%d colors,%d stateColors, %d array,%d style,%d string,%d dimens] mTheme.size=%d used %dms",
         package.c_str(),preloadCount,count, mIDS.size(),mColors.size(),mStateColors.size(),mArraies.size(), mStyles.size(),
         mStrings.size(),mDimensions.size(),mTheme.getAttributeCount(),int(SystemClock::uptimeMillis()-sttm));
    return pak?0:-1;
}

static bool guessExtension(ZIPArchive*pak,std::string&ioname) {
    static const char* exts[]={".xml",".9.png",".png",".jpg",".gif",".apng",".webp",nullptr};
    if(ioname.find('.')!=std::string::npos)
        return true;
    for(int i=0;exts[i];i++){
        if(pak->hasEntry(ioname+exts[i],false)){
            ioname += exts[i];
            return true;
        }
    }
    return false;
}

//"@[package:][+]id/filname"
const std::string Assets::parseResource(const std::string&fullResId,std::string*res,std::string*ns)const {
    std::string pkg = mName;
    std::string relname= fullResId;
    std::string fullid = fullResId;

    size_t pos = fullid.find_last_of("@+");
    if(pos!=std::string::npos)fullid =fullid.erase(0,pos+1);

    pos= fullid.find(":");
    if(pos != std::string::npos) {
        pkg = fullid.substr(0,pos);
        relname = fullid.substr(pos+1);
    } else { //id/xxx
        pos = mName.find_last_of('/');
        if(pos != std::string::npos)
            pkg = mName.substr(pos+1);
        relname = fullid;
    }
    if(pkg =="android") pkg="cdroid";
    if( ns) *ns = pkg;
    if(res)*res = relname;
    return pkg+":"+relname;
}

ZIPArchive*Assets::getResource(const std::string&fullResId,std::string*relativeResID,std::string*outPackage)const {
    std::string package,resname;
    parseResource(fullResId,&resname,&package);
    auto it = mResources.find(package);
    ZIPArchive* pak = nullptr;
    if(outPackage) *outPackage = package;
    if(it != mResources.end()) { //convert noextname ->extname.
        pak = it->second;
        guessExtension(pak,resname);
        if(relativeResID) *relativeResID = resname;
    }
    LOGV_IF(pak==nullptr && resname.size(),"resource for [%s] is%s found",fullResId.c_str(),(pak?"":" not"));
    return pak;
}

std::unique_ptr<std::istream> Assets::getInputStream(const std::string&fullresid,std::string*outpkg) {
    std::string resname,package;
    ZIPArchive*pak = getResource(fullresid,&resname,&package);
    if(outpkg)*outpkg = package;
    if(pak){
        std::istream*stream = pak->getInputStream(resname);
        if(stream)return std::unique_ptr<std::istream>(stream);
    }
    if(fullresid.empty()||resname.empty())return nullptr;
    struct stat fs;
    if(stat(fullresid.c_str(),&fs)<0)return nullptr;
    return std::make_unique<std::ifstream>(fullresid);
}

void Assets::loadStrings(const std::string&lan) {
    const std::string suffix = "/strings-"+lan+".xml";
    for(auto a:mResources) {
        std::vector<std::string>files;
        a.second->getEntries(files);
        for(auto fileName:files){
            if( (TextUtils::endWith(fileName,".xml") && TextUtils::endWith(fileName,suffix) )==false)continue;
            loadKeyValues(a.first,fileName,nullptr);
            LOGD("load %s for '%s'",fileName.c_str(),lan.c_str());
        }
    }
}

Cairo::RefPtr<Cairo::ImageSurface> Assets::loadImage(std::istream&stream,int width,int height){
   return ImageDecoder::loadImage(stream,width,height); 
}

Cairo::RefPtr<Cairo::ImageSurface> Assets::loadImage(const std::string&resname,int width,int height){
    std::unique_ptr<std::istream> stm = getInputStream(resname);
    if(stm)return loadImage(*stm,width,height);
    return nullptr;
}

int Assets::getId(const std::string&resname)const {
    std::string resid,pkg;
    std::string key = resname;
    if(key.empty())return -1;
    if(key.length()&&(key.find('/')==std::string::npos))
        return TextUtils::strtol(key);
    auto pos = key.find('+');
    if(pos != std::string::npos)
        key.erase(pos,1);
    parseResource(key,&resid,&pkg);

    auto it = mIDS.find(pkg+":"+resid);
    return (it == mIDS.end())?-1:it->second;
}

int Assets::getNextAutofillId(){
    return mNextAutofillViewId++;
}

const std::string Assets::getString(const std::string& resid,const std::string&lan) {
    if((!lan.empty())&&(mLanguage!=lan)) {
        loadStrings(lan);
    }
    std::string str = resid;
    std::string pkg,name = resid;
    parseResource(resid,&name,&pkg);
    name = AttributeSet::normalize(pkg,resid);
    auto itr = mStrings.find(name);
    if(itr != mStrings.end()) {
        str = itr->second;
    }
    TextUtils::replace(str,"\\n","\n");
    return str;
}

size_t Assets::getArray(const std::string&resid,std::vector<int>&out) {
    std::string pkg,name = resid;
    std::string fullname = parseResource(resid,&name,&pkg);
    auto it = mArraies.find(fullname);
    if(it != mArraies.end()) {
        for(auto itm:it->second)
           out.push_back(std::stoi(itm));
        return it->second.size();
    }
    return  0;
}

size_t Assets::getArray(const std::string&resid,std::vector<std::string>&out) {
    std::string pkg,name = resid;
    std::string fullname = parseResource(resid,&name,&pkg);
    auto it = mArraies.find(fullname);
    if(it != mArraies.end()) {
        for(auto itm:it->second){
            itm = AttributeSet::normalize(pkg,itm);
            out.push_back(itm);
        }
        return it->second.size();
    }
    ZIPArchive * pak = getResource(resid,&name,nullptr);
    if(pak)pak->forEachEntry([&out,pkg](const std::string&res){
        if(TextUtils::startWith(res,"font")){
            std::string fullres = AttributeSet::normalize(pkg,res);
            out.push_back(fullres);
        }
        return out.size();
    });
    return 0;
}


Drawable* Assets::getDrawable(const std::string&resid) {
    Drawable* d = nullptr;
    std::string resname,package,ext,fullresid;
    if(resid.empty()||(resid.compare("null")==0)) {
        return nullptr;
    }
    fullresid = parseResource(resid,&resname,&package);
    ZIPArchive* pak = getResource(fullresid,&resname,nullptr);
    {
        auto it = mDrawables.find(fullresid);
        if( it != mDrawables.end() ) {
            if(it->second.expired()==false) {
                auto cs=it->second.lock();
                d= cs->newDrawable();
                LOGV("%s:%p use_count=%d",fullresid.c_str(),d,it->second.use_count());
                return d;
            }
            mDrawables.erase(it);
        }
    }
    auto extpos = resname.rfind(".");
    if(extpos!=std::string::npos)
        ext = resname.substr(extpos+1);
    //wrap png to drawable,make app develop simply
    if((resname[0]=='#')||(resname[1]=='x')||(resname[1]=='X')){
        LOGV("color %s",fullresid.c_str());
        d = new ColorDrawable(Color::parseColor(resname));
        mDrawables.insert(std::pair<std::string,std::weak_ptr<Drawable::ConstantState>>(fullresid,d->getConstantState()));
        return d;
    }
    if(resname.find("color/")!=std::string::npos){
        auto itc = mColors.find(fullresid);
        auto its = mStateColors.find(fullresid);
        if( itc != mColors.end() ){
            const uint32_t cc = (uint32_t)getColor(fullresid);
            LOGV("%s use colors as drawable",fullresid.c_str());
            d = new ColorDrawable(cc);
            mDrawables.insert(std::pair<std::string,std::weak_ptr<Drawable::ConstantState>>(fullresid,d->getConstantState()));
            return d;
        } else if(its != mStateColors.end()){
            LOGV("%s use colorstatelist as drawable",fullresid.c_str());
            d = new StateListDrawable(*its->second);
            mDrawables.insert(std::pair<std::string,std::weak_ptr<Drawable::ConstantState>>(fullresid,d->getConstantState()));
            return d;
        }
    }

    if(resname.find("attr/")!=std::string::npos) {//for reference resource
        resname = mTheme.getString(resname.substr(5));
        d = getDrawable(resname);
    } else if(resname.find("color/")!=std::string::npos) {
        const uint32_t cc = (uint32_t)getColor(fullresid);
        return new ColorDrawable(cc);
    } else if(ext.compare("xml")){
        if(resname.find(":")==std::string::npos){
            struct stat st;
            if(stat(resname.c_str(),&st))
                resname = package+":"+resname;
        }
        d = ImageDecoder::createAsDrawable(this,resname);
    }
    if( (d == nullptr) && (ext.compare("xml")==0) ) {
        d = DrawableInflater::loadDrawable(this,fullresid);//fromStream(this,zs,resname,package);
    }
    if(d) {
        mDrawables.insert({fullresid,std::weak_ptr<Drawable::ConstantState>(d->getConstantState())});
    }
    return d;
}

int Assets::getDimension(const std::string&refid)const{
    std::string pkg,name = refid;
    parseResource(name,nullptr,&pkg);
    name = AttributeSet::normalize(pkg,name);
    auto it = mDimensions.find(name);
    if(it != mDimensions.end()) 
        return it->second;
    throw std::runtime_error("Resource not found:" + refid);
}

int Assets::getDimensionPixelSize(const std::string&refid,int def)const{
    std::string pkg,name = refid;
    parseResource(name,nullptr,&pkg);
    name = AttributeSet::normalize(pkg,name);
    auto it = mDimensions.find(name);
    char*p = nullptr;
    std::string value;
    if(it != mDimensions.end())value =it->second;
    def = std::strtol(value.c_str(),&p,10);
    if(*p){
        const DisplayMetrics& dm=getDisplayMetrics();
        if(strncmp(p,"dp",2)==0||strncmp(p,"dip",3)==0)
            def = (dm.density * def /*+0.5f*/);
        if(strncmp(p,"sp",2)==0)
            def = int(dm.scaledDensity * def /*+0.5f*/);
    }
    return def;

}

bool Assets::getBoolean(const std::string&refid)const{
    return getDimension(refid);
}

float Assets::getFloat(const std::string&refid)const{
    const int32_t iv=getDimension(refid);
    return *((float*)&iv);
}

#pragma GCC push_options
#pragma GCC optimize("O0")
//codes between pragma will crashed in ubuntu GCC V8.x,bus GCC V7 wroked well.
int Assets::getColor(const std::string&refid) {
    std::string pkg,relname,name = refid;
    parseResource(name,&relname,&pkg);
    name = AttributeSet::normalize(pkg,name);
    auto it = mColors.find(name);
    if(it != mColors.end()) {
        return it->second;
    } if(relname.compare(0,4,"attr")==0){
        relname=relname.substr(5);
        name =  mTheme.getString(relname);
        return getColor(name);
    }else if(refid.find("?")!=std::string::npos){
        std::string clrRef = name;//mTheme.getString(name.substr(6));
        TextUtils::replace(clrRef,"attr","color");
        it = mColors.find(clrRef);
        if(it != mColors.end())
            return it->second;
        name = name.substr(name.find_last_of(":?/")+1);
        clrRef = mTheme.getString(name);
        return getColor(clrRef);
    }else if((refid[0]=='#')||refid.find(':')==std::string::npos) {
        return Color::parseColor(refid);
    } else if(refid.find("color/")==std::string::npos) { //refid is defined as an color reference
        parseResource(refid,&name,nullptr);
        name = mTheme.getString(name);
        return getColor(name);
    }
    throw std::runtime_error("Resource not found:" + refid);
}

ColorStateList* Assets::getColorStateList(const std::string&fullresid) {
    std::string pkg,name = fullresid,relname;
    parseResource(name,&relname,&pkg);
    name = AttributeSet::normalize(pkg,name);
    auto itc = mColors.find(name);
    auto its = mStateColors.find(name);
    if( its != mStateColors.end())
        return its->second;
    else if(itc != mColors.end()){
        ColorStateList* cls = new ColorStateList(itc->second);
        mStateColors.insert(std::pair<const std::string,ColorStateList*>(name,cls));
        return cls;
    }else if( name.size()&&(fullresid.find("attr")==std::string::npos) ) {
        const size_t slashpos = fullresid.find("/");
        try{
            ColorStateList* cls=ColorStateList::inflate(this,fullresid);
            mStateColors.insert(std::pair<const std::string,ColorStateList*>(name,cls));
            return cls;
        }catch(std::invalid_argument&e){
            std::string realName;
            parseResource(fullresid,&realName,nullptr);
            if(realName.find("?")!=std::string::npos)
            realName = mTheme.getString(realName);
            itc = mColors.find(realName);
            if(itc != mColors.end()){
                ColorStateList* cls = new ColorStateList(itc->second);
                mStateColors.insert(std::pair<const std::string,ColorStateList*>(name,cls));
                return cls;
            }
        }
    } else if(fullresid.find("attr")!=std::string::npos) {
        const size_t slashpos = fullresid.find("/");
        std::string name = fullresid.substr(slashpos+1);
        name = mTheme.getString(name);
        if(!name.empty())return getColorStateList(name);
    }
    LOGD_IF(!fullresid.empty(),"%s not found",fullresid.c_str());
    return nullptr;
}

#pragma GCC pop_options

void Assets::clearStyles() {
    mStyles.clear();
}

AttributeSet Assets::obtainStyledAttributes(const std::string&resname) {
    AttributeSet atts;
    std::string pkg,name = resname;
    size_t pos = name.find("attr/");
    if(pos!=std::string::npos){
        do {
            std::string key;
            if((pos=name.find('?'))!=std::string::npos)
                name.erase(pos,1);
            if((pos =name.find('/'))!=std::string::npos)
                name=name.substr(pos+1);
            key = name;
            name= mTheme.getString(key);
            atts.add(key,name);
            if((pos=name.find('@'))!=std::string::npos)
                name.erase(pos,1);
            pos = name.find("attr");
        }while(pos!=std::string::npos);
    }else{
        if((pos=name.find('?'))!=std::string::npos)
            name.erase(pos,1);
    }
    name = parseResource(name,nullptr,&pkg);
    auto it = mStyles.find(name);
    if(it != mStyles.end()){
        atts = it->second;
    }
    atts.setContext(this,pkg);
    std::string parent = atts.getString("parent");
    if(parent.length()) {
        if(parent.find('/')==std::string::npos)
            parent = std::string("style/")+parent;
        if(parent.find(':')==std::string::npos)
            parent = pkg+":"+parent;
        AttributeSet parentAtts = obtainStyledAttributes(parent);
        atts.inherit(parentAtts);
    }
    return atts;
}
}//namespace

