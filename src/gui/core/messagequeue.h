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
#ifndef __OS_MESSAGE_QUEUE_H__
#define __OS_MESSAGE_QUEUE_H__
#include <cstdint>
#include <mutex>
#include <vector>
#include <unordered_map>
#include <core/message.h>
#include <core/looper.h>

namespace cdroid{

/**
 * 移植自 android.os.MessageQueue (蓝本: android-36 MessageQueue.java, Legacy 路径)。
 *
 * 单锁 (std::recursive_mutex 替代 synchronized(this)) + 按 when 升序的单链表
 * (mMessages 头 / mLast 尾)。native 层委托 CDROID 现有 cdroid::Looper
 * (epoll/eventfd/addFd), 复刻 Android 的 NativeMessageQueue
 * (android_os_MessageQueue.cpp)。
 *
 * 由 cdroid::Looper 拥有 (getQueue()), cdroid::Handler 经它投递/派发消息;
 * cdroid::Looper::drainMessageQueue 在主循环周期性 pump 里非阻塞排空到期消息。
 */
class MessageQueue : public LooperCallback{
public:
    /** IdleHandler, MessageQueue.java:2281-2290 */
    class IdleHandler{
    public:
        virtual ~IdleHandler() = default;
        /** @return true 保留, false 自动移除 */
        virtual bool queueIdle() = 0;
    };

    /** OnFileDescriptorEventListener, MessageQueue.java:2295-2365 */
    class OnFileDescriptorEventListener{
    public:
        static constexpr int EVENT_INPUT = 1 << 0;   // :2312
        static constexpr int EVENT_OUTPUT = 1 << 1;  // :2328
        static constexpr int EVENT_ERROR = 1 << 2;   // :2341
        virtual ~OnFileDescriptorEventListener() = default;
        /** @return 新的监听事件集, 0 表示注销 */
        virtual int onFileDescriptorEvents(int fd, int events) = 0;  // :2364
    };

    explicit MessageQueue(bool quitAllowed, cdroid::Looper* nativeLooper = nullptr);  // :131
    virtual ~MessageQueue();

    // public API, MessageQueue.java:322-1265
    bool isIdle()const;                                                               // :322
    void addIdleHandler(IdleHandler* handler);                                        // :352
    void removeIdleHandler(IdleHandler* handler);                                     // :382
    bool isPolling()const;                                                            // :413
    void addOnFileDescriptorEventListener(int fd, int events, OnFileDescriptorEventListener* listener);  // :466
    void removeOnFileDescriptorEventListener(int fd);                                 // :508
    int postSyncBarrier();                                                            // :1100
    void removeSyncBarrier(int token);                                                // :1265

    // package-private (整合阶段由 Looper/Handler 调用)
    Message* next();                                  // :1021
    void quit(bool safe);                             // :1029
    bool enqueueMessage(Message* msg, int64_t when);  // :1392

    // package-private removal/query (由 Handler 调用), 对标 MessageQueue.java Legacy 分支。
    // 只读查询 → const (mLock 为 mutable)。
    bool hasMessages(const Handler* h, int what, void* object)const;              // :1541
    bool hasMessages(const Handler* h, const Runnable& r, void* object)const;            // :1628 (hasCallbacks)
    void removeMessages(const Handler* h, int what, void* object);           // :1725
    void removeMessages(const Handler* h, const Runnable& r, void* object);         // :1845 (removeCallbacks)
    void removeCallbacksAndMessages(const Handler* h, void* token);          // :1993

    // 非阻塞: 取出一条到期 (when<=now) 消息, barrier 时跳到 async; 无到期返回 nullptr。
    // 供 cdroid::Looper 的周期 pump 排空 Java 消息 (不阻塞/不算 timeout/不跑 IdleHandler)。
    Message* nextDue();

    // cdroid::LooperCallback: native fd 事件回调入口 (对应 android dispatchEvents, :559)
    int handleEvent(int fd, int events, void* data) override;

    cdroid::Looper* getLooper() const { return mLooper; }

private:
    /** FileDescriptorRecord, MessageQueue.java:2367-2379 */
    struct FileDescriptorRecord{
        int fd = 0;
        int events = 0;
        OnFileDescriptorEventListener* listener = nullptr;
        int seq = 0;  // 代际号, 并发更新校验
    };

    const bool mQuitAllowed;       // :71
    cdroid::Looper* mLooper;       // native Looper (NativeMessageQueue 持有的 native Looper)
    Message* mMessages = nullptr;  // :82 链表头
    Message* mLast = nullptr;      // :84 链表尾
    std::vector<IdleHandler*> mIdleHandlers;                                       // :86
    std::unordered_map<int, FileDescriptorRecord*> mFileDescriptorRecords;         // :87
    bool mQuitting = false;        // :89
    bool mBlocked = false;         // :92
    int mAsyncMessageCount = 0;    // :96
    int mNextBarrierToken = 0;     // :2740 (Legacy, 初值 0)
    mutable std::recursive_mutex mLock;    // synchronized(this); mutable: const 查询方法 (isIdle/isPolling/hasMessages) 须加锁

    // native 方法, 委托 cdroid::Looper (对应 android_os_MessageQueue.cpp)
    void nativeInit(cdroid::Looper* nativeLooper);
    void nativeDestroy();
    void nativePollOnce(int timeoutMillis);
    void nativeWake();
    bool nativeIsPolling()const;
    void nativeSetFileDescriptorEvents(int fd, int events);

    int postSyncBarrier(int64_t when);   // :1108
    void updateOnFileDescriptorEventListenerLocked(int fd, int events, OnFileDescriptorEventListener* listener);  // :519
    void removeAllMessagesLocked();       // :2079
    void removeAllFutureMessagesLocked(); // :2093
    static int toLooperEvents(int events);  // Java EVENT_* -> cdroid::Looper EVENT_*
};

}//namespace cdroid
#endif/*__OS_MESSAGE_QUEUE_H__*/
