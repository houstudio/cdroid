#include <widget/swipehelper.h>
#include <core/systemclock.h>
#include <private/windowmanager.h>
namespace cdroid{

SwipeHelper*SwipeHelper::mInst=nullptr;

SwipeHelper::SwipeHelper(Context*ctx) {
    mContext = ctx;
    mPreContentViewAnimator = ValueAnimator::ofFloat({0.f, .0f});
    mPreContentViewAnimator->setInterpolator(new AccelerateDecelerateInterpolator());
    mPreContentViewAnimator->addUpdateListener(ValueAnimator::AnimatorUpdateListener([this](ValueAnimator&anim){
        float value = anim.getAnimatedValue().get<float>();
        View* preContentView = getPreContentView();
        if ( preContentView != nullptr) {
            if (value <= mPreContentViewX) {//说明滑动结束，恢复0值
            }
            preContentView->setX(value);
        }
    }));

    mCurContentViewAnimator = ValueAnimator::ofFloat({0.f, 0.f});
    mCurContentViewAnimator->setInterpolator(new AccelerateDecelerateInterpolator());
    mCurContentViewAnimator->addUpdateListener(ValueAnimator::AnimatorUpdateListener([this](ValueAnimator&anim){
        float value = anim.getAnimatedValue().get<float>();
        View* curContentView = getCurContentView();
        if ( curContentView != nullptr) {
            curContentView->setX(value);
            if (value >= mScreenWidth) {
                WindowManager::getInstance().removeWindow((Window*)curContentView);
            }
        }
    }));
}

SwipeHelper& SwipeHelper::get(Context*ctx) {
    if (mInst == nullptr) {
        mInst = new SwipeHelper(ctx);
    }
    return *mInst;
}


View* SwipeHelper::getCurContentView() {
    std::vector<Window*>wins;
    WindowManager::getInstance().getVisibleWindows(wins);
    return wins.back();
}

View*SwipeHelper::getPreContentView() {
    std::vector<Window*>wins;
    WindowManager::getInstance().getVisibleWindows(wins);
    wins.pop_back();
    return wins.size()?wins.back():nullptr;
}

bool SwipeHelper::processTouchEvent(MotionEvent& ev) {
    float upX,distance,preDistance;
    View* curContentView = nullptr;
    View* preContentView = nullptr;
    if (mCurContentViewAnimator->isRunning() || mPreContentViewAnimator->isRunning()) {
        return true;
    }

    curContentView = getCurContentView();
    preContentView = getPreContentView();
    if (curContentView == nullptr && preContentView == nullptr) {
        return false;
    }

    if (mScreenWidth <= 0) {
        mScreenWidth = mContext->getDisplayMetrics().widthPixels;
        mPreContentViewX = -mScreenWidth * 0.3f;
    }

    switch (ev.getActionMasked()) {
    case MotionEvent::ACTION_DOWN:
        mDownX = ev.getX();
        if (mDownX > START_SLIDE_X) {
            return false;//按下边缘位置才允许滑动
        }
        mDownTime = SystemClock::currentTimeMillis();
        preContentView->setX(mPreContentViewX);
        break;
    case MotionEvent::ACTION_MOVE:
        if (mDownX > START_SLIDE_X) {
            return false;//按下边缘位置才允许滑动
        }
        mMoveX = ev.getX();
        if (mMoveX > mMaxMoveX) mMaxMoveX = mMoveX;
        if (mMoveX > (mScreenWidth - START_SLIDE_X / 2)) {//滑到最右端
            preContentView->setX(0);
            WindowManager::getInstance().removeWindow((Window*)preContentView);
            return true;
        }
        //move Current
        distance = mMoveX - mDownX;
        if (distance < 0) {
            distance = 0;
        }
        curContentView->setX(distance);

        //move Prev
        preDistance = mPreContentViewX + distance / 3;
        if (preDistance > 0) {
            preDistance = 0;
        }
        if(preContentView)
            preContentView->setX(preDistance);
        break;
    case MotionEvent::ACTION_UP:
    case MotionEvent::ACTION_CANCEL:
        if (mDownX > START_SLIDE_X) return false;//按下边缘位置才允许滑动
        upX = ev.getX();
        if (upX < mMaxMoveX && (mMaxMoveX - upX) / (SystemClock::currentTimeMillis() - mDownTime) > 0.1) {//快速向左滑动
            animateCurContentView(curContentView->getX(), 0);
            if(preContentView)
                animatePreContentView(preContentView->getX(), mPreContentViewX);
        } else if ((upX - mDownX) / (SystemClock::currentTimeMillis() - mDownTime) > 0.5//快速向右滑动
                  || upX > mScreenWidth / 2) {//大于屏幕一半就结束当前Activity，根据自己需求而定吧
            animateCurContentView(curContentView->getX(), mScreenWidth);
            animatePreContentView(preContentView->getX(), 0);
        } else {
            animateCurContentView(curContentView->getX(), 0);
            if(preContentView)
                animatePreContentView(preContentView->getX(), mPreContentViewX);
        }
        mMaxMoveX = 0;
        break;
    }
    return true;
}

void SwipeHelper::animatePreContentView(float start, float end) {
    mPreContentViewAnimator->setFloatValues({start, end});
    mPreContentViewAnimator->start();
}

void SwipeHelper::animateCurContentView(float start, float end) {
    mCurContentViewAnimator->setFloatValues({start, end});
    mCurContentViewAnimator->start();
}

}//endof namespace
