#include <drawables/ninepatchdrawable.h>
#include <gravity.h>
#include <fstream>
#include <cdlog.h>
namespace cdroid{
//https://github.com/soramimi/QtNinePatch/blob/master/NinePatch.cpp

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
    bitmap->get_ninepatch(mHorz,mVert);
    if(padding)
       mPadding=*padding;
    mNinePatch=bitmap;
    mPadding.set(0,0,0,0);
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
        if(needsMirroring()||1){
            const float cx=mBounds.left+mBounds.width/2.f;
            const float cy=mBounds.left+mBounds.height/2.f;
            canvas.scale(-1.f,1.f);
            canvas.translate(cx,cy);
        }
        canvas.draw_ninepatch(mNinePatchState->mNinePatch,mBounds,mNinePatchState->mHorz,mNinePatchState->mVert);
        canvas.restore();
    }
}

Drawable*NinePatchDrawable::inflate(Context*ctx,const AttributeSet&atts){
    const std::string src=atts.getString("src");
    RefPtr<ImageSurface>bmp;
    if(ctx==nullptr){
	std::string path=atts.getAbsolutePath(src);
        std::ifstream fs(path);
        bmp=ImageSurface::create_from_stream(fs);
    }else{
        std::unique_ptr<std::istream>is=ctx->getInputStream(src);
        bmp=ImageSurface::create_from_stream(*is);
    }
    return new NinePatchDrawable(bmp);
}

}

