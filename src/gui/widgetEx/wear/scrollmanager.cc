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
#include <core/mathutils.h>
#include <widgetEx/wear/scrollmanager.h>
namespace cdroid{

ScrollManager::ScrollManager(){
    mMinRadiusFraction = 0.0f;
    mMinRadiusFractionSquared = mMinRadiusFraction * mMinRadiusFraction;

    mScrollDegreesPerScreen = 180;
    mScrollRadiansPerScreen = MathUtils::toRadians(mScrollDegreesPerScreen);
 
    mDown = false;
    mScrolling = false;
    mRecyclerView = nullptr;
    mVelocityTracker = nullptr;
}

ScrollManager::~ScrollManager(){
    if(mVelocityTracker){
        mVelocityTracker->recycle();
    }
}

void ScrollManager::setRecyclerView(RecyclerView* recyclerView, int width, int height) {
    mRecyclerView = recyclerView;
    mScreenRadiusPx = std::max(width, height) / 2.f;
    mScreenRadiusPxSquared = mScreenRadiusPx * mScreenRadiusPx;
    mScrollPixelsPerRadian = height / mScrollRadiansPerScreen;
    mScrollRadiansPerScreen = (float) MathUtils::toRadians(mScrollDegreesPerScreen);
    mVelocityTracker = VelocityTracker::obtain();
}

/** Remove the binding with a {@link RecyclerView} */
void ScrollManager::clearRecyclerView() {
    mRecyclerView = nullptr;
}

bool ScrollManager::onTouchEvent(MotionEvent& event) {
    float deltaX = event.getRawX() - mScreenRadiusPx;
    float deltaY = event.getRawY() - mScreenRadiusPx;
    float radiusSquared = deltaX * deltaX + deltaY * deltaY;
    int velocityY =0;
    MotionEvent* vtev = MotionEvent::obtain(event);
    mVelocityTracker->addMovement(*vtev);
    vtev->recycle();

    switch (event.getActionMasked()) {
    case MotionEvent::ACTION_DOWN:
        if (radiusSquared / mScreenRadiusPxSquared > mMinRadiusFractionSquared) {
            mDown = true;
            return true; // Consume the event.
        }
        break;

    case MotionEvent::ACTION_MOVE:
        if (mScrolling) {
            float angleRadians = (float) std::atan2(deltaY, deltaX);
            float deltaRadians = angleRadians - mLastAngleRadians;
            deltaRadians = normalizeAngleRadians(deltaRadians);
            int scrollPixels = std::round(deltaRadians * mScrollPixelsPerRadian);
            if (scrollPixels != 0) {
                mRecyclerView->scrollBy(0 /* x */, scrollPixels /* y */);
                // Recompute deltaRadians in terms of rounded scrollPixels.
                deltaRadians = scrollPixels / mScrollPixelsPerRadian;
                mLastAngleRadians += deltaRadians;
                mLastAngleRadians = normalizeAngleRadians(mLastAngleRadians);
            }
            // Always consume the event so that we never break the circular scrolling
            // gesture.
            return true;
        }

        if (mDown) {
            float deltaXFromCenter = event.getRawX() - mScreenRadiusPx;
            float deltaYFromCenter = event.getRawY() - mScreenRadiusPx;
            float distFromCenter = (float) std::hypot(deltaXFromCenter, deltaYFromCenter);
            if (distFromCenter != 0) {
                deltaXFromCenter /= distFromCenter;
                deltaYFromCenter /= distFromCenter;

                mScrolling = true;
                mRecyclerView->invalidate();
                mLastAngleRadians = (float) std::atan2(deltaYFromCenter, deltaXFromCenter);
                return true; // Consume the event.
            }
        } else {
            // Double check we're not missing an event we should really be handling.
            if (radiusSquared / mScreenRadiusPxSquared > mMinRadiusFractionSquared) {
                mDown = true;
                return true; // Consume the event.
            }
        }
        break;

    case MotionEvent::ACTION_UP:
        mDown = false;
        mScrolling = false;
        mVelocityTracker->computeCurrentVelocity(ONE_SEC_IN_MS,
                mRecyclerView->getMaxFlingVelocity());
        velocityY = (int) mVelocityTracker->getYVelocity();
        if (event.getX() < FLING_EDGE_RATIO * mScreenRadiusPx) {
            velocityY = -velocityY;
        }
        mVelocityTracker->clear();
        if (std::abs(velocityY) > mRecyclerView->getMinFlingVelocity()) {
            return mRecyclerView->fling(0, (int) (VELOCITY_MULTIPLIER * velocityY));
        }
        break;

    case MotionEvent::ACTION_CANCEL:
        if (mDown) {
            mDown = false;
            mScrolling = false;
            mRecyclerView->invalidate();
            return true; // Consume the event.
        }
        break;
    }

    return false;
}

float ScrollManager::normalizeAngleRadians(float angleRadians) {
    if (angleRadians < -M_PI) {
        angleRadians = (float) (angleRadians + M_PI * 2.0);
    }
    if (angleRadians > M_PI) {
        angleRadians = (float) (angleRadians - M_PI * 2.0);
    }
    return angleRadians;
}

void ScrollManager::setScrollDegreesPerScreen(float degreesPerScreen){
    mScrollDegreesPerScreen = degreesPerScreen;
    mScrollRadiansPerScreen = (float) MathUtils::toRadians(mScrollDegreesPerScreen);
}

void ScrollManager::setBezelWidth(float fraction) {
    mMinRadiusFraction = 1.f - fraction;
    mMinRadiusFractionSquared = mMinRadiusFraction * mMinRadiusFraction;
}

float ScrollManager::getScrollDegreesPerScreen() const{
    return mScrollDegreesPerScreen;
}

float ScrollManager::getBezelWidth() const{
    return 1.0f - mMinRadiusFraction;
}
}/*endof namespace*/
