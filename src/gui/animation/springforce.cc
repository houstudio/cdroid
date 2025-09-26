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

#include <animation/springforce.h>
namespace cdroid{
SpringForce::SpringForce() {
     mMassState = new DynamicAnimation::MassState();
}

/**
 * Creates a spring with a given final rest position.
 *
 * @param finalPosition final position of the spring when it reaches equilibrium
 */
SpringForce::SpringForce(float finalPosition)
    :SpringForce() {
    mFinalPosition = finalPosition;
}

SpringForce::~SpringForce(){
    delete mMassState;
}

SpringForce& SpringForce::setStiffness(float stiffness) {
    if (stiffness <= 0) {
        throw std::invalid_argument("Spring stiffness constant must be positive.");
    }
    mNaturalFreq = std::sqrt(stiffness);
    // All the intermediate values need to be recalculated.
    mInitialized = false;
    return *this;
}

float SpringForce::getStiffness() const{
    return (float) (mNaturalFreq * mNaturalFreq);
}

SpringForce& SpringForce::setDampingRatio(float dampingRatio) {
    if (dampingRatio < 0) {
        throw std::invalid_argument("Damping ratio must be non-negative");
    }
    mDampingRatio = dampingRatio;
    // All the intermediate values need to be recalculated.
    mInitialized = false;
    return *this;
}

float SpringForce::getDampingRatio() const{
    return (float) mDampingRatio;
}

SpringForce& SpringForce::setFinalPosition(float finalPosition) {
    mFinalPosition = finalPosition;
    return *this;
}

float SpringForce::getFinalPosition() const{
    return (float) mFinalPosition;
}

float SpringForce::getAcceleration(float lastDisplacement, float lastVelocity) const{

    lastDisplacement -= getFinalPosition();

    double k = mNaturalFreq * mNaturalFreq;
    double c = 2 * mNaturalFreq * mDampingRatio;

    return (float) (-k * lastDisplacement - c * lastVelocity);
}

bool SpringForce::isAtEquilibrium(float value, float velocity) const{
    if (std::abs(velocity) < mVelocityThreshold
            && std::abs(value - getFinalPosition()) < mValueThreshold) {
        return true;
    }
    return false;
}

void SpringForce::init() {
    if (mInitialized) {
        return;
    }

    if (mFinalPosition == UNSET) {
        throw std::runtime_error("Error: Final position of the spring must be"
                " set before the animation starts");
    }

    if (mDampingRatio > 1) {
        // Over damping
        mGammaPlus = -mDampingRatio * mNaturalFreq
                + mNaturalFreq * std::sqrt(mDampingRatio * mDampingRatio - 1);
        mGammaMinus = -mDampingRatio * mNaturalFreq
                - mNaturalFreq * std::sqrt(mDampingRatio * mDampingRatio - 1);
    } else if (mDampingRatio >= 0 && mDampingRatio < 1) {
        // Under damping
        mDampedFreq = mNaturalFreq * std::sqrt(1 - mDampingRatio * mDampingRatio);
    }

    mInitialized = true;
}

DynamicAnimation::MassState* SpringForce::updateValues(double lastDisplacement, double lastVelocity,
        long timeElapsed) {
    init();

    double deltaT = timeElapsed / 1000.0; // unit: seconds
    lastDisplacement -= mFinalPosition;
    double displacement;
    double currentVelocity;
    if (mDampingRatio > 1) {
        // Overdamped
        double coeffA =  lastDisplacement - (mGammaMinus * lastDisplacement - lastVelocity)
                / (mGammaMinus - mGammaPlus);
        double coeffB =  (mGammaMinus * lastDisplacement - lastVelocity)
                / (mGammaMinus - mGammaPlus);
        displacement = coeffA * std::pow(M_E, mGammaMinus * deltaT)
                + coeffB * std::pow(M_E, mGammaPlus * deltaT);
        currentVelocity = coeffA * mGammaMinus * std::pow(M_E, mGammaMinus * deltaT)
                + coeffB * mGammaPlus * std::pow(M_E, mGammaPlus * deltaT);
    } else if (mDampingRatio == 1) {
        // Critically damped
        double coeffA = lastDisplacement;
        double coeffB = lastVelocity + mNaturalFreq * lastDisplacement;
        displacement = (coeffA + coeffB * deltaT) * std::pow(M_E, -mNaturalFreq * deltaT);
        currentVelocity = (coeffA + coeffB * deltaT) * std::pow(M_E, -mNaturalFreq * deltaT)
                * (-mNaturalFreq) + coeffB * std::pow(M_E, -mNaturalFreq * deltaT);
    } else {
        // Underdamped
        double cosCoeff = lastDisplacement;
        double sinCoeff = (1 / mDampedFreq) * (mDampingRatio * mNaturalFreq
                * lastDisplacement + lastVelocity);
        displacement = std::pow(M_E, -mDampingRatio * mNaturalFreq * deltaT)
                * (cosCoeff * std::cos(mDampedFreq * deltaT)
                + sinCoeff * std::sin(mDampedFreq * deltaT));
        currentVelocity = displacement * (-mNaturalFreq) * mDampingRatio
                + std::pow(M_E, -mDampingRatio * mNaturalFreq * deltaT)
                * (-mDampedFreq * cosCoeff * std::sin(mDampedFreq * deltaT)
                + mDampedFreq * sinCoeff * std::cos(mDampedFreq * deltaT));
    }

    mMassState->mValue = (float) (displacement + mFinalPosition);
    mMassState->mVelocity = (float) currentVelocity;
    return mMassState;
}

/**
 * This threshold defines how close the animation value needs to be before the animation can
 * finish. This default value is based on the property being animated, e.g. animations on alpha,
 * scale, translation or rotation would have different thresholds. This value should be small
 * enough to avoid visual glitch of "jumping to the end". But it shouldn't be so small that
 * animations take seconds to finish.
 *
 * @param threshold the difference between the animation value and final spring position that
 *                  is allowed to end the animation when velocity is very low
 */
void SpringForce::setValueThreshold(double threshold) {
    mValueThreshold = std::abs(threshold);
    mVelocityThreshold = mValueThreshold * VELOCITY_THRESHOLD_MULTIPLIER;
}
}
