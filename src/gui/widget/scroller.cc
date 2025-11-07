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
#include <widget/scroller.h>
#include <core/systemclock.h>
#include <utils/mathutils.h>
#include <view/viewconfiguration.h>
#include <animation/animationutils.h>
#include <cdlog.h>

namespace cdroid{

float Scroller::SPLINE_POSITION [NB_SAMPLES + 1];
float Scroller::SPLINE_TIME [NB_SAMPLES + 1];

const Scroller::ViscousFluidInterpolator Scroller::gViscousFluidInterpolator;

Scroller::Scroller(Context* context):Scroller(context,nullptr,true){
}

Scroller::Scroller(Context* context,const Interpolator* interpolator, bool flywheel) {
    if( (SPLINE_POSITION[NB_SAMPLES]!=1.f) || (SPLINE_TIME[NB_SAMPLES]!=1.f) )
        sInit();
    mMode  = FLING_MODE/*0*/;
    mStartX= mStartY =0;
    mFinalX= mFinalY =0;
    mMinX  = mMinY = 0;
    mMaxX  = mMaxY = 0;
    mCurrX = mCurrY= 0;
    mStartTime= 0;
    mDuration = 1;
    mDistance = 0;
    mFinished = true;
    mCurrVelocity =0.0f;
    mDeltaX= mDeltaY= 0.0f;
    mDurationReciprocal = 1.0f;

    mInterpolator = (interpolator == nullptr) ? &gViscousFluidInterpolator : interpolator;
    mPpi = context->getDisplayMetrics().density * 160.0f;
    mDeceleration = computeDeceleration(ViewConfiguration::getScrollFriction());
    mFlywheel = flywheel;
    mFlingFriction = ViewConfiguration::getScrollFriction();
    mPhysicalCoeff = computeDeceleration(0.84f); // look and feel tuning
}

Scroller::~Scroller(){
}

void Scroller::sInit(){
    float x_min = 0.0f;
    float y_min = 0.0f;
    for (int i = 0; i < NB_SAMPLES; i++) {
        float alpha = (float) i / NB_SAMPLES;

        float x_max = 1.0f;
        float x, coef;
        while (true) {
            x = x_min + (x_max - x_min) / 2.0f;
            coef = 3.0f * x * (1.0f - x);
            float tx = coef * ((1.0f - x) * P1 + x * P2) + x * x * x;
            if (abs(tx - alpha) < 1E-5) break;
            if (tx > alpha) x_max = x;
            else x_min = x;
        }
        SPLINE_POSITION[i] = coef * ((1.0f - x) * START_TENSION + x) + x * x * x;

        float y_max = 1.0f;
        float y;
        while (true) {
            y = y_min + (y_max - y_min) / 2.0f;
            coef = 3.0f * y * (1.0f - y);
            float dy = coef * ((1.0f - y) * START_TENSION + y) + y * y * y;
            if (abs(dy - alpha) < 1E-5) break;
            if (dy > alpha) y_max = y;
            else y_min = y;
        }
        SPLINE_TIME[i] = coef * ((1.0f - y) * P1 + y * P2) + y * y * y;
    }
    SPLINE_POSITION[NB_SAMPLES] = SPLINE_TIME[NB_SAMPLES] = 1.0f;
}

void Scroller::setFriction(float friction) {
    mDeceleration = computeDeceleration(friction);
    mFlingFriction = friction;
}
    
float Scroller::computeDeceleration(float friction) {
     return  9.80665f//SensorManager.GRAVITY_EARTH   // g (m/s^2)
                  * 39.37f               // inch/meter
                  * mPpi                 // pixels per inch
                  * friction;
}

bool Scroller::isFinished()const{
    return mFinished;
}
    
void Scroller::forceFinished(bool finished) {
    mFinished = finished;
}
    
int Scroller::getDuration()const {
    return mDuration;
}
    
int Scroller::getCurrX()const{
    return mCurrX;
}
    
int Scroller::getCurrY()const{
    return mCurrY;
}
    
float Scroller::getCurrVelocity()const {
    return mMode == FLING_MODE ?
            mCurrVelocity : mVelocity - mDeceleration * timePassed() / 2000.0f;
}

int Scroller::getStartX()const{
    return mStartX;
}
    
int Scroller::getStartY()const{
    return mStartY;
}
    
int Scroller::getFinalX()const{
    return mFinalX;
}
    
int Scroller::getFinalY()const{
    return mFinalY;
}

bool Scroller::computeScrollOffset() {
    if (mFinished) return false;

    const int64_t timePassed = (int)(AnimationUtils::currentAnimationTimeMillis() - mStartTime);
    
    if (timePassed < mDuration) {
        int index;
        float x,t,distanceCoef,velocityCoef;
        switch (mMode) {
        case SCROLL_MODE:
            x = mInterpolator->getInterpolation(timePassed * mDurationReciprocal);
            mCurrX = mStartX + round(x * mDeltaX);
            mCurrY = mStartY + round(x * mDeltaY);
            break;
        case FLING_MODE:
            t = (float) timePassed / mDuration;
            index = (int) (NB_SAMPLES * t);
            distanceCoef = 1.f;
            velocityCoef = 0.f;
            if (index < NB_SAMPLES) {
                float t_inf = (float) index / NB_SAMPLES;
                float t_sup = (float) (index + 1) / NB_SAMPLES;
                float d_inf = SPLINE_POSITION[index];
                float d_sup = SPLINE_POSITION[index + 1];
                velocityCoef = (d_sup - d_inf) / (t_sup - t_inf);
                distanceCoef = d_inf + (t - t_inf) * velocityCoef;
            }

            mCurrVelocity = velocityCoef * mDistance / mDuration * 1000.0f;
                
            mCurrX = mStartX + (int)round(distanceCoef * (mFinalX - mStartX));
            // Pin to mMinX <= mCurrX <= mMaxX
            mCurrX = std::min(mCurrX, mMaxX);
            mCurrX = std::max(mCurrX, mMinX);
                
            mCurrY = mStartY + (int)round(distanceCoef * (mFinalY - mStartY));
            //round must converto int ,otherwise willcause NumberPicker::scrollBy enter infinity in its while loop
            // Pin to mMinY <= mCurrY <= mMaxY
            mCurrY = std::min(mCurrY, mMaxY);
            mCurrY = std::max(mCurrY, mMinY);
            if (mCurrX == mFinalX && mCurrY == mFinalY) {
                mFinished = true;
            }
            break;
        }
    }else {
        mCurrX = mFinalX;
        mCurrY = mFinalY;
        mFinished = true;
    }
    return true;
}
    
void Scroller::startScroll(int startX, int startY, int dx, int dy) {
    startScroll(startX, startY, dx, dy, DEFAULT_DURATION);
}

void Scroller::startScroll(int startX, int startY, int dx, int dy, int duration) {
    mMode = SCROLL_MODE;
    mFinished = false;
    mDuration = duration;
    mStartTime = AnimationUtils::currentAnimationTimeMillis();
    mStartX = startX;
    mStartY = startY;
    mFinalX = startX + dx;
    mFinalY = startY + dy;
    mDeltaX = dx;
    mDeltaY = dy;
    mDurationReciprocal = 1.0f / (float) mDuration;
}

