#ifndef __TIME_ANIMATOR_H__
#define __TIME_ANIMATOR_H__
#include <animation/valueanimator.h>
namespace cdroid{

class TimeAnimator:public ValueAnimator{
public:
    typedef std::function<void(TimeAnimator&,long totalTime,long detaTime)>TimeListener;
private:
    TimeListener mListener;
    int64_t mPreviousTime;
public:
    TimeAnimator();
    void start();
    bool animateBasedOnTime(int64_t currentTime)override;
    void setCurrentPlayTime(int64_t playTime)override;
    void setTimeListener(const TimeListener listener);
    void animateValue(float fraction)override;
    void initAnimation()override;
};

}//endof namespace
#endif
