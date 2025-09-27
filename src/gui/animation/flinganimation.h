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
#ifndef __FLING_ANIMATION_H__
#define __FLING_ANIMATION_H__
#include <animation/force.h>
#include <animation/dynamicanimation.h>
namespace cdroid{
class FlingAnimation:public DynamicAnimation {
private:
    class DragForce;
    DragForce* mFlingForce;
public:
    FlingAnimation(FloatValueHolder* floatValueHolder);
    FlingAnimation(void* object,const FloatProperty* property);

    /**
     * Sets the friction for the fling animation. The greater the friction is, the sooner the
     * animation will slow down. When not set, the friction defaults to 1.
     *
     * @param friction the friction used in the animation
     * @return the animation whose friction will be scaled
     * @throws IllegalArgumentException if the input friction is not positive
     */
    FlingAnimation& setFriction(float friction);

    /**
     * Returns the friction being set on the animation via {@link #setFriction(float)}. If the
     * friction has not been set, the default friction of 1 will be returned.
     *
     * @return friction being used in the animation
     */
    float getFriction() const;

    /**
     * Sets the min value of the animation. When a fling animation reaches the min value, the
     * animation will end immediately. Animations will not animate beyond the min value.
     *
     * @param minValue minimum value of the property to be animated
     * @return the Animation whose min value is being set
     */
    FlingAnimation& setMinValue(float minValue) override;

    /**
     * Sets the max value of the animation. When a fling animation reaches the max value, the
     * animation will end immediately. Animations will not animate beyond the max value.
     *
     * @param maxValue maximum value of the property to be animated
     * @return the Animation whose max value is being set
     */
    FlingAnimation& setMaxValue(float maxValue) override;

    /**
     * Start velocity of the animation. Default velocity is 0. Unit: pixel/second
     *
     * <p>A <b>non-zero</b> start velocity is required for a FlingAnimation. If no start velocity is
     * set through {@link #setStartVelocity(float)}, the start velocity defaults to 0. In that
     * case, the fling animation will consider itself done in the next frame.
     *
     * <p>Note when using a fixed value as the start velocity (as opposed to getting the velocity
     * through touch events), it is recommended to define such a value in dp/second and convert it
     * to pixel/second based on the density of the screen to achieve a consistent look across
     * different screens.
     *
     * <p>To convert from dp/second to pixel/second:
     * <pre class="prettyprint">
     * float pixelPerSecond = TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP, dpPerSecond,
     *         getResources().getDisplayMetrics());
     * </pre>
     *
     * @param startVelocity start velocity of the animation in pixel/second
     * @return the Animation whose start velocity is being set
     */
    FlingAnimation& setStartVelocity(float startVelocity) override;

    bool updateValueAndVelocity(long deltaT) override;

    float getAcceleration(float value, float velocity) const override;

    bool isAtEquilibrium(float value, float velocity) const override;
    void setValueThreshold(float threshold) override;
};
class FlingAnimation::DragForce :public Force {
private:
    static constexpr float DEFAULT_FRICTION = -4.2f;

    // This multiplier is used to calculate the velocity threshold given a certain value
    // threshold. The idea is that if it takes >= 1 frame to move the value threshold amount,
    // then the velocity is a reasonable threshold.
    static constexpr float VELOCITY_THRESHOLD_MULTIPLIER = 1000.f / 16.f;
    float mFriction = DEFAULT_FRICTION;
    float mVelocityThreshold;

    // Internal state to hold a value/velocity pair.
    DynamicAnimation::MassState* mMassState;// = new DynamicAnimation.MassState();
public:
    void setFrictionScalar(float frictionScalar);
    float getFrictionScalar() const;

    MassState* updateValueAndVelocity(float value, float velocity, long deltaT);

    float getAcceleration(float position, float velocity)const override;

    bool isAtEquilibrium(float value, float velocity)const override;
    void setValueThreshold(float threshold);
};

}/*endof namespace*/
#endif/*__FLING_ANIMATION_H__*/
