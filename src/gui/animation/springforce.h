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
#ifndef __DYNAMIC_SPRING_FORCE_H__
#define __DYNAMIC_SPRING_FORCE_H__
#include <cfloat>
#include <animation/force.h>
#include <animation/dynamicanimation.h>
namespace cdroid{
class SpringForce:public Force {
public:
    static constexpr float STIFFNESS_HIGH = 10000.f;
    static constexpr float STIFFNESS_MEDIUM = 1500.f;
    static constexpr float STIFFNESS_LOW = 200.f;
    static constexpr float STIFFNESS_VERY_LOW = 50.f;
    static constexpr float DAMPING_RATIO_HIGH_BOUNCY = 0.2f;
    static constexpr float DAMPING_RATIO_MEDIUM_BOUNCY = 0.5f;
    static constexpr float DAMPING_RATIO_LOW_BOUNCY = 0.75f;
    static constexpr float DAMPING_RATIO_NO_BOUNCY = 1.f;

private:
    static constexpr double VELOCITY_THRESHOLD_MULTIPLIER = 1000.0 / 16.0;
    static constexpr double UNSET = DBL_MAX;
public:
    // Natural frequency
    double mNaturalFreq = std::sqrt(STIFFNESS_MEDIUM);
    // Damping ratio.
    double mDampingRatio = DAMPING_RATIO_MEDIUM_BOUNCY;
private:
    // Indicates whether the spring has been initialized
    bool mInitialized = false;

    // Threshold for velocity and value to determine when it's reasonable to assume that the spring
    // is approximately at rest.
    double mValueThreshold;
    double mVelocityThreshold;

    // Intermediate values to simplify the spring function calculation per frame.
    double mGammaPlus;
    double mGammaMinus;
    double mDampedFreq;

    // Final position of the spring. This must be set before the start of the animation.
    double mFinalPosition = UNSET;

    // Internal state to hold a value/velocity pair.
    DynamicAnimation::MassState* mMassState;
private:
    void init();
public:
    SpringForce();
    SpringForce(float finalPosition);
    ~SpringForce()override;
    /**
     * Sets the stiffness of a spring. The more stiff a spring is, the more force it applies to
     * the object attached when the spring is not at the final position. Default stiffness is
     * {@link #STIFFNESS_MEDIUM}.
     *
     * @param stiffness non-negative stiffness constant of a spring
     * @return the spring force that the given stiffness is set on
     * @throws IllegalArgumentException if the given spring stiffness is not positive
     */
    SpringForce& setStiffness(float stiffness);
    float getStiffness()const;

    /**
     * Spring damping ratio describes how oscillations in a system decay after a disturbance.
     * <p>
     * When damping ratio > 1 (over-damped), the object will quickly return to the rest position
     * without overshooting. If damping ratio equals to 1 (i.e. critically damped), the object will
     * return to equilibrium within the shortest amount of time. When damping ratio is less than 1
     * (i.e. under-damped), the mass tends to overshoot, and return, and overshoot again. Without
     * any damping (i.e. damping ratio = 0), the mass will oscillate forever.
     * <p>
     * Default damping ratio is {@link #DAMPING_RATIO_MEDIUM_BOUNCY}.
     *
     * @param dampingRatio damping ratio of the spring, it should be non-negative
     * @return the spring force that the given damping ratio is set on
     * @throws IllegalArgumentException if the {@param dampingRatio} is negative.
     */
    SpringForce& setDampingRatio(float dampingRatio);
    float getDampingRatio() const;

    /**
     * Sets the rest position of the spring.
     *
     * @param finalPosition rest position of the spring
     * @return the spring force that the given final position is set on
     */
    SpringForce& setFinalPosition(float finalPosition);
    float getFinalPosition() const;

    /*********************** Below are private APIs *********************/
    float getAcceleration(float lastDisplacement, float lastVelocity) const override;
    bool isAtEquilibrium(float value, float velocity) const override;
    
    DynamicAnimation::MassState* updateValues(double lastDisplacement, double lastVelocity,long timeElapsed);
    void setValueThreshold(double threshold);
};
}/*endof namespace*/
#endif/*__DYNAMIC_SPRING_FORCE_H__*/
