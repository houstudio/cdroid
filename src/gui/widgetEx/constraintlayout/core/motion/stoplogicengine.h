/*
 * Copyright (C) 2020 The Android Open Source Project
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
 * Ported to C++ for CDROID from androidx.constraintlayout.core.motion.utils.StopLogicEngine.
 *
 * Velocity-profile stop engine: drives a value from currentPos to destination carrying an initial
 * velocity, ending at zero velocity, via a piecewise-constant-acceleration profile of 1-3 stages
 * (accelerate / cruise / decelerate, chosen from the distance, velocity and acceleration limits so
 * that velocity is continuous across stages). Used by MotionLayout's <OnSwipe> continuous-velocity
 * auto-completion (the Android default on release). All times are in seconds; velocity in
 * progress/sec; acceleration in progress/sec^2.
 */
#ifndef CDROID_CONSTRAINTLAYOUT_CORE_MOTION_STOP_LOGIC_ENGINE_H
#define CDROID_CONSTRAINTLAYOUT_CORE_MOTION_STOP_LOGIC_ENGINE_H

namespace cdroid {

class StopLogicEngine {
public:
    // Configure the profile: animate from currentPos to destination starting at currentVelocity
    // (progress/sec), taking at most maxTime seconds, bounded by maxAcceleration and maxVelocity.
    void config(float currentPos, float destination, float currentVelocity,
                float maxTime, float maxAcceleration, float maxVelocity);

    // Position at `time` seconds since config (currentPos-relative sign handled internally).
    float getInterpolation(float time);
    // Velocity (progress/sec) at the last getInterpolation time, sign-corrected for direction.
    float getVelocity() const;
    // Velocity (progress/sec) at an explicit time (StopEngine interface parity).
    float getVelocity(float time) const;
    // True once the profile is exhausted (time past the last stage).
    bool isStopped() const;

private:
    float calcY(float time);
    void setup(float velocity, float distance, float maxAcceleration, float maxVelocity, float maxTime);

    float mStage1Velocity = 0, mStage2Velocity = 0, mStage3Velocity = 0;
    float mStage1Duration = 0, mStage2Duration = 0, mStage3Duration = 0;
    float mStage1EndPosition = 0, mStage2EndPosition = 0, mStage3EndPosition = 0;
    int   mNumberOfStages = 0;
    bool  mBackwards = false;
    float mStartPosition = 0;
    float mLastTime = 0;
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_CORE_MOTION_STOP_LOGIC_ENGINE_H
