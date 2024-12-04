#ifndef __VALUEANIMATOR_H__
#define __VALUEANIMATOR_H__
#include <animation/animatorset.h>
#include <animation/propertyvaluesholder.h>
#include <animation/animationhandler.h>
#include <map>

namespace cdroid{

typedef std::function<float (float fraction, float startValue, float endValue)>TypeEvaluator; 

class ValueAnimator:public Animator,public AnimationHandler::AnimationFrameCallback{
public:
    static constexpr int RESTART = 1;
    static constexpr int REVERSE = 2;
    static constexpr int INFINITE= -1;
    typedef CallbackBase<void,ValueAnimator&>AnimatorUpdateListener;
private:
    friend class AnimatorSet;
    static float sDurationScale;
    //static TimeInterpolator sDefaultInterpolator = new AccelerateDecelerateInterpolator();
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
    int64_t mStartTime = -1;
    bool mStartTimeCommitted;
    float mSeekFraction = -1;
    bool mInitialized = false;
    std::vector<PropertyValuesHolder*>mValues;
    std::map<const std::string,PropertyValuesHolder*>mValuesMap;
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
    void startWithoutPulsing(bool inReverse);
    virtual bool animateBasedOnTime(int64_t currentTime);
    virtual void animateBasedOnPlayTime(int64_t currentPlayTime, int64_t lastPlayTime, bool inReverse);
    void skipToEndValue(bool inReverse);
    bool isInitialized();
    bool pulseAnimationFrame(int64_t frameTime);
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
    ValueAnimator& setDuration(long duration);
    long getDuration();
    long getTotalDuration();
    virtual void setCurrentPlayTime(int64_t playTime);
    void setCurrentFraction(float fraction);
    int64_t getCurrentPlayTime();
    long getStartDelay();
    void setStartDelay(long startDelay);
    static long getFrameDelay();
    static void setFrameDelay(long frameDelay);
    AnimateValue getAnimatedValue();
    AnimateValue getAnimatedValue(const std::string&propertyName);
    void setRepeatCount(int value);
    int  getRepeatCount()const;
    void setRepeatMode(int value);
    int  getRepeatMode()const;
    void addUpdateListener(AnimatorUpdateListener listener);
    void removeUpdateListener(AnimatorUpdateListener listener);
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
};

}//endof namespace
#endif//__VALUEANIMATOR_H__
