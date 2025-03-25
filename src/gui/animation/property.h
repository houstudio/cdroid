#ifndef __ANIMATION_PROPERTY_H__
#define __ANIMATION_PROPERTY_H__
#include <string>
#include <core/variant.h>
#include <drawables/pathparser.h>
namespace cdroid{
typedef nonstd::variant<int,float,PathParser::PathData>AnimateValue;

#if variant_CPP17_OR_GREATER
   #define GET_VARIANT(vt,type) std::get<type>(vt)
#else
   #define GET_VARIANT(vt,type) vt.get<type>()
#endif

class Property{
public:
    static constexpr int UNDEFINED =-1;
    static constexpr int INT_TYPE = 0;
    static constexpr int COLOR_TYPE=1;
    static constexpr int FLOAT_TYPE=2;
    static constexpr int PATH_TYPE =3;
protected:
    int mType;
    std::string mName;
public:
    Property(const std::string&name);
    Property(const std::string&name,int type);
    virtual AnimateValue get(void* t);
    virtual void set(void* object,const AnimateValue& value);
    const std::string getName()const;
    int getType()const;
    static Property*fromName(const std::string&propertyName);
    static Property*fromName(const std::string&className,const std::string&propertyName);
    static bool reigsterProperty(const std::string&name,Property*prop);
};

}/*endof namespace*/
#endif/*__ANIMATION_PROPERTY_H__*/
