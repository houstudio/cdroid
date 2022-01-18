#include <drawables/drawablecontainer.h>
#include <gravity.h>
#include <cdlog.h>
namespace cdroid{

class BlockInvalidateCallback:public Drawable::Callback {
private:
    Drawable::Callback* mCallback;
public:
    BlockInvalidateCallback* wrap(Drawable::Callback* callback) {
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

    void scheduleDrawable(Drawable& who,Runnable what, long when)override{
        if (mCallback != nullptr) {
            mCallback->scheduleDrawable(who, what, when);
        }
    }

    void unscheduleDrawable(Drawable& who,Runnable what)override{
        if (mCallback != nullptr) {
            mCallback->unscheduleDrawable(who, what);
        }
    }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////

DrawableContainer::DrawableContainerState::DrawableContainerState(const DrawableContainerState*orig,DrawableContainer*own){
    mOwner = own;
    mDensity = 160;//Drawable.resolveDensity(res, orig != null ? orig.mDensity : 0);
    mVariablePadding=false;
    mConstantSize=false;
    mCheckedConstantState = true;
    mCanConstantState = true;

    if(orig==nullptr)
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
        Drawable*d=orig->mDrawables[i];
        if(d==nullptr)continue;
        std::shared_ptr<ConstantState> cs=d->getConstantState();
        if(cs)
            mDrawableFutures[i]=cs;
        else
            mDrawables[i]=d;
    }
}
DrawableContainer::DrawableContainerState::~DrawableContainerState(){
    for_each( mDrawables.begin(), mDrawables.end(),[](Drawable*d){delete d;});
    mDrawables.clear();
}

int DrawableContainer::DrawableContainerState::getChangingConfigurations()const{
    return mChangingConfigurations | mChildrenChangingConfigurations;
}

int DrawableContainer::DrawableContainerState::getChildCount()const{
    return mDrawables.size();
}

Drawable*DrawableContainer::DrawableContainerState::getChild(int index){
    Drawable*dr=mDrawables.at(index);
    if(dr)return dr;
    if (mDrawableFutures.size()) {
        auto it=mDrawableFutures.find(index);
        if (it!=mDrawableFutures.end()) {
            auto cs = it->second;
            Drawable* prepared = prepareDrawable(cs->newDrawable());
            mDrawables[index] = prepared;
            LOGV("getChild(%d)=%p",index,prepared);
            mDrawableFutures.erase(it);
            return prepared;
        }
    }
    return nullptr;
}


int DrawableContainer::DrawableContainerState::addChild(Drawable* dr){
    const int pos=mDrawables.size();
    dr->mutate();
    dr->setVisible(false, true);
    dr->setCallback(mOwner);

    LOGV("%p addChild(%d:%p)",this,mDrawables.size(),dr);
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
    mCheckedOpacity =false;
    mCheckedStateful =false;
}

void DrawableContainer::DrawableContainerState::mutate(){
    int i=0;
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

bool DrawableContainer::DrawableContainerState::getConstantPadding(Rect&rect) {
    if (mVariablePadding)return false;

    if (!mConstantPadding.empty()|| mCheckedPadding) {
        return mConstantPadding.left>=0&&mConstantPadding.top>=0&&mConstantPadding.width>=0&&mConstantPadding.height>=0;
    }

    createAllFutures();

    Rect r ={0,0,0,0};
    Rect t ={0,0,0,0};
    for (auto dr:mDrawables) {
       if (dr->getPadding(t)) {
           if (t.left > r.left) r.left = t.left;
           if (t.top > r.top) r.top = t.top;
           if (t.right() > r.right()) r.width = t.right()-r.left;
           if (t.bottom() > r.bottom()) r.height = t.bottom()-r.top;
       }
    }
    mCheckedPadding = true;
    rect=mConstantPadding;
    return true;;
}

std::shared_ptr<DrawableContainer::DrawableContainerState> DrawableContainer::cloneConstantState(){
    return std::make_shared<DrawableContainerState>(mDrawableContainerState.get(),this);
}

void DrawableContainer::setConstantState(std::shared_ptr<DrawableContainerState>state){
     mDrawableContainerState=state;
     // The locally cached drawables may have changed.
     if (mCurIndex >= 0) {
         mCurrDrawable = state->getChild(mCurIndex);
         if (mCurrDrawable != nullptr) {
            initializeDrawableForDisplay(mCurrDrawable);
         }
     }
}

Drawable*DrawableContainer::mutate(){
    if (!mMutated && Drawable::mutate() == this) {
        std::shared_ptr<DrawableContainerState> clone=cloneConstantState();
        LOGD("DrawableContainerState %p cloned  from %p",clone.get(),mDrawableContainerState.get());
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

Drawable* DrawableContainer::DrawableContainerState::prepareDrawable(Drawable* child) {
    child->setLayoutDirection(mLayoutDirection);
    child = child->mutate();
    child->setCallback(mOwner);
    return child;
}

void DrawableContainer::DrawableContainerState::createAllFutures(){
    if (mDrawableFutures.size()) {
        int futureCount = mDrawableFutures.size();
        for (int keyIndex = 0; keyIndex < futureCount; keyIndex++) {
            auto cs = mDrawableFutures[keyIndex];
            mDrawables[keyIndex] = prepareDrawable(cs->newDrawable());
        }
        mDrawableFutures.clear();//=nullptr;
    }
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
///////////////////////////////////////////////////////////////////////////////////////////////////////
DrawableContainer::DrawableContainer(){
    mDrawableContainerState=std::make_shared<DrawableContainerState>(nullptr,this);
    mHasAlpha =false;
    mCurIndex = mLastIndex = -1;
    mCurrDrawable = mLastDrawable = nullptr;
    mBlockInvalidateCallback = nullptr;
    mExitAnimationEnd =0;
    mEnterAnimationEnd=0;
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
       result = (r.left | r.top | r.width | r.height) != 0;
    } else {
       if (mCurrDrawable != nullptr) {
           result = mCurrDrawable->getPadding(padding);
       } else {
           result = Drawable::getPadding(padding);
       }
    }
    if (needsMirroring()) {
        const int left = padding.left;
        const int right = padding.top;
        padding.left = right;
        padding.top = left;
    }
    return result;
}

Insets DrawableContainer::getOpticalInsets(){

    return  mCurrDrawable?mCurrDrawable->getOpticalInsets():Insets();
}

int DrawableContainer::getChangingConfigurations()const{
    return Drawable::getChangingConfigurations()
                | mDrawableContainerState->getChangingConfigurations();
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

bool DrawableContainer::isAutoMirrored(){
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

int DrawableContainer::getIntrinsicWidth()const {
    LOGV("%p constSize=%d cur=%p size=%d",this,mDrawableContainerState->isConstantSize(),mCurrDrawable,
        (mCurrDrawable?mCurrDrawable->getIntrinsicWidth():-1));
    if (mDrawableContainerState->isConstantSize()) {
        return mDrawableContainerState->getConstantWidth();
    }
    return mCurrDrawable != nullptr ? mCurrDrawable->getIntrinsicWidth() : -1;
}

int DrawableContainer::getIntrinsicHeight()const{
    if (mDrawableContainerState->isConstantSize()) {
        return mDrawableContainerState->getConstantHeight();
    }
    return mCurrDrawable != nullptr ? mCurrDrawable->getIntrinsicHeight() : -1;
}

int DrawableContainer::getMinimumWidth()const{
    if (mDrawableContainerState->isConstantSize()) {
        return mDrawableContainerState->getConstantMinimumWidth();
    }
    return mCurrDrawable != nullptr ? mCurrDrawable->getMinimumWidth() : 0;
}

int DrawableContainer::getMinimumHeight()const{
    if (mDrawableContainerState->isConstantSize()) {
        return mDrawableContainerState->getConstantMinimumHeight();
    }
    return mCurrDrawable != nullptr ? mCurrDrawable->getMinimumHeight() : 0;
}

int DrawableContainer::getCurrentIndex()const{
    return mCurIndex;
}

void DrawableContainer::initializeDrawableForDisplay(Drawable*d){
    if (mBlockInvalidateCallback == nullptr)
        mBlockInvalidateCallback = new BlockInvalidateCallback();

    d->setCallback(mBlockInvalidateCallback->wrap(d->getCallback()));

    d->setVisible(isVisible(), true);
    //d->setDither(mDither);
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
    if (mCurrDrawable != nullptr) {
        mCurrDrawable->setVisible(false, false);
    }
    if(index>=0&&index<getChildCount()){
        mCurIndex = index;
        mCurrDrawable = mDrawableContainerState->getChild(index);
        initializeDrawableForDisplay(mCurrDrawable);
        return true;
    }
    return false;
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
    return mDrawableContainerState->mDrawables.at(index);
}

void DrawableContainer::invalidateDrawable(Drawable& who){
    if (mDrawableContainerState != nullptr) {
        mDrawableContainerState->invalidateCache();
    }

    if (&who == mCurrDrawable && getCallback() != nullptr) {
        getCallback()->invalidateDrawable(*this);
    }
}

void DrawableContainer::scheduleDrawable(Drawable&who,Runnable what, long when){
    if (mCallback != nullptr)
        mCallback->scheduleDrawable(who, what, when);
}

void DrawableContainer::unscheduleDrawable(Drawable& who,Runnable what){
    if (mCallback != nullptr) 
        mCallback->unscheduleDrawable(who, what);
}

void DrawableContainer::draw(Canvas&canvas){
    if(mCurrDrawable!=nullptr)
        mCurrDrawable->draw(canvas);
    if (mLastDrawable != nullptr) 
        mLastDrawable->draw(canvas);
}

}
