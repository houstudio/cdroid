#ifndef __VIEWFLIPPER_H__
#define __VIEWFLIPPER_H__
#include <widget/viewanimator.h>

namespace cdroid{

class ViewFlipper:public ViewAnimator{
private:
    static constexpr int DEFAULT_INTERVAL = 3000;
    int mFlipInterval = DEFAULT_INTERVAL;
    bool mAutoStart = false;

    bool mRunning = false;
    bool mStarted = false;
    bool mVisible = false;
    bool mUserPresent = true;
    Runnable mFlipRunnable;
private:
    void updateRunning(bool flipNow);
    void doFlip();
public:
    ViewFlipper(int w,int h);
    ViewFlipper(Context* context,const AttributeSet& attrs);
    void setFlipInterval(int milliseconds);
    void startFlipping();
    void stopFlipping();
    bool isFlipping()const;
    void setAutoStart(bool autoStart);
    bool isAutoStart()const;
};
}//endof namespace

#endif
