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
 * Ported to C++ for CDROID from androidx.constraintlayout.motion.widget.TouchResponse.
 *
 * Drives a MotionLayout transition from a touch drag (the <OnSwipe> of a MotionScene). The drag is
 * mapped to progress via the anchor's travel distance: as progress goes 0->1 the anchor point (on
 * the touchAnchorId view) moves from its start to end position, and a drag delta projects onto that
 * travel to yield a progress delta (Motion.getDpDt gives pixels-per-progress). On release the
 * transition auto-completes to the nearest end (or the velocity-favoured one).
 *
 * Falls back to the layout's own dimension as the drag range when no anchor is set. On drag start it
 * calls requestDisallowInterceptTouchEvent(true) so an outer scroller/pager does not steal the
 * gesture (e.g. horizontal OnSwipe inside a ViewPager2).
 *
 * Deferred (faithful): spring physics (SpringStopEngine), velocity-tracked fling completion,
 * nestedScrollFlags, touchRegion, rotation mode.
 */
#ifndef CDROID_CONSTRAINTLAYOUT_WIDGET_TOUCH_RESPONSE_H
#define CDROID_CONSTRAINTLAYOUT_WIDGET_TOUCH_RESPONSE_H

#include <widgetEx/constraintlayout/motion/motionscene.h>

namespace cdroid {

class MotionEvent;
class MotionLayout;
class VelocityTracker;

class TouchResponse {
  public:
    // Build from the scene's <OnSwipe> config. `layout` must outlive this TouchResponse.
    TouchResponse(MotionLayout* layout, const MotionScene::OnSwipe& cfg);
    ~TouchResponse();

    // MotionEvent stages. onMove returns true once a drag is in progress.
    void onDown(const MotionEvent& evt);
    // True once the finger has moved past touch slop in the drag direction (MotionLayout uses this
    // to decide whether to intercept — so a tap still reaches <OnClick> children).
    bool dragSlopExceeded(const MotionEvent& evt) const;
    bool onMove(const MotionEvent& evt);
    void onUp(const MotionEvent& evt);

    bool dragStarted() const {
        return mDragStarted;
    }

  private:
    MotionLayout* mLayout;
    int   mTouchAnchorId;          // MotionScene::UNSET if none -> layout-dimension fallback
    float mAnchorLocX, mAnchorLocY; // anchor point on the widget (fraction), from touchAnchorSide
    float mTouchDirX, mTouchDirY;   // unit vector of the drag direction (TOUCH_DIRECTION table)
    int   mOnTouchUp;
    float mDragScale;
    int   mAutoCompleteMode = MotionScene::OnSwipe::COMPLETE_CONTINUOUS_VELOCITY;
    float mMaxVelocity = 4.0f;       // progress/sec cap (continuous-velocity auto-completion)
    float mMaxAcceleration = 1.2f;   // progress/sec^2 cap (continuous-velocity auto-completion)
    float mSpringMass = 1.0f, mSpringStiffness = 400.0f, mSpringDamping = 10.0f;
    float mSpringStopThreshold = 0.01f;
    int   mSpringBoundary = 0;

    VelocityTracker* mVelocityTracker = nullptr; // px/s tracking for the spring release velocity
    float mLastX = 0, mLastY = 0;
    float mDragVx = 0, mDragVy = 0; // recent drag delta (fallback if no tracker)
    bool  mDragStarted = false;
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_WIDGET_TOUCH_RESPONSE_H
