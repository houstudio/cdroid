#include <widget/progressbar.h>
#include <animation/objectanimator.h>
#include <cdlog.h>

#define  MAX_LEVEL  10000
namespace cdroid{

class ProgressTintInfo{
public:
    ColorStateList* mIndeterminateTintList;
    PorterDuffMode mIndeterminateTintMode;
    bool mHasIndeterminateTint;
    bool mHasIndeterminateTintMode;

    ColorStateList* mProgressTintList;
    PorterDuffMode mProgressTintMode;
    bool mHasProgressTint;
    bool mHasProgressTintMode;

    ColorStateList* mProgressBackgroundTintList;
    PorterDuffMode mProgressBackgroundTintMode;
    bool mHasProgressBackgroundTint;
    bool mHasProgressBackgroundTintMode;

    ColorStateList* mSecondaryProgressTintList;
    PorterDuffMode mSecondaryProgressTintMode;
    bool mHasSecondaryProgressTint;
    bool mHasSecondaryProgressTintMode;
};

ProgressBar::ProgressBar(Context*ctx,const AttributeSet& attrs)
  :View(ctx,attrs){
    initProgressBar();
    mOnlyIndeterminate=attrs.getBoolean("indeterminateOnly",mOnlyIndeterminate);
    setIndeterminate(attrs.getBoolean("indeterminate",false));
    setProgressDrawable(ctx->getDrawable(attrs.getString("progressDrawable")));
    setIndeterminateDrawable(ctx->getDrawable(attrs.getString("indeterminateDrawable")));

    setProgress(attrs.getInt("progress",mProgress));
    setSecondaryProgress(attrs.getInt("secondaryProgress",mSecondaryProgress));

    mDuration = attrs.getInt("indeterminateDuration",mDuration);
    mMinWidth = attrs.getDimensionPixelSize("minWidth", mMinWidth);
    mMaxWidth = attrs.getDimensionPixelSize("maxWidth", mMaxWidth);
    mMinHeight= attrs.getDimensionPixelSize("minHeight", mMinHeight);
    mMaxHeight= attrs.getDimensionPixelSize("maxHeight", mMaxHeight);
    mMirrorForRtl =attrs.getBoolean("mirrorForRtl",false); 

    setMin(attrs.getInt("min",mMin));
    setMax(attrs.getInt("max",mMax));
}

ProgressBar::ProgressBar(int width, int height):View(width,height){
    initProgressBar();
    mMirrorForRtl=false;
    indeterminatePos=0;
    mHasAnimation=false;
    mAttached=false;
    mRefreshIsPosted=false;
    mShouldStartAnimationDrawable=false;
    setProgressDrawable(mContext->getDrawable("cdroid:drawable/progress_horizontal.xml"));
}

ProgressBar::~ProgressBar(){
    if(mProgressDrawable)mProgressDrawable->setCallback(nullptr);
    if(mIndeterminateDrawable)mIndeterminateDrawable->setCallback(nullptr);
    delete mProgressDrawable;
    delete mIndeterminateDrawable;
    delete mAnimator;
    delete mAnimation;
	delete mTransformation;
}

void ProgressBar::initProgressBar(){
    mMin = 0;
    mMax = 100;
    mProgress = 0;
    mSecondaryProgress = 0;
    mSampleWidth   = 0;
    mIndeterminate = false;
    mOnlyIndeterminate = false;
    mDuration = 4000;
    mProgressTintInfo=nullptr;
    //mBehavior = AlphaAnimation.RESTART;
    mMinWidth  = 48;
    mMaxWidth  = 96;
    mMinHeight = 48;
    mMaxHeight = 96;
    mCurrentDrawable = nullptr;
    mProgressDrawable= nullptr;
    mIndeterminateDrawable=nullptr;
    mAnimator = nullptr;
    mAnimation= nullptr;
    mTransformation=nullptr;
    mHasAnimation= false;
    mInDrawing   = false;
    mRefreshIsPosted =false;
    mData.push_back(RefreshData());
    mData.push_back(RefreshData());
}

void ProgressBar::setMin(int value){
    if(mMin!=value){
        mMin=value;
        invalidate(true);
    }
}

void ProgressBar::setMax(int value){
    if(mMax!=value){
        mMax=value;
        invalidate(true);
    }
}

void ProgressBar::setRange(int vmin,int vmax){
    if( (mMin!=vmin)||(mMax!=vmax)){
        mMin=vmin;
        mMax=vmax;
        invalidate(true);
    }
}

Drawable*ProgressBar::getCurrentDrawable()const{
    return mCurrentDrawable;
}

void ProgressBar::drawableStateChanged(){
    View::drawableStateChanged();
    updateDrawableState();
}

void ProgressBar::drawableHotspotChanged(float x, float y){
    View::drawableHotspotChanged(x,y);
    if(mProgressDrawable)mProgressDrawable->setHotspot(x,y);
    if(mIndeterminateDrawable)mIndeterminateDrawable->setHotspot(x,y); 
}

bool ProgressBar::verifyDrawable(Drawable* who)const{
    return who == mProgressDrawable || who == mIndeterminateDrawable|| View::verifyDrawable(who);
}

void ProgressBar::onVisualProgressChanged(int id, float progress){
}

void ProgressBar::onAttachedToWindow(){
    View::onAttachedToWindow();
    mAttached = true;
    if (mIndeterminate) startAnimation();

    const int count = mData.size();
    for (int i = 0; i < count&&i<1; i++) {
        RefreshData rd = mData[i];
        doRefreshProgress(rd.id, rd.progress, rd.fromUser, true, rd.animate);
    }
}

void ProgressBar::onDetachedFromWindow(){
    if(mIndeterminate)stopAnimation();
    if(mRefreshProgressRunnable){
        removeCallbacks(mRefreshProgressRunnable);
        mRefreshIsPosted = false;
    }
    View::onDetachedFromWindow();
    mAttached =false;
}

void ProgressBar::setVisualProgress(int id, float progress){
    mVisualProgress = progress;

    Drawable* d = mCurrentDrawable;

    if (dynamic_cast<LayerDrawable*>(d)) {
        d = ((LayerDrawable*) d)->findDrawableByLayerId(id);
        if (d == nullptr) {
            // If we can't find the requested layer, fall back to setting
            // the level of the entire drawable. This will break if
            // progress is set on multiple elements, but the theme-default
            // drawable will always have all layer IDs present.
            d = mCurrentDrawable;
        }
    }
    if (d) d->setLevel((progress * MAX_LEVEL));
    else invalidate(true);
    onVisualProgressChanged(id, progress);
}

void ProgressBar::doRefreshProgress(int id, int progress, bool fromUser,bool callBackToApp, bool animate){
    int range = mMax - mMin;
    const float scale = range > 0 ? (float)(progress - mMin) / (float) range : 0;
    const bool isPrimary = id == ID_PRIMARY;//R.id.progress;

    if (isPrimary && animate) {
        FloatPropertyValuesHolder*prop=nullptr;
        if(mAnimator==nullptr){
            mAnimator = ObjectAnimator::ofFloat(this,"progress",{scale});
            mAnimator->setAutoCancel(true);
            mAnimator->setDuration(PROGRESS_ANIM_DURATION);
            mAnimator->setInterpolator(new  DecelerateInterpolator());
            mAnimator->addUpdateListener(ValueAnimator::AnimatorUpdateListener([this](ValueAnimator&anim){
                FloatPropertyValuesHolder*fp=(FloatPropertyValuesHolder*)anim.getValues(0);
                setVisualProgress(ID_PRIMARY,fp->getAnimatedValue());
            }));
        }
        prop=(FloatPropertyValuesHolder*)mAnimator->getValues(0);
        prop->setValues({scale});
        mAnimator->start();
    } else {
        setVisualProgress(id, scale);
    }
    if (isPrimary && callBackToApp) {
        onProgressRefresh(scale, fromUser, progress);
    }
}

void ProgressBar::onMeasure(int widthMeasureSpec, int heightMeasureSpec){
    int dw = 0;
    int dh = 0;

    Drawable* d = mCurrentDrawable;
    if (d != nullptr) {
        dw = std::max(mMinWidth, std::min(mMaxWidth, d->getIntrinsicWidth()));
        dh = std::max(mMinHeight, std::min(mMaxHeight, d->getIntrinsicHeight()));
        LOGD("%p drawable.size=%dx%d",this,d->getIntrinsicWidth(),d->getIntrinsicHeight());
    }
    updateDrawableState();

    dw += mPaddingLeft + mPaddingRight;
    dh += mPaddingTop + mPaddingBottom;

    int measuredWidth = resolveSizeAndState(dw, widthMeasureSpec, 0);
    int measuredHeight = resolveSizeAndState(dh, heightMeasureSpec, 0);
    setMeasuredDimension(measuredWidth, measuredHeight);
}

void ProgressBar::refreshProgress(int id, int progress, bool fromUser,bool animate){
    RefreshData&rd=mData[id-1];
    rd.progress=progress;
    rd.fromUser=fromUser;
    rd.animate=animate;
    //doRefreshProgress(id, progress, fromUser, true, animate);
    //return ;
    if(!mRefreshIsPosted){
        if(mAttached&&!mRefreshIsPosted){
            mRefreshProgressRunnable =[this](){
                for(int i=ID_PRIMARY;i<=ID_SECONDARY;i++){
                    RefreshData&rd=mData[i-1];
                    doRefreshProgress(i, rd.progress, rd.fromUser, true, rd.animate);
                }
                mRefreshIsPosted=false;
            };
            post(mRefreshProgressRunnable);
            mRefreshIsPosted=true;
        }
    }
}

bool ProgressBar::setProgressInternal(int value, bool fromUser,bool animate){
    if(mIndeterminate)return false;
    if(value<mMin)value=mMin;
    if(value>mMax)value=mMax;
    if(mProgress==value)return false;
    mProgress=value;
    refreshProgress(ID_PRIMARY,mProgress,fromUser,value);
    return true;
}

void ProgressBar::setProgress(int value){
    setProgressInternal(value,FALSE);
}

void ProgressBar::setSecondaryProgress(int secondaryProgress) {
    if (mIndeterminate) return;

    if (secondaryProgress < mMin) secondaryProgress = mMin;

    if (secondaryProgress > mMax) secondaryProgress = mMax;

    if (secondaryProgress != mSecondaryProgress) {
        mSecondaryProgress = secondaryProgress;
        refreshProgress(ID_SECONDARY, mSecondaryProgress, false, false);
    }
}

void ProgressBar::incrementProgressBy(int diff) {
    setProgress(mProgress + diff);
}

void ProgressBar::incrementSecondaryProgressBy(int diff){
    setSecondaryProgress(mSecondaryProgress + diff);
}

int ProgressBar::getProgress()const{
    return mProgress;
}

bool ProgressBar::isIndeterminate()const{
    return mIndeterminate;
}

void ProgressBar::setIndeterminate(bool indeterminate){
    if ((!mOnlyIndeterminate || !mIndeterminate) && indeterminate != mIndeterminate) {
        mIndeterminate = indeterminate;

        if (indeterminate) {
            // swap between indeterminate and regular backgrounds
            swapCurrentDrawable(mIndeterminateDrawable);
            startAnimation();
        } else {
            swapCurrentDrawable(mProgressDrawable);
            stopAnimation();
        }
    }
}

void ProgressBar::setProgressDrawable(Drawable*d){
    if (mProgressDrawable != d) {
        if (mProgressDrawable != nullptr) {
            mProgressDrawable->setCallback(nullptr);
            unscheduleDrawable(*mProgressDrawable);
            if(mProgressDrawable==mCurrentDrawable)
                mCurrentDrawable=nullptr;
            delete mProgressDrawable;
        }

        mProgressDrawable = d;

        if (d != nullptr) {
            d->setCallback(this);
            d->setLayoutDirection(getLayoutDirection());
            if (d->isStateful())d->setState(getDrawableState());

            // Make sure the ProgressBar is always tall enough
            int drawableHeight = d->getMinimumHeight();
            if (mMaxHeight < drawableHeight) {
                mMaxHeight = drawableHeight;
                requestLayout();
            }
            applyProgressTints();
        }

        if (!mIndeterminate) {
            swapCurrentDrawable(d);
            postInvalidate();
        }

        updateDrawableBounds(getWidth(), getHeight());
        updateDrawableState();

        doRefreshProgress(ID_PRIMARY, mProgress, false, false, false);
        doRefreshProgress(ID_SECONDARY , mSecondaryProgress, false, false, false);
    }
}

Drawable*ProgressBar::getProgressDrawable()const{
    return mProgressDrawable;
}

void ProgressBar::setIndeterminateDrawable(Drawable*d){
    if (mIndeterminateDrawable != d) {
        if (mIndeterminateDrawable != nullptr) {
            mIndeterminateDrawable->setCallback(nullptr);
            unscheduleDrawable(*mIndeterminateDrawable);
            delete mIndeterminateDrawable;
        }

        mIndeterminateDrawable = d;

        if (d != nullptr) {
            LOGD("drawable %p setcbk-->%p",d,this);
            d->setCallback(this);
            d->setLayoutDirection(getLayoutDirection());
            if (d->isStateful()) {
                d->setState(getDrawableState());
            }
            //applyIndeterminateTint();
        }

        if (mIndeterminate) {
            swapCurrentDrawable(d);
            postInvalidate();
        }
    }
}

Drawable*ProgressBar::getIndeterminateDrawable()const{
    return mIndeterminateDrawable;
}

void ProgressBar::swapCurrentDrawable(Drawable*newDrawable){
    Drawable* oldDrawable = mCurrentDrawable;
    mCurrentDrawable = newDrawable;
    if (oldDrawable != mCurrentDrawable) {
        if (oldDrawable ) oldDrawable->setVisible(false, false);
        if (mCurrentDrawable)
            mCurrentDrawable->setVisible(getVisibility() == VISIBLE, false);
    }
}

void ProgressBar::updateDrawableState(){
    const std::vector<int> state = getDrawableState();
    bool changed = false;

    if (mProgressDrawable  && mProgressDrawable->isStateful()) {
        changed |= mProgressDrawable->setState(state);
    }

    if (mIndeterminateDrawable && mIndeterminateDrawable->isStateful()) {
        changed |= mIndeterminateDrawable->setState(state);
    }

    if (changed) postInvalidate();
}

void ProgressBar::startAnimation() {
    if (getVisibility() != VISIBLE){// || getWindowVisibility() != VISIBLE) {
        return;
    }
    if (dynamic_cast<Animatable*>(mIndeterminateDrawable)) {
        mShouldStartAnimationDrawable = true;
        mHasAnimation=false;
    }else{
        mHasAnimation = true;
        if (mInterpolator == nullptr) {
            mInterpolator = new LinearInterpolator();
        }

        if (mTransformation == nullptr) {
            mTransformation = new Transformation();
        } else {
            mTransformation->clear();
        }

        if (mAnimation == nullptr) {
            mAnimation = new AlphaAnimation(0.0f, 1.0f);
        } else {
            mAnimation->reset();
        }

        mAnimation->setRepeatMode(mBehavior);
        mAnimation->setRepeatCount(Animation::INFINITE);
        mAnimation->setDuration(mDuration);
        mAnimation->setInterpolator(mInterpolator);
        mAnimation->setStartTime(Animation::START_ON_FIRST_FRAME);
    }
    LOGD("mIndeterminateDrawable=%p",mIndeterminateDrawable);
    postInvalidate();
}

void ProgressBar::stopAnimation() {
    if (dynamic_cast<Animatable*>(mIndeterminateDrawable)) {
        ((Animatable*) mIndeterminateDrawable)->stop();
        mShouldStartAnimationDrawable = false;
    }
    postInvalidate();
}

void ProgressBar::drawTrack(Canvas&canvas){
    Drawable* d = mCurrentDrawable;
    if (d == nullptr)return;
    // Translate canvas so a indeterminate circular progress bar with padding
    // rotates properly in its animation
    canvas.save();

    if (isLayoutRtl() && mMirrorForRtl) {
        canvas.translate(getWidth() - mPaddingRight, mPaddingTop);
        canvas.scale(-1.0f, 1.0f);
    } else {
        canvas.translate(mPaddingLeft, mPaddingTop);
    }

    const long time = getDrawingTime();
    if (mHasAnimation) {
        mAnimation->getTransformation(time, *mTransformation);
        const float scale = mTransformation->getAlpha();
        mInDrawing = true;
        d->setLevel((int) (scale * MAX_LEVEL));
        mInDrawing = false;
        postInvalidateOnAnimation();
    }
    d->draw(canvas);
    canvas.restore();

    if (mShouldStartAnimationDrawable && dynamic_cast<Animatable*>(d)) {
        ((AnimatedRotateDrawable*) d)->start();
        mShouldStartAnimationDrawable = false;
    }
}

void ProgressBar::updateDrawableBounds(int w,int h){
    w -= mPaddingRight + mPaddingLeft;
    h -= mPaddingTop + mPaddingBottom;
    int right = w;
    int bottom = h;

    LOGV("mIndeterminateDrawable=%p,mOnlyIndeterminate=%d",mIndeterminateDrawable,mOnlyIndeterminate);
    if (mIndeterminateDrawable) {
        int top = 0;
        int left = 0;
        // Aspect ratio logic does not apply to AnimationDrawables
        if (mOnlyIndeterminate /*&& !(mIndeterminateDrawable instanceof AnimationDrawable)*/) {
            // Maintain aspect ratio. Certain kinds of animated drawables
            // get very confused otherwise.
            const int intrinsicWidth = mIndeterminateDrawable->getIntrinsicWidth();
            const int intrinsicHeight = mIndeterminateDrawable->getIntrinsicHeight();
            const float intrinsicAspect = (float) intrinsicWidth / intrinsicHeight;
            const float boundAspect = (float) w / h;
            if (intrinsicAspect != boundAspect) {
                if (boundAspect > intrinsicAspect) {
                    // New width is larger. Make it smaller to match height.
                    const int width = (int) (h * intrinsicAspect);
                    left = (w - width) / 2;
                    right = left + width;
                } else {
                    // New height is larger. Make it smaller to match width.
                    const int height = (int) (w * (1 / intrinsicAspect));
                    top = (h - height) / 2;
                    bottom = top + height;
                }
            }LOGV("intrinsicsize=%dx%d",intrinsicWidth,intrinsicHeight);
        }
        if (isLayoutRtl() && mMirrorForRtl) {
            int tempLeft = left;
            left = w - right;
            right = w - tempLeft;
        }
        LOGD("%p setBounds(%d,%d-%d,%d) wh=%dx%d",this,left, top, right, bottom,w,h);
        mIndeterminateDrawable->setBounds(left, top, right-left, bottom-top);
    }

    if (mProgressDrawable != nullptr) {
        mProgressDrawable->setBounds(0, 0, right, bottom);
    }
}

void ProgressBar::onSizeChanged(int w,int h,int ow,int oh){
    updateDrawableBounds(w,h);
}

void ProgressBar::onDraw(Canvas&canvas) {
    View::onDraw(canvas);
    drawTrack(canvas);
}

void ProgressBar::setMirrorForRtl(bool mirrorRtl){
    if(mMirrorForRtl!=mirrorRtl){
        mMirrorForRtl = mirrorRtl;
        invalidate(true);
    }
}

bool ProgressBar::getMirrorForRtl()const{
    return mMirrorForRtl;
}

void ProgressBar::applyProgressTints() {
    if (mProgressDrawable && mProgressTintInfo) {
        applyPrimaryProgressTint();
        applyProgressBackgroundTint();
        applySecondaryProgressTint();
    }
}

void ProgressBar::applyPrimaryProgressTint(){
    if (mProgressTintInfo->mHasProgressTint
          || mProgressTintInfo->mHasProgressTintMode) {
        Drawable* target = getTintTarget(ID_PRIMARY/*R.id.progress*/, true);
        if (target != nullptr) {
            if (mProgressTintInfo->mHasProgressTint) {
                target->setTintList(mProgressTintInfo->mProgressTintList);
            }
            if (mProgressTintInfo->mHasProgressTintMode) {
                target->setTintMode(mProgressTintInfo->mProgressTintMode);
            }

            // The drawable (or one of its children) may not have been
            // stateful before applying the tint, so let's try again.
            if (target->isStateful()) {
                target->setState(getDrawableState());
            }
        }
    }
}

void ProgressBar::applyProgressBackgroundTint(){
     if (mProgressTintInfo->mHasProgressBackgroundTint
            || mProgressTintInfo->mHasProgressBackgroundTintMode) {
        Drawable* target = getTintTarget(ID_BACKGROUND/*R.id.background*/, false);
        if (target != nullptr) {
            if (mProgressTintInfo->mHasProgressBackgroundTint) {
                target->setTintList(mProgressTintInfo->mProgressBackgroundTintList);
            }
            if (mProgressTintInfo->mHasProgressBackgroundTintMode) {
                target->setTintMode(mProgressTintInfo->mProgressBackgroundTintMode);
            }

            // The drawable (or one of its children) may not have been
            // stateful before applying the tint, so let's try again.
            if (target->isStateful()) {
                target->setState(getDrawableState());
            }
        }
    }
}

void ProgressBar::applySecondaryProgressTint(){
     if (mProgressTintInfo->mHasSecondaryProgressTint
            || mProgressTintInfo->mHasSecondaryProgressTintMode) {
        Drawable* target = getTintTarget(ID_SECONDARY/*R.id.secondaryProgress*/, false);
            if (target != nullptr) {
                if (mProgressTintInfo->mHasSecondaryProgressTint) {
                    target->setTintList(mProgressTintInfo->mSecondaryProgressTintList);
                }
                if (mProgressTintInfo->mHasSecondaryProgressTintMode) {
                    target->setTintMode(mProgressTintInfo->mSecondaryProgressTintMode);
                }

                // The drawable (or one of its children) may not have been
                // stateful before applying the tint, so let's try again.
                if (target->isStateful()) {
                    target->setState(getDrawableState());
                }
            }
        }
}

void ProgressBar::setProgressTintList(ColorStateList*tint){
    if (mProgressTintInfo == nullptr) {
        mProgressTintInfo = new ProgressTintInfo();
    }
    mProgressTintInfo->mProgressTintList = tint;
    mProgressTintInfo->mHasProgressTint = true;

    if (mProgressDrawable ) {
        applyPrimaryProgressTint();
    }
}

ColorStateList* ProgressBar::getProgressTintList()const{
    return mProgressTintInfo  ? mProgressTintInfo->mProgressTintList : nullptr;
}

void ProgressBar::setProgressTintMode(int tintMode) {
    if (mProgressTintInfo == nullptr) {
        mProgressTintInfo = new ProgressTintInfo();
    }
    mProgressTintInfo->mProgressTintMode = (PorterDuffMode)tintMode;
    mProgressTintInfo->mHasProgressTintMode = true;

    if (mProgressDrawable) {
        applyPrimaryProgressTint();
    }
}

int ProgressBar::getProgressTintMode()const{
    return mProgressTintInfo ? mProgressTintInfo->mProgressTintMode : 0;
}

void ProgressBar::setProgressBackgroundTintList(ColorStateList* tint) {
    if (mProgressTintInfo == nullptr) {
        mProgressTintInfo = new ProgressTintInfo();
    }
    mProgressTintInfo->mProgressBackgroundTintList = tint;
    mProgressTintInfo->mHasProgressBackgroundTint = true;

    if (mProgressDrawable != nullptr) {
        applyProgressBackgroundTint();
    }
}

ColorStateList*ProgressBar::getProgressBackgroundTintList()const{
    return mProgressTintInfo ? mProgressTintInfo->mProgressBackgroundTintList : nullptr;
}

void ProgressBar::setProgressBackgroundTintMode(int tintMode) {
    if (mProgressTintInfo == nullptr) {
        mProgressTintInfo = new ProgressTintInfo();
    }
    mProgressTintInfo->mProgressBackgroundTintMode = (PorterDuffMode)tintMode;
    mProgressTintInfo->mHasProgressBackgroundTintMode = true;

    if (mProgressDrawable !=nullptr) {
        applyProgressBackgroundTint();
    }
}

int ProgressBar::getProgressBackgroundTintMode()const{
    return mProgressTintInfo  ? mProgressTintInfo->mProgressBackgroundTintMode :0;
}

void ProgressBar::setSecondaryProgressTintList(ColorStateList* tint) {
    if (mProgressTintInfo == nullptr) {
        mProgressTintInfo = new ProgressTintInfo();
    }
    mProgressTintInfo->mSecondaryProgressTintList = tint;
    mProgressTintInfo->mHasSecondaryProgressTint = true;

    if (mProgressDrawable != nullptr) {
        applySecondaryProgressTint();
    }
}

ColorStateList* ProgressBar::getSecondaryProgressTintList()const{
    return mProgressTintInfo ? mProgressTintInfo->mSecondaryProgressTintList : nullptr;
}

void ProgressBar::setSecondaryProgressTintMode(int tintMode) {
    if (mProgressTintInfo == nullptr) {
        mProgressTintInfo = new ProgressTintInfo();
    }
    mProgressTintInfo->mSecondaryProgressTintMode = (PorterDuffMode)tintMode;
    mProgressTintInfo->mHasSecondaryProgressTintMode = true;

    if (mProgressDrawable != nullptr) {
        applySecondaryProgressTint();
    }
}

int ProgressBar::getSecondaryProgressTintMode()const{
    return mProgressTintInfo ? mProgressTintInfo->mSecondaryProgressTintMode : 0;
}

Drawable* ProgressBar::getTintTarget(int layerId, bool shouldFallback){
    Drawable* layer = nullptr;

    Drawable* d = mProgressDrawable;
    if (d != nullptr) {
        mProgressDrawable = d->mutate();
        if (dynamic_cast<LayerDrawable*>(d)) {
            layer = ((LayerDrawable*) d)->findDrawableByLayerId(layerId);
        }
        if (shouldFallback && layer == nullptr) {
            layer = d;
        }
    }
    return layer;
}

void ProgressBar::setProgressDrawableTiled(Drawable* d) {
    /*if (d != nullptr) {
        d = tileify(d, false);
    }
    setProgressDrawable(d);*/
}

}//end namespace
