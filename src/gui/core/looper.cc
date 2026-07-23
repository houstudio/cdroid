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
#include <algorithm>
#include <fcntl.h>
#include <pthread.h>
#include <core/looper.h>
#include <core/messagequeue.h>
#include <core/handler.h>
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

// Wake channel backend, picked at compile time. eventfd is an epoll-ready fd on
// Linux; wepoll (the Windows epoll used below) only polls sockets, so on Windows
// we use a loopback socket pair; any other POSIX box falls back to a self-pipe,
// which is the pre-eventfd AOSP Looper idiom. All three expose a read fd that
// sits in the epoll set and a write fd that wake() pokes.
#if (defined(_WIN32)||defined(_WIN64)||defined(_MSVC_VER))
  #include <winsock2.h>
  #include <ws2tcpip.h>
  #define WAKE_USE_SOCKET 1
#elif defined(HAVE_EVENTFD)
  #define WAKE_USE_EVENTFD 1
#else
  #define WAKE_USE_PIPE 1
#endif
#include <limits.h>

#define DEBUG_POLL_AND_WAKE 0
#define DEBUG_CALLBACKS 0
#define USE_THREADLOCAL 1
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
        mWakeEventFd(-1),mWakeWriteFd(-1),mEpoll(nullptr),
        mNextRequestSeq(WAKE_EVENT_FD_SEQ+1),
        mResponseIndex(0), mNextMessageUptime(LLONG_MAX),
        mQueue(nullptr) {
    if (!openWakeFds()) {
        LOGE("Looper: could not open wake fds; wake() will be ineffective");
    }
    std::lock_guard<std::recursive_mutex>_l(mLock);
    rebuildEpollLocked();
    mQueue = new MessageQueue(true /*quitAllowed*/, this);
}

#define FLAG_OWNED 2
#define FLAG_REMOVED 1
Looper::~Looper() {
    LOGD("~Looper %p sMainLooper=%p",this,sMainLooper);
    delete mQueue;
    mQueue = nullptr;
    closeWakeFds();
    mHandlers.clear();
    delete mEpoll;
    for(EventHandler*hdl:mEventHandlers){
        if((hdl->mFlags&FLAG_OWNED)==FLAG_OWNED)delete hdl;
    }
    mEventHandlers.clear();
}

static void initTLSKey() {
#ifndef USE_THREADLOCAL
    const int error = pthread_key_create(&gTLSKey,[](void*st){
        Looper* const self = static_cast<Looper*>(st);
        if(self !=nullptr){
            delete self;
        }
    });
    LOGE_IF(error != 0, "Could not allocate TLS key: %s", strerror(error));
#endif
}

Looper* Looper::getDefault(){
    return getMainLooper();
}

Looper* Looper::getMainLooper(){
    return sMainLooper;
}

Looper* Looper::myLooper(){
    return getForThread();
}

