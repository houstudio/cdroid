/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * C++ port of android.os.MessageQueue (cdroid::MessageQueue, Legacy path).
 * Reference: /opt/android-sdk/sources/android-36/android/os/MessageQueue.java (Legacy branch only)
 *            /home/git/android_12.0_mid_rkr13/frameworks/base/core/jni/android_os_MessageQueue.cpp
 *********************************************************************************/
#include <core/messagequeue.h>
#include <core/systemclock.h>
#include <porting/cdlog.h>
#include <algorithm>
#include <climits>

namespace cdroid{

// ============================================================================
// Construction / native methods (delegate to cdroid::Looper, aligning with NativeMessageQueue)
// ============================================================================

MessageQueue::MessageQueue(bool quitAllowed, cdroid::Looper* nativeLooper)
    : mQuitAllowed(quitAllowed)
    , mLooper(nullptr){
    nativeInit(nativeLooper);
}

MessageQueue::~MessageQueue(){
    nativeDestroy();
}

// android_os_MessageQueue.cpp:78-85
// nativeLooper: passed in explicitly by the owner (cdroid::Looper) to avoid a TLS timing issue
//               during main-Looper construction; falls back to the thread-local when not supplied
//               (standalone construction, e.g. tests).
void MessageQueue::nativeInit(cdroid::Looper* nativeLooper){
    mLooper = nativeLooper ? nativeLooper : cdroid::Looper::getForThread();
    if (mLooper == nullptr) {
        mLooper = new cdroid::Looper(false);  // allowNonCallbacks = false
        mLooper->setForThread(mLooper);
    }
}

void MessageQueue::nativeDestroy(){
    std::lock_guard<std::recursive_mutex> lock(mLock);
    for (auto& kv : mFileDescriptorRecords) {
        if (mLooper) mLooper->removeFd(kv.first);
        delete kv.second;
    }
    mFileDescriptorRecords.clear();
}

// android_os_MessageQueue.cpp:107-119
void MessageQueue::nativePollOnce(int timeoutMillis){
    if (mLooper) mLooper->pollOnce(timeoutMillis);
}

// android_os_MessageQueue.cpp:121-123
void MessageQueue::nativeWake(){
    if (mLooper) mLooper->wake();
}

// android_os_MessageQueue.cpp:199
bool MessageQueue::nativeIsPolling()const{
    return mLooper && mLooper->isPolling();
}

// android_os_MessageQueue.cpp:125-139
void MessageQueue::nativeSetFileDescriptorEvents(int fd, int events){
    if (!mLooper) return;
    if (events != 0) {
        mLooper->addFd(fd, cdroid::Looper::POLL_CALLBACK, toLooperEvents(events), this, nullptr);
    } else {
        mLooper->removeFd(fd);
    }
}

int MessageQueue::toLooperEvents(int events){
    int looperEvents = 0;
    if (events & OnFileDescriptorEventListener::EVENT_INPUT)  looperEvents |= cdroid::Looper::EVENT_INPUT;
    if (events & OnFileDescriptorEventListener::EVENT_OUTPUT) looperEvents |= cdroid::Looper::EVENT_OUTPUT;
    if (events & OnFileDescriptorEventListener::EVENT_ERROR)  looperEvents |= cdroid::Looper::EVENT_ERROR;
    return looperEvents;
}

// ============================================================================
// Simple queries / IdleHandler, MessageQueue.java:308-386
// ============================================================================

bool MessageQueue::isIdle()const{  // isIdleLegacy :308
    std::lock_guard<std::recursive_mutex> lock(mLock);
    int64_t now = SystemClock::uptimeMillis();
    return mMessages == nullptr || now < mMessages->when;
}

void MessageQueue::addIdleHandler(IdleHandler* handler){  // :352
    std::lock_guard<std::recursive_mutex> lock(mLock);
    if (handler) mIdleHandlers.push_back(handler);
}

void MessageQueue::removeIdleHandler(IdleHandler* handler){  // :382
    std::lock_guard<std::recursive_mutex> lock(mLock);
    mIdleHandlers.erase(std::remove(mIdleHandlers.begin(), mIdleHandlers.end(), handler),
                        mIdleHandlers.end());
}

bool MessageQueue::isPolling()const{  // :413 -> isPollingLocked :421
    std::lock_guard<std::recursive_mutex> lock(mLock);
    return !mQuitting && nativeIsPolling();
}

// ============================================================================
// FileDescriptor monitoring, MessageQueue.java:466-555
// ============================================================================

void MessageQueue::addOnFileDescriptorEventListener(int fd, int events, OnFileDescriptorEventListener* listener){  // :466
    std::lock_guard<std::recursive_mutex> lock(mLock);
    updateOnFileDescriptorEventListenerLocked(fd, events, listener);
}

void MessageQueue::removeOnFileDescriptorEventListener(int fd){  // :508
    std::lock_guard<std::recursive_mutex> lock(mLock);
    updateOnFileDescriptorEventListenerLocked(fd, 0, nullptr);
}

// updateOnFileDescriptorEventListenerLocked :519-555
void MessageQueue::updateOnFileDescriptorEventListenerLocked(int fd, int events, OnFileDescriptorEventListener* listener){
    auto it = mFileDescriptorRecords.find(fd);
    FileDescriptorRecord* record = (it != mFileDescriptorRecords.end()) ? it->second : nullptr;
    if (record != nullptr && record->events == events) {
        return;  // no change
    }
    if (events != 0) {
        events |= OnFileDescriptorEventListener::EVENT_ERROR;  // :537 force ERROR on
        if (record == nullptr) {
            record = new FileDescriptorRecord();
            record->fd = fd;
            mFileDescriptorRecords[fd] = record;
        }
        record->listener = listener;
        record->events = events;
        record->seq += 1;  // generation number
        nativeSetFileDescriptorEvents(fd, events);  // :549
    } else if (record != nullptr) {
        record->events = 0;
        mFileDescriptorRecords.erase(it);
        delete record;
        nativeSetFileDescriptorEvents(fd, 0);
    }
}

// dispatchEvents :559-635 (entered from native handleEvent; CDROID uses handleEvent directly)
int MessageQueue::handleEvent(int fd, int events, void* /*data*/){
    OnFileDescriptorEventListener* listener = nullptr;
    int oldWatchedEvents = 0;
    int seq = 0;
    {
        std::lock_guard<std::recursive_mutex> lock(mLock);
        auto it = mFileDescriptorRecords.find(fd);
        if (it == mFileDescriptorRecords.end()) {
            return 0;  // spurious, no listener
        }
        FileDescriptorRecord* record = it->second;
        oldWatchedEvents = record->events;
        events &= oldWatchedEvents;  // :573 filter by the currently watched set
        if (events == 0) {
            return 0;  // spurious
        }
        listener = record->listener;
        seq = record->seq;
    }
    // Invoke the listener outside the lock.
    int newWatchedEvents = listener ? listener->onFileDescriptorEvents(fd, events) : 0;
    if (newWatchedEvents != 0) {
        newWatchedEvents |= OnFileDescriptorEventListener::EVENT_ERROR;  // :601
    }
    // Update the record if the watched set changed.
    if (newWatchedEvents != oldWatchedEvents) {
        std::lock_guard<std::recursive_mutex> lock(mLock);
        auto it = mFileDescriptorRecords.find(fd);
        if (it != mFileDescriptorRecords.end() && it->second->seq == seq) {
            FileDescriptorRecord* record = it->second;
            if (newWatchedEvents == 0) {
                record->events = 0;
                mFileDescriptorRecords.erase(it);
                delete record;
                nativeSetFileDescriptorEvents(fd, 0);  // removeFd
            } else {
                record->events = newWatchedEvents;
                nativeSetFileDescriptorEvents(fd, newWatchedEvents);  // modify
            }
        }
    }
    return 1;  // non-zero: keep cdroid::Looper from auto-unregistering the fd (MessageQueue owns fd lifetime)
}

// ============================================================================
// Sync barrier, MessageQueue.java:1100-1251
//   A barrier is a Message with target==nullptr; arg1 holds the token.
// ============================================================================

int MessageQueue::postSyncBarrier(){  // postSyncBarrierLegacy :1071 -> postSyncBarrier(when) :1108
    return postSyncBarrier(SystemClock::uptimeMillis());
}

int MessageQueue::postSyncBarrier(int64_t when){  // Legacy branch :1131-1167
    std::lock_guard<std::recursive_mutex> lock(mLock);
    const int token = mNextBarrierToken++;  // :1132 post-increment, first token = 0
    Message* msg = Message::obtain();
    msg->markInUse();
    msg->when = when;
    msg->arg1 = token;        // token stored in arg1
    // target stays nullptr — that is the barrier marker
    Message* prev = nullptr;
    Message* p = mMessages;
    if (when != 0) {
        while (p != nullptr && p->when <= when) {
            prev = p;
            p = p->next;
        }
    }
    if (prev == nullptr) {  // head insert
        if (p == nullptr) mLast = msg;
        msg->next = p;
        mMessages = msg;
    } else {
        msg->next = p;
        prev->next = msg;
        if (p == nullptr) mLast = msg;
    }
    return token;  // do not wake (a barrier is meant to stall)
}

// removeSyncBarrierLegacy :1216-1249
void MessageQueue::removeSyncBarrier(int token){
    bool needWake = false;
    {
        std::lock_guard<std::recursive_mutex> lock(mLock);
        Message* prev = nullptr;
        Message* p = mMessages;
        while (p != nullptr && (p->target != nullptr || p->arg1 != token)) {
            prev = p;
            p = p->next;
        }
        if (p == nullptr) {
            LOGW("removeSyncBarrier: token %d not found", token);  // android throws
            return;
        }
        if (prev != nullptr) {
            prev->next = p->next;
            if (prev->next == nullptr) mLast = prev;
            needWake = false;
        } else {
            mMessages = p->next;
            needWake = (mMessages == nullptr || mMessages->target != nullptr);
            if (mMessages == nullptr) mLast = nullptr;
        }
        p->recycleUnchecked();
    }
    if (needWake && !mQuitting) {
        nativeWake();  // :1247-1249
    }
}

// ============================================================================
// enqueueMessage, MessageQueue.java:1284-1390
// ============================================================================

bool MessageQueue::enqueueMessage(Message* msg, int64_t when){
    if (msg == nullptr) {
        return false;
    }
    if (msg->target == nullptr) {
        // target==null is reserved for barriers; not allowed through enqueueMessage
        LOGE("enqueueMessage: message must have a target");
        return false;
    }
    std::lock_guard<std::recursive_mutex> lock(mLock);
    if (msg->isInUse()) {
        LOGE("enqueueMessage: message already in use");  // android throws :1286
        return false;
    }
    if (mQuitting) {
        LOGW("enqueueMessage: queue quitting, drop message what=%d", msg->what);  // :1291
        msg->recycle();
        return false;
    }
    msg->markInUse();
    msg->when = when;
    bool needWake = false;
    Message* p = mMessages;
    if (p == nullptr || when == 0 || when < p->when) {  // head insert :1302-1311
        msg->next = p;
        mMessages = msg;
        needWake = mBlocked;
        if (p == nullptr) mLast = msg;
    } else {  // middle/tail insert :1312-1377
        needWake = mBlocked && p->target == nullptr && msg->isAsynchronous();  // :1316
        Message* prev = p;
        Message* cur = p->next;
        while (cur != nullptr && when >= cur->when) {
            if (needWake && cur->isAsynchronous()) needWake = false;
            prev = cur;
            cur = cur->next;
        }
        msg->next = cur;
        prev->next = msg;
        if (cur == nullptr) mLast = msg;
    }
    if (msg->isAsynchronous()) mAsyncMessageCount++;  // :1380
    if (needWake) nativeWake();  // :1385
    return true;
}

// ============================================================================
// next, MessageQueue.java:897-1014 (nextLegacy)
// ============================================================================

Message* MessageQueue::next(){
    int nextPollTimeoutMillis = 0;      // :907 do not block on the first round
    for (;;) {
        if (nextPollTimeoutMillis != 0) {
            // android: Binder.flushPendingCommands(); CDROID has no binder, skip
        }
        // pollInner drains due messages and, once the queue is idle, fires IdleHandler
        // (see runIdleHandlers). IdleHandler is owned by Looper::drainMessageQueue rather than
        // inlined here, to avoid double-firing on the pump path.
        nativePollOnce(nextPollTimeoutMillis);  // :913 blocking core
        {
            std::lock_guard<std::recursive_mutex> lock(mLock);  // :915
            if (mQuitting) {  // :963 check quitting after processing messages
                return nullptr;
            }
            const int64_t now = SystemClock::uptimeMillis();  // :917
            Message* prevMsg = nullptr;                        // :918
            Message* msg = mMessages;
            if (msg != nullptr && msg->target == nullptr) {    // :920 barrier
                do {
                    prevMsg = msg;
                    msg = msg->next;
                } while (msg != nullptr && !msg->isAsynchronous());
            }
            if (msg != nullptr) {
                if (now < msg->when) {  // :928 not due yet
                    nextPollTimeoutMillis = (int)std::min<int64_t>(msg->when - now, INT_MAX);
                } else {  // :931 due — unlink and return
                    mBlocked = false;
                    if (prevMsg != nullptr) {  // unlink past a barrier
                        prevMsg->next = msg->next;
                        if (prevMsg->next == nullptr) mLast = prevMsg;
                    } else {  // unlink the head
                        mMessages = msg->next;
                        if (mMessages == nullptr) mLast = nullptr;
                    }
                    msg->next = nullptr;
                    msg->markInUse();
                    if (msg->isAsynchronous()) mAsyncMessageCount--;
                    return msg;
                }
            } else {
                nextPollTimeoutMillis = -1;  // :957 queue empty, block forever until wake
            }
            mBlocked = true;  // about to block: enqueueMessage head-insert wakes based on this
        }
    }
}

// IdleHandler is managed separately: runs the pending IdleHandlers once when the queue is idle
// (no message due now), mirroring the idle segment of MessageQueue.java next() (:971-1012).
// Extracted so Looper::drainMessageQueue — the convergence point of every pump path
// (pollAll/pollOnce/nativePollOnce) — drives it: whether the caller uses next() or nextDue(),
// IdleHandler fires whenever the queue has no message to process.
// Snapshot under the lock, execute outside (same as Android, so an idle callback may touch the
// queue without holding the lock / deadlocking).
void MessageQueue::runIdleHandlers(){
    std::vector<IdleHandler*> snapshot;
    {
        std::lock_guard<std::recursive_mutex> lock(mLock);
        if (mQuitting) return;
        const int64_t now = SystemClock::uptimeMillis();
        if (!(mMessages == nullptr || now < mMessages->when)) return;  // a due message: not idle
        if (mIdleHandlers.empty()) return;
        snapshot = mIdleHandlers;  // snapshot under lock, execute outside
    }
    for (size_t i = 0; i < snapshot.size(); i++) {
        IdleHandler* idler = snapshot[i];
        bool keep = idler->queueIdle();
        if (!keep) {
            std::lock_guard<std::recursive_mutex> lock(mLock);
            mIdleHandlers.erase(std::remove(mIdleHandlers.begin(), mIdleHandlers.end(), idler),
                                mIdleHandlers.end());
        }
    }
}

// ============================================================================
// quit, MessageQueue.java:1029-1064 / 2079-2125
// ============================================================================

void MessageQueue::quit(bool safe){  // :1029
    if (!mQuitAllowed) {
        LOGE("Main thread not allowed to quit.");  // android throws :1030
        return;
    }
    {
        std::lock_guard<std::recursive_mutex> lock(mLock);  // Legacy :1047
        if (mQuitting) return;  // :1049
        mQuitting = true;       // :1052
        if (safe) removeAllFutureMessagesLocked();  // :1054
        else removeAllMessagesLocked();
    }
    nativeWake();  // :1061 wake next() so it observes mQuitting
}

void MessageQueue::removeAllMessagesLocked(){  // :2079
    Message* p = mMessages;
    while (p != nullptr) {
        Message* n = p->next;
        p->recycleUnchecked();
        p = n;
    }
    mMessages = nullptr;
    mLast = nullptr;
    mAsyncMessageCount = 0;
}

void MessageQueue::removeAllFutureMessagesLocked(){  // :2093
    const int64_t now = SystemClock::uptimeMillis();
    Message* p = mMessages;
    if (p == nullptr) return;
    if (p->when > now) {  // :2100 head already in the future, recycle the whole list
        removeAllMessagesLocked();
        return;
    }
    Message* prev = nullptr;
    while (p != nullptr && p->when <= now) {  // :2107 find the first future message
        prev = p;
        p = p->next;
    }
    if (prev != nullptr) {
        prev->next = nullptr;  // :2115 truncate
        mLast = prev;
        while (p != nullptr) {  // recycle everything from p onward
            Message* n = p->next;
            if (p->isAsynchronous()) mAsyncMessageCount--;
            p->recycleUnchecked();
            p = n;
        }
    }
}

// ============================================================================
// Non-blocking pop of a due message (used by cdroid::Looper's periodic pump to drain Java msgs).
// Mirrors the due-message extraction of next() (messagequeue.cc:329-357), including skipping a
// barrier to the first async. Unlike next(): does not block / compute timeout / run IdleHandler;
// returns nullptr when nothing is due.
// ============================================================================

Message* MessageQueue::nextDue(){
    std::lock_guard<std::recursive_mutex> lock(mLock);
    if (mQuitting) return nullptr;
    const int64_t now = SystemClock::uptimeMillis();
    Message* prevMsg = nullptr;
    Message* msg = mMessages;
    if (msg != nullptr && msg->target == nullptr) {  // barrier: skip to the first async
        do {
            prevMsg = msg;
            msg = msg->next;
        } while (msg != nullptr && !msg->isAsynchronous());
    }
    if (msg != nullptr && now >= msg->when) {  // due — unlink
        if (prevMsg != nullptr) {
            prevMsg->next = msg->next;
            if (prevMsg->next == nullptr) mLast = prevMsg;
        } else {
            mMessages = msg->next;
            if (mMessages == nullptr) mLast = nullptr;
        }
        msg->next = nullptr;
        msg->markInUse();
        if (msg->isAsynchronous()) mAsyncMessageCount--;
        return msg;
    }
    return nullptr;  // empty / no async after a barrier / nothing due
}

// ============================================================================
// Package-private removal/query, mirroring the MessageQueue.java Legacy branch
//   hasMessagesLegacy :1528 / :1614 / :1652
//   removeMessagesLegacy :1680 / :1800
//   removeCallbacksAndMessagesLegacy :1949
// CDROID extension: maintains the mLast tail pointer (Java MessageQueue has no mLast).
// ============================================================================

bool MessageQueue::hasMessages(const Handler* h, int what, void* object)const{  // :1541
    if (h == nullptr) return false;
    std::lock_guard<std::recursive_mutex> lock(mLock);
    Message* p = mMessages;
    while (p != nullptr) {
        if (p->target == h && p->what == what && (object == nullptr || p->obj == object)) {
            return true;
        }
        p = p->next;
    }
    return false;
}

bool MessageQueue::hasMessages(const Handler* h,const Runnable& r, void* object)const{  // :1628 (hasCallbacks)
    if (h == nullptr) return false;
    std::lock_guard<std::recursive_mutex> lock(mLock);
    Message* p = mMessages;
    while (p != nullptr) {
        if (p->target == h && p->callback == r && (object == nullptr || p->obj == object)) {
            return true;
        }
        p = p->next;
    }
    return false;
}

void MessageQueue::removeMessages(const Handler* h, int what, void* object){  // :1725 -> :1680
    if (h == nullptr) return;
    std::lock_guard<std::recursive_mutex> lock(mLock);
    Message* p = mMessages;
    while (p != nullptr && p->target == h && p->what == what
           && (object == nullptr || p->obj == object)) {  // contiguous matching head
        Message* n = p->next;
        if (p->isAsynchronous()) mAsyncMessageCount--;
        p->recycleUnchecked();
        p = n;
    }
    mMessages = p;
    if (p == nullptr) mLast = nullptr;
    while (p != nullptr) {  // middle/tail
        Message* n = p->next;
        if (n != nullptr && n->target == h && n->what == what
                && (object == nullptr || n->obj == object)) {
            Message* nn = n->next;
            if (n->isAsynchronous()) mAsyncMessageCount--;
            n->recycleUnchecked();
            p->next = nn;
            if (nn == nullptr) mLast = p;
            continue;
        }
        p = n;
    }
}

void MessageQueue::removeMessages(const Handler* h,const Runnable& r, void* object){  // :1845 -> :1800
    if (h == nullptr || r == nullptr) return;
    std::lock_guard<std::recursive_mutex> lock(mLock);
    Message* p = mMessages;
    while (p != nullptr && p->target == h && p->callback == r
           && (object == nullptr || p->obj == object)) {
        Message* n = p->next;
        if (p->isAsynchronous()) mAsyncMessageCount--;
        p->recycleUnchecked();
        p = n;
    }
    mMessages = p;
    if (p == nullptr) mLast = nullptr;
    while (p != nullptr) {
        Message* n = p->next;
        if (n != nullptr && n->target == h && n->callback == r
                && (object == nullptr || n->obj == object)) {
            Message* nn = n->next;
            if (n->isAsynchronous()) mAsyncMessageCount--;
            n->recycleUnchecked();
            p->next = nn;
            if (nn == nullptr) mLast = p;
            continue;
        }
        p = n;
    }
}

void MessageQueue::removeCallbacksAndMessages(const Handler* h, void* token){  // :1993 -> :1949
    if (h == nullptr) return;
    std::lock_guard<std::recursive_mutex> lock(mLock);
    Message* p = mMessages;
    while (p != nullptr && p->target == h
           && (token == nullptr || p->obj == token)) {
        Message* n = p->next;
        if (p->isAsynchronous()) mAsyncMessageCount--;
        p->recycleUnchecked();
        p = n;
    }
    mMessages = p;
    if (p == nullptr) mLast = nullptr;
    while (p != nullptr) {
        Message* n = p->next;
        if (n != nullptr && n->target == h && (token == nullptr || n->obj == token)) {
            Message* nn = n->next;
            if (n->isAsynchronous()) mAsyncMessageCount--;
            n->recycleUnchecked();
            p->next = nn;
            if (nn == nullptr) mLast = p;
            continue;
        }
        p = n;
    }
}

}//namespace cdroid
