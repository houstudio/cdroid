#ifndef __CREATIONEXTRAS_H__
#define __CREATIONEXTRAS_H__
/*********************************************************************************
 * Port of androidx.lifecycle.viewmodel.CreationExtras. Kotlin's reified Key<T>()
 * factory becomes identity-by-address Key instances (each named key is a unique
 * static object). Values are stored type-erased via shared_ptr<void>; put() owns
 * (shared_ptr<T>), putRaw() borrows (no-op deleter). std::any is unavailable in
 * C++14, so this is the type-erasure mechanism.
 *********************************************************************************/
#include <map>
#include <memory>
namespace cdroid{
namespace lifecycle{

class MutableCreationExtras;

class CreationExtras{
public:
    /** Identity key; compared by address. */
    struct Key{ virtual ~Key(){} };

    CreationExtras() = default;
    virtual ~CreationExtras() = default;

    /** Returns the value bound to key cast to T*, or null. */
    template<typename T> T* get(const Key* key) const {
        auto it = mExtras.find(key);
        if(it == mExtras.end()) return nullptr;
        return static_cast<T*>(it->second.get());
    }
    bool contains(const Key* key) const { return mExtras.count(key) > 0; }

    friend class MutableCreationExtras;
protected:
    std::map<const Key*, std::shared_ptr<void>> mExtras;
};

class MutableCreationExtras : public CreationExtras{
public:
    MutableCreationExtras() = default;
    MutableCreationExtras(const CreationExtras& initial){ mExtras = initial.mExtras; }

    /** Stores an owned value (shared_ptr<T>). */
    template<typename T> void put(const Key* key, std::shared_ptr<T> value){
        mExtras[key] = std::static_pointer_cast<void>(value);
    }
    /** Stores a borrowed pointer (not deleted). */
    template<typename T> void putRaw(const Key* key, T* ptr){
        mExtras[key] = std::shared_ptr<void>(static_cast<void*>(ptr), [](void*){});
    }
};

/** Read-only empty CreationExtras singleton. */
const CreationExtras& EmptyCreationExtras();

}//namespace lifecycle
}//namespace cdroid
#endif
