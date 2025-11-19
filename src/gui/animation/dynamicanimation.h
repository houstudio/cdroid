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
*/
#ifndef __DYNAMIC_ANIMATION_H__
#define __DYNAMIC_ANIMATION_H__
#include <cfloat>
#include <view/view.h>
#include <core/callbackbase.h>
#include <animation/floatvalueholder.h>
#include <animation/animationhandler.h>
#include <animation/floatperpertycompat.h>
namespace cdroid{
class DynamicAnimation :public AnimationHandler::AnimationFrameCallback {
private:
    // Use the max value of float to indicate an unset state.
    static constexpr float UNSET = FLT_MAX;
    // Multiplier to the min visible change value for value threshold
    static constexpr float THRESHOLD_MULTIPLIER = 0.75f;
public:
    static constexpr float MIN_VISIBLE_CHANGE_PIXELS = 1.f;
    static constexpr float MIN_VISIBLE_CHANGE_ROTATION_DEGREES = 1.f / 10.f;
    static constexpr float MIN_VISIBLE_CHANGE_ALPHA = 1.f / 256.f;
    static constexpr float MIN_VISIBLE_CHANGE_SCALE = 1.f / 500.f;

    using OnAnimationEndListener = CallbackBase<void,DynamicAnimation&,bool/*canceled*/,float/*value*/,float/*velocity*/>;
    using OnAnimationUpdateListener = CallbackBase<void,DynamicAnimation&,float/*value*/,float/*velocity*/>;

    // Internal state for value/velocity pair.
    class MassState {
    public:
        float mValue;
        float mVelocity;
    };
public:
    static const FloatProperty& TRANSLATION_X;
    static const FloatProperty& TRANSLATION_Y;
    static const FloatProperty& TRANSLATION_Z;
    static const FloatProperty& SCALE_X;
    static const FloatProperty& SCALE_Y;
    static const FloatProperty& ROTATION;
    static const FloatProperty& ROTATION_X;
    static const FloatProperty& ROTATION_Y;
    static const FloatProperty& X;
    static const FloatProperty& Y;
    static const FloatProperty& Z;
    static const FloatProperty& ALPHA;
    static const FloatProperty& SCROLL_X;
    static const FloatProperty& SCROLL_Y;
protected:
    // Internal tracking for velocity.
    float mVelocity = 0;
    // Internal tracking for value.
    float mValue = UNSET;
    // Min and max values that defines the range of the animation values.
    float mMaxValue = FLT_MAX;
    float mMinValue = -mMaxValue;

    // Tracks whether start value is set. If not, the animation will obtain the value at the time
    // of starting through the getter and use that as the starting value of the animation.
    bool mStartValueIsSet = false;
    // Package private tracking of animation lifecycle state. Visible to subclass animations.
    bool mRunning = false;

    // Target to be animated.
    void* mTarget;
    // View property id.
    const FloatProperty* mProperty;
private:
    // Last frame time. Always gets reset to -1  at the end of the animation.
    int64_t mLastFrameTime = 0;
    float mMinVisibleChange;

    // List of end listeners
    std::vector<OnAnimationEndListener> mEndListeners;
    // List of update listeners
    std::vector<OnAnimationUpdateListener> mUpdateListeners;
private:
    void startAnimationInternal();
    void endAnimationInternal(bool canceled);
public:
    DynamicAnimation(FloatValueHolder* floatValueHolder);

    DynamicAnimation(void* object,const FloatProperty* property);
    virtual ~DynamicAnimation();
    DynamicAnimation& setStartValue(float startValue);
    virtual DynamicAnimation& setStartVelocity(float startVelocity);

    virtual DynamicAnimation& setMaxValue(float max);
    virtual DynamicAnimation& setMinValue(float min);

    DynamicAnimation& addEndListener(const OnAnimationEndListener& listener);
    void removeEndListener(const OnAnimationEndListener& listener);

    DynamicAnimation& addUpdateListener(const OnAnimationUpdateListener& listener);
    void removeUpdateListener(const OnAnimationUpdateListener& listener);

    DynamicAnimation& setMinimumVisibleChange(float minimumVisibleChange);
    float getMinimumVisibleChange()const;

    /****************Animation Lifecycle Management***************/
    virtual void start();
    virtual void cancel();
    bool isRunning() const;

    /************************** Private APIs below ********************************/
    bool doAnimationFrame(int64_t frameTime)override;
    void commitAnimationFrame(int64_t)override;

    virtual bool updateValueAndVelocity(long deltaT)=0;

    void setPropertyValue(float value);

    float getValueThreshold() const;

    float getPropertyValue() const;

    AnimationHandler& getAnimationHandler()const;
    /****************Sub class animations**************/
    virtual float getAcceleration(float value, float velocity)const=0;
    virtual bool isAtEquilibrium(float value, float velocity)const=0;
    virtual void setValueThreshold(float threshold)=0;
};
}/*endof namespace*/
#endif/*__DYNAMIC_ANIMATION_H__*/
