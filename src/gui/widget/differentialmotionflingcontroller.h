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
#ifndef __DIFFERENTIAL_MOTION_FLING_CONTROLLER_H__
#define __DIFFERENTIAL_MOTION_FLING_CONTROLLER_H__
#include <climits>

namespace cdroid{

class Context;
class MotionEvent;
class VelocityTracker;

/* Port of androidx.core.view.DifferentialMotionFlingTarget. Represents an entity that may be
 * flung by a differential motion (e.g. mouse wheel / rotary AXIS_SCROLL). */
class DifferentialMotionFlingTarget {
public:
    virtual ~DifferentialMotionFlingTarget() = default;
    // Start flinging by a given velocity (pixels/second). Returns true if fling was initiated.
    virtual bool startDifferentialMotionFling(float velocity) = 0;
    // Stop any ongoing fling on the target caused by a differential motion.
    virtual void stopDifferentialMotionFling() = 0;
    // Raw MotionEvent axis value multiplied by this factor yields pixels.
    virtual float getScaledScrollFactor() = 0;
};

/* Port of androidx.core.view.DifferentialMotionFlingController. Orchestrates fling for
 * differential motions reported on the target View.
 *
 * The two @VisibleForTesting function-interface injection points (FlingVelocityThresholdCalculator
 * and DifferentialVelocityProvider) from the source are omitted; the production code path uses the
 * static default implementations directly. androidx.core.view.ViewConfigurationCompat and
 * VelocityTrackerCompat are thin shims and are inlined here to the CDROID framework APIs. */
class DifferentialMotionFlingController {
public:
    DifferentialMotionFlingController(Context* context, DifferentialMotionFlingTarget* target);
    ~DifferentialMotionFlingController();
    // Called to report when a differential motion happens on the target View.
    void onMotionEvent(MotionEvent& event, int axis);
private:
    Context* mContext;
    DifferentialMotionFlingTarget* mTarget;
    VelocityTracker* mVelocityTracker = nullptr;
    float mLastFlingVelocity = 0.f;
    int mLastProcessedAxis = -1;
    int mLastProcessedSource = -1;
    int mLastProcessedDeviceId = -1;
    // Initialize min and max to +infinity and 0, to effectively disable fling at start.
    int mFlingVelocityThresholds[2] = {INT_MAX, 0};
private:
    bool calculateFlingVelocityThresholds(MotionEvent& event, int axis);
    float getCurrentVelocity(MotionEvent& event, int axis);
    static void calculateFlingVelocityThresholds(Context* context, int* buffer,
            MotionEvent& event, int axis);
    static float getCurrentVelocity(VelocityTracker* vt, MotionEvent& event, int axis);
};

}/*endof namespace*/
#endif/*__DIFFERENTIAL_MOTION_FLING_CONTROLLER_H__*/
