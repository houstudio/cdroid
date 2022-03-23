#include <view/swipehelper.h>
#include <core/systemclock.h>
#include <private/windowmanager.h>
#include <cdlog.h>
namespace cdroid{

SwipeHelper*SwipeHelper::mInst=nullptr;

SwipeHelper::SwipeHelper(Context*ctx) {
    mContext  = ctx;
    mEdgeSlop = 10;
    mVelocityTracker = VelocityTracker::obtain();
    mMaximumVelocity = ViewConfiguration::get(mContext).getScaledMaximumFlingVelocity();
    mMinimumVelocity = mContext->getDisplayMetrics().density*MIN_FLING_VELOCITY;
    mFlingDistance   = mContext->getDisplayMetrics().density*MIN_DISTANCE_FOR_FLING;
    mScreenWidth = mContext->getDisplayMetrics().widthPixels;

    mPreContentViewAnimator = ValueAnimator::ofFloat({.0f, 1.f});
    mPreContentViewAnimator->setInterpolator(new AccelerateDecelerateInterpolator());
    mPreContentViewAnimator->addUpdateListener(ValueAnimator::AnimatorUpdateListener([this](ValueAnimator&anim){
        const float value = anim.getAnimatedValue().get<float>();
        const int xx = mPrevStartX+(mPrevEndX-mPrevStartX)*value;
        Window* prev = (Window*)getPreContentView();
        LOGV("move PrevWindow(%p) to %d cancel=%d",prev,xx,mCancel);
        if ( prev) prev->setPos( xx , prev->getY() );
    }));

    mCurContentViewAnimator = ValueAnimator::ofFloat({.0f, 1.f});
    mCurContentViewAnimator->setInterpolator(new AccelerateDecelerateInterpolator());
    mCurContentViewAnimator->addUpdateListener(ValueAnimator::AnimatorUpdateListener([this](ValueAnimator&anim){
        const float value = anim.getAnimatedValue().get<float>();
        const int xx = mCurrentStartX+(mCurrentEndX-mCurrentStartX)*value;
        Window* cur = (Window*)getCurContentView();
        LOGV("move CurrentWindow(%p) to %d cancel=%d",cur,xx,mCancel);
        if ( cur != nullptr ) {
            cur->setPos( xx , cur->getY() );
            if( (xx == mCurrentEndX) && (mCancel == false)){
                cur->close();
                LOGD("CurrentWindow(%p)destroied",cur);
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

void SwipeHelper::setSlop(int slop){
    mEdgeSlop = slop;
}

int SwipeHelper::getSlop()const{
    return mEdgeSlop;
}

/*we conside that the first window is desktop,we must skip it*/
View* SwipeHelper::getCurContentView() {
    std::vector<Window*>wins;
    WindowManager::getInstance().getVisibleWindows(wins);
    return wins.size()>1?wins.back():nullptr;
}

View*SwipeHelper::getPreContentView() {
    std::vector<Window*>wins;
    WindowManager::getInstance().getVisibleWindows(wins);
    wins.pop_back();
    return wins.size()>1?wins.back():nullptr;
}

bool SwipeHelper::onTouchEvent(MotionEvent& ev) {
    float distance;
    int velocity;
    View* curWindow = nullptr;
    View* prevWindow = nullptr;
    if (mCurContentViewAnimator->isRunning() || mPreContentViewAnimator->isRunning()) {
        return true;
    }

    curWindow  = getCurContentView();
    prevWindow = getPreContentView();
    if (curWindow == nullptr && prevWindow == nullptr) {
        return false;
    }
    mWindowWidth = curWindow->getWidth();
    mVelocityTracker->addMovement(ev);
    switch (ev.getActionMasked()) {
    case MotionEvent::ACTION_DOWN:
        mDownX = ev.getX();
        if ((mDownX > mEdgeSlop)&&(mDownX<mWindowWidth-mEdgeSlop)) return false;

        LOGD("views=%p/%p mDownX=%d mScreenWidth=%d",curWindow,curWindow,mDownX,mScreenWidth);
        mCurrentStartX = curWindow->getX();
        mCurrentOrigX  = mCurrentEndX = mCurrentStartX;
        mActivePointerId = ev.getPointerId(0);
        mCancel = false;
        if(prevWindow){
            mPrevStartX= mPrevEndX  = prevWindow->getX();
            mPrevOrigX = mPrevStartX;
        }
        break;
    case MotionEvent::ACTION_MOVE:
        if ((mDownX > mEdgeSlop)&&(mDownX<mWindowWidth-mEdgeSlop)) return false;

        distance = ev.getX()-mDownX;
        if(mCurrentEndX==mCurrentStartX){
            if(distance>0){
                mCurrentEndX+= mScreenWidth;
                mPrevEndX   = prevWindow?prevWindow->getX():0;
                mPrevStartX = mPrevEndX - mScreenWidth;
            }else{
                mCurrentEndX-=mScreenWidth; 
                mPrevEndX   = prevWindow?prevWindow->getX():0;
                mPrevStartX = mPrevStartX + mScreenWidth;
            }
        }
        velocity = (int) mVelocityTracker->getXVelocity(mActivePointerId);
        if( (abs(velocity)>mMaximumVelocity) && (abs(distance)>mFlingDistance) ) {
            if(prevWindow)
                prevWindow->setPos(mPrevEndX,prevWindow->getY());
            LOGD("destroy curWindow(%p)",curWindow);
            WindowManager::getInstance().removeWindow((Window*)curWindow);
            return true;
        }
        //move Current(Top)Window
        curWindow->setPos(mCurrentStartX+distance,curWindow->getY());
        LOGV("current.x=%.f",mCurrentStartX+distance);
        //move Previous Window
        if(prevWindow) prevWindow->setPos(mPrevStartX+distance,prevWindow->getY());
        break;
    case MotionEvent::ACTION_UP:
    case MotionEvent::ACTION_CANCEL:
        if ((mDownX > mEdgeSlop)&&(mDownX<mWindowWidth-mEdgeSlop)) return false;
        mVelocityTracker->computeCurrentVelocity(1000, mMaximumVelocity);
        velocity = (int) mVelocityTracker->getXVelocity(mActivePointerId);
        distance = ev.getX() - mDownX;
        LOGD("distance=%.f mFlingDistance=%d Velocity %d Range(%d,%d)",distance,
            mFlingDistance,velocity,mMinimumVelocity,mMaximumVelocity);
        mCurrentStartX += distance;
        mPrevStartX += distance;
        if ( abs(distance)>mFlingDistance && std::abs(velocity) > mMinimumVelocity){
            mCurContentViewAnimator->start();
            if(prevWindow)mPreContentViewAnimator->start();
        } else {
            mCancel = true;
            mPrevEndX = mPrevOrigX;
            mCurrentEndX = mCurrentOrigX;
            mCurContentViewAnimator->start();
            LOGD("Cancel Destroy");
            if(prevWindow)mPreContentViewAnimator->start();
        }
        break;
    }
    return true;
}

}//endof namespace
