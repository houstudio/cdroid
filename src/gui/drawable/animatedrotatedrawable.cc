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
#include <widget/internal_R.h>
#include <drawable/animatedrotatedrawable.h>
#include <drawable/bitmapdrawable.h>
#include <widget/framework_styleable.h>
#include <core/systemclock.h>
#include <utils/mathutils.h>
#include <porting/cdlog.h>
#include <fstream>

using namespace Cairo;
namespace cdroid{
using namespace cdroid::internal;

AnimatedRotateDrawable::AnimatedRotateState::AnimatedRotateState(){
    // AOSP AnimatedRotateState defaults: absolute top-left pivot (0,0), NOT
    // a centered relative one.
    mPivotX = mPivotY = 0;
    mPivotXRel=mPivotYRel=false;
    mFrameDuration=150;
    mFramesCount=12;
}

AnimatedRotateDrawable::AnimatedRotateState::AnimatedRotateState(const AnimatedRotateState& orig)
    :DrawableWrapperState(orig){
    mPivotXRel = orig.mPivotXRel;
    mPivotX = orig.mPivotX;
    mPivotYRel = orig.mPivotYRel;
    mPivotY = orig.mPivotY;
    mFramesCount = orig.mFramesCount;
    mFrameDuration = orig.mFrameDuration;
}

AnimatedRotateDrawable* AnimatedRotateDrawable::AnimatedRotateState::newDrawable(){
    return new AnimatedRotateDrawable(std::dynamic_pointer_cast<AnimatedRotateState>(shared_from_this()));
}
int  AnimatedRotateDrawable::AnimatedRotateState::getChangingConfigurations()const{
    return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////
AnimatedRotateDrawable::AnimatedRotateDrawable()
    :AnimatedRotateDrawable(std::make_shared<AnimatedRotateState>()){
}

AnimatedRotateDrawable::AnimatedRotateDrawable(std::shared_ptr<AnimatedRotateState> state):DrawableWrapper(state){
    mState  = state;
    mRunning= false;
    mIncrement= 360./state->mFramesCount;
    mCurrentDegrees = .0f;

    updateLocalState();
    mNextFrame = [this](){
        mCurrentDegrees += mIncrement;
        if( (mIncrement < 0) && (mCurrentDegrees < -360.f - mIncrement) ){
            mCurrentDegrees = 0.f;
        }else if ((mIncrement > 0) && (mCurrentDegrees > 360.0f - mIncrement)) {
            mCurrentDegrees = 0.0f;
        }
        invalidateSelf();
        nextFrame();
    };
}

AnimatedRotateDrawable::~AnimatedRotateDrawable(){
    stop();
    mNextFrame = nullptr;
}

std::shared_ptr<DrawableWrapper::DrawableWrapperState> AnimatedRotateDrawable::mutateConstantState(){
    mState = std::make_shared<AnimatedRotateState>(*mState);
    return mState;
}

float AnimatedRotateDrawable::getPivotX()const{
    return mState->mPivotX;
}

float AnimatedRotateDrawable::getPivotY()const{
    return mState->mPivotY;
}

void AnimatedRotateDrawable::setPivotX(float pivotX){
    mState->mPivotX=pivotX;
}

void AnimatedRotateDrawable::setPivotY(float pivotY){
     mState->mPivotY=pivotY;
}

bool AnimatedRotateDrawable::isPivotXRelative()const{
    return mState->mPivotXRel;
}

void AnimatedRotateDrawable::setPivotXRelative(bool relative){
    mState->mPivotXRel=relative;
}

bool AnimatedRotateDrawable::isPivotYRelative()const{
    return mState->mPivotYRel;
}

void AnimatedRotateDrawable::setPivotYRelative(bool relative){
    mState->mPivotYRel=relative;
}

void AnimatedRotateDrawable::start() {
    LOGV("AnimatedRotateDrawable.start %p, mRunning=%d",this,mRunning);
    if (!mRunning) {
        mRunning = true;
        nextFrame();
    }
}

void AnimatedRotateDrawable::stop() {
    LOGV("AnimatedRotateDrawable.stoped %p,running=%d",this,mRunning);
    mRunning = false;
    unscheduleSelf(mNextFrame);
}

bool AnimatedRotateDrawable::isRunning() {
    return mRunning;
}

void AnimatedRotateDrawable::nextFrame() {
    // AOSP nextFrame() only (un)schedules; the degree advance lives solely in
    // the mNextFrame runnable — doing it here as well advanced the rotation
    // twice per tick.
    unscheduleSelf(mNextFrame);
    if(mRunning){
        scheduleSelf(mNextFrame,SystemClock::uptimeMillis()+mState->mFrameDuration);
    }
}

bool AnimatedRotateDrawable::setVisible(bool visible, bool restart){
    const bool changed = DrawableWrapper::setVisible(visible, restart);
    if (visible) {
        if (changed || restart) {
            mCurrentDegrees = 0.0f;
            nextFrame();
        }
    } else {
        unscheduleSelf(mNextFrame);
    }
    return changed;
}

void AnimatedRotateDrawable::setFramesCount(int framesCount) {
    mState->mFramesCount = framesCount;
    mIncrement = 360.0f / mState->mFramesCount;
}

void AnimatedRotateDrawable::setFramesDuration(int framesDuration) {
    mState->mFrameDuration = framesDuration;
}

void AnimatedRotateDrawable::updateLocalState(){
    mIncrement = 360.0f / mState->mFramesCount;

    // Force the wrapped drawable to use filtering and AA, if applicable,
    // so that it looks smooth when rotated.
    Drawable* drawable = getDrawable();
    if (drawable != nullptr) {
        drawable->setFilterBitmap(true);
        if (dynamic_cast<BitmapDrawable*>(drawable)) {
            ((BitmapDrawable*)drawable)->setAntiAlias(true);
        }
    }
}

void AnimatedRotateDrawable::draw(Canvas& canvas) {
    Drawable* drawable = getDrawable();
    const Rect bounds = drawable->getBounds();
    const int w = bounds.width;
    const int h = bounds.height;

    const float px = bounds.left + (mState->mPivotXRel ? (w * mState->mPivotX) : mState->mPivotX);
    const float py = bounds.top  + (mState->mPivotYRel ? (h * mState->mPivotY) : mState->mPivotY);
    LOGV("%p bounds(%d,%d %d,%d) pivot=%f,%f pxy=%f,%f degrees=%f",this,bounds.left,bounds.top,bounds.width,bounds.height,
         mState->mPivotX, mState->mPivotY,px,py,mCurrentDegrees);
#if 0
    auto sdot = [](float a,float b,float c,float d){
        return a * b + c * d;
    };
    const float radians =M_PI*mCurrentDegrees/180.f;
    const float fsin = sin(radians);
    const float fcos = cos(radians);
    Matrix mtx(fcos,fsin, -fsin,fcos, sdot(fsin,py,1-fcos,px), sdot(-fsin,px,1-fcos,py));
#else
    Matrix mtx=identity_matrix();
    mtx.translate(px,py);
    mtx.rotate(MathUtils::toRadians(mCurrentDegrees));
    mtx.translate(-px,-py);
#endif
    if(drawable){
        canvas.save();
        canvas.transform(mtx);
        drawable->draw(canvas);
        canvas.restore();
    }
}

void AnimatedRotateDrawable::inflate(Resources& r,XmlPullParser&parser,const AttributeSet&atts, const Resources::Theme* theme){
    auto ta = obtainAttributes(r, theme, atts, R::styleable::AnimatedRotateDrawable);
    if (ta) {
        updateStateFromTypedArray(*ta);
        // frameDuration/framesCount lack a framework arsc id (CDROID-private): read via
        // the string bridge (text-XML works; binary returns the default — harmless).
        mState->mFramesCount = ta->getInt(R::styleable::AnimatedRotateDrawable_framesCount, mState->mFramesCount);
        mState->mFrameDuration = ta->getInt(R::styleable::AnimatedRotateDrawable_frameDuration, mState->mFrameDuration);
    }
    DrawableWrapper::inflate(r,parser,atts, theme);
    updateLocalState();
}

void AnimatedRotateDrawable::updateStateFromTypedArray(const TypedArray& a){
    // AOSP AnimatedRotateState: rel-ness follows the value TYPE; a FRACTION
    // pivot ("50%") is read with getFraction (a plain getFloat cannot decode a
    // complex fraction value and fell back to 0, moving the rotation center to
    // the top-left corner), an absolute one with getDimension.
    TypedValue tv;
    if (a.getValue(R::styleable::AnimatedRotateDrawable_pivotX, &tv)) {
        mState->mPivotXRel = (tv.type == TypedValue::TYPE_FRACTION);
        mState->mPivotX = mState->mPivotXRel
            ? a.getFraction(R::styleable::AnimatedRotateDrawable_pivotX, 1, 1, mState->mPivotX)
            : (float)a.getDimensionPixelOffset(R::styleable::AnimatedRotateDrawable_pivotX, (int)mState->mPivotX);
    }
    if (a.getValue(R::styleable::AnimatedRotateDrawable_pivotY, &tv)) {
        mState->mPivotYRel = (tv.type == TypedValue::TYPE_FRACTION);
        mState->mPivotY = mState->mPivotYRel
            ? a.getFraction(R::styleable::AnimatedRotateDrawable_pivotY, 1, 1, mState->mPivotY)
            : (float)a.getDimensionPixelOffset(R::styleable::AnimatedRotateDrawable_pivotY, (int)mState->mPivotY);
    }
}
}

