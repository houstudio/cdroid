#ifndef __ALOOPER_H__
#define __ALOOPER_H__
#include <unordered_map>
#include <vector>
#include <mutex>
#include <list>
#include <cstdint>
#include <core/message.h>

namespace cdroid{
typedef int64_t nsecs_t;
typedef int (*Looper_callbackFunc)(int fd, int events, void* data);

class LooperCallback{
protected:
    virtual ~LooperCallback();
public:
    virtual int handleEvent(int fd, int events, void* data) = 0;
};

class SimpleLooperCallback : public LooperCallback {
protected:
    virtual ~SimpleLooperCallback();
public:
    SimpleLooperCallback(Looper_callbackFunc callback);
    virtual int handleEvent(int fd, int events, void* data);
private:
    Looper_callbackFunc mCallback;
};

class MessageHandler{
private:
    uint32_t mFlags;
    friend class Looper;
protected:
    MessageHandler();
    void setOwned(bool looper);
    virtual ~MessageHandler();
public:
    virtual void handleMessage(Message& message)=0;
    virtual void handleIdle();
};

class EventHandler{
protected:
    uint32_t mFlags;
    friend class Looper;
protected:
    EventHandler();
    virtual ~EventHandler();
    void setOwned(bool looper);
public:
    virtual int checkEvents()=0;
    virtual int handleEvents()=0;
};

class Looper{
private:
    using SequenceNumber = uint64_t;
    struct Request {
        int fd;
        int ident;
        int events;
        LooperCallback* callback;
        void* data;
        uint32_t getEpollEvents() const;
    };
    struct Response {
        SequenceNumber seq;
        int events;
        Request request;
    };

    struct MessageEnvelope {
        MessageEnvelope() : uptime(0),handler(nullptr){}
        MessageEnvelope(nsecs_t u, MessageHandler* h,
            const Message& m) : uptime(u), handler(h), message(m) {
        }
        nsecs_t uptime;
        MessageHandler* handler;
        Message message;
    };
    
    bool mAllowNonCallbacks; // immutable

    int  mWakeEventFd;// immutable
    std::recursive_mutex mLock;

    std::list<MessageEnvelope> mMessageEnvelopes; // guarded by mLock
    bool mSendingMessage; // guarded by mLock
    std::list<MessageHandler*>mHandlers;
    std::list<EventHandler*> mEventHandlers;

    // Whether we are currently waiting for work.  Not protected by a lock,
    // any use of it is racy anyway.
    bool mPolling;

    class IOEventProcessor* mEpoll;
    int  mEpollFd;// guarded by mLock but only modified on the looper thread
    bool mEpollRebuildRequired; // guarded by mLock

    // Locked list of file descriptor monitoring requests.
    std::unordered_map<SequenceNumber, Request> mRequests; //guarded by mLock
    std::unordered_map<int,SequenceNumber>mSequenceNumberByFd; //guarded by mLock
    SequenceNumber mNextRequestSeq;

    // This state is only used privately by pollOnce and does not require a lock since
    // it runs on a single thread.
    std::vector<Response> mResponses;
    size_t mResponseIndex;
    nsecs_t mNextMessageUptime;
private:
    static Looper*sMainLooper;
    int doEventHandlers();
    int pollInner(int timeoutMillis);
    int removeSequenceNumberLocked(SequenceNumber seq);
    void awoken();
    void pushResponse(int events, const Request& request);
    void rebuildEpollLocked();
    void scheduleEpollRebuildLocked();
    static void initTLSKey();
    static void threadDestructor(void*);
protected:
public:
    enum {
        POLL_WAKE = -1,
        POLL_CALLBACK = -2,
        POLL_TIMEOUT = -3,
        POLL_ERROR = -4,
    };
    enum {
        EVENT_INPUT = 1 << 0,
        EVENT_OUTPUT = 1 << 1,
        EVENT_ERROR = 1 << 2,
        EVENT_HANGUP = 1 << 3,
        EVENT_INVALID = 1 << 4,
    };

    enum {
        PREPARE_ALLOW_NON_CALLBACKS = 1<<0
    };
public:
    Looper(bool allowNonCallbacks=false);
    virtual ~Looper();
    [[deprecated("This function is deprecated, please use myLooper() or getMainLooper() instead.")]]
    static Looper*getDefault();
    static Looper*getMainLooper();
    static Looper*myLooper();
    static void prepareMainLooper();
    static Looper*prepare(int opts);
    static void setForThread(Looper* looper);
    static Looper* getForThread();
    bool getAllowNonCallbacks() const;
    int  pollOnce(int timeoutMillis, int* outFd, int* outEvents, void** outData);
    inline int pollOnce(int timeoutMillis) {
        return pollOnce(timeoutMillis, NULL, NULL, NULL);
    }
    int  pollAll(int timeoutMillis, int* outFd, int* outEvents, void** outData);
    inline int pollAll(int timeoutMillis) {
        return pollAll(timeoutMillis, NULL, NULL, NULL);
    }
    void wake();
    int  addFd(int fd, int ident, int events, Looper_callbackFunc callback, void* data);
    int  addFd(int fd, int ident, int events, const LooperCallback* callback, void* data);
    int  removeFd(int fd);
    void sendMessage(const MessageHandler* handler, const Message& message);
    void sendMessageDelayed(nsecs_t uptimeDelay, const MessageHandler* handler,const Message& message);
    void sendMessageAtTime(nsecs_t uptime, const MessageHandler* handler,const Message& message);
    void removeMessages(const MessageHandler* handler);
    void removeMessages(const MessageHandler* handler, int what);
    void removeCallbacks(const MessageHandler* handler,Runnable r);
    bool isPolling() const;

    void addHandler(MessageHandler*);
    void removeHandler(MessageHandler*);
    void addEventHandler(const EventHandler*handler);
    void removeEventHandler(const EventHandler*handler);
};
}
#endif
