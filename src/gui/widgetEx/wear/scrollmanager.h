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
#ifndef __WEAR_SCROLL_MANAGER_H__
#define __WEAR_SCROLL_MANAGER_H__
#include <widgetEx/recyclerview/recyclerview.h>

namespace cdroid{
class ScrollManager {
    // One second in milliseconds.
private:
    static constexpr int ONE_SEC_IN_MS = 1000;
    static constexpr float VELOCITY_MULTIPLIER = 1.5f;
    static constexpr float FLING_EDGE_RATIO = 1.5f;

    float mMinRadiusFraction;
    float mMinRadiusFractionSquared;

    float mScrollDegreesPerScreen;
    float mScrollRadiansPerScreen;

    float mScreenRadiusPx;
    float mScreenRadiusPxSquared;
    float mScrollPixelsPerRadian;
    float mLastAngleRadians;

    bool mDown;
    bool mScrolling;
    RecyclerView* mRecyclerView;
    VelocityTracker* mVelocityTracker;
private:
    static float normalizeAngleRadians(float angleRadians);
public:
    ScrollManager();
    ~ScrollManager();
    void setRecyclerView(RecyclerView* recyclerView, int width, int height);

    void clearRecyclerView();

    bool onTouchEvent(MotionEvent& event);

    void setScrollDegreesPerScreen(float degreesPerScreen);
    float getScrollDegreesPerScreen() const;

    void setBezelWidth(float fraction);
    float getBezelWidth() const;
};
}/*endof namespace*/
#endif/*__WEAR_SCROLL_MANAGER_H__*/
