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

/*
 * Ported to C++ for CDROID from androidx.constraintlayout.core.motion.utils.SpringStopEngine.
 */
#include <widgetEx/constraintlayout/core/motion/springstopengine.h>

#include <cmath>

namespace cdroid {

void SpringStopEngine::springConfig(float currentPos, float target, float currentVelocity,
                                    float mass, float stiffness, float damping,
                                    float stopThreshold, int boundaryMode) {
    mTargetPos = target;
    mDamping = damping;
    mPos = currentPos;
    // Android stores currentVelocity in an @SuppressWarnings("unused") mLastVelocity (never read),
    // so the release velocity never actually drove the spring. CDROID seeds mV with it so a fling's
    // momentum carries into the settle animation.
    mV = currentVelocity;
    mStiffness = stiffness;
    mMass = mass;
    mStopThreshold = stopThreshold;
    mBoundaryMode = boundaryMode;
    mLastTime = 0;
}

float SpringStopEngine::getInterpolation(float time) {
    compute(time - mLastTime);
    mLastTime = time;
    if (isStopped()) {
        mPos = (float) mTargetPos;
    }
    return mPos;
}

bool SpringStopEngine::isStopped() const {
    const double x = (mPos - mTargetPos);
    const double k = mStiffness;
    const double v = mV;
    const double m = mMass;
    const double energy = v * v * m + k * x * x;
    const double max_def = std::sqrt(energy / k);
    return max_def <= mStopThreshold;
}

void SpringStopEngine::compute(double dt) {
    if (dt <= 0) return; // nothing to compute with no time delta

    const double k = mStiffness;
    const double c = mDamping;
    // Over-sample proportional to the spring's natural frequency and the step size — keeps the
    // midpoint integration stable at high stiffness / large frames.
    int overSample = (int) (1 + 9 / (std::sqrt(mStiffness / mMass) * dt * 4));
    dt /= overSample;

    for (int i = 0; i < overSample; i++) {
        double x = (mPos - mTargetPos);
        double a = (-k * x - c * mV) / mMass;
        // Refining the acceleration via the midpoint lifts accuracy above a naive Euler step.
        double avgV = mV + a * dt / 2;                       // pass 1: average velocity
        double avgX = mPos + dt * avgV / 2 - mTargetPos;     // pass 1: average position
        a = (-avgX * k - avgV * c) / mMass;                  // acceleration at the midpoint

        double dv = a * dt;                                  // velocity change over the step
        avgV = mV + dv / 2;                                  // average velocity = current + half change
        mV += (float) dv;
        mPos += (float) (avgV * dt);
        if (mBoundaryMode > 0) {
            if (mPos < 0 && ((mBoundaryMode & 1) == 1)) {   // bit0 = bounce off the 0 boundary
                mPos = -mPos;
                mV = -mV;
            }
            if (mPos > 1 && ((mBoundaryMode & 2) == 2)) {   // bit1 = bounce off the 1 boundary
                mPos = 2 - mPos;
                mV = -mV;
            }
        }
    }
}

} // namespace cdroid
