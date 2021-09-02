#ifndef __ATTRIBUTESET_H__
#define __ATTRIBUTESET_H__
#include <string>
#include <map>
namespace cdroid{
class AttributeSet{
private:
    std::string basePath;//only for path file 
    std::map<const std::string,const std::string>mAttrs;
    static std::map<const std::string,int>mIntAttrs;
public:
    AttributeSet();
    AttributeSet(const char*atts[],int size=0);
    bool add(const std::string&,const std::string&value);
    bool hasAttribute(const std::string&key)const;
    int size()const;
    int set(const char*atts[],int size=0);
    void setBasePath(const std::string&path);
    const std::string getAbsolutePath(const std::string&file)const;
    const std::string getAttributeValue(const std::string&key)const;
    bool getBoolean(const std::string&key,bool def=false)const;
    int getInt(const std::string&key,int def=0)const;
    int getResourceId(const std::string&key,int def=0)const;
    int getColor(const std::string&key,int def=0xFFFFFFFF)const;
    float getFloat(const std::string&key,float def=.0)const;
    const std::string getString(const std::string&key,const std::string&def=std::string())const;
    int getGravity(const std::string&key,int defvalue=0)const;
    int getDimensionPixelSize(const std::string&key,int def=0)const;
    int getDimensionPixelOffset(const std::string&key,int def=0)const;
    int getLayoutDimension(const std::string&key,int def)const;
    float getFraction(const std::string&key,int base,int pbase,float def=.0)const;
    void dump()const;
};
}
#endif
