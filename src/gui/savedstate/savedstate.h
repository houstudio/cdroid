#ifndef __SAVEDSTATE_SAVEDSTATE_H__
#define __SAVEDSTATE_SAVEDSTATE_H__
/*********************************************************************************
 * Port of androidx.savedstate.SavedState. A key-value container backed by
 * nonstd::any (core/any.h) supporting primitive types and nested SavedState.
 * Replaces the kotlinx-serialization SavedState codec with a plain map (the
 * stable read/write/getSavedState/putSavedState/contains/remove/isEmpty/putAll
 * surface used by Fragment/Navigation).
 *********************************************************************************/
#include <string>
#include <unordered_map>
#include <core/any.h>

namespace cdroid{
namespace savedstate{

class SavedState{
public:
    SavedState() = default;

    template<typename T> void put(const std::string& key, const T& value){ mMap[key] = value; }
    template<typename T> bool get(const std::string& key, T* out) const {
        auto it = mMap.find(key);
        if(it == mMap.end()) return false;
        if(out){ try{ *out = nonstd::any_cast<T>(it->second); } catch(const nonstd::bad_any_cast&){ return false; } }
        return true;
    }

    void putSavedState(const std::string& key, const SavedState& value){ mMap[key] = value; }
    // Returns a pointer into this SavedState (valid until the entry is removed/modified).
    const SavedState* getSavedState(const std::string& key) const {
        auto it = mMap.find(key);
        if(it == mMap.end()) return nullptr;
        return nonstd::any_cast<SavedState>(&it->second);
    }

    bool contains(const std::string& key) const { return mMap.find(key) != mMap.end(); }
    void remove(const std::string& key){ mMap.erase(key); }
    bool isEmpty() const { return mMap.empty(); }
    void clear(){ mMap.clear(); }
    void putAll(const SavedState& other){
        for(const auto& kv : other.mMap) mMap[kv.first] = kv.second;
    }

private:
    std::unordered_map<std::string, nonstd::any> mMap;
};

}//namespace savedstate
}//namespace cdroid
#endif
