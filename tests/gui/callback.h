#pragma once
#include <functional>
#include <type_traits>
#include <atomic>
namespace test{
static std::atomic_int ID(0);

template<typename R,typename... Args>
class CallbackBase{
private:
    using Functor=std::function<R(Args...)>;
    Functor fun;
    long mID;
public:
    CallbackBase(){fun=nullptr_t();mID=ID++;}
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
    bool operator==(const CallbackBase&b){
        return mID==b.mID;
    }
    operator bool()const{
        return fun!=nullptr;
    }
    R operator()(Args...args)const{
        return fun(std::forward<Args>(args)...);
    }
};
}
