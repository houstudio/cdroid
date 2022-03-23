#ifndef __SWIPEHELPER_H__
#define __SWIPEHELPER_H__
#include <view/view.h>

namespace cdroid{

class SwipeHelper{
private:
    constexpr static int MIN_DISTANCE_FOR_FLING=100;
    constexpr static int MIN_FLING_VELOCITY = 400;
    bool mCancel;
    int mEdgeSlop;
    int mScreenWidth;
    int mWindowWidth;

    int mCurrentOrigX;
    int mCurrentStartX;
    int mCurrentEndX;

    int mPrevOrigX;
    int mPrevStartX; 
    int mPrevEndX;

    int mDownX;
    int mActivePointerId;
    int mFlingDistance;
    int mMaximumVelocity;
    int mMinimumVelocity;
    Context*mContext;
    ValueAnimator* mPreContentViewAnimator;
    ValueAnimator* mCurContentViewAnimator;
    VelocityTracker* mVelocityTracker;
    static SwipeHelper*mInst;
private:
    SwipeHelper(Context*ctx);
    View* getCurContentView();
    View* getPreContentView();
public:
    static SwipeHelper&get(Context*ctx);
    bool onTouchEvent(MotionEvent& ev);
    void setSlop(int slop);
    int getSlop()const;
};

}//endof namespace

#endif
