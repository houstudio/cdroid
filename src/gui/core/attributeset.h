#ifndef __ATTRIBUTESET_H__
#define __ATTRIBUTESET_H__
#include <string>
#include <map>
#include <core/displaymetrics.h>

namespace cdroid{
class AttributeSet{
private:
    std::string mPackage;
    class Context*mContext;
    DisplayMetrics mDisplayMetrics;
    std::map<std::string,std::string>mAttrs;
public:
    AttributeSet();
    AttributeSet(Context*ctx,const std::string&package);
    void setContext(Context*,const std::string&package);
    bool add(const std::string&,const std::string&value);
    bool hasAttribute(const std::string&key)const;
    int  size()const;
    int  set(const char*atts[],int size=0);
    static std::string normalize(const std::string&pkg,const std::string&property);
    std::map<std::string,std::string>&getEntries();
    int  inherit(const AttributeSet&other);
    const std::string getAttributeValue(const std::string&key)const;
    bool getBoolean(const std::string&key,bool def=false)const;
    int  getInt(const std::string&key,int def=0)const;
    int  getInt(const std::string&key,const std::map<const std::string,int>&keyvaluemaps,int def=0)const;
    int  getResourceId(const std::string&key,int def=0)const;
    int  getColor(const std::string&key,int def=0xFFFFFFFF)const;
    float getFloat(const std::string&key,float def=.0)const;
    const std::string getString(const std::string&key,const std::string&def=std::string())const;
    int  getGravity(const std::string&key,int defvalue=0)const;
    int  getDimensionPixelSize(const std::string&key,int def=0)const;
    int  getDimensionPixelOffset(const std::string&key,int def=0)const;
    int  getLayoutDimension(const std::string&key,int def)const;
    float getFraction(const std::string&key,int base,int pbase,float def=.0)const;
    void dump()const;
};
}
#endif
