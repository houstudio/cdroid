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
#include <animation/animationhandler.h>
#include <animation/objectanimator.h>
#include <core/systemclock.h>
#include <porting/cdlog.h>

namespace cdroid{

void AnimationHandler::MyFrameCallbackProvider::postFrameCallback(const Choreographer::FrameCallback& callback) {
    Choreographer::getInstance().postFrameCallback(callback);
}

void AnimationHandler::MyFrameCallbackProvider::postCommitCallback(Runnable& runnable) {
    Choreographer::getInstance().postCallback(Choreographer::CALLBACK_COMMIT, runnable, nullptr);
}

int64_t AnimationHandler::MyFrameCallbackProvider::getFrameTime() {
    return Choreographer::getInstance().getFrameTime();
}

long AnimationHandler::MyFrameCallbackProvider::getFrameDelay() {
    return Choreographer::getInstance().getFrameDelay();
}

void AnimationHandler::MyFrameCallbackProvider::setFrameDelay(long delay) {
    Choreographer::setFrameDelay(delay);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AnimationHandler::AnimationHandler(){
    mProvider  = nullptr;
    mListDirty = false;
    mFrameCallback = std::bind(&AnimationHandler::doFrame,this,std::placeholders::_1);
}

AnimationHandler::~AnimationHandler(){
    delete mProvider;
}


void AnimationHandler::doFrame(int64_t frameTimeNanos){
    LOGV("not used,frame refresh callback");
    doAnimationFrame(getProvider()->getFrameTime());
    if( mAnimationCallbacks.size() ){
        getProvider()->postFrameCallback(mFrameCallback);
    }
}

void AnimationHandler::doAnimationFrame(int64_t frameTime){
    const int size = mAnimationCallbacks.size();
    for (auto callback:mAnimationCallbacks) {
        if (callback == nullptr) continue;

        if (isCallbackDue(callback, frameTime)) {
            callback->doAnimationFrame(frameTime);/*doAnimationFrame mybe call removeCallback!!!*/
            auto it = std::find(mCommitCallbacks.begin(),mCommitCallbacks.end(),callback);
            if (it != mCommitCallbacks.end()){
                Runnable runner;
                runner = [this,callback](){
                    commitAnimationFrame(callback, getProvider()->getFrameTime());
                };
                getProvider()->postCommitCallback(runner);
            }
        }
    }
    cleanUpList();
}

bool AnimationHandler::isCallbackDue(AnimationFrameCallback* callback, int64_t currentTime){
    auto it = mDelayedCallbackStartTime.find(callback);
    if(it == mDelayedCallbackStartTime.end()) return true;
    if (it->second < currentTime) {
        mDelayedCallbackStartTime.erase(it);
        return true;
    }
    return false;    
}

void AnimationHandler::commitAnimationFrame(AnimationFrameCallback* callback, int64_t frameTime){
    auto it = mDelayedCallbackStartTime.find(callback);
    auto itc = std::find(mCommitCallbacks.begin(),mCommitCallbacks.end(),callback);
    if ((it == mDelayedCallbackStartTime.end()) && (itc != mCommitCallbacks.end()) ) {
        callback->commitAnimationFrame(frameTime);/*commitAnimationFrame mybe call removeCallback!!!*/
        mCommitCallbacks.erase(itc);
    }
}

void AnimationHandler::cleanUpList(){
    if (mListDirty) {
       for (auto it = mAnimationCallbacks.begin();it != mAnimationCallbacks.end();){
           if ((*it) == nullptr) {
               it = mAnimationCallbacks.erase(it);
           }else it++;
       }
       mListDirty = false;
    }
}

static NeverDestroyed<AnimationHandler>mInst;
AnimationHandler&AnimationHandler::getInstance(){
    return *mInst;
}

AnimationHandler::AnimationFrameCallbackProvider*AnimationHandler::getProvider(){
    if(mProvider==nullptr)
        mProvider = new MyFrameCallbackProvider();
    return mProvider;
}
    
void AnimationHandler::setProvider(const AnimationFrameCallbackProvider* provider){
    if(provider==nullptr)
        mProvider = new MyFrameCallbackProvider();
    else
        mProvider = (AnimationFrameCallbackProvider*)provider;
}

void AnimationHandler::addAnimationFrameCallback(AnimationFrameCallback* callback, long delay){
    if(mAnimationCallbacks.size()==0)
       getProvider()->postFrameCallback(mFrameCallback);
    auto it = std::find(mAnimationCallbacks.begin(),mAnimationCallbacks.end(),callback);

    if(it == mAnimationCallbacks.end())
        mAnimationCallbacks.push_back((AnimationFrameCallback*)callback);
    if(delay>0){
        mDelayedCallbackStartTime.insert({callback,SystemClock::uptimeMillis()+delay});
    }
}

void AnimationHandler::addOneShotCommitCallback(AnimationFrameCallback* callback){
    auto it = std::find(mCommitCallbacks.begin(),mCommitCallbacks.end(),callback);
    if (it == mCommitCallbacks.end()) {
        mCommitCallbacks.push_back((AnimationFrameCallback*)callback);
    } 
}

void AnimationHandler::removeCallback(AnimationFrameCallback* callback){
    auto it1 = std::find(mCommitCallbacks.begin(),mCommitCallbacks.end(),callback);
    auto it2 = mDelayedCallbackStartTime.find((AnimationFrameCallback*)callback);
    if(it1 != mCommitCallbacks.end())mCommitCallbacks.erase(it1);
    if(it2 != mDelayedCallbackStartTime.end())mDelayedCallbackStartTime.erase(it2);
    
    auto it3 = std::find(mAnimationCallbacks.begin(),mAnimationCallbacks.end(),callback);
    if (it3 != mAnimationCallbacks.end()) {
        (*it3) = nullptr;
        mListDirty = true;
    }
}

int AnimationHandler::getCallbackSize()const{
    int count = 0;
    for(auto anim:mAnimationCallbacks)
        if(anim)count++;
    return count;
}

int AnimationHandler::getAnimationCount(){
    return mInst->getCallbackSize();
}    

void AnimationHandler::setFrameDelay(long delay){
    getInstance().getProvider()->setFrameDelay(delay);
}

long AnimationHandler::getFrameDelay() {
    return getInstance().getProvider()->getFrameDelay();
}

void AnimationHandler::autoCancelBasedOn(ObjectAnimator* objectAnimator){
    for(auto cb:mAnimationCallbacks){
        if(cb == nullptr) continue;
        if(objectAnimator->shouldAutoCancel(cb))
            dynamic_cast<Animator*>(cb)->cancel();
    }
}
}//endof namespace
