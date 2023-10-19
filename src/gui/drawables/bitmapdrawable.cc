#include <drawables/bitmapdrawable.h>
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
    mSrcDensityOverride = 0;
    mTargetDensity = 160;
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
    mTransparency = bitmapState.mTransparency;
    //mRebuildShader = bitmapState.mRebuildShader;
    mAutoMirrored = bitmapState.mAutoMirrored;
}

BitmapDrawable::BitmapState::~BitmapState(){
    delete mTint;
}

Drawable* BitmapDrawable::BitmapState::newDrawable(){
    return new BitmapDrawable(shared_from_this());    
}

int BitmapDrawable::BitmapState::getChangingConfigurations()const{
    return mChangingConfigurations |(mTint ? mTint->getChangingConfigurations() : 0);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

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
    std::ifstream fs(resname,std::ios::binary);
    RefPtr<ImageSurface>b;
    LOGV("%s",resname.c_str());
    if((ctx==nullptr)||fs.good()){
        b = ImageSurface::create_from_stream(fs);
    }else {
        b = ctx->getImage(resname);
    }
    setBitmap(b);
}

BitmapDrawable::~BitmapDrawable(){
    delete  mTintFilter;
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
        return PixelFormat::TRANSPARENT;
    if((bmp->get_content()&Cairo::Content::CONTENT_ALPHA)==0)
        return PixelFormat::OPAQUE;

    if(bmp->get_content()&CONTENT_COLOR==0){
        switch(bmp->get_format()){
        case Surface::Format::A1: return PixelFormat::TRANSPARENT;//CAIRO_IMAGE_HAS_BILEVEL_ALPHA;
        case Surface::Format::A8:
            for(int y=0;y<bmp->get_height();y++){
                uint8_t*alpha=bmp->get_data()+bmp->get_stride()*y;
                for(int x=0;x<bmp->get_width();x++,alpha++)
                    if(*alpha > 0 && *alpha < 255)
                        return PixelFormat::TRANSLUCENT;//CAIRO_IMAGE_HAS_ALPHA;
            }
            return PixelFormat::TRANSPARENT;//CAIRO_IMAGE_HAS_BILEVEL_ALPHA;
        default:return PixelFormat::TRANSLUCENT; 
        }
    }
    if(bmp->get_format()==Surface::Format::RGB16_565||bmp->get_format()==Surface::Format::RGB24)
        return PixelFormat::OPAQUE;
    if(bmp->get_format()!=Surface::Format::ARGB32)
        return PixelFormat::TRANSLUCENT;
    for(int y=0;y<bmp->get_height();y++){
        uint32_t*pixel=(uint32_t*)(bmp->get_data()+bmp->get_stride()*y);
        for (int x = 0; x < bmp->get_width(); x++, pixel++){
            int a = (*pixel & 0xff000000) >> 24;
            if (a > 0 && a < 255)return PixelFormat::TRANSLUCENT;//CAIRO_IMAGE_HAS_ALPHA;
            else if(a==0)return PixelFormat::TRANSPARENT;//CAIRO_IMAGE_HAS_BILEVEL_ALPHA
        }
    }
    return  PixelFormat::OPAQUE;
}

int BitmapDrawable::getOpacity(){
    if(mBitmapState->mGravity != Gravity::FILL)
        return PixelFormat::TRANSLUCENT;
    if(mBitmapState->mBitmap==nullptr)
        return PixelFormat::TRANSPARENT;

    return mBitmapState->mTransparency;
}

