#include <looper.h>
#include <unistd.h>
#include <string.h>
#include <sys/eventfd.h>
#include <cdtypes.h>
#include <cdlog.h>
#include <limits.h>
#include <systemclock.h>

#define DEBUG_POLL_AND_WAKE 0
#define DEBUG_CALLBACKS 0

#if USED_POLL == NOPOLL
#define EPOLLIN  1
#define EPOLLOUT 4
#define EPOLLERR 8
#define EPOLLHUP 0x10
#elif USED_POLL==POLL
#define EPOLLIN  POLLIN
#define EPOLLOUT POLLOUT
#define EPOLLERR POLLERR
#define EPOLLHUP POLLHUP
#endif
namespace cdroid{

static constexpr int EPOLL_SIZE_HINT = 8;
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

void Looper::Looper::Request::initEventItem(struct epoll_event* eventItem) const{
    int epollEvents = 0;
    if (events & EVENT_INPUT) epollEvents |= EPOLLIN;
    if (events & EVENT_OUTPUT)epollEvents |= EPOLLOUT;

    memset(eventItem, 0, sizeof(epoll_event)); // zero out unused members of data field union
    eventItem->events = epollEvents;
    eventItem->data.fd = fd;
}

Looper::Looper(bool allowNonCallbacks) :
        mAllowNonCallbacks(allowNonCallbacks), mSendingMessage(false),
        mPolling(false), mEpollFd(-1), mEpollRebuildRequired(false),
        mNextRequestSeq(0), mResponseIndex(0), mNextMessageUptime(LLONG_MAX) {
    mWakeEventFd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    LOGE_IF(mWakeEventFd < 0, "Could not make wake event fd: %s",strerror(errno));
    std::lock_guard<std::recursive_mutex>_l(mLock);
    rebuildEpollLocked();
}

Looper::~Looper() {
    close(mWakeEventFd);
    mWakeEventFd = -1;
    if (mEpollFd >= 0) {
        close(mEpollFd);
    }
    for(EventHandler*hdl:mEventHandlers){
        delete hdl;
    }mEventHandlers.clear();
}

void Looper::initTLSKey() {
    int error = pthread_key_create(&gTLSKey, threadDestructor);
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

void Looper::setForThread(Looper* looper){
    Looper*old = getForThread();
    delete old;
    pthread_setspecific(gTLSKey,looper);
}

Looper*Looper::getForThread(){
   int result = pthread_once(&gTLSOnce,initTLSKey);
   LOGW_IF(result!=0,"pthread_once failed");
   Looper*looper =(Looper*)pthread_getspecific(gTLSKey);
   return looper;
}

bool Looper::getAllowNonCallbacks() const {
    return mAllowNonCallbacks;
}

void Looper::rebuildEpollLocked() {
    // Close old epoll instance if we have one.
#if USED_POLL == EPOLL
    if (mEpollFd >= 0) {
        LOGV("%p ~ rebuildEpollLocked - rebuilding epoll set", this);
        close(mEpollFd);
    }

    // Allocate the new epoll instance and register the wake pipe.
    mEpollFd = epoll_create(EPOLL_SIZE_HINT);
    LOGE_IF(mEpollFd < 0, "Could not create epoll instance: %s", strerror(errno));
#endif
    struct epoll_event eventItem;
    memset(& eventItem, 0, sizeof(epoll_event)); // zero out unused members of data field union
    eventItem.events  = EPOLLIN;
    eventItem.data.fd = mWakeEventFd;
#if USED_POLL == EPOLL
    int result = epoll_ctl(mEpollFd, EPOLL_CTL_ADD, mWakeEventFd, & eventItem);
    LOGE_IF(result != 0, "Could not add wake event fd to epoll instance: %s",strerror(errno));
#endif
    for (auto it=mRequests.begin();it!=mRequests.end(); it++) {
        const Request& request = it->second;
        struct epoll_event eventItem;
        request.initEventItem(&eventItem);
#if USED_POLL == EPOLL
        int epollResult = epoll_ctl(mEpollFd, EPOLL_CTL_ADD, request.fd, & eventItem);
        if (epollResult < 0) {
            LOGE("Error adding epoll events for fd %d while rebuilding epoll set: %s",request.fd, strerror(errno));
        }
#endif
    }
}

void Looper::scheduleEpollRebuildLocked() {
    if (!mEpollRebuildRequired) {
        LOGD_IF(DEBUG_CALLBACKS,"%p  scheduleEpollRebuildLocked - scheduling epoll set rebuild", this);
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
                LOGD_IF(DEBUG_POLL_AND_WAKE,"%p returning signalled identifier %d: fd=%d, events=0x%x, data=%p", this, ident, fd, events, data);
                if (outFd != nullptr) *outFd = fd;
                if (outEvents != nullptr) *outEvents = events;
                if (outData != nullptr) *outData = data;
                return ident;
            }
        }
        if (result != 0) {
            LOGD_IF(DEBUG_POLL_AND_WAKE,"%p returning result %d", this, result);
            if (outFd != nullptr) *outFd = 0;
            if (outEvents != nullptr) *outEvents = 0;
            if (outData != nullptr) *outData = nullptr;
            return result;
        }
        result = pollInner(timeoutMillis);
    }
}

