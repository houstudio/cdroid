#include <drawables/drawablewrapper.h>
#include <cdlog.h>


namespace cdroid{

DrawableWrapper::DrawableWrapperState::DrawableWrapperState(){
    mDrawableState=nullptr;
    mDensity=160;//DisplayMetrics.DENSITY_DEFAULT
    mSrcDensityOverride =0;
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

Drawable*DrawableWrapper::DrawableWrapperState::newDrawable(){
    return new DrawableWrapper(shared_from_this());
}

bool DrawableWrapper::DrawableWrapperState::canConstantState()const {
    return mDrawableState!=nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DrawableWrapper::DrawableWrapper(Drawable*dr){
    mState=nullptr;
    mDrawable =dr;
    mMutated=false;
}

DrawableWrapper::DrawableWrapper(std::shared_ptr<DrawableWrapperState>state){
    mState=state;
    mDrawable=nullptr;
    mMutated=false;
    updateLocalState();
}

void DrawableWrapper::updateLocalState() {
    if (mState && mState->mDrawableState) {
        Drawable* dr = mState->mDrawableState->newDrawable();
        setDrawable(dr);
    }
}

DrawableWrapper::~DrawableWrapper(){
    delete mDrawable;
}

bool DrawableWrapper::onLevelChange(int level){
    return mDrawable != nullptr && mDrawable->setLevel(level);
}

void DrawableWrapper::setDrawable(Drawable*dr){
    if (mDrawable != nullptr)
        mDrawable->setCallback(nullptr);
    mDrawable=dr;
    if(dr){
        dr->setCallback(this);
        dr->setVisible(isVisible(), true);
        dr->setState(getState());
        dr->setLevel(getLevel());
        dr->setBounds(getBounds());
        dr->setLayoutDirection(getLayoutDirection());   
        if(mState)
            mState->mDrawableState=dr->getConstantState(); 
    }
    invalidateSelf();
}

Drawable*DrawableWrapper::getDrawable(){
    return mDrawable;
}

int DrawableWrapper::getIntrinsicWidth()const {
    return mDrawable != nullptr ? mDrawable->getIntrinsicWidth() : -1;
}

int DrawableWrapper::getIntrinsicHeight()const {
    return mDrawable != nullptr ? mDrawable->getIntrinsicHeight() : -1;
}

std::shared_ptr<DrawableWrapper::DrawableWrapperState> DrawableWrapper::mutateConstantState(){
    return std::make_shared<DrawableWrapperState>(*mState);
}

Drawable*DrawableWrapper::mutate(){
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
    if (mDrawable ) 
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
                | mDrawable->getChangingConfigurations();
}

void DrawableWrapper::onBoundsChange(const Rect&bounds){
    if(mDrawable)
        mDrawable->setBounds(bounds);
}

void DrawableWrapper::invalidateDrawable(Drawable& who){
    Drawable::Callback* callback = getCallback();
    if (callback != nullptr) {
        callback->invalidateDrawable(*this);
    }
}

void DrawableWrapper::scheduleDrawable(Drawable&who,Runnable what, long when){
    Drawable::Callback* callback = getCallback();
    if (callback != nullptr) {
        callback->scheduleDrawable(*this, what, when);
    }
}

void DrawableWrapper::unscheduleDrawable(Drawable& who,Runnable what){
    Drawable::Callback* callback = getCallback();
    if (callback != nullptr) {
        callback->unscheduleDrawable(*this, what);
    }
}

void DrawableWrapper::draw(Canvas&canvas){
    if (mDrawable != nullptr) {
        mDrawable->draw(canvas);
    }
}

}
