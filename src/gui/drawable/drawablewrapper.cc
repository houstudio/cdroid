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
#include <drawable/drawablewrapper.h>
#include <core/context.h>
#include <widget/framework_styleable.h>
#include <porting/cdlog.h>

namespace cdroid{
using namespace cdroid::internal;

DrawableWrapper::DrawableWrapperState::DrawableWrapperState(){
    mDensity = DisplayMetrics::DENSITY_DEFAULT;
    mDrawableState = nullptr;
    mSrcDensityOverride = 0;
    mChangingConfigurations =0;
}

DrawableWrapper::DrawableWrapperState::DrawableWrapperState(const DrawableWrapperState& orig){
    mThemeAttrs = orig.mThemeAttrs;
    mChangingConfigurations = orig.mChangingConfigurations;
    mDrawableState = orig.mDrawableState;
    mSrcDensityOverride = orig.mSrcDensityOverride;
    mDensity = orig.mDensity;
}

void DrawableWrapper::DrawableWrapperState::setDensity(int targetDensity){
    if (mDensity != targetDensity) {
       const int sourceDensity = mDensity;
       mDensity = targetDensity;
       onDensityChanged(sourceDensity, targetDensity);
    }
}

int DrawableWrapper::DrawableWrapperState::getChangingConfigurations()const{
    // AOSP: mChangingConfigurations | the wrapped child's (a constant 0
    // under-reported config changes for cached wrapper states).
    return mChangingConfigurations
        | (mDrawableState ? mDrawableState->getChangingConfigurations() : 0);
}

void DrawableWrapper::DrawableWrapperState::onDensityChanged(int sourceDensity, int targetDensity){
}

DrawableWrapper*DrawableWrapper::DrawableWrapperState::newDrawable(){
    // AOSP java:554: newDrawable(null) — dispatches to the (Resources) form.
    return (DrawableWrapper*)newDrawable(nullptr);
}

Drawable*DrawableWrapper::DrawableWrapperState::newDrawable(Resources* res){
    return new DrawableWrapper(shared_from_this(), res);
}

bool DrawableWrapper::DrawableWrapperState::canConstantState()const {
    return mDrawableState!=nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DrawableWrapper::DrawableWrapper(Drawable*dr){
    mState = nullptr;
    mDrawable= dr;
    mMutated = false;
    if (dr) dr->setCallback(this);  // androidx: ctor routes through setDrawable, which sets the wrapped drawable's callback to this wrapper
}

DrawableWrapper::DrawableWrapper(std::shared_ptr<DrawableWrapperState>state,Resources*res){
    mState = state;
    mDrawable= nullptr;
    mMutated = false;
    updateLocalState(res);
}

void DrawableWrapper::updateLocalState(Resources* res) {
    if (mState && mState->mDrawableState) {
        Drawable* dr = mState->mDrawableState->newDrawable(res);
        setDrawable(dr);
    }
}

DrawableWrapper::~DrawableWrapper(){
    if(mDrawable)
        mDrawable->setCallback(nullptr);
    delete mDrawable;
    mDrawable = nullptr;
}

void DrawableWrapper::setDrawable(Drawable*dr){
    if (mDrawable != nullptr)
        mDrawable->setCallback(nullptr);
    delete mDrawable;
    mDrawable = dr;
    if(dr){
        dr->setCallback(this);
        dr->setVisible(isVisible(), true);
        dr->setState(getState());
        dr->setLevel(getLevel());
        dr->setBounds(getBounds());
        dr->setLayoutDirection(getLayoutDirection());
        if(mState)
            mState->mDrawableState = dr->getConstantState(); 
    }
    invalidateSelf();
}

Drawable*DrawableWrapper::getDrawable()const{
    return mDrawable;
}

bool DrawableWrapper::isStateful()const{
    return mDrawable && mDrawable->isStateful();
}

bool DrawableWrapper::hasFocusStateSpecified()const{
    return mDrawable && mDrawable->hasFocusStateSpecified();
}

bool DrawableWrapper::onStateChange(const std::vector<int>& state) {
    if (mDrawable && mDrawable->isStateful()) {
        bool changed = mDrawable->setState(state);
        if (changed)  onBoundsChange(getBounds());
        return changed;
    }
    return false;
}

void DrawableWrapper::jumpToCurrentState() {
    if (mDrawable != nullptr) {
        mDrawable->jumpToCurrentState();
    }
}

bool DrawableWrapper::onLevelChange(int level) {
    return mDrawable && mDrawable->setLevel(level);
}

void DrawableWrapper::onBoundsChange(const Rect& bounds) {
    if (mDrawable ) {
        mDrawable->setBounds(bounds);
    }
}

int DrawableWrapper::getIntrinsicWidth() {
    return mDrawable != nullptr ? mDrawable->getIntrinsicWidth() : -1;
}

int DrawableWrapper::getIntrinsicHeight() {
    return mDrawable != nullptr ? mDrawable->getIntrinsicHeight() : -1;
}

void DrawableWrapper::getOutline(Outline& outline) {
    if (mDrawable != nullptr) {
        mDrawable->getOutline(outline);
    } else {
        Drawable::getOutline(outline);
    }
}

void DrawableWrapper::getHotspotBounds(Rect& outRect)const{
    if(mDrawable)mDrawable->getHotspotBounds(outRect);
    else outRect = mBounds;
}

// AOSP DrawableWrapper forwards opacity/hotspot/layout-direction to the
// wrapped drawable; without these a wrapped ripple anchored its feedback at
// 0,0 and wrapped opaque bitmaps reported UNKNOWN opacity.
int DrawableWrapper::getOpacity()const{
    return mDrawable ? mDrawable->getOpacity() : PixelFormat::TRANSLUCENT;
}

void DrawableWrapper::setHotspot(float x,float y){
    if(mDrawable)mDrawable->setHotspot(x,y);
}

void DrawableWrapper::setHotspotBounds(int left,int top,int width,int height){
    if(mDrawable)mDrawable->setHotspotBounds(left,top,width,height);
}

bool DrawableWrapper::onLayoutDirectionChanged(int layoutDirection){
    return mDrawable != nullptr && mDrawable->setLayoutDirection(layoutDirection);
}

std::shared_ptr<DrawableWrapper::DrawableWrapperState> DrawableWrapper::mutateConstantState(){
    // androidx DrawableWrapper.mutateConstantState returns mState as-is (no copy). The
    // DrawableWrapper(Drawable*) ctor leaves mState null (it assigns mDrawable directly instead
    // of going through setDrawable), so guard the copy to avoid derefing a null state.
    if (mState == nullptr) return nullptr;
    return std::make_shared<DrawableWrapperState>(*mState);
}

DrawableWrapper*DrawableWrapper::mutate(){
    if (!mMutated && Drawable::mutate() == this) {
        mState=mutateConstantState();
        if (mDrawable != nullptr) {
            mDrawable->mutate();
        }
        if (mState != nullptr) {
            // AOSP DrawableWrapper.mutate(): store the child's BASE ConstantState.
            // The dynamic cast to DrawableWrapperState returned null for every
            // non-wrapper child (bitmap/gradient/...), so a mutated wrapper lost
            // its constant state entirely (clones came back childless).
            mState->mDrawableState = (mDrawable != nullptr) ? mDrawable->getConstantState() : nullptr;
        }
        mMutated = true;
    }
    return this;
}

void DrawableWrapper::clearMutated(){
    Drawable::clearMutated();
    if (mDrawable)
        mDrawable->clearMutated();
    mMutated = false;
}

std::shared_ptr<Drawable::ConstantState>DrawableWrapper::getConstantState(){
    if(mState != nullptr && mState->canConstantState())
        return mState;
    return nullptr;
}

int DrawableWrapper::getChangingConfigurations()const{
    return Drawable::getChangingConfigurations()
                | (mState != nullptr ? mState->getChangingConfigurations() : 0)
                | (mDrawable ? mDrawable->getChangingConfigurations() : 0);
}

void DrawableWrapper::invalidateDrawable(Drawable& who){
    Drawable::Callback* callback = getCallback();
    if (callback != nullptr) {
        callback->invalidateDrawable(*this);
    }
}

void DrawableWrapper::scheduleDrawable(Drawable&who,const Runnable& what, int64_t when){
    Drawable::Callback* callback = getCallback();
    if (callback != nullptr) {
        callback->scheduleDrawable(*this, what, when);
    }
}

void DrawableWrapper::unscheduleDrawable(Drawable& who,const Runnable& what){
    Drawable::Callback* callback = getCallback();
    if (callback != nullptr) {
        callback->unscheduleDrawable(*this, what);
    }
}

bool DrawableWrapper::getPadding(Rect& padding){
    return mDrawable && mDrawable->getPadding(padding);
}

Insets DrawableWrapper::getOpticalInsets(){
    return mDrawable ? mDrawable->getOpticalInsets() : Insets();
}

bool DrawableWrapper::setVisible(bool visible, bool restart){
    const bool superChanged = Drawable::setVisible(visible, restart);
    const bool changed = mDrawable && mDrawable->setVisible(visible, restart);
    return superChanged | changed;
}


void DrawableWrapper::setAlpha(int alpha){
    if (mDrawable)mDrawable->setAlpha(alpha);
}

int DrawableWrapper::getAlpha()const{
    return mDrawable ? mDrawable->getAlpha() : 255;
}

void DrawableWrapper::setColorFilter(const cdroid::RefPtr<ColorFilter>&colorFilter){
    if(mDrawable)mDrawable->setColorFilter(colorFilter);
}

const cdroid::RefPtr<ColorFilter>DrawableWrapper::getColorFilter()const{
    Drawable*dr = getDrawable();
    if(dr)return dr->getColorFilter();
    return Drawable::getColorFilter();
}

void DrawableWrapper::setTintList(const RefPtr<ColorStateList>&tint){
    if(mDrawable)mDrawable->setTintList(tint);
}

void DrawableWrapper::setTintMode(int tintMode){
    if(mDrawable)mDrawable->setTintMode(tintMode);
}

void DrawableWrapper::draw(Canvas&canvas){
    if (mDrawable != nullptr) {
        mDrawable->draw(canvas);
    }
}

// AOSP DrawableWrapper.canApplyTheme/applyTheme: re-resolve the recorded
// ?attr ids and forward to the wrapped drawable.
bool DrawableWrapper::canApplyTheme(){
    Drawable* dr = getDrawable();
    return (mState && !mState->mThemeAttrs.empty()) || (dr && dr->canApplyTheme())
           || Drawable::canApplyTheme();
}

void DrawableWrapper::applyTheme(const Resources::Theme& t){
    Drawable::applyTheme(t);
    auto state = mState;
    if (state && !state->mThemeAttrs.empty()) {
        auto a = t.resolveAttributes(state->mThemeAttrs, R::styleable::DrawableWrapper);
        if (a) updateStateFromTypedArray(*a);
        state->mThemeAttrs.clear();
    }
    Drawable* dr = getDrawable();
    if (dr && dr->canApplyTheme()) {
        dr->mutate();
        dr->applyTheme(t);
        dr->clearMutated();
    }
}

void DrawableWrapper::inflate(Resources&r,XmlPullParser&parser,const AttributeSet&atts,const Resources::Theme* theme){
    Drawable::inflate(r,parser,atts, theme);
    auto state = mState;
    if (state == nullptr) {
        return;
    }
    // The density may have changed since the last update. This will
    // apply scaling to any existing constant state properties.
    //final int densityDpi = r.getDisplayMetrics().densityDpi;
    //final int targetDensity = densityDpi == 0 ? DisplayMetrics.DENSITY_DEFAULT : densityDpi;
    //state->setDensity(targetDensity);
    //state->mSrcDensityOverride = mSrcDensityOverride;

    // AOSP DrawableWrapper.inflate: obtainAttributes(R.styleable.DrawableWrapper).
    auto ta = obtainAttributes(r, theme, atts, R::styleable::DrawableWrapper);
    if (ta) updateStateFromTypedArray(*ta);
    inflateChildDrawable(r, parser, atts, theme);
}

void DrawableWrapper::updateStateFromTypedArray(const TypedArray& a) {
    auto state = mState;
    if (state == nullptr) {
        return;
    }

    // Account for any configuration changes.
    //state.mChangingConfigurations |= a.getChangingConfigurations();

    // Extract the theme attributes, if any. Java shadows two mThemeAttrs
    // fields (subclass state vs DrawableWrapperState); C++ has one, so only
    // overwrite when the WRAPPER-level styleable actually captured entries —
    // an unconditional assign wiped the subclass extracts done before
    // DrawableWrapper::inflate (clip/inset/scale/rotate ?attr re-resolution).
    auto wrapperThemeAttrs = a.extractThemeAttrs();
    if (!wrapperThemeAttrs.empty()) {
        state->mThemeAttrs = wrapperThemeAttrs;
    }
    if (a.hasValue(R::styleable::DrawableWrapper_drawable)) {
        setDrawable(a.getDrawable(R::styleable::DrawableWrapper_drawable));
    }
}

void DrawableWrapper::inflateChildDrawable(Resources& r,XmlPullParser& parser,const AttributeSet& attrs,const Resources::Theme* theme){
    // Seek to the first child element.
    Drawable* dr = nullptr;
    int type;
    // AOSP uses the wrapper's OWN depth (no +1): the loop must stop at the
    // wrapper's end tag, not at the first child's — with +1 a <rotate> holding
    // multiple children kept only the first and left the parser inside the
    // wrapper element, perturbing the parent inflate loop.
    const int outerDepth = parser.getDepth();
    while ((type = parser.next()) != XmlPullParser::END_DOCUMENT
            && (type != XmlPullParser::END_TAG || parser.getDepth() > outerDepth)) {
        if (type == XmlPullParser::START_TAG) {
            // AOSP inflateChildElements: the child inflates under the state's
            // source-density override (0 = none) for getDrawableForDensity loads.
            dr = Drawable::createFromXmlInnerForDensity(r,parser,attrs,mState->mSrcDensityOverride,theme);
        }
    }

    if (dr != nullptr)  setDrawable(dr);
}

}
