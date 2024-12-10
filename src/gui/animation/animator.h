#ifndef __ANIMATOR_H__
#define __ANIMATOR_H__
#include <vector>
#include <memory>
#include <functional>
#include <animation/interpolators.h>
#include <core/callbackbase.h>
namespace cdroid{

template <class T>
class ConstantState{
public:
    virtual int getChangingConfigurations()=0;
    virtual T newInstance()=0;
};

class Animator{
private:
    class AnimatorConstantState;
public: 
    static constexpr long DURATION_INFINITE = -1;
    
    class AnimatorListener:public EventSet{
    public:
        CallbackBase<void,Animator& /*animation*/, bool/*isReverse*/>onAnimationStart;
        CallbackBase<void,Animator& /*animation*/, bool/*isReverse*/>onAnimationEnd;
        CallbackBase<void,Animator& /*animation*/>onAnimationCancel;
        CallbackBase<void,Animator& /*animation*/>onAnimationRepeat;
    };
    class AnimatorPauseListener:public EventSet{
    public:
         CallbackBase<void,Animator&> onAnimationPause;
         CallbackBase<void,Animator&> onAnimationResume;
    };
private:
    AnimatorConstantState* mConstantState;
protected:
    friend class AnimatorSet;
    bool mPaused = false;
    int mChangingConfigurations = 0;
    std::vector<AnimatorListener> mListeners;
    std::vector<AnimatorPauseListener> mPauseListeners;
    virtual void skipToEndValue(bool inReverse);
    virtual bool isInitialized();
public:
    virtual bool pulseAnimationFrame(int64_t frameTime);
    virtual void startWithoutPulsing(bool inReverse);
    virtual void animateBasedOnPlayTime(int64_t currentPlayTime, int64_t lastPlayTime, bool inReverse);
public:
    virtual ~Animator();
    virtual void start();
    virtual void cancel();
    virtual void end();
    virtual void pause();
    virtual void resume();
    bool isPaused();
    virtual Animator*clone()const;
    virtual long getStartDelay()=0;
    virtual void setStartDelay(long startDelay)=0;
    virtual Animator& setDuration(long duration)=0;
    virtual long getDuration()=0;
    virtual long getTotalDuration();
    virtual void setInterpolator(TimeInterpolator* value)=0;
    virtual TimeInterpolator* getInterpolator();
    virtual bool isRunning()=0;
    virtual bool isStarted();
    void addListener(const AnimatorListener& listener);
    void removeListener(const AnimatorListener& listener);
    std::vector<AnimatorListener> getListeners();
    void addPauseListener(const AnimatorPauseListener& listener);
    void removePauseListener(const AnimatorPauseListener& listener);
    void removeAllListeners();
    virtual int getChangingConfigurations();
    void setChangingConfigurations(int configs);
    void appendChangingConfigurations(int configs);
    std::shared_ptr<ConstantState<Animator*>> createConstantState();
    virtual void setupStartValues();
    virtual void setupEndValues();
    virtual void setTarget(void*target);
    virtual bool canReverse();
    virtual void reverse();
    virtual void setAllowRunningAsynchronously(bool);
};

class AnimatorListenerAdapter:public Animator::AnimatorListener,public Animator::AnimatorPauseListener{
public:
    AnimatorListenerAdapter();
};

class Animator::AnimatorConstantState:public ConstantState<Animator*> {
    Animator* mAnimator;
    int mChangingConf;
public:
    AnimatorConstantState(Animator* animator);
    int getChangingConfigurations()override;
    Animator* newInstance()override;
};
}//endof namespace
#endif //__ANIMATOR_H__
