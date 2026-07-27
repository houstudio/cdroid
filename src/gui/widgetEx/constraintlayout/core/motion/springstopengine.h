/*
 * Copyright (C) 2021 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Ported to C++ for CDROID from androidx.constraintlayout.core.motion.utils.SpringStopEngine.
 *
 * 1-D damped spring stop engine: drives a value to a target via the damped-spring ODE
 * (a = (-k*x - c*v)/m), integrated with adaptive over-sampling + the midpoint method for stability
 * at high stiffness, with energy-threshold stopping and bitmask bounce boundaries. Used by
 * MotionLayout's <OnSwipe> spring auto-completion.
 */
#ifndef CDROID_CONSTRAINTLAYOUT_CORE_MOTION_SPRING_STOP_ENGINE_H
#define CDROID_CONSTRAINTLAYOUT_CORE_MOTION_SPRING_STOP_ENGINE_H

namespace cdroid {

class SpringStopEngine {
public:
    // Configure the spring: start at currentPos with currentVelocity, animating to target.
    //   mass/stiffness/damping — the ODE parameters (k = stiffness, c = damping, m = mass)
    //   stopThreshold — the max residual deflection (energy-based) at which the spring is settled
    //   boundaryMode — 0 = overshoot freely; bit0 (1) = bounce off 0; bit1 (2) = bounce off 1; 3 = both
    void springConfig(float currentPos, float target, float currentVelocity,
                      float mass, float stiffness, float damping,
                      float stopThreshold, int boundaryMode);

    // Advance the spring by (time - lastTime) seconds and return the current position. `time` is
    // cumulative seconds since the spring started (monotonic). On settle the position snaps to target.
    float getInterpolation(float time);
    // Current velocity (progress/sec). The time arg is unused (kept for StopEngine interface parity).
    float getVelocity(float time) const { (void)time; return mV; }
    // True once the spring's residual energy can't deflect it past stopThreshold from the target.
    bool isStopped() const;

private:
    void compute(double dt);

    double mDamping = 0.5;
    double mStiffness = 0;
    double mTargetPos = 0;
    float  mLastTime = 0;
    float  mPos = 0;
    float  mV = 0;
    float  mMass = 0;
    float  mStopThreshold = 0;
    int    mBoundaryMode = 0;
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_CORE_MOTION_SPRING_STOP_ENGINE_H
