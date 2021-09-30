#ifndef __TIME_ANIMATOR_H__
#define __TIME_ANIMATOR_H__
#include <animation/valueanimator.h>
namespace cdroid{

class TimeAnimator:public ValueAnimator{
public:
    typedef std::function<void(TimeAnimator&,long totalTime,long detaTime)>TimeListener;
private:
    TimeListener mListener;
    long mPreviousTime;
public:
    TimeAnimator();
    void start();
    bool animateBasedOnTime(long currentTime)override;
    void setCurrentPlayTime(long playTime)override;
    void setTimeListener(TimeListener listener);
    void animateValue(float fraction)override;
    void initAnimation()override;
};

}//endof namespace
#endif