int Looper::pollEvents(int timeoutMillis){
    int result = POLL_WAKE;
    mResponses.clear();
    mResponseIndex = 0;
    mPolling = true;
    int j,eventCount; 
    struct epoll_event eventItems[EPOLL_MAX_EVENTS];
#if USED_POLL == EPOLL
    eventCount = epoll_wait(mEpollFd, eventItems, EPOLL_MAX_EVENTS, timeoutMillis);
#elif USED_POLL == POLL
    std::vector<struct pollfd>pollfds;
    for (auto r:mRequests){
        struct pollfd pfd;
        pfd.fd=r.second.fd;
        pfd.events=r.second.events;
        pollfds.push_back(pfd);
    }
    eventCount=poll(pollfds.data(),pollfds.size(),timeoutMillis);
    j=0;
    for(auto f:pollfds){
        if(f.revents==0)continue;
        eventItems[j].data.fd=f.fd;
        eventItems[j].events=f.revents;
        j++;
    }
#endif
    // No longer idling.
    mPolling = false;

    if (mEpollRebuildRequired) {
        mEpollRebuildRequired = false;
        rebuildEpollLocked();
        goto Done;
    }

    // Check for poll error.
    if (eventCount < 0) {
        if (errno == EINTR) {
            goto Done;
        }
        LOGW("Poll failed with an unexpected error: %s", strerror(errno));
        result = POLL_ERROR;
        goto Done;
    }

    // Check for poll timeout.
    if (eventCount == 0) {
        LOGD_IF(DEBUG_POLL_AND_WAKE,"%p  pollOnce - timeout", this);
        result = POLL_TIMEOUT;
        goto Done;
    }

    // Handle all events.
    LOGD_IF(DEBUG_POLL_AND_WAKE,"%p handling events from %d fds", this, eventCount);

    for (int i = 0; i < eventCount; i++) {
        int fd = eventItems[i].data.fd;
        uint32_t epollEvents = eventItems[i].events;
        if (fd == mWakeEventFd) {
            if (epollEvents & EPOLLIN) {
                awoken();
            } else {
                LOGW("Ignoring unexpected epoll events 0x%x on wake event fd.", epollEvents);
            }
        } else {
            auto it= mRequests.find(fd);
            if (it!=mRequests.end()){
                int events = 0;
                if (epollEvents & EPOLLIN) events |= EVENT_INPUT;
                if (epollEvents & EPOLLOUT) events |= EVENT_OUTPUT;
                if (epollEvents & EPOLLERR) events |= EVENT_ERROR;
                if (epollEvents & EPOLLHUP) events |= EVENT_HANGUP;
                pushResponse(events, it->second);
            } else {
                LOGW("Ignoring unexpected epoll events 0x%x on fd %d that is "
                        "no longer registered.", epollEvents, fd);
            }
        }
    }
    // Invoke all response callbacks.
    for (size_t i = 0; i < mResponses.size(); i++) {
        Response& response = mResponses.at(i);
        if (response.request.ident == POLL_CALLBACK) {
            int fd = response.request.fd;
            int events = response.events;
            void* data = response.request.data;
            LOGD_IF(DEBUG_POLL_AND_WAKE || DEBUG_CALLBACKS,"%pinvoking fd event callback %p: fd=%d, events=0x%x, data=%p",
                    this, response.request.callback, fd, events, data);

            // Invoke the callback.  Note that the file descriptor may be closed by
            // the callback (and potentially even reused) before the function returns so
            // we need to be a little careful when removing the file descriptor afterwards.
            int callbackResult = response.request.callback->handleEvent(fd, events, data);
            if (callbackResult == 0) {
                removeFd(fd, response.request.seq);
            }

            // Clear the callback reference in the response structure promptly because we
            // will not clear the response vector itself until the next poll.
            //response.request.callback.clear();
            result = POLL_CALLBACK;
        }
    }
Done:
    return result;
}

