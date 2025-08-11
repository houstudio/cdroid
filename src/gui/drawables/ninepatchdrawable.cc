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
#include <drawables/ninepatchdrawable.h>
#include <image-decoders/imagedecoder.h>
#include <drawables/ninepatch.h>
#include <fstream>
#include <cdlog.h>
using namespace Cairo;
namespace cdroid{
//https://github.com/soramimi/QtNinePatch/blob/master/NinePatch.cpp

NinePatchDrawable::NinePatchDrawable():NinePatchDrawable(std::make_shared<NinePatchState>()){
}

NinePatchDrawable::NinePatchDrawable(std::shared_ptr<NinePatchState>state){
    mNinePatchState = state;
    mAlpha = 255;
    mMutated = false;
    mFilterBitmap = false;
    mTintFilter = nullptr;
    mTargetDensity=160;
    mOutlineRadius=0.f;
    mPadding.setEmpty();
    computeBitmapSize();
}

NinePatchDrawable::NinePatchDrawable(Context*ctx,const std::string&resid):NinePatchDrawable(){
    mNinePatchState->setBitmap(ctx,resid);
    computeBitmapSize();
}

NinePatchDrawable::NinePatchDrawable(RefPtr<ImageSurface>bmp):NinePatchDrawable(){
    mNinePatchState->setBitmap(bmp);
    computeBitmapSize();
}

NinePatchDrawable::~NinePatchDrawable(){
    delete mTintFilter;
}

void NinePatchDrawable::computeBitmapSize(){
    mPadding.setEmpty();
    if ( (mNinePatchState->mNinePatch==nullptr)|| (mNinePatchState->mNinePatch->mImage==nullptr))return;
    const RefPtr<ImageSurface> ninePatch = mNinePatchState->mNinePatch->mImage;

    const int sourceDensity =160;// ninePatch.getDensity();
    const int targetDensity = mTargetDensity;

    const Insets sourceOpticalInsets = mNinePatchState->mOpticalInsets;
    if (sourceOpticalInsets != Insets::NONE) {
        const int left  = Drawable::scaleFromDensity( sourceOpticalInsets.left   , sourceDensity, targetDensity, true);
        const int top   = Drawable::scaleFromDensity( sourceOpticalInsets.top    , sourceDensity, targetDensity, true);
        const int right = Drawable::scaleFromDensity( sourceOpticalInsets.right  , sourceDensity, targetDensity, true);
        const int bottom= Drawable::scaleFromDensity( sourceOpticalInsets.bottom , sourceDensity, targetDensity, true);
        mOpticalInsets = Insets::of(left, top, right, bottom);
    } else {
        mOpticalInsets = Insets::NONE;
    }

    const Rect sourcePadding = mNinePatchState->mPadding;
    if (!sourcePadding.empty()){
        mPadding.left  = Drawable::scaleFromDensity( sourcePadding.left  , sourceDensity, targetDensity, false);
        mPadding.top   = Drawable::scaleFromDensity( sourcePadding.top   , sourceDensity, targetDensity, false);
        mPadding.width = Drawable::scaleFromDensity( sourcePadding.width , sourceDensity, targetDensity, false);
        mPadding.height= Drawable::scaleFromDensity( sourcePadding.height, sourceDensity, targetDensity, false);
    }

    mBitmapHeight= Drawable::scaleFromDensity( ninePatch->get_height(), sourceDensity, targetDensity, true);
    mBitmapWidth = Drawable::scaleFromDensity( ninePatch->get_width() , sourceDensity, targetDensity, true);

    /*const NinePatch.InsetStruct insets = ninePatch.getBitmap().getNinePatchInsets();
    if (insets != null) {
        Rect outlineRect = insets.outlineRect;
        mOutlineInsets = NinePatch.InsetStruct.scaleInsets(outlineRect.left, outlineRect.top,
                    outlineRect.right, outlineRect.bottom, targetDensity / (float) sourceDensity);
        mOutlineRadius = Drawable::scaleFromDensity(insets.outlineRadius, sourceDensity, targetDensity);
    } else {
        mOutlineInsets = null;
    }*/
}

void NinePatchDrawable::setTargetDensity(int density){
    if (density == 0) {
        density =DisplayMetrics::DENSITY_DEFAULT;
    }
    if (mTargetDensity != density) {
        mTargetDensity = density;
        computeBitmapSize();
        invalidateSelf();
    }
}

Insets NinePatchDrawable::getOpticalInsets(){
    Insets&opticalInsets = mOpticalInsets; 
    if (needsMirroring()) {
        return Insets::of(opticalInsets.right, opticalInsets.top,
                opticalInsets.left, opticalInsets.bottom);
    } else {
        return opticalInsets;
    }
}

void NinePatchDrawable::setAlpha(int alpha) {
    if(mAlpha!=alpha){
        mAlpha = alpha;
        invalidateSelf();
    }
}

bool NinePatchDrawable::getPadding(Rect& padding){
    padding = mPadding;
    return (padding.left | padding.top | padding.width | padding.height) != 0;
}

void NinePatchDrawable::getOutline(Outline& outline) {
    const Rect bounds = getBounds();
    if (bounds.empty()) {
        return;
    }
    // Improve getOutline implementation to mimic Android NinePatchDrawable behavior:
    // 1. Prefer NinePatch's outlineRect/outlineRadius if available
    // 2. Otherwise, fallback to opticalInsets
    // 3. If neither is available, fallback to Drawable::getOutline

    // 1. Prefer NinePatch outlineRect/outlineRadius
    if (mNinePatchState && mNinePatchState->mNinePatch) {
        Rect outlineRect = mNinePatchState->mNinePatch->getOutlineRect();
        float outlineRadius = mNinePatchState->mNinePatch->getOutlineRadius();
        if (!outlineRect.empty()) {
            //must be scaled by density
            const int sourceDensity = 160;
            const int targetDensity = mTargetDensity ? mTargetDensity : 160;
            Rect scaledOutlineRect;
            scaledOutlineRect.left   = Drawable::scaleFromDensity(outlineRect.left,   sourceDensity, targetDensity, false);
            scaledOutlineRect.top    = Drawable::scaleFromDensity(outlineRect.top,    sourceDensity, targetDensity, false);
            scaledOutlineRect.width  = Drawable::scaleFromDensity(outlineRect.width,  sourceDensity, targetDensity, false);
            scaledOutlineRect.height = Drawable::scaleFromDensity(outlineRect.height, sourceDensity, targetDensity, false);
            const float scaledRadius = Drawable::scaleFromDensity(outlineRadius, sourceDensity, targetDensity, true);

            outline.setRoundRect(
                bounds.left + scaledOutlineRect.left,
                bounds.top + scaledOutlineRect.top,
                bounds.width-scaledOutlineRect.left-scaledOutlineRect.width,
                bounds.height-scaledOutlineRect.top-scaledOutlineRect.height,
                scaledRadius
            );
            outline.setAlpha(getAlpha() / 255.0f);
            return;
        }
    }

    // 2. Otherwise, fallback to opticalInsets
    if (mNinePatchState != nullptr) {
        Insets insets = mNinePatchState->mOpticalInsets;
        if (insets != Insets::NONE) {
            outline.setRoundRect(
                bounds.left + insets.left,
                bounds.top + insets.top,
                bounds.width - insets.left - insets.right,
                bounds.height - insets.top - insets.bottom,
                mOutlineRadius
            );
            outline.setAlpha(getAlpha() / 255.0f);
            return;
        }
    }

    // 3. If neither is available, fallback to Drawable::getOutline
    Drawable::getOutline(outline);
    if (mNinePatchState != nullptr/*&& mOutlineInsets != nullptr*/) {
        //NinePatch.InsetStruct insets =mNinePatchState.mNinePatch.getBitmap().getNinePatchInsets();
        Insets insets = mNinePatchState->mOpticalInsets;
        if (insets!=Insets::NONE) {
            outline.setRoundRect(bounds.left + insets.left,//mOutlineInsets.left,
                    bounds.top + insets.top,//mOutlineInsets.top,
                    bounds.width - insets.right,//mOutlineInsets.right,
                    bounds.height - insets.bottom,//mOutlineInsets.bottom,
                    mOutlineRadius);
            outline.setAlpha(/*insets.outlineAlpha */ (getAlpha() / 255.0f));
            return;
        }
    }
}

int NinePatchDrawable::getAlpha()const{
    return mAlpha;
}

void NinePatchDrawable::setTintList(const ColorStateList* tint){
    if( mNinePatchState->mTint!=tint ){
        mNinePatchState->mTint = tint;
        mTintFilter = updateTintFilter(mTintFilter, tint, mNinePatchState->mTintMode);
        invalidateSelf();
    }
}

void NinePatchDrawable::setTintMode(int tintMode) {
    mNinePatchState->mTintMode = tintMode;
    mTintFilter = updateTintFilter(mTintFilter, mNinePatchState->mTint, tintMode);
    invalidateSelf();
}

void NinePatchDrawable::setAutoMirrored(bool mirrored) {
    mNinePatchState->mAutoMirrored = mirrored;
}

bool NinePatchDrawable::needsMirroring() {
    return isAutoMirrored() && (getLayoutDirection() == LayoutDirection::RTL);
}

bool NinePatchDrawable::isAutoMirrored()const{
    return mNinePatchState->mAutoMirrored;
}

void NinePatchDrawable::setFilterBitmap(bool filter){
    mFilterBitmap = filter;
}

bool NinePatchDrawable::isFilterBitmap()const{
    return mFilterBitmap;
}

int NinePatchDrawable::getIntrinsicWidth() {
    return mBitmapWidth;
}

int NinePatchDrawable::getIntrinsicHeight() {
    return mBitmapHeight;
}

NinePatchDrawable* NinePatchDrawable::mutate() {
    if (!mMutated && Drawable::mutate() == this) {
        mNinePatchState=std::make_shared<NinePatchState>(*mNinePatchState);
        mMutated = true;
    }
    return this;
}

bool NinePatchDrawable::onStateChange(const std::vector<int>& stateSet){
    if (mNinePatchState->mTint && mNinePatchState->mTintMode != PorterDuff::Mode::NOOP) {
        mTintFilter = updateTintFilter(mTintFilter, mNinePatchState->mTint, mNinePatchState->mTintMode);
        return true;
    }
    return false;
}

bool NinePatchDrawable::isStateful()const{
    return Drawable::isStateful() || (mNinePatchState->mTint && mNinePatchState->mTint->isStateful());
}

bool NinePatchDrawable::hasFocusStateSpecified()const {
    return mNinePatchState->mTint && mNinePatchState->mTint->hasFocusStateSpecified();
}

std::shared_ptr<Drawable::ConstantState>NinePatchDrawable::getConstantState(){
    return mNinePatchState;
}

void NinePatchDrawable::draw(Canvas&canvas){
    if(mNinePatchState->mNinePatch){
        canvas.save();
        if(mTintFilter){
            canvas.rectangle(mBounds.left,mBounds.top,mBounds.width,mBounds.height);
            canvas.clip();
            canvas.push_group();
        }
        if(needsMirroring()){
            const float cx=mBounds.left+mBounds.width/2.f;
            const float cy=mBounds.left+mBounds.height/2.f;
            canvas.scale(-1.f,1.f);
            canvas.translate(cx,cy);
        }
        mNinePatchState->draw(canvas,mBounds,mAlpha);
        if(mTintFilter){
            mTintFilter->apply(canvas,mBounds);
            canvas.pop_group_to_source();
            canvas.paint();
        }
        canvas.restore();
    }
}

void NinePatchDrawable::inflate(XmlPullParser&parser,const AttributeSet&atts){
   Drawable::inflate(parser,atts);
   updateStateFromTypedArray(atts);
   computeBitmapSize();
}

void NinePatchDrawable::updateStateFromTypedArray(const AttributeSet&a){
    auto state = mNinePatchState;

    // Account for any configuration changes.
    //state->mChangingConfigurations |= a.getChangingConfigurations();

    // Extract the theme attributes, if any.
    //state.mThemeAttrs = a.extractThemeAttrs();

    state->mDither = a.getBoolean("dither", state->mDither);

    const std::string srcResId = a.getString("src");
    if (!srcResId.empty()) {
        Rect padding ,opticalInsets;
        Cairo::RefPtr<Cairo::ImageSurface> bitmap;
        auto is= a.getContext()->getInputStream(srcResId);
        bitmap = ImageDecoder::loadImage(*is,-1,-1);
        if (bitmap == nullptr) {
            throw std::logic_error(//a.getPositionDescription() +
                    ": <nine-patch> requires a valid src attribute");
        } else {//if (bitmap.getNinePatchChunk() == null) {
            state->mNinePatch = std::make_shared<NinePatch>(bitmap);
            state->mPadding = state->mNinePatch->getPadding();
            mOutlineRadius = state->mNinePatch->getRadius();
            if(state->mPadding.empty()){
                throw std::logic_error(//a.getPositionDescription() +
                     ": <nine-patch> requires a valid 9-patch source image");
            }
        }
        opticalInsets = state->mNinePatch->getOpticalInsets();
        state->mOpticalInsets = Insets::of(opticalInsets);
    }

    state->mAutoMirrored = a.getBoolean("autoMirrored", state->mAutoMirrored);
    state->mBaseAlpha = a.getFloat("alpha", state->mBaseAlpha);

    const int tintMode = a.getInt("tintMode", -1);
    if (tintMode != -1) {
        //state->mTintMode = Drawable::parseTintMode(tintMode, Mode.SRC_IN);
    }

    ColorStateList* tint = a.getColorStateList("tint");
    if (tint != nullptr) {
        state->mTint = tint;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////

NinePatchDrawable::NinePatchState::NinePatchState(){
    mTint = nullptr;
    mBaseAlpha=1.0f;
    mDither = true;
    mTint = nullptr;
    mTintMode = DEFAULT_TINT_MODE;
    mChangingConfigurations = 0;
    mAutoMirrored =false;
    mPadding.set(0,0,0,0);
    mOpticalInsets.set(0,0,0,0);
}

void NinePatchDrawable::NinePatchState::setBitmap(Context*ctx,const std::string&resid,const Rect*padding){
    auto bitmap = ctx->loadImage(resid,-1,-1);
    setBitmap(bitmap,padding);
}

void NinePatchDrawable::NinePatchState::setBitmap(RefPtr<ImageSurface>bitmap,const Rect*padding){
    if(bitmap){
        mNinePatch = RefPtr<NinePatch>(new NinePatch(bitmap));
        mPadding = mNinePatch->getPadding();
    }
    if(padding)mPadding=*padding;
    LOGV("ninpatch %p size=%dx%d padding=(%d,%d,%d,%d)",this,bitmap->get_width(),bitmap->get_height(),
        mPadding.left,mPadding.top,mPadding.width,mPadding.height);
}

NinePatchDrawable::NinePatchState::NinePatchState(const NinePatchState&orig){
    mTint = orig.mTint;
    mNinePatch= orig.mNinePatch;
    mTintMode = orig.mTintMode;
    mPadding = orig.mPadding;
    mOpticalInsets = orig.mOpticalInsets;
    mBaseAlpha = orig.mBaseAlpha;
    mDither = orig.mDither;
    mChangingConfigurations=orig.mChangingConfigurations;
    mAutoMirrored = orig.mAutoMirrored;
    mNinePatch = orig.mNinePatch;
    //mThemeAttrs = orig.mThemeAttrs;
}

NinePatchDrawable*NinePatchDrawable::NinePatchState::newDrawable(){
    return new NinePatchDrawable(shared_from_this());
}

int NinePatchDrawable::NinePatchState::getChangingConfigurations()const{
   return mChangingConfigurations|(mTint ? mTint->getChangingConfigurations() : 0);
}

void NinePatchDrawable::NinePatchState::draw(Canvas&canvas,const Rect&rect,int alpha){
    mNinePatch->setImageSize(rect.width, rect.height);
    mNinePatch->draw(canvas,rect.left,rect.top,float(alpha)/255.f);
}

}

