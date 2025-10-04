/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
#include <widget/edgeeffect.h>
#include <core/systemclock.h>
#include <utils/mathutils.h>
#include <cdtypes.h>
#include <cdlog.h>
#include <animation/valueanimator.h>
#include <animation/animationutils.h>
#include <cmath>

namespace cdroid{

constexpr float EdgeEffect::MAX_ALPHA ;
constexpr float EdgeEffect::PULL_GLOW_BEGIN;
constexpr int EdgeEffect::MIN_VELOCITY;
constexpr int EdgeEffect::MAX_VELOCITY;

EdgeEffect::EdgeEffect(Context* context){
    mInterpolator = new DecelerateInterpolator();
    mDisplacement = 0.5f;
    mBounds.set(0,0,0,0);
    mPullDistance = 0;
    mState = STATE_IDLE;
    mGlowAlpha = 0;
    mGlowAlphaStart = mGlowAlphaFinish = 0.f;
    mBaseGlowScale = 0;
    mGlowScaleY = 0;
    mGlowScaleYStart = mGlowScaleYFinish = 0.f;
    mStartTime= 0;
    mColor = 0x4000FF00;
    mEdgeEffectType = TYPE_GLOW;
    mDuration = PULL_DECAY_TIME;
    mDistance = 0;
    mVelocity = 0.f;
}

EdgeEffect::EdgeEffect(Context* context,const AttributeSet& attrs)
    :EdgeEffect(context){
    mColor = attrs.getColor("colorEdgeEffect", 0xff666666);
}

EdgeEffect::~EdgeEffect(){
    delete mInterpolator;
}

void EdgeEffect::setSize(int width, int height){
    float r = width * RADIUS_FACTOR / SIN;
    float y = COS * r;
    float h = r - y;
    float oR = height * RADIUS_FACTOR / SIN;
    float oy = COS * oR;
    float oh = oR - oy;
    mWidth  = width;
    mHeight = height;
    mRadius = r;
    mBaseGlowScale = h > 0 ? std::min(oh / h, 1.f) : 1.f;
    mBounds.set(mBounds.left, mBounds.top, width, (int) std::min((float)height, h));
}

int EdgeEffect::getCurrentEdgeEffectBehavior() {
    if (!ValueAnimator::areAnimatorsEnabled()) {
        return TYPE_NONE;
    } else {
        return mEdgeEffectType;
    }
}

bool EdgeEffect::isFinished()const{
    return mState == STATE_IDLE;
}

void EdgeEffect::finish(){
    mState = STATE_IDLE;
    mDistance = 0;
    mVelocity = 0;
}

void EdgeEffect::onPull(float deltaDistance){
    onPull(deltaDistance, 0.5f);
}

void EdgeEffect::onPull(float deltaDistance, float displacement){
    int edgeEffectBehavior = getCurrentEdgeEffectBehavior();
    if (edgeEffectBehavior == TYPE_NONE) {
        finish();
        return;
    }
    const int64_t now = SystemClock::uptimeMillis();
    mTargetDisplacement = displacement;
    if (mState == STATE_PULL_DECAY && now - mStartTime < mDuration && edgeEffectBehavior == TYPE_GLOW) {
        return;
    }
    if (mState != STATE_PULL) {
        if (edgeEffectBehavior == TYPE_STRETCH)
            mPullDistance = mDistance;
        else
            mGlowScaleY = std::max(PULL_GLOW_BEGIN, mGlowScaleY);
    }
    mState = STATE_PULL;

    mStartTime = now;
    mDuration = PULL_TIME;

    mPullDistance += deltaDistance;
    if (edgeEffectBehavior == TYPE_STRETCH) {
        // Don't allow stretch beyond 1
        mPullDistance = std::min(1.f, mPullDistance);
    }
    mDistance = std::max(.0f, mPullDistance);
    mVelocity = 0;

    if (mPullDistance == 0) {
        mGlowScaleY = mGlowScaleYStart = 0;
        mGlowAlpha = mGlowAlphaStart = 0;
    } else {

        float absdd = std::abs(deltaDistance);
        mGlowAlpha = mGlowAlphaStart = std::min(MAX_ALPHA,
                mGlowAlpha + (absdd * PULL_DISTANCE_ALPHA_GLOW_FACTOR));
        float scale = (float) (std::max(0.f, 1.f - 1.f /
               (float)std::sqrt(std::abs(mPullDistance) * mBounds.height) - 0.3f) / 0.7f);

        mGlowScaleY = mGlowScaleYStart = scale;
    }

    mGlowAlphaFinish = mGlowAlpha;
    mGlowScaleYFinish = mGlowScaleY;
    if((edgeEffectBehavior ==TYPE_STRETCH) && (mDistance ==0.f)){
        mState = STATE_IDLE;
    }
}

float EdgeEffect::onPullDistance(float deltaDistance, float displacement) {
    int edgeEffectBehavior = getCurrentEdgeEffectBehavior();
    if (edgeEffectBehavior == TYPE_NONE) {
        return 0.f;
    }
    float finalDistance = std::max(.0f, deltaDistance + mDistance);
    float delta = finalDistance - mDistance;
    if (delta == 0.f && mDistance == 0.f) {
        return 0.f; // No pull, don't do anything.
    }

    if (mState != STATE_PULL && mState != STATE_PULL_DECAY && edgeEffectBehavior == TYPE_GLOW) {
        // Catch the edge glow in the middle of an animation.
        mPullDistance = mDistance;
        mState = STATE_PULL;
    }
    onPull(delta, displacement);
    return delta;
}

float EdgeEffect::getDistance()const{
    return mDistance;
}

void EdgeEffect::onRelease(){
    mPullDistance = 0;

    if (mState != STATE_PULL && mState != STATE_PULL_DECAY) {
        return;
    }

    mState = STATE_RECEDE;
    mGlowAlphaStart = mGlowAlpha;
    mGlowScaleYStart = mGlowScaleY;

    mGlowAlphaFinish = 0.f;
    mGlowScaleYFinish = 0.f;
    mVelocity = 0.f;

    mStartTime = SystemClock::uptimeMillis();
    mDuration = RECEDE_TIME;
}

void EdgeEffect::onAbsorb(int velocity){
    int edgeEffectBehavior = getCurrentEdgeEffectBehavior();
    if (edgeEffectBehavior == TYPE_STRETCH) {
        mState = STATE_RECEDE;
        mVelocity = velocity * ON_ABSORB_VELOCITY_ADJUSTMENT;
        mStartTime = AnimationUtils::currentAnimationTimeMillis();
    } else if (edgeEffectBehavior == TYPE_GLOW){
        mState = STATE_ABSORB;
        mVelocity= 0;
        velocity = std::min(std::max(MIN_VELOCITY, std::abs(velocity)), MAX_VELOCITY);

        mStartTime = SystemClock::uptimeMillis();
        mDuration = 0.15f + (velocity * 0.02f);

        // The glow depends more on the velocity, and therefore starts out
        // nearly invisible.
         mGlowAlphaStart = GLOW_ALPHA_START;
        mGlowScaleYStart = std::max(mGlowScaleY, 0.f);


        // Growth for the size of the glow should be quadratic to properly
        // respond to a user's scrolling speed. The faster the scrolling speed, the more
        // intense the effect should be for both the size and the saturation.
        mGlowScaleYFinish = std::min(0.025f + (velocity * (velocity / 100) * 0.00015f) / 2, 1.f);
        // Alpha should change for the glow as well as size.
        mGlowAlphaFinish = std::max(
            mGlowAlphaStart, std::min(velocity * VELOCITY_GLOW_FACTOR * 0.00001f, MAX_ALPHA));
        mTargetDisplacement = 0.5f;
    }else {
        finish();
    }
}

void EdgeEffect::setColor(int color){
    mColor=color;
}

int EdgeEffect::getColor()const{
    return mColor;
}

bool EdgeEffect::draw(Canvas& canvas){
    const int edgeEffectBehavior = getCurrentEdgeEffectBehavior();
    if (edgeEffectBehavior == TYPE_GLOW){
        update();
        const float centerX = mBounds.centerX();
        const float centerY = mBounds.height - mRadius;
        canvas.save();

        const float displacement = std::max(0.f, std::min(mDisplacement, 1.f)) - 0.5f;
        const float translateX = mBounds.width * displacement / 2;
   
        canvas.rectangle(mBounds);
        canvas.clip();
        canvas.set_color(mColor);
        canvas.curve_to(mBounds.left,mBounds.top,mBounds.width/2+translateX,mBounds.height*mGlowScaleY,mBounds.width,0);
        canvas.fill();

        canvas.restore();
    }else if(edgeEffectBehavior == TYPE_STRETCH /*&& canvas instanceof RecordingCanvas*/){
    }else{
        mState = STATE_IDLE;
        mDistance = 0;
        mVelocity = 0;
    } 
    bool oneLastFrame = false;
    if ((mState == STATE_RECEDE) && (mDistance ==0.f) && (mVelocity == 0.f)) {
        mState = STATE_IDLE;
        oneLastFrame = true;
    }
    return (mState != STATE_IDLE) || oneLastFrame;
}

int EdgeEffect::getMaxHeight()const{
   return (int) (mBounds.height * MAX_GLOW_SCALE + 0.5f);
}

void EdgeEffect::update() {
    const auto time = SystemClock::uptimeMillis();
    float t = std::min((time - mStartTime) / mDuration, 1.f);

    float interp = mInterpolator->getInterpolation(t);

    mGlowAlpha = mGlowAlphaStart + (mGlowAlphaFinish - mGlowAlphaStart) * interp;
    mGlowScaleY = mGlowScaleYStart + (mGlowScaleYFinish - mGlowScaleYStart) * interp;
    if (mState != STATE_PULL) {
        mDistance = calculateDistanceFromGlowValues(mGlowScaleY, mGlowAlpha);
    }
    mDisplacement = (mDisplacement + mTargetDisplacement) / 2;

    if (t >= 1.f - EPSILON) {
        switch (mState) {
        case STATE_ABSORB:
            mState = STATE_RECEDE;
            mStartTime = SystemClock::uptimeMillis();
            mDuration = RECEDE_TIME;

            mGlowAlphaStart = mGlowAlpha;
            mGlowScaleYStart = mGlowScaleY;

            // After absorb, the glow should fade to nothing.
            mGlowAlphaFinish = 0.f;
            mGlowScaleYFinish = 0.f;
            break;
        case STATE_PULL:
            mState = STATE_PULL_DECAY;
            mStartTime = SystemClock::uptimeMillis();
            mDuration = PULL_DECAY_TIME;

            mGlowAlphaStart = mGlowAlpha;
            mGlowScaleYStart = mGlowScaleY;

            // After pull, the glow should fade to nothing.
            mGlowAlphaFinish = 0.f;
            mGlowScaleYFinish = 0.f;
            break;
        case STATE_PULL_DECAY:
            mState = STATE_RECEDE;
            break;
        case STATE_RECEDE:
            mState = STATE_IDLE;
            break;
        }
    }
}

void EdgeEffect::updateSpring() {
    int64_t time = AnimationUtils::currentAnimationTimeMillis();
    float deltaT = (time - mStartTime) / 1000.f; // Convert from millis to seconds
    if (deltaT < 0.001f) {
        return; // Must have at least 1 ms difference
    }
    mStartTime = time;

    if (abs(mVelocity) <= LINEAR_VELOCITY_TAKE_OVER
                && abs(mDistance * mHeight) < LINEAR_DISTANCE_TAKE_OVER
                && MathUtils::signum(mVelocity) == -MathUtils::signum(mDistance)
    ) {
        // This is close. The spring will slowly reach the destination. Instead, we
        // will interpolate linearly so that it arrives at its destination quicker.
        mVelocity = MathUtils::signum(mVelocity) * LINEAR_VELOCITY_TAKE_OVER;

        float targetDistance = mDistance + (mVelocity * deltaT / mHeight);
        if (MathUtils::signum(targetDistance) != MathUtils::signum(mDistance)) {
            // We have arrived
            mDistance = 0;
            mVelocity = 0;
        } else {
            mDistance = targetDistance;
        }
        return;
    }
    double mDampedFreq = NATURAL_FREQUENCY * sqrt(1 - DAMPING_RATIO * DAMPING_RATIO);

    // We're always underdamped, so we can use only those equations:
    double cosCoeff = mDistance * mHeight;
    double sinCoeff = (1 / mDampedFreq) * (DAMPING_RATIO * NATURAL_FREQUENCY
            * mDistance * mHeight + mVelocity);
    double distance = pow(M_E, -DAMPING_RATIO * NATURAL_FREQUENCY * deltaT)
            * (cosCoeff * cos(mDampedFreq * deltaT)
            + sinCoeff * sin(mDampedFreq * deltaT));
    double velocity = distance * (-NATURAL_FREQUENCY) * DAMPING_RATIO
            + pow(M_E, -DAMPING_RATIO * NATURAL_FREQUENCY * deltaT)
            * (-mDampedFreq * cosCoeff * sin(mDampedFreq * deltaT)
                + mDampedFreq * sinCoeff * cos(mDampedFreq * deltaT));
    mDistance = (float) distance / mHeight;
    mVelocity = (float) velocity;
    if (mDistance > 1.f) {
        mDistance = 1.f;
        mVelocity = .0f;
    }
    if (isAtEquilibrium()) {
        mDistance = 0;
        mVelocity = 0;
    }
}

float EdgeEffect::calculateDistanceFromGlowValues(float scale, float alpha) {
    if (scale >= 1.f) {
        // It should asymptotically approach 1, but not reach there.
        // Here, we're just choosing a value that is large.
        return 1.f;
    }
    if (scale > .0f) {
        float v = 1.f / 0.7f / (mGlowScaleY - 1.f);
        return v * v / mBounds.height;
    }
    return alpha / PULL_DISTANCE_ALPHA_GLOW_FACTOR;
}

bool EdgeEffect::isAtEquilibrium()const{
    double displacement = mDistance * mHeight; // in pixels
    double velocity = mVelocity;

    // Don't allow displacement to drop below 0. We don't want it stretching the opposite
    // direction if it is flung that way. We also want to stop the animation as soon as
    // it gets very close to its destination.
    return displacement < 0 || (abs(velocity) < VELOCITY_THRESHOLD
            && displacement < VALUE_THRESHOLD);
}

float EdgeEffect::dampStretchVector(float normalizedVec)const{
    float sign = normalizedVec > .0f ? 1.f : -1.f;
    float overscroll = abs(normalizedVec);
    float linearIntensity = LINEAR_STRETCH_INTENSITY * overscroll;
    double scalar = M_E / SCROLL_DIST_AFFECTED_BY_EXP_STRETCH;
    double expIntensity = EXP_STRETCH_INTENSITY * (1.f - exp(-overscroll * scalar));
    return sign * (float) (linearIntensity + expIntensity);
}

}//namespace
