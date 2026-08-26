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
#include <drawable/ninepatchdrawable.h>
#include <drawable/ninepatchrenderer.h>
#include <image-decoders/imagedecoder.h>
#include <content/asset.h>
#include <widget/framework_styleable.h>
#include <content/typedvalue.h>
#include <utils/textutils.h>
#include <porting/cdlog.h>
#include <fstream>
using namespace Cairo;
namespace cdroid{
using namespace cdroid::internal;
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

NinePatchDrawable::NinePatchDrawable(RefPtr<ImageSurface>bmp,const std::vector<uint8_t>*ninePatchChunk):NinePatchDrawable(){
    mNinePatchState->setBitmap(bmp,nullptr,ninePatchChunk);
    computeBitmapSize();
}

NinePatchDrawable::~NinePatchDrawable(){
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

    mOutlineRadius = mNinePatchState->mNinePatch->getRadius();
    // Outline rect/radius/alpha come from the NinePatchRenderer (scanner-faithful on the
    // border-scan path; chunk path falls back). CDROID's npTc chunk carries no outline
    // fields, so the Android Bitmap.getNinePatchInsets()/InsetStruct path can't apply.
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
            outline.setAlpha(mNinePatchState->mNinePatch->getOutlineAlpha() / 255.0f);
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

    // 3. If neither is available, fallback to Drawable::getOutline + opticalInsets.
    Drawable::getOutline(outline);
    if (mNinePatchState != nullptr) {
        const Insets insets = mNinePatchState->mOpticalInsets;
        if (insets!=Insets::NONE) {
            // Subtract BOTH horizontal insets (the second fallback does; this
            // one omitted insets.left, leaving the outline too wide).
            outline.setRoundRect(bounds.left + insets.left,
                    bounds.top + insets.top,
                    bounds.width - insets.left - insets.right,
                    bounds.height - insets.top - insets.bottom,
                    mOutlineRadius);
            outline.setAlpha(getAlpha() / 255.0f);
            return;
        }
    }
}

int NinePatchDrawable::getAlpha()const{
    return mAlpha;
}

void NinePatchDrawable::setTintList(const RefPtr<ColorStateList>& tint){
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
        ColorFilter* tintFilter = beginTintGroup(canvas, mBounds, mTintFilter.get());
        if(needsMirroring()){
            // AOSP: mirror about the bounds center — canvas.scale(-1, 1, cx, cy).
            // The old code used mBounds.left for the Y center and a bare
            // scale(-1,1)+translate, which mirrors about the wrong pivot and
            // shifts the patch by cy.
            const float cx = mBounds.left+mBounds.width/2.f;
            const float cy = mBounds.top+mBounds.height/2.f;
            canvas.translate(cx,cy);
            canvas.scale(-1.f,1.f);
            canvas.translate(-cx,-cy);
        }
        mNinePatchState->draw(canvas,mBounds,mAlpha);
        if(tintFilter) endTintGroup(canvas, mBounds, tintFilter);
        canvas.restore();
    }
}

// AOSP NinePatchDrawable.canApplyTheme/applyTheme.
bool NinePatchDrawable::canApplyTheme(){
    return (mNinePatchState && !mNinePatchState->mThemeAttrs.empty()) || Drawable::canApplyTheme();
}

void NinePatchDrawable::applyTheme(const Resources::Theme& t){
    Drawable::applyTheme(t);
    if (mNinePatchState && !mNinePatchState->mThemeAttrs.empty()) {
        auto a = t.resolveAttributes(mNinePatchState->mThemeAttrs, R::styleable::NinePatchDrawable);
        if (a) updateStateFromTypedArray(*a);
        mNinePatchState->mThemeAttrs.clear();
    }
    computeBitmapSize();
}

void NinePatchDrawable::inflate(Resources&r,XmlPullParser&parser,const AttributeSet&atts,const Resources::Theme* theme){
   Drawable::inflate(r,parser,atts, theme);
   // AOSP: all attr reads + src loading happen inside updateStateFromTypedArray.
   auto ta = obtainAttributes(r, theme, atts, R::styleable::NinePatchDrawable);
   if (ta) updateStateFromTypedArray(*ta);
   computeBitmapSize();
}

