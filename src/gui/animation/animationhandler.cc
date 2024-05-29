#include <animation/animationhandler.h>
#include <animation/objectanimator.h>
#include <core/systemclock.h>

namespace cdroid{

void AnimationHandler::MyFrameCallbackProvider::postFrameCallback(const Choreographer::FrameCallback& callback) {
    Choreographer::getInstance().postFrameCallback(callback);
}

void AnimationHandler::MyFrameCallbackProvider::postCommitCallback(Runnable& runnable) {
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

AnimationHandler::AnimationHandler(){
    mProvider  = nullptr;
    mListDirty = false;
	mInLooper = false;
    mFrameCallback = std::bind(&AnimationHandler::doFrame,this,std::placeholders::_1);
}

AnimationHandler::~AnimationHandler(){
    delete mProvider;
}

int AnimationHandler::checkEvents(){
    return mAnimationCallbacks.size();
}

int AnimationHandler::handleEvents(){
    const long currentTime = getProvider()->getFrameTime();
    doAnimationFrame(currentTime);
    return 1;
}

void AnimationHandler::doFrame(long){
    LOGD("not used,frame refresh callback");
}

void AnimationHandler::doAnimationFrame(long frameTime){
    const int size = mAnimationCallbacks.size();
    for (auto callback:mAnimationCallbacks) {
        if (callback == nullptr) continue;

        if (isCallbackDue(callback, frameTime)) {
            callback->doAnimationFrame(frameTime);/*doAnimationFrame mybe call removeCallback!!!*/
            auto it = std::find(mCommitCallbacks.begin(),mCommitCallbacks.end(),callback);
            if (it != mCommitCallbacks.end()){
                Runnable runner;
                runner = [this,callback,frameTime](){
                    commitAnimationFrame(callback, frameTime);
                };
                getProvider()->postCommitCallback(runner);
            }
        }
    }
    cleanUpList();
}

bool AnimationHandler::isCallbackDue(AnimationFrameCallback* callback, long currentTime){
    auto it = mDelayedCallbackStartTime.find(callback);
    if(it == mDelayedCallbackStartTime.end()) return true;
    if (it->second < currentTime) {
        mDelayedCallbackStartTime.erase(it);
        return true;
    }
    return false;    
}

void AnimationHandler::commitAnimationFrame(AnimationFrameCallback* callback, long frameTime){
    auto it = mDelayedCallbackStartTime.find(callback);
    auto itc = std::find(mCommitCallbacks.begin(),mCommitCallbacks.end(),callback);
    if ((it == mDelayedCallbackStartTime.end()) && (itc != mCommitCallbacks.end()) ) {
        callback->commitAnimationFrame(frameTime);/*commitAnimationFrame mybe call removeCallback!!!*/
        mCommitCallbacks.erase(itc);
    }
}

void AnimationHandler::cleanUpList(){
    if (mListDirty) {
       for (auto it = mAnimationCallbacks.begin();it != mAnimationCallbacks.end();it++){
           if ((*it) == nullptr) {
               it = mAnimationCallbacks.erase(it);
           }
       }
       mListDirty = false;
    }
}

static NeverDestroyed<AnimationHandler>mInst;
AnimationHandler&AnimationHandler::getInstance(){
    if(!mInst->mInLooper){
        Looper::getDefault()->addEventHandler(mInst.get());
		mInst->mInLooper = true;
    }
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

void AnimationHandler::addAnimationFrameCallback(AnimationFrameCallback* callback, long delay){
    if(mAnimationCallbacks.size()==0)
       getProvider()->postFrameCallback(mFrameCallback);
    auto it=std::find(mAnimationCallbacks.begin(),mAnimationCallbacks.end(),callback);

    if(it==mAnimationCallbacks.end())
        mAnimationCallbacks.push_back((AnimationFrameCallback*)callback);
    if(delay>0){
        mDelayedCallbackStartTime.insert({callback,SystemClock::uptimeMillis()+delay});
    }
}

void AnimationHandler::addOneShotCommitCallback(AnimationFrameCallback* callback){
    auto it=std::find(mCommitCallbacks.begin(),mCommitCallbacks.end(),callback);
    if (it==mCommitCallbacks.end()) {
        mCommitCallbacks.push_back((AnimationFrameCallback*)callback);
    } 
}

void AnimationHandler::removeCallback(AnimationFrameCallback* callback){
    auto it1=std::find(mCommitCallbacks.begin(),mCommitCallbacks.end(),callback);
    auto it2=mDelayedCallbackStartTime.find((AnimationFrameCallback*)callback);
    if(it1!=mCommitCallbacks.end())mCommitCallbacks.erase(it1);
    if(it2!=mDelayedCallbackStartTime.end())mDelayedCallbackStartTime.erase(it2);
    
    auto it3=std::find(mAnimationCallbacks.begin(),mAnimationCallbacks.end(),callback);
    if (it3!=mAnimationCallbacks.end()) {
        (*it3)=nullptr;
        mListDirty = true;
    }
}

int AnimationHandler::getCallbackSize()const{
    int count=0;
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
        if(cb==nullptr)continue;
        if(objectAnimator->shouldAutoCancel(cb))
            dynamic_cast<Animator*>(cb)->cancel();
    }
}
}//endof namespace
