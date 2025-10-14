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
#include <drawable/drawablecontainer.h>
#include <core/systemclock.h>
#include <porting/cdlog.h>
namespace cdroid{

class BlockInvalidateCallback:public Drawable::Callback {
private:
    Drawable::Callback* mCallback;
public:
    BlockInvalidateCallback():mCallback(nullptr){
    }
    BlockInvalidateCallback* wrap(Drawable::Callback* callback){
        mCallback = callback;
        return this;
    }

    Drawable::Callback* unwrap() {
        Drawable::Callback* callback = mCallback;
        mCallback = nullptr;
        return callback;
    }

    void invalidateDrawable(Drawable& who)override{
        // Ignore invalidation.
    }

    void scheduleDrawable(Drawable& who,const Runnable& what, int64_t when)override{
        if (mCallback != nullptr) {
            mCallback->scheduleDrawable(who, what, when);
        }
    }

    void unscheduleDrawable(Drawable& who,const Runnable& what)override{
        if (mCallback != nullptr) {
            mCallback->unscheduleDrawable(who, what);
        }
    }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////

DrawableContainer::DrawableContainerState::DrawableContainerState(const DrawableContainerState*orig,DrawableContainer*own){
    mOwner = own;
    mDensity = Drawable::resolveDensity(orig? orig->mDensity : 0);
    mVariablePadding = false;
    mConstantSize = false;
    mCheckedConstantState = true;
    mCheckedConstantSize  = false;
    mCanConstantState = true;
    mAutoMirrored = false;
    mMutated = false;
    mDither  = false;
    mCheckedStateful  = false;
    mCheckedOpacity   = false;
    mLayoutDirection  = LayoutDirection::LTR;
    mEnterFadeDuration= 0;
    mExitFadeDuration = 0;
    mTintList = nullptr;
    mColorFilter = nullptr;
    if(orig == nullptr)
        return;

    mChangingConfigurations = orig->mChangingConfigurations;
    mChildrenChangingConfigurations = orig->mChildrenChangingConfigurations;


    mVariablePadding = orig->mVariablePadding;
    mConstantSize = orig->mConstantSize;
    mDither = orig->mDither;
    mMutated = orig->mMutated;
    mLayoutDirection = orig->mLayoutDirection;
    mEnterFadeDuration = orig->mEnterFadeDuration;
    mExitFadeDuration = orig->mExitFadeDuration;
    mAutoMirrored = orig->mAutoMirrored;
    /*mColorFilter = orig.mColorFilter;
    mHasColorFilter = orig.mHasColorFilter;
    mTintList = orig.mTintList;
    mTintMode = orig.mTintMode;
    mHasTintList = orig.mHasTintList;
    mHasTintMode = orig.mHasTintMode;*/

    if (orig->mDensity == mDensity) {
        mConstantPadding = orig->mConstantPadding;
        mCheckedPadding = true;

        if (orig->mCheckedConstantSize) {
            mConstantWidth = orig->mConstantWidth;
            mConstantHeight = orig->mConstantHeight;
            mConstantMinimumWidth = orig->mConstantMinimumWidth;
            mConstantMinimumHeight = orig->mConstantMinimumHeight;
            mCheckedConstantSize = true;
        }
    }

    if (orig->mCheckedOpacity) {
        mOpacity = orig->mOpacity;
        mCheckedOpacity = true;
    }

    if (orig->mCheckedStateful) {
        mStateful = orig->mStateful;
        mCheckedStateful = true;
    }

    mDrawables.resize(orig->mDrawables.size());
    // Postpone cloning children and futures until we're absolutely
    // sure that we're done computing values for the original state.

    mDrawableFutures = orig->mDrawableFutures;
    // Create futures for drawables with constant states. If a
    // drawable doesn't have a constant state, then we can't clone
    // it and we'll have to reference the original.
    for(int i=0;i<orig->mDrawables.size();i++){
        Drawable*d = orig->mDrawables[i];
        if(d == nullptr)continue;
        std::shared_ptr<ConstantState> cs=d->getConstantState();
        if(cs)
            mDrawableFutures.put(i,cs);
        else
            mDrawables[i]=d;
    }
}

DrawableContainer::DrawableContainerState::~DrawableContainerState(){
    for_each( mDrawables.begin(), mDrawables.end(),[](Drawable*d){delete d;});
    mDrawables.clear();
    //delete mTintList;//tintlist cant be destroied
    delete mColorFilter;
}

int DrawableContainer::DrawableContainerState::getChangingConfigurations()const{
    return mChangingConfigurations | mChildrenChangingConfigurations;
}

int DrawableContainer::DrawableContainerState::addChild(Drawable* dr){
    const int pos = (int)mDrawables.size();
    dr->mutate();
    dr->setVisible(false, true);
    dr->setCallback(mOwner);

    LOGV("addChild(%p)[%d] to %p",dr,mDrawables.size(),this);
    mDrawables.push_back(dr);
    mChildrenChangingConfigurations |= dr->getChangingConfigurations();

    invalidateCache();

    mConstantPadding.set(0,0,0,0);
    mCheckedPadding = false;
    mCheckedConstantSize = false;
    mCheckedConstantState = false;
    return pos;
}

void DrawableContainer::DrawableContainerState::invalidateCache(){
    mCheckedOpacity = false;
    mCheckedStateful= false;
}

void DrawableContainer::DrawableContainerState::createAllFutures(){
    const size_t futureCount = mDrawableFutures.size();
    for (size_t keyIndex = 0; keyIndex < futureCount; keyIndex++) {
        const int index= mDrawableFutures.keyAt(keyIndex);
        std::shared_ptr<ConstantState>cs =mDrawableFutures.valueAt(keyIndex);
        delete mDrawables[index];
        mDrawables[index] = prepareDrawable(cs->newDrawable());
    }
    mDrawableFutures.clear();
}

Drawable* DrawableContainer::DrawableContainerState::prepareDrawable(Drawable* child) {
    child->setLayoutDirection(mLayoutDirection);
    child = child->mutate();
    child->setCallback(mOwner);
    return child;
}

int DrawableContainer::DrawableContainerState::getChildCount()const{
    return (int)mDrawables.size();
}

std::vector<Drawable*> DrawableContainer::DrawableContainerState::getChildren(){
    createAllFutures();
    return mDrawables;
}

Drawable*DrawableContainer::DrawableContainerState::getChild(int index){
    Drawable*dr = mDrawables.at(index);
    if(dr)return dr;
    if (mDrawableFutures.size()) {
        const int keyIndex = mDrawableFutures.indexOfKey(index);
        if (keyIndex>=0) {
            std::shared_ptr<ConstantState> cs = mDrawableFutures.valueAt(keyIndex);
            Drawable* prepared = prepareDrawable(cs->newDrawable());
            mDrawables[index] = prepared;
            mDrawableFutures.removeAt(keyIndex);
            LOGV("getChild(%d)=%p",index,prepared);
            return prepared;
        }
    }
    return nullptr;
}

bool DrawableContainer::DrawableContainerState::setLayoutDirection(int layoutDirection, int currentIndex){
    bool changed = false;
    // No need to call createAllFutures, since future drawables will
    // change layout direction when they are prepared.
    for (int i = 0; i < mDrawables.size(); i++) {
        if (mDrawables[i] != nullptr) {
            const bool childChanged = mDrawables[i]->setLayoutDirection(layoutDirection);
            if (i == currentIndex) {
                changed = childChanged;
            }
        }
    }
    mLayoutDirection = layoutDirection;
    return changed;    
}

void DrawableContainer::DrawableContainerState::mutate(){
    for (auto dr:mDrawables) {
       if (dr!= nullptr)dr->mutate();
    }
    mMutated = true;
}

void DrawableContainer::DrawableContainerState::clearMutated(){
    for (auto dr:mDrawables){
        if (dr)dr->clearMutated();
    }
    mMutated = false;
}

void DrawableContainer::DrawableContainerState::setVariablePadding(bool variable) {
    mVariablePadding = variable;
}

bool DrawableContainer::DrawableContainerState::canConstantState() {
    if (mCheckedConstantState) {
        return mCanConstantState;
    }
    createAllFutures();
    mCheckedConstantState = true;
    for (auto dr:mDrawables) {
        if (dr->getConstantState() == nullptr) {
            mCanConstantState = false;
            return false;
        }
    }
    mCanConstantState = true;
    return true;
}

bool DrawableContainer::DrawableContainerState::getConstantPadding(Rect&rect) {
    if (mVariablePadding)return false;

    if (!mConstantPadding.empty()|| mCheckedPadding) {
        rect= mConstantPadding;
        return mConstantPadding.left>=0&&mConstantPadding.top>=0&&mConstantPadding.width>=0&&mConstantPadding.height>=0;
    }

    createAllFutures();

    Rect r ={0,0,0,0};
    Rect t ={0,0,0,0};
    for (auto dr:mDrawables) {
        if (dr->getPadding(t)) {
            if (t.left > r.left) r.left = t.left;
            if (t.top > r.top) r.top = t.top;
            if(t.width > r.width ) r.width = t.width;
            if(t.height> r.height) r.height= t.height;
        }
    }
    mCheckedPadding = true;
    mConstantPadding =r;
    rect = r;
    return true;
}

void DrawableContainer::DrawableContainerState::setConstantSize(bool constant) {
    mConstantSize = constant;
}

bool DrawableContainer::DrawableContainerState::isConstantSize() const{
    return mConstantSize;
}

std::shared_ptr<DrawableContainer::DrawableContainerState> DrawableContainer::cloneConstantState(){
    return std::make_shared<DrawableContainerState>(mDrawableContainerState.get(),this);
}

void DrawableContainer::setConstantState(std::shared_ptr<DrawableContainerState>state){
    mDrawableContainerState = state;
    // The locally cached drawables may have changed.
    if (mCurIndex >= 0) {
        mCurrDrawable = state->getChild(mCurIndex);
        if (mCurrDrawable != nullptr) {
            initializeDrawableForDisplay(mCurrDrawable);
        }
    }
    mLastIndex = -1;
    mLastDrawable = nullptr;
}

DrawableContainer*DrawableContainer::mutate(){
    if (!mMutated && Drawable::mutate() == this) {
        std::shared_ptr<DrawableContainerState> clone=cloneConstantState();
        LOGV("DrawableContainerState %p cloned  from %p",clone.get(),mDrawableContainerState.get());
        clone->mutate();
        setConstantState(clone);
        mMutated = true;
    }
    return this;
}

int DrawableContainer::DrawableContainerState::getConstantWidth() {
    if (!mCheckedConstantSize) computeConstantSize();
    return mConstantWidth;
}

int DrawableContainer::DrawableContainerState::getConstantHeight() {
    if (!mCheckedConstantSize) computeConstantSize();
    return mConstantHeight;
}

int DrawableContainer::DrawableContainerState::getConstantMinimumWidth() {
    if (!mCheckedConstantSize)computeConstantSize();
    return mConstantMinimumWidth;
}

int DrawableContainer::DrawableContainerState::getConstantMinimumHeight() {
    if (!mCheckedConstantSize) computeConstantSize();
    return mConstantMinimumHeight;
}

void DrawableContainer::DrawableContainerState::computeConstantSize() {
    mCheckedConstantSize = true;
    createAllFutures();

    mConstantWidth = mConstantHeight = -1;
    mConstantMinimumWidth = mConstantMinimumHeight = 0;
    for (auto dr:mDrawables) {
        int s = dr->getIntrinsicWidth();
        if (s > mConstantWidth) mConstantWidth = s;
        s = dr->getIntrinsicHeight();
        if (s > mConstantHeight) mConstantHeight = s;
        s = dr->getMinimumWidth();
        if (s > mConstantMinimumWidth) mConstantMinimumWidth = s;
        s = dr->getMinimumHeight();
        if (s > mConstantMinimumHeight) mConstantMinimumHeight = s;
    }
}
bool DrawableContainer::DrawableContainerState::isStateful(){
    if (mCheckedStateful)
        return mStateful;
    createAllFutures();

    bool isStateful = false;
    for (auto dr:mDrawables) {
        if (dr->isStateful()) {
            isStateful = true;
            break;
        }
    }

    mStateful = isStateful;
    mCheckedStateful = true;
    return isStateful;
}

int DrawableContainer::DrawableContainerState::getOpacity(){
    if (mCheckedOpacity) {
        return mOpacity;
    }

    createAllFutures();

    const int N = (int)mDrawables.size();
    int op = (N > 0) ? mDrawables[0]->getOpacity() : PixelFormat::TRANSPARENT;
    for (int i = 1; i < N; i++) {
        op = Drawable::resolveOpacity(op, mDrawables[i]->getOpacity());
    }

    mOpacity = op;
    mCheckedOpacity = true;
    return op;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
DrawableContainer::DrawableContainer(){
    mDrawableContainerState= std::make_shared<DrawableContainerState>(nullptr,this);
    mHasAlpha = false;
    mMutated  = false;
    mAlpha = 0xFF;
    mHotspotBounds.set(0,0,0,0);
    mCurIndex = mLastIndex = -1;
    mCurrDrawable = mLastDrawable = nullptr;
    mBlockInvalidateCallback = nullptr;
    mExitAnimationEnd = 0;
    mEnterAnimationEnd= 0;
}

DrawableContainer::DrawableContainer(Context*ctx,const AttributeSet&atts):DrawableContainer(){
    mDrawableContainerState->setConstantSize(atts.getBoolean("constantSize"));
    mDrawableContainerState->setVariablePadding(atts.getBoolean("variablePadding")); 
}

DrawableContainer::~DrawableContainer(){
    std::vector<Drawable*>&ds=mDrawableContainerState->mDrawables;
    for_each(ds.begin(),ds.end(),[](Drawable*d){delete d;});
    ds.clear();
    delete mBlockInvalidateCallback;
}

void DrawableContainer::clearMutated(){
    Drawable::clearMutated();
    mDrawableContainerState->clearMutated();
    mMutated = false; 
}

bool DrawableContainer::needsMirroring(){
    return isAutoMirrored() && getLayoutDirection() ==LayoutDirection::RTL;
}

bool DrawableContainer::getPadding(Rect&padding){
    Rect r={0,0,0,0};
    bool result;
    if (mDrawableContainerState->getConstantPadding(r)){//r!=null) {
       padding=r;
       result = (r.left|r.top|r.width|r.height)!=0;
    } else {
       if (mCurrDrawable != nullptr) {
           result = mCurrDrawable->getPadding(padding);
       } else {
           result = Drawable::getPadding(padding);
       }
    }
    if (needsMirroring()) {
        const int left = padding.left;
        const int right= padding.top;
        padding.left= right;
        padding.top = left;
    }
    return result;
}

Insets DrawableContainer::getOpticalInsets(){
    return  mCurrDrawable?mCurrDrawable->getOpticalInsets():Insets();
}

void DrawableContainer::getOutline(Outline& outline) {
    if (mCurrDrawable != nullptr) {
        mCurrDrawable->getOutline(outline);
    }
}

int DrawableContainer::getChangingConfigurations()const{
    return Drawable::getChangingConfigurations()
             | mDrawableContainerState->getChangingConfigurations();
}

void DrawableContainer::setAlpha(int alpha) {
    if (!mHasAlpha || (mAlpha != alpha)) {
        mHasAlpha = true;
        mAlpha = alpha;
        if (mCurrDrawable != nullptr) {
            if (mEnterAnimationEnd == 0) {
                mCurrDrawable->setAlpha(alpha);
            } else {
                animate(false);
            }
        }
    }
}

int DrawableContainer::getAlpha() const{
    return mAlpha;
}

void DrawableContainer::setDither(bool dither) {
    if (mDrawableContainerState->mDither != dither) {
        mDrawableContainerState->mDither = dither;
        if (mCurrDrawable != nullptr) {
            mCurrDrawable->setDither(mDrawableContainerState->mDither);
        }
    }
}

void DrawableContainer::setColorFilter(ColorFilter*colorFilter){

    if (mDrawableContainerState->mColorFilter != colorFilter) {
        mDrawableContainerState->mColorFilter = colorFilter;

        if (mCurrDrawable) {
            mCurrDrawable->setColorFilter(colorFilter);
        }
    }
}

void DrawableContainer::setTintList(const ColorStateList*tint){
    if( mDrawableContainerState->mTintList!=tint ){
        mDrawableContainerState->mTintList = tint;
        if(mCurrDrawable)
            mCurrDrawable->setTintList(tint);
    }
}

void DrawableContainer::setTintMode(int tintMode){

    if (mDrawableContainerState->mTintMode != tintMode) {
        mDrawableContainerState->mTintMode = tintMode;

        if (mCurrDrawable) {
            mCurrDrawable->setTintMode(tintMode);
        }
    }
}

void DrawableContainer::setEnterFadeDuration(int ms) {
    mDrawableContainerState->mEnterFadeDuration = ms;
}

void DrawableContainer::setExitFadeDuration(int ms) {
    mDrawableContainerState->mExitFadeDuration = ms;
}

void DrawableContainer::onBoundsChange(const Rect&bounds){
    if (mLastDrawable != nullptr) {
        mLastDrawable->setBounds(bounds);
    }
    if (mCurrDrawable != nullptr) {
        mCurrDrawable->setBounds(bounds);
    }
}

bool DrawableContainer::onStateChange(const std::vector<int>&state){
    if (mLastDrawable != nullptr)
        return mLastDrawable->setState(state);
    if (mCurrDrawable != nullptr)
        return mCurrDrawable->setState(state);
    return false;
}

bool DrawableContainer::onLevelChange(int level) {
    if (mLastDrawable != nullptr)
        return mLastDrawable->setLevel(level);
    if (mCurrDrawable != nullptr)
        return mCurrDrawable->setLevel(level);
    return false;
}

bool DrawableContainer::onLayoutDirectionChanged(int layoutDirection) {
    // Let the container handle setting its own layout direction. Otherwise,
    // we're accessing potentially unused states.
    return mDrawableContainerState->setLayoutDirection(layoutDirection, getCurrentIndex());
}

bool DrawableContainer::isStateful()const{
    return mDrawableContainerState->isStateful();
}

bool DrawableContainer::hasFocusStateSpecified()const{
    if (mCurrDrawable) {
        return mCurrDrawable->hasFocusStateSpecified();
    }
    if (mLastDrawable) {
        return mLastDrawable->hasFocusStateSpecified();
    }
    return false;
}

void DrawableContainer::setAutoMirrored(bool mirrored){
    if (mDrawableContainerState->mAutoMirrored != mirrored) {
        mDrawableContainerState->mAutoMirrored = mirrored;
        if (mCurrDrawable ) {
            mCurrDrawable->setAutoMirrored(mDrawableContainerState->mAutoMirrored);
        }
    }
}

bool DrawableContainer::isAutoMirrored()const{
    return mDrawableContainerState->mAutoMirrored;
}

void DrawableContainer::jumpToCurrentState(){
    bool changed = false;
    if (mLastDrawable ) {
        mLastDrawable->jumpToCurrentState();
        mLastDrawable = nullptr;
        mLastIndex = -1;
        changed = true;
    }
    if (mCurrDrawable) {
        mCurrDrawable->jumpToCurrentState();
        if (mHasAlpha) {
            mCurrDrawable->setAlpha(mAlpha);
        }
    }
    if (mExitAnimationEnd != 0) {
        mExitAnimationEnd = 0;
        changed = true;
     }
    if (mEnterAnimationEnd != 0) {
        mEnterAnimationEnd = 0;
        changed = true;
    }
    if (changed) {
        invalidateSelf();
    }
}

int DrawableContainer::getIntrinsicWidth() {
    LOGV("%p constSize=%d cur=%p size=%d",this,mDrawableContainerState->isConstantSize(),mCurrDrawable,
        (mCurrDrawable?mCurrDrawable->getIntrinsicWidth():-1));
    if (mDrawableContainerState->isConstantSize()) {
        return mDrawableContainerState->getConstantWidth();
    }
    return mCurrDrawable != nullptr ? mCurrDrawable->getIntrinsicWidth() : -1;
}

int DrawableContainer::getIntrinsicHeight() {
    if (mDrawableContainerState->isConstantSize()) {
        return mDrawableContainerState->getConstantHeight();
    }
    return mCurrDrawable != nullptr ? mCurrDrawable->getIntrinsicHeight() : -1;
}

int DrawableContainer::getMinimumWidth() {
    if (mDrawableContainerState->isConstantSize()) {
        return mDrawableContainerState->getConstantMinimumWidth();
    }
    return mCurrDrawable != nullptr ? mCurrDrawable->getMinimumWidth() : 0;
}

int DrawableContainer::getMinimumHeight() {
    if (mDrawableContainerState->isConstantSize()) {
        return mDrawableContainerState->getConstantMinimumHeight();
    }
    return mCurrDrawable != nullptr ? mCurrDrawable->getMinimumHeight() : 0;
}

void DrawableContainer::setHotspotBounds(int left, int top, int width, int height) {
    mHotspotBounds.set(left, top, width, height);
    if (mCurrDrawable) {
        mCurrDrawable->setHotspotBounds(left, top,width,height);
    }
}

void DrawableContainer::getHotspotBounds(Rect& outRect)const{
    if(mHotspotBounds.empty())outRect = mHotspotBounds;
    else Drawable::getHotspotBounds(outRect);
}

int DrawableContainer::getCurrentIndex()const{
    return mCurIndex;
}

Drawable*DrawableContainer::getCurrent(){
    return mCurrDrawable;
}

void DrawableContainer::animate(bool schedule) {
    mHasAlpha = true;

    const int64_t now = SystemClock::uptimeMillis();
    bool animating = false;
    if (mCurrDrawable != nullptr) {
        if (mEnterAnimationEnd != 0) {
            if (mEnterAnimationEnd <= now) {
                mCurrDrawable->setAlpha(mAlpha);
                mEnterAnimationEnd = 0;
            } else {
                const int animAlpha = (int)((mEnterAnimationEnd-now)*255)
                        / mDrawableContainerState->mEnterFadeDuration;
                mCurrDrawable->setAlpha(((255-animAlpha)*mAlpha)/255);
                animating = true;
            }
        }
    } else {
        mEnterAnimationEnd = 0;
    }
    if (mLastDrawable != nullptr) {
        if (mExitAnimationEnd != 0) {
            if (mExitAnimationEnd <= now) {
                mLastDrawable->setVisible(false, false);
                mLastDrawable = nullptr;
                mLastIndex = -1;
                mExitAnimationEnd = 0;
            } else {
                const int animAlpha = (int)((mExitAnimationEnd-now)*255)
                        / mDrawableContainerState->mExitFadeDuration;
                mLastDrawable->setAlpha((animAlpha*mAlpha)/255);
                animating = true;
            }
        }
    } else {
        mExitAnimationEnd = 0;
    }
    LOGV_IF(mCurrDrawable&&mLastDrawable,"%p \nCur %d:%p[%s] alpha=%d\nLast %d:%p[%s] alpha=%d",this,
		mCurIndex,mCurrDrawable,mCurrDrawable->getConstantState()->mResource.c_str(),mCurrDrawable->getAlpha(),
		mLastIndex,mLastDrawable,mLastDrawable->getConstantState()->mResource.c_str(),mLastDrawable->getAlpha());
    if (schedule && animating) {
        scheduleSelf(mAnimationRunnable, now + 1000 / 60);
    }
}

void DrawableContainer::initializeDrawableForDisplay(Drawable*d){
    if (mBlockInvalidateCallback == nullptr)
        mBlockInvalidateCallback = new BlockInvalidateCallback();

    d->setCallback(mBlockInvalidateCallback->wrap(d->getCallback()));

    if(mDrawableContainerState->mEnterFadeDuration <= 0 && mHasAlpha){
        d->setAlpha(mAlpha);
    }
    if (mDrawableContainerState->mColorFilter) {
        // Color filter always overrides tint.
        d->setColorFilter(mDrawableContainerState->mColorFilter);
    } else {
        if (mDrawableContainerState->mTintList) {
            d->setTintList(mDrawableContainerState->mTintList);
        }
        /*if (mDrawableContainerState->mHasTintMode) {
            d->setTintBlendMode(mDrawableContainerState->mBlendMode);
        }*/
    }
    d->setVisible(isVisible(), true);
    d->setDither(mDrawableContainerState->mDither);
    d->setState(getState());
    d->setLevel(getLevel());
    d->setBounds(getBounds());
    d->setLayoutDirection(getLayoutDirection());
    d->setAutoMirrored(mDrawableContainerState->mAutoMirrored);    
    d->setCallback(mBlockInvalidateCallback->unwrap());
}

void DrawableContainer::setCurrentIndex(int index){
    selectDrawable(index);
}

bool DrawableContainer::selectDrawable(int index){
    if(index==mCurIndex)return false;
    const int64_t now = SystemClock::uptimeMillis();
    if(mDrawableContainerState->mExitFadeDuration > 0){
        if (mLastDrawable != nullptr) {
            mLastDrawable->setVisible(false, false);
        }
        if(mCurrDrawable!=nullptr){
            mLastDrawable = mCurrDrawable;
            mLastIndex = mCurIndex;
            mExitAnimationEnd = now + mDrawableContainerState->mExitFadeDuration;
        }else{
            mLastDrawable = nullptr;
            mLastIndex = -1;
            mExitAnimationEnd = 0;
        }
    }else if(mCurrDrawable!=nullptr){
        mCurrDrawable->setVisible(false, false);
    }
    if( (index>=0) && (index<getChildCount()) ){
        mCurIndex = index;
        mCurrDrawable = mDrawableContainerState->getChild(index);
        if(mCurrDrawable){
            if(mDrawableContainerState->mEnterFadeDuration>0){
                mEnterAnimationEnd = now + mDrawableContainerState->mEnterFadeDuration;
            }
            initializeDrawableForDisplay(mCurrDrawable);
        }
    }else{
        mCurrDrawable = nullptr;
        mCurIndex = -1;
    }

    if (mEnterAnimationEnd || mExitAnimationEnd) {
        if (mAnimationRunnable == nullptr) {
            mAnimationRunnable = [this](){
                animate(true);
                invalidateSelf();
            };
        } else {
            unscheduleSelf(mAnimationRunnable);
        }
        // Compute first frame and schedule next animation.
        animate(true);
    }
    invalidateSelf();
    return true;
}

int DrawableContainer::addChild(Drawable*d){
    return mDrawableContainerState->addChild(d);
}

int DrawableContainer::getChildCount()const{
    return mDrawableContainerState->getChildCount();
}

std::shared_ptr<Drawable::ConstantState>DrawableContainer::getConstantState(){
    if (mDrawableContainerState->canConstantState()) {
        mDrawableContainerState->mChangingConfigurations = getChangingConfigurations();
        return mDrawableContainerState;
    }
    return nullptr;
}

Drawable*DrawableContainer::getChild(int index){
    auto &drs=mDrawableContainerState->mDrawables;
    return ((index>=0)&&(index<drs.size())) ? drs[index] : nullptr;
}

void DrawableContainer::invalidateDrawable(Drawable& who){
    if (mDrawableContainerState != nullptr) {
        mDrawableContainerState->invalidateCache();
    }

    if ((&who == mCurrDrawable) && (getCallback() != nullptr)) {
        getCallback()->invalidateDrawable(*this);
    }
}

void DrawableContainer::scheduleDrawable(Drawable&who,const Runnable& what, int64_t when){
    if ((&who == mCurrDrawable)&&(mCallback != nullptr))
        mCallback->scheduleDrawable(who, what, when);
}

void DrawableContainer::unscheduleDrawable(Drawable& who,const Runnable& what){
    if ((&who == mCurrDrawable)&&(mCallback != nullptr)) 
        mCallback->unscheduleDrawable(who, what);
}

bool DrawableContainer::setVisible(bool visible, bool restart) {
    bool changed = Drawable::setVisible(visible, restart);
    if (mLastDrawable != nullptr) {
        mLastDrawable->setVisible(visible, restart);
    }
    if (mCurrDrawable != nullptr) {
        mCurrDrawable->setVisible(visible, restart);
    }
    return changed;
}

int DrawableContainer::getOpacity() {
    return ((mCurrDrawable == nullptr) || !mCurrDrawable->isVisible()) ? PixelFormat::TRANSPARENT :
            mDrawableContainerState->getOpacity();
}

void DrawableContainer::draw(Canvas&canvas){
    if(mCurrDrawable != nullptr)
        mCurrDrawable->draw(canvas);
    if (mLastDrawable != nullptr) 
        mLastDrawable->draw(canvas);
}

}
