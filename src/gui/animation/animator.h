#pragma once
#include <vector>
#include <memory>
#include <functional>
#include <animation/interpolators.h>

namespace cdroid{

template <class T>
class ConstantState{
public:
    virtual int getChangingConfigurations()=0;
    virtual T newInstance()=0;
};

typedef Interpolator* TimeInterpolator;

class Animator{
private:
    class AnimatorConstantState:public ConstantState<Animator*> {
        Animator* mAnimator;
        int mChangingConf;
    public:
        AnimatorConstantState(Animator* animator) {
            mAnimator = animator;
            // ensure a reference back to here so that constante state is not gc'ed.
            mAnimator->mConstantState = this;
            mChangingConf = mAnimator->getChangingConfigurations();
        }

        int getChangingConfigurations()override {
            return mChangingConf;
        }

        Animator* newInstance()override{
            Animator* clone = mAnimator->clone();
            clone->mConstantState = this;
            return clone;
        }
    };
public: 
    static constexpr long DURATION_INFINITE = -1;
    
    class AnimatorListener{
    public:
        std::function<void (Animator& animation, bool isReverse)>onAnimationStart;
        std::function<void (Animator& animation, bool isReverse)>onAnimationEnd;
        std::function<void (Animator& animation)>onAnimationCancel;
        std::function<void (Animator& animation)>onAnimationRepeat;
    };
    class AnimatorPauseListener{
    public:
         std::function<void(Animator&)> onAnimationPause;
         std::function<void(Animator&)> onAnimationResume;
    };
private:
    AnimatorConstantState* mConstantState;
protected:
    bool mPaused = false;
    int mChangingConfigurations = 0;
    std::vector<AnimatorListener> mListeners;
    std::vector<AnimatorPauseListener> mPauseListeners;
protected:
    bool pulseAnimationFrame(long frameTime);
    void startWithoutPulsing(bool inReverse);
    void skipToEndValue(bool inReverse);
    bool isInitialized();
    virtual void animateBasedOnPlayTime(long currentPlayTime, long lastPlayTime, bool inReverse);
public:
    virtual void start();
    virtual void cancel();
    virtual void end();
    virtual void pause();
    virtual void resume();
    bool isPaused();
    virtual Animator*clone();
    virtual long getStartDelay()=0;
    virtual void setStartDelay(long startDelay)=0;
    virtual Animator& setDuration(long duration)=0;
    virtual long getDuration()=0;
    long getTotalDuration();
    virtual void setInterpolator(TimeInterpolator value)=0;
    TimeInterpolator getInterpolator();
    virtual bool isRunning()=0;
    virtual bool isStarted();
    void addListener(AnimatorListener listener);
    void removeListener(AnimatorListener listener);
    std::vector<AnimatorListener> getListeners();
    void addPauseListener(AnimatorPauseListener listener);
    void removePauseListener(AnimatorPauseListener listener);
    void removeAllListeners();
    int getChangingConfigurations();
    void setChangingConfigurations(int configs);
    void appendChangingConfigurations(int configs);
    std::shared_ptr<ConstantState<Animator*>> createConstantState();
    void setupStartValues();
    void setupEndValues();
    void setTarget(void*target);
    virtual bool canReverse();
    virtual void reverse(); 
};

class AnimatorListenerAdapter:public Animator::AnimatorListener,Animator::AnimatorPauseListener{
public:
    AnimatorListenerAdapter();
};
}
