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
    float getVelocity(float time) const {
        (void)time;
        return mV;
    }
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
