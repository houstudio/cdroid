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
    // android.os.Bundle.putParcelable / getParcelable — stores a borrowed
    // Parcelable* (the Bundle does not take ownership; AOSP stores a strong
    // reference, the single-threaded CDROID runtime borrows instead).
    void putParcelable(const std::string& key, Parcelable* value){
        data_[key] = value;
    }
    Parcelable* getParcelable(const std::string& key) const{
        auto it = data_.find(key);
        if(it == data_.end()) return nullptr;
        try{ return any_cast<Parcelable*>(it->second); }
        catch(const bad_any_cast&){ return nullptr; }
    }
};
}
#endif
