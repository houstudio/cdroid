#include <animation/animator.h>

namespace cdroid{
Animator::~Animator(){
}
Animator*Animator::clone(){
    return nullptr;
}

void Animator::start() {
}

void Animator::cancel() {
}

void Animator::end() {
}

void Animator::pause() {
    if (isStarted() && !mPaused) {
        mPaused = true;
        for (auto l:mPauseListeners) {
            if(l.onAnimationPause)l.onAnimationPause(*this);
        }
    }
}

void Animator::resume() {
    if (mPaused) {
        mPaused = false;
        for (auto l:mPauseListeners)
            if(l.onAnimationResume)l.onAnimationResume(*this);
    }
}

bool Animator::isPaused() {
   return mPaused;
}

long Animator::getTotalDuration() {
    long duration = getDuration();
    if (duration == DURATION_INFINITE) {
        return DURATION_INFINITE;
    } else {
        return getStartDelay() + duration;
    }
}

TimeInterpolator* Animator::getInterpolator() {
    return nullptr;
}

bool Animator::isStarted() {
    // Default method returns value for isRunning(). Subclasses should override to return a
    // real value.
    return isRunning();
}

std::shared_ptr<ConstantState<Animator*>> Animator::createConstantState(){
    return nullptr;//new AnimatorConstantState(this);
}

void Animator::addListener(AnimatorListener listener) {
    mListeners.push_back(listener);
}

void Animator::removeListener(AnimatorListener listener){
}

std::vector<Animator::AnimatorListener> Animator::getListeners() {
    return mListeners;
}

void Animator::addPauseListener(AnimatorPauseListener listener) {
    mPauseListeners.push_back(listener);
}

void Animator::removePauseListener(AnimatorPauseListener listener){
}

void Animator::removeAllListeners() {
    mListeners.clear();
    mPauseListeners.clear();
}

int Animator::getChangingConfigurations() {
    return mChangingConfigurations;
}

void Animator::setChangingConfigurations(int configs) {
    mChangingConfigurations = configs;
}

void Animator::appendChangingConfigurations(int configs) {
    mChangingConfigurations |= configs;
}

void Animator::setupStartValues() {
}

void Animator::setupEndValues() {
}

void Animator::setTarget(void*target){
}

bool Animator::canReverse() {
    return false;
}

void Animator::reverse(){
}

bool Animator::pulseAnimationFrame(long frameTime) {
    // TODO: Need to find a better signal than this. There's a bug in SystemUI that's preventing
    // returning !isStarted() from working.
    return false;
}

void Animator::startWithoutPulsing(bool inReverse) {
    if (inReverse) {
        reverse();
    } else {
        start();
    }
}

void Animator::skipToEndValue(bool inReverse) {
}

bool Animator::isInitialized() {
    return true;
}

void Animator::animateBasedOnPlayTime(long currentPlayTime, long lastPlayTime, bool inReverse) {
}

AnimatorListenerAdapter::AnimatorListenerAdapter(){
    onAnimationCancel=onAnimationRepeat=onAnimationPause=onAnimationResume=[](Animator&anim){};
    onAnimationEnd=onAnimationStart=[](Animator&aim,bool reverse){};
}

}//endof namespace
