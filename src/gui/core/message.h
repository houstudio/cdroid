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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02101-1301  USA
 *********************************************************************************/
#ifndef __OS_MESSAGE_H__
#define __OS_MESSAGE_H__
#include <cstdint>
#include <mutex>
#include <core/callbackbase.h>

namespace cdroid{
class Bundle;  // cdroid::Bundle (core/bundle.h), 懒构造于 getData()
class Handler; // cdroid::Handler (Java 模型), Message.target 指向它

/**
 * 移植自 android.os.Message (蓝本: android-36 Message.java)。
 *
 * 引用语义: 通过 obtain() 从对象池获取、recycle() 归还。**不要栈构造或值拷贝** ——
 * next 指针 + 对象池与值语义不兼容。
 *
 * 统一类型: cdroid::Message 即池化 Message (原过渡期的 cdroid::os::Message 已正名至此,
 * looper.h 中的旧 struct cdroid::Message 已删除合并)。Handler/Looper/MessageQueue 统一用它。
 *
 * 省略项 (细节取舍): Android 的 tracing/权限字段 mEventId / mSendingThreadName /
 * sendingUid / workSourceUid / replyTo(Messenger) 未移植 (依赖 system-server/binder)。
 */
class Message{
public:
    // Message.java:128-134
    static constexpr int FLAG_IN_USE = 1 << 0;
    static constexpr int FLAG_ASYNCHRONOUS = 1 << 1;
    static constexpr int FLAGS_TO_CLEAR_ON_COPY_FROM = FLAG_IN_USE;

    int what = 0;               // Message.java:57
    int arg1 = 0;               // :64
    int arg2 = 0;               // :71
    void* obj = nullptr;        // :83
    int flags = 0;              // :137
    int64_t when = 0;           // :146  (时基 SystemClock::uptimeMillis)
    Handler* target = nullptr;  // :155
    Runnable callback;          // :158  (cdroid::Runnable)
    Bundle* data = nullptr;     // :152
    Message* next = nullptr;    // :162  (对象池链表 / MessageQueue 调度链表)

    Message();  // :554

    // obtain 重载, Message.java:178-316 —— 全部从对象池取, 返回所有权裸指针
    static Message* obtain();
    static Message* obtain(Message* orig);
    static Message* obtain(Handler* h);
    static Message* obtain(Handler* h, Runnable callback);
    static Message* obtain(Handler* h, int what);
    static Message* obtain(Handler* h, int what, void* obj);
    static Message* obtain(Handler* h, int what, int arg1, int arg2);
    static Message* obtain(Handler* h, int what, int arg1, int arg2, void* obj);

    void recycle();             // :333
    void recycleUnchecked();    // :349

    void copyFrom(Message* o);  // :379 (Android: copyFrom(Message o))

    int64_t getWhen();                 // :399
    void setTarget(Handler* target);   // :403
    Handler* getTarget();              // :415
    Runnable getCallback();            // :427
    Message* setCallback(Runnable r);  // :433 (@hide)
    Bundle* getData();                 // :449 (懒构造)
    Bundle* peekData();                // :465
    void setData(Bundle* data);        // :475
    Message* setWhat(int what);        // :484 (@hide)
    void sendToTarget();               // :493

    bool isAsynchronous();        // :505
    void setAsynchronous(bool async);  // :535
    bool isInUse();               // :543
    void markInUse();             // :548

private:
    // 对象池, Message.java:166-172 (synchronized(sPoolSync) → std::mutex)
    static Message* sPool;
    static int sPoolSize;
    static std::mutex sPoolSync;
    static constexpr int MAX_POOL_SIZE = 50;  // :170
};

}//namespace cdroid
#endif/*__OS_MESSAGE_H__*/