Looper* Looper::prepare(int opts){
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

static thread_local Looper*gThreadLocalLooper=nullptr;
void Looper::setForThread(Looper* looper){
    Looper*old = getForThread();
    delete old;
#ifdef USE_THREADLOCAL
    gThreadLocalLooper = looper;
#else
    pthread_setspecific(gTLSKey,looper);
#endif
}

Looper* Looper::getForThread(){
#ifdef USE_THREADLOCAL
    return gThreadLocalLooper;
#else
    const int result = pthread_once(&gTLSOnce,initTLSKey);
    LOGW_IF(result != 0,"pthread_once failed");
    Looper*looper = static_cast<Looper*>(pthread_getspecific(gTLSKey));
    return looper;
#endif
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
    // Register the wake fd with the (fresh) epoll instance. It is always present
    // after openWakeFds() (eventfd / pipe / socket pair); without it wake() could
    // never break the epoll_wait in pollInner.
    if (mWakeEventFd >= 0) {
        struct epoll_event wakeEvent = createEpollEvent(EPOLLIN,WAKE_EVENT_FD_SEQ);
        int result = mEpoll->addFd(mWakeEventFd,wakeEvent);//epoll_ctl(mEpollFd, EPOLL_CTL_ADD, mWakeEventFd, &wakeEvent);
        LOGE_IF(result != 0, "Could not add wake fd to epoll instance: %s",strerror(errno));
    }
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

MessageQueue* Looper::getQueue(){
    return mQueue;
}

bool Looper::loopOnce(){
    if(mQueue == nullptr) return false;
    Message* msg = mQueue->next();/*maybe blocked*/
    if(msg == nullptr) return false;
    if(msg->target) msg->target->dispatchMessage(msg);
    msg->recycleUnchecked();
    return true;
}

void Looper::loop(){
    while(loopOnce()) {}
}

void Looper::drainMessageQueue(){
    if (mQueue == nullptr) return;
    while (auto* msg = mQueue->nextDue()) {
        if (msg->target) msg->target->dispatchMessage(msg);
        msg->recycleUnchecked();
    }
}

int Looper::doEventHandlers(){
    int count = 0;
    if(mNextMessageUptime>(nsecs_t)SystemClock::uptimeMillis()){
        for(auto it = mHandlers.begin();it != mHandlers.end();){
            MessageHandler*hdl = (*it);
            uint32_t eFlags = (*it)->mFlags;
            if((eFlags&FLAG_REMOVED)==0){
                hdl->handleIdle(); count++;
                eFlags = hdl->mFlags;
            }
            if(eFlags&FLAG_REMOVED){
                if(eFlags&FLAG_OWNED){
                    delete hdl;
                }
                it = mHandlers.erase(it);
                continue;
            }
            it++;
        }        
    }
    for(auto it=mEventHandlers.begin();it!=mEventHandlers.end();){
        EventHandler*es=(*it);
        uint32_t eFlags = es->mFlags;
        if(es&&((eFlags&FLAG_REMOVED)==0)){
            if(es->checkEvents()>0){
                es->handleEvents();  count++;
            }
            eFlags = es->mFlags;//Maybe EventHandler::handleEvents will remove itself,so we recheck the flags
        }
        if(eFlags&FLAG_REMOVED){
            if(eFlags&FLAG_OWNED){
                delete es;//EventHandler owned by looper must be freed here
            }
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
    drainMessageQueue();
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

bool Looper::openWakeFds() {
#if defined(WAKE_USE_EVENTFD)
    mWakeEventFd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (mWakeEventFd < 0) {
        LOGE("Could not make wake event fd: %s", strerror(errno));
        return false;
    }
    mWakeWriteFd = mWakeEventFd; // eventfd: a single fd serves both ends
    return true;
#elif defined(WAKE_USE_SOCKET)
    // wepoll only polls sockets, so the wake channel is a loopback TCP pair.
    // WSAStartup is ref-counted; calling it here decouples us from whoever
    // created the epoll instance (wepoll/IOEventProcessor also call it).
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        LOGE("wake socket: WSAStartup failed %d", WSAGetLastError());
        return false;
    }
    SOCKET listenFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    SOCKET connFd   = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenFd == INVALID_SOCKET || connFd == INVALID_SOCKET) {
        LOGE("wake socket: socket() failed %d", WSAGetLastError());
        if (listenFd != INVALID_SOCKET) closesocket(listenFd);
        if (connFd   != INVALID_SOCKET) closesocket(connFd);
        return false;
    }
    BOOL reuse = TRUE;
    setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = 0; // let the stack pick a port
    if (bind(listenFd, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR ||
        listen(listenFd, 1) == SOCKET_ERROR) {
        LOGE("wake socket: bind/listen failed %d", WSAGetLastError());
        closesocket(listenFd); closesocket(connFd);
        return false;
    }
    // Read back the assigned port, then connect the connector end. Loopback
    // connect completes synchronously, so the accept() below cannot race.
    int addrlen = sizeof(addr);
    getsockname(listenFd, (struct sockaddr*)&addr, &addrlen);
    if (connect(connFd, (struct sockaddr*)&addr, addrlen) == SOCKET_ERROR) {
        LOGE("wake socket: connect failed %d", WSAGetLastError());
        closesocket(listenFd); closesocket(connFd);
        return false;
    }
    SOCKET acceptFd = accept(listenFd, nullptr, nullptr);
    closesocket(listenFd);
    if (acceptFd == INVALID_SOCKET) {
        LOGE("wake socket: accept failed %d", WSAGetLastError());
        closesocket(connFd);
        return false;
    }
    // Non-blocking on both ends so wake()/awoken() can never stall the loop.
    u_long nb = 1;
    ioctlsocket(connFd,   FIONBIO, &nb);
    ioctlsocket(acceptFd, FIONBIO, &nb);
    BOOL nodelay = TRUE; // wake packets are 1 byte; disable Nagle coalescing
    setsockopt(connFd, IPPROTO_TCP, TCP_NODELAY, (char*)&nodelay, sizeof(nodelay));

    mWakeEventFd = (int)acceptFd; // read end
    mWakeWriteFd = (int)connFd;   // write end
    return true;
#else // WAKE_USE_PIPE — pre-eventfd AOSP idiom, portable across POSIX
    int pipefd[2];
    if (pipe(pipefd) < 0) {
        LOGE("Could not create wake pipe: %s", strerror(errno));
        return false;
    }
    // Non-blocking + close-on-exec on both ends (pipe2() is not portable).
    for (int i = 0; i < 2; i++) {
        int fl = fcntl(pipefd[i], F_GETFL);
        fcntl(pipefd[i], F_SETFL, fl | O_NONBLOCK);
        fcntl(pipefd[i], F_SETFD, FD_CLOEXEC);
    }
    mWakeEventFd = pipefd[0]; // read end
    mWakeWriteFd = pipefd[1]; // write end
    return true;
#endif
}

void Looper::closeWakeFds() {
#if defined(WAKE_USE_SOCKET)
    if (mWakeEventFd >= 0) closesocket((SOCKET)mWakeEventFd);
    if (mWakeWriteFd >= 0 && mWakeWriteFd != mWakeEventFd) closesocket((SOCKET)mWakeWriteFd);
#else
    if (mWakeEventFd >= 0) close(mWakeEventFd);
    if (mWakeWriteFd >= 0 && mWakeWriteFd != mWakeEventFd) close(mWakeWriteFd);
#endif
    mWakeEventFd = -1;
    mWakeWriteFd = -1;
}

void Looper::wake() {
    LOGD_IF(DEBUG_POLL_AND_WAKE,"%p  wake", this);
#if defined(WAKE_USE_EVENTFD)
    uint64_t inc = 1;
    long nWrite;
    do {
        nWrite = write(mWakeWriteFd, &inc, sizeof(uint64_t));
    } while ((nWrite == -1) && (errno == EINTR));
    if (nWrite != sizeof(uint64_t)) {
        char buff[128];
        LOGE_IF(errno!=EAGAIN,"Could not write wake signal to fd %d: %s",mWakeWriteFd, strerror_r(errno,buff,sizeof(buff)));
    }
#elif defined(WAKE_USE_SOCKET)
    // 1 byte is enough; WSAEWOULDBLOCK means a byte is already queued and the
    // epoll will fire regardless, so it is not an error.
    char c = 'W';
    int nWrite;
    do {
        nWrite = send((SOCKET)mWakeWriteFd, &c, 1, 0);
    } while (nWrite == SOCKET_ERROR && WSAGetLastError() == WSAEINTR);
    if (nWrite == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK) {
        LOGE("Could not write wake signal: WSA error %d", WSAGetLastError());
    }
#else // WAKE_USE_PIPE
    char c = 'W';
    ssize_t nWrite;
    do {
        nWrite = write(mWakeWriteFd, &c, 1);
    } while ((nWrite == -1) && (errno == EINTR));
    if (nWrite == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {
        char buff[128];
        LOGE("Could not write wake signal to fd %d: %s",mWakeWriteFd, strerror_r(errno,buff,sizeof(buff)));
    }
#endif
}

void Looper::awoken() {
    LOGD_IF(DEBUG_POLL_AND_WAKE,"%p  awoken", this);
#if defined(WAKE_USE_EVENTFD)
    uint64_t counter;
    long result;
    do {
        result = read(mWakeEventFd, &counter, sizeof(uint64_t));
    } while ((result == -1) && (errno == EINTR));
#elif defined(WAKE_USE_SOCKET)
    // Drain everything pending so the (level-triggered) socket stops readifying
    // and epoll_wait can block again. recv must be used on Windows sockets, not
    // the CRT _read() that `read` is macroed to elsewhere in this file.
    char buf[16];
    int result;
    do {
        result = recv((SOCKET)mWakeEventFd, buf, sizeof(buf), 0);
    } while (result > 0);
#else // WAKE_USE_PIPE — drain the self-pipe until it would block.
    char buf[16];
    ssize_t result;
    do {
        result = read(mWakeEventFd, buf, sizeof(buf));
    } while (result > 0);
#endif
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
    auto it = std::find(mHandlers.begin(),mHandlers.end(),handler);
    if(it==mHandlers.end()){
        mHandlers.insert(mHandlers.begin(),handler);
    }
}

void Looper::removeHandler(MessageHandler*handler){
    for(auto it = mHandlers.begin();it != mHandlers.end();it++){
        if( (*it) == handler){
            if(handler->mFlags & FLAG_OWNED){
                // looper 拥有: 延迟到 doEventHandlers 再 delete+erase。
                // 调用方可能正处在本 handler 自身的派发栈上 (如 SelfDestroyHandler::handleMessage
                // -> removeHandler(this)), 现删会令返回后访问 this 变 UAF。
                handler->mFlags |= FLAG_REMOVED;
            }else{
                // 外部拥有: 立即擦除指针。否则外部 delete 后 mHandlers 留下悬挂项,
                // doEventHandlers 后续读到已释放内存 (mFlags) → UAF/double-free。
                mHandlers.erase(it);
            }
            break;
        }
    }
}

void Looper::addEventHandler(const EventHandler*handler){
    auto it =std::find(mEventHandlers.begin(),mEventHandlers.end(),const_cast<EventHandler*>(handler));
    if( it == mEventHandlers.end()){
        mEventHandlers.insert(mEventHandlers.begin(),const_cast<EventHandler*>(handler));
    }
}

void Looper::removeEventHandler(const EventHandler*handler){
    for(auto it = mEventHandlers.begin();it != mEventHandlers.end();it++){
        if( (*it) ==handler){
            if((*it)->mFlags & FLAG_OWNED){
                // looper 拥有: 延迟到 doEventHandlers 再 delete+erase (同 removeHandler 理由)。
                (*it)->mFlags |=FLAG_REMOVED;
            }else{
                // 外部拥有: 立即擦除, 避免外部 delete 留悬挂项致 doEventHandlers UAF。
                mEventHandlers.erase(it);
            }
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

MessageHandler::MessageHandler(){
    mFlags = 0;
}

MessageHandler::~MessageHandler(){
}

void MessageHandler::setOwned(bool looper){
    if(looper)mFlags|=FLAG_OWNED;
    else mFlags&=~FLAG_OWNED;
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
    if(looper)mFlags|=FLAG_OWNED;
    else mFlags&=~FLAG_OWNED;
}

}
