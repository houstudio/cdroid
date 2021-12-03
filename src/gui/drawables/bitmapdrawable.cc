#include <drawables/bitmapdrawable.h>
#include <gravity.h>
#include <fstream>
#include <app.h>
#include <cdlog.h>

namespace cdroid{

BitmapDrawable::BitmapState::BitmapState(){
    mGravity  = Gravity::FILL;
    mBaseAlpha= 1.0f;
    mAlpha = 255;
    mTint  = nullptr;
    mTransparency= -1;
    mTintMode    = DEFAULT_TINT_MODE;
    mTileModeX =mTileModeY=-1;
    mAutoMirrored= false;
    mSrcDensityOverride=0;
    mTargetDensity=160;
    mChangingConfigurations=0;
}

BitmapDrawable::BitmapState::BitmapState(RefPtr<ImageSurface>bitmap)
    :BitmapState(){
    mBitmap = bitmap;
    mTransparency = computeTransparency(bitmap);
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
    //mRebuildShader = bitmapState.mRebuildShader;
    mAutoMirrored = bitmapState.mAutoMirrored;
}

Drawable* BitmapDrawable::BitmapState::newDrawable(){
    return new BitmapDrawable(shared_from_this());    
}

int BitmapDrawable::BitmapState::getChangingConfigurations()const{
    return mChangingConfigurations |(mTint ? mTint->getChangingConfigurations() : 0);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

BitmapDrawable::BitmapDrawable(RefPtr<ImageSurface>img){
    mBitmapState=std::make_shared<BitmapState>(img);
    mDstRectAndInsetsDirty=true;
    computeBitmapSize();
    mMutated=false;
    mTintFilter=nullptr;
}

BitmapDrawable::BitmapDrawable(std::shared_ptr<BitmapState>state){
    mBitmapState=state;
    mDstRectAndInsetsDirty=true;
    mMutated=false;
    mTintFilter=nullptr;
    computeBitmapSize();
}

BitmapDrawable::BitmapDrawable(std::istream&is){
    mTintFilter=nullptr;
    mBitmapState=std::make_shared<BitmapState>();
    RefPtr<ImageSurface>b=ImageSurface::create_from_stream(is);
    setBitmap(b);
}

BitmapDrawable::BitmapDrawable(const std::string&resname){
    mBitmapState=std::make_shared<BitmapState>();
    std::ifstream fs(resname);
    RefPtr<ImageSurface>b;
    mTintFilter=nullptr;
    LOGV("%s",resname.c_str());
    if(fs.good())
        b=ImageSurface::create_from_stream(fs);
    else {
        b=App::getInstance().getImage(resname);
    }
    setBitmap(b);
}

RefPtr<ImageSurface> BitmapDrawable::getBitmap()const{
    return mBitmapState->mBitmap;
}

void BitmapDrawable::setBitmap(RefPtr<ImageSurface>bmp){
    mBitmapState->mBitmap=bmp;
    mBitmapState->mTransparency = computeTransparency(bmp);
    LOGV("setbitmap %p",bmp.get());
    mDstRectAndInsetsDirty=true;
    computeBitmapSize();
    invalidateSelf();
}

int BitmapDrawable::getAlpha()const{
    return mBitmapState->mAlpha;
}

void BitmapDrawable::setAlpha(int alpha){
    mBitmapState->mAlpha=alpha&0xFF;
}

int BitmapDrawable::getGravity()const{
    return mBitmapState->mGravity;
}

int BitmapDrawable::getIntrinsicWidth()const{
    return mBitmapWidth;
}

int BitmapDrawable::getIntrinsicHeight()const{
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
    if(mBitmapState->mTileModeX!=xmode||mBitmapState->mTileModeY!=ymode){
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

bool BitmapDrawable::isAutoMirrored(){
    return mBitmapState->mAutoMirrored;
}

int BitmapDrawable::computeTransparency(RefPtr<ImageSurface>bmp){
    if(bmp==nullptr||bmp->get_width()==0||bmp->get_height()==0)
        return Drawable::TRANSPARENT;
    if((bmp->get_content()&Cairo::Content::CONTENT_ALPHA)==0)
        return Drawable::OPAQUE;

    if(bmp->get_content()&CONTENT_COLOR==0){
        switch(bmp->get_format()){
        case Surface::Format::A1: return Drawable::TRANSPARENT;//CAIRO_IMAGE_HAS_BILEVEL_ALPHA;
        case Surface::Format::A8:
            for(int y=0;y<bmp->get_height();y++){
                uint8_t*alpha=bmp->get_data()+bmp->get_stride()*y;
                for(int x=0;x<bmp->get_width();x++,alpha++)
                    if(*alpha > 0 && *alpha < 255)
                        return Drawable::TRANSLUCENT;//CAIRO_IMAGE_HAS_ALPHA;
            }
            return Drawable::TRANSPARENT;//CAIRO_IMAGE_HAS_BILEVEL_ALPHA;
        default:return Drawable::TRANSLUCENT; 
        }
    }
    if(bmp->get_format()==Surface::Format::RGB16_565||bmp->get_format()==Surface::Format::RGB24)
        return Drawable::OPAQUE;
    if(bmp->get_format()!=Surface::Format::ARGB32)
        return Drawable::TRANSLUCENT;
    for(int y=0;y<bmp->get_height();y++){
        uint32_t*pixel=(uint32_t*)(bmp->get_data()+bmp->get_stride()*y);
        for (int x = 0; x < bmp->get_width(); x++, pixel++){
            int a = (*pixel & 0xff000000) >> 24;
            if (a > 0 && a < 255)return Drawable::TRANSLUCENT;//CAIRO_IMAGE_HAS_ALPHA;
            else if(a==0)return Drawable::TRANSPARENT;//CAIRO_IMAGE_HAS_BILEVEL_ALPHA
        }
    }
    return  Drawable::OPAQUE;
}

int BitmapDrawable::getOpacity(){
    if(mBitmapState->mGravity != Gravity::FILL)
        return TRANSLUCENT;
    if(mBitmapState->mBitmap==nullptr)
        return TRANSPARENT;

    return mBitmapState->mTransparency;
}

void BitmapDrawable::setTintList(ColorStateList*tint){
    if (mBitmapState->mTint != tint) {
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
        if (true/*mBitmapState->mTileModeX == nullptr && mBitmapState->mTileModeY == nullptr*/) {
            const int layoutDir = getLayoutDirection();
            mDstRect.set(0,0,0,0);
            Gravity::apply(mBitmapState->mGravity,mBitmapWidth,mBitmapHeight,mBounds, mDstRect, layoutDir);
            const int left  = mDstRect.left - mBounds.left;
            const int top   = mDstRect.top - mBounds.top;
            const int right = mBounds.right() - mDstRect.right();
            const int bottom= mBounds.bottom()- mDstRect.bottom();
            mOpticalInsets.set(left, top, right, bottom);
        } else {
            mDstRect=getBounds();
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
    if (mBitmapState->mTint  /*&& mBitmapState->mTintMode != nullptr*/) {
        mTintFilter = updateTintFilter(mTintFilter, mBitmapState->mTint, mBitmapState->mTintMode);
        return true;
    }
    return false;    
}

Drawable*BitmapDrawable::mutate(){
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

void BitmapDrawable::draw(Canvas&canvas){
    if(mBitmapState->mBitmap==nullptr) return;
    updateDstRectAndInsetsIfDirty();
    LOGV("BitmapSize=%dx%d bounds=%d,%d-%d,%d dst=%d,%d-%d,%d alpha=%d mColorFilter=%p",mBitmapWidth,mBitmapHeight,
            mBounds.left,mBounds.top,mBounds.width,mBounds.height, mDstRect.left,mDstRect.top,
	    mDstRect.width,mDstRect.height,mBitmapState->mAlpha,mTintFilter);

    const float sw=mBitmapWidth, sh=mBitmapHeight;
    float dx = mBounds.left    , dy = mBounds.top;
    float dw = mBounds.width   , dh = mBounds.height;
    const float fx = dw / sw  , fy = dh / sh;
    const float alpha=mBitmapState->mBaseAlpha*mBitmapState->mAlpha/255;

    LOGD_IF(mBounds.empty(),"%p's bounds is empty,skip drawing,otherwise will caused crash",this);
    canvas.save();
    canvas.rectangle(mBounds.left,mBounds.top,mBounds.width,mBounds.height);

    if(mBitmapState->mTileModeX>=0||mBitmapState->mTileModeY>=0){
        RefPtr<SurfacePattern> pat=SurfacePattern::create(mBitmapState->mBitmap);
        switch(mBitmapState->mTileModeX){
        case TileMode::CLAMP : pat->set_extend(Pattern::Extend::PAD)   ; break;
        case TileMode::REPEAT: pat->set_extend(Pattern::Extend::REPEAT); break;
        case TileMode::MIRROR: pat->set_extend(Pattern::Extend::REFLECT);break;
        }
        canvas.set_source(pat);
        canvas.fill();
    }else{
        canvas.clip();
        if ( (mBounds.width !=mBitmapWidth)  || (mBounds.height != mBitmapHeight) ) {
            canvas.scale(dw/sw,dh/sh);
            dx /= fx;       dy /= fy;
            dw /= fx;       dh /= fy;
        }

        if(needMirroring()){
            canvas.translate(mDstRect.width,0);
            canvas.scale(-1.f,1.f);
        }
        canvas.set_source(mBitmapState->mBitmap, dx, dy );
        canvas.get_source_for_surface()->set_filter(SurfacePattern::Filter::BEST);
        canvas.paint_with_alpha(alpha);
    }
    canvas.restore();
    if(mTintFilter)mTintFilter->apply(canvas,mBounds);
}

Insets BitmapDrawable::getOpticalInsets() {
    updateDstRectAndInsetsIfDirty();
    return mOpticalInsets;
}

Drawable*BitmapDrawable::inflate(Context*ctx,const AttributeSet&atts){
    const std::string src=atts.getString("src");
    bool antialias=atts.getBoolean("antialias",true);
    bool dither=atts.getBoolean("dither",true);
    bool filter=atts.getBoolean("filter",true);
    bool mipMap=atts.getBoolean("mipMap",true);
    int gravity=atts.getGravity("gravity",Gravity::CENTER);
    static std::map<const std::string,int>kvs={
	      {"disabled",TileMode::DISABLED}, {"clamp",TileMode::CLAMP},
		  {"repeat",TileMode::REPEAT},  {"mirror",TileMode::MIRROR}};
    const int tileMode = atts.getInt("tileMode",kvs,-1);
    int tileModeX= atts.getInt("tileModeX",kvs,tileMode);
    int tileModeY= atts.getInt("tileModeY",kvs,tileMode);
    std::string path=src;
    if(ctx==nullptr)path=atts.getAbsolutePath(src);

    LOGD("src=%s",src.c_str());
    if(src.empty())  return nullptr;
    BitmapDrawable*d=new BitmapDrawable(path);
    LOGD("bitmap=%p",d);
    d->setGravity(gravity);
    d->setTileModeXY(tileModeX,tileModeY); 
    return d;
}

}

