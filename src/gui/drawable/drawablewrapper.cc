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
#include <drawable/drawablewrapper.h>
#include <porting/cdlog.h>

namespace cdroid{

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
    return 0;
}

void DrawableWrapper::DrawableWrapperState::onDensityChanged(int sourceDensity, int targetDensity){
}

DrawableWrapper*DrawableWrapper::DrawableWrapperState::newDrawable(){
    return new DrawableWrapper(shared_from_this());
}

bool DrawableWrapper::DrawableWrapperState::canConstantState()const {
    return mDrawableState!=nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DrawableWrapper::DrawableWrapper(Drawable*dr){
    mState = nullptr;
    mDrawable= dr;
    mMutated = false;
}

DrawableWrapper::DrawableWrapper(std::shared_ptr<DrawableWrapperState>state){
    mState = state;
    mDrawable= nullptr;
    mMutated = false;
    updateLocalState();
}

void DrawableWrapper::updateLocalState() {
    if (mState && mState->mDrawableState) {
        Drawable* dr = mState->mDrawableState->newDrawable();
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

std::shared_ptr<DrawableWrapper::DrawableWrapperState> DrawableWrapper::mutateConstantState(){
    return std::make_shared<DrawableWrapperState>(*mState);
}

DrawableWrapper*DrawableWrapper::mutate(){
    if (!mMutated && Drawable::mutate() == this) {
        mState=mutateConstantState();
        if (mDrawable != nullptr) {
            mDrawable->mutate();
        }
        if (mState != nullptr) {
            mState->mDrawableState =std::dynamic_pointer_cast<DrawableWrapperState>(mDrawable != nullptr ? mDrawable->getConstantState() : nullptr);
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
                | (mDrawable&&mDrawable->getChangingConfigurations());
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

void DrawableWrapper::setColorFilter(ColorFilter*colorFilter){
    if(mDrawable)mDrawable->setColorFilter(colorFilter);
}

ColorFilter*DrawableWrapper::getColorFilter(){
    Drawable*dr = getDrawable();
    if(dr)return dr->getColorFilter();
    return Drawable::getColorFilter();
}

void DrawableWrapper::setTintList(const ColorStateList*tint){
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

void DrawableWrapper::inflate(XmlPullParser&parser,const AttributeSet&atts){
    Drawable::inflate(parser,atts);
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

    updateStateFromTypedArray(atts);
    inflateChildDrawable(parser, atts);
}

void DrawableWrapper::updateStateFromTypedArray(const AttributeSet&atts) {
    auto state = mState;
    if (state == nullptr) {
        return;
    }

    // Account for any configuration changes.
    //state.mChangingConfigurations |= a.getChangingConfigurations();

    // Extract the theme attributes, if any.
    //state.mThemeAttrs = a.extractThemeAttrs();
    if (atts.hasAttribute("drawable")) {
        setDrawable(atts.getDrawable("drawable"));
    }
}

void DrawableWrapper::inflateChildDrawable(XmlPullParser& parser,const AttributeSet& attrs){
    // Seek to the first child element.
    Drawable* dr = nullptr;
    int type;
    const int outerDepth = parser.getDepth()+1;
    while ((type = parser.next()) != XmlPullParser::END_DOCUMENT
            && (type != XmlPullParser::END_TAG || parser.getDepth() > outerDepth)) {
        if (type == XmlPullParser::START_TAG) {
            dr = Drawable::createFromXmlInnerForDensity(parser,attrs,0/*mState->mSrcDensityOverride*/);
        }
    }

    if (dr != nullptr)  setDrawable(dr);
}

}