void NinePatchDrawable::updateStateFromTypedArray(const TypedArray& a){
    auto state = mNinePatchState;
    Resources& r = const_cast<Resources&>(a.getResources());
    // AOSP: extract the theme attributes for later re-resolution (applyTheme).
    state->mThemeAttrs = a.extractThemeAttrs();

    state->mDither = a.getBoolean(R::styleable::NinePatchDrawable_dither, state->mDither);

    // AOSP: src loading inside updateStateFromTypedArray (only param is TypedArray).
    const int srcResId = a.getResourceId(R::styleable::NinePatchDrawable_src, 0);
    if (srcResId != 0) {
        TypedValue tv;
        Asset* asset = r.openRawResource(srcResId, &tv);
        if (asset) {
            // Density from the TypedValue (AOSP: value.density → display density).
            int density = DisplayMetrics::DENSITY_DEFAULT;
            if (tv.density == TypedValue::DENSITY_DEFAULT) {
                density = DisplayMetrics::DENSITY_DEFAULT;
            } else if (tv.density != TypedValue::DENSITY_NONE) {
                density = tv.density;
            }

            const off64_t sz = asset->getLength();
            if (sz > 0) {
                std::string buf((size_t)sz, '\0');
                asset->read(&buf[0], (size_t)sz);
                std::vector<uint8_t> ninePatchChunk;
                auto stream = std::make_unique<std::istringstream>(std::move(buf));
                auto bitmap = ImageDecoder::loadImage(*stream, -1, -1, &ninePatchChunk);
                if (bitmap) {
                    const std::vector<uint8_t>* chunkPtr = ninePatchChunk.empty() ? nullptr : &ninePatchChunk;
                    try {
                        state->mNinePatch = std::make_shared<NinePatchRenderer>(bitmap, chunkPtr);
                    } catch (...) {
                        LOGW("<nine-patch> renderer threw");
                    }
                    if (state->mNinePatch) {
                        state->mPadding = state->mNinePatch->getPadding();
                        mOutlineRadius = state->mNinePatch->getRadius();
                        state->mOpticalInsets = state->mNinePatch->getOpticalInsets();
                    }
                } else {
                    LOGW("<nine-patch> src did not decode (0x%x)", srcResId);
                }
            }
            delete asset;
        }
    }

    state->mAutoMirrored = a.getBoolean(R::styleable::NinePatchDrawable_autoMirrored, state->mAutoMirrored);
    state->mBaseAlpha = a.getFloat(R::styleable::NinePatchDrawable_alpha, state->mBaseAlpha);

    /* AOSP: Drawable.parseTintMode maps the XML enum (src_over=3 ... multiply=14)
     * onto PorterDuff.Mode values. Storing the raw int left "multiply" (14) as an
     * unrelated operator and the tint painted the whole group rect. */
    const int tintMode = a.getInt(R::styleable::NinePatchDrawable_tintMode, PorterDuff::NOOP);
    if (tintMode != PorterDuff::NOOP) {
        state->mTintMode = (int)parseTintMode(tintMode, (PorterDuff::Mode)state->mTintMode);
    }

    auto tint = a.getColorStateList(R::styleable::NinePatchDrawable_tint);
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

void NinePatchDrawable::NinePatchState::setBitmap(RefPtr<ImageSurface>bitmap,const Rect*padding,
        const std::vector<uint8_t>*ninePatchChunk){
    if(bitmap){
        mNinePatch = RefPtr<NinePatchRenderer>(new NinePatchRenderer(bitmap,ninePatchChunk));
        mPadding   = mNinePatch->getPadding();
        mOpticalInsets = mNinePatch->getOpticalInsets();
    }
    if(padding)mPadding=*padding;
    LOGV("ninpatch %p size=%dx%d padding=(%d,%d,%d,%d) radius=%d opticalInsets=(%d,%d,%d,%d)",
            this,bitmap->get_width(),bitmap->get_height(),
            mPadding.left,mPadding.top,mPadding.width,mPadding.height,mNinePatch->getRadius(),
            mOpticalInsets.left,mOpticalInsets.top,mOpticalInsets.right,mOpticalInsets.bottom);
}

NinePatchDrawable::NinePatchState::NinePatchState(const NinePatchState&orig){
    mThemeAttrs = orig.mThemeAttrs;   // AOSP keeps them; dropping lost ?attr re-resolution
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

