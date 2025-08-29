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
#ifndef __VALUEANIMATOR_H__
#define __VALUEANIMATOR_H__
#include <animation/animatorset.h>
#include <animation/propertyvaluesholder.h>
#include <animation/animationhandler.h>
#include <unordered_map>

namespace cdroid{

class ValueAnimator:public Animator,public AnimationHandler::AnimationFrameCallback{
public:
    static constexpr int RESTART = 1;
    static constexpr int REVERSE = 2;
    static constexpr int INFINITE= -1;
    typedef CallbackBase<void,ValueAnimator&>AnimatorUpdateListener;
private:
    friend class AnimatorSet;
    static float sDurationScale;
    bool mResumed = false;
    bool mReversing;
    bool mRunning = false;
    bool mStarted = false;
    bool mStartListenersCalled = false;
    bool mAnimationEndRequested = false;
    bool mSelfPulse = true;
    bool mSuppressSelfPulseRequested = false;

    float mOverallFraction = 0.f;
    float mCurrentFraction = 0.f;
    float mDurationScale = -1.f;

    int64_t mPauseTime;
    int64_t mLastFrameTime = -1;
    int64_t mFirstFrameTime= -1;

    long mDuration = 300;
    long mStartDelay = 0;
    int mRepeatCount = 0;
    int mRepeatMode = RESTART;
    TimeInterpolator* mInterpolator = nullptr;
    static TimeInterpolator* sDefaultInterpolator;
    std::vector<AnimatorUpdateListener> mUpdateListeners;
protected:
    bool mStartTimeCommitted;
    bool mInitialized = false;
    int64_t mStartTime = -1;
    float mSeekFraction = -1;
    std::vector<PropertyValuesHolder*>mValues;
    std::unordered_map<std::string,PropertyValuesHolder*>mValuesMap;
private:
    float resolveDurationScale()const;
    int64_t getScaledDuration()const;
    int getCurrentIteration(float fraction);
    float getCurrentIterationFraction(float fraction, bool inReverse);
    float clampFraction(float fraction);
    bool shouldPlayBackward(int iteration, bool inReverse);
    bool isPulsingInternal();
    void notifyStartListeners();
    void start(bool playBackwards);
    void startAnimation();
    void endAnimation();
    void addOneShotCommitCallback();
    void removeAnimationCallback();
    void addAnimationCallback(long delay);
protected:
    virtual void animateValue(float fraction);
    void startWithoutPulsing(bool inReverse)override;
    virtual bool animateBasedOnTime(int64_t currentTime);
    void animateBasedOnPlayTime(int64_t currentPlayTime, int64_t lastPlayTime, bool inReverse)override;
    void skipToEndValue(bool inReverse)override;
    bool isInitialized()override;
    bool pulseAnimationFrame(int64_t frameTime)override;
public:
    ValueAnimator();
    ValueAnimator(const ValueAnimator&);
    ~ValueAnimator()override;
    static void setDurationScale(float durationScale);
    static float getDurationScale();
    static bool areAnimatorsEnabled();
    static ValueAnimator* ofInt(const std::vector<int>&);
    static ValueAnimator* ofArgb(const std::vector<int>&);
    static ValueAnimator* ofFloat(const std::vector<float>&);
    static ValueAnimator* ofPropertyValuesHolder(const std::vector<PropertyValuesHolder*>&);

    virtual void setValues(const std::vector<PropertyValuesHolder*>&);
    virtual void setIntValues(const std::vector<int>&);
    virtual void setFloatValues(const std::vector<float>&);
    std::vector<PropertyValuesHolder*>&getValues();
    const std::vector<PropertyValuesHolder*>&getValues()const;
    PropertyValuesHolder*getValues(int idx);
    PropertyValuesHolder*getValues(const std::string&propname);
    virtual void initAnimation();
    ValueAnimator& setDuration(long duration)override;
    long getDuration()override;
    long getTotalDuration()override;
    void overrideDurationScale(float durationScale);
    void setCurrentFraction(float fraction);
    virtual void setCurrentPlayTime(int64_t playTime);
    int64_t getCurrentPlayTime();
    long getStartDelay()override;
    void setStartDelay(long startDelay)override;
    static long getFrameDelay();
    static void setFrameDelay(long frameDelay);
    AnimateValue getAnimatedValue();
    AnimateValue getAnimatedValue(const std::string&propertyName);
    void setRepeatCount(int value);
    int  getRepeatCount()const;
    void setRepeatMode(int value);
    int  getRepeatMode()const;
    void addUpdateListener(const AnimatorUpdateListener& listener);
    void removeUpdateListener(const AnimatorUpdateListener& listener);
    void removeAllUpdateListeners();
    void setInterpolator(TimeInterpolator* value)override;
    TimeInterpolator* getInterpolator()override;
    void setEvaluator(TypeEvaluator value);
    void start()override;
    void cancel()override;
    void end()override;
    void resume()override;
    void pause()override;
    bool isRunning()override;
    bool isStarted()override;
    void reverse()override;
    bool canReverse()override;

    void commitAnimationFrame(int64_t frameTime)override;
    bool doAnimationFrame(int64_t frameTime)override;
    float getAnimatedFraction();
    ValueAnimator* clone()const override;
    AnimationHandler& getAnimationHandler()const;
    std::string toString()const override;
};

}//endof namespace
#endif//__VALUEANIMATOR_H__
