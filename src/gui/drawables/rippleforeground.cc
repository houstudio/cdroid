#include <drawables/rippleforeground.h>
#include <core/systemclock.h>
namespace cdroid{

RippleForeground::RippleForeground(RippleDrawable* owner,const Rect& bounds, float startingX, float startingY,
            bool forceSoftware):RippleComponent(owner, bounds){

    mForceSoftware = forceSoftware;
    mStartingX = startingX;
    mStartingY = startingY;

    // Take 60% of the maximum of the width and height, then divided half to get the radius.
    mStartRadius = std::max(bounds.width, bounds.height) * 0.3f;
    //clampStartingPosition();
}

void RippleForeground::onTargetRadiusChanged(float targetRadius){
    //clampStartingPosition();
    //switchToUiThreadAnimation();
}

void RippleForeground::getBounds(Rect& bounds) {
    int outerX = (int) mTargetX;
    int outerY = (int) mTargetY;
    int r = (int) mTargetRadius + 1;
    bounds.set(outerX - r, outerY - r, r+r,r + r);
}

void RippleForeground::move(float x, float y) {
    mStartingX = x;
    mStartingY = y;
    //clampStartingPosition();
}

bool RippleForeground::hasFinishedExit()const{
    return mHasFinishedExit;
}

long RippleForeground::computeFadeOutDelay() {
    long timeSinceEnter = SystemClock::uptimeMillis() - mEnterStartedAtMillis;
    if (timeSinceEnter > 0 && timeSinceEnter < OPACITY_HOLD_DURATION) {
        return OPACITY_HOLD_DURATION - timeSinceEnter;
    }
    return 0;
}


void RippleForeground::enter(){
}

void RippleForeground::exit(){
}

void RippleForeground::end(){
}

}
