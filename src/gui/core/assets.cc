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


namespace cdroid{

Assets::Assets(){
    addResource("cdroid.pak","cdroid");
}
Assets::Assets(const std::string&path):Assets(){
    addResource(path);
    mName=path;
}
Assets::~Assets(){
    images.clear();
    strings.clear();
    for(auto it=mResources.begin();it!=mResources.end();it++)
       delete it->second;
    mResources.clear();
    LOGD("%p Destroied",this);
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
    return pak?0:-1;
}

//"@android:drawable/ic_dialog_email"
//"@drawable/test"
ZIPArchive*Assets::getResource(const std::string&fullResId,std::string*relativeResid)const{
    std::string pakName=mName;
    size_t pos=fullResId.find(':');
    std::string relname;
    if(pos!=std::string::npos){
        int startat=(fullResId.find('@')==std::string::npos)?0:1;
        pakName=fullResId.substr(startat,pos-startat);
        relname=fullResId.substr(pos+1);
    }else{
        pos=mName.find_last_of('/');
        if(pos!=std::string::npos)
            pakName=mName.substr(pos+1);
        relname=fullResId;
    }
    auto it=mResources.find(pakName);
    for(auto it=mResources.begin();it!=mResources.end();it++)
        LOGD_IF(it==mResources.end(),"%s:%p",it->first.c_str(),it->second);

    if(relativeResid) *relativeResid=relname;

    if(it!=mResources.end()) return it->second;

    LOGD("resource for [%s|%s] is not found",pakName.c_str(),relname.c_str());
    return nullptr;
}

std::unique_ptr<std::istream> Assets::getInputStream(const std::string&fullresid){
    std::string resname;
    ZIPArchive*pak=getResource(fullresid,&resname);
    std::istream*stream=pak?pak->getInputStream(resname):nullptr;
    std::unique_ptr<std::istream>is(stream);
    return is;
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

RefPtr<ImageSurface>Assets::getImage(const std::string&fullresid,bool cache){
    size_t capacity=0;
    std::string resname;
    ZIPArchive*pak=getResource(fullresid,&resname);

    auto it=images.find(fullresid);

    std::for_each(images.begin(),images.end(),[&capacity](const std::map<std::string,RefPtr<ImageSurface>>::value_type&it){
        RefPtr<ImageSurface>img=it.second;
        size_t picsize=img->get_width()*img->get_height()*4;
        capacity+=picsize;
        LOGV("%dx%d %dK[%s]",img->get_width(),img->get_height(),picsize/1024,it.first.c_str());
    });
    LOGV("image cache size=%dK",capacity/1024);

    if(it!=images.end()){
        RefPtr<ImageSurface>refimg(it->second);
        return refimg;
    }

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
    if(cache && (img!=nullptr)){
        images.insert(std::pair<const std::string,RefPtr<ImageSurface> >(fullresid,img));
        LOGV("image %s size=%dx%d",fullresid.c_str(),img->get_width(),img->get_height());
    }
    return img;
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
    if(TextUtils::endWith(resname,".9.png")){
	    d=new NinePatchDrawable(getImage(fullresid,false));
    }else if (TextUtils::endWith(resname,".png")){
	    d=new BitmapDrawable(fullresid);
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

AttributeSet Assets::obtainStyledAttributes(const std::string&fullresid){
    std::string resname;
    ZIPArchive*pak=getResource(fullresid,&resname);
    void*zfile=pak?pak->getZipHandle(resname):nullptr;
    AttributeSet attr;
    return attr;
}

}//namespace

