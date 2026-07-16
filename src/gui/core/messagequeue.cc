/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * android.os.MessageQueue 的 C++ 移植实现 (cdroid::MessageQueue, Legacy 路径)。
 * 蓝本: /opt/android-sdk/sources/android-36/android/os/MessageQueue.java (仅 *Legacy 分支)
 *        /home/git/android_12.0_mid_rkr13/frameworks/base/core/jni/android_os_MessageQueue.cpp
 *********************************************************************************/
#include <core/messagequeue.h>
#include <core/systemclock.h>
#include <porting/cdlog.h>
#include <algorithm>
#include <climits>

namespace cdroid{

// ============================================================================
// 构造 / native 方法 (委托 cdroid::Looper, 对齐 NativeMessageQueue)
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
// nativeLooper: 由拥有方 (cdroid::Looper) 显式传入, 避免主 Looper 构造期 TLS 时序问题;
//               未传时回退到 thread-local (独立构造场景, 如测试)。
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
// 简单查询 / IdleHandler, MessageQueue.java:308-386
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
// FileDescriptor 监听, MessageQueue.java:466-555
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
        return;  // 无变化
    }
    if (events != 0) {
        events |= OnFileDescriptorEventListener::EVENT_ERROR;  // :537 强制加 ERROR
        if (record == nullptr) {
            record = new FileDescriptorRecord();
            record->fd = fd;
            mFileDescriptorRecords[fd] = record;
        }
        record->listener = listener;
        record->events = events;
        record->seq += 1;  // 代际号
        nativeSetFileDescriptorEvents(fd, events);  // :549
    } else if (record != nullptr) {
        record->events = 0;
        mFileDescriptorRecords.erase(it);
        delete record;
        nativeSetFileDescriptorEvents(fd, 0);
    }
}

// dispatchEvents :559-635 (由 native handleEvent 调入; CDROID 直接用 handleEvent)
int MessageQueue::handleEvent(int fd, int events, void* /*data*/){
    OnFileDescriptorEventListener* listener = nullptr;
    int oldWatchedEvents = 0;
    int seq = 0;
    {
        std::lock_guard<std::recursive_mutex> lock(mLock);
        auto it = mFileDescriptorRecords.find(fd);
        if (it == mFileDescriptorRecords.end()) {
            return 0;  // spurious, 无 listener
        }
        FileDescriptorRecord* record = it->second;
        oldWatchedEvents = record->events;
        events &= oldWatchedEvents;  // :573 按当前监听集过滤
        if (events == 0) {
            return 0;  // spurious
        }
        listener = record->listener;
        seq = record->seq;
    }
    // 锁外调 listener
    int newWatchedEvents = listener ? listener->onFileDescriptorEvents(fd, events) : 0;
    if (newWatchedEvents != 0) {
        newWatchedEvents |= OnFileDescriptorEventListener::EVENT_ERROR;  // :601
    }
    // 变化则更新 record
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
    return 1;  // 非0: 不让 cdroid::Looper 自动注销 fd (fd 生命周期由 MessageQueue 自管)
}

// ============================================================================
// Sync barrier, MessageQueue.java:1100-1251
//   barrier = target==nullptr 的 Message, arg1 存 token
// ============================================================================

int MessageQueue::postSyncBarrier(){  // postSyncBarrierLegacy :1071 -> postSyncBarrier(when) :1108
    return postSyncBarrier(SystemClock::uptimeMillis());
}

