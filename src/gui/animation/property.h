#ifndef __ANIMATION_PROPERTY_H__
#define __ANIMATION_PROPERTY_H__
#include <string>
#include <core/variant.h>
namespace cdroid{
typedef nonstd::variant<int,uint32_t,float>AnimateValue;

#if variant_CPP17_OR_GREATER
   #define GET_VARIANT(vt,type) std::get<type>(vt)
#else
   #define GET_VARIANT(vt,type) vt.get<type>()
#endif

class Property{
private:
    std::string mName;
public:
    Property(const std::string&name);
    virtual AnimateValue get(void* t);
    virtual void set(void* object,const AnimateValue& value);
    const std::string getName()const;
    static Property*fromName(const std::string&);
    static bool reigsterProperty(const std::string&name,Property*prop);
};

}/*endof namespace*/
#endif/*__ANIMATION_PROPERTY_H__*/
