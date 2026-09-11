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
#include <drawable/insetdrawable.h>
#include <widget/framework_styleable.h>
#include <cdlog.h>
namespace cdroid{
using namespace cdroid::internal;

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
    return (InsetDrawable*)newDrawable(nullptr);
}

Drawable*InsetDrawable::InsetState::newDrawable(Resources* res){
    return new InsetDrawable(std::dynamic_pointer_cast<InsetState>(shared_from_this()), res);
}

InsetDrawable::InsetDrawable():DrawableWrapper(std::make_shared<InsetState>(), nullptr){
    mState = std::dynamic_pointer_cast<InsetState>(DrawableWrapper::mState);
}

InsetDrawable::InsetDrawable(std::shared_ptr<InsetState>state,Resources*res):DrawableWrapper(state,res){
    mState = state;
}

InsetDrawable::InsetDrawable(Drawable*drawable,int inset)
    :InsetDrawable(drawable,inset,inset,inset,inset){
}

InsetDrawable::InsetDrawable(Drawable* drawable,int insetLeft,int insetTop,int insetRight,int insetBottom)
    :InsetDrawable(std::make_shared<InsetState>(), nullptr){
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

void InsetDrawable::getInsets(Rect& out) const{
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

int InsetDrawable::getOpacity() const{
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

    // AOSP builds an ltrb rect (left+insetL, top+insetT, right-insetR,
    // bottom-insetB); cdroid's Rect is left/top/width/height, so BOTH insets
    // come off each axis — subtracting only the trailing one shrank the rect
    // by half and left it flush against the far edge.
    const int il = (int)mState->mInsetLeft.getDimension(bounds.width);
    const int it = (int)mState->mInsetTop.getDimension(bounds.height);
    const int ir = (int)mState->mInsetRight.getDimension(bounds.width);
    const int ib = (int)mState->mInsetBottom.getDimension(bounds.height);
    r.left   += il;
    r.top    += it;
    r.width  -= il + ir;
    r.height -= it + ib;
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

void InsetDrawable::inflate(Resources&r,XmlPullParser&parser,const AttributeSet&atts,const Resources::Theme* theme){
    // Inset attribute may be overridden by more specific attributes.
    auto ta = obtainAttributes(r, theme, atts, R::styleable::InsetDrawable);
    if (ta) {
        mState->mThemeAttrs = ta->extractThemeAttrs();
        updateStateFromTypedArray(*ta);
    }
    DrawableWrapper::inflate(r,parser,atts, theme);
    verifyRequiredAttributes();
}

// AOSP InsetDrawable.canApplyTheme/applyTheme.
bool InsetDrawable::canApplyTheme(){
    return (mState && !mState->mThemeAttrs.empty()) || DrawableWrapper::canApplyTheme();
}

void InsetDrawable::applyTheme(const Resources::Theme& t){
    DrawableWrapper::applyTheme(t);
    if (mState && !mState->mThemeAttrs.empty()) {
        auto a = t.resolveAttributes(mState->mThemeAttrs, R::styleable::InsetDrawable);
        if (a) updateStateFromTypedArray(*a);
        mState->mThemeAttrs.clear();
    }
}

void InsetDrawable::verifyRequiredAttributes(){
    // If we're not waiting on a theme, verify required attributes.
    if (getDrawable() == nullptr /*&& (mState.mThemeAttrs == null
            || mState.mThemeAttrs[R.styleable.InsetDrawable_drawable] == 0)*/) {
        LOGE("<inset> tag requires a 'drawable' attribute or child tag defining a drawable");
    }
}

void InsetDrawable::updateStateFromTypedArray(const TypedArray& a){
    // AOSP getInset(): a fraction value (%) sets the fraction field, anything
    // else reads as a density-applied dimension offset. The old plain
    // getFloat lost density scaling (16dp stayed 16px on a 2x display) and
    // fed the f<1 heuristic in InsetValue::set(float). Absent attributes keep
    // the existing value (AOSP behavior; android:inset must not be wiped).
    auto setInset = [&a](InsetDrawable::InsetValue& v, size_t idx) {
        TypedValue tv;
        if (!a.getValue(idx, &tv)) return;
        if (tv.type == TypedValue::TYPE_FRACTION) {
            v.set(a.getFraction(idx, 1, 1, 0.f), 0);
        } else {
            v.set(0.f, a.getDimensionPixelOffset(idx, 0));
        }
    };
    if (a.hasValue(R::styleable::InsetDrawable_inset)) {
        setInset(mState->mInsetLeft,   R::styleable::InsetDrawable_inset);
        setInset(mState->mInsetTop,    R::styleable::InsetDrawable_inset);
        setInset(mState->mInsetRight,  R::styleable::InsetDrawable_inset);
        setInset(mState->mInsetBottom, R::styleable::InsetDrawable_inset);
    }
    setInset(mState->mInsetLeft,   R::styleable::InsetDrawable_insetLeft);
    setInset(mState->mInsetTop,    R::styleable::InsetDrawable_insetTop);
    setInset(mState->mInsetRight,  R::styleable::InsetDrawable_insetRight);
    setInset(mState->mInsetBottom, R::styleable::InsetDrawable_insetBottom);
}
}/*endof namespace*/

