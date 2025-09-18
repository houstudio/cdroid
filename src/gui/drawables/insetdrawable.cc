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
#include <drawables/insetdrawable.h>
#include <cdlog.h>
namespace cdroid{

void InsetDrawable::InsetValue::set(float f,int d){
    mFraction = f;
    mDimension= d;
}
void InsetDrawable::InsetValue::set(float f){
   mFraction =(f<1.f)?f:0.f;
   mDimension=(f>=1.f)?int(f):0;
}

int InsetDrawable::InsetValue::getDimension(int boundSize)const{
    return (int) (boundSize * mFraction) + mDimension;
}

InsetDrawable::InsetState::InsetState():DrawableWrapperState(){
    mInset.set(0,0,0,0);
}

InsetDrawable::InsetState::InsetState(const InsetState& orig)
    :DrawableWrapperState(orig){
    mInset = orig.mInset;
    mInsetLeft = orig.mInsetLeft;
    mInsetRight= orig.mInsetRight;
    mInsetTop  = orig.mInsetTop;
    mInsetBottom=orig.mInsetBottom;
}

void InsetDrawable::InsetState::applyDensityScaling(int sourceDensity, int targetDensity){
}

void InsetDrawable::InsetState::onDensityChanged(int sourceDensity, int targetDensity){
}

InsetDrawable*InsetDrawable::InsetState::newDrawable(){
    return new InsetDrawable(std::dynamic_pointer_cast<InsetState>(shared_from_this()));
}

InsetDrawable::InsetDrawable():DrawableWrapper(std::make_shared<InsetState>()){
    mState = std::dynamic_pointer_cast<InsetState>(DrawableWrapper::mState);
}

InsetDrawable::InsetDrawable(std::shared_ptr<InsetState>state):DrawableWrapper(state){
    mState = state;
}

InsetDrawable::InsetDrawable(Drawable*drawable,int inset)
    :InsetDrawable(drawable,inset,inset,inset,inset){
}

InsetDrawable::InsetDrawable(Drawable* drawable,int insetLeft,int insetTop,int insetRight,int insetBottom)
    :InsetDrawable(std::make_shared<InsetState>()){
    setDrawable(drawable);
    mState->mInset.set(insetLeft,insetTop,insetRight,insetBottom);
    mState->mInsetLeft.set(0.f, insetLeft);
    mState->mInsetTop.set(0.f, insetTop);
    mState->mInsetRight.set(0.f, insetRight);
    mState->mInsetBottom.set(0.f, insetBottom);
}

std::shared_ptr<DrawableWrapper::DrawableWrapperState> InsetDrawable::mutateConstantState(){
    mState=std::make_shared<InsetState>(*mState);
    return mState;
}

void InsetDrawable::getInsets(Rect& out) {
    Rect b = getBounds();
    out.left  = mState->mInsetLeft.getDimension(b.width);
    out.width = mState->mInsetRight.getDimension(b.width);
    out.top   = mState->mInsetTop.getDimension(b.height);
    out.height= mState->mInsetBottom.getDimension(b.height);
}

bool InsetDrawable::getPadding(Rect& padding) {
    bool pad = DrawableWrapper::getPadding(padding);
    Rect tmp;
    getInsets(tmp);
    padding.left  += tmp.left;
    padding.width += tmp.width;
    padding.top   += tmp.top;
    padding.height+= tmp.height;

    return pad || (tmp.left | tmp.width | tmp.top | tmp.height) != 0;
}

Insets InsetDrawable::getOpticalInsets() {
    const Insets contentInsets = DrawableWrapper::getOpticalInsets();
    Rect tmp;
    getInsets(tmp);
    return Insets::of(
            contentInsets.left + tmp.left,
            contentInsets.top + tmp.top,
            contentInsets.right + tmp.width,
            contentInsets.bottom + tmp.height);
}

int InsetDrawable::getOpacity() {
    int opacity = TRANSLUCENT;
    Rect tmp;
    getInsets(tmp);
    if(getDrawable())
	opacity = getDrawable()->getOpacity();
    if (opacity == OPAQUE && (tmp.left > 0 || tmp.top > 0 || tmp.width > 0 || tmp.height > 0)) {
        return TRANSLUCENT;
    }
    return opacity;
}

void InsetDrawable::onBoundsChange(const Rect&bounds){
    Rect r = bounds;
  
    r.left  += mState->mInsetLeft.getDimension(bounds.width);
    r.top   += mState->mInsetTop.getDimension(bounds.height);
    r.width -= mState->mInsetRight.getDimension(bounds.width);
    r.height-= mState->mInsetBottom.getDimension(bounds.height);
    DrawableWrapper::onBoundsChange(r);
}

int InsetDrawable::getIntrinsicWidth() {
    const int childWidth = getDrawable()->getIntrinsicWidth();
    const float fraction = mState->mInsetLeft.mFraction + mState->mInsetRight.mFraction;
    if (childWidth < 0 || fraction >= 1) {
        return -1;
    }
    return (int) (childWidth / (1.f - fraction)) + mState->mInsetLeft.mDimension
        + mState->mInsetRight.mDimension;
}

int InsetDrawable::getIntrinsicHeight() {
    const int childHeight = getDrawable()->getIntrinsicHeight();
    const float fraction = mState->mInsetTop.mFraction + mState->mInsetBottom.mFraction;
    if (childHeight < 0 || fraction >= 1) {
        return -1;
    }
    return (int) (childHeight / (1.f - fraction)) + mState->mInsetTop.mDimension
        + mState->mInsetBottom.mDimension;
}

void InsetDrawable::getOutline(Outline&outline){
    getDrawable()->getOutline(outline);
}

std::shared_ptr<Drawable::ConstantState>InsetDrawable::getConstantState(){
    return mState;
}

void InsetDrawable::inflate(XmlPullParser&parser,const AttributeSet&atts){
    // Inset attribute may be overridden by more specific attributes.
    updateStateFromTypedArray(atts);
    DrawableWrapper::inflate(parser,atts);
    verifyRequiredAttributes();
}

void InsetDrawable::verifyRequiredAttributes(){
    // If we're not waiting on a theme, verify required attributes.
    if (getDrawable() == nullptr /*&& (mState.mThemeAttrs == null
            || mState.mThemeAttrs[R.styleable.InsetDrawable_drawable] == 0)*/) {
        LOGE("<inset> tag requires a 'drawable' attribute or child tag defining a drawable");
    }
}

void InsetDrawable::updateStateFromTypedArray(const AttributeSet&atts){
    if (atts.hasAttribute("inset")) {
        const float inset = atts.getFloat("inset", 0);
        mState->mInsetLeft.set(inset);
        mState->mInsetTop.set(inset);
        mState->mInsetRight.set(inset);
        mState->mInsetBottom.set(inset);
    }
    mState->mInsetLeft.set(atts.getFloat("insetLeft", 0.f));
    mState->mInsetTop.set(atts.getFloat("insetTop", 0.f));
    mState->mInsetRight.set(atts.getFloat("insetRight", 0.f));
    mState->mInsetBottom.set(atts.getFloat("insetBottom", 0.f));
}
}/*endof namespace*/

