#pragma once
#include <animation/animation.h>

namespace cdroid{
class View;
class LayoutAnimationController{
public:
    static constexpr int ORDER_NORMAL  = 0;
    static constexpr int ORDER_REVERSE = 1;
    static constexpr int ORDER_RANDOM  = 2;
    class AnimationParameters {
    /* The number of children in the view group containing the view to which
     * these parameters are attached.*/
    public: int count;

    /** The index of the view to which these parameters are attached in its
     * containing view group.*/
    public: int index;
    };
private:
    float mDelay;
    int mOrder;
    long mDuration;
    long mMaxDelay; 
protected:
    Animation* mAnimation;
    Interpolator* mInterpolator;

    virtual long getDelayForView(View* view);
    int getTransformedIndex(const AnimationParameters* params);
public:
    LayoutAnimationController(Context* context, const AttributeSet& attrs);
    LayoutAnimationController(Animation* animation,float delay);
    LayoutAnimationController(Animation* animation);
    int getOrder()const;
    void setOrder(int);
    void setAnimation(Context* context,const std::string&resourceID);
    void setAnimation(Animation* animation);
    Animation* getAnimation();
    void setInterpolator(Context* context,const std::string&resourceID);
    void setInterpolator(Interpolator* interpolator);
    Interpolator* getInterpolator();
    float getDelay()const;
    void setDelay(float);
    virtual bool willOverlap();
    void start();
    Animation* getAnimationForView(View* view);
    bool isDone()const;
};

}//endof namespace
