/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Ported to C++ for CDROID from androidx.constraintlayout.motion.widget.TouchResponse.
 */
#include <widgetEx/constraintlayout/touchresponse.h>

#include <cmath>

#include <widgetEx/constraintlayout/motionlayout.h>

namespace cdroid {

namespace {
// TOUCH_DIRECTION[dragDirection] and TOUCH_SIDES[touchAnchorSide] (Android TouchResponse tables).
constexpr float kDir[6][2] = {      // up, down, left, right, start, end
    {0, -1}, {0, 1}, {-1, 0}, {1, 0}, {-1, 0}, {1, 0}
};
constexpr float kSide[7][2] = {     // top, left, right, bottom, middle, start, end
    {0.5f, 0.0f}, {0.0f, 0.5f}, {1.0f, 0.5f}, {0.5f, 1.0f},
    {0.5f, 0.5f}, {0.0f, 0.5f}, {1.0f, 0.5f}
};
} // namespace

TouchResponse::TouchResponse(MotionLayout* layout, const MotionScene::OnSwipe& cfg)
    : mLayout(layout)
    , mTouchAnchorId(cfg.touchAnchorId)
    , mOnTouchUp(cfg.onTouchUp)
    , mDragScale(cfg.dragScale) {
    const int d = cfg.dragDirection;
    mTouchDirX = (d >= 0 && d <= 5) ? kDir[d][0] : 1.0f;
    mTouchDirY = (d >= 0 && d <= 5) ? kDir[d][1] : 0.0f;
    const int s = cfg.touchAnchorSide;
    mAnchorLocX = (s >= 0 && s <= 6) ? kSide[s][0] : 0.5f;
    mAnchorLocY = (s >= 0 && s <= 6) ? kSide[s][1] : 0.5f;
}

void TouchResponse::onDown(float x, float y) {
    mLastX = x;
    mLastY = y;
    mDragVx = mDragVy = 0;
    mDragStarted = false;
}

bool TouchResponse::dragSlopExceeded(float x, float y) const {
    constexpr float kTouchSlop = 8.0f; // ViewConfiguration::TOUCH_SLOP
    const float dx = x - mLastX;
    const float dy = y - mLastY;
    const float moved = (mTouchDirX != 0) ? (dx * mTouchDirX) : (dy * mTouchDirY);
    return std::abs(moved) > kTouchSlop;
}

bool TouchResponse::onMove(float x, float y) {
    const float dx = x - mLastX;
    const float dy = y - mLastY;
    mLastX = x;
    mLastY = y;
    mDragVx = dx;
    mDragVy = dy;

    float pos = mLayout->getProgress();
    if (!mDragStarted) {
        mDragStarted = true;
        // Claim the gesture so an outer scroller/pager (e.g. ViewPager2) does not steal it.
        mLayout->requestDisallowInterceptTouchEvent(true);
    }

    // Map the drag delta to a progress delta via the anchor's pixels-per-progress (getAnchorDpDt).
    float dpdt[2] = {0, 0};
    mLayout->getAnchorDpDt(mTouchAnchorId, pos, mAnchorLocX, mAnchorLocY, dpdt);
    const float movementInDir = mTouchDirX * dpdt[0] + mTouchDirY * dpdt[1];
    float change;
    if (std::abs(movementInDir) > 0.01f) {
        change = (dx * mTouchDirX + dy * mTouchDirY) / movementInDir;
    } else {
        // No anchor (or it barely moves): fall back to the layout's own dimension as the range.
        const float delta = (mTouchDirX != 0) ? (dx * mTouchDirX) : (dy * mTouchDirY);
        const float range = (mTouchDirX != 0) ? mLayout->getWidth() : mLayout->getHeight();
        change = (range > 0) ? delta / range : 0.0f;
    }
    pos += change;
    if (pos < 0.0f) pos = 0.0f;
    if (pos > 1.0f) pos = 1.0f;
    mLayout->setProgress(pos);
    return true;
}

void TouchResponse::onUp(float /*x*/, float /*y*/) {
    const bool wasDragging = mDragStarted;
    mDragStarted = false;
    mLayout->requestDisallowInterceptTouchEvent(false); // release the gesture to the parent again

    if (mOnTouchUp == MotionScene::OnSwipe::ON_UP_STOP) return; // hold at the dragged-to progress

    // Direction: velocity if the last drag was significant, else the current progress position.
    const float pos = mLayout->getProgress();
    float dpdt[2] = {0, 0};
    mLayout->getAnchorDpDt(mTouchAnchorId, pos, mAnchorLocX, mAnchorLocY, dpdt);
    const float movementInDir = mTouchDirX * dpdt[0] + mTouchDirY * dpdt[1];
    bool towardEnd = (pos >= 0.5f);
    if (std::abs(movementInDir) > 0.01f) {
        const float velocity = (mDragVx * mTouchDirX + mDragVy * mTouchDirY) / movementInDir;
        if (std::abs(velocity) > 0.5f) towardEnd = (velocity > 0);
    }
    if (!wasDragging) return;
    if (mOnTouchUp == MotionScene::OnSwipe::ON_UP_AUTOCOMPLETE_TO_START) towardEnd = false;
    if (mOnTouchUp == MotionScene::OnSwipe::ON_UP_AUTOCOMPLETE_TO_END)   towardEnd = true;
    if (towardEnd) mLayout->transitionToEnd();
    else           mLayout->transitionToStart();
}

} // namespace cdroid
