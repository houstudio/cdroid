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
#define _CRT_SECURE_NO_WARNINGS
#include <cstring>
#include <fcntl.h>
#include <pthread.h>
#include <core/looper.h>
#include <core/systemclock.h>
#include <core/epollwrapper.h>
#include <porting/cdlog.h>
#if defined(_WIN32)||defined(_WIN64)||defined(_MSVC_VER)
  #include <io.h>
  #define close  _close
  #define read  _read
  #define write  _write
  static const char* strerror_r(int errnum, char* buf, size_t buflen) {
      strerror_s(buf, buflen, errnum);
      return buf;
  }
#elif defined(__Linux__)||defined(__unix__)
  #include <unistd.h>
  #include <sys/eventfd.h>
#endif
#include <limits.h>

#define DEBUG_POLL_AND_WAKE 0
#define DEBUG_CALLBACKS 0

/*REF:http://androidxref.com/9.0.0_r3/xref/system/core/libutils/Looper.cpp*/
namespace cdroid{

// Maximum number of file descriptors for which to retrieve poll events each iteration.
static constexpr int EPOLL_MAX_EVENTS = 16;
static pthread_once_t gTLSOnce = PTHREAD_ONCE_INIT;
static pthread_key_t  gTLSKey = 0;

static int toMillisecondTimeoutDelay(nsecs_t referenceTime, nsecs_t timeoutTime){
    nsecs_t timeoutDelayMillis;
    if (timeoutTime > referenceTime) {
        uint64_t timeoutDelay = uint64_t(timeoutTime - referenceTime);
        if (timeoutDelay > uint64_t(INT_MAX - 1)) {
            timeoutDelayMillis = -1;
        } else {
            timeoutDelayMillis = timeoutDelay;
        }
    } else {
        timeoutDelayMillis = 0;
    }
    return (int)timeoutDelayMillis;
}

namespace {
    constexpr uint64_t WAKE_EVENT_FD_SEQ = 1;
    epoll_event createEpollEvent(uint32_t events, uint64_t seq) {
        epoll_event e={events};
        e.data.u64 = seq;
        return e;
    }
} 

Looper* Looper::sMainLooper;
Looper::Looper(bool allowNonCallbacks) :
        mAllowNonCallbacks(allowNonCallbacks),
        mSendingMessage(false),mPolling(false),
        mEpollRebuildRequired(false),
        mWakeEventFd(-1),mEpoll(nullptr),
        mNextRequestSeq(WAKE_EVENT_FD_SEQ+1),
        mResponseIndex(0), mNextMessageUptime(LLONG_MAX) {
#if defined(HAVE_EVENTFD)
    mWakeEventFd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    LOGE_IF(mWakeEventFd < 0, "Could not make wake event fd: %s",strerror(errno));
#endif
    std::lock_guard<std::recursive_mutex>_l(mLock);
    rebuildEpollLocked();
}

Looper::~Looper() {
    close(mWakeEventFd);
    mWakeEventFd = -1;
    delete mEpoll;
    for(EventHandler*hdl:mEventHandlers){
        if((hdl->mFlags&2)==2)delete hdl;
    }
    mEventHandlers.clear();
}

void Looper::initTLSKey() {
    const int error = pthread_key_create(&gTLSKey, threadDestructor);
    LOGE_IF(error != 0, "Could not allocate TLS key: %s", strerror(error));
}

void Looper::threadDestructor(void *st) {
    Looper* const self = static_cast<Looper*>(st);
    if (self != nullptr) {
        //self->decStrong((void*)threadDestructor);
	    delete self;
    }
}

Looper*Looper::getDefault(){
    return getMainLooper();
}

Looper*Looper::getMainLooper(){
    return sMainLooper;
}

Looper*Looper::myLooper(){
    return getForThread();
}

Looper*Looper::prepare(int opts){
    Looper* looper = getForThread();
    const bool allowNonCallbacks = opts & PREPARE_ALLOW_NON_CALLBACKS;
    if(looper==nullptr){
        looper = new Looper(allowNonCallbacks);
        setForThread(looper);
    }
    LOGW_IF(looper->getAllowNonCallbacks()!=allowNonCallbacks,"Looper already prepared for this thread with a different"
	    " value for the LOOPER_PREPARE_ALLOW_NON_CALLBACKS option.");
    return looper;
}

void Looper::prepareMainLooper(){
    prepare(false);
    FATAL_IF(sMainLooper,"The main Looper has been prepared");
    sMainLooper = myLooper();
}

void Looper::setForThread(Looper* looper){
    Looper*old = getForThread();
    delete old;
    pthread_setspecific(gTLSKey,looper);
}

Looper*Looper::getForThread(){
   const int result = pthread_once(&gTLSOnce,initTLSKey);
   LOGW_IF(result != 0,"pthread_once failed");
   Looper*looper = static_cast<Looper*>(pthread_getspecific(gTLSKey));
   return looper;
}

bool Looper::getAllowNonCallbacks() const {
    return mAllowNonCallbacks;
}

void Looper::rebuildEpollLocked() {
    // Close old epoll instance if we have one.
    if (mEpoll != nullptr) {
        LOGV("%p ~ rebuildEpollLocked - rebuilding epoll set", this);
        delete mEpoll;
        mEpoll = nullptr;
    }

    // Allocate the new epoll instance and register the wake pipe.
    //mEpollFd = epoll_create(EPOLL_CLOEXEC);
    mEpoll = IOEventProcessor::create();
    LOGE_IF(mEpoll ==nullptr, "Could not create epoll instance: %s", strerror(errno));
#if defined(HAVE_EVENTFD)
    struct epoll_event wakeEvent = createEpollEvent(EPOLLIN,WAKE_EVENT_FD_SEQ);
    int result = mEpoll->addFd(mWakeEventFd,wakeEvent);//epoll_ctl(mEpollFd, EPOLL_CTL_ADD, mWakeEventFd, &wakeEvent);
    LOGE_IF(result != 0, "Could not add wake event fd to epoll instance: %s",strerror(errno));
#endif
    for (auto it=mRequests.begin();it!=mRequests.end(); it++) {
        const SequenceNumber& seq = it->first;
        const Request& request = it->second;
        epoll_event eventItem = createEpollEvent(request.events,seq);
        const int epollResult = mEpoll->addFd(request.fd,eventItem);//epoll_ctl(mEpollFd, EPOLL_CTL_ADD, request.fd, & eventItem);
        LOGE_IF(epollResult<0,"Error adding epoll events for fd %d while rebuilding epoll set: %s",request.fd, strerror(errno));
    }
}

void Looper::scheduleEpollRebuildLocked() {
    if (!mEpollRebuildRequired) {
#if DEBUG_CALLBACKS
        LOGD("%p scheduleEpollRebuildLocked - scheduling epoll set rebuild", this);
#endif
        mEpollRebuildRequired = true;
        wake();
    }
}

int Looper::pollOnce(int timeoutMillis, int* outFd, int* outEvents, void** outData) {
    int result = 0;
    for (;;) {
        while (mResponseIndex < mResponses.size()) {
            const Response& response = mResponses.at(mResponseIndex++);
            int ident = response.request.ident;
            if (ident >= 0) {
                int fd = response.request.fd;
                int events = response.events;
                void* data = response.request.data;
#if DEBUG_POLL_AND_WAKE
                LOGD("%p returning signalled identifier %d: fd=%d, events=0x%x, data=%p", this, ident, fd, events, data);
#endif
                if (outFd != nullptr) *outFd = fd;
                if (outEvents != nullptr) *outEvents = events;
                if (outData != nullptr) *outData = data;
                return ident;
            }
        }
        if (result != 0) {
#if DEBUG_POLL_AND_WAKE
            LOGD("%p returning result %d", this, result);
#endif
            if (outFd != nullptr) *outFd = 0;
            if (outEvents != nullptr) *outEvents = 0;
            if (outData != nullptr) *outData = nullptr;
            return result;
        }
        result = pollInner(timeoutMillis);
    }
}

int Looper::doEventHandlers(){
    int count = 0;
    if(mNextMessageUptime>(nsecs_t)SystemClock::uptimeMillis()){
        for(auto it = mHandlers.begin();it != mHandlers.end();){
            MessageHandler*hdl = (*it);
            uint32_t eFlags = (*it)->mFlags;
            if((eFlags&1)==0){
                hdl->handleIdle(); count++;
                eFlags = hdl->mFlags;
            }
            if(eFlags&2) delete hdl;
            if(eFlags&1){
                it = mHandlers.erase(it);
                continue;
            }
            it++;
        }        
    }
    for(auto it=mEventHandlers.begin();it!=mEventHandlers.end();){
        EventHandler*es=(*it);
        uint32_t eFlags = es->mFlags;
        if(es&&((eFlags&1)==0)){
            if(es->checkEvents()>0){
                es->handleEvents();  count++;
            }
            eFlags = es->mFlags;//Maybe EventHandler::handleEvents will remove itself,so we recheck the flags
        }
        if((eFlags&3)==3) delete es;//EventHandler owned by looper must be freed here
        if((eFlags&1)==1){
            it = mEventHandlers.erase(it);
            continue;
        }
        it++;
    }
    return count;
}


int Looper::pollInner(int timeoutMillis) {
#if DEBUG_POLL_AND_WAKE
    LOGD("%p waiting: timeoutMillis=%d mNextMessageUptime=%lld/%lld",this,timeoutMillis,mNextMessageUptime,LLONG_MAX);
#endif
    // Adjust the timeout based on when the next message is due.
    if (timeoutMillis != 0 && mNextMessageUptime != LLONG_MAX) {
        nsecs_t now = SystemClock::uptimeMillis();
        int messageTimeoutMillis = toMillisecondTimeoutDelay(now, mNextMessageUptime);
        if ( (messageTimeoutMillis >= 0 ) && (timeoutMillis < 0 || messageTimeoutMillis < timeoutMillis)) {
            timeoutMillis = messageTimeoutMillis;
        }
#if DEBUG_POLL_AND_WAKE
        LOGD("%p next message in %lld ns, adjusted timeout:timeoutMillis=%d",this,mNextMessageUptime - now, timeoutMillis);
#endif
    }

    //Poll
    int result = POLL_WAKE;
    mResponses.clear();
    mResponseIndex =0;
    //We are about to idle
    mPolling = true;
    std::vector<struct epoll_event> eventItems;
    const int eventCount = mEpoll->waitEvents(eventItems,timeoutMillis);// epoll_wait(mEpollFd, eventItems, EPOLL_MAX_EVENTS, timeoutMillis);
    //No longer idling.
    mPolling = false;
    // Acquire lock.
    mLock.lock();

    //Rebuild epoll set if needed.
    if(mEpollRebuildRequired){
        mEpollRebuildRequired = false;
        rebuildEpollLocked();
        goto Done;
    }
    //Check fore -poll error
    if(eventCount<0){
        if(errno==EINTR)goto Done;
        LOGW("Poll failed with an unexpected error: %s", strerror(errno));
        result = POLL_ERROR;
        goto Done;
    }
    //Check for poll timeout
    if(eventCount==0){
#if DEBUG_POLL_AND_WAKE
        LOGD("%p pollOnce - timeout",this);
#endif
        result = POLL_TIMEOUT;
        goto Done;
    }
    // Handle all events.
#if DEBUG_POLL_AND_WAKE
    LOGD("%p pollOnce - handling events from %d fds", this, eventCount);
#endif
    for (int i = 0; i < eventCount; i++) {
        const SequenceNumber seq = eventItems[i].data.u64;
        const uint32_t epollEvents = eventItems[i].events;
        if (seq == WAKE_EVENT_FD_SEQ) {
            if (epollEvents & EVENT_INPUT) {
                awoken();
            } else {
                LOGW("Ignoring unexpected epoll events 0x%x on wake event fd.", epollEvents);
            }
        } else {
            const auto& request_it = mRequests.find(seq);
            if (request_it != mRequests.end()) {
                const auto& request = request_it->second;
                mResponses.push_back({ seq,int(epollEvents),request });
            } else {
                LOGW("Ignoring unexpected epoll events 0x%x for sequence number %lld"
                      " that is no longer registered.",epollEvents, seq);
            }
        }
    }
Done:
    // Invoke pending message callbacks.
    mNextMessageUptime = LLONG_MAX;
    while (mMessageEnvelopes.size() != 0) {
        nsecs_t now = SystemClock::uptimeMillis();
        const MessageEnvelope& messageEnvelope = mMessageEnvelopes.front();
        if (messageEnvelope.uptime <= now) {
            //Remove the envelope from the list. We keep a strong reference to the handler
            //until the call to handleMessage finishes. Then we drop it so that the handler
            //can be deleted *before* we reacquire our lock.
            {//obtain handler
                MessageHandler* handler = messageEnvelope.handler;
                Message message = messageEnvelope.message;
                mMessageEnvelopes.pop_front();
                mSendingMessage = true;
                mLock.unlock();
#if DEBUG_POLL_AND_WAKE||DEBUG_CALLBACKS
                LOGD("%psending message: handler=%p, what=%d",this, handler, message.what);
#endif
                handler->dispatchMessage(message);
            }//release handler

            mLock.lock();
            mSendingMessage = false;
            result = POLL_CALLBACK;
        } else {
            // The last message left at the head of the queue determines the next wakeup time.
            mNextMessageUptime = messageEnvelope.uptime;
            break;
        }
    }
    //Release Lock.
    mLock.unlock();
    //EventHandlers;
    doEventHandlers();
    //Invoke all response callbacks.
    for (size_t i = 0; i < mResponses.size(); i++) {
        Response& response = mResponses.at(i);//editItemAt(i);
        if (response.request.ident == POLL_CALLBACK) {
            const int fd = response.request.fd;
            const int events = response.events;
            void* data = response.request.data;
#if DEBUG_POLL_AND_WAKE || DEBUG_CALLBACKS
            LOGD("%p ~ pollOnce - invoking fd event callback %p: fd=%d, events=0x%x, data=%p",
                    this, response.request.callback, fd, events, data);
#endif
            // Invoke the callback.  Note that the file descriptor may be closed by
            // the callback (and potentially even reused) before the function returns so
            // we need to be a little careful when removing the file descriptor afterwards.
            int callbackResult =0;
            if(response.request.callback1)
                callbackResult=response.request.callback1->handleEvent(fd, events, data);
            else
                callbackResult=response.request.callback2(fd, events, data);
            if (callbackResult == 0) {
                std::lock_guard<std::recursive_mutex> _l(mLock);
                removeSequenceNumberLocked(response.seq);
            }

            // Clear the callback reference in the response structure promptly because we
            // will not clear the response vector itself until the next poll.
            response.request.callback1 = nullptr;//clear();
            response.request.callback2 = nullptr;
            result = POLL_CALLBACK;
        }
    }
    return result;
}

int Looper::pollAll(int timeoutMillis, int* outFd, int* outEvents, void** outData) {
    if (timeoutMillis <= 0) {
        int result;
        do {
            result = pollOnce(timeoutMillis, outFd, outEvents, outData);
        } while (result == POLL_CALLBACK);
        return result;
    } else {
        nsecs_t endTime = SystemClock::uptimeMillis() + timeoutMillis;
        for (;;) {
            int result = pollOnce(timeoutMillis, outFd, outEvents, outData);
            if (result != POLL_CALLBACK) {
                return result;
            }

            nsecs_t now = SystemClock::uptimeMillis();
            timeoutMillis = toMillisecondTimeoutDelay(now, endTime);
            if (timeoutMillis == 0) {
                return POLL_TIMEOUT;
            }
        }
    }
}

void Looper::wake() {
#if defined(HAVE_EVENTFD)
    LOGD_IF(DEBUG_POLL_AND_WAKE,"%p  wake", this);
    uint64_t inc = 1;
    long nWrite;
    do {
        nWrite = write(mWakeEventFd, &inc, sizeof(uint64_t));
    } while ((nWrite == -1) && (errno == EINTR));
    if (nWrite != sizeof(uint64_t)) {
        char buff[128];
        LOGE_IF(errno!=EAGAIN,"Could not write wake signal to fd %d: %s",mWakeEventFd, strerror_r(errno,buff,sizeof(buff)));
    }
#endif
}

void Looper::awoken() {
    LOGD_IF(DEBUG_POLL_AND_WAKE,"%p  awoken", this);
    uint64_t counter;
    long result;
    do {
        result = read(mWakeEventFd, &counter, sizeof(uint64_t));
    } while ((result == -1) && (errno == EINTR));
}

void Looper::pushResponse(int events, const Request& request) {
    Response response;
    response.events = events;
    response.request = request;
    mResponses.push_back(response);
}

int Looper::addFd(int fd, int ident, int events, Looper_callbackFunc callback, void* data) {
    return addFd(fd, ident, events, nullptr,callback, data);
}

int Looper::addFd(int fd, int ident, int events,const LooperCallback* callback,void* data){
    return addFd(fd, ident, events, callback,nullptr, data); 
}

int Looper::addFd(int fd, int ident, int events,const LooperCallback* callback1,Looper_callbackFunc callback2, void* data) {
#if DEBUG_CALLBACKS
    LOGD("%p  addFd - fd=%d, ident=%d, events=0x%x, callback=%p, data=%p", this, fd, ident, events, callback, data);
#endif
    if ((callback1 == nullptr)&&(callback2==nullptr)) {
        if (! mAllowNonCallbacks) {
            LOGE("Invalid attempt to set NULL callback but not allowed for this looper.");
            return -1;
        }

        if (ident < 0) {
            LOGE("Invalid attempt to set NULL callback with ident < 0.");
            return -1;
        }
    } else {
        ident = POLL_CALLBACK;
    }

    { // acquire lock
        std::lock_guard<std::recursive_mutex> _l(mLock);
        if (mNextRequestSeq == WAKE_EVENT_FD_SEQ) mNextRequestSeq++;
        const SequenceNumber seq = mNextRequestSeq++;

        Request request;
        request.fd = fd;
        request.ident = ident;
        request.events = events;
        request.callback1 = (LooperCallback*)callback1;
        request.callback2 = callback2;
        request.data = data;
        if (mNextRequestSeq == -1) mNextRequestSeq = 0; // reserve sequence number -1

        epoll_event eventItem = createEpollEvent(request.events,seq);
        auto seq_it = mSequenceNumberByFd.find(fd);
        if (seq_it == mSequenceNumberByFd.end()) {
            const int epollResult = mEpoll->addFd(fd,eventItem);
            if (epollResult < 0) {
                LOGE("Error adding epoll events for fd %d: %s", fd, strerror(errno));
                return -1;
            }
            mRequests.emplace(seq, request);
            mSequenceNumberByFd.emplace(fd,seq);
        } else {
            int epollResult = mEpoll->modifyFd(fd,eventItem);
            if (epollResult < 0) {
                if (errno == ENOENT) {
                    // Tolerate ENOENT because it means that an older file descriptor was
                    // closed before its callback was unregistered and meanwhile a new
                    // file descriptor with the same number has been created and is now
                    // being registered for the first time.  This error may occur naturally
                    // when a callback has the side-effect of closing the file descriptor
                    // before returning and unregistering itself.  Callback sequence number
                    // checks further ensure that the race is benign.
                    //
                    // Unfortunately due to kernel limitations we need to rebuild the epoll
                    // set from scratch because it may contain an old file handle that we are
                    // now unable to remove since its file descriptor is no longer valid.
                    // No such problem would have occurred if we were using the poll system
                    // call instead, but that approach carries others disadvantages.
#if DEBUG_CALLBACKS
                    LOGD("%p  addFd - EPOLL_CTL_MOD failed due to file descriptor "
                            "being recycled, falling back on EPOLL_CTL_ADD: %s", this, strerror(errno));
#endif
                    epollResult = mEpoll->addFd(fd,eventItem);
                    if (epollResult < 0) {
                        LOGE("Error modifying or adding epoll events for fd %d: %s", fd, strerror(errno));
                        return -1;
                    }
                    scheduleEpollRebuildLocked();
                } else {
                    LOGE("Error modifying epoll events for fd %d: %s", fd, strerror(errno));
                    return -1;
                }
            }
            const SequenceNumber oldSeq = seq_it->second;
            mRequests.erase(oldSeq);
            mRequests.emplace(seq,request);
            seq_it->second = seq;
        }
    } // release lock
    return 1;
}

int Looper::removeFd(int fd) {
    std::lock_guard<std::recursive_mutex> _l(mLock);
    const auto it = mSequenceNumberByFd.find(fd);
    if( it == mSequenceNumberByFd.end() ) return 0;
    return removeSequenceNumberLocked(it->second);
}

int Looper::removeSequenceNumberLocked(SequenceNumber seq){
    const auto& request_it = mRequests.find(seq);
    if (request_it == mRequests.end()) {
        return 0;
    }
    const int fd = request_it->second.fd;
    LOGD_IF(DEBUG_CALLBACKS,"%p ~ removeFd - fd=%d, seq=%u", this, fd, seq);

    // Always remove the FD from the request map even if an error occurs while
    // updating the epoll set so that we avoid accidentally leaking callbacks.
    mRequests.erase(request_it);
    mSequenceNumberByFd.erase(fd);

    int epollResult = mEpoll->removeFd(fd);// epoll_ctl(mEpollFd, EPOLL_CTL_DEL, fd, nullptr);
    if (epollResult < 0) {
        if (errno == EBADF || errno == ENOENT) {
            // Tolerate EBADF or ENOENT because it means that the file descriptor was closed
            // before its callback was unregistered. This error may occur naturally when a
            // callback has the side-effect of closing the file descriptor before returning and
            // unregistering itself.
            //
            // Unfortunately due to kernel limitations we need to rebuild the epoll
            // set from scratch because it may contain an old file handle that we are
            // now unable to remove since its file descriptor is no longer valid.
            // No such problem would have occurred if we were using the poll system
            // call instead, but that approach carries other disadvantages.
#if DEBUG_CALLBACKS
            LOGD("%p ~ removeFd - EPOLL_CTL_DEL failed due to file descriptor "
                  "being closed: %s",this, strerror(errno));
#endif
            scheduleEpollRebuildLocked();
        } else {
            // Some other error occurred.  This is really weird because it means
            // our list of callbacks got out of sync with the epoll set somehow.
            // We defensively rebuild the epoll set to avoid getting spurious
            // notifications with nowhere to go.
            char buff[128];
            LOGE("Error removing epoll events for fd %d: %s", fd, strerror_r(errno,buff,sizeof(buff)));
            scheduleEpollRebuildLocked();
            return -1;
        }
    }
    return 1;
}

void Looper::sendMessage(const MessageHandler* handler, const Message& message) {
    const nsecs_t now = SystemClock::uptimeMillis();
    sendMessageAtTime(now, handler, message);
}

void Looper::sendMessageDelayed(nsecs_t uptimeDelay, const MessageHandler* handler,
        const Message& message) {
    const nsecs_t now = SystemClock::uptimeMillis();
    sendMessageAtTime(now + uptimeDelay, handler, message);
}

void Looper::sendMessageAtTime(nsecs_t uptime, const MessageHandler* handler,
        const Message& message) {
#if DEBUG_CALLBACKS
    LOGD("%p  sendMessageAtTime - uptime=%lld, handler=%p, what=%d", this, uptime, handler, message.what);
#endif
    size_t i = 0;
    { // acquire lock
        std::lock_guard<std::recursive_mutex> _l(mLock);

        std::list<MessageEnvelope>::const_iterator it;
        for(it = mMessageEnvelopes.begin();it != mMessageEnvelopes.end();++it){
            if(it->uptime >= uptime)break;
            i += 1;
        }

        MessageEnvelope messageEnvelope(uptime,const_cast<MessageHandler*>(handler), message);
        mMessageEnvelopes.insert(it,messageEnvelope);

        // Optimization: If the Looper is currently sending a message, then we can skip
        // the call to wake() because the next thing the Looper will do after processing
        // messages is to decide when the next wakeup time should be.  In fact, it does
        // not even matter whether this code is running on the Looper thread.
        if (mSendingMessage) return;
    } // release lock

    // Wake the poll loop only when we enqueue a new message at the head.
    if (i == 0) wake();
}

void Looper::addHandler(MessageHandler*handler){
    mHandlers.insert(mHandlers.begin(),handler);
}

void Looper::removeHandler(MessageHandler*handler){
    for(auto it = mHandlers.begin();it != mHandlers.end();it++){
        if( (*it) == handler){
            mHandlers.erase(it);
            handler->mFlags |= 1;//set Erase Flags.removed in doEventHandlers
            break;
        }
    }
}

void Looper::addEventHandler(const EventHandler*handler){
    mEventHandlers.insert(mEventHandlers.begin(),const_cast<EventHandler*>(handler));
}

void Looper::removeEventHandler(const EventHandler*handler){
    for(auto it = mEventHandlers.begin();it != mEventHandlers.end();it++){
        if( (*it) ==handler){
            (*it)->mFlags |=1;//set removed flags,removed in doEventHandlers
            //mEventHandlers.erase(it);
            break;
        }
    }
}

bool Looper::hasMessages(const MessageHandler* handler,int what,void*obj){
    std::lock_guard<std::recursive_mutex> _l(mLock);
    for( auto it = mMessageEnvelopes.begin();it != mMessageEnvelopes.end();it++){
        const Message&m = it->message;
        if((it->handler==handler)&&(m.what==what)&&((m.obj==obj)||(obj==nullptr))){
           return true;
        }
    }
    return false;
}

void Looper::removeMessages(const MessageHandler* handler) {
#if DEBUG_CALLBACKS
    LOGD("%p  removeMessages - handler=%p", this, handler);
#endif
    { // acquire lock
        std::lock_guard<std::recursive_mutex> _l(mLock);

        for( auto it = mMessageEnvelopes.begin();it != mMessageEnvelopes.end();){
            if(it->handler==handler){
               it = mMessageEnvelopes.erase(it);
               continue;
            }it++;
        } 
    } // release lock
}

void Looper::removeMessages(const MessageHandler* handler, int what) {
#if DEBUG_CALLBACKS
    LOGD("%p  removeMessages - handler=%p, what=%d size=%d", this, handler, what,mMessageEnvelopes.size());
#endif
    { // acquire lock
        std::lock_guard<std::recursive_mutex> _l(mLock);

        for( auto it = mMessageEnvelopes.begin();it != mMessageEnvelopes.end();){
            if((it->handler==handler) && (it->message.what==what)){
                it = mMessageEnvelopes.erase(it);
                continue;
            }it++;
        }
    } // release lock
}

void Looper::removeCallbacks(const MessageHandler* handler,const Runnable& r){
    std::lock_guard<std::recursive_mutex> _l(mLock);
    for( auto it = mMessageEnvelopes.begin();it != mMessageEnvelopes.end();){
        if((it->handler==handler) && (it->message.callback==r)){
            it = mMessageEnvelopes.erase(it);
            continue;
        }it++;
    }
}

bool Looper::isPolling() const {
    return mPolling;
}

LooperCallback::~LooperCallback(){
}

Message::Message(int msg){
    what= msg;
    arg1 = arg2 =0;
    obj = nullptr;
    target= nullptr;
}

MessageHandler::MessageHandler(){
    mFlags = 0;
}

MessageHandler::~MessageHandler(){
}

void MessageHandler::setOwned(bool looper){
    if(looper)mFlags|=2;
    else mFlags&=~2;
}

void MessageHandler::dispatchMessage(Message&message){
    if(message.callback)
        message.callback();
    else
        handleMessage(message);
}

void MessageHandler::handleIdle(){
}

EventHandler::EventHandler(){
    mFlags = 0;
}

EventHandler::~EventHandler(){
}

void EventHandler::setOwned(bool looper){
    if(looper)mFlags|=2;
    else mFlags&=~2;
}

}
