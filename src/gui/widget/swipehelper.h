#ifndef __SWIPEHELPER_H__
#define __SWIPEHELPER_H__
#include <widget/view.h>

namespace cdroid{

class SwipeHelper{
private:
    constexpr static int START_SLIDE_X = 30;
    float mDownX;
    float mMoveX;
    float mMaxMoveX;
    long  mDownTime;
    int   mScreenWidth;
    float mPreContentViewX;
    Context*mContext;
    ValueAnimator* mPreContentViewAnimator;
    ValueAnimator* mCurContentViewAnimator;
    static SwipeHelper*mInst;
private:
    SwipeHelper(Context*ctx);
    View* getCurContentView();
    View* getPreContentView();
    void animatePreContentView(float start, float end);
    void animateCurContentView(float start, float end);
public:
    SwipeHelper&get(Context*ctx);
    bool processTouchEvent(MotionEvent& ev);
};

}//endof namespace

#endif
