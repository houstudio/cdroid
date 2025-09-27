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
#include <animation/flinganimation.h>
namespace cdroid{

FlingAnimation::FlingAnimation(FloatValueHolder* floatValueHolder)
    :DynamicAnimation(floatValueHolder){
    mFlingForce->setValueThreshold(getValueThreshold());
}

/**
 * This creates a FlingAnimation that animates the property of the given object.
 *
 * @param object the Object whose property will be animated
 * @param property the property to be animated
 * @param <K> the class on which the property is declared
 */
FlingAnimation::FlingAnimation(void* object,const FloatProperty* property)
    :DynamicAnimation(object, property){
    mFlingForce->setValueThreshold(getValueThreshold());
}

/**
 * Sets the friction for the fling animation. The greater the friction is, the sooner the
 * animation will slow down. When not set, the friction defaults to 1.
 *
 * @param friction the friction used in the animation
 * @return the animation whose friction will be scaled
 * @throws IllegalArgumentException if the input friction is not positive
 */
FlingAnimation& FlingAnimation::setFriction(float friction) {
    if (friction <= 0) {
        throw std::invalid_argument("Friction must be positive");
    }
    mFlingForce->setFrictionScalar(friction);
    return *this;
}

/**
 * Returns the friction being set on the animation via {@link #setFriction(float)}. If the
 * friction has not been set, the default friction of 1 will be returned.
 *
 * @return friction being used in the animation
 */
float FlingAnimation::getFriction() const{
    return mFlingForce->getFrictionScalar();
}

/**
 * Sets the min value of the animation. When a fling animation reaches the min value, the
 * animation will end immediately. Animations will not animate beyond the min value.
 *
 * @param minValue minimum value of the property to be animated
 * @return the Animation whose min value is being set
 */
FlingAnimation& FlingAnimation::setMinValue(float minValue) {
    DynamicAnimation::setMinValue(minValue);
    return *this;
}

/**
 * Sets the max value of the animation. When a fling animation reaches the max value, the
 * animation will end immediately. Animations will not animate beyond the max value.
 *
 * @param maxValue maximum value of the property to be animated
 * @return the Animation whose max value is being set
 */
FlingAnimation& FlingAnimation::setMaxValue(float maxValue) {
    DynamicAnimation::setMaxValue(maxValue);
    return *this;
}

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
FlingAnimation& FlingAnimation::setStartVelocity(float startVelocity) {
    DynamicAnimation::setStartVelocity(startVelocity);
    return *this;
}

bool FlingAnimation::updateValueAndVelocity(long deltaT) {

    MassState* state = mFlingForce->updateValueAndVelocity(mValue, mVelocity, deltaT);
    mValue = state->mValue;
    mVelocity = state->mVelocity;

    // When the animation hits the max/min value, consider animation done.
    if (mValue < mMinValue) {
        mValue = mMinValue;
        return true;
    }
    if (mValue > mMaxValue) {
        mValue = mMaxValue;
        return true;
    }

    if (isAtEquilibrium(mValue, mVelocity)) {
        return true;
    }
    return false;
}

float FlingAnimation::getAcceleration(float value, float velocity) const{
    return mFlingForce->getAcceleration(value, velocity);
}

bool FlingAnimation::isAtEquilibrium(float value, float velocity) const{
    return (value >= mMaxValue) || (value <= mMinValue)
            || mFlingForce->isAtEquilibrium(value, velocity);
}

void FlingAnimation::setValueThreshold(float threshold) {
    mFlingForce->setValueThreshold(threshold);
}

////private final DynamicAnimation.MassState mMassState = new DynamicAnimation.MassState();

void FlingAnimation::DragForce::setFrictionScalar(float frictionScalar) {
    mFriction = frictionScalar * DEFAULT_FRICTION;
}

float FlingAnimation::DragForce::getFrictionScalar() const{
    return mFriction / DEFAULT_FRICTION;
}

DynamicAnimation::MassState* FlingAnimation::DragForce::updateValueAndVelocity(float value, float velocity, long deltaT) {
    mMassState->mVelocity = (float) (velocity * std::exp((deltaT / 1000.f) * mFriction));
    mMassState->mValue = (float) (value - velocity / mFriction
            + velocity / mFriction * std::exp(mFriction * deltaT / 1000.f));
    if (isAtEquilibrium(mMassState->mValue, mMassState->mVelocity)) {
        mMassState->mVelocity = 0.f;
    }
    return mMassState;
}

float FlingAnimation::DragForce::getAcceleration(float position, float velocity) const{
    return velocity * mFriction;
}

bool FlingAnimation::DragForce::isAtEquilibrium(float value, float velocity) const{
    return std::abs(velocity) < mVelocityThreshold;
}

void FlingAnimation::DragForce::setValueThreshold(float threshold) {
    mVelocityThreshold = threshold * VELOCITY_THRESHOLD_MULTIPLIER;
}

}
