/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
#ifndef __CALLBACK_BASE_H__
#define __CALLBACK_BASE_H__
#include <functional>
#include <type_traits>
#include <atomic>
#include <memory>
#include <core/object.h>

namespace cdroid{

template<typename R,typename... Args>
class CallbackBase{
protected:
    using Functor=std::function<R(Args...)>;
    std::shared_ptr<Functor>mFunctor;
public:
    CallbackBase():mFunctor(std::make_shared<Functor>()){
    }
    CallbackBase(const Functor&a):mFunctor(std::make_shared<Functor>(a)){
    }
    CallbackBase(const CallbackBase&b):mFunctor(b.mFunctor){
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
    CallbackBase&operator=(std::nullptr_t){
        mFunctor= nullptr;
        return *this;
    }
    bool operator == (const CallbackBase&b)const{
        return mFunctor.get() == b.mFunctor.get();
    }
    operator bool()const{ 
        return (mFunctor!=nullptr)&&(*mFunctor!=nullptr);
    }
    bool operator==(std::nullptr_t)const{
        return (mFunctor==nullptr)||(*mFunctor == nullptr);
    }
    bool operator!=(std::nullptr_t)const{
        return (mFunctor!=nullptr)&&(*mFunctor != nullptr);
    }
    virtual R operator()(Args...args){
        if(*this)return (*mFunctor)(std::forward<Args>(args)...);
        return R();
    }
};

class EventSet{
/*The Event(Listener)'s container to hold multi Evnet(Listener)*/
protected:
    std::shared_ptr<void*>mID;
public:
    EventSet(){
        mID = std::make_shared<void*>(this);
    }
    EventSet(const EventSet&other){
        mID = other.mID;
    }
    EventSet& operator=(const EventSet&other){
        mID = other.mID;
        return *this;
    }
    bool operator == (const EventSet&other)const{
        return mID == other.mID;
    }
    bool operator != (const EventSet&other)const{
        return mID != other.mID;
    }
};

using Runnable=CallbackBase<void>;

}//endof namespace
#endif/*__CALLBACK_BASE_H__*/
