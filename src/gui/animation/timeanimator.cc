#include <animation/timeanimator.h>
#include <animation/animationutils.h>
namespace cdroid{

TimeAnimator::TimeAnimator(){
    mPreviousTime = -1;
    mListener =nullptr;
}

void TimeAnimator::start(){
    mPreviousTime = -1;
    ValueAnimator::start();
}

bool TimeAnimator::animateBasedOnTime(int64_t currentTime){
    if(mListener != nullptr){
        long totalTime = currentTime - mStartTime;
        long detaTime  = (mPreviousTime<0)?0:(currentTime -mPreviousTime);
        mPreviousTime  = currentTime;
        mListener(*this,totalTime,detaTime);
    }
    return true;
}

void TimeAnimator::setCurrentPlayTime(int64_t playTime){
    const int64_t currentTime=AnimationUtils::currentAnimationTimeMillis();
    mStartTime = std::max(mStartTime,currentTime - playTime);
    animateBasedOnTime(currentTime);
}

void TimeAnimator::setTimeListener(TimeAnimator::TimeListener listener){
    mListener=listener;
}

void TimeAnimator::animateValue(float fraction){
    //NOTHING
}

void TimeAnimator::initAnimation(){
    //NOTHING
}

}//endof namespace
