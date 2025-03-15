#ifndef __ANIMATED_STATELIST_DRAWABLE_H__
#define __ANIMATED_STATELIST_DRAWABLE_H__
#include <drawables/statelistdrawable.h>
#include <drawables/animationdrawable.h>
#include <animation/interpolators.h>
#include <animation/objectanimator.h>
#include <core/sparsearray.h>
namespace cdroid{

class AnimatedStateListDrawable:public StateListDrawable{
private:
    static constexpr const char* ELEMENT_TRANSITION = "transition";
    static constexpr const char* ELEMENT_ITEM = "item";
protected:
    class AnimatedStateListState;
    class FrameInterpolator;
    class Transition;
    class AnimatableTransition;
    class AnimationDrawableTransition;
    class AnimatedVectorDrawableTransition;
    typedef Drawable AnimatedVectorDrawable;
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
    void init();
    void updateStateFromTypedArray(const AttributeSet&atts);
    void inflateChildElement(XmlPullParser&,const AttributeSet&);
    int parseItem(XmlPullParser&,const AttributeSet&);
    int parseTransition(XmlPullParser&,const AttributeSet&);
protected:
    AnimatedStateListDrawable(Context*,const AttributeSet&);
    bool onStateChange(const std::vector<int>&stateSet)override;
    void setConstantState(std::shared_ptr<DrawableContainerState> state)override;
    std::shared_ptr<DrawableContainerState>cloneConstantState()override;
public:
    AnimatedStateListDrawable();
    bool setVisible(bool visible, bool restart)override;
    void addState(const std::vector<int>&stateSet,Drawable* drawable, int id);
    /* transition must derived from Drawable & Animatable*/
    void addTransition(int fromId, int toId,Drawable* transition, bool reversible);
    bool isStateful()const override;
    void jumpToCurrentState()override;
    AnimatedStateListDrawable* mutate()override;
    void clearMutated()override;
    void inflate(XmlPullParser&parser,const AttributeSet&atts)override;
};

class AnimatedStateListDrawable::AnimatedStateListState:public StateListState{
private:
    static constexpr int64_t REVERSED_BIT = 0x100000000l;
    static constexpr int64_t REVERSIBLE_FLAG_BIT = 0x200000000l;
protected:
    LongSparseLongArray mTransitions;
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
    AnimatedStateListDrawable* newDrawable()override;
    static int64_t generateTransitionKey(int fromId, int toId);
};

class AnimatedStateListDrawable::FrameInterpolator:public TimeInterpolator {
private:
    std::vector<int> mFrameTimes;
    int mFrames;
    int mTotalDuration;
public:
    FrameInterpolator(AnimationDrawable* d, bool reversed);
    int updateFrames(AnimationDrawable* d, bool reversed);
    int getTotalDuration();
    float getInterpolation(float input)override;
};

class AnimatedStateListDrawable::Transition {
protected:
    Drawable*mDrawable;
public:
    virtual void start()= 0;
    virtual void stop() = 0;
    virtual void reverse();
    // Not supported by default.
    virtual bool canReverse();
};

class AnimatedStateListDrawable::AnimatableTransition:public Transition {
private:
    Animatable* mA;
public:
    AnimatableTransition(Animatable* a);
    void start()override;
    void stop()override;
};

class AnimatedStateListDrawable::AnimationDrawableTransition:public Transition {
private:
    Property*mProperty;
    ObjectAnimator* mAnim;
    FrameInterpolator*mFrameInterpolator;	
    // Even AnimationDrawable is always reversible technically, but
    // we should obey the XML's android:reversible flag.
    bool mHasReversibleFlag;
public:
    AnimationDrawableTransition(AnimationDrawable* ad,bool reversed, bool hasReversibleFlag);
	~AnimationDrawableTransition();
    bool canReverse()override;
    void start()override;
    void reverse()override;
    void stop()override;
};
class AnimatedStateListDrawable::AnimatedVectorDrawableTransition:public Transition {
private:
    AnimatedVectorDrawable* mAvd;
    // mReversed is indicating the current transition's direction.
    bool mReversed;

    // mHasReversibleFlag is indicating whether the whole transition has
    // reversible flag set to true.
    // If mHasReversibleFlag is false, then mReversed is always false.
    bool mHasReversibleFlag;
public:
    AnimatedVectorDrawableTransition(AnimatedVectorDrawable* avd,bool reversed, bool hasReversibleFlag);
    bool canReverse()override;
    void start()override;
    void reverse()override;
    void stop()override;
};
}//endof namespace
#endif//__ANIMATED_STATELIST_DRAWABLE_H__
