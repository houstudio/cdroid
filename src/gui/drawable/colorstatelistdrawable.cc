#include <drawable/colorstatelistdrawable.h>
#include <drawable/colordrawable.h>
#include <algorithm>

namespace cdroid{

ColorStateListDrawable::ColorStateListDrawable(){
    mState = std::make_shared<ColorStateListDrawableState>();
    initializeColorDrawable();
}

ColorStateListDrawable::ColorStateListDrawable(const cdroid::RefPtr<ColorStateList>& colorStateList){
    mState = std::make_shared<ColorStateListDrawableState>();
    initializeColorDrawable();
    setColorStateList(colorStateList);
}

ColorStateListDrawable::ColorStateListDrawable(std::shared_ptr<ColorStateListDrawableState> state){
    mState = state;
    initializeColorDrawable();
    onStateChange(getState());
}

ColorStateListDrawable::~ColorStateListDrawable(){
    delete mColorDrawable;
}

void ColorStateListDrawable::draw(Canvas& canvas){
    mColorDrawable->draw(canvas);
}

int ColorStateListDrawable::getAlpha()const{
    return mColorDrawable->getAlpha();
}

bool ColorStateListDrawable::isStateful()const{
    return mState->isStateful();
}

bool ColorStateListDrawable::hasFocusStateSpecified()const{
    return mState->hasFocusStateSpecified();
}

Drawable* ColorStateListDrawable::getCurrent(){
    return mColorDrawable;
}

bool ColorStateListDrawable::canApplyTheme(){
    return Drawable::canApplyTheme() || mState->canApplyTheme();
}

void ColorStateListDrawable::setAlpha(int alpha){
    mState->mAlpha = alpha;
    onStateChange(getState());
}

void ColorStateListDrawable::clearAlpha(){
    mState->mAlpha = -1;
    onStateChange(getState());
}

void ColorStateListDrawable::setTintList(const cdroid::RefPtr<ColorStateList>& tint){
    mState->mTint = tint;
    mColorDrawable->setTintList(tint);
    onStateChange(getState());
}

void ColorStateListDrawable::setTintBlendMode(int blendMode){
    mState->mBlendMode = blendMode;
    mColorDrawable->setTintBlendMode(blendMode);
    onStateChange(getState());
}

const cdroid::RefPtr<ColorFilter> ColorStateListDrawable::getColorFilter()const{
    return mColorDrawable->getColorFilter();
}

void ColorStateListDrawable::setColorFilter(const cdroid::RefPtr<ColorFilter>& colorFilter){
    mColorDrawable->setColorFilter(colorFilter);
}

int ColorStateListDrawable::getOpacity()const{
    return mColorDrawable->getOpacity();
}

void ColorStateListDrawable::onBoundsChange(const Rect& bounds){
    Drawable::onBoundsChange(bounds);
    mColorDrawable->setBounds(bounds);
}

bool ColorStateListDrawable::onStateChange(const std::vector<int>& state){
    // androidx ColorStateListDrawable.onStateChange (152-170).
    if (mState->mColor) {
        int color = mState->mColor->getColorForState(state, mState->mColor->getDefaultColor());
        if (mState->mAlpha != -1) {
            color = (color & 0xFFFFFF) | (std::max(0, std::min(255, mState->mAlpha)) << 24);
        }
        if (color != mColorDrawable->getColor()) {
            mColorDrawable->setColor(color);
            mColorDrawable->setState(state);
            return true;
        } else {
            return mColorDrawable->setState(state);
        }
    }
    return false;
}

void ColorStateListDrawable::invalidateDrawable(Drawable& who){
    if (&who == mColorDrawable && getCallback() != nullptr) {
        getCallback()->invalidateDrawable(*this);
    }
}

void ColorStateListDrawable::scheduleDrawable(Drawable& who,const Runnable& what,int64_t when){
    if (&who == mColorDrawable && getCallback() != nullptr) {
        getCallback()->scheduleDrawable(*this, what, when);
    }
}

void ColorStateListDrawable::unscheduleDrawable(Drawable& who,const Runnable& what){
    if (&who == mColorDrawable && getCallback() != nullptr) {
        getCallback()->unscheduleDrawable(*this, what);
    }
}

std::shared_ptr<Drawable::ConstantState> ColorStateListDrawable::getConstantState(){
    // androidx (194-198): OR in this drawable's config not already tracked by the state.
    mState->mChangingConfigurations = mState->mChangingConfigurations
            | (getChangingConfigurations() & ~mState->getChangingConfigurations());
    return mState;
}

cdroid::RefPtr<ColorStateList> ColorStateListDrawable::getColorStateList(){
    if (!mState->mColor) {
        return ColorStateList::valueOf(mColorDrawable->getColor());
    }
    return mState->mColor;
}

int ColorStateListDrawable::getChangingConfigurations()const{
    return Drawable::getChangingConfigurations() | mState->getChangingConfigurations();
}

Drawable* ColorStateListDrawable::mutate(){
    if (!mMutated && Drawable::mutate() == this) {
        mState = std::make_shared<ColorStateListDrawableState>(*mState);
        mMutated = true;
    }
    return this;
}

void ColorStateListDrawable::clearMutated(){
    Drawable::clearMutated();
    mMutated = false;
}

void ColorStateListDrawable::setColorStateList(const cdroid::RefPtr<ColorStateList>& colorStateList){
    mState->mColor = colorStateList;
    onStateChange(getState());
}

void ColorStateListDrawable::initializeColorDrawable(){
    mColorDrawable = new ColorDrawable();
    mColorDrawable->setCallback(this);
    if (mState->mTint) {
        mColorDrawable->setTintList(mState->mTint);
    }
    if (mState->mBlendMode != Drawable::DEFAULT_BLEND_MODE) {
        mColorDrawable->setTintBlendMode(mState->mBlendMode);
    }
}

////////////////////////////////////////////////////////////////////////////////////

ColorStateListDrawable::ColorStateListDrawableState::ColorStateListDrawableState(){
}

ColorStateListDrawable::ColorStateListDrawableState::ColorStateListDrawableState(const ColorStateListDrawableState& orig){
    mColor = orig.mColor;
    mTint = orig.mTint;
    mAlpha = orig.mAlpha;
    mBlendMode = orig.mBlendMode;
    mChangingConfigurations = orig.mChangingConfigurations;
}

Drawable* ColorStateListDrawable::ColorStateListDrawableState::newDrawable(){
    return new ColorStateListDrawable(shared_from_this());
}

int ColorStateListDrawable::ColorStateListDrawableState::getChangingConfigurations()const{
    return mChangingConfigurations
            | (mColor ? mColor->getChangingConfigurations() : 0)
            | (mTint ? mTint->getChangingConfigurations() : 0);
}

bool ColorStateListDrawable::ColorStateListDrawableState::isStateful()const{
    return (mColor && mColor->isStateful())
            || (mTint && mTint->isStateful());
}

bool ColorStateListDrawable::ColorStateListDrawableState::hasFocusStateSpecified()const{
    return (mColor && mColor->hasFocusStateSpecified())
            || (mTint && mTint->hasFocusStateSpecified());
}

bool ColorStateListDrawable::ColorStateListDrawableState::canApplyTheme()const{
    // androidx (289-292) checks mColor/mTint canApplyTheme; CDROID ComplexColor/ColorStateList
    // has no canApplyTheme yet, so the faithful default is false (CTS does not exercise theming).
    return false;
}

}/*namespace*/
