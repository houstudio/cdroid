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
    clampStartingPosition();
}

void RippleForeground::onTargetRadiusChanged(float targetRadius){
    clampStartingPosition();
    //switchToUiThreadAnimation();
}

void RippleForeground::drawSoftware(Canvas& c,float origAlpha) {
    int alpha = (int) (origAlpha * mOpacity + 0.5f);
    float radius = getCurrentRadius();
    if (alpha > 0 && radius > 0) {
        const float x = getCurrentX();
        const float y = getCurrentY();
        c.arc(x, y, radius,0,M_PI*2);
    }
}

void RippleForeground::pruneSwFinished() {
    /*for (auto anim:mRunningSwAnimators.size() - 1; i >= 0; i--) {
        if (!mRunningSwAnimators.get(i).isRunning()) {
            mRunningSwAnimators.remove(i);
        }
    }*/
    //mRunningSwAnimators.remove_if([](const Animator*anim){return !anim->isRunning();});
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
    clampStartingPosition();
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

void RippleForeground::startSoftwareEnter() {
    for (auto anim:mRunningSwAnimators) {
         anim->cancel();
    }
    mRunningSwAnimators.clear();
#if 0
    ObjectAnimator* tweenRadius = ObjectAnimator.ofFloat(this, TWEEN_RADIUS, 1);
    tweenRadius->setDuration(RIPPLE_ENTER_DURATION);
    tweenRadius->setInterpolator(DECELERATE_INTERPOLATOR);
    tweenRadius->start();
    mRunningSwAnimators.push_back(tweenRadius);

    ObjectAnimator* tweenOrigin = ObjectAnimator.ofFloat(this, TWEEN_ORIGIN, 1);
    tweenOrigin->setDuration(RIPPLE_ORIGIN_DURATION);
    tweenOrigin->setInterpolator(DECELERATE_INTERPOLATOR);
    tweenOrigin->start();
    mRunningSwAnimators.push_back(tweenOrigin);

    ObjectAnimator* opacity = ObjectAnimator.ofFloat(this, OPACITY, 1);
    opacity->setDuration(OPACITY_ENTER_DURATION);
    opacity->setInterpolator(LINEAR_INTERPOLATOR);
    opacity->start();
    mRunningSwAnimators.push_back(opacity);
#endif
}

void RippleForeground::startSoftwareExit() {
#if 0
    ObjectAnimator* opacity = ObjectAnimator.ofFloat(this, OPACITY, 0);
    opacity->setDuration(OPACITY_EXIT_DURATION);
    opacity->setInterpolator(LINEAR_INTERPOLATOR);
    opacity->addListener(mAnimationListener);
    opacity->setStartDelay(computeFadeOutDelay());
    opacity->start();
    mRunningSwAnimators.push_back(opacity);
#endif
}

void RippleForeground::enter(){
    mEnterStartedAtMillis = SystemClock::uptimeMillis();
    startSoftwareEnter();
}

void RippleForeground::exit(){
    startSoftwareExit();
}

float RippleForeground::getCurrentX() {
    return lerp(mClampedStartingX - mBounds.centerX(), mTargetX, mTweenX);
}

float RippleForeground::getCurrentY() {
    return lerp(mClampedStartingY - mBounds.centerY(), mTargetY, mTweenY);
}

float RippleForeground::getCurrentRadius() {
    return lerp(mStartRadius, mTargetRadius, mTweenRadius);
}

void RippleForeground::end(){
    for (auto anim:mRunningSwAnimators) {
        anim->end();
    }
    mRunningSwAnimators.clear();
    /*for (auto animhw:mRunningHwAnimators;) {
        animhw->end();
    }
    mRunningHwAnimators.clear();*/
}

void RippleForeground::onAnimationPropertyChanged() {
    if (!mUsingProperties) {
        invalidateSelf();
    }
}

void RippleForeground::draw(Canvas&canvas,float alpha){
    pruneSwFinished();
    drawSoftware(canvas,alpha);
    /*const bool hasDisplayListCanvas = !mForceSoftware && c instanceof DisplayListCanvas;
    if (hasDisplayListCanvas) {
        DisplayListCanvas hw = (DisplayListCanvas) c;
        drawHardware(hw, p);
    } else {
        drawSoftware(c, p);
    }*/
}

void RippleForeground::clampStartingPosition(){
    float cX = mBounds.centerX();
    float cY = mBounds.centerY();
    float dX = mStartingX - cX;
    float dY = mStartingY - cY;
    float r = mTargetRadius - mStartRadius;
    if (dX * dX + dY * dY > r * r) {
        // Point is outside the circle, clamp to the perimeter.
        double angle = std::atan2(dY, dX);
        mClampedStartingX = cX + (float) (std::cos(angle) * r);
        mClampedStartingY = cY + (float) (std::sin(angle) * r);
    } else {
        mClampedStartingX = mStartingX;
        mClampedStartingY = mStartingY;
    }
}

}
