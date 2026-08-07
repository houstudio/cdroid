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
class Bundle;  // cdroid::Bundle (core/bundle.h), lazily constructed in getData()
class Handler; // cdroid::Handler (Java model); Message.target points at it

/**
 * Ported from android.os.Message (reference: android-36 Message.java).
 *
 * Reference semantics: obtain() from the pool, recycle() to return it. **Do not stack-construct
 * or value-copy** — the next pointer + object pool are incompatible with value semantics.
 *
 * Unified type: cdroid::Message is the pooled Message (the transitional cdroid::os::Message was
 * renamed here, and the old struct cdroid::Message in looper.h was removed/merged).
 * Handler/Looper/MessageQueue all use it uniformly.
 *
 * Omissions (deliberate trade-offs): Android's tracing/permission fields mEventId /
 * mSendingThreadName / sendingUid / workSourceUid / replyTo (Messenger) are not ported (they need
 * system-server/binder).
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
    int64_t when = 0;           // :146  (time base: SystemClock::uptimeMillis)
    Handler* target = nullptr;  // :155
    Runnable callback;          // :158  (cdroid::Runnable)
    Bundle* data = nullptr;     // :152
    Message* next = nullptr;    // :162  (object-pool list / MessageQueue scheduling list)

    Message();  // :554

    // obtain overloads, Message.java:178-316 — all take from the pool, returning an owning raw pointer
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

    int64_t getWhen()const;                 // :399
    void setTarget(Handler* target);   // :403
    Handler* getTarget()const;         // :415
    Runnable getCallback()const;            // :427
    Message* setCallback(const Runnable& r);  // :433 (@hide)
    Bundle* getData();                 // :449 (lazily constructed, mutates self -> non-const)
    Bundle* peekData()const;           // :465
    void setData(Bundle* data);        // :475
    Message* setWhat(int what);        // :484 (@hide)
    void sendToTarget();               // :493 (passes this to sendMessage(Message*) -> non-const)

    bool isAsynchronous()const;        // :505
    void setAsynchronous(bool async);  // :535
    bool isInUse()const;          // :543
    void markInUse();             // :548

private:
    // Object pool, Message.java:166-172 (synchronized(sPoolSync) -> std::mutex)
    static Message* sPool;
    static int sPoolSize;
    static std::mutex sPoolSync;
    static constexpr int MAX_POOL_SIZE = 50;  // :170
};

}//namespace cdroid
#endif/*__OS_MESSAGE_H__*/
