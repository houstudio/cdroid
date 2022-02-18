#include <drawables/ninepatchdrawable.h>
#include <gravity.h>
#include <fstream>
#include <cdlog.h>
namespace cdroid{
//https://github.com/soramimi/QtNinePatch/blob/master/NinePatch.cpp

NinePatchDrawable::NinePatchDrawable(std::shared_ptr<NinePatchState>state){
    mNinePatchState=state;
    mAlpha=255;
    mTintFilter=nullptr;
    computeBitmapSize();
}

NinePatchDrawable::NinePatchDrawable(Context*ctx,const std::string&resid){
    mNinePatchState=std::make_shared<NinePatchState>(ctx->getImage(resid));
    mAlpha=255;
    mTintFilter=nullptr;
    computeBitmapSize();
}

NinePatchDrawable::NinePatchDrawable(RefPtr<ImageSurface>bmp){
    mNinePatchState=std::make_shared<NinePatchState>(bmp);
    mAlpha=255;
    mTintFilter=nullptr;
    computeBitmapSize();
}

NinePatchDrawable::~NinePatchDrawable(){
    delete mTintFilter;
}

void NinePatchDrawable::computeBitmapSize(){
    const RefPtr<ImageSurface> ninePatch = mNinePatchState->mNinePatch;
    if (ninePatch == nullptr) return;
    const int sourceDensity =160;// ninePatch.getDensity();
    const int targetDensity =160;// mTargetDensity;

    const Insets sourceOpticalInsets = mNinePatchState->mOpticalInsets;
    if (sourceOpticalInsets != Insets::NONE) {
        const int left  = Drawable::scaleFromDensity( sourceOpticalInsets.left   , sourceDensity, targetDensity, true);
        const int top   = Drawable::scaleFromDensity( sourceOpticalInsets.top    , sourceDensity, targetDensity, true);
        const int right = Drawable::scaleFromDensity( sourceOpticalInsets.right  , sourceDensity, targetDensity, true);
        const int bottom= Drawable::scaleFromDensity( sourceOpticalInsets.bottom , sourceDensity, targetDensity, true);
        mOpticalInsets.set(left, top, right, bottom);// = Insets.of(left, top, right, bottom);
    } else {
        mOpticalInsets.set(0,0,0,0);// = Insets.NONE;
    }

    const Rect sourcePadding = mNinePatchState->mPadding;
    if (1/*sourcePadding != nullptr*/) {
        
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
        mAlpha=alpha;
        invalidateSelf();
    }
}

bool NinePatchDrawable::getPadding(Rect& padding){
    padding=mPadding;
    return (padding.left | padding.top | padding.width | padding.height) != 0;
}

int NinePatchDrawable::getAlpha()const{
    return mAlpha;
}

void NinePatchDrawable::setTintList(ColorStateList* tint){
    mNinePatchState->mTint = tint;
    mTintFilter = updateTintFilter(mTintFilter, tint, mNinePatchState->mTintMode);
    invalidateSelf();    
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
    return isAutoMirrored() && getLayoutDirection() == LayoutDirection::RTL;
}

bool NinePatchDrawable::isAutoMirrored(){
    return mNinePatchState->mAutoMirrored;
}

int NinePatchDrawable::getIntrinsicWidth()const{
    return mBitmapWidth;
}

int NinePatchDrawable::getIntrinsicHeight()const {
    return mBitmapHeight;
}

Drawable* NinePatchDrawable::mutate() {
    if (!mMutated && Drawable::mutate() == this) {
        mNinePatchState=std::make_shared<NinePatchState>(*mNinePatchState);
        mMutated = true;
    }
    return this;
}

bool NinePatchDrawable::onStateChange(const std::vector<int>& stateSet){
    if (mNinePatchState->mTint && mNinePatchState->mTintMode != TintMode::NONOP) {
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
        if(needsMirroring()){
            const float cx=mBounds.left+mBounds.width/2.f;
            const float cy=mBounds.left+mBounds.height/2.f;
            canvas.scale(-1.f,1.f);
            canvas.translate(cx,cy);
        }
        mNinePatchState->draw(canvas,mBounds);
        canvas.restore();
    }
}

Drawable*NinePatchDrawable::inflate(Context*ctx,const AttributeSet&atts){
    const std::string src=atts.getString("src");
    RefPtr<ImageSurface>bmp;
    std::unique_ptr<std::istream>is=ctx->getInputStream(src);
    bmp=ImageSurface::create_from_stream(*is);
    return new NinePatchDrawable(bmp);
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

NinePatchDrawable::NinePatchState::NinePatchState(RefPtr<ImageSurface>bitmap,const Rect*padding)
  :NinePatchDrawable::NinePatchState(){
    mNinePatch=bitmap;
    get_ninepatch(mHorz,mVert,true);
    
    mPadding.set(0,0,0,0);
    if(mHorz.size()){
        if(!mHorz[0].stretchable)mPadding.left=mHorz[0].len;
        if(!mHorz.back().stretchable)mPadding.width=mHorz.back().len;
    } 
    if(mVert.size()){
        if(!mVert[0].stretchable)mPadding.top=mVert[0].len;
        if(!mVert.back().stretchable)mPadding.height=mVert.back().len;
    } 
    if(padding)mPadding=*padding;
    get_ninepatch(mHorz,mVert,false);
    LOGV("ninpatch %p size=%dx%d padding=(%d,%d,%d,%d)",this,bitmap->get_width(),bitmap->get_height(),
         mPadding.left,mPadding.top,mPadding.width,mPadding.height);
    for(auto h:mHorz)LOGV("%p HORZ %d,%d,%d)",this,h.pos,h.len,h.stretchable);
    for(auto v:mVert)LOGV("%p VERT %d,%d,%d)",this,v.pos,v.len,v.stretchable);
}

NinePatchDrawable::NinePatchState::NinePatchState(const NinePatchState&orig){
    mTint = orig.mTint;
    mNinePatch=orig.mNinePatch;
    mTintMode = orig.mTintMode;
    mPadding = orig.mPadding;
    mOpticalInsets = orig.mOpticalInsets;
    mBaseAlpha = orig.mBaseAlpha;
    mDither = orig.mDither;
    mHorz =orig.mHorz;
    mVert =orig.mVert;
    mChangingConfigurations=orig.mChangingConfigurations;
    mAutoMirrored = orig.mAutoMirrored;
    //mThemeAttrs = orig.mThemeAttrs;
}

Drawable*NinePatchDrawable::NinePatchState::newDrawable(){
    return new NinePatchDrawable(shared_from_this());
}

int NinePatchDrawable::NinePatchState::getChangingConfigurations()const{
   return mChangingConfigurations|(mTint ? mTint->getChangingConfigurations() : 0);
}

static unsigned int get_pixel(RefPtr<ImageSurface>img,int x,int y){
    unsigned int*data=(unsigned int*)(img->get_data()+y*img->get_stride());
    return data[x];
}
static bool isStretchableMarker(unsigned int px){
    return (px>>24)==0xFF;
}

int NinePatchDrawable::NinePatchState::get_ninepatch(std::vector<DIV>&divHorz,std::vector<DIV>&divVert,bool padding){
    int i;
    const int width = mNinePatch->get_width();
    const int height= mNinePatch->get_height();
    divHorz.clear();
    divVert.clear();
#define check_pixel(x,y) if(get_pixel(mNinePatch,x,y)>>24!=0) return 0;
    if(width<3||height<3)return 0;
    check_pixel(0,0);
    check_pixel(width-1,0);
    check_pixel(0,height-1);
    check_pixel(width-1,height-1);
    //horz stretch infos
    int pos = 1;
    int horz_stretch=0;
    int vert_stretch=0;
    unsigned int last,next;
    int edge=padding?(height-1):0;
    last=get_pixel(mNinePatch,pos,edge);
    for(int x=1;x<width-1;x++){
        next=get_pixel(mNinePatch,x+1,edge);
        if(isStretchableMarker(last)!=isStretchableMarker(next)||(x==width-2)){
            bool stretchable=isStretchableMarker(last);
            int len=x + 1 - pos;
            DIV d={pos,len,stretchable};
            divHorz.push_back(d);
            if(stretchable)horz_stretch+=len;
            last=next; pos=x+1;
        }
    }
    //vert streatch infos
    pos = 1;
    edge=padding?(width-1):0;
    last=get_pixel(mNinePatch,edge,pos);
    for(int y=1;y<height-1;y++){
        next=get_pixel(mNinePatch,edge,y+1);
        if(isStretchableMarker(last)!=isStretchableMarker(next)||(y==height-2)){
            bool stretchable = isStretchableMarker(last);
            int len = y + 1 - pos;
            DIV d={pos,len,stretchable};
            divVert.push_back(d);
            if (stretchable)vert_stretch += len;
            last = next;   pos = y+1;
        }
    }
    return horz_stretch<<16|vert_stretch;
}

void NinePatchDrawable::NinePatchState::draw(Canvas&canvas,const Rect&rect){
    int dw=rect.width;
    int dh=rect.height;
    int sw=mNinePatch->get_width();
    int sh=mNinePatch->get_height();
    float horz_stretch=0;
    float vert_stretch=0;

    float horz_mul=0,vert_mul =0;
    int dy0=0, dy1=0;
    float vstretch=0;
    for_each(mHorz.begin(),mHorz.end(),[&](const DIV&v){if(v.stretchable)horz_stretch+=v.len;});
    for_each(mVert.begin(),mVert.end(),[&](const DIV&v){if(v.stretchable)vert_stretch+=v.len;});

    if (horz_stretch > 0) horz_mul = (float)(dw - (sw - 2 - horz_stretch)) / horz_stretch;
    if (vert_stretch > 0) vert_mul = (float)(dh - (sh - 2 - vert_stretch)) / vert_stretch;
    for(int i=0;i<(int)mVert.size();i++){
        int sy0=mVert[i].pos;
        if(i+1==(int)mVert.size()){
            dy1=dh;
        }else if(mVert[i].stretchable){
            vstretch=(float)mVert[i].len*vert_mul;
            float s=floor(vstretch);
            vstretch-=s;
            dy1+=(int)s;
        }else{
            dy1+=mVert[i].len;
        }
        int dx0=0,dx1=0;
        float hstretch=0;
        for(int j=0;j<(int)mHorz.size();j++){
            int sx0=mHorz[j].pos;
            if(j+1==(int)mHorz.size()){
                dx1=dw;
            }else if(mHorz[j].stretchable){
                hstretch+=(float)mHorz[j].len*horz_mul;
                float s=floor(hstretch);
                hstretch-=s;
                dx1+=(int)s;
            }else{
                dx1+=mHorz[j].len;
            }
            RECT rd={dx0,dy0,dx1-dx0+1,dy1-dy0+1};
            RECT rs={sx0, sy0,mHorz[j].len,mVert[i].len};
            LOGV("rs=(%d,%d,%d,%d)",rs.left,rs.top,rs.width,rs.height);
            rd.offset(rect.left,rect.top);
            canvas.draw_image(mNinePatch,rd,&rs);
            dx0=dx1;
        }
        dy0=dy1;
    }
}

}

