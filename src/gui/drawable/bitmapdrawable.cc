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
#include <drawable/bitmapdrawable.h>
#include <image-decoders/imagedecoder.h>
#include <content/typedarray.h>
#include <core/context.h>
#include <content/typedvalue.h>
#include <content/asset.h>
#include <text/textutils.h>
#include <widget/framework_styleable.h>
#include <fstream>
#include <app.h>
#include <cdlog.h>

using namespace Cairo;
namespace cdroid{
using namespace cdroid::internal;

BitmapDrawable::BitmapState::BitmapState(){
    mGravity  = Gravity::FILL;
    mBaseAlpha= 1.0f;
    mAlpha = 255;
    mTransparency = -1;
    mTintMode     = DEFAULT_TINT_MODE;
    mTileModeX = mTileModeY = -1;
    mAutoMirrored = false;
    mFilterBitmap = false;
    mMipMap = false;
    mDither = false;
    mAntiAlias = false;
    mSrcDensityOverride = 0;
    mTargetDensity = 160;
    mBitmapDensity = 0;   // unknown until the decode side reports it
    mChangingConfigurations=0;
}

BitmapDrawable::BitmapState::BitmapState(RefPtr<ImageSurface>bitmap)
    :BitmapState(){
    mBitmap = bitmap;
    mTransparency = ImageDecoder::getTransparency(bitmap);
}

BitmapDrawable::BitmapState::BitmapState(const BitmapState&bitmapState){
    mBitmap = bitmapState.mBitmap;
    mTint   = bitmapState.mTint;
    mTintMode   = bitmapState.mTintMode;
    mThemeAttrs = bitmapState.mThemeAttrs;
    mChangingConfigurations = bitmapState.mChangingConfigurations;
    mGravity = bitmapState.mGravity;
    mTransparency= bitmapState.mTransparency;
    mTileModeX = bitmapState.mTileModeX;
    mTileModeY = bitmapState.mTileModeY;
    mSrcDensityOverride = bitmapState.mSrcDensityOverride;
    mTargetDensity = bitmapState.mTargetDensity;
    mBitmapDensity = bitmapState.mBitmapDensity;
    mBaseAlpha = bitmapState.mBaseAlpha;
    mAlpha = bitmapState.mAlpha;
    mDither= bitmapState.mDither;
    mAntiAlias= bitmapState.mAntiAlias;
    mMipMap= bitmapState.mMipMap;
    mFilterBitmap = bitmapState.mFilterBitmap;
    mTransparency = bitmapState.mTransparency;
    //mRebuildShader = bitmapState.mRebuildShader;
    mAutoMirrored = bitmapState.mAutoMirrored;
    mResource = bitmapState.mResource;
}

BitmapDrawable::BitmapState::~BitmapState(){
    mBitmap = nullptr;
    LOGV("%p %s",this,mResource.c_str());
}

BitmapDrawable* BitmapDrawable::BitmapState::newDrawable(){
    return new BitmapDrawable(shared_from_this());
}

int BitmapDrawable::BitmapState::getChangingConfigurations()const{
    return mChangingConfigurations |(mTint ? mTint->getChangingConfigurations() : 0);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
BitmapDrawable::BitmapDrawable():BitmapDrawable(std::make_shared<BitmapState>(nullptr)){
}

BitmapDrawable::BitmapDrawable(RefPtr<ImageSurface>img){
    mBitmapState = std::make_shared<BitmapState>(img);
    mDstRectAndInsetsDirty = true;
    mMutated = false;
    computeBitmapSize();
}

BitmapDrawable::BitmapDrawable(std::shared_ptr<BitmapState>state){
    mBitmapState = state;
    mDstRectAndInsetsDirty = true;
    mMutated = false;
    // AOSP: the One True Constructor ends in updateLocalState() — rebuild the
    // tint filter from the shared state, otherwise every newDrawable()/clone
    // (e.g. ProgressBar.tileify) loses the tint parsed at inflate time.
    mTintFilter = updateTintFilter(mTintFilter, mBitmapState->mTint, mBitmapState->mTintMode);
    computeBitmapSize();
}

BitmapDrawable::BitmapDrawable(Context*ctx,const std::string&resname)
  :BitmapDrawable(std::make_shared<BitmapState>()){
    RefPtr<ImageSurface>b;
    b = ImageDecoder::loadImage(ctx,resname);
    mBitmapState->mResource = resname;
    setBitmap(b);
#if defined(DEBUG) && ( defined(__x86_64__) || defined(__i386__) )
    const char*tNames[] = {"UNKNOWN","TRANSLUCENT","TRANSPARENT","OPAQUE"};
    DisplayMetrics dm = ctx->getDisplayMetrics();
    const size_t fullScreenSize=dm.widthPixels*dm.heightPixels;
    const size_t bitmapSize = b==nullptr?0:b->get_width()*b->get_height();
    LOGI_IF( ((mBitmapState->mTransparency!=PixelFormat::OPAQUE) && (bitmapSize>=fullScreenSize/4))
	   ||(b&&b->get_format()!=Cairo::Surface::Format::ARGB32),
        "is %-12s %d*%d*%d format=%d '%s' maby cause compose more slowly" , tNames[mBitmapState->mTransparency],
	b->get_width(),b->get_height(),b->get_stride()/b->get_width(),b->get_format(),mBitmapState->mResource.c_str());
#endif
}

BitmapDrawable::~BitmapDrawable(){
    LOGV("%p:%p use_count=%d %s",this,mBitmapState->mBitmap.get(),mBitmapState.use_count(),mBitmapState->mResource.c_str());
}

RefPtr<ImageSurface> BitmapDrawable::getBitmap()const{
    return mBitmapState->mBitmap;
}

void BitmapDrawable::setBitmap(RefPtr<ImageSurface>bmp){
    mBitmapState->mBitmap = bmp;
    mBitmapState->mTransparency = ImageDecoder::getTransparency(bmp);
    mDstRectAndInsetsDirty = true;
    computeBitmapSize();
    invalidateSelf();
}

int BitmapDrawable::getAlpha()const{
    return mBitmapState->mAlpha;
}

void BitmapDrawable::setAlpha(int alpha){
    // AOSP invalidates when the alpha actually changed; the bare store left
    // alpha changes unpainted until something else invalidated.
    const int oldAlpha = mBitmapState->mAlpha;
    mBitmapState->mAlpha = alpha&0xFF;
    if (mBitmapState->mAlpha != oldAlpha) {
        invalidateSelf();
    }
}

int BitmapDrawable::getGravity()const{
    return mBitmapState->mGravity;
}

void BitmapDrawable::setDither(bool dither){
    mBitmapState->mDither = dither;
    invalidateSelf();
}

int BitmapDrawable::getIntrinsicWidth() {
    return mBitmapWidth;
}

int BitmapDrawable::getIntrinsicHeight() {
    return mBitmapHeight;
}

void BitmapDrawable::setTargetDensity(int density) {
    if (mBitmapState->mTargetDensity != density) {
        mBitmapState->mTargetDensity = density;
        computeBitmapSize();
        invalidateSelf();
    }
}

void BitmapDrawable::setSourceDensity(int density) {
    if (mBitmapState->mBitmapDensity != density) {
        mBitmapState->mBitmapDensity = density;
        computeBitmapSize();
        invalidateSelf();
    }
}

int BitmapDrawable::getTileModeX()const{
    return mBitmapState->mTileModeX;
}

int BitmapDrawable::getTileModeY()const{
    return mBitmapState->mTileModeY;
}

void BitmapDrawable::setTileModeX(int mode){
    setTileModeXY(mode,mBitmapState->mTileModeY);
}

void BitmapDrawable::setTileModeY(int mode){
    setTileModeXY(mBitmapState->mTileModeX,mode);
}

void BitmapDrawable::setTileModeXY(int xmode,int ymode){
    if((mBitmapState->mTileModeX!=xmode)||(mBitmapState->mTileModeY!=ymode)){
        mBitmapState->mTileModeX=xmode;
        mBitmapState->mTileModeY=ymode;
        mDstRectAndInsetsDirty  =true;
        invalidateSelf();
    }
}

void BitmapDrawable::setAutoMirrored(bool mirrored){
    if(mBitmapState->mAutoMirrored!=mirrored){
        mBitmapState->mAutoMirrored=mirrored;
        invalidateSelf();
    }
}

bool BitmapDrawable::isAutoMirrored()const{
    return mBitmapState->mAutoMirrored;
}

int BitmapDrawable::getOpacity()const{
    if(mBitmapState->mGravity != Gravity::FILL)
        return PixelFormat::TRANSLUCENT;
    if(mBitmapState->mBitmap==nullptr)
        return PixelFormat::TRANSPARENT;

    return mBitmapState->mTransparency;
}

void BitmapDrawable::setTintList(const RefPtr<ColorStateList>&tint){
    if( mBitmapState->mTint!=tint ){
        mBitmapState->mTint = tint;
        mTintFilter = updateTintFilter(mTintFilter, tint, mBitmapState->mTintMode);
        invalidateSelf();
    }
}

void BitmapDrawable::setTintMode(int tintMode) {
    if (mBitmapState->mTintMode != tintMode) {
        mBitmapState->mTintMode = tintMode;
        mTintFilter = updateTintFilter(mTintFilter, mBitmapState->mTint, tintMode);
        invalidateSelf();
    }
}

int BitmapDrawable::getTintMode()const{
    return mBitmapState->mTintMode;
}

std::shared_ptr<Drawable::ConstantState>BitmapDrawable::getConstantState(){
    // androidx BitmapDrawable.getConstantState (line 974): OR the drawable's config into the state.
    mBitmapState->mChangingConfigurations |= getChangingConfigurations();
    return mBitmapState;
}

void BitmapDrawable::setGravity(int gravity){
    if(mBitmapState->mGravity!=gravity){
        mBitmapState->mGravity=gravity;
        mDstRectAndInsetsDirty=true;
        invalidateSelf();
    }
}

void BitmapDrawable::setMipMap(bool mipMap) {
    mBitmapState->mMipMap = mipMap;
    invalidateSelf();
}

bool BitmapDrawable::hasMipMap() const{
    return mBitmapState->mBitmap!=nullptr;  //&& mBitmapStatemBitmap.hasMipMap();
}

void BitmapDrawable::setAntiAlias(bool aa) {
    mBitmapState->mAntiAlias = aa;
    invalidateSelf();
}

bool BitmapDrawable::hasAntiAlias() const{
    return mBitmapState->mAntiAlias;
}

void BitmapDrawable::setFilterBitmap(bool filter) {
    if(mBitmapState->mFilterBitmap!=filter){
        mBitmapState->mFilterBitmap = filter;
        invalidateSelf();
    }
}

bool BitmapDrawable::isFilterBitmap() const{
    return mBitmapState->mFilterBitmap;
}

namespace {
/*android-36 Bitmap.scaleFromDensity verbatim (the intrinsics path): DENSITY_NONE
  on either side — cdroid stand-in 0 — or equal densities keep the size, else
  scale rounding up. Drawable::scaleFromDensity (the NinePatch path) rounds
  half-away instead; BitmapDrawable intrinsics use the Bitmap body.*/
int scaleFromBitmapDensity(int size, int sourceDensity, int targetDensity) {
    if (sourceDensity == 0 || targetDensity == 0 || sourceDensity == targetDensity)
        return size;
    return ((size * targetDensity) + (sourceDensity >> 1)) / sourceDensity;
}
}

void BitmapDrawable::computeBitmapSize() {
    if (mBitmapState->mBitmap != nullptr) {
        // AOSP: bitmap.getScaledWidth/Height(mTargetDensity). The surface has no
        // density metadata; mBitmapDensity is the Bitmap.mDensity stand-in
        // (0 = DENSITY_NONE/unknown -> raw pixels, so programmatic bitmaps and
        // density-matched resources keep their exact prior sizes).
        mBitmapWidth = scaleFromBitmapDensity(mBitmapState->mBitmap->get_width(),
                mBitmapState->mBitmapDensity, mBitmapState->mTargetDensity);
        mBitmapHeight = scaleFromBitmapDensity(mBitmapState->mBitmap->get_height(),
                mBitmapState->mBitmapDensity, mBitmapState->mTargetDensity);
    } else {
        mBitmapWidth = mBitmapHeight = -1;
    }
}

void BitmapDrawable::updateDstRectAndInsetsIfDirty(){
    if (mDstRectAndInsetsDirty) {
        if ((mBitmapState->mTileModeX == TileMode::DISABLED) && (mBitmapState->mTileModeY == TileMode::DISABLED)) {
            const int layoutDir = getLayoutDirection();
            mDstRect.set(0,0,0,0);
            Gravity::apply(mBitmapState->mGravity,mBitmapWidth,mBitmapHeight,mBounds, mDstRect, layoutDir);
            const int left  = mDstRect.left - mBounds.left;
            const int top   = mDstRect.top - mBounds.top;
            const int right = mBounds.right() - mDstRect.right();
            const int bottom= mBounds.bottom()- mDstRect.bottom();
            mOpticalInsets.set(left, top, right, bottom);
        } else {
            mDstRect = getBounds();
            mOpticalInsets.set(0,0,0,0);// = Insets.NONE;
        }
    }
    mDstRectAndInsetsDirty = false;
}

bool BitmapDrawable::needMirroring(){
    return isAutoMirrored()&&getLayoutDirection()==LayoutDirection::RTL;
}

void BitmapDrawable::onBoundsChange(const Rect&r){
    mDstRectAndInsetsDirty = true;
}

bool BitmapDrawable::onStateChange(const std::vector<int>&stateSet){
    if (mBitmapState->mTint && (mBitmapState->mTintMode != PorterDuff::NOOP)) {
        mTintFilter = updateTintFilter(mTintFilter, mBitmapState->mTint, mBitmapState->mTintMode);
        return true;
    }
    return false;    
}

bool BitmapDrawable::isStateful() const{
    return (mBitmapState->mTint != nullptr && mBitmapState->mTint->isStateful())
            || Drawable::isStateful();
}

BitmapDrawable*BitmapDrawable::mutate(){
    if (!mMutated && Drawable::mutate() == this) {
        mBitmapState=std::make_shared<BitmapState>(*mBitmapState);
        mMutated = true;
    }
    return this;
}

void BitmapDrawable::clearMutated() {
    Drawable::clearMutated();
    mMutated = false;
}

static void setPatternByTileMode(RefPtr<SurfacePattern>pat,int tileMode){
    switch(tileMode){
    case TileMode::DISABLED:break;
    case TileMode::CLAMP : pat->set_extend(Pattern::Extend::PAD);     break;
    case TileMode::REPEAT: pat->set_extend(Pattern::Extend::REPEAT);  break;
    case TileMode::MIRROR: pat->set_extend(Pattern::Extend::REFLECT); break;
    }
}

static int getRotateAngle(Canvas&canvas){
    double xx, yx, xy, yy, x0, y0;
    Cairo::Matrix ctx = canvas.get_matrix();
    double radians = atan2(ctx.yy, ctx.xy);
    return int(radians*180.f/M_PI);
}

/* Source-space tinted copy of the bitmap, memoized in the state (AOSP applies
 * the color filter on the paint during the single draw pass; this bakes it
 * into a bitmap copy once per (bitmap, filter) pair instead of the per-draw
 * tint group's push_group + full-rect filter + composite-back passes).
 * Keyed by pointers: a tint/state change builds a new PorterDuffColorFilter
 * object, and a new bitmap/new state naturally misses. */
Cairo::RefPtr<Cairo::ImageSurface> BitmapDrawable::tintedBitmap(BitmapState& state,
        ColorFilter* filter) {
    if (state.mTintedCache && state.mTintedWith == filter
            && state.mTintedFrom == state.mBitmap.get()) {
        return state.mTintedCache;
    }
    Cairo::RefPtr<Cairo::ImageSurface> source = state.mBitmap;
    Cairo::RefPtr<Cairo::ImageSurface> copy = Cairo::ImageSurface::create(
            Cairo::Surface::Format::ARGB32, source->get_width(), source->get_height());
    {
        Cairo::RefPtr<Cairo::Context> cc = Cairo::Context::create(copy);
        cc->set_source(source, 0, 0);
        cc->set_operator(Cairo::Context::Operator::SOURCE);
        cc->paint();
    }
    Canvas filterCanvas(copy);
    Rect r;   // (l,t,w,h); apply() paints the whole target anyway
    r.set(0, 0, copy->get_width(), copy->get_height());
    filter->apply(filterCanvas, r);

    state.mTintedCache = copy;
    state.mTintedWith = filter;
    state.mTintedFrom = state.mBitmap.get();
    return copy;
}

void BitmapDrawable::draw(Canvas&canvas){
    if(mBitmapState->mBitmap==nullptr) return;
    updateDstRectAndInsetsIfDirty();
    LOGV("BitmapSize=%dx%d bounds=%d,%d-%d,%d dst=%d,%d-%d,%d alpha=%d mColorFilter=%p",mBitmapWidth,mBitmapHeight,
            mBounds.left,mBounds.top,mBounds.width,mBounds.height, mDstRect.left,mDstRect.top,
	    mDstRect.width,mDstRect.height,mBitmapState->mAlpha,mTintFilter);

    LOGD_IF(mBounds.empty(),"%p's(%d,%d) bounds is empty,skip drawing,otherwise will caused crash",this,mBitmapWidth,mBitmapHeight);
    if(mBounds.empty())return;

    canvas.save();
    // mColorFilter beats tint (beginTintGroup's rule); with a filter in
    // effect the bitmap is swapped for its memoized tinted copy and the
    // whole draw runs untinted-path (AOSP: the filter rides the paint).
    ColorFilter* tintFilter = mColorFilter ? (ColorFilter*) mColorFilter.get()
                                           : mTintFilter.get();
    Cairo::RefPtr<Cairo::ImageSurface> source = mBitmapState->mBitmap;
    if (tintFilter) source = tintedBitmap(*mBitmapState, tintFilter);
    const int angle_degrees = getRotateAngle(canvas);
    const SurfacePattern::Filter filterMode = (mBitmapState->mFilterBitmap)||(angle_degrees%90)
                    ? SurfacePattern::Filter::BILINEAR : SurfacePattern::Filter::NEAREST;
    //SurfacePattern::Filter::GOOD : SurfacePattern::Filter::FAST;GOOD/FAST seems more slowly than ,BILINEAR/NEAREST
    const Pattern::Dither ditherMode = mBitmapState->mDither ? Pattern::Dither::GOOD : Pattern::Dither::DEFAULT;

    if((filterMode==SurfacePattern::Filter::NEAREST)||(mBitmapState->mAntiAlias==false))
        canvas.set_antialias(Cairo::ANTIALIAS_NONE);
    else
        canvas.set_antialias(Cairo::ANTIALIAS_DEFAULT);

    if((mBitmapState->mTileModeX>=0)||(mBitmapState->mTileModeY>=0)){
        RefPtr<SurfacePattern> pat =SurfacePattern::create(source);
        if(mBitmapState->mTileModeX!=TileMode::DISABLED){
            RefPtr<Surface> subs = ImageSurface::create(Surface::Format::ARGB32,mBounds.width,mBitmapHeight);
            RefPtr<Cairo::Context> subcanvas = Cairo::Context::create(subs);
            subcanvas->rectangle(0,0,mBounds.width,mBitmapHeight);
            setPatternByTileMode(pat,mBitmapState->mTileModeX);
            subcanvas->set_source(pat);
            subcanvas->fill();

            RefPtr<SurfacePattern>pats= SurfacePattern::create(subs);
            canvas.set_source(pats);
            if( (mBounds.height>mBitmapHeight) && (mBitmapState->mTileModeY==TileMode::DISABLED) )
                 setPatternByTileMode(pats,TileMode::CLAMP);
            else setPatternByTileMode(pats,mBitmapState->mTileModeY);
            canvas.rectangle(mBounds.left,mBounds.top,mBounds.width,mBounds.height);
            canvas.fill();
        }else{
            RefPtr<Surface> subs = ImageSurface::create(Surface::Format::ARGB32,mBitmapWidth,mBounds.height);
            RefPtr<Cairo::Context> subcanvas = Cairo::Context::create(subs);
           
            subcanvas->rectangle(0,0,mBitmapWidth,mBounds.height);
            setPatternByTileMode(pat,mBitmapState->mTileModeY);
            subcanvas->set_source(pat);
            subcanvas->fill();

            RefPtr<SurfacePattern>pats = SurfacePattern::create(subs); 
            canvas.set_source(pats);
            if( (mBounds.width>mBitmapWidth) && (mBitmapState->mTileModeX==TileMode::DISABLED))
                setPatternByTileMode(pats,TileMode::CLAMP);
            else setPatternByTileMode(pats,mBitmapState->mTileModeX);
            canvas.rectangle(mBounds.left,mBounds.top,mBounds.width,mBounds.height);
            canvas.fill();
        } 
    }else {
        // AOSP draw(): canvas.drawBitmap(bitmap, null, mDstRect, paint) — the
        // destination is the gravity-applied mDstRect (computed in
        // updateDstRectAndInsetsIfDirty), not the full bounds. Stretching to
        // the bounds ignored every non-FILL gravity (center etc.).
        const float sw = float(mBitmapWidth), sh = float(mBitmapHeight);
        const float fx = float(mDstRect.width) / sw, fy = float(mDstRect.height) / sh;
        const float alpha = mBitmapState->mBaseAlpha*mBitmapState->mAlpha/255.f;

        LOGV_IF(mBitmapState->mFilterBitmap&&(mBitmapWidth*mBitmapHeight>=512*512),
                   "%p[%s] size=%dx%d opacity=%d . should setFilterBitmap(false) to make render faster",
                   this,getConstantState()->mResource.c_str(),mBitmapWidth,mBitmapHeight,getOpacity());

        canvas.rectangle(mBounds.left,mBounds.top,mBounds.width,mBounds.height);
        canvas.clip();
        canvas.translate(mDstRect.left, mDstRect.top);
        if ( (mDstRect.width !=mBitmapWidth) || (mDstRect.height != mBitmapHeight) ) {
            canvas.scale(fx,fy);
#if defined(__x86_64__)||defined(__amd64__)||defined(__i386__)
            LOGD_IF((mBitmapWidth*mBitmapHeight>=512*512)||(std::min(fx,fy)<0.1f)||(std::max(fx,fy)>10.f),
                "%p bitmap %s scaled %dx%d->%d,%d",this,mBitmapState->mResource.c_str() ,mBitmapWidth,mBitmapHeight,mDstRect.width,mDstRect.height);
#endif
        }

        if(needMirroring()){
            canvas.translate(mDstRect.width,0);
            canvas.scale(-1.f,1.f);
        }
        canvas.set_source(source, 0, 0);
        if(getOpacity()==PixelFormat::OPAQUE){
            canvas.set_operator(Cairo::Context::Operator::SOURCE);
        }
        Cairo::RefPtr<SurfacePattern>spat = canvas.get_source_for_surface();
        if(spat){
            spat->set_filter(filterMode);
            spat->set_dither(ditherMode);
        }
        canvas.paint_with_alpha(alpha);
    }

    canvas.restore();
}

Insets BitmapDrawable::getOpticalInsets() {
    updateDstRectAndInsetsIfDirty();
    return mOpticalInsets;
}

void BitmapDrawable::getOutline(Outline& outline) {
    updateDstRectAndInsetsIfDirty();
    outline.setRect(mDstRect);

    // Only opaque Bitmaps can report a non-0 alpha,
    // since only they are guaranteed to fill their bounds. Compare against
    // the PixelFormat constant, not the literal 255 (OPAQUE is 3 — the old
    // comparison could never be true, so outline alpha was always 0).
    const bool opaqueOverShape = getOpacity() == PixelFormat::OPAQUE;
    outline.setAlpha(opaqueOverShape ? getAlpha() / 255.0f : 0.0f);
}

void BitmapDrawable::updateStateFromTypedArray(const TypedArray& a, int srcDensityOverride){
    auto& state = *mBitmapState;
    Resources& r = const_cast<Resources&>(a.getResources());
    // AOSP: extract the theme attributes for later re-resolution (applyTheme).
    state.mThemeAttrs = a.extractThemeAttrs();

    // AOSP: store density override + resolve target density from the display.
    state.mSrcDensityOverride = srcDensityOverride;
    const DisplayMetrics& dm = r.getDisplayMetrics();
    state.mTargetDensity = Drawable::resolveDensity(dm.densityDpi);

    // AOSP: src is read HERE (inside updateStateFromTypedArray), not in inflate().
    // Density-aware: getValueForDensity resolves the best config, then the bitmap
    // is decoded at that density.
    const int srcResId = a.getResourceId(R::styleable::BitmapDrawable_src, 0);
    if (srcResId != 0) {
        TypedValue tv;
        // CDROID has no getValueForDensity; getValue resolves for the current config.
        if (r.getValue(srcResId, &tv, true) && tv.string) {
            std::string path = TextUtils::utf16_utf8((const uint16_t*)tv.string, tv.stringLen);
            if (!path.empty()) {
                // android-36 :833-848: pretend the requested density is the
                // display density — an exact override match maps to the display
                // density (no downstream scaling), a mismatch gets the request/
                // display ratio so computeBitmapSize forces the scaling.
                if (srcDensityOverride > 0 && tv.density > 0
                        && tv.density != TypedValue::DENSITY_NONE) {
                    if (tv.density == srcDensityOverride) {
                        tv.density = dm.densityDpi;
                    } else {
                        tv.density = (tv.density * dm.densityDpi) / srcDensityOverride;
                    }
                }
                int density = 0;   // Bitmap.DENSITY_NONE stand-in
                if (tv.density == TypedValue::DENSITY_DEFAULT) {
                    density = DisplayMetrics::DENSITY_DEFAULT;
                } else if (tv.density != TypedValue::DENSITY_NONE) {
                    density = tv.density;
                }
                // AOSP: r.openRawResource(srcResId, value) → InputStream → decode.
                // CDROID Asset has read(), not getInputStream(); slurp into buffer.
                Asset* asset = r.openRawResource(srcResId, &tv);
                if (asset) {
                    const off64_t sz = asset->getLength();
                    if (sz > 0) {
                        std::string buf((size_t)sz, '\0');
                        asset->read(&buf[0], (size_t)sz);
                        auto stream = std::make_unique<std::istringstream>(std::move(buf));
                        auto bmp = ImageDecoder::loadImage(*stream);
                        if (bmp) {
                            state.mBitmap = bmp;
                            state.mTransparency = ImageDecoder::getTransparency(bmp);
                        }
                    }
                    delete asset;
                }
                // CDROID ImageSurface doesn't carry density metadata; the
                // decoded bucket's density lives in mBitmapDensity (the
                // Bitmap.mDensity stand-in) so computeBitmapSize can scale it
                // to the target. mSrcDensityOverride keeps its AOSP meaning.
                state.mBitmapDensity = density;
            }
        }
    }

    state.mAutoMirrored = a.getBoolean(R::styleable::BitmapDrawable_autoMirrored, state.mAutoMirrored);
    state.mBaseAlpha = a.getFloat(R::styleable::BitmapDrawable_alpha, state.mBaseAlpha);

    const int tintMode = a.getInt(R::styleable::BitmapDrawable_tintMode, -1);
    if (tintMode != -1) {
        state.mTintMode = parseTintMode(tintMode, PorterDuff::Mode::SRC_IN);
    }
    auto tint = a.getColorStateList(R::styleable::BitmapDrawable_tint);
    if (tint) state.mTint = tint;

    state.mAntiAlias = a.getBoolean(R::styleable::BitmapDrawable_antialias, state.mAntiAlias);
    state.mFilterBitmap = a.getBoolean(R::styleable::BitmapDrawable_filter, state.mFilterBitmap);
    state.mDither = a.getBoolean(R::styleable::BitmapDrawable_dither, state.mDither);
    state.mGravity = a.getInt(R::styleable::BitmapDrawable_gravity, state.mGravity);

    const int tileMode = a.getInt(R::styleable::BitmapDrawable_tileMode, TileMode::DISABLED);
    state.mTileModeX = a.getInt(R::styleable::BitmapDrawable_tileModeX, tileMode);
    state.mTileModeY = a.getInt(R::styleable::BitmapDrawable_tileModeY, tileMode);

    computeBitmapSize();
}

// AOSP BitmapDrawable.canApplyTheme/applyTheme: pending ?attr re-resolution.
bool BitmapDrawable::canApplyTheme(){
    return (mBitmapState && !mBitmapState->mThemeAttrs.empty()) || Drawable::canApplyTheme();
}

void BitmapDrawable::applyTheme(const Resources::Theme& t){
    Drawable::applyTheme(t);
    if (mBitmapState && !mBitmapState->mThemeAttrs.empty()) {
        auto a = t.resolveAttributes(mBitmapState->mThemeAttrs, R::styleable::BitmapDrawable);
        if (a) updateStateFromTypedArray(*a, 0);
        mBitmapState->mThemeAttrs.clear();
    }
    // AOSP applyTheme also ends in updateLocalState(): refresh the tint
    // filter and size from the re-resolved state.
    mTintFilter = updateTintFilter(mTintFilter, mBitmapState->mTint, mBitmapState->mTintMode);
    computeBitmapSize();
}

void BitmapDrawable::inflate(Resources&r,XmlPullParser&parser,const AttributeSet&atts,const Resources::Theme* theme){
    Drawable::inflate(r,parser,atts,theme);
    // AOSP: all attr reads + src loading happen inside updateStateFromTypedArray.
    auto ta = obtainAttributes(r, theme, atts, R::styleable::BitmapDrawable);
    if (ta) updateStateFromTypedArray(*ta, 0);
    // AOSP ends inflate with updateLocalState(), which rebuilds the tint
    // filter from the freshly parsed state — without it a statically tinted
    // bitmap (e.g. ratingbar_material's ?attr/colorControlActivated stars)
    // draws untinted (black) because mTintFilter stays null.
    mTintFilter = updateTintFilter(mTintFilter, mBitmapState->mTint, mBitmapState->mTintMode);
}

}

