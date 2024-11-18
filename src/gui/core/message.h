#ifndef __MESSAGE_H__
#define __MESSAGE_H__
#include <core/callbackbase.h>
#include <mutex>
namespace cdroid{

class Message{
public:
    static constexpr int MAX_POOL_SIZE=50;
    static constexpr int FLAG_IN_USE = 1 << 0;
    /** If set message is asynchronous */
    static constexpr int FLAG_ASYNCHRONOUS = 1 << 1;
    /** Flags to clear in the copyFrom method */
    static constexpr int FLAGS_TO_CLEAR_ON_COPY_FROM = FLAG_IN_USE;
public:
    long long  when;
    int what;
    int arg1;
    int arg2;
    int flags;
    void*obj;
    Runnable callback;
    class Handler*target;
    Message*next;
private:
    static std::mutex mLock;
    static Message*sPool;
    static int sPoolSize;
public:
    Message(int what=0);
    bool isAsynchronous()const{
        return (flags & FLAG_ASYNCHRONOUS) != 0;
    }
    bool isInUse()const{
        return ((flags & FLAG_IN_USE) == FLAG_IN_USE);
    }
    void markInUse(){
        flags |= FLAG_IN_USE;
    }
    void sendToTarget();
    void recycle();
    void recycleUnchecked();
    static Message* obtain();
    static Message* obtain(const Message&);
    static Message* obtain(Handler* h);
};

}
#endif
