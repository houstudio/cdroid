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
#ifndef __PROPERTY_VALUES_ANIMATORSET_H__
#define __PROPERTY_VALUES_ANIMATORSET_H__
#include <animation/interpolators.h>
#include <animation/propertyvaluesholder.h>
#include <animation/valueanimator.h>
namespace cdroid {
namespace hwui {
class Tree;
using  VectorDrawableRoot= Tree;
class PropertyAnimator {
public:
    PropertyAnimator(PropertyValuesHolder* holder, TimeInterpolator* interpolator, int64_t startDelay,
                     int64_t duration, int repeatCount, int repeatMode);
    void setCurrentPlayTime(int64_t playTime);
    int64_t getTotalDuration() { return mTotalDuration; }
    // fraction range: [0, 1], iteration range [0, repeatCount]
    void setFraction(float fraction, int64_t iteration);

private:
    std::unique_ptr<PropertyValuesHolder> mPropertyValuesHolder;
    std::unique_ptr<TimeInterpolator> mInterpolator;
    int64_t mStartDelay;
    int64_t mDuration;
    uint32_t mRepeatCount;
    int64_t mTotalDuration;
    int mRepeatMode;
    double mLatestFraction = 0;
};

// TODO: This class should really be named VectorDrawableAnimator
class PropertyValuesAnimatorSet/* : public BaseRenderNodeAnimator */{

public:
    /*class PropertyAnimatorSetListener : public Animator::AnimationListener {
    public:
        explicit PropertyAnimatorSetListener(PropertyValuesAnimatorSet* set) : mSet(set) {}
        virtual void onAnimationFinished(BaseRenderNodeAnimator* animator) override;
    private:
        PropertyValuesAnimatorSet* mSet;
    };*/

private:
    int64_t mStartTime;
    int64_t mDuration;
    int64_t mStartDelay;

    float mLastFraction = 0.0f;
    bool mInitialized = false;
    Tree/*VectorDrawableRoot*/* mVectorDrawable;
    bool mIsInfinite = false;
    // This request id gets incremented (on UI thread only) when a new request to modfiy the
    // lifecycle of an animation happens, namely when start/end/reset/reverse is called.
    uint32_t mRequestId = 0;
    std::vector<std::unique_ptr<PropertyAnimator> > mAnimators;
private:
    friend class PropertyAnimatorSetListener;
    void init();
public:
    PropertyValuesAnimatorSet();
    void setInterpolator(Interpolator* interpolator);
    void setStartValue(float value);
    void setDuration(int64_t duration);
    void setStartDelay(int64_t startDelay);
    //void start(const Animator::AnimationListener& listener);
    //void reverse(const Animator::AnimationListener& listener);
    virtual void reset();// override;
    virtual void end();// override;

    void addPropertyAnimator(PropertyValuesHolder* propertyValuesHolder,TimeInterpolator* interpolators,
            int64_t startDelays, int64_t durations,int repeatCount, int repeatMode);
    //virtual uint32_t dirtyMask();
    bool isInfinite() { return mIsInfinite; }
    void setVectorDrawable(VectorDrawableRoot* vd) { mVectorDrawable = vd; }
    VectorDrawableRoot* getVectorDrawable() const { return mVectorDrawable; }
    //AnimationListener* getOneShotListener() { return mOneShotListener; }
    void clearOneShotListener() { /*mOneShotListener = nullptr;*/ }
    uint32_t getRequestId() const { return mRequestId; }

protected:
    //virtual float getValue(RenderNode* target) const override;
    //virtual void setValue(RenderNode* target, float value) override;
    virtual void onPlayTimeChanged(int64_t playTime);// override;

private:
    //void onFinished(BaseRenderNodeAnimator* animator);
    // Listener set from outside
    //Animator::AnimationListener mOneShotListener;
};

}  // namespace hwui
}  // namespace cdroid
#endif/*__PROPERTY_VALUES_ANIMATORSET_H__*/
