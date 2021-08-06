#include <choreographer.h>
#include <systemclock.h>
#include <cdlog.h>

namespace cdroid{
    #define DEFAULT_FRAME_DELAY 30
    #define FRAME_CALLBACK_TOKEN 1
    class CallbackRecord {
        public:
        CallbackRecord* next;
        long dueTime;
        Runnable action; // Runnable or FrameCallback
        long token;
        CallbackRecord(long dtm,Runnable r,long tk){
            dueTime=dtm;
            action=r;
            token=tk;
            next=nullptr;
        }
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
            CallbackRecord* callback = new CallbackRecord(dueTime,action,token);//obtainCallbackLocked(dueTime, action, token);
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
                if ((/*action == nullptr ||*/ callback->action == action)
                    && (token == 0 || callback->token == token)) {
                        if (predecessor != nullptr) {
                            predecessor->next = next;
                        } else {
                            mHead = next;
                        }
                        delete callback;//recycleCallbackLocked(callback);
                    } else {
                        predecessor = callback;
                    }
                callback = next;
            }
        }
};

//////////////////////////////////////////////////////////////////////////////////////////

Choreographer*Choreographer::mInst=nullptr;

Choreographer::Choreographer(){
    mFrameScheduled=false;
    mCallbacksRunning=false;
    mLastFrameTimeNanos=0;
    mFrameIntervalNanos=0;
    for(int i=0;i<CALLBACK_LAST;i++)
    mCallbackQueues[i]=new CallbackQueue;
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
    mCallbackQueues[callbackType]->removeCallbacksLocked(action,(long)token);
}

void Choreographer::removeFrameCallback(FrameCallback callback){
    //removeCallbacksInternal(CALLBACK_ANIMATION, callback,(void*)FRAME_CALLBACK_TOKEN);
}

void Choreographer::postFrameCallback(FrameCallback callback){

}

void Choreographer::postFrameCallbackDelayed(FrameCallback callback, long delayMillis){
}

void Choreographer::scheduleFrameLocked(long now){
    if (!mFrameScheduled) {
        mFrameScheduled = true;
        long nextFrameTime = std::max(mLastFrameTimeNanos /*/ TimeUtils.NANOS_PER_MS*/ + DEFAULT_FRAME_DELAY, now);
        LOG(DEBUG)<<"Scheduling next frame in " << (nextFrameTime - now) << " ms.";
        //Message msg = mHandler.obtainMessage(MSG_DO_FRAME);
        //msg.setAsynchronous(true);
        //mHandler.sendMessageAtTime(msg, nextFrameTime);
    }
}

void Choreographer::doCallbacks(int callbackType, long frameTimeNanos){

}

}//endof namespace
