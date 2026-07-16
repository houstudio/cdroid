/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * android.os.Message 的 C++ 移植实现 (cdroid::Message)。
 * 蓝本: /opt/android-sdk/sources/android-36/android/os/Message.java
 *********************************************************************************/
#include <core/message.h>
#include <core/handler.h>
#include <core/bundle.h>
#include <porting/cdlog.h>

namespace cdroid{

// 对象池静态成员定义, Message.java:167-170
Message* Message::sPool = nullptr;
int Message::sPoolSize = 0;
std::mutex Message::sPoolSync;

Message::Message(){
}

// Message.java:178-190
Message* Message::obtain(){
    std::lock_guard<std::mutex> lock(sPoolSync);
    if (sPool != nullptr) {
        Message* m = sPool;
        sPool = m->next;
        m->next = nullptr;
        m->flags = 0;
        sPoolSize--;
        return m;
    }
    return new Message();
}

Message* Message::obtain(Message* orig){
    Message* m = obtain();
    m->copyFrom(orig);
    return m;
}

Message* Message::obtain(Handler* h){
    Message* m = obtain();
    m->target = h;
    return m;
}

Message* Message::obtain(Handler* h, Runnable callback){
    Message* m = obtain();
    m->target = h;
    m->callback = callback;
    return m;
}

Message* Message::obtain(Handler* h, int what){
    Message* m = obtain();
    m->target = h;
    m->what = what;
    return m;
}

Message* Message::obtain(Handler* h, int what, void* obj){
    Message* m = obtain();
    m->target = h;
    m->what = what;
    m->obj = obj;
    return m;
}

Message* Message::obtain(Handler* h, int what, int arg1, int arg2){
    Message* m = obtain();
    m->target = h;
    m->what = what;
    m->arg1 = arg1;
    m->arg2 = arg2;
    return m;
}

Message* Message::obtain(Handler* h, int what, int arg1, int arg2, void* obj){
    Message* m = obtain();
    m->target = h;
    m->what = what;
    m->arg1 = arg1;
    m->arg2 = arg2;
    m->obj = obj;
    return m;
}

// Message.java:333-347
void Message::recycle(){
    if (isInUse()) {
        // gCheckRecycle 默认 true; CDROID 无 Java 异常机制, 用 LOGE 替代 (细节取舍)
        LOGE("This message cannot be recycled because it is in use.");
        return;
    }
    recycleUnchecked();
}

// Message.java:349-377
void Message::recycleUnchecked(){
    flags = FLAG_IN_USE;
    what = 0;
    arg1 = 0;
    arg2 = 0;
    obj = nullptr;
    when = 0;
    target = nullptr;
    callback = nullptr;
    data = nullptr;  // 注: Bundle 所有权未定 (CDROID 无 GC), 整合阶段细化
    std::lock_guard<std::mutex> lock(sPoolSync);
    if (sPoolSize < MAX_POOL_SIZE) {
        next = sPool;
        sPool = this;
        sPoolSize++;
    }
}

// Message.java:379-397
void Message::copyFrom(Message* o){
    flags = o->flags & ~FLAGS_TO_CLEAR_ON_COPY_FROM;
    what = o->what;
    arg1 = o->arg1;
    arg2 = o->arg2;
    obj = o->obj;
    // data: Android 用 o.data.clone(); CDROID Bundle 无 clone, 浅拷 (细节取舍)
    data = o->data;
}

int64_t Message::getWhen()const{ return when; }
void Message::setTarget(Handler* t){ target = t; }
Handler* Message::getTarget()const{ return target; }
Runnable Message::getCallback()const{ return callback; }
Message* Message::setCallback(const Runnable& r){ callback = r; return this; }

// Message.java:449-463 (懒构造)
Bundle* Message::getData(){
    if (data == nullptr) data = new Bundle();
    return data;
}
Bundle* Message::peekData()const{ return data; }
void Message::setData(Bundle* d){ data = d; }
Message* Message::setWhat(int w){ what = w; return this; }

// Message.java:493
void Message::sendToTarget(){
    if (target) target->sendMessage(this);
}

// Message.java:505-540
bool Message::isAsynchronous()const{ return (flags & FLAG_ASYNCHRONOUS) != 0; }
void Message::setAsynchronous(bool async){
    if (async) flags |= FLAG_ASYNCHRONOUS;
    else flags &= ~FLAG_ASYNCHRONOUS;
}

// Message.java:543-552
bool Message::isInUse()const{ return (flags & FLAG_IN_USE) == FLAG_IN_USE; }
void Message::markInUse(){ flags |= FLAG_IN_USE; }

}//namespace cdroid
