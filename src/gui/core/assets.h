#ifndef __ASSETS_H__
#define __ASSETS_H__
#include <map>
#include <memory>
#include <string>
#include <functional>
#include <drawables/drawable.h>

namespace cdroid{

class Assets:public Context{
private:
    std::string mLanguage;
    std::string mDefault;//default resource
    std::map<const std::string,std::string>strings;
    std::map<const std::string,int>mIDS;
    std::map<const std::string,std::weak_ptr<Drawable::ConstantState>>mDrawables;
    std::map<const std::string,class ZIPArchive*>mResources;
    std::map<const std::string,AttributeSet>mStyles;
    void parseResource(const std::string&fullresid,std::string*res,std::string*ns)const;
    ZIPArchive*getResource(const std::string & fullresid, std::string* relativeResid)const;
    int fetchIdFromResource(const std::string&fullresid);
protected:
    std::string mName;
    DisplayMetrics mDisplayMetrics;
    void loadStrings(const std::string&lan);
    int addResource(const std::string&path,const std::string&name=std::string());
    int loadKeyValues(const std::string&resid,
        std::function<void(const std::string&tag,const AttributeSet*,const AttributeSet&,const std::string&,int)>func);
    int loadAttributes(std::map<const std::string,AttributeSet>&atts,const std::string&resid);
public:
    Assets();
    Assets(const std::string&path);
    ~Assets();
    int loadStyles(const std::string&resid);
    void clearStyles();
    const DisplayMetrics&getDisplayMetrics()override;
    int getId(const std::string&)const override;
    const std::string& getString(const std::string&id,const std::string&lan="")override;
    RefPtr<Cairo::ImageSurface> getImage(const std::string&resname)override;
    std::vector<std::string> getStringArray(const std::string&resname,const std::string&arrayname)const;
    std::unique_ptr<std::istream> getInputStream(const std::string&resname)override;
    Drawable * getDrawable(const std::string&resid)override;
    ColorStateList* getColorStateList(const std::string&resid)override;
    AttributeSet obtainStyledAttributes(const std::string&)override;
};

}//namespace
#endif
