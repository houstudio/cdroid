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
#ifndef __AUTOSCROLL_HELPER_H__
#define __AUTOSCROLL_HELPER_H__
#include <view/view.h>
#include <limits.h>
namespace cdroid{
class AbsListView;
class AutoScrollHelper{
public:
    static constexpr float RELATIVE_UNSPECIFIED = 0;
    static constexpr float NO_MAX = 100000.f;//FLT_MAX;//Float.MAX_VALUE;
    static constexpr float NO_MIN = 0;
    static constexpr int EDGE_TYPE_INSIDE = 0;
    static constexpr int EDGE_TYPE_INSIDE_EXTEND = 1;
    static constexpr int EDGE_TYPE_OUTSIDE = 2;
private:
    static constexpr int HORIZONTAL = 0;
    static constexpr int VERTICAL   = 1;
    static constexpr int DEFAULT_EDGE_TYPE = EDGE_TYPE_INSIDE_EXTEND;
    static constexpr int DEFAULT_MINIMUM_VELOCITY_DIPS = 315;
    static constexpr int DEFAULT_MAXIMUM_VELOCITY_DIPS = 1575;
    static constexpr float DEFAULT_MAXIMUM_EDGE = NO_MAX;
    static constexpr float DEFAULT_RELATIVE_EDGE = 0.2f;
    static constexpr float DEFAULT_RELATIVE_VELOCITY = 1.f;
    static constexpr int DEFAULT_ACTIVATION_DELAY = 10;//ViewConfiguration::getTapTimeout();
    static constexpr int DEFAULT_RAMP_UP_DURATION = 500;
    static constexpr int DEFAULT_RAMP_DOWN_DURATION = 500;

    class ClampedScroller{
    private:
        int mRampUpDuration;
        int mRampDownDuration;
        float mTargetVelocityX;
        float mTargetVelocityY;

        int64_t mStartTime;
        int64_t mDeltaTime;
        int mDeltaX;
        int mDeltaY;

        int64_t mStopTime;
        float mStopValue;
        int mEffectiveRampDown;
        float getValueAt(int64_t currentTime);
        float interpolateValue(float value);
    public:
        ClampedScroller();
        void setRampUpDuration(int durationMillis);
        void setRampDownDuration(int durationMillis);
        void start();
        void requestStop();
        bool isFinished();
        void computeScrollDelta();
        void setTargetVelocity(float x, float y);
        int getHorizontalDirection()const;
        int getVerticalDirection()const;
        int getDeltaX()const;
        int getDeltaY()const;
    };
    ClampedScroller* mScroller;
    Interpolator* mEdgeInterpolator;
    protected: View* mTarget;
    Runnable mRunnable;
    float mRelativeEdges[2];
    float mMaximumEdges[2];
    int mEdgeType;
    int mActivationDelay;
    float mRelativeVelocity[2];
    float mMinimumVelocity[2];
    float mMaximumVelocity[2];
    bool mAlreadyDelayed;

    /** Whether to reset the scroller start time on the next animation. */
    bool mNeedsReset;

    /** Whether to send a cancel motion event to the target view. */
    bool mNeedsCancel;

    /** Whether the auto-scroller is actively scrolling. */
    bool mAnimating;

    /** Whether the auto-scroller is enabled. */
    bool mEnabled;

    /** Whether the auto-scroller consumes events when scrolling. */
    bool mExclusive;
private:
    bool shouldAnimate();
    void startAnimating();
    void requestStop();
    float computeTargetVelocity(int direction, float coordinate, float srcSize, float dstSize);
    float getEdgeValue(float relativeValue, float size, float maxValue, float current);
    float constrainEdgeValue(float current, float leading);
    void cancelTargetTouch();
    void animationRun();
public:
    AutoScrollHelper(View*target);
    virtual ~AutoScrollHelper();
    AutoScrollHelper& setEnabled(bool enabled);
    bool isEnabled()const;
    AutoScrollHelper& setExclusive(bool exclusive);
    bool isExclusive()const;
    AutoScrollHelper& setMaximumVelocity(float horizontalMax, float verticalMax);
    AutoScrollHelper& setMinimumVelocity(float horizontalMin, float verticalMin);
    AutoScrollHelper& setRelativeVelocity(float horizontal, float vertical);
    AutoScrollHelper& setEdgeType(int type);
    AutoScrollHelper& setRelativeEdges(float horizontal, float vertical);
    AutoScrollHelper& setMaximumEdges(float horizontalMax, float verticalMax);
    AutoScrollHelper& setActivationDelay(int delayMillis);
    AutoScrollHelper& setRampUpDuration(int durationMillis);
    AutoScrollHelper& setRampDownDuration(int durationMillis);
    bool onTouch(View& v, MotionEvent& event);

    virtual void scrollTargetBy(int deltaX, int deltaY)=0;
    virtual bool canTargetScrollHorizontally(int direction)=0;
    virtual bool canTargetScrollVertically(int direction)=0;
};

class AbsListViewAutoScroller:public AutoScrollHelper{
public:
    AbsListViewAutoScroller(AbsListView* target);
    void scrollTargetBy(int deltaX, int deltaY)override;
    bool canTargetScrollHorizontally(int direction)override;
    bool canTargetScrollVertically(int direction)override;
};

}/*endof namespace*/
#endif/*__AUTOSCROLL_HELPER_H__*/
