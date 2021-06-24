#include <choreographer.h>
#include <systemclock.h>
namespace cdroid{
#if 0
#define FRAME_CALLBACK_TOKEN 1
class CallbackRecord {
public:
    CallbackRecord* next;
    long dueTime;
    Runnable action; // Runnable or FrameCallback
    long token;

    void run(long frameTimeNanos) {
        if (token == FRAME_CALLBACK_TOKEN) {
            //((FrameCallback)action).doFrame(frameTimeNanos);
        } else {
            //((Runnable)action).run();
        }
    }
};
template<class T>
static const void * addr_of(T &&obj) noexcept{
   struct A {};
   return &reinterpret_cast<const A &>(obj);
}

class CallbackQueue {
private:
    CallbackRecord* mHead;
public:
    bool hasDueCallbacksLocked(long now) {
        return mHead != nullptr && mHead->dueTime <= now;
    }

    CallbackRecord* extractDueCallbacksLocked(long now) {
        CallbackRecord* callbacks = mHead;
        if (callbacks == nullptr || callbacks->dueTime > now) {
            return nullptr;
        }

        CallbackRecord* last = callbacks;
        CallbackRecord* next = last->next;
        while (next != nullptr) {
            if (next->dueTime > now) {
                last->next = nullptr;
                break;
            }
            last = next;
            next = next->next;
        }
        mHead = next;
        return callbacks;
    }

    void addCallbackLocked(long dueTime, Runnable action, long token) {
        CallbackRecord* callback = obtainCallbackLocked(dueTime, action, token);
        CallbackRecord* entry = mHead;
        if (entry == nullptr) {
            mHead = callback;
            return;
        }
        if (dueTime < entry->dueTime) {
            callback->next = entry;
            mHead = callback;
            return;
        }
        while (entry->next != nullptr) {
            if (dueTime < entry->next->dueTime) {
                callback->next = entry->next;
                break;
            }
            entry = entry->next;
        }
        entry->next = callback;
    }

    void removeCallbacksLocked(Runnable action, long token) {
        CallbackRecord* predecessor = nullptr;
        for (CallbackRecord* callback = mHead; callback != nullptr;) {
            CallbackRecord* next = callback->next;
            if ((action == nullptr || addr_of(callback->action) == addr_of(action))
                    && (token == 0 || callback->token == token)) {
                if (predecessor != nullptr) {
                    predecessor->next = next;
                } else {
                    mHead = next;
                }
                recycleCallbackLocked(callback);
            } else {
                predecessor = callback;
            }
            callback = next;
        }
    }
};
#endif
//////////////////////////////////////////////////////////////////////////////////////////

Choreographer*Choreographer::mInst=nullptr;

Choreographer::Choreographer(){
    mFrameScheduled=false;
    mCallbacksRunning=false;
    mLastFrameTimeNanos=0;
    mFrameIntervalNanos=0;
}

Choreographer&Choreographer::getInstance(){
    if(mInst==nullptr)
        mInst=new Choreographer();
    return *mInst;
}   

long Choreographer::getFrameTime(){
    return SystemClock::uptimeMillis();
}

long Choreographer::getFrameTimeNanos(){
    return SystemClock::uptimeMillis();    
}

long Choreographer::getLastFrameTimeNanos(){
    return mLastFrameTimeNanos;
}

long Choreographer::getFrameIntervalNanos() {
    return mFrameIntervalNanos;
}

void Choreographer::removeCallbacksInternal(int callbackType,Runnable action, void* token){

}

void Choreographer::removeFrameCallback(FrameCallback callback){

}

void Choreographer::postFrameCallback(FrameCallback callback){

}

}
