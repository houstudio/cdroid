#ifndef __ANIMATED_STATELIST_DRAWABLE_H__
#define __ANIMATED_STATELIST_DRAWABLE_H__
#include <drawables/statelistdrawable.h>
#include <drawables/animationdrawable.h>
#include <animation/interpolators.h>
#include <core/sparsearray.h>
namespace cdroid{

class AnimatedStateListDrawable:public StateListDrawable{
protected:
    class AnimatedStateListState:public StateListState{
    private:
        static constexpr long REVERSED_BIT = 0x100000000l;
        static constexpr long REVERSIBLE_FLAG_BIT = 0x200000000l;
    protected:
        LongSparseArray mTransitions;
        SparseIntArray mStateIds;
    public:
        AnimatedStateListState(const AnimatedStateListState* orig,AnimatedStateListDrawable* owner);
        void mutate()override;
        int addTransition(int fromId, int toId,Drawable* anim, bool reversible);
        int addStateSet(std::vector<int> stateSet,Drawable*drawable, int id);
        int indexOfKeyframe(const std::vector<int>&stateSet);
        int getKeyframeIdAt(int index);
        int indexOfTransition(int fromId, int toId);
        bool isTransitionReversed(int fromId, int toId);
        bool transitionHasReversibleFlag(int fromId, int toId);
        Drawable* newDrawable()override;
        static long generateTransitionKey(int fromId, int toId);
    };
    class FrameInterpolator:public TimeInterpolator {
    private:
        std::vector<int> mFrameTimes;
        int mFrames;
        int mTotalDuration;
    public:
        FrameInterpolator(AnimationDrawable* d, bool reversed);
        int  updateFrames(AnimationDrawable* d, bool reversed);
        int  getTotalDuration();
        float getInterpolation(float input)override;
    };

    class Transition {
    public:
        virtual void start()=0;
        virtual void stop()=0;

        virtual void reverse()=0;
        // Not supported by default.
        virtual bool canReverse() {
            return false;
        }
    };
private:
    std::shared_ptr<AnimatedStateListState> mState;

    /** The currently running transition, if any. */
    Transition* mTransition;

    /** Index to be set after the transition ends. */
    int mTransitionToIndex = -1;

    /** Index away from which we are transitioning. */
    int mTransitionFromIndex = -1;

    bool mMutated;
private:
    bool selectTransition(int toIndex);
    AnimatedStateListDrawable(std::shared_ptr<AnimatedStateListState> state);
protected:
    bool onStateChange(const std::vector<int>&stateSet)override;
    void setConstantState(std::shared_ptr<DrawableContainerState> state)override;
    std::shared_ptr<DrawableContainerState>cloneConstantState()override;
public:
    AnimatedStateListDrawable();
    bool setVisible(bool visible, bool restart)override;
    void addState(const std::vector<int>&stateSet,Drawable* drawable, int id);
    void addTransition(int fromId, int toId,Transition* transition, bool reversible);
    bool isStateful()const override;
    void jumpToCurrentState()override;
    Drawable* mutate()override;
    void clearMutated()override;
};

}//endof namespace
#endif//__ANIMATED_STATELIST_DRAWABLE_H__
