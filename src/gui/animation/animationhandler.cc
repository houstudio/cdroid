#include <animation/animationhandler.h>
#include <core/choreographer.h>
#include <core/systemclock.h>

namespace cdroid{


void AnimationHandler::MyFrameCallbackProvider::postFrameCallback(const Choreographer::FrameCallback& callback) {
    Choreographer::getInstance().postFrameCallback(callback);
}

void AnimationHandler::MyFrameCallbackProvider::postCommitCallback(const Runnable& runnable) {
    Choreographer::getInstance().postCallback(Choreographer::CALLBACK_COMMIT, runnable, nullptr);
}

long AnimationHandler::MyFrameCallbackProvider::getFrameTime() {
    return Choreographer::getInstance().getFrameTime();
}

long AnimationHandler::MyFrameCallbackProvider::getFrameDelay() {
    return Choreographer::getInstance().getFrameDelay();
}

void AnimationHandler::MyFrameCallbackProvider::setFrameDelay(long delay) {
    Choreographer::setFrameDelay(delay);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AnimationHandler*AnimationHandler::mInst=nullptr;

AnimationHandler::AnimationHandler(){
    mFrameCallback=std::bind(&AnimationHandler::doFrame,this,std::placeholders::_1);
}

void AnimationHandler::doFrame(long){
}

void AnimationHandler::doAnimationFrame(long frameTime){
    long currentTime = SystemClock::uptimeMillis();
    int size = mAnimationCallbacks.size();
    for (auto callback:mAnimationCallbacks) {
        if (callback == nullptr) continue;

        if (isCallbackDue(callback, currentTime)) {
            callback->doAnimationFrame(frameTime);
            auto it=std::find(mCommitCallbacks.begin(),mCommitCallbacks.end(),callback);
            if (it!=mCommitCallbacks.end()){
                Runnable runner;
                runner=[this,callback](){
                    commitAnimationFrame(callback, getProvider()->getFrameTime());
                };
                getProvider()->postCommitCallback(runner);
            }
        }
    }
    cleanUpList();
}

bool AnimationHandler::isCallbackDue(AnimationFrameCallback* callback, long currentTime){
    auto it=mDelayedCallbackStartTime.find(callback);
    if(it==mDelayedCallbackStartTime.end())return false;
    if (it->second < currentTime) {
        mDelayedCallbackStartTime.erase(it);
        return true;
    }
    return false;    
}

void AnimationHandler::commitAnimationFrame(AnimationFrameCallback* callback, long frameTime){
    auto it=mDelayedCallbackStartTime.find(callback);
    auto itc=std::find(mCommitCallbacks.begin(),mCommitCallbacks.end(),callback);
    if (it==mDelayedCallbackStartTime.end() && itc!=mCommitCallbacks.end()) {
        callback->commitAnimationFrame(frameTime);
        mCommitCallbacks.erase(itc);
    }
}

void AnimationHandler::cleanUpList(){
    if (mListDirty) {
       for (auto it=mAnimationCallbacks.begin();it!=mAnimationCallbacks.end();it++){
           if ((*it) == nullptr) {
               it=mAnimationCallbacks.erase(it);
           }
       }
       mListDirty = false;
    }
}

AnimationHandler&AnimationHandler::getInstance(){
    if(mInst==nullptr)mInst=new AnimationHandler();
    return *mInst;
}

AnimationHandler::AnimationFrameCallbackProvider*AnimationHandler::getProvider(){
    if(mProvider==nullptr)
        mProvider=new MyFrameCallbackProvider();
    return mProvider;
}
    
void AnimationHandler::setProvider(const AnimationFrameCallbackProvider* provider){
    if(provider==nullptr)
        mProvider=new MyFrameCallbackProvider();
    else
        mProvider=(AnimationFrameCallbackProvider*)provider;
}

void AnimationHandler::addAnimationFrameCallback(const AnimationFrameCallback* callback, long delay){
    if(mAnimationCallbacks.size()==0)
       getProvider()->postFrameCallback(mFrameCallback);
    auto it=std::find(mAnimationCallbacks.begin(),mAnimationCallbacks.end(),(AnimationFrameCallback*)callback);
    if(it==mAnimationCallbacks.end())
        mAnimationCallbacks.push_back((AnimationFrameCallback*)callback);
    if(delay>0)
        mDelayedCallbackStartTime.insert(std::pair<AnimationFrameCallback*,long>
            ((AnimationFrameCallback*)callback,delay));
}

void AnimationHandler::addOneShotCommitCallback(const AnimationFrameCallback* callback){
    auto it=std::find(mCommitCallbacks.begin(),mCommitCallbacks.end(),(AnimationFrameCallback*)callback);
    if (it==mCommitCallbacks.end()) {
        mCommitCallbacks.push_back((AnimationFrameCallback*)callback);
    } 
}

void AnimationHandler::removeCallback(const AnimationFrameCallback* callback){
    auto it1=std::find(mCommitCallbacks.begin(),mCommitCallbacks.end(),(AnimationFrameCallback*)callback);
    auto it2=mDelayedCallbackStartTime.find((AnimationFrameCallback*)callback);
    mCommitCallbacks.erase(it1);
    mDelayedCallbackStartTime.erase(it2);
    
    auto it3=std::find(mAnimationCallbacks.begin(),mAnimationCallbacks.end(),(AnimationFrameCallback*)callback);
    if (it3!=mAnimationCallbacks.end()) {
        (*it3)=nullptr;
        mListDirty = true;
    }
}

}//endof namespace
