#ifndef __NEVER_DESTROYED_H__
#define __NEVER_DESTROYED_H__
#include <type_traits>
#include <utility>
namespace  cdroid{

template <typename T>
class NeverDestroyed {
public:
    template <typename... Args>
    explicit NeverDestroyed(Args&&... args){
        new (storagePointer()) T(std::forward<Args>(args)...);
    }

    NeverDestroyed(const NeverDestroyed&) = delete;
    NeverDestroyed& operator=(const NeverDestroyed&) = delete;

    ~NeverDestroyed(){
        get()->~T();
    }
    T* get() const { return storagePointer();}
    T* operator->() const { return storagePointer(); }
    T& operator*() const { return *storagePointer(); }
private:
    using PointerType = typename std::remove_const<T>::type*;
    //alignas(T) unsigned char m_storage[sizeof(T)];
    typename std::aligned_storage<sizeof(T), std::alignment_of<T>::value>::type m_storage;
    PointerType storagePointer() const { return const_cast<PointerType>(reinterpret_cast<const T*>(&m_storage)); }
};
}/*endof namespace*/
#endif
