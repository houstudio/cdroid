#ifndef __INTENT_H__
#define __INTENT_H__
#include <core/attributeset.h>
#include <core/context.h>

namespace cdroid{
class Window;

class Intent{
public:
    typedef std::function< Window*(int,int) > WindowFactory;
private:
    static std::map<std::string,WindowFactory>mIntentMaps;
    std::string mAction;
    Context*mContext;
public:
    static int registIntent(const std::string&intentName,WindowFactory factory);
    Intent(const std::string&);
    Intent(const Intent&other);
    bool operator==(const Intent&other)const;
};

template<typename T>
class IntentRegister{
public:
    IntentRegister(const std::string&name,Intent::WindowFactory ){
        Intent::registIntent(name,[](int w,int h){return new T(w,h);});
    }
};

#define DECLARE_INTENT(T) static IntentRegister<T> intent_inflater_##T(#T,"");
}//endof namespace
#endif
