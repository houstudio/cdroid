#include <view/choreographer.h>
#include <systemclock.h>
#include <cdlog.h>

namespace cdroid{
#define DEFAULT_FRAME_DELAY 20
#define FRAME_CALLBACK_TOKEN 1
 
long Choreographer::sFrameDelay = DEFAULT_FRAME_DELAY;
#define USE_FRAME_TIME 1
Choreographer::Choreographer(){
    mLooper = nullptr;
    mFrameScheduled  = false;
    mCallbacksRunning= false;
    mLastFrameTimeNanos = 0;
    mFrameIntervalNanos = (1000000000 / getRefreshRate());
    mCallbackPool = nullptr;
    for(int i = 0;i <= CALLBACK_LAST;i++){
        mCallbackQueues[i] = new CallbackQueue(this);
    }
}

float Choreographer::getRefreshRate() {
#if 0
    DisplayInfo di = DisplayManagerGlobal.getInstance().getDisplayInfo(
            Display::DEFAULT_DISPLAY);
    return di.getMode().getRefreshRate();
#else
    return 20.f;
#endif
}

static NeverDestroyed<Choreographer>mInst;

Choreographer& Choreographer::getInstance(){
    if(mInst->mLooper==nullptr){
        mInst->mLooper = Looper::getMainLooper();
        mInst->mLooper->addEventHandler(mInst.get());
    }
    return *mInst;
}   

long Choreographer::getFrameDelay(){
    return sFrameDelay;
}

void Choreographer::setFrameDelay(long frameDelay){
    sFrameDelay = frameDelay;
}

long Choreographer::getFrameTime()const{
    return getFrameTimeNanos()/SystemClock::NANOS_PER_MS;
}

long Choreographer::subtractFrameDelay(long delayMillis) {
    const long frameDelay = sFrameDelay;
    return delayMillis <= frameDelay ? 0 : delayMillis - frameDelay;
}

long Choreographer::getFrameTimeNanos()const{
    return USE_FRAME_TIME ? mLastFrameTimeNanos:SystemClock::uptimeNanos();
}

long Choreographer::getLastFrameTimeNanos()const{
    return USE_FRAME_TIME ? mLastFrameTimeNanos:SystemClock::uptimeNanos();
}

long Choreographer::getFrameIntervalNanos()const{
    return mFrameIntervalNanos;
}

Choreographer::CallbackRecord* Choreographer::obtainCallbackLocked(long dueTime,void* action, void* token) {
    CallbackRecord* callback = mCallbackPool;
    if (callback == nullptr) {
        callback = new CallbackRecord();
    } else {
        mCallbackPool = callback->next;
        callback->next= nullptr;
    }
    callback->dueTime= dueTime;
    if(((long)token) == FRAME_CALLBACK_TOKEN){
        callback->frameCallback = *((FrameCallback*)action);
    }else{
        callback->action = *((Runnable*)action);
    }
    callback->token = token;
    return callback;
}

void Choreographer::recycleCallbackLocked(CallbackRecord* callback) {
    callback->action= nullptr;
    callback->token = nullptr;
    callback->next  = mCallbackPool;
    mCallbackPool   = callback;
}

void Choreographer::postCallbackDelayedInternal(int callbackType,void* action, void* token, long delayMillis){
    LOGV("type=%d ,action=%p,token=%p ,delayMillis=%d",callbackType,action,token,delayMillis);

    const long now = SystemClock::uptimeMillis();
    const long dueTime = now + delayMillis;
    mCallbackQueues[callbackType]->addCallbackLocked(dueTime, action, token);
    
    /*if (dueTime <= now) {
        scheduleFrameLocked(now);
    } else {
        Message msg = mHandler.obtainMessage(MSG_DO_SCHEDULE_CALLBACK, action);
        msg.arg1 = callbackType;
        msg.setAsynchronous(true);
        mHandler.sendMessageAtTime(msg, dueTime);
    }*/
}

void Choreographer::removeCallbacks(int callbackType,const Runnable* action, void* token){
    removeCallbacksInternal(callbackType,(void*)action,token);
}

void Choreographer::removeCallbacksInternal(int callbackType,void* action, void* token){
    mCallbackQueues[callbackType]->removeCallbacksLocked(action,token);
}

void Choreographer::postCallback(int callbackType,const Runnable& action, void* token){
     postCallbackDelayed(callbackType,action,token,0);
}

void Choreographer::postCallbackDelayed(int callbackType,const Runnable& action,void*token,long delayMillis){
    postCallbackDelayedInternal(callbackType,(void*)&action, token, delayMillis);
}

void Choreographer::removeFrameCallback(const FrameCallback& callback){
    removeCallbacksInternal(CALLBACK_ANIMATION,(void*)&callback,(void*)FRAME_CALLBACK_TOKEN);
}

void Choreographer::postFrameCallback(const FrameCallback& callback){
    postFrameCallbackDelayed(callback, 0);
}

void Choreographer::postFrameCallbackDelayed(const FrameCallback& callback, long delayMillis){
    FATAL_IF(callback==nullptr,"callback must not be null");

    postCallbackDelayedInternal(CALLBACK_ANIMATION,(void*)&callback, (void*)FRAME_CALLBACK_TOKEN, delayMillis);    
}

void Choreographer::scheduleFrameLocked(long now){
    if (!mFrameScheduled) {
        mFrameScheduled = true;
        nsecs_t nextFrameTime = std::max(mLastFrameTimeNanos /SystemClock::NANOS_PER_MS + sFrameDelay, nsecs_t(now));
        LOG(DEBUG)<<"Scheduling next frame in " << (nextFrameTime - now) << " ms.";
        //Message msg = mHandler.obtainMessage(MSG_DO_FRAME);
        //msg.setAsynchronous(true);
        //mHandler.sendMessageAtTime(msg, nextFrameTime);
    }
}

int Choreographer::checkEvents(){
    const nsecs_t now = SystemClock::uptimeNanos();
    const nsecs_t last= getLastFrameTimeNanos();
    return (now - last)>=getFrameIntervalNanos();
}

int Choreographer::handleEvents(){
    const nsecs_t now = SystemClock::uptimeNanos();
    doFrame(now,0);
    return 0;
}

void Choreographer::doFrame(long frameTimeNanos,int frame){
    mFrameScheduled = false;
    mLastFrameTimeNanos = frameTimeNanos;
    //LOGV("mLastFrameTimeNanos=%lld",mLastFrameTimeNanos);
    doCallbacks(Choreographer::CALLBACK_INPUT, frameTimeNanos);

    //mFrameInfo.markAnimationsStart();
    doCallbacks(Choreographer::CALLBACK_ANIMATION, frameTimeNanos);

    //mFrameInfo.markPerformTraversalsStart();
    doCallbacks(Choreographer::CALLBACK_TRAVERSAL, frameTimeNanos);

    doCallbacks(Choreographer::CALLBACK_COMMIT, frameTimeNanos);    
}

void Choreographer::doCallbacks(int callbackType, long frameTimeNanos){
    CallbackRecord* callbacks;
    /*synchronized (mLock)*/{
        // We use "now" to determine when callbacks become due because it's possible
        // for earlier processing phases in a frame to post callbacks that should run
        // in a following phase, such as an input event that causes an animation to start.
        const nsecs_t now = SystemClock::uptimeNanos();
        callbacks = mCallbackQueues[callbackType]->extractDueCallbacksLocked(now/SystemClock::NANOS_PER_MS);
        if (callbacks == nullptr) {
            return;
        }
        mCallbacksRunning = true;

        // Update the frame time if necessary when committing the frame.
        // We only update the frame time if we are more than 2 frames late reaching
        // the commit phase.  This ensures that the frame time which is observed by the
        // callbacks will always increase from one frame to the next and never repeat.
        // We never want the next frame's starting frame time to end up being less than
        // or equal to the previous frame's commit frame time.  Keep in mind that the
        // next frame has most likely already been scheduled by now so we play it
        // safe by ensuring the commit time is always at least one frame behind.
        if (callbackType == Choreographer::CALLBACK_COMMIT) {
            const nsecs_t jitterNanos = now - frameTimeNanos;
            if (jitterNanos >= 2 * mFrameIntervalNanos) {
                const long lastFrameOffset = jitterNanos % mFrameIntervalNanos + mFrameIntervalNanos;
                LOGV("Commit callback delayed by %.f ms which is more than twice the frame interval of %.f ms"
                     "Setting frame time to  %.f ms in the past.",
                    (jitterNanos * 0.000001f), (mFrameIntervalNanos * 0.000001f) , (lastFrameOffset * 0.000001f));
                //mDebugPrintNextFrameTimeDelta = true;
                frameTimeNanos = now - lastFrameOffset;
                mLastFrameTimeNanos = frameTimeNanos;
            }
        }
    }
    for (CallbackRecord* c = callbacks; c != nullptr; c = c->next) {
        LOGV("RunCallback: type=%d, acction=%p token=%p , latencyMillis=%lld %lld",callbackType,
            c->action , c->token, (SystemClock::uptimeMillis() - c->dueTime),c->dueTime);
        c->run(frameTimeNanos);
    }
    /*synchronized (mLock)*/ {
        mCallbacksRunning = false;
        do {
            CallbackRecord* next = callbacks->next;
            recycleCallbackLocked(callbacks);
            callbacks = next;
        } while (callbacks != nullptr);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////

Choreographer::CallbackRecord::CallbackRecord(){
   next = nullptr;
   token= nullptr;
   dueTime = 0;
}

void Choreographer::CallbackRecord::run(long frameTimeNanos) {
    if (((long int)token) == FRAME_CALLBACK_TOKEN) {
        frameCallback(frameTimeNanos);
    } else {
        if(action)action();
    }
}

Choreographer::CallbackQueue::CallbackQueue(Choreographer*choreographer){
    mHead = nullptr;
    mChoreographer = choreographer;
}

bool Choreographer::CallbackQueue::hasDueCallbacksLocked(long now) const{
    return (mHead != nullptr) && (mHead->dueTime <= now);
}

Choreographer::CallbackRecord* Choreographer::CallbackQueue::extractDueCallbacksLocked(long now) {
    CallbackRecord* callbacks = mHead;
    if ( (callbacks == nullptr) || (callbacks->dueTime > now) ) {
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

void Choreographer::CallbackQueue::addCallbackLocked(long dueTime,void* action,void* token) {
    CallbackRecord* callback = mChoreographer->obtainCallbackLocked(dueTime, action, token);
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

void Choreographer::CallbackQueue::removeCallbacksLocked(void* action, void* token) {
    CallbackRecord* predecessor = nullptr;
    for (CallbackRecord* callback = mHead; callback != nullptr;) {
        CallbackRecord* next = callback->next;
        if ( ((((long)token) == FRAME_CALLBACK_TOKEN) && (callback->frameCallback==*(FrameCallback*)action) )
             ||( ((long)token!=FRAME_CALLBACK_TOKEN) && (action == nullptr || callback->action == *(Runnable*)action)
                     && (token == nullptr || callback->token == token) )) {
            if (predecessor != nullptr) {
                predecessor->next = next;
            } else {
                mHead = next;
            }
            mChoreographer->recycleCallbackLocked(callback);
        } else {
            predecessor = callback;
        }
        callback = next;
    }
}

}//endof namespace