void BitmapDrawable::setTintList(ColorStateList*tint){
    if( tint == nullptr ){
	    delete mBitmapState->mTint;
	    mBitmapState->mTint = nullptr;
	    delete mTintFilter;
	    mTintFilter = nullptr;
    }else{
	if(mBitmapState->mTint)
	    *mBitmapState->mTint = *tint;
	else
	    mBitmapState->mTint = new ColorStateList(*tint);
	mTintFilter = updateTintFilter(mTintFilter, tint, mBitmapState->mTintMode);
    }LOGD("%p tint=%p",this,tint);
    invalidateSelf();
    /*if (*mBitmapState->mTint != *tint) {
        *mBitmapState->mTint = new ColorStateList(*tint);//must deep copy
        mTintFilter = updateTintFilter(mTintFilter, tint, mBitmapState->mTintMode);
        invalidateSelf();
    }*/
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
        if (mBitmapState->mTileModeX == TileMode::DISABLED && mBitmapState->mTileModeY == TileMode::DISABLED) {
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
    if (mBitmapState->mTint  && mBitmapState->mTintMode != TintMode::NOOP) {
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

static void setPatternByTileMode(RefPtr<SurfacePattern>pat,int tileMode){
    switch(tileMode){
    case TileMode::DISABLED:break;
    case TileMode::CLAMP : pat->set_extend(Pattern::Extend::PAD);     break;
    case TileMode::REPEAT: pat->set_extend(Pattern::Extend::REPEAT);  break;
    case TileMode::MIRROR: pat->set_extend(Pattern::Extend::REFLECT); break;
    }
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
    if(mBitmapState->mTileModeX>=0||mBitmapState->mTileModeY>=0){
        RefPtr<SurfacePattern> pat =SurfacePattern::create(mBitmapState->mBitmap);
        if(mBitmapState->mTileModeX!=TileMode::DISABLED){
            RefPtr<Surface> subs=ImageSurface::create(Surface::Format::ARGB32,mBounds.width,mBitmapHeight);
            RefPtr<Cairo::Context>subcanvas=Cairo::Context::create(subs);
            subcanvas->rectangle(0,0,mBounds.width,mBitmapHeight);
            setPatternByTileMode(pat,mBitmapState->mTileModeX); 
            subcanvas->set_source(pat);
            subcanvas->fill();

            RefPtr<SurfacePattern>pats= SurfacePattern::create(subs); 
            canvas.set_source(pats);
            if(mBounds.height>mBitmapHeight&&mBitmapState->mTileModeY==TileMode::DISABLED)
                 setPatternByTileMode(pats,TileMode::CLAMP);
            else setPatternByTileMode(pats,mBitmapState->mTileModeY);
            canvas.rectangle(mBounds.left,mBounds.top,mBounds.width,mBounds.height);
            canvas.fill();
        }else{
            RefPtr<Surface> subs=ImageSurface::create(Surface::Format::ARGB32,mBitmapWidth,mBounds.height);
            RefPtr<Cairo::Context>subcanvas=Cairo::Context::create(subs);
           
            subcanvas->rectangle(0,0,mBitmapWidth,mBounds.height);
            setPatternByTileMode(pat,mBitmapState->mTileModeY);
            subcanvas->set_source(pat);
            subcanvas->fill();

            RefPtr<SurfacePattern>pats= SurfacePattern::create(subs); 
            canvas.set_source(pats);
            if(mBounds.width>mBitmapWidth&&mBitmapState->mTileModeX==TileMode::DISABLED)
                setPatternByTileMode(pats,TileMode::CLAMP);
            else setPatternByTileMode(pats,mBitmapState->mTileModeX);
            canvas.rectangle(mBounds.left,mBounds.top,mBounds.width,mBounds.height);
            canvas.fill();
        } 
    }else {
        const float sw=mBitmapWidth, sh = mBitmapHeight;
        float dx = mBounds.left    , dy = mBounds.top;
        float dw = mBounds.width   , dh = mBounds.height;
        const float fx = dw / sw   , fy = dh / sh;
        const float alpha=mBitmapState->mBaseAlpha*mBitmapState->mAlpha/255.f;

        canvas.rectangle(mBounds.left,mBounds.top,mBounds.width,mBounds.height);
        canvas.clip();
        if ( (mBounds.width !=mBitmapWidth)  || (mBounds.height != mBitmapHeight) ) {
            canvas.scale(dw/sw,dh/sh);
            dx /= fx;       dy /= fy;
            dw /= fx;       dh /= fy;
            LOGD_IF(mBitmapWidth*mBitmapHeight>=512*512,"bitmap scaled %dx%d->%d,%d",mBitmapWidth,mBitmapHeight,mBounds.width,mBounds.height);
        }

        if(needMirroring()){
            canvas.translate(mDstRect.width,0);
            canvas.scale(-1.f,1.f);
        }
        canvas.set_source(mBitmapState->mBitmap, dx, dy);
        if(alpha==1.f){
            canvas.rectangle(dx,dy,dw,dh);
            canvas.fill();
        }else{
            Cairo::RefPtr<SurfacePattern>spat = canvas.get_source_for_surface();
            if(spat)spat->set_filter(SurfacePattern::Filter::GOOD);
            canvas.paint_with_alpha(alpha);
        }
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

Drawable*BitmapDrawable::inflate(Context*ctx,const AttributeSet&atts){
    const std::string src=atts.getString("src");
    bool antialias= atts.getBoolean("antialias",true);
    bool dither = atts.getBoolean("dither",true);
    bool filter = atts.getBoolean("filter",true);
    bool mipMap = atts.getBoolean("mipMap",true);
    const int gravity= atts.getGravity("gravity",Gravity::CENTER);
    static std::map<const std::string,int>kvs={
	      {"disabled",TileMode::DISABLED}, {"clamp",TileMode::CLAMP},
		  {"repeat",TileMode::REPEAT},  {"mirror",TileMode::MIRROR}};
    const int tileMode = atts.getInt("tileMode",kvs,-1);
    const int tileModeX= atts.getInt("tileModeX",kvs,tileMode);
    const int tileModeY= atts.getInt("tileModeY",kvs,tileMode);
    
    BitmapDrawable*d = new BitmapDrawable(ctx,src);
    LOGD("bitmap=%p",d);
    d->setGravity(gravity);
    d->setTileModeXY(tileModeX,tileModeY); 
    return d;
}

}

