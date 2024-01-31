#ifndef __CHOREO_GRAPHER_H__
#define __CHOREO_GRAPHER_H__
#include <core/looper.h>
#include <core/neverdestroyed.h>
#include <drawables/drawable.h>
namespace cdroid{
class Choreographer{
public:
    enum CallbackType{
        CALLBACK_INPUT=0,
        CALLBACK_ANIMATION=1,
        CALLBACK_TRAVERSAL=2,
        CALLBACK_COMMIT=3,
        CALLBACK_LAST=CALLBACK_COMMIT
    };
typedef std::function<void(long)>FrameCallback;
private:
    bool mFrameScheduled;
    bool mCallbacksRunning;
    long mLastFrameTimeNanos;
    long mFrameIntervalNanos;
    class CallbackQueue* mCallbackQueues[CALLBACK_LAST];
    static long sFrameDelay;
    friend NeverDestroyed<Choreographer>;
    Choreographer();
    void removeCallbacksInternal(int callbackType,const Runnable* action, void* token);
    void postCallbackDelayedInternal(int callbackType,void* action, void* token, long delayMillis);
    void scheduleFrameLocked(long);
protected:
    void doCallbacks(int callbackType, long frameTimeNanos);
public:
    static Choreographer& getInstance();
    static long getFrameDelay();
    static void setFrameDelay(long frameDelay);
    long getFrameTimeNanos();
    long getLastFrameTimeNanos();
    long getFrameTime();
    long getFrameIntervalNanos();
    void postCallback(int callbackType, Runnable& action, void* token);
    void postCallbackDelayed(int callbackType,Runnable& action,void*token,long delayMillis);
    void removeCallbacks(int callbackType, const Runnable* action,void*token);
    void removeFrameCallback(const FrameCallback& callback);
    void postFrameCallbackDelayed(const FrameCallback& callback, long delayMillis);
    void postFrameCallback(const FrameCallback& callback);
};
}

#endif
