#ifndef __ANIMATION_PROPERTY_H__
#define __ANIMATION_PROPERTY_H__
#include <string>
namespace cdroid{

class Property{
private:
    std::string mName;
public:
    Property(const std::string&name);
    virtual float get(void* t);
    virtual void set(void* object, float value);
    const std::string getName()const;
};

}/*endof namespace*/
#endif/*__ANIMATION_PROPERTY_H__*/
