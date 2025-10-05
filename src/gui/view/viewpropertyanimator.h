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
#ifndef __VIEW_PROPERTY_ANIMATOR_H__
#define __VIEW_PROPERTY_ANIMATOR_H__
#include <set>
#include <animation/valueanimator.h>

namespace cdroid{

class ViewPropertyAnimator{
protected:
    class AnimatorEventListener:public Animator::AnimatorListener{
    };
    class NameValuesHolder {
    public:
        int mNameConstant;
        float mFromValue;
        float mDeltaValue;
        NameValuesHolder(int nameConstant, float fromValue, float deltaValue) {
            mNameConstant = nameConstant;
            mFromValue = fromValue;
            mDeltaValue = deltaValue;
        }
    };
    class PropertyBundle{
    public:
        int mPropertyMask;
        std::vector<NameValuesHolder> mNameValuesHolder;
        PropertyBundle(int propertyMask,const std::vector<NameValuesHolder>& nameValuesHolder);
        bool cancel(int propertyConstant);
    };
    static constexpr int NONE           = 0x0000;
    static constexpr int TRANSLATION_X  = 0x0001;
    static constexpr int TRANSLATION_Y  = 0x0002;
    static constexpr int TRANSLATION_Z  = 0x0004;
    static constexpr int SCALE_X        = 0x0008;
    static constexpr int SCALE_Y        = 0x0010;
    static constexpr int ROTATION       = 0x0020;
    static constexpr int ROTATION_X     = 0x0040;
    static constexpr int ROTATION_Y     = 0x0080;
    static constexpr int X              = 0x0100;
    static constexpr int Y              = 0x0200;
    static constexpr int Z              = 0x0400;
    static constexpr int ALPHA          = 0x0800;
    static constexpr int TRANSFORM_MASK = TRANSLATION_X | TRANSLATION_Y | TRANSLATION_Z |
            SCALE_X | SCALE_Y | ROTATION | ROTATION_X | ROTATION_Y | X | Y | Z;
private:
    View* mView;
    bool mDurationSet;
    bool mStartDelaySet;
    bool mInterpolatorSet;
    long mDuration;
    long mStartDelay;
    const TimeInterpolator* mInterpolator;
    Animator::AnimatorListener mListener;
    ValueAnimator::AnimatorUpdateListener mUpdateListener;
    ValueAnimator* mTempValueAnimator;

    //AnimatorEventListener mAnimatorEventListener;
    Animator::AnimatorListener mAnimatorEventListener;
    ValueAnimator::AnimatorUpdateListener mAnimatorEventListener2;

    std::vector<NameValuesHolder> mPendingAnimations;
    Runnable mAnimationStarter;
    Runnable mPendingSetupAction;
    Runnable mPendingCleanupAction;
    Runnable mPendingOnStartAction;
    Runnable mPendingOnEndAction;

    std::unordered_map<Animator*, PropertyBundle> mAnimatorMap;
    std::unordered_map<Animator*, Runnable> mAnimatorSetupMap;
    std::unordered_map<Animator*, Runnable> mAnimatorCleanupMap;
    std::unordered_map<Animator*, Runnable> mAnimatorOnStartMap;
    std::unordered_map<Animator*, Runnable> mAnimatorOnEndMap;
private:
    static std::set<Animator*>map2set(const std::unordered_map<Animator*,PropertyBundle>&amap);
    void animateProperty(int constantName, float toValue);
    void animatePropertyBy(int constantName, float byValue);
    void animatePropertyBy(int constantName, float startValue, float byValue);
    void setValue(int propertyConstant, float value);
    float getValue(int propertyConstant)const;
    void startAnimation();
public:
    ViewPropertyAnimator(View* view);
    ~ViewPropertyAnimator();
    ViewPropertyAnimator& setDuration(long duration);
    long getDuration();
    ViewPropertyAnimator& setStartDelay(long startDelay);
    long getStartDelay()const;
    ViewPropertyAnimator& setInterpolator(const TimeInterpolator* interpolator);
    const TimeInterpolator*getInterpolator();
    ViewPropertyAnimator& setListener(const Animator::AnimatorListener& listener);
    Animator::AnimatorListener getListener()const;
    ViewPropertyAnimator& setUpdateListener(const ValueAnimator::AnimatorUpdateListener& listener);
    ValueAnimator::AnimatorUpdateListener getUpdateListener()const;
    void start();
    void cancel();

    ViewPropertyAnimator& x(float value);
    ViewPropertyAnimator& xBy(float value);
    ViewPropertyAnimator& y(float value);
    ViewPropertyAnimator& yBy(float value);
    ViewPropertyAnimator& z(float value);
    ViewPropertyAnimator& zBy(float value);

    ViewPropertyAnimator& rotation(float value);
    ViewPropertyAnimator& rotationBy(float value);
    ViewPropertyAnimator& rotationX(float value);
    ViewPropertyAnimator& rotationXBy(float value);
    ViewPropertyAnimator& rotationY(float value);
    ViewPropertyAnimator& rotationYBy(float value);

    ViewPropertyAnimator& translationX(float value);
    ViewPropertyAnimator& translationXBy(float value);
    ViewPropertyAnimator& translationY(float value);
    ViewPropertyAnimator& translationYBy(float value);
    ViewPropertyAnimator& translationZ(float value);
    ViewPropertyAnimator& translationZBy(float value);

    ViewPropertyAnimator& scaleX(float value);
    ViewPropertyAnimator& scaleXBy(float value);
    ViewPropertyAnimator& scaleY(float value);
    ViewPropertyAnimator& scaleYBy(float value);

    ViewPropertyAnimator& alpha(float value);
    ViewPropertyAnimator& alphaBy(float value);

    ViewPropertyAnimator& withStartAction(Runnable runnable);
    ViewPropertyAnimator& withEndAction(Runnable runnable);
    bool hasActions()const;
};
}//endof namespace

#endif
