#include <widget/viewflipper.h>
#include <cdlog.h>

namespace cdroid{

ViewFlipper::ViewFlipper(int w,int h):ViewAnimator(w,h){
    mFlipRunnable=std::bind(&ViewFlipper::doFlip,this);
    mVisible =true;
}

ViewFlipper::ViewFlipper(Context* context,const AttributeSet& attrs)
  :ViewAnimator(context,attrs){
    mFlipRunnable=std::bind(&ViewFlipper::doFlip,this);
}

void ViewFlipper::doFlip(){
    if (mRunning) {
        showNext();
        postDelayed(mFlipRunnable, mFlipInterval);
    }
}

void ViewFlipper::setFlipInterval(int milliseconds) {
    mFlipInterval = milliseconds;
}

void ViewFlipper::startFlipping() {
    mStarted = true;
    updateRunning(true);
}

void ViewFlipper::stopFlipping() {
    mStarted = false;
    updateRunning(true);
}

bool ViewFlipper::isFlipping() const{
    return mStarted;
}

void ViewFlipper::setAutoStart(bool autoStart) {
    mAutoStart = autoStart;
}

bool ViewFlipper::isAutoStart()const{
    return mAutoStart;
}

void ViewFlipper::updateRunning(bool flipNow){
    bool running = mVisible && mStarted && mUserPresent;
    LOGV("running=%d mVisible=%d mStarted=%d mUserPresent=%d mRunning=%d",
         running,mVisible,mStarted,mUserPresent,mRunning);
    if (running != mRunning) {
        if (running) {
            showOnly(mWhichChild, flipNow);
            postDelayed(mFlipRunnable, mFlipInterval);
        } else {
            removeCallbacks(mFlipRunnable);
        }
        mRunning = running;
    }
}

}
