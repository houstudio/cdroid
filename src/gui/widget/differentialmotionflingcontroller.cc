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
#include <widget/differentialmotionflingcontroller.h>
#include <view/viewconfiguration.h>
#include <view/velocitytracker.h>
#include <view/motionevent.h>
#include <cmath>

namespace cdroid{

namespace {
// Equivalent of java.lang.Math.signum(float) without pulling in MathUtils.
inline float mathSignum(float v) {
    return (v > 0.f) ? 1.f : (v < 0.f ? -1.f : 0.f);
}
}

DifferentialMotionFlingController::DifferentialMotionFlingController(Context* context,
        DifferentialMotionFlingTarget* target) {
    mContext = context;
    mTarget = target;
}

DifferentialMotionFlingController::~DifferentialMotionFlingController() {
    if (mVelocityTracker != nullptr) {
        mVelocityTracker->recycle();
        mVelocityTracker = nullptr;
    }
}

void DifferentialMotionFlingController::onMotionEvent(MotionEvent& event, int axis) {
    bool flingParamsChanged = calculateFlingVelocityThresholds(event, axis);
    if (mFlingVelocityThresholds[0] == INT_MAX) {
        // Integer.MAX_VALUE means that the device does not support fling for the current
        // configuration. Do not proceed any further.
        if (mVelocityTracker != nullptr) {
            mVelocityTracker->recycle();
            mVelocityTracker = nullptr;
        }
        return;
    }

    float scaledVelocity = getCurrentVelocity(event, axis) * mTarget->getScaledScrollFactor();

    float velocityDirection = mathSignum(scaledVelocity);
    // Stop ongoing fling if there has been state changes affecting fling, or if the current
    // velocity (if non-zero) is opposite of the velocity that last caused fling.
    if (flingParamsChanged
            || (velocityDirection != mathSignum(mLastFlingVelocity)
                && velocityDirection != 0.f)) {
        mTarget->stopDifferentialMotionFling();
    }

    if (std::abs(scaledVelocity) < mFlingVelocityThresholds[0]) {
        return;
    }

    // Clamp the scaled velocity between [-max, max].
    scaledVelocity = std::max((float) -mFlingVelocityThresholds[1],
            std::min(scaledVelocity, (float) mFlingVelocityThresholds[1]));

    bool flung = mTarget->startDifferentialMotionFling(scaledVelocity);
    mLastFlingVelocity = flung ? scaledVelocity : 0.f;
}

bool DifferentialMotionFlingController::calculateFlingVelocityThresholds(MotionEvent& event, int axis) {
    int source = event.getSource();
    int deviceId = event.getDeviceId();
    if (mLastProcessedSource != source
            || mLastProcessedDeviceId != deviceId
            || mLastProcessedAxis != axis) {
        calculateFlingVelocityThresholds(mContext, mFlingVelocityThresholds, event, axis);
        // Save data about this processing so that we don't have to re-process fling thresholds
        // for similar parameters.
        mLastProcessedSource = source;
        mLastProcessedDeviceId = deviceId;
        mLastProcessedAxis = axis;
        return true;
    }
    return false;
}

void DifferentialMotionFlingController::calculateFlingVelocityThresholds(Context* context,
        int* buffer, MotionEvent& event, int axis) {
    ViewConfiguration& vc = ViewConfiguration::get(context);
    buffer[0] = vc.getScaledMinimumFlingVelocity(event.getDeviceId(), axis, event.getSource());
    buffer[1] = vc.getScaledMaximumFlingVelocity(event.getDeviceId(), axis, event.getSource());
}

float DifferentialMotionFlingController::getCurrentVelocity(MotionEvent& event, int axis) {
    if (mVelocityTracker == nullptr) {
        mVelocityTracker = VelocityTracker::obtain();
    }
    return getCurrentVelocity(mVelocityTracker, event, axis);
}

float DifferentialMotionFlingController::getCurrentVelocity(VelocityTracker* vt,
        MotionEvent& event, int axis) {
    vt->addMovement(event);
    vt->computeCurrentVelocity(1000);
    return vt->getAxisVelocity(axis);
}

}/*endof namespace*/
