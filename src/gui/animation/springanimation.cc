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
#include <animation/springanimation.h>
namespace cdroid{

SpringAnimation::SpringAnimation(FloatValueHolder* floatValueHolder)
    :DynamicAnimation(floatValueHolder){
    mSpring = nullptr;
}

SpringAnimation::SpringAnimation(void*object,const FloatProperty* property)
    :DynamicAnimation(object, property){
    mSpring = nullptr;
}

SpringAnimation::SpringAnimation(void* object,const FloatProperty*property,float finalPosition)
    :DynamicAnimation(object, property){
    mSpring = new SpringForce(finalPosition);
}

SpringAnimation::~SpringAnimation(){
    delete mSpring;
}

SpringForce* SpringAnimation::getSpring() const{
    return mSpring;
}

SpringAnimation& SpringAnimation::setSpring(SpringForce* force) {
    mSpring = force;
    return *this;
}

void SpringAnimation::start() {
    sanityCheck();
    mSpring->setValueThreshold(getValueThreshold());
    DynamicAnimation::start();
}

void SpringAnimation::animateToFinalPosition(float finalPosition) {
    if (isRunning()) {
        mPendingPosition = finalPosition;
    } else {
        if (mSpring == nullptr) {
            mSpring = new SpringForce(finalPosition);
        }
        mSpring->setFinalPosition(finalPosition);
        start();
    }
}

void SpringAnimation::skipToEnd() {
    if (!canSkipToEnd()) {
        throw std::runtime_error("Spring animations can only come to an end when there is damping");
    }
    if (Looper::myLooper() != Looper::getMainLooper()) {
        throw std::runtime_error("Animations may only be started on the main thread");
    }
    if (mRunning) {
        mEndRequested = true;
    }
}

bool SpringAnimation::canSkipToEnd() const{
    return mSpring->mDampingRatio > 0;
}

/************************ Below are private APIs *************************/

void SpringAnimation::sanityCheck() {
    if (mSpring == nullptr) {
        throw std::runtime_error("Incomplete SpringAnimation: Either final"
                " position or a spring force needs to be set.");
    }
    double finalPosition = mSpring->getFinalPosition();
    if (finalPosition > mMaxValue) {
        throw std::runtime_error("Final position of the spring cannot be greater than the max value.");
    } else if (finalPosition < mMinValue) {
        throw std::runtime_error("Final position of the spring cannot be less than the min value.");
    }
}

bool SpringAnimation::updateValueAndVelocity(long deltaT) {
    // If user had requested end, then update the value and velocity to end state and consider
    // animation done.
    if (mEndRequested) {
        if (mPendingPosition != UNSET) {
            mSpring->setFinalPosition(mPendingPosition);
            mPendingPosition = UNSET;
        }
        mValue = mSpring->getFinalPosition();
        mVelocity = 0;
        mEndRequested = false;
        return true;
    }

    if (mPendingPosition != UNSET) {
        double lastPosition = mSpring->getFinalPosition();
        // Approximate by considering half of the time spring position stayed at the old
        // position, half of the time it's at the new position.
        DynamicAnimation::MassState* massState = mSpring->updateValues(mValue, mVelocity, deltaT / 2);
        mSpring->setFinalPosition(mPendingPosition);
        mPendingPosition = UNSET;

        massState = mSpring->updateValues(massState->mValue, massState->mVelocity, deltaT / 2);
        mValue = massState->mValue;
        mVelocity = massState->mVelocity;

    } else {
        MassState* massState = mSpring->updateValues(mValue, mVelocity, deltaT);
        mValue = massState->mValue;
        mVelocity = massState->mVelocity;
    }

    mValue = std::max(mValue, mMinValue);
    mValue = std::min(mValue, mMaxValue);

    if (isAtEquilibrium(mValue, mVelocity)) {
        mValue = mSpring->getFinalPosition();
        mVelocity = 0.f;
        return true;
    }
    return false;
}

float SpringAnimation::getAcceleration(float value, float velocity) const{
    return mSpring->getAcceleration(value, velocity);
}

bool SpringAnimation::isAtEquilibrium(float value, float velocity) const{
    return mSpring->isAtEquilibrium(value, velocity);
}

void SpringAnimation::setValueThreshold(float threshold) {
}
}
