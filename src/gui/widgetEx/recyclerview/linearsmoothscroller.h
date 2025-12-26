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
#ifndef __LINEAR_SMOOTH_SCROLLER_H__
#define __LINEAR_SMOOTH_SCROLLER_H__
#include <widgetEx/recyclerview/recyclerview.h>
namespace cdroid{
class LinearSmoothScroller:public RecyclerView::SmoothScroller {
private:
    static constexpr float MILLISECONDS_PER_INCH = 25.f;
    static constexpr int TARGET_SEEK_SCROLL_DISTANCE_PX = 10000;
    static constexpr float TARGET_SEEK_EXTRA_SCROLL_RATIO = 1.2f;
    float mMillisPerPixel;
    DisplayMetrics mDisplayMetrics;
public:
    static constexpr int SNAP_TO_START = -1;
    static constexpr int SNAP_TO_END = 1;
    static constexpr int SNAP_TO_ANY = 0;
protected:
    const LinearInterpolator* mLinearInterpolator;
    const DecelerateInterpolator* mDecelerateInterpolator;
    PointF mTargetVector;
    int mInterimTargetDx = 0;
    int mInterimTargetDy = 0;
    bool mHasCalculatedMillisPerPixel;
    bool mTargetVectorUsable;
private:
    float getSpeedPerPixel();
protected:
    float calculateSpeedPerPixel(const DisplayMetrics& displayMetrics) const;
public:
    LinearSmoothScroller(Context* context);
    ~LinearSmoothScroller()override;
    void onStart()override;
    void onTargetFound(View* targetView, RecyclerView::State& state, Action& action)override;
    void onSeekTargetStep(int dx, int dy, RecyclerView::State& state, Action& action)override;
    void onStop()override;
    virtual float calculateSpeedPerPixel(DisplayMetrics& displayMetrics);
    int calculateTimeForDeceleration(int dx);
    virtual int calculateTimeForScrolling(int dx);
    int getHorizontalSnapPreference();
    int getVerticalSnapPreference();
    void updateActionForInterimTarget(Action& action);
    int clampApplyScroll(int tmpDt, int dt);
    virtual int calculateDtToFit(int viewStart, int viewEnd, int boxStart, int boxEnd, int
        snapPreference);

    virtual int calculateDyToMakeVisible(View* view, int snapPreference);
    virtual int calculateDxToMakeVisible(View* view, int snapPreference);
};
}/*endof namespace*/
#endif/*__LINEAR_SMOOTH_SCROLLER_H__*/
