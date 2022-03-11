#ifndef __STATELIST_ANIMATOR_H__
#define __STATELIST_ANIMATOR_H__
#include <animation/valueanimator.h>
namespace cdroid{

class StateListAnimator{
protected:
    class Tuple {
    public:
        std::vector<int>mSpecs;
        ValueAnimator* mAnimator;
        Tuple(const std::vector<int>&specs, ValueAnimator* animator) {
            mSpecs = specs;
            mAnimator = animator;
        }
    };
private:
    class StateListAnimatorConstantState: public std::enable_shared_from_this<StateListAnimatorConstantState>,public ConstantState<StateListAnimator*>{
    protected:
        int mChangingConf;
        StateListAnimator*mAnimator;
    public:
        StateListAnimatorConstantState(StateListAnimator*animator);
        int getChangingConfigurations()const;
        StateListAnimator*newInstance()override;  
    };
private:
    std::vector<Tuple>mTuples;
    Tuple* mLastMatch;
    Animator*mRunningAnimator;
    class View*mView;
    std::shared_ptr<StateListAnimatorConstantState> mConstantState;
    ValueAnimator::AnimatorListener mAnimationListener;
    int mChangingConfigurations;

    void clearTarget();
    void start(Tuple* match);
    void cancel();
public:
    StateListAnimator();
    void addState(const std::vector<int>&specs, ValueAnimator* animator);
    void setState(const std::vector<int>&state);
    Animator* getRunningAnimator();
    View* getTarget();
    void setTarget(View* view);
    void jumpToCurrentState();
    int getChangingConfigurations()const;
    void setChangingConfigurations(int configs);
    void appendChangingConfigurations(int configs);
    std::shared_ptr<ConstantState<StateListAnimator*>>createConstantState();
};
}//endof namespace
#endif//__STATELIST_ANIMATOR_H__
