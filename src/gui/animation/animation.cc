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
#include <animation/animation.h>
#include <animation/animationutils.h>
#include <systemclock.h>
#include <limits>
#include <cdtypes.h>
#include <cdlog.h>

using namespace Cairo;
namespace cdroid{

Animation::Animation() {
    mStartTime    =-1;
    mStartOffset  =0;
    mInterpolator =nullptr;
    mStarted  = false;
    mEnded    = false;
    mCycleFlip=false;
    mInitialized= false;
    mFillBefore = true;
    mFillAfter  = false;
    mFillEnabled= false;
    mRepeatMode = RESTART;
    mMore = mOneMoreTime = true;
    ensureInterpolator();
}

Animation::Animation(const Animation&o){
    mStartTime  = -1;//o.mStartTime;
    mStartOffset= 0;//o.mStartOffset;
    mDuration   = o.mDuration;
    mRepeatCount= o.mRepeatCount;
    mRepeatMode = o.mRepeatMode;
    mCycleFlip  = o.mCycleFlip;
    mFillEnabled= o.mFillEnabled;
    mFillBefore = o.mFillBefore;
    mFillAfter  = o.mFillAfter;
    mZAdjustment= o.mZAdjustment;
    mMore       = o.mMore;
    mOneMoreTime= o.mOneMoreTime;
    mRepeated   = 0;
    mStarted = mEnded = false;
    mBackgroundColor = o.mBackgroundColor;
    mInterpolator=nullptr;
    ensureInterpolator(); 
}

Animation::Animation(Context* context, const AttributeSet& attrs){
    setDuration(attrs.getInt("duration",0));
    setStartOffset(attrs.getInt("startOffset",0));
    setFillEnabled(attrs.getBoolean("fillEnabled",mFillEnabled));
    setFillBefore (attrs.getBoolean("fillBefore",mFillBefore));
    setFillAfter  (attrs.getBoolean("fillAfter",mFillAfter));
    setRepeatCount(attrs.getInt("repeatCount",mRepeatCount));
    setRepeatMode (attrs.getInt("repeatMode",RESTART));
    //setBackgroundColor(Color::parseColor(attrs.getString("background")));
    const std::string resid=attrs.getString("interpolator");
    if(!resid.empty())setInterpolator(context,resid);else mInterpolator=nullptr;
}

Animation::~Animation(){
}

Animation::Description Animation::Description::parseValue(const std::string&v){
    Description d;
    const float ret=std::strtof(v.c_str(),nullptr);
    if(v.find("%p")!=std::string::npos)d.type = RELATIVE_TO_PARENT;
    else if(v.find("%")!=std::string::npos)d.type= RELATIVE_TO_SELF;
    else d.type = ABSOLUTE;
    d.value =  (d.type==ABSOLUTE)?ret:(ret/100.f);
    return d;
}

float Animation::getPivotType(const std::string&v,int &type){
    const float ret=std::strtof(v.c_str(),nullptr);
    if(v.find("%p")!=std::string::npos)type = RELATIVE_TO_PARENT;
    else if(v.find("%")!=std::string::npos)type= RELATIVE_TO_SELF;
    else type = ABSOLUTE;
    return (type==ABSOLUTE)?ret:(ret/100.f);
}

Animation* Animation::clone()const{
    Animation* animation = new Animation(*this);
    //animation->mPreviousRegion = new RectF();
    //animation->mRegion = new RectF();
    //animation->mTransformation = new Transformation();
    //animation->mPreviousTransformation = new Transformation();
    return animation;
}

void Animation::reset() {
    mPreviousRegion.setEmpty();
    mPreviousTransformation.clear();
    mInitialized = false;
    mCycleFlip = false;
    mRepeated = 0;
    mMore = true;
    mOneMoreTime = true;
    mListenerHandler = nullptr;
}

void Animation::cancel() {
    if (mStarted && !mEnded) {
        fireAnimationEnd();
        mEnded = true;
        //guard.close();
    }
    // Make sure we move the animation to the end
    mStartTime = LLONG_MIN;
    mMore = mOneMoreTime = false;
}

void Animation::detach() {
    if (mStarted && !mEnded) {
        mEnded = true;
        //guard.close();
        fireAnimationEnd();
    }
}

bool Animation::isInitialized()const{
    return mInitialized;
}

void Animation::initialize(int width, int height, int parentWidth, int parentHeight) {
    reset();
    mInitialized = true;
}

//void Animation::setListenerHandler(Handler handler){}

void Animation::setInterpolator(Context* context,const std::string&resID) {
    setInterpolator(AnimationUtils::loadInterpolator(context, resID));
}

void Animation::setInterpolator(const Interpolator* i) {
    mInterpolator = i;
}

void Animation::setStartOffset(int64_t startOffset) {
    mStartOffset = startOffset;
}

void Animation::setDuration(int64_t durationMillis) {
    if (durationMillis < 0) {
        throw std::runtime_error("Animation duration cannot be negative");
    }
    mDuration = durationMillis;
}

void Animation::restrictDuration(int64_t durationMillis) {
    // If we start after the duration, then we just won't run.
    if (mStartOffset > durationMillis) {
        mStartOffset = durationMillis;
        mDuration = 0;
        mRepeatCount = 0;
        return;
    }

    int64_t dur = mDuration + mStartOffset;
    if (dur > durationMillis) {
        mDuration = durationMillis-mStartOffset;
        dur = durationMillis;
    }
    // If the duration is 0 or less, then we won't run.
    if (mDuration <= 0) {
        mDuration = 0;
        mRepeatCount = 0;
        return;
    }
    // Reduce the number of repeats to keep below the maximum duration.
    // The comparison between mRepeatCount and duration is to catch
    // overflows after multiplying them.
    if (mRepeatCount < 0 || mRepeatCount > durationMillis
            || (dur*mRepeatCount) > durationMillis) {
        // Figure out how many times to do the animation.  Subtract 1 since
        // repeat count is the number of times to repeat so 0 runs once.
        mRepeatCount = (int)(durationMillis/dur) - 1;
        if (mRepeatCount < 0) mRepeatCount = 0;
    }
}

void Animation::scaleCurrentDuration(float scale) {
    mDuration = int64_t(mDuration * scale);
    mStartOffset = int64_t(mStartOffset * scale);
}

void Animation::setStartTime(int64_t startTimeMillis) {
    mStartTime = startTimeMillis;
    mStarted   = mEnded = false;
    mCycleFlip = false;
    mRepeated  = 0;
    mMore = true;
}

void Animation::start() {
    setStartTime(-1);
}

void Animation::startNow() {
    setStartTime(AnimationUtils::currentAnimationTimeMillis());
}

void Animation::setRepeatMode(int repeatMode) {
    mRepeatMode = repeatMode;
}

void Animation::setRepeatCount(int repeatCount) {
    if (repeatCount < 0) {
        repeatCount = INFINITE;
    }
    mRepeatCount = repeatCount;
}

bool Animation::isFillEnabled()const{
    return mFillEnabled;
}

void Animation::setFillEnabled(bool fillEnabled) {
    mFillEnabled = fillEnabled;
}

void Animation::setFillBefore(bool fillBefore) {
    mFillBefore = fillBefore;
}

void Animation::setFillAfter(bool fillAfter) {
    mFillAfter = fillAfter;
}

void Animation::setZAdjustment(int zAdjustment) {
    mZAdjustment = zAdjustment;
}

void Animation::setBackgroundColor(int bg) {
    mBackgroundColor = bg;
}

float Animation::getScaleFactor()const{
    return mScaleFactor;
}
void Animation::setDetachWallpaper(bool detachWallpaper) {
    mDetachWallpaper = detachWallpaper;
}

void Animation::setShowWallpaper(bool showWallpaper) {
    mShowWallpaper = showWallpaper;
}

const Interpolator* Animation::getInterpolator()const{
    return mInterpolator;
}

int64_t Animation::getStartTime()const{
    return mStartTime;
}

int64_t Animation::getDuration() const{
    return mDuration;
}

int64_t Animation::getStartOffset() const{
    return mStartOffset;
}

int Animation::getRepeatMode()const{
    return mRepeatMode;
}

int Animation::getRepeatCount()const{
    return mRepeatCount;
}

bool Animation::getFillBefore()const{
    return mFillBefore;
}

bool Animation::getFillAfter()const{
    return mFillAfter;
}

int Animation::getZAdjustment()const{
    return mZAdjustment;
}

int Animation::getBackgroundColor()const{
    return mBackgroundColor;
}

bool Animation::getDetachWallpaper()const{
    return mDetachWallpaper;
}

bool Animation::getShowWallpaper()const{
    return mShowWallpaper;
}

bool Animation::willChangeTransformationMatrix()const{
    // assume we will change the matrix
    return true;
}

bool Animation::willChangeBounds()const{
    // assume we will change the bounds
    return true;
}

int Animation::getExtensionEdges()const{
    return 0;
}
void Animation::setAnimationListener(const AnimationListener& listener) {
    mListener = listener;
}

void Animation::ensureInterpolator() {
    if (mInterpolator == nullptr) {
        mInterpolator = AccelerateDecelerateInterpolator::Instance;
    }
}

int64_t Animation::computeDurationHint() {
    return (getStartOffset() + getDuration()) * (getRepeatCount() + 1);
}

bool Animation::getTransformation(int64_t currentTime, Transformation& outTransformation) {
    if (mStartTime == -1) mStartTime = currentTime;

    int64_t startOffset = getStartOffset();
    int64_t duration = mDuration;
    float normalizedTime;
    if (duration != 0) {
        normalizedTime = ((float)(currentTime-(mStartTime+startOffset))) /(float) duration;
    } else {
        // time is a step-change with a zero duration
        normalizedTime = currentTime < mStartTime ? 0.0f : 1.0f;
    }

    const bool expired = normalizedTime >= 1.0f || isCanceled();
    mMore = !expired;

    if (!mFillEnabled) normalizedTime = std::max(std::min(normalizedTime, 1.0f), 0.0f);

    if ((normalizedTime >= 0.0f || mFillBefore) && (normalizedTime <= 1.0f || mFillAfter)) {
        if (!mStarted) {
            fireAnimationStart();
            mStarted = true;
        }

        if (mFillEnabled) normalizedTime = std::max(std::min(normalizedTime, 1.0f), 0.0f);

        if (mCycleFlip) normalizedTime = 1.0f - normalizedTime;

        const float interpolatedTime = mInterpolator->getInterpolation(normalizedTime);
        applyTransformation(interpolatedTime, outTransformation);
    }
    if (expired) {
        if (mRepeatCount == mRepeated || isCanceled()) {
            if (!mEnded) {
                mEnded = true;
                fireAnimationEnd();
            }
        } else {
            if (mRepeatCount > 0)  mRepeated++;

            if (mRepeatMode == REVERSE)  mCycleFlip = !mCycleFlip;

            mStartTime = -1;
            mMore = true;

            fireAnimationRepeat();
        }
    }

    if (!mMore && mOneMoreTime) {
        mOneMoreTime = false;
        return true;
    }

    return mMore;
}

bool Animation::isCanceled() {
    return mStartTime == LLONG_MIN;
}

void Animation::fireAnimationStart() {
    if (mListener.onAnimationStart) {
        if (mListenerHandler == nullptr) mListener.onAnimationStart(*this);
        //else mListenerHandler.postAtFrontOfQueue(mOnStart);
    }
}

void Animation::fireAnimationRepeat() {
    if (mListener.onAnimationRepeat) {
        if (mListenerHandler == nullptr) mListener.onAnimationRepeat(*this);
        //else mListenerHandler.postAtFrontOfQueue(mOnRepeat);
    }
}

void Animation::fireAnimationEnd() {
    if (mListener.onAnimationEnd) {
        if (mListenerHandler == nullptr) mListener.onAnimationEnd(*this);
        //else mListenerHandler.postAtFrontOfQueue(mOnEnd);
    }
}

bool Animation::getTransformation(int64_t currentTime, Transformation& outTransformation,float scale) {
    mScaleFactor = scale;
    return getTransformation(currentTime, outTransformation);
}

void Animation::applyTransformation(float interpolatedTime, Transformation& t) {
}

bool Animation::hasStarted()const{
    return mStarted;
}

bool Animation::hasEnded()const{
    return mEnded;
}

float Animation::resolveSize(int type, float value, int size, int parentSize) {
    switch (type) {
    case ABSOLUTE:return value;
    case RELATIVE_TO_SELF:return size * value;
    case RELATIVE_TO_PARENT:return parentSize * value;
    default:return value;
    }
}

void Animation::getInvalidateRegion(int left, int top, int width, int height,
            Rect& invalidate, Transformation& transformation) {

    Rect tempRegion = mRegion;
    Rect previousRegion = mPreviousRegion;

    invalidate.set(left, top, width, height);
    transformation.getMatrix().transform_rectangle((RectangleInt&)invalidate);
    // Enlarge the invalidate region to account for rounding errors
    invalidate.inset(-1, -1);
    tempRegion = invalidate;//.set(invalidate);
    invalidate.Union(previousRegion);

    previousRegion = tempRegion;//.set(tempRegion);

    Transformation tempTransformation = mTransformation;
    Transformation& previousTransformation = mPreviousTransformation;

    tempTransformation.set(transformation);
    transformation.set(previousTransformation);
    previousTransformation.set(tempTransformation);
}

void Animation::initializeInvalidateRegion(int left, int top, int width, int height) {
    Rect region = mPreviousRegion;
    region.set(left, top, width, height);
    // Enlarge the invalidate region to account for rounding errors
    region.inset(-1, -1);
    if (mFillBefore) {
        Transformation previousTransformation = mPreviousTransformation;
        applyTransformation(mInterpolator->getInterpolation(0.0f), previousTransformation);
    }
}

void Animation::finalize(){
    /*try {
        if (guard != null) {
            guard.warnIfOpen();
        }
    } finally {
        super.finalize();
    }*/
}

bool Animation::hasAlpha(){
    return false;
}

}
