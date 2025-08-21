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
#include <core/handler.h>
#include <core/systemclock.h>

namespace cdroid{

Handler::Handler():Handler(nullptr){
}

Handler::Handler(Callback callback)
    :Handler(Looper::getMainLooper(),callback){
}

Handler::Handler(Looper*looper,Callback callback):
    mLooper(looper),mCallback(callback){
    mLooper->addHandler(this);
}

Handler::~Handler(){
    mLooper->removeMessages(this);
    mLooper->removeHandler(this);
}

Message Handler::getPostMessage(const Runnable& r){
    Message m =obtainMessage();
    m.callback = r;
    return m;
}

Message Handler::getPostMessage(const Runnable& r,void*token){
    Message m =obtainMessage();
    m.obj=token;
    m.callback = r;
    return m;

}

void Handler::handleMessage(Message& msg) {
}

void Handler::handleIdle(){
}

bool Handler::hasMessages(int what){
   return mLooper->hasMessages(this,what,nullptr);
}

bool Handler::hasMessages(int what,void*object){
    return mLooper->hasMessages(this,what,object);
}

void Handler::removeMessages(int what){
    mLooper->removeMessages(this,what);
}

void Handler::removeMessages(int what,void*object){
    mLooper->removeMessages(this,what);
}

bool Handler::hasCallbacks(const Runnable& r){
    return true;
}

void Handler::removeCallbacks(const Runnable& r){
    mLooper->removeCallbacks(this,r);
}

void Handler::removeCallbacksAndMessages(void*token){
    LOGE("TODO");
    //mLooper->removeMessages(this,token);
}

Looper* Handler::getLooper(){
    return mLooper;
}

void Handler::handleCallback(Message& message) {
    message.callback();
}

void Handler::dispatchMessage(Message& msg) {
    if (msg.callback != nullptr) {
        handleCallback(msg);
    } else {
        if (mCallback != nullptr) {
            if (mCallback(msg)){//mCallbackack.handleMessage(msg)) {
                return;
            }
        }
        handleMessage(msg);
    }
}

Message Handler::obtainMessage(){
    return obtainMessage(0);
}

Message Handler::obtainMessage(int what){
    return obtainMessage(what,0,0); 
}

Message Handler::obtainMessage(int what,int arg1,int arg2){
    return obtainMessage(what,arg1,arg2,nullptr);
}

Message Handler::obtainMessage(int what,int arg1,int arg2,void*obj){
    Message m;
    m.what=what;
    m.arg1=arg1;
    m.arg2=arg2;
    m.obj =obj;
    m.target=this;
    return m;
}

bool Handler::sendMessage(const Message& msg){
    return sendMessageDelayed(msg, 0);
}

bool Handler::sendEmptyMessage(int what){
    return sendEmptyMessageDelayed(what, 0);
}

bool Handler::sendEmptyMessageDelayed(int what, long delayMillis) {
    Message msg = obtainMessage(what);
    return sendMessageDelayed(msg, delayMillis);
}

bool Handler::sendEmptyMessageAtTime(int what, int64_t uptimeMillis) {
    Message msg = obtainMessage(what);
    return sendMessageAtTime(msg, uptimeMillis);
}

bool Handler::sendMessageDelayed(const Message& msg, long delayMillis){
    if (delayMillis < 0) {
        delayMillis = 0;
    }
    return sendMessageAtTime(msg, SystemClock::uptimeMillis() + delayMillis);
}

bool Handler::sendMessageAtTime(const Message& msg, int64_t uptimeMillis) {
    mLooper->sendMessageAtTime(uptimeMillis,this,msg);
    return true;
}

bool Handler::post(const Runnable& r){
    Message msg = getPostMessage(r);
    return sendMessageDelayed(msg, 0);
}

bool Handler::postAtTime(const Runnable& r, int64_t uptimeMillis){
    Message msg = getPostMessage(r);
    return sendMessageAtTime(msg, uptimeMillis);
}

bool Handler::postDelayed(const Runnable& r, long delayMillis){
    Message msg = getPostMessage(r);
    return sendMessageDelayed(msg, delayMillis);
}

}