int MessageQueue::postSyncBarrier(int64_t when){  // Legacy 分支 :1131-1167
    std::lock_guard<std::recursive_mutex> lock(mLock);
    const int token = mNextBarrierToken++;  // :1132 后增, 首个 token = 0
    Message* msg = Message::obtain();
    msg->markInUse();
    msg->when = when;
    msg->arg1 = token;        // token 存 arg1
    // target 保持 nullptr —— 这就是 barrier 标志
    Message* prev = nullptr;
    Message* p = mMessages;
    if (when != 0) {
        while (p != nullptr && p->when <= when) {
            prev = p;
            p = p->next;
        }
    }
    if (prev == nullptr) {  // 头插
        if (p == nullptr) mLast = msg;
        msg->next = p;
        mMessages = msg;
    } else {
        msg->next = p;
        prev->next = msg;
        if (p == nullptr) mLast = msg;
    }
    return token;  // 不 wake (barrier 目的是 stall)
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
        // target==null 是 barrier 专用, 不允许通过 enqueueMessage 投递
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
    if (p == nullptr || when == 0 || when < p->when) {  // 头插 :1302-1311
        msg->next = p;
        mMessages = msg;
        needWake = mBlocked;
        if (p == nullptr) mLast = msg;
    } else {  // 中/尾插 :1312-1377
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
    int pendingIdleHandlerCount = -1;   // :906 仅首次迭代为 -1
    int nextPollTimeoutMillis = 0;      // :907 首轮不阻塞
    for (;;) {
        if (nextPollTimeoutMillis != 0) {
            // android: Binder.flushPendingCommands(); CDROID 无 binder, 跳过
        }
        nativePollOnce(nextPollTimeoutMillis);  // :913 阻塞核心
        std::vector<IdleHandler*> idleHandlersCopy;
        {
            std::lock_guard<std::recursive_mutex> lock(mLock);  // :915
            if (mQuitting) {  // :963 (消息处理之后再判 quitting)
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
                if (now < msg->when) {  // :928 未到期
                    nextPollTimeoutMillis = (int)std::min<int64_t>(msg->when - now, INT_MAX);
                } else {  // :931 到期, 摘出返回
                    mBlocked = false;
                    if (prevMsg != nullptr) {  // barrier 后摘
                        prevMsg->next = msg->next;
                        if (prevMsg->next == nullptr) mLast = prevMsg;
                    } else {  // 头摘
                        mMessages = msg->next;
                        if (mMessages == nullptr) mLast = nullptr;
                    }
                    msg->next = nullptr;
                    msg->markInUse();
                    if (msg->isAsynchronous()) mAsyncMessageCount--;
                    return msg;
                }
            } else {
                nextPollTimeoutMillis = -1;  // :957 队列空, 永久阻塞等 wake
            }
            // IdleHandler 触发条件 :971
            if (pendingIdleHandlerCount < 0
                && (mMessages == nullptr || now < mMessages->when)) {
                pendingIdleHandlerCount = (int)mIdleHandlers.size();
            }
            if (pendingIdleHandlerCount <= 0) {  // :975 无 idle handler, 继续阻塞
                mBlocked = true;
                continue;
            }
            idleHandlersCopy = mIdleHandlers;  // 锁内拷快照, 锁外执行
        }
        // 锁外执行 idle :989-1005
        for (size_t i = 0; i < idleHandlersCopy.size(); i++) {
            IdleHandler* idler = idleHandlersCopy[i];
            bool keep = idler->queueIdle();
            if (!keep) {
                std::lock_guard<std::recursive_mutex> lock(mLock);
                mIdleHandlers.erase(std::remove(mIdleHandlers.begin(), mIdleHandlers.end(), idler),
                                    mIdleHandlers.end());
            }
        }
        pendingIdleHandlerCount = 0;  // :1008 idle 只在首次迭代跑
        nextPollTimeoutMillis = 0;    // :1012 跑过 idle 回去不阻塞重查
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
    nativeWake();  // :1061 唤醒 next() 让它看到 mQuitting
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
    if (p->when > now) {  // :2100 队首就在未来, 整队回收
        removeAllMessagesLocked();
        return;
    }
    Message* prev = nullptr;
    while (p != nullptr && p->when <= now) {  // :2107 找首个未来消息
        prev = p;
        p = p->next;
    }
    if (prev != nullptr) {
        prev->next = nullptr;  // :2115 截断
        mLast = prev;
        while (p != nullptr) {  // 回收 p 起的所有
            Message* n = p->next;
            if (p->isAsynchronous()) mAsyncMessageCount--;
            p->recycleUnchecked();
            p = n;
        }
    }
}

// ============================================================================
// 非阻塞取到期消息 (供 cdroid::Looper 周期 pump 排空 Java 消息)
// 镜像 next() 的到期提取 (messagequeue.cc:329-357), 含 barrier 跳到 async;
// 与 next() 的区别: 不阻塞/不算 timeout/不跑 IdleHandler, 无到期返回 nullptr。
// ============================================================================

Message* MessageQueue::nextDue(){
    std::lock_guard<std::recursive_mutex> lock(mLock);
    if (mQuitting) return nullptr;
    const int64_t now = SystemClock::uptimeMillis();
    Message* prevMsg = nullptr;
    Message* msg = mMessages;
    if (msg != nullptr && msg->target == nullptr) {  // barrier: 跳到首个 async
        do {
            prevMsg = msg;
            msg = msg->next;
        } while (msg != nullptr && !msg->isAsynchronous());
    }
    if (msg != nullptr && now >= msg->when) {  // 到期, 摘出
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
    return nullptr;  // 队空 / barrier 后无 async / 未到期
}

// ============================================================================
// 包私有 removal/query, 对标 MessageQueue.java Legacy 分支
//   hasMessagesLegacy :1528 / :1614 / :1652
//   removeMessagesLegacy :1680 / :1800
//   removeCallbacksAndMessagesLegacy :1949
// CDROID 扩展: 维护 mLast 尾指针 (Java MessageQueue 无 mLast)。
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
           && (object == nullptr || p->obj == object)) {  // 连续匹配的头部
        Message* n = p->next;
        if (p->isAsynchronous()) mAsyncMessageCount--;
        p->recycleUnchecked();
        p = n;
    }
    mMessages = p;
    if (p == nullptr) mLast = nullptr;
    while (p != nullptr) {  // 中/尾部
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
