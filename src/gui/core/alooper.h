#ifndef __ALOOPER_H__
#define __ALOOPER_H__
#include <functional>
#include <sys/epoll.h>
#include <map>
#include <vector>
namespace alooper{

typedef int64_t nsecs_t;
typedef std::function<int(int fd, int events, void* data)>LooperCallback;
struct Message {
    Message() : what(0) { }
    Message(int w) : what(w) { }

    /* The message type. (interpretation is left up to the handler) */
    int what;
};

class MessageHandler{
protected:
    virtual ~MessageHandler();

public:
    /* Handles a message. */
    virtual void handleMessage(const Message& message) = 0;
};

class Looper{
private:
    struct Request {
        int fd;
        int ident;
        int events;
        int seq;
        LooperCallback callback;
        void* data;
        void initEventItem(struct epoll_event* eventItem) const;
    };
    struct Response {
        int events;
        Request request;
    };

    struct MessageEnvelope {
        MessageEnvelope() : uptime(0) { }
        MessageEnvelope(nsecs_t u, MessageHandler* h,
            const Message& m) : uptime(u), handler(h), message(m) {
        }
        nsecs_t uptime;
        MessageHandler* handler;
        Message message;
    };
    
    bool mAllowNonCallbacks; // immutable

    int mWakeEventFd;  // immutable
    //Mutex mLock;

    std::vector<MessageEnvelope> mMessageEnvelopes; // guarded by mLock
    bool mSendingMessage; // guarded by mLock

    // Whether we are currently waiting for work.  Not protected by a lock,
    // any use of it is racy anyway.
    volatile bool mPolling;

    int mEpollFd; // guarded by mLock but only modified on the looper thread
    bool mEpollRebuildRequired; // guarded by mLock

    // Locked list of file descriptor monitoring requests.
    std::map<int, Request> mRequests;  // guarded by mLock
    int mNextRequestSeq;

    // This state is only used privately by pollOnce and does not require a lock since
    // it runs on a single thread.
    std::vector<Response> mResponses;
    size_t mResponseIndex;
    nsecs_t mNextMessageUptime;
private:
    int pollInner(int timeoutMillis);
    int removeFd(int fd, int seq);
    void awoken();
    void pushResponse(int events, const Request& request);
    void rebuildEpollLocked();
protected:
    virtual ~Looper(){};
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
    bool getAllowNonCallbacks() const;
    int pollOnce(int timeoutMillis, int* outFd, int* outEvents, void** outData);
    inline int pollOnce(int timeoutMillis) {
        return pollOnce(timeoutMillis, NULL, NULL, NULL);
    }
    int pollAll(int timeoutMillis, int* outFd, int* outEvents, void** outData);
    inline int pollAll(int timeoutMillis) {
        return pollAll(timeoutMillis, NULL, NULL, NULL);
    }
    void wake();
    int addFd(int fd, int ident, int events, LooperCallback callback, void* data);
    int removeFd(int fd);
    void sendMessage(MessageHandler& handler, const Message& message);
    void sendMessageDelayed(nsecs_t uptimeDelay, const MessageHandler& handler,const Message& message);
    void sendMessageAtTime(nsecs_t uptime, const MessageHandler& handler,const Message& message);
    void removeMessages(const MessageHandler& handler);
    void removeMessages(const MessageHandler& handler, int what);
    bool isPolling() const;
    
};
}
#endif
