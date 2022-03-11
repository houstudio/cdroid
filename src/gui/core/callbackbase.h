#pragma once
#include <functional>
#include <type_traits>
#include <atomic>
#include <memory>

namespace cdroid{

class Object{
};
template<typename R,typename... Args>
class CallbackBase{
private:
    using Functor=std::function<R(Args...)>;
    Functor fun;
    std::shared_ptr<int>mState;
public:
    CallbackBase(){
        fun=std::nullptr_t();
        mState=std::make_shared<int>(0);
    }
    void newInstance(){//called by uieventsource
        mState=std::make_shared<int>(0);
    }
    CallbackBase(const Functor&a):CallbackBase(){
        fun=a;
    }
    CallbackBase(const CallbackBase&b){
        mState=b.mState;
        fun=b.fun;
    }
    CallbackBase&operator=(const Functor&a){
        fun=a;
        return *this;
    }
    CallbackBase&operator=(const CallbackBase&b){
        mState=b.mState;
        fun=b.fun;
        return *this;
    }
    bool operator==(const CallbackBase&b)const{
        return mState.get()==b.mState.get();
    }
    operator bool()const{ return fun!=nullptr;  }
    bool operator==(std::nullptr_t)const{
       return fun==nullptr;
    }
    bool operator!=(std::nullptr_t)const{
       return fun!=nullptr;
    }
    virtual R operator()(Args...args){
        return fun(std::forward<Args>(args)...);
    }
};
}

