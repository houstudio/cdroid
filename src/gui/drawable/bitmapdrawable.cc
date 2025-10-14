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
#include <drawable/bitmapdrawable.h>
#include <image-decoders/imagedecoder.h>
#include <fstream>
#include <app.h>
#include <cdlog.h>

using namespace Cairo;
namespace cdroid{

BitmapDrawable::BitmapState::BitmapState(){
    mGravity  = Gravity::FILL;
    mBaseAlpha= 1.0f;
    mAlpha = 255;
    mTint  = nullptr;
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
    //delete mTint;//tint cant be destroied
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
    mTintFilter = nullptr;
    computeBitmapSize();
}

BitmapDrawable::BitmapDrawable(std::shared_ptr<BitmapState>state){
    mBitmapState = state;
    mDstRectAndInsetsDirty = true;
    mMutated = false;
    mTintFilter = nullptr;
    computeBitmapSize();
}

BitmapDrawable::BitmapDrawable(Context*ctx,const std::string&resname)
  :BitmapDrawable(std::make_shared<BitmapState>()){
    RefPtr<ImageSurface>b;
#if 0
    std::ifstream fs(resname,std::ios::binary);
    if((ctx==nullptr)||fs.good()){
        b = ImageSurface::create_from_stream(fs);
    }else {
        b = ctx->loadImage(resname);
    }
#else
    b = ImageDecoder::loadImage(ctx,resname);
#endif
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
    delete mTintFilter;
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
    mBitmapState->mAlpha = alpha&0xFF;
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

int BitmapDrawable::getOpacity(){
    if(mBitmapState->mGravity != Gravity::FILL)
        return PixelFormat::TRANSLUCENT;
    if(mBitmapState->mBitmap==nullptr)
        return PixelFormat::TRANSPARENT;

    return mBitmapState->mTransparency;
}

void BitmapDrawable::setTintList(const ColorStateList*tint){
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

void BitmapDrawable::computeBitmapSize() {
    if (mBitmapState->mBitmap != nullptr) {
        mBitmapWidth = mBitmapState->mBitmap->get_width();//getScaledWidth(mTargetDensity);
        mBitmapHeight= mBitmapState->mBitmap->get_height();//getScaledHeight(mTargetDensity);
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

bool BitmapDrawable::onStateChange(const std::vector<int>&){
    if (mBitmapState->mTint  && (mBitmapState->mTintMode != PorterDuff::NOOP)) {
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

void BitmapDrawable::draw(Canvas&canvas){
    if(mBitmapState->mBitmap==nullptr) return;
    updateDstRectAndInsetsIfDirty();
    LOGV("BitmapSize=%dx%d bounds=%d,%d-%d,%d dst=%d,%d-%d,%d alpha=%d mColorFilter=%p",mBitmapWidth,mBitmapHeight,
            mBounds.left,mBounds.top,mBounds.width,mBounds.height, mDstRect.left,mDstRect.top,
	    mDstRect.width,mDstRect.height,mBitmapState->mAlpha,mTintFilter);

    LOGD_IF(mBounds.empty(),"%p's(%d,%d) bounds is empty,skip drawing,otherwise will caused crash",this,mBitmapWidth,mBitmapHeight);
    if(mBounds.empty())return;

    canvas.save();
    if(mTintFilter){
        canvas.rectangle(mBounds.left,mBounds.top,mBounds.width,mBounds.height);
        canvas.clip();
        canvas.push_group();
    }
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
        RefPtr<SurfacePattern> pat =SurfacePattern::create(mBitmapState->mBitmap);
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
        const float sw = float(mBitmapWidth), sh = float(mBitmapHeight);
        float dx = float(mBounds.left)  , dy = float(mBounds.top);
        float dw = float(mBounds.width) , dh = float(mBounds.height);
        const float fx = dw / sw   , fy = dh / sh;
        const float alpha = mBitmapState->mBaseAlpha*mBitmapState->mAlpha/255.f;

        LOGV_IF(mBitmapState->mFilterBitmap&&(mBitmapWidth*mBitmapHeight>=512*512),
                   "%p[%s] size=%dx%d opacity=%d . should setFilterBitmap(false) to make render faster",
                   this,getConstantState()->mResource.c_str(),mBitmapWidth,mBitmapHeight,getOpacity());

        canvas.rectangle(mBounds.left,mBounds.top,mBounds.width,mBounds.height);
        canvas.clip();
        if ( (mBounds.width !=mBitmapWidth) || (mBounds.height != mBitmapHeight) ) {
            canvas.scale(dw/sw,dh/sh);
            dx /= fx;
            dy /= fy;
#if defined(__x86_64__)||defined(__amd64__)||defined(__i386__)
            LOGD_IF((mBitmapWidth*mBitmapHeight>=512*512)||(std::min(fx,fy)<0.1f)||(std::max(fx,fy)>10.f),
                "%p bitmap %s scaled %dx%d->%d,%d",this,mBitmapState->mResource.c_str() ,mBitmapWidth,mBitmapHeight,mBounds.width,mBounds.height);
#endif
        }

        if(needMirroring()){
            canvas.translate(mDstRect.width,0);
            canvas.scale(-1.f,1.f);
        }
        canvas.set_source(mBitmapState->mBitmap, dx, dy);
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

    if(mTintFilter){
        mTintFilter->apply(canvas,mBounds);
        canvas.pop_group_to_source();
        canvas.paint();
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
    // since only they are guaranteed to fill their bounds
    const int opaqueOverShape = getOpacity()==255;//mBitmapState->mBitmap != nullptr&& !mBitmapState->mBitmap->hasAlpha();
    outline.setAlpha(opaqueOverShape ? getAlpha() / 255.0f : 0.0f);
}

void BitmapDrawable::inflate(XmlPullParser&parser,const AttributeSet&atts){
    Drawable::inflate(parser,atts);
    auto bmp = ImageDecoder::loadImage(atts.getContext(),atts.getString("src"));
    mBitmapState->mBitmap = bmp;
    static std::unordered_map<std::string,int>kvs={
          {"disabled",TileMode::DISABLED}, {"clamp",TileMode::CLAMP},
          {"repeat",TileMode::REPEAT},  {"mirror",TileMode::MIRROR}};
    const int tileMode=atts.getInt("tileMode",kvs,-1);
    mBitmapState->mTileModeX =atts.getInt("tileModeX",kvs,tileMode);
    mBitmapState->mTileModeY =atts.getInt("tileModeY",kvs,tileMode);
    mBitmapState->mGravity = atts.getGravity("gravity",Gravity::CENTER);
    mBitmapState->mDither = atts.getBoolean("antialias",true);
    mBitmapState->mFilterBitmap=atts.getBoolean("filter",false);
    mBitmapState->mAntiAlias=atts.getBoolean("antialias",true);
    setBitmap(bmp);//computeBitmapSize();
}

}

