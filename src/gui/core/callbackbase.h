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
protected:
    using Functor=std::function<R(Args...)>;
    std::shared_ptr<Functor>mFunctor;
public:
    CallbackBase(){
        mFunctor = std::make_shared<Functor>();
    }
    CallbackBase(const Functor&a):CallbackBase(){
        mFunctor = std::make_shared<Functor>(a);
    }
    CallbackBase(const CallbackBase&b){
        mFunctor = b.mFunctor;
    }
    virtual ~CallbackBase(){}
    CallbackBase&operator=(const Functor&a){
        mFunctor = std::make_shared<Functor>(a);
        return *this;
    }
    CallbackBase&operator=(const CallbackBase&b){
        mFunctor = b.mFunctor;
        return *this;
    }
    bool operator == (const CallbackBase&b)const{
        return mFunctor.get() == b.mFunctor.get();
    }
    operator bool()const{ 
        return (*mFunctor)!=nullptr;
    }
    bool operator==(std::nullptr_t)const{
        return (*mFunctor) == nullptr;
    }
    bool operator!=(std::nullptr_t)const{
        return (*mFunctor) != nullptr;
    }
    void reset(){
        mFunctor = nullptr;
    }
    virtual R operator()(Args...args){
        return (*mFunctor)(std::forward<Args>(args)...);
    }
};
typedef CallbackBase<void>Runnable;

}//endof namespace

