#ifndef __BUNDLE_H__
#define __BUNDLE_H__
#include <core/basebundle.h>
namespace cdroid{
class Bundle:public BaseBundle{
public:
    // androidx Bundle.putBundle / getBundle — store/retrieve a nested Bundle (data_ is protected).
    void putBundle(const std::string& key, Bundle* value){
        data_[key] = value;
    }
    Bundle* getBundle(const std::string& key) const{
        auto it = data_.find(key);
        if(it == data_.end()) return nullptr;
        try{ return any_cast<Bundle*>(it->second); }
        catch(const bad_any_cast&){ return nullptr; }
    }
};
}
#endif
