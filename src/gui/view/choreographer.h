#ifndef __CHOREO_GRAPHER_H__
#define __CHOREO_GRAPHER_H__
#include <core/looper.h>
#include <core/neverdestroyed.h>
#include <drawables/drawable.h>
namespace cdroid{
class Choreographer:protected EventHandler{
public:
    enum CallbackType{
        CALLBACK_INPUT = 0,
        CALLBACK_ANIMATION=1,
        CALLBACK_TRAVERSAL=2,
        CALLBACK_COMMIT= 3,
        CALLBACK_LAST  = CALLBACK_COMMIT
    };
    typedef CallbackBase<void,int64_t>FrameCallback;
    static constexpr long DEFAULT_FRAME_DELAY=33;
private:
    class CallbackRecord{
    public:
        CallbackRecord*next;
        int64_t dueTime;
        Runnable action;
        FrameCallback frameCallback;
        void* token;
    public:
        CallbackRecord();
        void run(int64_t frameTimeNanos);
    };
    class CallbackQueue{
    public:
        Choreographer*mChoreographer;
        CallbackRecord*mHead;
    public:
        CallbackQueue(Choreographer*choreographer);
        bool hasDueCallbacksLocked(int64_t now)const;
        CallbackRecord* extractDueCallbacksLocked(int64_t now);
        void addCallbackLocked(int64_t dueTime, void* action,void* token);
        int removeCallbacksLocked(void* action, void* token);
    };
private:
    Looper *mLooper;
    bool mFrameScheduled;
    bool mCallbacksRunning;
    nsecs_t mLastFrameTimeNanos;
    nsecs_t mFrameIntervalNanos;
    CallbackRecord* mCallbackPool;
    CallbackQueue* mCallbackQueues[CALLBACK_LAST+1];
    static long sFrameDelay;
    friend NeverDestroyed<Choreographer>;
    Choreographer();
    static float getRefreshRate();
    CallbackRecord* obtainCallbackLocked(int64_t dueTime,void* action,void* token);
    void recycleCallbackLocked(CallbackRecord* callback);
    int removeCallbacksInternal(int callbackType,void* action, void* token);
    void postCallbackDelayedInternal(int callbackType,void* action, void* token, int64_t delayMillis);
    void scheduleFrameLocked(int64_t);
protected:
    int checkEvents()override;
    int handleEvents()override;
    void doFrame(nsecs_t frameTimeNanos,int frame);
    void doCallbacks(int callbackType, long frameTimeMillis);
public:
    static Choreographer& getInstance();
    static long getFrameDelay();
    static void setFrameDelay(long frameDelay);
    static int64_t subtractFrameDelay(long delayMillis);
    nsecs_t getFrameTimeNanos()const;
    nsecs_t getLastFrameTimeNanos()const;
    long getFrameTime()const;
    nsecs_t getFrameIntervalNanos()const;
    void postCallback(int callbackType,const Runnable& action, void* token);
    void postCallbackDelayed(int callbackType,const Runnable& action,void*token,long delayMillis);
    int removeCallbacks(int callbackType, const Runnable* action,void*token);
    int removeFrameCallback(const FrameCallback& callback);
    void postFrameCallbackDelayed(const FrameCallback& callback, long delayMillis);
    void postFrameCallback(const FrameCallback& callback);
};
}

#endif
