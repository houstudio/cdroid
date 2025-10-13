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
#include <drawables/rotatedrawable.h>
#include <utils/mathutils.h>
#include <porting/cdlog.h>

using namespace Cairo;
namespace cdroid{

RotateDrawable::RotateState::RotateState()
    :DrawableWrapperState(){
    mFromDegrees =.0;
    mToDegrees = 360.0;
    mPivotX = mPivotY = 0.5;
    mPivotXRel = mPivotYRel = true;
    mCurrentDegrees = 0;
}

RotateDrawable::RotateState::RotateState(const RotateState& orig)
    :DrawableWrapperState(orig){
    mFromDegrees= orig.mFromDegrees;
    mToDegrees  = orig.mToDegrees;
    mPivotX = orig.mPivotX;
    mPivotY = orig.mPivotY;
    mPivotXRel= orig.mPivotXRel;
    mPivotYRel= orig.mPivotYRel;
    mCurrentDegrees = orig.mCurrentDegrees;
}

RotateDrawable*RotateDrawable::RotateState::newDrawable(){
    return new RotateDrawable(std::dynamic_pointer_cast<RotateState>(shared_from_this()));
}

//////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<DrawableWrapper::DrawableWrapperState> RotateDrawable::mutateConstantState(){
    mState = std::make_shared<RotateState>(*mState);
    return mState;
}

RotateDrawable::RotateDrawable(std::shared_ptr<RotateState>state):DrawableWrapper(state){
    mState = state;
}

RotateDrawable::RotateDrawable(Drawable*d)
    :RotateDrawable(std::make_shared<RotateState>()){
    setDrawable(d);
}

bool RotateDrawable::onLevelChange(int level){
    DrawableWrapper::onLevelChange(level);
    const float value = level / (float) MAX_LEVEL;
    const float degrees = MathUtils::lerp(mState->mFromDegrees,mState->mToDegrees, value);
    mState->mCurrentDegrees = degrees;
    invalidateSelf();
    return true;
}

float RotateDrawable::getFromDegrees()const{
    return mState->mFromDegrees;
}

float RotateDrawable::getToDegrees()const{
    return mState->mToDegrees;
}

void RotateDrawable::setFromDegrees(float fromDegrees) {
    if (mState->mFromDegrees != fromDegrees) {
        mState->mFromDegrees = fromDegrees;
        invalidateSelf();
    }
}

void RotateDrawable::setToDegrees(float toDegrees) {
    if (mState->mToDegrees != toDegrees) {
        mState->mToDegrees = toDegrees;
        invalidateSelf();
    }
}

float RotateDrawable::getPivotX()const{
    return mState->mPivotX;
}

float RotateDrawable::getPivotY()const{
    return mState->mPivotY;
}

void RotateDrawable::setPivotX(float pivotX) {
    if (mState->mPivotX != pivotX) {
        mState->mPivotX = pivotX;
        invalidateSelf();
    }
}

void RotateDrawable::setPivotY(float pivotY) {
    if (mState->mPivotY != pivotY) {
        mState->mPivotY = pivotY;
        invalidateSelf();
    }
}

void RotateDrawable::setPivotXRelative(bool relative) {
    if (mState->mPivotXRel != relative) {
        mState->mPivotXRel = relative;
        invalidateSelf();
    }
}

bool RotateDrawable::isPivotYRelative()const{
    return mState->mPivotYRel;
}

void RotateDrawable::setPivotYRelative(bool relative){
    if(mState->mPivotYRel!=relative){
        mState->mPivotYRel=relative;
        invalidateSelf();
    }
}

std::shared_ptr<Drawable::ConstantState>RotateDrawable::getConstantState(){
    return mState;
}

void RotateDrawable::draw(Canvas& canvas) {
    Drawable*d = getDrawable();
    const Rect bounds = getBounds();
    const float px = bounds.left + (mState->mPivotXRel ? (bounds.width * mState->mPivotX) : mState->mPivotX);
    const float py = bounds.top  + (mState->mPivotYRel ? (bounds.height * mState->mPivotY) : mState->mPivotY);
    LOGV("%p bounds(%d,%d %d,%d) pivot=%f,%f pxy=%f,%f degrees=%f",this,bounds.left,bounds.top,bounds.width,bounds.height,
         mState->mPivotX, mState->mPivotY,px,py,mState->mCurrentDegrees);
#if 0
    auto sdot = [](float a,float b,float c,float d){
        return a * b + c * d;
    };
    const float radians = M_PI*mState->mCurrentDegrees/180.f;
    const float fsin = sin(radians);
    const float fcos = cos(radians);
    Matrix mtx(fcos,fsin, -fsin,fcos, sdot(fsin,py,1-fcos,px), sdot(-fsin,px,1-fcos,py));
#else
    Matrix mtx=identity_matrix();
    mtx.translate(px,py);
    mtx.rotate(MathUtils::toRadians(mState->mCurrentDegrees));
    mtx.translate(-px,-py);
#endif
    if(d){
        canvas.save();
        canvas.transform(mtx);
        d->draw(canvas);
        canvas.restore();
    }
    LOGV("pos=%d,%d/%.f,%.f level=%d degress=%d",bounds.left,bounds.top,px,py,getLevel(),int(mState->mCurrentDegrees));
}

void RotateDrawable::inflate(XmlPullParser&parser,const AttributeSet&atts){
    updateStateFromTypedArray(atts);
    DrawableWrapper::inflate(parser,atts);
}

void RotateDrawable::updateStateFromTypedArray(const AttributeSet&atts){
    mState->mPivotX = atts.getFraction("pivotX",1,1,mState->mPivotX);
    mState->mPivotXRel = (mState->mPivotX <=1.f);

    mState->mPivotY = atts.getFraction("pivotY",1,1.0f,mState->mPivotY);
    mState->mPivotYRel = (mState->mPivotY <=1.0f);

    mState->mFromDegrees = atts.getFloat("fromDegrees", mState->mFromDegrees);
    mState->mToDegrees = atts.getFloat("toDegrees", mState->mToDegrees);
    mState->mCurrentDegrees = mState->mFromDegrees;
}
}/*endof namespace*/
