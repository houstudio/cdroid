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
#include <animation/valueanimator.h>
#include <animation/interpolators.h>
#include <systemclock.h>
#include <cmath>
#include <stdarg.h>
#include <cdlog.h>

namespace cdroid{

float ValueAnimator::sDurationScale = 1.f;
const TimeInterpolator*ValueAnimator::sDefaultInterpolator=AccelerateDecelerateInterpolator::Instance;

ValueAnimator::ValueAnimator()
  :Animator(){
    mReversing = false;
    mRepeatMode = RESTART;
    mSelfPulse = true;
    mRunning = false;
    mResumed = false;
    mStarted = false;
    mStartTime = -1;
    mPauseTime = 0;
    mLastFrameTime = -1;
    mFirstFrameTime= -1;
    mSeekFraction  = -1;
    mOverallFraction = 0.f;
    mCurrentFraction = 0.f;
    mStartTimeCommitted =false;
    mStartListenersCalled = false;
    mAnimationEndRequested = false;
    mSuppressSelfPulseRequested = false;
    mInitialized = false;
    mDuration = 300;
    mStartDelay = 0;
    mRepeatCount = 0;
    mDurationScale = -1.f;
    mInterpolator = sDefaultInterpolator;
}

ValueAnimator::ValueAnimator(const ValueAnimator&o){
    mSeekFraction = -1;
    mReversing = false;
    mInitialized = false;
    mStarted = false;
    mRunning = false;
    mPaused  = false;
    mResumed = false;
    mStartListenersCalled = false;
    mStartTime = -1;
    mPauseTime = -1;
    mLastFrameTime  = -1;
    mFirstFrameTime = -1;
    mStartTimeCommitted = false;
    mAnimationEndRequested = false;
    mOverallFraction = 0;
    mCurrentFraction = 0;
    mSelfPulse = true;
    mSuppressSelfPulseRequested = false;
    mInterpolator = sDefaultInterpolator;
    mDuration   = o.mDuration;
    mReversing  = o.mReversing;
    mRepeatMode = o.mRepeatMode;
    mRepeatCount= o.mRepeatCount;
    mStartDelay = o.mStartDelay;
    mDurationScale = o.mDurationScale;
    auto& oldValues = o.mValues;
    if (oldValues.size()) {
        const int numValues = (int)oldValues.size();
        for (int i = 0; i < numValues; ++i) {
            PropertyValuesHolder* newValuesHolder = new PropertyValuesHolder(*oldValues[i]);//.clone();
            mValues.push_back(newValuesHolder);
            mValuesMap.insert({newValuesHolder->getPropertyName(), newValuesHolder});
        }
    }
}

ValueAnimator::~ValueAnimator(){
    for(auto v:mValues)
        delete v;
    mValues.clear();
    removeAnimationCallback();
}

void ValueAnimator::setDurationScale(float durationScale) {
    sDurationScale = durationScale;
}


float ValueAnimator::getDurationScale() {
    return sDurationScale;
}

bool ValueAnimator::areAnimatorsEnabled() {
    return !(sDurationScale == 0);
}

ValueAnimator* ValueAnimator::ofInt(const std::vector<int>&values){
    ValueAnimator*anim = new ValueAnimator();
    anim->setIntValues(values);
    return anim;
}

ValueAnimator* ValueAnimator::ofArgb(const std::vector<int>&values){
    ValueAnimator*anim = new ValueAnimator();
    anim->setIntValues(values);
    return anim;
}

ValueAnimator* ValueAnimator::ofFloat(const std::vector<float>&values){
    ValueAnimator*anim = new ValueAnimator();
    anim->setFloatValues(values);
    return anim;
}

ValueAnimator* ValueAnimator::ofPropertyValuesHolder(const std::vector<PropertyValuesHolder*>&values){
    ValueAnimator*anim = new ValueAnimator();
    anim->setValues(values);
    return anim;
}

/**
 * Set the animated values for this object to this set of ints.
 * If there is only one value, it is assumed to be the end value of an animation,
 * and an initial value will be derived, if possible, by calling a getter function
 * on the object. Also, if any value is null, the value will be filled in when the animation
 * starts in the same way. This mechanism of automatically getting null values only works
 * if the PropertyValuesHolder object is used in conjunction
 * {@link ObjectAnimator}, and with a getter function
 * derived automatically from <code>propertyName</code>, since otherwise PropertyValuesHolder has
 * no way of determining what the value should be.
 *
 * @param values One or more values that the animation will animate between.
 */
void ValueAnimator::setIntValues(const std::vector<int>&values){
    if(values.empty())return;
    if(mValues.size()==0){
        setValues({PropertyValuesHolder::ofInt("",values)});
    }else{
        IntPropertyValuesHolder*valuesHolder = (IntPropertyValuesHolder*)mValues[0];
        valuesHolder->setValues(values);
    }
}

void ValueAnimator::setFloatValues(const std::vector<float>&values){
    if(values.empty())return;
    if(mValues.size()==0){
        setValues({PropertyValuesHolder::ofFloat("",values)});
    }else{
        FloatPropertyValuesHolder*valuesHolder = (FloatPropertyValuesHolder*)mValues[0];
        valuesHolder->setValues(values);
    }
}

void ValueAnimator::setValues(const std::vector<PropertyValuesHolder*>&values){
    mValues = values;
    mValuesMap.clear();
    for(auto prop:values){
        PropertyValuesHolder*valuesHolder = prop;
        mValuesMap.insert({valuesHolder->getPropertyName(),valuesHolder});
    }
    mInitialized = false;
}

std::vector<PropertyValuesHolder*>&ValueAnimator::getValues(){
    return mValues;
}

const std::vector<PropertyValuesHolder*>&ValueAnimator::getValues()const{
    return mValues;
}

PropertyValuesHolder* ValueAnimator::getValues(int idx){
    return mValues.at(idx);
}

PropertyValuesHolder* ValueAnimator::getValues(const std::string&propName){
    auto itr = mValuesMap.find(propName);
    if(itr==mValuesMap.end())return nullptr;
    return itr->second;
}

void ValueAnimator::initAnimation(){
    if(!mInitialized){
        for(auto& v:mValues){
            v->init();
        }
        mInitialized = true;
    }
}

void ValueAnimator::overrideDurationScale(float durationScale) {
     mDurationScale = durationScale;
}

float ValueAnimator::resolveDurationScale() const{
    return mDurationScale >= 0.f ? mDurationScale : sDurationScale;
}

int64_t ValueAnimator::getScaledDuration() const{
    return int64_t(mDuration * resolveDurationScale());
}

ValueAnimator& ValueAnimator::setDuration(int64_t duration){
    if (duration < 0) {
        throw std::logic_error("Animators cannot have negative duration");
    }
    mDuration = duration;
    return *this;
}

int64_t ValueAnimator::getDuration()const{
    return mDuration;
}

int64_t ValueAnimator::getTotalDuration(){
    if (mRepeatCount == INFINITE) {
        return DURATION_INFINITE;
    } else {
        return mStartDelay + (mDuration * (mRepeatCount + 1));
    }
}

void ValueAnimator::setCurrentPlayTime(int64_t playTime) {
    const float fraction = (mDuration > 0) ? (float) playTime / mDuration : 1;
    setCurrentFraction(fraction);
}

void ValueAnimator::setCurrentFraction(float fraction) {
    initAnimation();
    fraction = clampFraction(fraction);
    mStartTimeCommitted = true; // do not allow start time to be compensated for jank
    if (isPulsingInternal()) {
        const int64_t seekTime = int64_t(getScaledDuration()) * fraction;
        const int64_t currentTime = SystemClock::uptimeMillis();
        // Only modify the start time when the animation is running. Seek fraction will ensure
        // non-running animations skip to the correct start time.
        mStartTime = currentTime - seekTime;
    } else {
        // If the animation loop hasn't started, or during start delay, the startTime will be
        // adjusted once the delay has passed based on seek fraction.
        mSeekFraction = fraction;
    }
    mOverallFraction = fraction;
    const float currentIterationFraction = getCurrentIterationFraction(fraction, mReversing);
    animateValue(currentIterationFraction);
}

int ValueAnimator::getCurrentIteration(float fraction){
    fraction = clampFraction(fraction);
    // If the overall fraction is a positive integer, we consider the current iteration to be
    // complete. In other words, the fraction for the current iteration would be 1, and the
    // current iteration would be overall fraction - 1.
    double iteration = std::floor(fraction);
    if ((fraction == iteration) && (fraction > 0)) {
        iteration--;
    }
    return (int) iteration;
}

float ValueAnimator::getCurrentIterationFraction(float fraction, bool inReverse){
    fraction = clampFraction(fraction);
    int iteration = getCurrentIteration(fraction);
    float currentFraction = fraction - iteration;
    return shouldPlayBackward(iteration, inReverse) ? (1.f - currentFraction) : currentFraction;
}

float ValueAnimator::clampFraction(float fraction){
    if (fraction < 0) {
        fraction = 0;
    } else if (mRepeatCount != INFINITE) {
        fraction = std::min(fraction, (float)(mRepeatCount + 1));
    }
    return fraction;
}

bool ValueAnimator::shouldPlayBackward(int iteration, bool inReverse){
    if (iteration > 0 && mRepeatMode == REVERSE &&
          (iteration < (mRepeatCount + 1) || mRepeatCount == INFINITE)) {
        // if we were seeked to some other iteration in a reversing animator,
        // figure out the correct direction to start playing based on the iteration
        if (inReverse) {
            return (iteration % 2) == 0;
        } else {
            return (iteration % 2) != 0;
        }
    } else {
        return inReverse;
    }
}

int64_t ValueAnimator::getCurrentPlayTime() {
    if (!mInitialized || (!mStarted && mSeekFraction < 0)) {
        return 0;
    }
    if (mSeekFraction >= 0) {
        return mDuration * mSeekFraction;
    }
    float durationScale = resolveDurationScale();
    if (durationScale == 0.f) {
        durationScale = 1.f;
    }
    return ((SystemClock::uptimeMillis() - mStartTime) / durationScale);
}

int64_t ValueAnimator::getStartDelay() {
    return mStartDelay;
}

void ValueAnimator::setStartDelay(int64_t startDelay){
    LOGW_IF(startDelay<0,"Start delay should always be non-negative");
    mStartDelay = startDelay>0?startDelay:0;
}

long ValueAnimator::getFrameDelay(){
    return AnimationHandler::getInstance().getFrameDelay();
}

void ValueAnimator::setFrameDelay(long frameDelay){
    AnimationHandler::getInstance().setFrameDelay(frameDelay);
}

AnimateValue ValueAnimator::getAnimatedValue(){
    if(mValues.size())return mValues[0]->getAnimatedValue();
    return AnimateValue(0);
}

AnimateValue ValueAnimator::getAnimatedValue(const std::string&propertyName){
    auto it = mValuesMap.find(propertyName);
    if(it != mValuesMap.end())
        return it->second->getAnimatedValue();
    return AnimateValue(0);
}

void ValueAnimator::setRepeatCount(int value) {
    mRepeatCount = value;
}

int ValueAnimator::getRepeatCount()const{
    return mRepeatCount;
}

void ValueAnimator::setRepeatMode(int value) {
    mRepeatMode = value;
}

int ValueAnimator::getRepeatMode()const{
    return mRepeatMode;
}

void ValueAnimator::addUpdateListener(const AnimatorUpdateListener& listener){
    mUpdateListeners.push_back(listener);
}

void ValueAnimator::removeUpdateListener(const AnimatorUpdateListener& listener){
    for(auto it = mUpdateListeners.begin();it!=mUpdateListeners.end();it++){
        if( (*it) == listener){
             mUpdateListeners.erase(it);
             break;
        }
    }
}

void ValueAnimator::removeAllUpdateListeners(){
    mUpdateListeners.clear();
}

void ValueAnimator::setInterpolator(const TimeInterpolator* value){
    if(value)
        mInterpolator = value;
    else
        mInterpolator = LinearInterpolator::Instance;//new LinearInterpolator();
}

const TimeInterpolator* ValueAnimator::getInterpolator()const{
    return mInterpolator;
}

bool ValueAnimator::isPulsingInternal(){
    return mLastFrameTime >= 0;
}

void ValueAnimator::setEvaluator(TypeEvaluator value){
}

void ValueAnimator::notifyStartListeners() {
    if (mListeners.size() && !mStartListenersCalled) {
        std::vector<AnimatorListener>tmpListeners = mListeners;
        for (auto l:tmpListeners){
            if(l.onAnimationStart)l.onAnimationStart(*this, mReversing);
        }
    }
    mStartListenersCalled = true;
}

void ValueAnimator::start(bool playBackwards){
    mReversing = playBackwards;
    mSelfPulse = !mSuppressSelfPulseRequested;
    // Special case: reversing from seek-to-0 should act as if not seeked at all.
    if (playBackwards && (mSeekFraction != -1) && (mSeekFraction != 0) ) {
        if (mRepeatCount == INFINITE) {
            // Calculate the fraction of the current iteration.
            float fraction = (float) (mSeekFraction - std::floor(mSeekFraction));
            mSeekFraction = 1.f - fraction;
        } else {
            mSeekFraction = 1.f + mRepeatCount - mSeekFraction;
        }
    }
    mStarted = true;
    mPaused = false;
    mRunning = false;
    mAnimationEndRequested = false;
    // Resets mLastFrameTime when start() is called, so that if the animation was running,
    // calling start() would put the animation in the
    // started-but-not-yet-reached-the-first-frame phase.
    mLastFrameTime = -1;
    mFirstFrameTime = -1;
    mStartTime = -1;
    addAnimationCallback(0);

    if ( (mStartDelay == 0) || (mSeekFraction >= 0) || mReversing) {
        // If there's no start delay, init the animation and notify start listeners right away
        // to be consistent with the previous behavior. Otherwise, postpone this until the first
        // frame after the start delay.
        startAnimation();
        if (mSeekFraction == -1) {
            // No seek, start at play time 0. Note that the reason we are not using fraction 0
            // is because for animations with 0 duration, we want to be consistent with pre-N
            // behavior: skip to the final value immediately.
             setCurrentPlayTime(0);
        } else {
            setCurrentFraction(mSeekFraction);
        }
    }
}

void ValueAnimator::start(){
    start(false);
}

void ValueAnimator::startWithoutPulsing(bool inReverse){
    mSuppressSelfPulseRequested = true;
    if (inReverse) {
        reverse();
    } else {
        start();
    }
    mSuppressSelfPulseRequested = false;
}

void ValueAnimator::cancel(){
    if (mAnimationEndRequested) {
        return;
    }
    // Only cancel if the animation is actually running or has been started and is about
    // to run
    // Only notify listeners if the animator has actually started
    if ((mStarted || mRunning) && mListeners.size()) {
        if (!mRunning) {// If it's not yet running, then start listeners weren't called. Call them now.
            notifyStartListeners();
        }
        std::vector<AnimatorListener>tmpListeners = mListeners;
        for (AnimatorListener ls:tmpListeners) {
            if(ls.onAnimationCancel)
                ls.onAnimationCancel(*this);
        }
    }
    endAnimation();
}

void ValueAnimator::end(){
    if (!mRunning) {
        // Special case if the animation has not yet started; get it ready for ending
        startAnimation();
        mStarted = true;
    } else if (!mInitialized) {
        initAnimation();
    }
    animateValue(shouldPlayBackward(mRepeatCount, mReversing) ? 0.f : 1.f);
    endAnimation();
}

void ValueAnimator::resume(){
    if (mPaused && !mResumed) {
        mResumed = true;
        if (mPauseTime > 0) {
            addAnimationCallback(0);
        }
    }
    Animator::resume();
}

void ValueAnimator::pause() {
    const bool previouslyPaused = mPaused;
    Animator::pause();
    if (!previouslyPaused && mPaused) {
        mPauseTime = -1;
        mResumed = false;
    }
}

bool ValueAnimator::isRunning() {
    return mRunning;
}

bool ValueAnimator::isStarted() {
    return mStarted;
}

void ValueAnimator::reverse() {
    if (isPulsingInternal()) {
        const int64_t currentTime = SystemClock::uptimeMillis();
        const int64_t currentPlayTime = currentTime - mStartTime;
        const int64_t timeLeft = getScaledDuration() - currentPlayTime;
        mStartTime = currentTime - timeLeft;
        mStartTimeCommitted = true; // do not allow start time to be compensated for jank
        mReversing = !mReversing;
    } else if (mStarted) {
        mReversing = !mReversing;
        end();
    } else {
        start(true);
    }
}

bool ValueAnimator::canReverse(){
    return true;
}

void ValueAnimator::startAnimation(){
    mAnimationEndRequested = false;
    initAnimation();
    mRunning = true;
    if (mSeekFraction >= 0) {
        mOverallFraction = mSeekFraction;
    } else {
        mOverallFraction = 0.f;
    }
    notifyStartListeners();
}

void ValueAnimator::endAnimation(){
    if (mAnimationEndRequested) {
        return;
    }
    removeAnimationCallback();

    mAnimationEndRequested = true;
    mPaused = false;
    const bool notify = (mStarted || mRunning) && mListeners.size();
    if (notify && !mRunning) {
        // If it's not yet running, then start listeners weren't called. Call them now.
        notifyStartListeners();
    }
    mRunning = false;
    mStarted = false;
    mStartListenersCalled = false;
    mLastFrameTime = -1;
    mFirstFrameTime = -1;
    mStartTime = -1;
    if (notify && mListeners.size()) {
        std::vector<AnimatorListener>tmpListeners = mListeners;
        for (AnimatorListener l:tmpListeners) {
            if(l.onAnimationEnd)l.onAnimationEnd(*this, mReversing);
        }
    }
    // mReversing needs to be reset *after* notifying the listeners for the end callbacks.
    mReversing = false;
}

void ValueAnimator::commitAnimationFrame(int64_t frameTime){
    if (!mStartTimeCommitted) {
        mStartTimeCommitted = true;
        const int64_t adjustment = frameTime - mLastFrameTime;
        if (adjustment > 0) {
            mStartTime += adjustment;
            LOGV("%p Adjusted start time by %d ms",this,adjustment);
        }
    }
}

bool ValueAnimator::animateBasedOnTime(int64_t currentTime){
    bool done = false;
    if (mRunning) {
        const int64_t scaledDuration = getScaledDuration();
        const float fraction = scaledDuration > 0 ? (float)(currentTime - mStartTime) / scaledDuration : 1.f;
        const float lastFraction = mOverallFraction;
        const bool newIteration = (int) fraction > (int) lastFraction;
        const bool lastIterationFinished = (fraction >= mRepeatCount + 1) && (mRepeatCount != INFINITE);
        if (scaledDuration == 0) {
            // 0 duration animator, ignore the repeat count and skip to the end
            done = true;
        } else if (newIteration && !lastIterationFinished) {
            // Time to repeat
            for (AnimatorListener l:mListeners) {
                if(l.onAnimationRepeat)l.onAnimationRepeat(*this);
            }
        } else if (lastIterationFinished) {
            done = true;
        }
        mOverallFraction = clampFraction(fraction);
        const float currentIterationFraction = getCurrentIterationFraction(mOverallFraction, mReversing);
        animateValue(currentIterationFraction);
    }
    return done;
}

void ValueAnimator::animateBasedOnPlayTime(int64_t currentPlayTime, int64_t lastPlayTime, bool inReverse){
    if ((currentPlayTime < 0) || (lastPlayTime < 0)) {
        throw std::logic_error("Error: Play time should never be negative.");
    }
    initAnimation();
    // Check whether repeat callback is needed only when repeat count is non-zero
    if (mRepeatCount > 0) {
        int iteration = (int) (currentPlayTime / mDuration);
        int lastIteration = (int) (lastPlayTime / mDuration);

        // Clamp iteration to [0, mRepeatCount]
        iteration = std::min(iteration, mRepeatCount);
        lastIteration = std::min(lastIteration, mRepeatCount);

        if (iteration != lastIteration) {
            for (AnimatorListener l:mListeners) {
                if(l.onAnimationRepeat)l.onAnimationRepeat(*this);
            }
        }
    }
    if ((mRepeatCount != INFINITE) && (currentPlayTime >= ((mRepeatCount + 1) * mDuration))) {
        skipToEndValue(inReverse);
    } else {
        // Find the current fraction:
        float fraction = currentPlayTime / (float) mDuration;
        fraction = getCurrentIterationFraction(fraction, inReverse);
        animateValue(fraction);
    }
}

void ValueAnimator::skipToEndValue(bool inReverse){
    initAnimation();
    float endFraction = inReverse ? 0.f : 1.f;
    if ((mRepeatCount % 2 == 1) && (mRepeatMode == REVERSE)) {
        // This would end on fraction = 0
        endFraction = 0.f;
    }
    animateValue(endFraction);
}

bool ValueAnimator::isInitialized(){
    return mInitialized;
}

bool ValueAnimator::doAnimationFrame(int64_t frameTime){
    if (mStartTime < 0) {
        // First frame. If there is start delay, start delay count down will happen *after* this
        // frame.
        mStartTime = mReversing ? frameTime : (frameTime + (mStartDelay * resolveDurationScale()));
    }

    // Handle pause/resume
    if (mPaused) {
        mPauseTime = frameTime;
        removeAnimationCallback();
        return false;
    } else if (mResumed) {
        mResumed = false;
        if (mPauseTime > 0) {
            // Offset by the duration that the animation was paused
            mStartTime += (frameTime - mPauseTime);
        }
    }

    if (!mRunning) {
        // If not running, that means the animation is in the start delay phase of a forward
        // running animation. In the case of reversing, we want to run start delay in the end.
        if ((mStartTime > frameTime) && (mSeekFraction == -1)) {
            // This is when no seek fraction is set during start delay. If developers change the
            // seek fraction during the delay, animation will start from the seeked position
            // right away.
            return false;
        } else {
            // If mRunning is not set by now, that means non-zero start delay,
            // no seeking, not reversing. At this point, start delay has passed.
            mRunning = true;
            startAnimation();
        }
    }

    if (mLastFrameTime < 0) {
        if (mSeekFraction >= 0) {
            int64_t seekTime = int64_t(getScaledDuration() * mSeekFraction);
            mStartTime = frameTime - seekTime;
            mSeekFraction = -1;
        }
        mStartTimeCommitted = false; // allow start time to be compensated for jank
    }
    mLastFrameTime = frameTime;
    // The frame time might be before the start time during the first frame of
    // an animation.  The "current time" must always be on or after the start
    // time to avoid animating frames at negative time intervals.  In practice, this
    // is very rare and only happens when seeking backwards.
    const int64_t currentTime = std::max(frameTime, mStartTime);
    const bool finished = animateBasedOnTime(currentTime);
    if (finished) endAnimation();
    return finished;
}

bool ValueAnimator::pulseAnimationFrame(int64_t frameTime){
    if (mSelfPulse) {
        // Pulse animation frame will *always* be after calling start(). If mSelfPulse isn't
        // set to false at this point, that means child animators did not call super's start().
        // This can happen when the Animator is just a non-animating wrapper around a real
        // functional animation. In this case, we can't really pulse a frame into the animation,
        // because the animation cannot necessarily be properly initialized (i.e. no start/end
        // values set).
        return false;
    }
    return doAnimationFrame(frameTime);
}

void ValueAnimator::addOneShotCommitCallback() {
    if (!mSelfPulse) {
        return;
    }
    getAnimationHandler().addOneShotCommitCallback(this);
}

void ValueAnimator::removeAnimationCallback() {
    if (!mSelfPulse) {
        return;
    }
    getAnimationHandler().removeCallback(this);
}

void ValueAnimator::addAnimationCallback(int64_t delay) {
    if (!mSelfPulse) {
        return;
    }
    getAnimationHandler().addAnimationFrameCallback(this,delay);
}

float ValueAnimator::getAnimatedFraction() {
    return mCurrentFraction;
}

void ValueAnimator::animateValue(float fraction) {
    fraction = mInterpolator->getInterpolation(fraction);
    mCurrentFraction = fraction;
    for (auto v:mValues) {
        v->calculateValue(fraction);
    }
    for (auto l:mUpdateListeners) {
        l(*this);
    }
}

ValueAnimator*ValueAnimator::clone()const {
    ValueAnimator*anim= new ValueAnimator(*this);

    if (!mUpdateListeners.empty()) {
        anim->mUpdateListeners = mUpdateListeners;
    }
    anim->mSeekFraction = -1;
    anim->mReversing = false;
    anim->mInitialized = false;
    anim->mStarted = false;
    anim->mRunning = false;
    anim->mPaused = false;
    anim->mResumed = false;
    anim->mStartListenersCalled = false;
    anim->mStartTime = -1;
    anim->mStartTimeCommitted = false;
    anim->mAnimationEndRequested = false;
    anim->mPauseTime = -1;
    anim->mLastFrameTime = -1;
    anim->mFirstFrameTime = -1;
    anim->mOverallFraction = 0;
    anim->mCurrentFraction = 0;
    anim->mSelfPulse = true;
    anim->mSuppressSelfPulseRequested = false;
    return anim;
}

AnimationHandler& ValueAnimator::getAnimationHandler()const{
    return AnimationHandler::getInstance();
}

std::string ValueAnimator::toString()const{
    return "ValueAnimator@"+std::to_string((long)this);
}

}//endof namespace 
