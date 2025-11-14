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
#ifndef __VIEW_DRAG_HELPER_H__
#define __VIEW_DRAG_HELPER_H__
#include <view/viewgroup.h>
#include <widget/overscroller.h>

namespace cdroid{

class ViewDragHelper{
public:
    static constexpr int INVALID_POINTER = -1;
 
    /* A view is not currently being dragged or animating as a result of a fling/snap.*/
    static constexpr int STATE_IDLE = 0;
 
    /* A view is currently being dragged. The position is currently changing as a result
     * of user input or simulated user input.*/
    static constexpr int STATE_DRAGGING = 1;
 
    /* A view is currently settling into place as a result of a fling or predefined non-interactive motion. */
    static constexpr int STATE_SETTLING = 2;
 
    /* Edge flag indicating that the left edge should be affected. */
    static constexpr int EDGE_LEFT = 1 << 0;
 
    /* Edge flag indicating that the right edge should be affected.*/
    static constexpr int EDGE_RIGHT = 1 << 1;
 
    /* Edge flag indicating that the top edge should be affected. */
    static constexpr int EDGE_TOP = 1 << 2;
 
    /* Edge flag indicating that the bottom edge should be affected. */
    static constexpr int EDGE_BOTTOM = 1 << 3;
 
    /* Edge flag set indicating all edges should be affected. */
    static constexpr int EDGE_ALL = EDGE_LEFT | EDGE_TOP | EDGE_RIGHT | EDGE_BOTTOM;
 
    /*Indicates that a check should occur along the horizontal axis */
    static constexpr int DIRECTION_HORIZONTAL = 1 << 0;
 
    /*Indicates that a check should occur along the vertical axis */
    static constexpr int DIRECTION_VERTICAL = 1 << 1;
 
    /* Indicates that a check should occur along all axes*/
    static constexpr int DIRECTION_ALL = DIRECTION_HORIZONTAL | DIRECTION_VERTICAL;

    class Callback{
    public:
        virtual ~Callback()=default;
        virtual void onViewDragStateChanged(int state);
        virtual void onViewPositionChanged(View& changedView, int left, int top,int dx,int dy);
        virtual void onViewCaptured(View& capturedChild, int activePointerId);
        virtual void onViewReleased(View&releasedChild,float xvel,float yvel);
        virtual void onEdgeTouched(int edgeFlags, int pointerId);
        virtual bool onEdgeLock(int edgeFlags);
        virtual void onEdgeDragStarted(int edgeFlags, int pointerId);
        virtual int getOrderedChildIndex(int index);
        virtual int getViewHorizontalDragRange(View& child);
        virtual int getViewVerticalDragRange(View& child);
        virtual bool tryCaptureView(View& child, int pointerId)=0;
        virtual int clampViewPositionHorizontal(View& child, int left, int dx);
        virtual int clampViewPositionVertical(View& child, int top, int dy);
    };
private:
    static constexpr int EDGE_SIZE = 20; // dp

    static constexpr int BASE_SETTLE_DURATION = 256; // ms
    static constexpr int MAX_SETTLE_DURATION = 600; // ms
private:
    // Current drag state; idle, dragging or settling
    int mDragState;

    // Distance to travel before a drag may begin
    int mTouchSlop;

    // Last known position/pointer tracking
    int mActivePointerId = INVALID_POINTER;
    std::vector<float> mInitialMotionX;
    std::vector<float> mInitialMotionY;
    std::vector<float> mLastMotionX;
    std::vector<float> mLastMotionY;
    std::vector<int> mInitialEdgesTouched;
    std::vector<int> mEdgeDragsInProgress;
    std::vector<int> mEdgeDragsLocked;
    int mPointersDown;

    VelocityTracker* mVelocityTracker;
    float mMaxVelocity;
    float mMinVelocity;

    int mEdgeSize;
    int mDefaultEdgeSize;
    int mTrackingEdges;

    OverScroller* mScroller;

    Callback* mCallback;

    View* mCapturedView;
    bool mReleaseInProgress;

    ViewGroup* mParentView;
    Interpolator*mInterpolator;
    Runnable mSetIdleRunnable;
private:
    ViewDragHelper(Context* context,ViewGroup* forParent,Callback* cb);
    bool forceSettleCapturedViewAt(int finalLeft, int finalTop, int xvel, int yvel);
    bool forceSettleCapturedViewAt(int finalLeft, int finalTop, int duration, Interpolator* interpolator);
    int computeSettleDuration(View* child, int dx, int dy, int xvel, int yvel);
    int computeAxisDuration(int delta, int velocity, int motionRange);
    int clampMag(int value, int absMin, int absMax);
    float clampMag(float value, float absMin, float absMax);
    float distanceInfluenceForSnapDuration(float f);
    void dispatchViewReleased(float xvel, float yvel);
    void clearMotionHistory();
    void clearMotionHistory(int pointerId);
    void ensureMotionHistorySizeForId(int pointerId);
    void saveInitialMotion(float x, float y, int pointerId);
    void saveLastMotion(MotionEvent& ev);
    void reportNewEdgeDrags(float dx, float dy, int pointerId);
    bool checkNewEdgeDrag(float delta, float odelta, int pointerId, int edge);
    bool checkTouchSlop(View* child, float dx, float dy);
    void releaseViewForPointerUp();
    void dragTo(int left, int top, int dx, int dy);
    int getEdgesTouched(int x, int y)const;
    bool isValidPointerForActionMove(int pointerId)const;
protected:
    bool canScroll(View* v, bool checkV, int dx, int dy, int x, int y);
public:
    static ViewDragHelper* create(ViewGroup*,Callback* cb);
    static ViewDragHelper* create(ViewGroup* forParent, float sensitivity,Callback* cb);
    virtual ~ViewDragHelper();
    void setMinVelocity(float minVel);
    float getMinVelocity()const;
    int getViewDragState()const;
    void setEdgeTrackingEnabled(int edgeFlags);
    int getEdgeSize()const;
    void setEdgeSize(int);
    int getDefaultEdgeSize()const;
    void captureChildView(View* childView, int activePointerId);
    View* getCapturedView()const;
    int getActivePointerId()const;
    int getTouchSlop()const;
    void cancel();
    void abort();
    bool smoothSlideViewTo(View* child, int finalLeft, int finalTop);
    bool smoothSlideViewTo(View* child, int finalLeft, int finalTop, int duration, Interpolator* interpolator);
    bool settleCapturedViewAt(int finalLeft, int finalTop);
    void flingCapturedView(int minLeft, int minTop, int maxLeft, int maxTop);
    bool continueSettling(bool deferCallbacks);
    bool isPointerDown(int pointerId)const;
    void setDragState(int state);//protected?
    bool tryCaptureViewForDrag(View* toCapture, int pointerId);//protected?
    bool shouldInterceptTouchEvent(MotionEvent& ev);
    void processTouchEvent(MotionEvent& ev);
    bool checkTouchSlop(int directions);
    bool checkTouchSlop(int directions, int pointerId);
    bool isEdgeTouched(int edges)const;
    bool isEdgeTouched(int edges, int pointerId)const;
    bool isCapturedViewUnder(int x, int y)const;
    bool isViewUnder(View* view, int x, int y)const;
    View* findTopChildUnder(int x, int y);
};

}//endof namespace
#endif//__VIEW_DRAG_HELPER_H__
