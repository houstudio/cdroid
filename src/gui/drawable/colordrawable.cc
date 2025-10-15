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
#include <drawable/colordrawable.h>
#include <drawable/colormatrix.h>
#include <porting/cdlog.h>


namespace cdroid{

ColorDrawable::ColorState::ColorState(){
    mTint = nullptr;
    mBaseColor= 0xFF000000;
    mUseColor = 0xFF000000;
    mTintMode = DEFAULT_TINT_MODE;
}

ColorDrawable::ColorState::ColorState(const ColorState& state){
    mBaseColor= state.mBaseColor;
    mUseColor = state.mUseColor;
    mTint = state.mTint;
    mTintMode=state.mTintMode;
}

ColorDrawable* ColorDrawable::ColorState::newDrawable(){
    return new ColorDrawable(shared_from_this());
}

int ColorDrawable::ColorState::getChangingConfigurations()const{
    return 0;
}

ColorDrawable::ColorDrawable(int color){
    mColorState=std::make_shared<ColorState>();
    mMutated = false;
    mTintFilter = nullptr;
    setColor(color);
}

ColorDrawable::ColorDrawable(std::shared_ptr<ColorState> state){
    mColorState = state;
    mTintFilter = nullptr;
    mMutated = false;
}

ColorDrawable::~ColorDrawable(){
    delete mTintFilter;
}

void ColorDrawable::inflate(XmlPullParser&parser,const AttributeSet&atts){
    mColorState->mBaseColor = atts.getColor("color", mColorState->mBaseColor);
    mColorState->mUseColor = mColorState->mBaseColor;
}

std::shared_ptr<Drawable::ConstantState>ColorDrawable::getConstantState(){
    return std::dynamic_pointer_cast<ConstantState>(mColorState);
}

ColorDrawable*ColorDrawable::mutate(){
    if (!mMutated && (Drawable::mutate() == this)) {
        mColorState=std::make_shared<ColorState>(*mColorState);
        mMutated = true;
    }
    return this;    
}

void ColorDrawable::clearMutated(){
    Drawable::clearMutated();
    mMutated=false;
}

void ColorDrawable::setColor(int color){
    if (mColorState->mBaseColor != color || mColorState->mUseColor != color) {
        mColorState->mBaseColor = mColorState->mUseColor = color;
        invalidateSelf();
    }
}

int ColorDrawable::getColor()const{
    return mColorState->mUseColor;
}

int ColorDrawable::getAlpha()const{
    return mColorState->mUseColor >>24;
}

void ColorDrawable::setAlpha(int alpha){
    alpha&=0xFF;
    const int baseAlpha = (unsigned int)mColorState->mBaseColor >> 24;
    const int useAlpha = baseAlpha * alpha >> 8;
    const int useColor = ((unsigned int)mColorState->mBaseColor << 8 >> 8) | (useAlpha << 24);
    if (mColorState->mUseColor != useColor) {
        mColorState->mUseColor = useColor;
        invalidateSelf();
    }
}

bool ColorDrawable::onStateChange(const std::vector<int>&stateSet){
    if (mColorState->mTint && mColorState->mTintMode != PorterDuff::Mode::NOOP) {
        mTintFilter = updateTintFilter(mTintFilter, mColorState->mTint, mColorState->mTintMode);
        return true;
    }
    return mColorState->mTint!=nullptr;
}

int ColorDrawable::getChangingConfigurations()const{
    return Drawable::getChangingConfigurations() | mColorState->getChangingConfigurations();
}

void ColorDrawable::setTintList(const ColorStateList* tint){
    if( mColorState->mTint!=tint ){
        mColorState->mTint = tint;
        mTintFilter = updateTintFilter(mTintFilter, tint, mColorState->mTintMode);
        invalidateSelf();
    }
}

void ColorDrawable::setTintMode(int tintMode) {
    mColorState->mTintMode = tintMode;
    mTintFilter = updateTintFilter(mTintFilter, mColorState->mTint, tintMode);
    LOGV("tintmode=%d",tintMode);
    invalidateSelf();
}

bool ColorDrawable::isStateful()const{
    return mColorState->mTint && mColorState->mTint->isStateful();
}

bool ColorDrawable::hasFocusStateSpecified()const{
    return mColorState->mTint && mColorState->mTint->hasFocusStateSpecified();
}

int ColorDrawable::getOpacity() {
    if (mTintFilter /*|| mPaint.getColorFilter() != null*/) {
       return PixelFormat::TRANSLUCENT;
    }
    switch (mColorState->mUseColor >> 24) {
    case 255:return PixelFormat::OPAQUE;
    case 0 : return PixelFormat::TRANSPARENT;
    }
    return PixelFormat::TRANSLUCENT;
}

void ColorDrawable::getOutline(Outline& outline) {
    outline.setRect(getBounds());
    outline.setAlpha(getAlpha() / 255.0f);
}

void ColorDrawable::draw(Canvas&canvas){
    LOGV("%p color=%x  bounds=%d,%d-%d,%d mTintFilter=%p",this,mColorState->mUseColor,
	mBounds.left,mBounds.top,mBounds.width,mBounds.height,mTintFilter);
    canvas.save();
    if((mColorState->mUseColor>>24)||mTintFilter){
        canvas.set_color(mColorState->mUseColor);
        if(mTintFilter)
            canvas.set_operator((Cairo::Context::Operator)PorterDuff::toOperator(mTintFilter->getMode()));
        else if(mTintFilter&&(mColorState->mTintMode!=PorterDuff::Mode::NOOP))
            canvas.set_operator((Cairo::Context::Operator)PorterDuff::toOperator(mColorState->mTintMode));
        /*HANDLE handler = canvas.getHandler();
        if(handler&&(mBounds.width==1024||mBounds.width==600)){
            GFXFillRect(handler,nullptr,mColorState->mUseColor);
        }else*/{
            canvas.rectangle(getBounds());
            canvas.fill();
        }
    }
    if(mTintFilter)
        mTintFilter->apply(canvas,mBounds);
    canvas.restore();
}

}
