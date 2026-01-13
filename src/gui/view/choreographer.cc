/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
#include <view/choreographer.h>
#include <systemclock.h>
#include <cdlog.h>

namespace cdroid{
#define FRAME_CALLBACK_TOKEN 1
#define USE_FRAME_TIME 0
 
long Choreographer::sFrameDelay = Choreographer::DEFAULT_FRAME_DELAY;

Choreographer::Choreographer(){
    mLooper = nullptr;
    mFrameScheduled  = false;
    mCallbacksRunning= false;
    mLastFrameTimeNanos = 0;
    mFrameIntervalNanos = static_cast<nsecs_t>(1E9/getRefreshRate());
    mCallbackPool = nullptr;
    for(int i = 0;i <= CALLBACK_LAST;i++){
        mCallbackQueues[i] = new CallbackQueue(this);
    }
    LOGD("%p",this);
}

Choreographer::~Choreographer(){
    for(int i = 0;i <= CALLBACK_LAST;i++){
        delete mCallbackQueues[i];
    }
    int count=0;
    CallbackRecord*next=mCallbackPool;
    while(next){
        CallbackRecord*nn=next->next;
        delete next;
        next=nn;
        count++;
    }
    mLooper->removeEventHandler(this);
    LOGD("%p released %d CallbackRecords",this,count);
}

float Choreographer::getRefreshRate() {
    return 1000.f/sFrameDelay;
}

static NeverDestroyed<Choreographer>mInst;

Choreographer& Choreographer::getInstance(){
    if(mInst->mLooper==nullptr){
        mInst->mLooper = Looper::getMainLooper();
        mInst->mLooper->addEventHandler(mInst.get());
        mInst->mFrameIntervalNanos = static_cast<nsecs_t>(1E9/getRefreshRate());
    }
    return *mInst;
}   

long Choreographer::getFrameDelay(){
    return sFrameDelay;
}

void Choreographer::setFrameDelay(long frameDelay){
    sFrameDelay = frameDelay;
}

int64_t Choreographer::getFrameTime()const{
    FATAL_IF(!mCallbacksRunning,"This method must only be called as "
           "part of a callback while a frame is in progress.");
    return getFrameTimeNanos()/SystemClock::NANOS_PER_MS;
}

int64_t Choreographer::subtractFrameDelay(long delayMillis) {
    const int64_t frameDelay = sFrameDelay;
    return delayMillis <= frameDelay ? 0 : delayMillis - frameDelay;
}

int64_t Choreographer::getFrameTimeNanos()const{
    return USE_FRAME_TIME ? mLastFrameTimeNanos:SystemClock::uptimeNanos();
}

/**
  * Like {@link #getLastFrameTimeNanos}, but always returns the last frame time, not matter
  * whether callbacks are currently running.
  * @return The frame start time of the last frame, in the {@link System#nanoTime()} time base.
  * @hide
  */
int64_t Choreographer::getLastFrameTimeNanos()const{
    return USE_FRAME_TIME ? mLastFrameTimeNanos:SystemClock::uptimeNanos();
}

int64_t Choreographer::getFrameIntervalNanos()const{
    return mFrameIntervalNanos;
}

Choreographer::CallbackRecord* Choreographer::obtainCallbackLocked(int64_t dueTime,void* action, void* token) {
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

void Choreographer::postCallbackDelayedInternal(int callbackType,void* action, void* token, int64_t delayMillis){
    LOGV("type=%d ,action=%p,token=%p ,delayMillis=%d",callbackType,action,token,delayMillis);

    const auto now = SystemClock::uptimeMillis();
    const auto dueTime = now + delayMillis;
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

int Choreographer::removeCallbacks(int callbackType,const Runnable* action, void* token){
    return removeCallbacksInternal(callbackType,(void*)action,token);
}

int Choreographer::removeCallbacksInternal(int callbackType,void* action, void* token){
    return mCallbackQueues[callbackType]->removeCallbacksLocked(action,token);
}

int Choreographer::hasCallbacks(int callbackType, const Runnable* action,void*token)const{
    return mCallbackQueues[callbackType]->hasCallbacksLocked((void*)action,token);
}

void Choreographer::postCallback(int callbackType,const Runnable& action, void* token){
     postCallbackDelayed(callbackType,action,token,0);
}

void Choreographer::postCallbackDelayed(int callbackType,const Runnable& action,void*token,long delayMillis){
    postCallbackDelayedInternal(callbackType,(void*)&action, token, delayMillis);
}

int Choreographer::removeFrameCallback(const FrameCallback& callback){
    return removeCallbacksInternal(CALLBACK_ANIMATION,(void*)&callback,(void*)FRAME_CALLBACK_TOKEN);
}

void Choreographer::postFrameCallback(const FrameCallback& callback){
    postFrameCallbackDelayed(callback, 0);
}

void Choreographer::postFrameCallbackDelayed(const FrameCallback& callback, long delayMillis){
    FATAL_IF(callback==nullptr,"callback must not be null");

    postCallbackDelayedInternal(CALLBACK_ANIMATION,(void*)&callback, (void*)FRAME_CALLBACK_TOKEN, delayMillis);    
}

void Choreographer::scheduleFrameLocked(int64_t now){
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
    return (now - mLastFrameTimeNanos)>=getFrameIntervalNanos();
}

int Choreographer::handleEvents(){
    const nsecs_t now = SystemClock::uptimeNanos();
    doFrame(now,0);
    return 0;
}

void Choreographer::doFrame(nsecs_t frameTimeNanos,int frame){
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
    /*synchronized (mLock)*/
    {
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
        LOGV("RunCallback: type=%d, action=%p token=%p , latencyMillis=%lld %lld",callbackType,
            &c->action , c->token, (SystemClock::uptimeMillis() - c->dueTime),c->dueTime);
        c->run(frameTimeNanos);
    }
    /*synchronized (mLock)*/
    {
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

void Choreographer::CallbackRecord::run(int64_t frameTimeNanos) {
    if (((long int)token) == FRAME_CALLBACK_TOKEN) {
        frameCallback(frameTimeNanos);
    } else {
        if(action)action();
    }
}

bool  Choreographer::CallbackRecord::compare(void*vaction,void*vtoken)const{
    if((long)this->token==FRAME_CALLBACK_TOKEN){
        return ( (vaction==nullptr) || (this->frameCallback==*(FrameCallback*)vaction))
            && ( (vtoken==nullptr) || (this->token==vtoken) );
    }else {
        return ( (vaction==nullptr) || (this->action==*(Runnable*)vaction) )
            && ( (vtoken==nullptr) || (this->token==vtoken) );
    }
}

Choreographer::CallbackQueue::CallbackQueue(Choreographer*choreographer){
    mHead = nullptr;
    mChoreographer = choreographer;
}

bool Choreographer::CallbackQueue::hasDueCallbacksLocked(int64_t now) const{
    return (mHead != nullptr) && (mHead->dueTime <= now);
}

Choreographer::CallbackRecord* Choreographer::CallbackQueue::extractDueCallbacksLocked(int64_t now) {
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

void Choreographer::CallbackQueue::addCallbackLocked(int64_t dueTime,void* action,void* token) {
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

int Choreographer::CallbackQueue::removeCallbacksLocked(void* action, void* token) {
    CallbackRecord* predecessor = nullptr;
    int count = 0;
    for (CallbackRecord* callback = mHead; callback != nullptr ; callback = callback->next) {
#if 0
        if ( ((((long)token) == FRAME_CALLBACK_TOKEN) && (callback->frameCallback==*(FrameCallback*)action) )
             ||( ((long)token!=FRAME_CALLBACK_TOKEN) && (action == nullptr || callback->action == *(Runnable*)action)
                     && (token == nullptr || callback->token == token) )) {
#else
        if(callback->compare(action,token)){
#endif
            if (predecessor != nullptr) {
                predecessor->next = callback->next;
            } else {
                mHead = callback->next;
            }
            count++;
            mChoreographer->recycleCallbackLocked(callback);
        } else {
            predecessor = callback;
        }
    }
    LOGV_IF(count,"removed %d Actions",count);
    return count;
}

int Choreographer::CallbackQueue::hasCallbacksLocked(void* action, void* token)const{
    CallbackRecord* predecessor = nullptr;
    int count = 0;
    for (CallbackRecord* callback = mHead; callback != nullptr;) {
        CallbackRecord* next = callback->next;
        if ( ((((long)token) == FRAME_CALLBACK_TOKEN) && (callback->frameCallback==*(FrameCallback*)action) )
             ||( ((long)token!=FRAME_CALLBACK_TOKEN) && (action == nullptr || callback->action == *(Runnable*)action)
                     && (token == nullptr || callback->token == token) )) {
            count++;
        } else {
            predecessor = callback;
        }
        callback = next;
    }
    LOGV_IF(count,"has %d Actions",count);
    return count;
}

}//endof namespace
