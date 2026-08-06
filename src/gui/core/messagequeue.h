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
 * Ported from android.os.MessageQueue (reference: android-36 MessageQueue.java, Legacy path).
 *
 * Single lock (std::recursive_mutex in place of synchronized(this)) + a singly linked list
 * ordered by `when` (mMessages head / mLast tail). The native layer delegates to CDROID's
 * existing cdroid::Looper (epoll/eventfd/addFd), replicating Android's NativeMessageQueue
 * (android_os_MessageQueue.cpp).
 *
 * Owned by cdroid::Looper (getQueue()); cdroid::Handler enqueues/dispatches messages through it,
 * and cdroid::Looper::drainMessageQueue non-blockingly drains due messages during the periodic
 * main-loop pump.
 */
class MessageQueue : public LooperCallback{
public:
    /** IdleHandler, MessageQueue.java:2281-2290 */
    class IdleHandler{
    public:
        virtual ~IdleHandler() = default;
        /** @return true to keep, false to auto-remove */
        virtual bool queueIdle() = 0;
    };

    /** OnFileDescriptorEventListener, MessageQueue.java:2295-2365 */
    class OnFileDescriptorEventListener{
    public:
        static constexpr int EVENT_INPUT = 1 << 0;   // :2312
        static constexpr int EVENT_OUTPUT = 1 << 1;  // :2328
        static constexpr int EVENT_ERROR = 1 << 2;   // :2341
        virtual ~OnFileDescriptorEventListener() = default;
        /** @return the new set of events to watch, 0 to unregister */
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

    // package-private (called by Looper/Handler during integration)
    Message* next();                                  // :1021
    void quit(bool safe);                             // :1029
    bool enqueueMessage(Message* msg, int64_t when);  // :1392

    // package-private removal/query (called by Handler), mirroring the MessageQueue.java Legacy branch.
    // Read-only queries are const (mLock is mutable).
    bool hasMessages(const Handler* h, int what, void* object)const;              // :1541
    bool hasMessages(const Handler* h, const Runnable& r, void* object)const;            // :1628 (hasCallbacks)
    void removeMessages(const Handler* h, int what, void* object);           // :1725
    void removeMessages(const Handler* h, const Runnable& r, void* object);         // :1845 (removeCallbacks)
    void removeCallbacksAndMessages(const Handler* h, void* token);          // :1993

    // Non-blocking: pops one due message (when<=now), skipping past a barrier to the first async;
    // returns nullptr when nothing is due. Used by cdroid::Looper's periodic pump to drain Java
    // messages (does not block / compute timeout).
    Message* nextDue();

    // IdleHandler is managed separately: runs the pending IdleHandlers once when the queue is
    // idle (no message due now), mirroring the idle segment of MessageQueue.java next()
    // (:971-1012). Called from Looper::drainMessageQueue — the single pump choke point reached
    // by every path — so IdleHandler fires regardless of whether the caller drives next() or
    // nextDue(), as long as there is no message to process.
    void runIdleHandlers();

    // cdroid::LooperCallback: native fd event callback entry (corresponds to android dispatchEvents, :559)
    int handleEvent(int fd, int events, void* data) override;

    cdroid::Looper* getLooper() const { return mLooper; }

private:
    /** FileDescriptorRecord, MessageQueue.java:2367-2379 */
    struct FileDescriptorRecord{
        int fd = 0;
        int events = 0;
        OnFileDescriptorEventListener* listener = nullptr;
        int seq = 0;  // generation number, used to validate concurrent updates
    };

    const bool mQuitAllowed;       // :71
    cdroid::Looper* mLooper;       // native Looper (the native Looper held by NativeMessageQueue)
    Message* mMessages = nullptr;  // :82 list head
    Message* mLast = nullptr;      // :84 list tail
    std::vector<IdleHandler*> mIdleHandlers;                                       // :86
    std::unordered_map<int, FileDescriptorRecord*> mFileDescriptorRecords;         // :87
    bool mQuitting = false;        // :89
    bool mBlocked = false;         // :92
    int mAsyncMessageCount = 0;    // :96
    int mNextBarrierToken = 0;     // :2740 (Legacy, initial value 0)
    mutable std::recursive_mutex mLock;    // synchronized(this); mutable: const query methods (isIdle/isPolling/hasMessages) must lock

    // native methods, delegate to cdroid::Looper (corresponds to android_os_MessageQueue.cpp)
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
