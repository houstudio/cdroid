#pragma once
#include <functional>
#include <type_traits>
#include <atomic>
namespace cdroid{

extern std::atomic_int mCallbackID;

template<typename R,typename... Args>
class CallbackBase{
private:
    using Functor=std::function<R(Args...)>;
    Functor fun;
    long mID;
public:
    CallbackBase(){fun=nullptr_t();mID=mCallbackID++;}
    CallbackBase(const Functor&a):CallbackBase(){
        fun=a;
    }
    CallbackBase&operator=(const Functor&a){
        fun=a;
        return *this;
    }
    CallbackBase&operator=(const CallbackBase&b){
        mID=b.mID;
        fun=b.fun;
        return *this;
    }
    bool operator==(const CallbackBase&b)const{
        return mID==b.mID;
    }
    operator bool()const{ return fun!=nullptr;  }
    operator int() const{ return mID; }
    bool operator==(nullptr_t)const{
       return fun==nullptr;
    }
    bool operator!=(nullptr_t)const{
       return fun!=nullptr;
    }
    R operator()(Args...args)const{
        return fun(std::forward<Args>(args)...);
    }
};
}

