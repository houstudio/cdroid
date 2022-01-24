#include <assets.h>
#include <algorithm>
#include <cdtypes.h>
#include <cdlog.h>
#include <json/json.h>
#include <ziparchive.h>
#include <iostreams.h>
#include <iostream>
#include <fstream>
#include <drawables.h>
#include <textutils.h>
#include <expat.h>
#include <limits.h>

namespace cdroid{

Assets::Assets(){
    addResource("cdroid.pak","cdroid");
}

Assets::Assets(const std::string&path):Assets(){
    addResource(path);
    mName=path;
}

Assets::~Assets(){
    for(auto it=mResources.begin();it!=mResources.end();it++)
       delete it->second;
    mResources.clear();
    LOGD("%p Destroied",this);
}

const DisplayMetrics& Assets::getDisplayMetrics(){
    return mDisplayMetrics;
}

int Assets::addResource(const std::string&path,const std::string&name){
    ZIPArchive*pak=new ZIPArchive(path);
    size_t pos=name.find_last_of('/');
    std::string key=name;
    key=(pos!=std::string::npos)?name.substr(pos+1):name;
    mResources.insert(std::pair<const std::string,ZIPArchive*>(key,pak));
    LOGD("%s:%p",key.c_str(),pak);
    std::vector<std::string>entries;
    pak->getEntries(entries);
    LOGD("entries.count=%d pakpath=%s",entries.size(),path.c_str());
    fetchIdFromResource(name+":values/ID.xml");
    return pak?0:-1;
}

static bool guessExtension(ZIPArchive*pak,std::string&ioname){
    bool ret=(ioname.find('.')!=std::string::npos);
    if(ret)return ret;
    if(TextUtils::startWith(ioname,"drawable")){
        if(ret=pak->hasEntry(ioname+".xml",false))
            ioname+=".xml";
    }else if(TextUtils::startWith(ioname,"mipmap")){
        if(ret=pak->hasEntry(ioname+".png",false)) ioname+=".png";
        else if(ret=pak->hasEntry(ioname+".9.png",false))ioname+=".9.png";
        else if(ret=pak->hasEntry(ioname+".jpg",false))ioname+=".jpg";
    }
    return ret;
}

//"@[+][package:]id/filname"
void Assets::parseResource(const std::string&fullResId,std::string*res,std::string*ns)const{
    std::string pkg=mName;
    size_t pos=fullResId.find(":");
    std::string relname;
    if(pos!=std::string::npos){
        const int pluspos=fullResId.find('+');
        const int atpos=fullResId.find('@');
        const int startat=(pluspos!=std::string::npos)?pluspos:((atpos==std::string::npos)?0:atpos);
        pkg=fullResId.substr(startat,pos-startat);
        relname=fullResId.substr(pos+1);
    }else{//@+id/
        pos=mName.find_last_of('/');
        if(pos!=std::string::npos)
            pkg=mName.substr(pos+1);
        pos=fullResId.find('+');
        if(pos==std::string::npos)
            pos=fullResId.find('@');
        if(pos!=std::string::npos)relname=fullResId.substr(pos+1);
        else relname=fullResId;
    }
    if(res)*res=relname;
    if(ns)*ns=pkg;
}

ZIPArchive*Assets::getResource(const std::string&fullResId,std::string*relativeResid)const{
    std::string package,resname;
    parseResource(fullResId,&resname,&package);
    auto it=mResources.find(package);
    ZIPArchive*pak=nullptr;
    if(it!=mResources.end()){//convert noextname ->extname.
        pak=it->second;
        guessExtension(pak,resname);
        if(relativeResid) *relativeResid=resname;
    }
    LOGV_IF(resname.size(),"resource for [%s::%s:%s] is%s found",package.c_str(),fullResId.c_str(),resname.c_str(),(pak?"":" not"));
    return pak;
}

std::unique_ptr<std::istream> Assets::getInputStream(const std::string&fullresid){
    std::string resname;
    ZIPArchive*pak=getResource(fullresid,&resname);
    std::istream*stream=pak?pak->getInputStream(resname):nullptr;
    std::unique_ptr<std::istream>is(stream);
    return is;
}

int Assets::fetchIdFromResource(const std::string&fullresid){
    int count=0;
    std::string package;
    parseResource(fullresid,nullptr,&package);
    package+=":id/";
    auto func=[&](const std::string&section,const AttributeSet*att1,
            const AttributeSet&att2,const std::string&value,int index){
        if(section.length()&&section[0]=='i'&&section[1]=='d'){
            const std::string name=package+att2.getString("name");
            mIDS[name]=TextUtils::strtol(value);
            LOGV("%s->%s",name.c_str(),value.c_str());
            count++;
        }
    };
    loadKeyValues(fullresid,func);
    LOGD("load %d ids from %s",count,fullresid.c_str());
    return count;
}

void Assets::loadStrings(const std::string&lan){
    const std::string fname="strings/strings-"+lan+".json";
    Json::CharReaderBuilder builder;
    Json::String errs;
    Json::Value root;
    ZIPArchive*pak=getResource(std::string(),nullptr);
    std::istream*zipis=pak->getInputStream(fname);
    LOGD("%s zip=%p good=%d",fname.c_str(),zipis,zipis?zipis->good():-2);
    if(zipis==nullptr||zipis->good()==false)
        return;
    Json::parseFromStream(builder,*zipis,&root,&errs);
    Json::Value::Members mems=root.getMemberNames();
    for(auto m:mems){
        strings[m]=root[m].asString();
    }
    delete zipis;
}

RefPtr<ImageSurface>Assets::getImage(const std::string&fullresid){
    size_t capacity=0;
    std::string resname;
    ZIPArchive*pak=getResource(fullresid,&resname);

    void*zfile=pak?pak->getZipHandle(resname):nullptr;
    ZipInputStream zipis(zfile);
    RefPtr<ImageSurface>img;
    if(!zipis.good()){
        std::ifstream fi(fullresid);
        img=loadImage(fi);
        LOGD_IF(zfile==nullptr&&fi.good()==false,"pak=%p %s open failed ",pak,resname.c_str());
        return img;
    }
    img=loadImage(zipis);
    LOGV_IF(img,"image %s size=%dx%d",fullresid.c_str(),img->get_width(),img->get_height());
    return img;
}

int Assets::getId(const std::string&key)const{
    std::string resid,pkg;
    if(key.empty())return -1;
    if(key.length()&&(key.find('/')==std::string::npos))
       return TextUtils::strtol(key);
    parseResource(key,&resid,&pkg);
    auto it=mIDS.find(pkg+":"+resid);
    return it==mIDS.end()?-1:it->second;
}

const std::string& Assets::getString(const std::string& id,const std::string&lan){
    if((!lan.empty())&&(mLanguage!=lan)){
        loadStrings(lan);
    }
    auto itr=strings.find(id);
    if(itr!=strings.end()&&!itr->second.empty()){
         return itr->second;
    }
    return id;
}

std::vector<std::string>Assets::getStringArray(const std::string&resname,const std::string&arrayname)const{
    Json::Value d;
    Json::CharReaderBuilder builder;
    Json::String errs;
    std::vector<std::string>sarray;
    ZIPArchive*pak=getResource(std::string(),nullptr);
    std::shared_ptr<std::istream>zipis(pak->getInputStream(resname));
    bool rc=Json::parseFromStream(builder,*zipis,&d,&errs);
    LOGE_IF(rc,"%s Error %s at %d",resname.c_str(),errs.c_str());
    return sarray;
}

Drawable* Assets::getDrawable(const std::string&fullresid){
    Drawable*d=nullptr;
    std::string resname;
    ZIPArchive*pak=getResource(fullresid,&resname);
    auto it=mDrawables.find(fullresid);
    if(it!=mDrawables.end() ){
        if(it->second.expired()==false){
            auto cs=it->second.lock();
            d= cs->newDrawable();
            LOGV("%s:%p use_count=%d",fullresid.c_str(),d,it->second.use_count());
            return d;
        }
        mDrawables.erase(it);
    }
    //wrap png to drawable,make app develop simply
    if(resname[0]=='#'||resname[1]=='x'||resname[1]=='X'||resname.find('/')==std::string::npos){
        LOGV("color %s",fullresid.c_str());
        return new ColorDrawable(Color::parseColor(resname));
    }
    if(TextUtils::endWith(resname,".9.png")){
        d=new NinePatchDrawable(this,fullresid);
    }else if (TextUtils::endWith(resname,".png")||TextUtils::endWith(resname,".jpg")){
        d=new BitmapDrawable(this,fullresid);
    }
    if( (d==nullptr) && (!fullresid.empty()) ){
        void*zfile=pak?pak->getZipHandle(resname):nullptr;
        if(zfile){
            ZipInputStream zs(zfile);
            d=Drawable::fromStream(this,zs,resname);
        }else if(!resname.empty()){
            std::ifstream fs(fullresid);
            d=Drawable::fromStream(nullptr,fs,resname);
        }
        LOGD_IF(zfile==nullptr,"drawable %s load failed",fullresid.c_str());
    }
    if(d){
        mDrawables.insert(std::pair<const std::string,
            std::weak_ptr<Drawable::ConstantState>>(fullresid,d->getConstantState()));
    }
    return d;
}

ColorStateList* Assets::getColorStateList(const std::string&fullresid){
    std::string resname;
    ZIPArchive*pak=getResource(fullresid,&resname);
    void*zfile=pak?pak->getZipHandle(resname):nullptr;
    if(zfile){
        ZipInputStream zs(zfile);
        return ColorStateList::fromStream(this,zs,resname);
    }else if(!fullresid.empty()){
        std::ifstream fs(fullresid);
        return ColorStateList::fromStream(this,fs,resname);
    }
    return nullptr;
}

typedef struct{
   std::vector<std::string> tags;
   std::vector<AttributeSet> attrs;
   std::string value;
   int level;
   int index;
   std::function<void(const std::string&,const AttributeSet*,const AttributeSet&,const std::string&,int)>func;
}KVPARSER;

static void startElement(void *userData, const XML_Char *name, const XML_Char **satts){
    KVPARSER*kvp=(KVPARSER*)userData;
    if(strcmp(name,"resources")){
        kvp->attrs.push_back(AttributeSet(satts));
        kvp->tags.push_back(name);
        kvp->level++;
        kvp->value=std::string();
    }else{//strcmp(name,"resources")
        //do nothing
        kvp->level=0;
    }
}

static void CharacterHandler(void *userData,const XML_Char *s, int len){
    KVPARSER*kvp=(KVPARSER*)userData;
    kvp->value+=std::string(s,len);
}

static void endElement(void *userData, const XML_Char *name){
    KVPARSER*kvp=(KVPARSER*)userData;
    if(strcmp(name,"resources")){
        TextUtils::trim(kvp->value);
        const std::string& tag=kvp->tags.back();
        const AttributeSet&atts1=kvp->attrs.back();
        const AttributeSet*atts0=kvp->level<2?nullptr:&kvp->attrs[kvp->level-2];
        kvp->func(tag,atts0,atts1,kvp->value,kvp->index);
        kvp->tags.pop_back();
        kvp->attrs.pop_back();
        kvp->value=std::string();
        kvp->index++;
        kvp->level--;  
    }else if(strcmp(name,"resources")){
        //kvp->func(kvp->parentAttr,name,name,INT_MIN);
    }
}

int Assets::loadStyles(const std::string&fullresid){
    return loadAttributes(mStyles,fullresid);
}

int Assets::loadKeyValues(const std::string&fullresid,std::function<void(const std::string&,
        const AttributeSet*,const AttributeSet&,const std::string&,int)>func){
    int len = 0;
    char buf[256];

    std::unique_ptr<std::istream>stream=getInputStream(fullresid);
    LOGE_IF(stream==nullptr,"%s load failed",fullresid.c_str());
    if(stream==nullptr)
        return 0;
    XML_Parser parser=XML_ParserCreate(nullptr);
    std::string curKey;
    std::string curValue;
    KVPARSER kvp;
    kvp.func=func;
    XML_SetUserData(parser,&kvp);
    XML_SetElementHandler(parser, startElement, endElement);
    XML_SetCharacterDataHandler(parser,CharacterHandler);
    do {
        stream->read(buf,sizeof(buf));
        len=stream->gcount();
        if (XML_Parse(parser, buf,len,len==0) == XML_STATUS_ERROR) {
            const char*es=XML_ErrorString(XML_GetErrorCode(parser));
            LOGE("%s at line %ld",es, XML_GetCurrentLineNumber(parser));
            XML_ParserFree(parser);
            return 0;
        }
    } while(len!=0);
    XML_ParserFree(parser); 
}

int Assets::loadAttributes(std::map<const std::string,AttributeSet>&attmaps,const std::string&fullresid){
    std::map<const std::string,AttributeSet>::iterator itr=attmaps.end();
    loadKeyValues(fullresid,[&attmaps,&itr](const std::string&tag,const AttributeSet*parentAttrs,
                const AttributeSet&atts,const std::string&value,int index){
        const std::string key=atts.getString("name");
        if(index==0){
            const std::string name=parentAttrs->getString("name");
            auto it2=attmaps.insert(std::make_pair<const std::string,AttributeSet>(name.c_str(),AttributeSet()));
            itr=it2.first;
        }
        if(index!=INT_MIN)itr->second.add(key,value);
    });
    LOGD("%d attributes loaded!",attmaps.size());
    return attmaps.size();
}

void Assets::clearStyles(){
    mStyles.clear();
}

AttributeSet Assets::obtainStyledAttributes(const std::string&name){
    auto it=mStyles.find(name);
    if(it!=mStyles.end())return it->second;
    return AttributeSet();
}

}//namespace

