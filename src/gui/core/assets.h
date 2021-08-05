#ifndef __ASSETS_H__
#define __ASSETS_H__
#include <map>
#include <memory>
#include <string>
#include <drawables/drawable.h>

namespace cdroid{

class Assets:public Context{
private:
    std::string mLanguage;
    std::string mDefault;//default resource
    std::map<const std::string,RefPtr<ImageSurface>>images;
    std::map<const std::string,std::string>strings;
    std::map<const std::string,std::weak_ptr<Drawable::ConstantState>>mDrawables;
    std::map<const std::string,class ZIPArchive*>mResources;
    ZIPArchive*getResource(const std::string & fullresid, std::string* relativeResid)const;
protected:
    std::string mName;
    void loadStrings(const std::string&lan);
    int addResource(const std::string&path,const std::string&name=std::string());
public:
    Assets();
    Assets(const std::string&path);
    ~Assets();
    const std::string& getString(const std::string&id,const std::string&lan="")override;
    RefPtr<Cairo::ImageSurface> getImage(const std::string&resname,bool cache=true)override;
    std::vector<std::string> getStringArray(const std::string&resname,const std::string&arrayname)const;
    std::unique_ptr<std::istream> getInputStream(const std::string&resname)override;
    Drawable * getDrawable(const std::string&resid)override;
    ColorStateList* getColorStateList(const std::string&resid)override;
};

}//namespace
#endif