    /**
     * Start scrolling based on a fling gesture. The distance travelled will
     * depend on the initial velocity of the fling.
     * 
     * @param startX Starting point of the scroll (X)
     * @param startY Starting point of the scroll (Y)
     * @param velocityX Initial velocity of the fling (X) measured in pixels per
     *        second.
     * @param velocityY Initial velocity of the fling (Y) measured in pixels per
     *        second
     * @param minX Minimum X value. The scroller will not scroll past this
     *        point.
     * @param maxX Maximum X value. The scroller will not scroll past this
     *        point.
     * @param minY Minimum Y value. The scroller will not scroll past this
     *        point.
     * @param maxY Maximum Y value. The scroller will not scroll past this
     *        point.
     */
void Scroller::fling(int startX, int startY, int velocityX, int velocityY,
        int minX, int maxX, int minY, int maxY) {
    // Continue a scroll or fling in progress
    if (mFlywheel && !mFinished) {
        float oldVel = getCurrVelocity();

        float dx = (float) (mFinalX - mStartX);
        float dy = (float) (mFinalY - mStartY);
        float hyp = (float) hypot(dx, dy);

        float ndx = dx / hyp;
        float ndy = dy / hyp;

        float oldVelocityX = ndx * oldVel;
        float oldVelocityY = ndy * oldVel;
        if (MathUtils::signum(velocityX) == MathUtils::signum(oldVelocityX) &&
                MathUtils::signum(velocityY) == MathUtils::signum(oldVelocityY)) {
            velocityX += oldVelocityX;
            velocityY += oldVelocityY;
        }
    }

    mMode = FLING_MODE;
    mFinished = false;

    float velocity = (float) hypot(velocityX, velocityY);
     
    mVelocity = velocity;
    mDuration = getSplineFlingDuration(velocity);
    mStartTime = AnimationUtils::currentAnimationTimeMillis();
    mStartX = startX;
    mStartY = startY;

    float coeffX = velocity == 0 ? 1.0f : velocityX / velocity;
    float coeffY = velocity == 0 ? 1.0f : velocityY / velocity;
    double totalDistance = getSplineFlingDistance(velocity);
    mDistance = (int) (totalDistance * MathUtils::signum(velocity));
     
    mMinX = minX;
    mMaxX = maxX;
    mMinY = minY;
    mMaxY = maxY;
    mFinalX = startX + (int) round(totalDistance * coeffX);
    // Pin to mMinX <= mFinalX <= mMaxX
    mFinalX = std::min(mFinalX, mMaxX);
    mFinalX = std::max(mFinalX, mMinX);
      
    mFinalY = startY + (int) round(totalDistance * coeffY);
    // Pin to mMinY <= mFinalY <= mMaxY
    mFinalY = std::min(mFinalY, mMaxY);
    mFinalY = std::max(mFinalY, mMinY);
}
    
double Scroller::getSplineDeceleration(float velocity) {
    return log(INFLEXION * abs(velocity) / (mFlingFriction * mPhysicalCoeff));
}

int Scroller::getSplineFlingDuration(float velocity) {
    double l = getSplineDeceleration(velocity);
    double decelMinusOne = DECELERATION_RATE - 1.0;
    return (int) (1000.0 * exp(l / decelMinusOne));
}

double Scroller::getSplineFlingDistance(float velocity) {
    double l = getSplineDeceleration(velocity);
    double decelMinusOne = DECELERATION_RATE - 1.0;
    return mFlingFriction * mPhysicalCoeff * exp(DECELERATION_RATE / decelMinusOne * l);
}

void Scroller::abortAnimation() {
    mCurrX = mFinalX;
    mCurrY = mFinalY;
    mFinished = true;
}
    
void Scroller::extendDuration(int extend) {
    long passed = timePassed();
    mDuration = passed + extend;
    mDurationReciprocal = 1.0f / mDuration;
    mFinished = false;
}

/**
 * Returns the time elapsed since the beginning of the scrolling.
 *
 * @return The elapsed time in milliseconds.*/
long Scroller::timePassed()const{
    return (long)(AnimationUtils::currentAnimationTimeMillis()- mStartTime);
}

/**
 * Sets the position (X) for this scroller.
 *
 * @param newX The new X offset as an absolute distance from the origin.
 * @see #extendDuration(int)
 * @see #setFinalY(int) */
void Scroller::setFinalX(int newX) {
    mFinalX = newX;
    mDeltaX = mFinalX - mStartX;
    mFinished = false;
}

/**
 * Sets the position (Y) for this scroller.
 *
 * @param newY The new Y offset as an absolute distance from the origin.
 * @see #extendDuration(int)
 * @see #setFinalX(int) */
void Scroller::setFinalY(int newY) {
    mFinalY = newY;
    mDeltaY = mFinalY - mStartY;
    mFinished = false;
}

bool Scroller::isScrollingInDirection(float xvel, float yvel) {
    return !mFinished && MathUtils::signum(xvel) == MathUtils::signum(mFinalX - mStartX) &&
            MathUtils::signum(yvel) == MathUtils::signum(mFinalY - mStartY);
}

/////////////////////////////////////////////////////////////////////////////////////
float Scroller::ViscousFluidInterpolator::VISCOUS_FLUID_NORMALIZE=1.0f / Scroller::ViscousFluidInterpolator::viscousFluid(1.0f);
float Scroller::ViscousFluidInterpolator::VISCOUS_FLUID_OFFSET = 1.0f - VISCOUS_FLUID_NORMALIZE * Scroller::ViscousFluidInterpolator::viscousFluid(1.0f);

float Scroller::ViscousFluidInterpolator::viscousFluid(float x){
    x *= VISCOUS_FLUID_SCALE;
    if (x < 1.0f) {
        x -= (1.0f - (float)exp(-x));
    } else {
        float start = 0.36787944117f;   // 1/e == exp(-1)
        x = 1.0f - (float)exp(1.0f - x);
        x = start + x * (1.0f - start);
    }
    return x;    
}

float Scroller::ViscousFluidInterpolator::getInterpolation(float input)const{
    float interpolated = VISCOUS_FLUID_NORMALIZE * viscousFluid(input);
    if (interpolated > 0) {
        return interpolated + VISCOUS_FLUID_OFFSET;
    }
    return interpolated;
}
}//end namespace
