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
 */
#include <widgetEx/constraintlayout/core/motion/stoplogicengine.h>

#include <cmath>

namespace cdroid {

void StopLogicEngine::config(float currentPos, float destination, float currentVelocity,
                             float maxTime, float maxAcceleration, float maxVelocity) {
    mStartPosition = currentPos;
    mBackwards = (currentPos > destination);
    if (mBackwards) {
        setup(-currentVelocity, currentPos - destination, maxAcceleration, maxVelocity, maxTime);
    } else {
        setup(currentVelocity, destination - currentPos, maxAcceleration, maxVelocity, maxTime);
    }
}

float StopLogicEngine::getInterpolation(float v) {
    float y = calcY(v);
    mLastTime = v;
    return mBackwards ? (mStartPosition - y) : (mStartPosition + y);
}

float StopLogicEngine::getVelocity() const {
    return mBackwards ? -getVelocity(mLastTime) : getVelocity(mLastTime);
}

float StopLogicEngine::getVelocity(float time) const {
    if (time <= mStage1Duration) {
        return mStage1Velocity + (mStage2Velocity - mStage1Velocity) * time / mStage1Duration;
    }
    if (mNumberOfStages == 1) return 0; // past the single stage
    time -= mStage1Duration;
    if (time < mStage2Duration) {
        return mStage2Velocity + (mStage3Velocity - mStage2Velocity) * time / mStage2Duration;
    }
    if (mNumberOfStages == 2) return 0; // past the second stage
    time -= mStage2Duration;
    if (time < mStage3Duration) {
        return mStage3Velocity - mStage3Velocity * time / mStage3Duration;
    }
    return 0; // profile exhausted
}

bool StopLogicEngine::isStopped() const {
    const float total = mStage1Duration + mStage2Duration + mStage3Duration;
    return mLastTime >= total;
}

float StopLogicEngine::calcY(float time) {
    if (time <= mStage1Duration) {
        return mStage1Velocity * time
               + (mStage2Velocity - mStage1Velocity) * time * time / (2 * mStage1Duration);
    }
    if (mNumberOfStages == 1) {
        return mStage1EndPosition;
    }
    time -= mStage1Duration;
    if (time < mStage2Duration) {
        return mStage1EndPosition + mStage2Velocity * time
               + (mStage3Velocity - mStage2Velocity) * time * time / (2 * mStage2Duration);
    }
    if (mNumberOfStages == 2) {
        return mStage2EndPosition;
    }
    time -= mStage2Duration;
    if (time <= mStage3Duration) {
        return mStage2EndPosition + mStage3Velocity * time
               - mStage3Velocity * time * time / (2 * mStage3Duration);
    }
    return mStage3EndPosition;
}

void StopLogicEngine::setup(float velocity, float distance, float maxAcceleration,
                            float maxVelocity, float maxTime) {
    if (velocity == 0) velocity = 0.0001f;
    mStage1Velocity = velocity;
    float min_time_to_stop = velocity / maxAcceleration;
    float stopDistance = min_time_to_stop * velocity / 2;

    if (velocity < 0) { // moving backward (away from destination)
        float timeToZeroVelocity = -velocity / maxAcceleration;
        float reversDistanceTraveled = timeToZeroVelocity * velocity / 2;
        float totalDistance = distance - reversDistanceTraveled;
        float peak_v = std::sqrt(maxAcceleration * totalDistance);
        if (peak_v < maxVelocity) { // backward accelerate then decelerate
            mNumberOfStages = 2;
            mStage1Velocity = velocity;
            mStage2Velocity = peak_v;
            mStage3Velocity = 0;
            mStage1Duration = (peak_v - velocity) / maxAcceleration;
            mStage2Duration = peak_v / maxAcceleration;
            mStage1EndPosition = (velocity + peak_v) * mStage1Duration / 2;
            mStage2EndPosition = distance;
            mStage3EndPosition = distance;
            return;
        }
        // backward accelerate, cruise, decelerate
        mNumberOfStages = 3;
        mStage1Velocity = velocity;
        mStage2Velocity = maxVelocity;
        mStage3Velocity = maxVelocity;
        mStage1Duration = (maxVelocity - velocity) / maxAcceleration;
        mStage3Duration = maxVelocity / maxAcceleration;
        float accDist = (velocity + maxVelocity) * mStage1Duration / 2;
        float decDist = (maxVelocity * mStage3Duration) / 2;
        mStage2Duration = (distance - accDist - decDist) / maxVelocity;
        mStage1EndPosition = accDist;
        mStage2EndPosition = (distance - decDist);
        mStage3EndPosition = distance;
        return;
    }

    if (stopDistance >= distance) { // cannot stop in time — forced hard stop
        float time = 2 * distance / velocity;
        mNumberOfStages = 1;
        mStage1Velocity = velocity;
        mStage2Velocity = 0;
        mStage1EndPosition = distance;
        mStage1Duration = time;
        return;
    }

    float distance_before_break = distance - stopDistance;
    float cruseTime = distance_before_break / velocity; // just cruise then stop?
    if (cruseTime + min_time_to_stop < maxTime) { // close enough: maintain v then brake
        mNumberOfStages = 2;
        mStage1Velocity = velocity;
        mStage2Velocity = velocity;
        mStage3Velocity = 0;
        mStage1EndPosition = distance_before_break;
        mStage2EndPosition = distance;
        mStage1Duration = cruseTime;
        mStage2Duration = velocity / maxAcceleration;
        return;
    }

    float peak_v = std::sqrt(maxAcceleration * distance + velocity * velocity / 2);
    mStage1Duration = (peak_v - velocity) / maxAcceleration;
    mStage2Duration = peak_v / maxAcceleration;
    if (peak_v < maxVelocity) { // accelerate then decelerate
        mNumberOfStages = 2;
        mStage1Velocity = velocity;
        mStage2Velocity = peak_v;
        mStage3Velocity = 0;
        mStage1EndPosition = (velocity + peak_v) * mStage1Duration / 2;
        mStage2EndPosition = distance;
        return;
    }
    // accelerate, cruise, decelerate
    mNumberOfStages = 3;
    mStage1Velocity = velocity;
    mStage2Velocity = maxVelocity;
    mStage3Velocity = maxVelocity;
    mStage1Duration = (maxVelocity - velocity) / maxAcceleration;
    mStage3Duration = maxVelocity / maxAcceleration;
    float accDist = (velocity + maxVelocity) * mStage1Duration / 2;
    float decDist = (maxVelocity * mStage3Duration) / 2;
    mStage2Duration = (distance - accDist - decDist) / maxVelocity;
    mStage1EndPosition = accDist;
    mStage2EndPosition = (distance - decDist);
    mStage3EndPosition = distance;
}

} // namespace cdroid
