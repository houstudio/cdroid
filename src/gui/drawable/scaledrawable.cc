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
#include <drawable/scaledrawable.h>
#include <widget/framework_styleable.h>

namespace cdroid{
using namespace cdroid::internal;

ScaleDrawable::ScaleState::ScaleState():DrawableWrapperState(){
    mScaleWidth = DO_NOT_SCALE;
    mScaleHeight= DO_NOT_SCALE;
    mGravity = Gravity::LEFT;
    mUseIntrinsicSizeAsMin = false;
    mInitialLevel = 0;
}

ScaleDrawable::ScaleState::ScaleState(const ScaleState& orig)
    :DrawableWrapperState(orig){
    mScaleWidth = orig.mScaleWidth;
    mScaleHeight = orig.mScaleHeight;
    mGravity = orig.mGravity;
    mUseIntrinsicSizeAsMin = orig.mUseIntrinsicSizeAsMin;
    mInitialLevel = orig.mInitialLevel;
}

ScaleDrawable* ScaleDrawable::ScaleState::newDrawable(){
    return (ScaleDrawable*)newDrawable(nullptr);
}

Drawable* ScaleDrawable::ScaleState::newDrawable(Resources* res){
    return new ScaleDrawable(std::dynamic_pointer_cast<ScaleState>(shared_from_this()), res);
}

////////////////////////////////////////////////////////////////////////////////////////////
ScaleDrawable::ScaleDrawable():ScaleDrawable(std::make_shared<ScaleState>(), nullptr){
}

ScaleDrawable::ScaleDrawable(std::shared_ptr<ScaleState> state,Resources*res):DrawableWrapper(state,res){
    mState = state;
    // AOSP ctor ends with updateLocalState(): initialize the local level to
    // the state's initial level — without it android:level never applied and
    // clones started at level 0 (invisible for a scale-based progress layer).
    setLevel(mState->mInitialLevel);
}

ScaleDrawable::ScaleDrawable(Drawable* drawable, int gravity,float scaleWidth,float scaleHeight)
    :ScaleDrawable(std::make_shared<ScaleState>(), nullptr){
    mState->mGravity    = gravity;
    mState->mScaleWidth = scaleWidth;
    mState->mScaleHeight= scaleHeight;
    mState->mUseIntrinsicSizeAsMin = false;
    setDrawable(drawable);
}

std::shared_ptr<Drawable::ConstantState>ScaleDrawable::getConstantState(){
    return mState;
}

std::shared_ptr<DrawableWrapper::DrawableWrapperState>ScaleDrawable::mutateConstantState(){
    mState = std::make_shared<ScaleState>(*mState);
    return mState;
}

bool ScaleDrawable::onLevelChange(int level) {
    DrawableWrapper::onLevelChange(level);
    onBoundsChange(getBounds());
    invalidateSelf();
    return true;
}

void ScaleDrawable::onBoundsChange(const Rect& bounds){
    Drawable*d = getDrawable();
    Rect r;
    const bool min = mState->mUseIntrinsicSizeAsMin;
    const int level = getLevel();

    int w = bounds.width;
    if (mState->mScaleWidth > 0.f) {
        const int iw = min ? d->getIntrinsicWidth() : 0;
        w -= (int) ((w - iw) * (MAX_LEVEL - level) * mState->mScaleWidth / MAX_LEVEL);
    }

    int h = bounds.height;
    if (mState->mScaleHeight > 0.f) {
        const int ih = min ? d->getIntrinsicHeight() : 0;
        h -= (int) ((h - ih) * (MAX_LEVEL - level) * mState->mScaleHeight / MAX_LEVEL);
    }

    const int layoutDirection = getLayoutDirection();
    Gravity::apply(mState->mGravity, w, h, bounds, r, layoutDirection);

    if (w > 0 && h > 0) {
        d->setBounds(r);
    }
}

int ScaleDrawable::getGravity()const{
    return mState->mGravity;
}

int ScaleDrawable::getOpacity()const{
    Drawable* d = getDrawable();
    if (d->getLevel() == 0) {
        return PixelFormat::TRANSPARENT;
    }
    const int opacity = d->getOpacity();
    if (opacity == PixelFormat::OPAQUE && d->getLevel() < MAX_LEVEL) {
        return PixelFormat::TRANSLUCENT;
    }
    return opacity;
}

void ScaleDrawable::draw(Canvas& canvas) {
    Drawable*d = getDrawable();
    if (d && d->getLevel() != 0) {
        d->draw(canvas);
    }
}

extern int getDimensionOrFraction(const AttributeSet&attrs,const std::string&key,int base,int def);

// AOSP ScaleDrawable.canApplyTheme/applyTheme.
bool ScaleDrawable::canApplyTheme(){
    return (mState && !mState->mThemeAttrs.empty()) || DrawableWrapper::canApplyTheme();
}

void ScaleDrawable::applyTheme(const Resources::Theme& t){
    DrawableWrapper::applyTheme(t);
    if (mState && !mState->mThemeAttrs.empty()) {
        auto a = t.resolveAttributes(mState->mThemeAttrs, R::styleable::ScaleDrawable);
        if (a) updateStateFromTypedArray(*a);
        mState->mThemeAttrs.clear();
    }
}

void ScaleDrawable::inflate(Resources&r,XmlPullParser&parser,const AttributeSet&atts,const Resources::Theme* theme){
    auto ta = obtainAttributes(r, theme, atts, R::styleable::ScaleDrawable);
    if (ta) {
        mState->mThemeAttrs = ta->extractThemeAttrs();
        // AOSP updateStateFromTypedArray: getFraction(scaleWidth, 1, 1, ...) —
        // a fraction is 0..1 (100% = 1.0). The old base=100 hack assumed the
        // text-XML string parser's percent units and produced 100.0, which
        // made onBoundsChange's AOSP formula compute a negative width (no
        // child bounds were ever set, so the progress layer never drew).
        mState->mScaleWidth = ta->getFraction(R::styleable::ScaleDrawable_scaleWidth, 1, 1, mState->mScaleWidth);
        mState->mScaleHeight = ta->getFraction(R::styleable::ScaleDrawable_scaleHeight, 1, 1, mState->mScaleHeight);
        updateStateFromTypedArray(*ta);
    }
    DrawableWrapper::inflate(r,parser,atts, theme);
}

void ScaleDrawable::updateStateFromTypedArray(const TypedArray& a){
    mState->mGravity = a.getInt(R::styleable::ScaleDrawable_scaleGravity, mState->mGravity);
    mState->mUseIntrinsicSizeAsMin = a.getBoolean(R::styleable::ScaleDrawable_useIntrinsicSizeAsMinimum, mState->mUseIntrinsicSizeAsMin);
    mState->mInitialLevel = a.getInt(R::styleable::ScaleDrawable_level, mState->mInitialLevel);
}

}
