/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * android.os.Handler 的 C++ 移植实现 (Java 模型, cdroid::MessageQueue 后端)。
 * 蓝本: /opt/android-sdk/sources/android-36/android/os/Handler.java
 *********************************************************************************/
#include <core/handler.h>
#include <core/messagequeue.h>
#include <core/systemclock.h>
#include <porting/cdlog.h>
namespace cdroid{

Handler::Handler():Handler(Looper::getMainLooper()){
}

Handler::Handler(Callback callback)
    :Handler(Looper::getMainLooper(),callback){
}

Handler::Handler(Looper*looper):Handler(looper,nullptr){
}

Handler::Handler(Looper*looper,Callback callback)
    :mLooper(looper)
    ,mQueue(looper?looper->getQueue():nullptr)
    ,mCallback(callback)
    ,mAsynchronous(false){
    if(mLooper)mLooper->addHandler(this);  // 保留: 注册到 mHandlers 使 handleIdle 生效
}

Handler::~Handler(){
    // 先清本 Handler 在 Java MessageQueue 中的待派发消息 (Android 靠 GC, C++ 须显式清, 防 UAF)。
    if(mQueue) mQueue->removeCallbacksAndMessages(this,nullptr);
    if(mLooper){
        mLooper->removeMessages(this);   // libutils mMessageEnvelopes 残留 (apps 经 looper->sendMessage 投递的)
        mLooper->removeHandler(this);
    }
}

// Handler.java:980-992
Message* Handler::getPostMessage(const Runnable& r){
    Message* m = Message::obtain();
    m->callback = r;
    return m;
}

Message* Handler::getPostMessage(const Runnable& r,void*token){
    Message* m = Message::obtain();
    m->obj = token;
    m->callback = r;
    return m;
}

// Handler.java:994-996
void Handler::handleCallback(Message* message){
    if(message->callback) message->callback();
}

void Handler::handleIdle(){
}

// MessageQueue 路径派发 (Handler.java:101-112): 由 Looper::drainMessageQueue 调用, msg 为池化 Message*。
void Handler::dispatchMessage(Message* msg){
    if(msg->callback){
        handleCallback(msg);
    }else{
        if(mCallback){
            if(mCallback(*msg))return;
        }
        handleMessage(*msg);  // apps/消费者 override 的 handleMessage(Message&)
    }
}

// libutils 路径派发: apps 经 looper->sendMessage(this, structMsg) 投递的消息 (mMessageEnvelopes) 由 pollInner 调此。
void Handler::dispatchMessage(Message& msg){
    if(msg.callback){
        msg.callback();
    }else{
        if(mCallback){
            if(mCallback(msg))return;
        }
        handleMessage(msg);  // apps/消费者 override 的 handleMessage(Message&)
    }
}

Looper* Handler::getLooper(){ return mLooper; }
MessageQueue* Handler::getQueue(){ return mQueue; }

// —— has/remove: 委托 MessageQueue (真实现, 替代原 LOGE("TODO")/硬编码) ——
bool Handler::hasMessages(int what){
    return mQueue && mQueue->hasMessages(this,what,nullptr);
}
bool Handler::hasMessages(int what,void*object){
    return mQueue && mQueue->hasMessages(this,what,object);
}
bool Handler::hasCallbacks(const Runnable& r){
    return mQueue && mQueue->hasMessages(this,r,nullptr);
}
void Handler::removeMessages(int what){
    if(mQueue) mQueue->removeMessages(this,what,nullptr);
}
void Handler::removeMessages(int what,void*object){
    if(mQueue) mQueue->removeMessages(this,what,object);
}
void Handler::removeCallbacks(const Runnable& r){
    if(mQueue) mQueue->removeMessages(this,r,nullptr);
}
void Handler::removeCallbacksAndMessages(void*token){
    if(mQueue) mQueue->removeCallbacksAndMessages(this,token);
}

// —— send/post (对齐 Handler.java) ——
bool Handler::sendMessage(Message* msg){
    return sendMessageDelayed(msg,0);
}
bool Handler::sendEmptyMessage(int what){
    return sendEmptyMessageDelayed(what,0);
}
bool Handler::sendEmptyMessageDelayed(int what,long delayMillis){
    Message* msg = Message::obtain(this,what);
    return sendMessageDelayed(msg,delayMillis);
}
bool Handler::sendEmptyMessageAtTime(int what,int64_t uptimeMillis){
    Message* msg = Message::obtain(this,what);
    return sendMessageAtTime(msg,uptimeMillis);
}
bool Handler::sendMessageDelayed(Message* msg,long delayMillis){
    if(delayMillis<0) delayMillis = 0;
    return sendMessageAtTime(msg,SystemClock::uptimeMillis()+delayMillis);
}
bool Handler::sendMessageAtTime(Message* msg,int64_t uptimeMillis){
    if(mQueue==nullptr){
        LOGW("sendMessageAtTime: no mQueue (looper not set)");
        return false;
    }
    return enqueueMessage(mQueue,msg,uptimeMillis);
}
bool Handler::sendMessageAtFrontOfQueue(Message* msg){
    if(mQueue==nullptr){
        LOGW("sendMessageAtFrontOfQueue: no mQueue (looper not set)");
        return false;
    }
    return enqueueMessage(mQueue,msg,0);
}

// Handler.java:782-791
bool Handler::enqueueMessage(MessageQueue* queue,Message* msg,int64_t uptimeMillis){
    msg->target = this;
    if(mAsynchronous) msg->setAsynchronous(true);
    return queue->enqueueMessage(msg,uptimeMillis);
}

// —— obtainMessage: 从对象池取, target=this ——
Message* Handler::obtainMessage(){ return Message::obtain(this); }
Message* Handler::obtainMessage(int what){ return Message::obtain(this,what); }
Message* Handler::obtainMessage(int what,void*obj){ return Message::obtain(this,what,obj); }
Message* Handler::obtainMessage(int what,int arg1,int arg2){ return Message::obtain(this,what,arg1,arg2); }
Message* Handler::obtainMessage(int what,int arg1,int arg2,void*obj){ return Message::obtain(this,what,arg1,arg2,obj); }

bool Handler::post(const Runnable& r){
    return sendMessageDelayed(getPostMessage(r),0);
}
bool Handler::postAtTime(const Runnable& r,int64_t uptimeMillis){
    return sendMessageAtTime(getPostMessage(r),uptimeMillis);
}
bool Handler::postDelayed(const Runnable& r,long delayMillis){
    return sendMessageDelayed(getPostMessage(r),delayMillis);
}

}//namespace cdroid
