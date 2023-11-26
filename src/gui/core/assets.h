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
    int mNextAutofillViewId;
    std::string mLanguage;
    std::string mDefault;//default resource
    std::string mThemeName;
    AttributeSet mTheme;
    std::map<const std::string,std::string>mStrings;
    std::map<const std::string,int>mIDS;
    std::map<const std::string,std::vector<std::string>>mArraies;
    std::map<const std::string,std::weak_ptr<Drawable::ConstantState>>mDrawables;
    std::map<const std::string,class ZIPArchive*>mResources;
    std::map<const std::string,AttributeSet>mStyles;
    std::map<const std::string,uint32_t>mColors;
    std::map<const std::string,ColorStateList*>mStateColors;
    const std::string parseResource(const std::string&fullresid,std::string*res,std::string*ns)const;
    void parseItem(const std::string&package,const std::vector<std::string>&tag,std::vector<AttributeSet>atts,const std::string&value);
    ZIPArchive*getResource(const std::string & fullresid, std::string* relativeResid,std::string*package)const;
protected:
    std::string mName;
    DisplayMetrics mDisplayMetrics;
    void loadStrings(const std::string&lan);
    int addResource(const std::string&path,const std::string&name=std::string());
    int loadKeyValues(const std::string&resid,
        std::function<void(const std::vector<std::string>&tags,const std::vector<AttributeSet>&,const std::string&)>func);
public:
    Assets();
    Assets(const std::string&path);
    ~Assets();
    int loadStyles(const std::string&resid);
    void clearStyles();
    const std::string getPackageName()const override;
    const std::string getTheme()const override;
    void setTheme(const std::string&theme)override;
    const DisplayMetrics&getDisplayMetrics()const override;
    int getId(const std::string&)const override;
    int getNextAutofillId()override;
    const std::string getString(const std::string&id,const std::string&lan="")override;
    Cairo::RefPtr<Cairo::ImageSurface> loadImage(const std::string&resname)override;
    Cairo::RefPtr<Cairo::ImageSurface> loadImage(const std::string&, int, int, int)override;
    std::vector<std::string> getStringArray(const std::string&resname,const std::string&arrayname)const;
    std::unique_ptr<std::istream> getInputStream(const std::string&resname,std::string*outpkg=nullptr)override;
    Drawable * getDrawable(const std::string&resid)override;
    int getColor(const std::string&resid)override;
    int getArray(const std::string&resid,std::vector<int>&)override;
    int getArray(const std::string&resid,std::vector<std::string>&)override;
    ColorStateList* getColorStateList(const std::string&resid)override;
    AttributeSet obtainStyledAttributes(const std::string&)override;
};

}//namespace
#endif