int Looper::pollInner(int timeoutMillis) {
    int result = POLL_WAKE;
    LOGD_IF(DEBUG_POLL_AND_WAKE,"%p waiting: timeoutMillis=%d mNextMessageUptime=%lld/%lld",
            this,timeoutMillis,mNextMessageUptime,LLONG_MAX);

    // Adjust the timeout based on when the next message is due.
    if (timeoutMillis != 0 && mNextMessageUptime != LLONG_MAX) {
        nsecs_t now = SystemClock::uptimeMillis();
        int messageTimeoutMillis = toMillisecondTimeoutDelay(now, mNextMessageUptime);
        if ( (messageTimeoutMillis >= 0 ) && (timeoutMillis < 0 || messageTimeoutMillis < timeoutMillis)) {
            timeoutMillis = messageTimeoutMillis;
        }
        LOGD_IF(DEBUG_POLL_AND_WAKE,"%p next message in %lld ns, adjusted timeout:timeoutMillis=%d",
                this,mNextMessageUptime - now, timeoutMillis);
    }
    nsecs_t t1=SystemClock::uptimeMillis();
    if(1||mNextMessageUptime>(nsecs_t)SystemClock::uptimeMillis()){
        doIdleHandlers(); 
    }
    for(auto it=mEventHandlers.begin();it!=mEventHandlers.end();it++){
        EventHandler*es=(*it);
        if(es&&((es->mFlags&1)==0)&&(es->checkEvents()>0)){ 
            es->handleEvents();
            //may be EventHandler::handleEvents will destroy itself,recheck the flags
            if((es->mFlags&1)==1) it = mEventHandlers.erase(it);
            if((es->mFlags&3)==3) delete es;//make EventHandler::handleEvents can destroy itself
            //EventHandler owned by looper must be freed here
        }
    }
    for(auto it=mEventHandlers.begin();it!=mEventHandlers.end();it++){
        EventHandler*es=(*it);
        if((es->mFlags&3)==3){
            it = mEventHandlers.erase(it);
            delete es;//EventHandler owned by looper must be freed here
        }
    }
    long elapsedMillis = SystemClock::uptimeMillis()-t1;

    // Acquire lock.
    mLock.lock();

    // Invoke pending message callbacks.
    mNextMessageUptime = LLONG_MAX;
    LOGD_IF(DEBUG_CALLBACKS,"mMessageEnvelopes.size=%d",mMessageEnvelopes.size());
    while (mMessageEnvelopes.size() != 0) {
        nsecs_t now = SystemClock::uptimeMillis();
        const MessageEnvelope& messageEnvelope = mMessageEnvelopes.front();
        if (messageEnvelope.uptime <= now) {
            // Remove the envelope from the list.
            // We keep a strong reference to the handler until the call to handleMessage
            // finishes.  Then we drop it so that the handler can be deleted *before*
            // we reacquire our lock.
            { // obtain handler
                MessageHandler* handler = messageEnvelope.handler;
                Message message = messageEnvelope.message;
                mMessageEnvelopes.pop_front();
                mSendingMessage = true;
                mLock.unlock();

                LOGD_IF(DEBUG_POLL_AND_WAKE||DEBUG_CALLBACKS,"%psending message: handler=%p, what=%d",
                        this, handler, message.what);
                if(message.callback)
                    message.callback();
                else 
                    handler->handleMessage(message);
            } // release handler

            mLock.lock();
            mSendingMessage = false;
            result = POLL_CALLBACK;
        } else {
            // The last message left at the head of the queue determines the next wakeup time.
            mNextMessageUptime = messageEnvelope.uptime;
            break;
        }
    }
    pollEvents(timeoutMillis>elapsedMillis?(timeoutMillis-elapsedMillis):0);
    mLock.unlock();

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

void Looper::doIdleHandlers(){
    for(auto it=mHandlers.begin();it!=mHandlers.end();it++){
        const int flags = (*it)->mFlags;
        if(flags&1){
            LOGD("delete Handler %p",(*it));
            if(flags&2)delete *it;
            it= mHandlers.erase(it);
            continue;
        }
        (*it)->handleIdle();
    }
}

//TEMP_FAILURE_RETRY defined in <unistd.h>
void Looper::wake() {
    LOGD_IF(DEBUG_POLL_AND_WAKE,"%p  wake", this);

    uint64_t inc = 1;
    ssize_t nWrite = TEMP_FAILURE_RETRY(write(mWakeEventFd, &inc, sizeof(uint64_t)));
    if (nWrite != sizeof(uint64_t)) {
        LOGE_IF(errno!=EAGAIN,"Could not write wake signal to fd %d: %s",mWakeEventFd, strerror(errno));
    }
}

void Looper::awoken() {
    LOGD_IF(DEBUG_POLL_AND_WAKE,"%p  awoken", this);

    uint64_t counter;
    TEMP_FAILURE_RETRY(read(mWakeEventFd, &counter, sizeof(uint64_t)));
}

void Looper::pushResponse(int events, const Request& request) {
    Response response;
    response.events = events;
    response.request = request;
    mResponses.push_back(response);
}

int Looper::addFd(int fd, int ident, int events, Looper_callbackFunc callback, void* data) {
    return addFd(fd, ident, events, callback ? new SimpleLooperCallback(callback) : nullptr, data);
}

int Looper::addFd(int fd, int ident, int events,const LooperCallback* callback, void* data) {
    LOGD_IF(DEBUG_CALLBACKS,"%p  addFd - fd=%d, ident=%d, events=0x%x, callback=%p, data=%p", this, fd, ident, events, callback, data);

    if (callback==nullptr) {
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

        Request request;
        request.fd = fd;
        request.ident = ident;
        request.events = events;
        request.seq = mNextRequestSeq++;
        request.callback = (LooperCallback*)callback;
        request.data = data;
        if (mNextRequestSeq == -1) mNextRequestSeq = 0; // reserve sequence number -1

        struct epoll_event eventItem;
        request.initEventItem(&eventItem);

        auto itfd = mRequests.find(fd);//indexOfKey(fd);
        if (itfd==mRequests.end()) {
#if USED_POLL ==EPOLL     
            int epollResult = epoll_ctl(mEpollFd, EPOLL_CTL_ADD, fd, & eventItem);
            if (epollResult < 0) {
                LOGE("Error adding epoll events for fd %d: %s", fd, strerror(errno));
                return -1;
            }
#endif
            mRequests[fd]=request;//.add(fd, request);
        } else {
#if USED_POLL ==EPOLL
            int epollResult = epoll_ctl(mEpollFd, EPOLL_CTL_MOD, fd, & eventItem);
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
                    LOGD_IF(DEBUG_CALLBACKS,"%p  addFd - EPOLL_CTL_MOD failed due to file descriptor "
                            "being recycled, falling back on EPOLL_CTL_ADD: %s", this, strerror(errno));
                    epollResult = epoll_ctl(mEpollFd, EPOLL_CTL_ADD, fd, & eventItem);
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
#endif
            itfd->second=request;//mRequests.replaceValueAt(requestIndex, request);
        }
    } // release lock
    return 1;
}

int Looper::removeFd(int fd) {
    return removeFd(fd, -1);
}

int Looper::removeFd(int fd, int seq) {
    LOGD_IF(DEBUG_CALLBACKS,"%p  removeFd - fd=%d, seq=%d", this, fd, seq);

    { // acquire lock
        std::lock_guard<std::recursive_mutex>  _l(mLock);
        auto itr= mRequests.find(fd);//indexOfKey(fd);
        if (itr==mRequests.end()) {
            return 0;
        }

        // Check the sequence number if one was given.
        if (seq != -1 && itr->second.seq != seq) {
            LOGD_IF(DEBUG_CALLBACKS,"%p  removeFd - sequence number mismatch, oldSeq=%d", this, itr->second.seq);
            return 0;
        }

        // Always remove the FD from the request map even if an error occurs while
        // updating the epoll set so that we avoid accidentally leaking callbacks.
        mRequests.erase(itr);
#if USED_POLL ==EPOLL
        int epollResult = epoll_ctl(mEpollFd, EPOLL_CTL_DEL, fd, nullptr);
        if (epollResult < 0) {
            if (seq != -1 && (errno == EBADF || errno == ENOENT)) {
                // Tolerate EBADF or ENOENT when the sequence number is known because it
                // means that the file descriptor was closed before its callback was
                // unregistered.  This error may occur naturally when a callback has the
                // side-effect of closing the file descriptor before returning and
                // unregistering itself.
                //
                // Unfortunately due to kernel limitations we need to rebuild the epoll
                // set from scratch because it may contain an old file handle that we are
                // now unable to remove since its file descriptor is no longer valid.
                // No such problem would have occurred if we were using the poll system
                // call instead, but that approach carries others disadvantages.
                LOGD_IF(DEBUG_CALLBACKS,"%p  removeFd - EPOLL_CTL_DEL failed due to file descriptor "
                        "being closed: %s", this, strerror(errno));
                scheduleEpollRebuildLocked();
            } else {
                // Some other error occurred.  This is really weird because it means
                // our list of callbacks got out of sync with the epoll set somehow.
                // We defensively rebuild the epoll set to avoid getting spurious
                // notifications with nowhere to go.
                LOGE("Error removing epoll events for fd %d: %s", fd, strerror(errno));
                scheduleEpollRebuildLocked();
                return -1;
            }
        }
#endif
    } // release lock
    return 1;
}

void Looper::sendMessage(const MessageHandler* handler, const Message& message) {
    nsecs_t now = SystemClock::uptimeMillis();
    sendMessageAtTime(now, handler, message);
}

void Looper::sendMessageDelayed(nsecs_t uptimeDelay, const MessageHandler* handler,
        const Message& message) {
    nsecs_t now = SystemClock::uptimeMillis();
    sendMessageAtTime(now + uptimeDelay, handler, message);
}

void Looper::sendMessageAtTime(nsecs_t uptime, const MessageHandler* handler,
        const Message& message) {
    LOGD_IF(DEBUG_CALLBACKS,"%p  sendMessageAtTime - uptime=%lld, handler=%p, what=%d", this, uptime, handler, message.what);

    size_t i = 0;
    { // acquire lock
        std::lock_guard<std::recursive_mutex> _l(mLock);

        std::list<MessageEnvelope>::iterator it;
        for(it=mMessageEnvelopes.begin();it!=mMessageEnvelopes.end();it++){
            if(it->uptime>=uptime)break;
            i+=1;
        }

        MessageEnvelope messageEnvelope(uptime,(MessageHandler*)handler, message);
        mMessageEnvelopes.insert(it,messageEnvelope);

        // Optimization: If the Looper is currently sending a message, then we can skip
        // the call to wake() because the next thing the Looper will do after processing
        // messages is to decide when the next wakeup time should be.  In fact, it does
        // not even matter whether this code is running on the Looper thread.
        if (mSendingMessage) return;
    } // release lock

    // Wake the poll loop only when we enqueue a new message at the head.
    if (i == 0)  wake();
}

void Looper::addHandler(MessageHandler*handler){
    mHandlers.insert(mHandlers.begin(),handler);
}

void Looper::removeHandler(MessageHandler*handler){
    for(auto it=mHandlers.begin();it!=mHandlers.end();it++){
        if((*it)==handler){
            handler->mFlags|=1;
            if((handler->mFlags&2)==0)//handler is not owned by looper,erase at once
                mHandlers.erase(it);
            break;
        }
    }
}

void Looper::addEventHandler(const EventHandler*handler){
    mEventHandlers.insert(mEventHandlers.begin(),(EventHandler*)handler);
}

void Looper::removeEventHandler(const EventHandler*handler){
    for(auto it=mEventHandlers.begin();it!=mEventHandlers.end();it++){
        if((*it)==handler){
            (*it)->mFlags |=1;//set removed flags
            if((handler->mFlags&2)==0)//handler is not owned by looper,erase at once
                mEventHandlers.erase(it);
            break;
        }
    }
}

void Looper::removeMessages(const MessageHandler* handler) {
    LOGD_IF(DEBUG_CALLBACKS,"%p  removeMessages - handler=%p", this, handler);
    { // acquire lock
        std::lock_guard<std::recursive_mutex> _l(mLock);

        for( auto it=mMessageEnvelopes.begin();it!=mMessageEnvelopes.end();it++){
            if(it->handler==handler)
               it=mMessageEnvelopes.erase(it);
        } 
    } // release lock
}

void Looper::removeMessages(const MessageHandler* handler, int what) {
    LOGD_IF(DEBUG_CALLBACKS,"%p  removeMessages - handler=%p, what=%d size=%d", this, handler, what,mMessageEnvelopes.size());
    { // acquire lock
        std::lock_guard<std::recursive_mutex> _l(mLock);

        for( auto it=mMessageEnvelopes.begin();it!=mMessageEnvelopes.end();it++){
            if((it->handler==handler) && (it->message.what==what)){
                it=mMessageEnvelopes.erase(it);
            }
        }
    } // release lock
}

void Looper::removeCallbacks(const MessageHandler* handler,Runnable r){
    std::lock_guard<std::recursive_mutex> _l(mLock);
    for( auto it = mMessageEnvelopes.begin();it != mMessageEnvelopes.end();it++){
        if((it->handler==handler) && (it->message.callback==r)){
            it = mMessageEnvelopes.erase(it);
        }
    }
}

bool Looper::isPolling() const {
    return mPolling;
}

LooperCallback::~LooperCallback(){
}

SimpleLooperCallback::SimpleLooperCallback(Looper_callbackFunc callback) :
        mCallback(callback) {
}

SimpleLooperCallback::~SimpleLooperCallback() {
}

int SimpleLooperCallback::handleEvent(int fd, int events, void* data) {
    return mCallback(fd, events, data);
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
