#ifndef __CHOREO_GRAPHER_H__
#define __CHOREO_GRAPHER_H__
#include <looper/looper.h>
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
    static Choreographer *mInst;
    Choreographer();
    void removeCallbacksInternal(int callbackType,Runnable action, void* token);
public:
    static Choreographer& getInstance();
    long getFrameTimeNanos();
    long getLastFrameTimeNanos();
    long getFrameTime();
    long getFrameIntervalNanos();

    void postCallbackDelayed(int callbackType,Runnable action,long delayMillis);
    void removeCallbacks(int callbackType, Runnable action);
    void removeFrameCallback(FrameCallback callback);
    void postFrameCallbackDelayed(FrameCallback callback, long delayMillis);
    void postFrameCallback(FrameCallback callback);
};
}

#endif
