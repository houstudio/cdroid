#include <widget/progressbar.h>
#include <animation/objectanimator.h>
#include <widget/R.h>
#include <cdlog.h>

#define  MAX_LEVEL  10000
namespace cdroid{

class ProgressTintInfo{
public:
    ColorStateList* mIndeterminateTintList;
    PorterDuffMode mIndeterminateTintMode;
    bool mHasIndeterminateTint;
    bool mHasIndeterminateTintMode;

    const ColorStateList* mProgressTintList;
    PorterDuffMode mProgressTintMode;
    bool mHasProgressTint;
    bool mHasProgressTintMode;

    const ColorStateList* mProgressBackgroundTintList;
    PorterDuffMode mProgressBackgroundTintMode;
    bool mHasProgressBackgroundTint;
    bool mHasProgressBackgroundTintMode;

    const ColorStateList* mSecondaryProgressTintList;
    PorterDuffMode mSecondaryProgressTintMode;
    bool mHasSecondaryProgressTint;
    bool mHasSecondaryProgressTintMode;
public:
    ProgressTintInfo(){
        mProgressTintList = nullptr;
        mProgressBackgroundTintList = nullptr;
        mSecondaryProgressTintList = nullptr;
    }
};

DECLARE_WIDGET(ProgressBar)

ProgressBar::ProgressBar(Context*ctx,const AttributeSet& attrs)
  :View(ctx,attrs){
    initProgressBar();
    Drawable* progressDrawable = attrs.getDrawable("progressDrawable");
    if(progressDrawable){
        if(needsTileify(progressDrawable))
            setIndeterminateDrawableTiled(progressDrawable);
        else
            setIndeterminateDrawable(progressDrawable);
    }

    Drawable* indeterminateDrawable = attrs.getDrawable("indeterminateDrawable");
    if(indeterminateDrawable){
        if(needsTileify(indeterminateDrawable))
            setIndeterminateDrawableTiled(indeterminateDrawable);
        else
            setIndeterminateDrawable(indeterminateDrawable);
    }

    mDuration = attrs.getInt("indeterminateDuration",mDuration);
    mMinWidth = attrs.getDimensionPixelSize("minWidth", mMinWidth);
    mMaxWidth = attrs.getDimensionPixelSize("maxWidth", mMaxWidth);
    mMinHeight= attrs.getDimensionPixelSize("minHeight", mMinHeight);
    mMaxHeight= attrs.getDimensionPixelSize("maxHeight", mMaxHeight);
    mBehavior = attrs.getInt("inteterminateBehavior",std::map<const std::string,int>{
       {"none",0},{"repeat",(int)Animation::RESTART},{"cycle",(int)Animation::INFINITE} },mBehavior);

    mOnlyIndeterminate=attrs.getBoolean("indeterminateOnly",mOnlyIndeterminate);
    mNoInvalidate = false;
    setIndeterminate(mOnlyIndeterminate||attrs.getBoolean("indeterminate",mIndeterminate));

    mMirrorForRtl = attrs.getBoolean("mirrorForRtl",false); 

    setMin(attrs.getInt("min",mMin));
    setMax(attrs.getInt("max",mMax));

    setProgress(attrs.getInt("progress",mProgress));
    setSecondaryProgress(attrs.getInt("secondaryProgress",mSecondaryProgress));

    applyProgressTints();
    applyIndeterminateTint();
}

ProgressBar::ProgressBar(int width, int height):View(width,height){
    initProgressBar();
    mMirrorForRtl = false;
    mHasAnimation = false;
    indeterminatePos = 0;
    mAttached = false;
    mShouldStartAnimationDrawable = false;
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

bool ProgressBar::needsTileify(Drawable* dr){
    if (dynamic_cast<LayerDrawable*>(dr)) {
        LayerDrawable* orig = (LayerDrawable*) dr;
        const int N = orig->getNumberOfLayers();
        for (int i = 0; i < N; i++) {
            if (needsTileify(orig->getDrawable(i))) {
                return true;
            }
        }
        return false;
    }

    if (dynamic_cast<StateListDrawable*>(dr)) {
        StateListDrawable* in = (StateListDrawable*) dr;
        const int N = in->getStateCount();
        for (int i = 0; i < N; i++) {
            if (needsTileify(in->getStateDrawable(i))) {
                return true;
             }
        }
        return false;
    }
    // If there's a bitmap that's not wrapped with a ClipDrawable or
    // ScaleDrawable, we'll need to wrap it and apply tiling.
    if (dynamic_cast<BitmapDrawable*>(dr)) {
        return true;
    }
    return false;
}

Drawable* ProgressBar::tileify(Drawable* drawable, bool clip){
    // TODO: This is a terrible idea that potentially destroys any drawable
    // that extends any of these classes. We *really* need to remove this.

    if (dynamic_cast<LayerDrawable*>(drawable)) {
        LayerDrawable* orig = (LayerDrawable*) drawable;
        const int N = orig->getNumberOfLayers();
        std::vector<Drawable*> outDrawables;

        for (int i = 0; i < N; i++) {
            const int id = orig->getId(i);
            Drawable*dr=tileify(orig->getDrawable(i),(id == R::id::progress || id == R::id::secondaryProgress));
            outDrawables.push_back(dr); 
        }

        LayerDrawable* clone = new LayerDrawable(outDrawables);
        for (int i = 0; i < N; i++) {
            clone->setId(i, orig->getId(i));
            clone->setLayerGravity(i, orig->getLayerGravity(i));
            clone->setLayerWidth(i, orig->getLayerWidth(i));
            clone->setLayerHeight(i, orig->getLayerHeight(i));
            clone->setLayerInsetLeft(i, orig->getLayerInsetLeft(i));
            clone->setLayerInsetRight(i, orig->getLayerInsetRight(i));
            clone->setLayerInsetTop(i, orig->getLayerInsetTop(i));
            clone->setLayerInsetBottom(i, orig->getLayerInsetBottom(i));
            clone->setLayerInsetStart(i, orig->getLayerInsetStart(i));
            clone->setLayerInsetEnd(i, orig->getLayerInsetEnd(i));
        }
        return clone;
    }

    if (dynamic_cast<StateListDrawable*>(drawable)) {
        StateListDrawable* in = (StateListDrawable*) drawable;
        StateListDrawable* out = new StateListDrawable();
        const int N = in->getStateCount();
        for (int i = 0; i < N; i++) {
            out->addState(in->getStateSet(i), tileify(in->getStateDrawable(i), clip));
        }

        return out;
    }

    if (dynamic_cast<BitmapDrawable*>(drawable)) {
        std::shared_ptr<Drawable::ConstantState> cs = drawable->getConstantState();
        BitmapDrawable* clone = (BitmapDrawable*) cs->newDrawable();
        clone->setTileModeXY(TileMode::REPEAT, TileMode::CLAMP);

        if (mSampleWidth <= 0) {
            mSampleWidth = clone->getIntrinsicWidth();
        }
        if (clip) {
            return new ClipDrawable(clone, Gravity::LEFT,ClipDrawable::HORIZONTAL);
        } else {
            return clone;
        }
    }
    return drawable;
}

Drawable* ProgressBar::tileifyIndeterminate(Drawable* drawable){
    if (dynamic_cast<AnimationDrawable*>(drawable)) {
        AnimationDrawable* background = (AnimationDrawable*) drawable;
        const int N = background->getNumberOfFrames();
        AnimationDrawable* newBg = new AnimationDrawable();
        newBg->setOneShot(background->isOneShot());
        for (int i = 0; i < N; i++) {
            Drawable* frame = tileify(background->getFrame(i), true);
            frame->setLevel(10000);
            newBg->addFrame(frame, background->getDuration(i));
        }
        newBg->setLevel(10000);
        drawable = newBg;
    }
    return drawable;
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
    mProgressTintInfo= nullptr;
    mBehavior = AlphaAnimation::RESTART;
    mMinWidth  = 24;
    mMaxWidth  = 96;
    mMinHeight = 24;
    mMaxHeight = 96;
    mNoInvalidate = true;
    mCurrentDrawable = nullptr;
    mProgressDrawable= nullptr;
    mIndeterminateDrawable = nullptr;
    mAnimator = nullptr;
    mAnimation= nullptr;
    mInterpolator  = nullptr;
    mTransformation= nullptr;
    mHasAnimation = false;
    mInDrawing    = false;
    mAggregatedIsVisible = false;
    mShouldStartAnimationDrawable = false;
    mRefreshIsPosted = false;
    mDatas.insert(std::pair<int,RefreshData>(R::id::progress,RefreshData()));
    mDatas.insert(std::pair<int,RefreshData>(R::id::secondaryProgress,RefreshData()));
}

void ProgressBar::setMin(int value){
    if(mMin!=value){
        mMin = value;
        invalidate(true);
    }
}

void ProgressBar::setMax(int value){
    if(mMax!=value){
        mMax = value;
        invalidate(true);
    }
}

void ProgressBar::setRange(int vmin,int vmax){
    if( (mMin!=vmin)||(mMax!=vmax)){
        mMin = vmin;
        mMax = vmax;
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

void ProgressBar::jumpDrawablesToCurrentState() {
    View::jumpDrawablesToCurrentState();
    if (mProgressDrawable) mProgressDrawable->jumpToCurrentState();
    if (mIndeterminateDrawable) mIndeterminateDrawable->jumpToCurrentState();
}

void ProgressBar::onResolveDrawables(int layoutDirection){
    Drawable* d = mCurrentDrawable;
    if (d != nullptr) {
        d->setLayoutDirection(layoutDirection);
    }
    if (mIndeterminateDrawable != nullptr) {
        mIndeterminateDrawable->setLayoutDirection(layoutDirection);
    }
    if (mProgressDrawable != nullptr) {
        mProgressDrawable->setLayoutDirection(layoutDirection);
    }
}

void ProgressBar::onVisualProgressChanged(int id, float progress){
}

void ProgressBar::onAttachedToWindow(){
    View::onAttachedToWindow();
    mAttached = true;
    if (mIndeterminate) startAnimation();

    for (auto d:mDatas) {
        RefreshData& rd = d.second;
        doRefreshProgress(d.first, rd.progress, rd.fromUser, true, rd.animate);
    }
}

void ProgressBar::onDetachedFromWindow(){
    if(mIndeterminate)stopAnimation();
    if(mRefreshProgressRunnable){
        removeCallbacks(mRefreshProgressRunnable);
        mRefreshIsPosted = false;
    }
    View::onDetachedFromWindow();
    if(mCurrentDrawable)unscheduleDrawable(*mCurrentDrawable);
    if(mProgressDrawable)unscheduleDrawable(*mProgressDrawable);
    mAttached =false;
}

void ProgressBar::setVisualProgress(int id, float progress){
    mVisualProgress = progress;

    Drawable* d = mCurrentDrawable;
    if (dynamic_cast<LayerDrawable*>(d)) {
        d = ((LayerDrawable*) d)->findDrawableByLayerId(id);
        LOGD_IF((d==nullptr)&&(id==R::id::progress),"LayerDrawable lost layer id=%d",id);
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
    const bool isPrimary = id == R::id::progress;
    LOGV_IF(isPrimary,"setProgress %d->%d animate=%d",id,progress,animate);
    if (isPrimary && animate) {
        Animator::AnimatorListener animListener;
        if(mAnimator==nullptr){
            mAnimator = ObjectAnimator::ofFloat(this,"progress",{mVisualProgress,scale});
            mAnimator->setAutoCancel(true);
            mAnimator->setDuration(PROGRESS_ANIM_DURATION);
            mAnimator->setInterpolator(DecelerateInterpolator::gDecelerateInterpolator.get());
            mAnimator->addUpdateListener(ValueAnimator::AnimatorUpdateListener([this](ValueAnimator&anim){
                setVisualProgress(R::id::progress,GET_VARIANT(anim.getAnimatedValue(),float));
            }));
	    mAnimator->addListener(mAnimtorListener);
        }
	mAnimator->end();
        mAnimator->getValues(0)->setValues(std::vector<float>({mVisualProgress,scale}));
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
        LOGV("%p:%d drawable.size=%dx%d",this,mID,d->getIntrinsicWidth(),d->getIntrinsicHeight());
    }
    updateDrawableState();

    dw += mPaddingLeft + mPaddingRight;
    dh += mPaddingTop + mPaddingBottom;

    const int measuredWidth  = resolveSizeAndState(dw, widthMeasureSpec, 0);
    const int measuredHeight = resolveSizeAndState(dh, heightMeasureSpec, 0);
    setMeasuredDimension(measuredWidth, measuredHeight);
}

void ProgressBar::refreshProgress(int id, int progress, bool fromUser,bool animate){
    RefreshData&rd= mDatas[id];
    rd.progress = progress;
    rd.fromUser = fromUser;
    rd.animate  = animate;
    doRefreshProgress(id, progress, fromUser, true, animate);
}

bool ProgressBar::setProgressInternal(int value, bool fromUser,bool animate){
    if(mIndeterminate)return false;
    if(value < mMin)value = mMin;
    if(value > mMax)value = mMax;
    if(mProgress == value)return false;
    mProgress = value;
    refreshProgress(R::id::progress,mProgress,fromUser,animate);
    return true;
}

void ProgressBar::setProgress(int value){
    setProgressInternal(value,false,false);
}

void ProgressBar::setProgress(int progress, bool animate){
    setProgressInternal(progress, false, animate);
}

void ProgressBar::setSecondaryProgress(int secondaryProgress) {
    if (mIndeterminate) return;

    if (secondaryProgress < mMin) secondaryProgress = mMin;

    if (secondaryProgress > mMax) secondaryProgress = mMax;

    if (secondaryProgress != mSecondaryProgress) {
        mSecondaryProgress = secondaryProgress;
        refreshProgress(R::id::secondaryProgress, mSecondaryProgress, false, false);
    }
}

int ProgressBar::getSecondaryProgress()const{
    return mIndeterminate ? 0 : mSecondaryProgress;
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
        } else if(mProgressDrawable){
            swapCurrentDrawable(mProgressDrawable);
            stopAnimation();
        }else{
            stopAnimation();
        }
    }
}

void ProgressBar::setProgressDrawable(Drawable*d){
    if (mProgressDrawable != d) {
        if (mProgressDrawable != nullptr) {
            mProgressDrawable->setCallback(nullptr);
            unscheduleDrawable(*mProgressDrawable);
            if(mProgressDrawable == mCurrentDrawable)
                mCurrentDrawable = nullptr;
            delete mProgressDrawable;
        }

        mProgressDrawable = d;

        if (d != nullptr) {
            d->setCallback(this);
            d->setLayoutDirection(getLayoutDirection());
            if (d->isStateful())d->setState(getDrawableState());

            // Make sure the ProgressBar is always tall enough
            const int drawableHeight = d->getMinimumHeight();
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

        doRefreshProgress(R::id::progress, mProgress, false, false, false);
        doRefreshProgress(R::id::secondaryProgress , mSecondaryProgress, false, false, false);
    }
}

Drawable*ProgressBar::getProgressDrawable()const{
    return mProgressDrawable;
}

int ProgressBar::getProgressGravity()const{
    int gravity = Gravity::NO_GRAVITY; 
    if(dynamic_cast<LayerDrawable*>(mProgressDrawable)){
        LayerDrawable*ld = (LayerDrawable*)mProgressDrawable;
        ClipDrawable*cd = dynamic_cast<ClipDrawable*>(ld->findDrawableByLayerId(R::id::progress));
        ScaleDrawable*sd= dynamic_cast<ScaleDrawable*>(ld->findDrawableByLayerId(R::id::progress));
        if(cd == nullptr && sd==nullptr){
            cd = dynamic_cast<ClipDrawable*>(ld->findDrawableByLayerId(R::id::secondaryProgress));
            sd = dynamic_cast<ScaleDrawable*>(ld->findDrawableByLayerId(R::id::secondaryProgress));
        }
        if(cd)gravity = cd->getGravity();
        if(sd)gravity = sd->getGravity();
    }else{
        ClipDrawable*cd = dynamic_cast<ClipDrawable*>(mProgressDrawable);
        ScaleDrawable*sd= dynamic_cast<ScaleDrawable*>(mProgressDrawable);
        if(cd)gravity = cd->getGravity();
        if(sd)gravity = sd->getGravity();
    }
    return gravity;
}


int ProgressBar::getProgressOrientation()const{
    const int gravity = getProgressGravity();
    if(gravity==Gravity::NO_GRAVITY)
        return (getWidth()>getHeight())?HORIZONTAL:VERTICAL;
    return Gravity::isHorizontal(gravity)?HORIZONTAL:VERTICAL;
}

void ProgressBar::setIndeterminateDrawable(Drawable*d){
    if (mIndeterminateDrawable != d) {
        if (mIndeterminateDrawable != nullptr) {
            mIndeterminateDrawable->setCallback(nullptr);
            unscheduleDrawable(*mIndeterminateDrawable);
            delete mIndeterminateDrawable;
        }

        mIndeterminateDrawable = d;
        updateDrawableBounds(getWidth(), getHeight());
        if (d != nullptr) {
            d->setCallback(this);
            d->setLayoutDirection(getLayoutDirection());
            if (d->isStateful()) {
                d->setState(getDrawableState());
            }
            applyIndeterminateTint();
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

void ProgressBar::setIndeterminateDrawableTiled(Drawable* d){
     if(d)d=tileifyIndeterminate(d);
     setIndeterminateDrawable(d);
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
    if ((getVisibility() != VISIBLE) ||(getWindowVisibility() != VISIBLE)) {
        return;
    }
    if (dynamic_cast<Animatable*>(mIndeterminateDrawable)) {
        mShouldStartAnimationDrawable = true;
        mHasAnimation=false;
    }else{
        mHasAnimation = true;
        if (mInterpolator == nullptr) {
            mInterpolator = LinearInterpolator::gLinearInterpolator.get();
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
    LOGV("mIndeterminateDrawable=%p",mIndeterminateDrawable);
    postInvalidate();
}

void ProgressBar::stopAnimation() {
    if (dynamic_cast<AnimatedRotateDrawable*>(mIndeterminateDrawable)) {
        ((AnimatedRotateDrawable*) mIndeterminateDrawable)->stop();
        mShouldStartAnimationDrawable = false;
    }else if(dynamic_cast<AnimatedImageDrawable*>(mIndeterminateDrawable)){
        ((AnimatedImageDrawable*) mIndeterminateDrawable)->stop();
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

    if (mShouldStartAnimationDrawable && dynamic_cast<Animatable*>(d)){
        if(dynamic_cast<AnimatedRotateDrawable*>(d)) 
            ((AnimatedRotateDrawable*) d)->start();
        else if(dynamic_cast<AnimationDrawable*>(d))
            ((AnimationDrawable*) d)->start();
        mShouldStartAnimationDrawable = false;
    }
    
}

void ProgressBar::onVisibilityAggregated(bool isVisible){
    View::onVisibilityAggregated(isVisible);
    if (isVisible != mAggregatedIsVisible) {
        mAggregatedIsVisible = isVisible;
        if (mIndeterminate) {
            // let's be nice with the UI thread
            if (isVisible) {
                startAnimation();
            } else {
                stopAnimation();
            }
        }
        if (mCurrentDrawable)mCurrentDrawable->setVisible(isVisible, false);
    }
}

void ProgressBar::invalidateDrawable(Drawable& dr) {
    if (!mInDrawing) {
        if (verifyDrawable(&dr)) {
            Rect dirty = dr.getBounds();
            int scrollX = mScrollX + mPaddingLeft;
            int scrollY = mScrollY + mPaddingTop;
            dirty.offset(scrollX,scrollY);
            invalidate(dirty);
        } else {
            View::invalidateDrawable(dr);
        }
    }
}

void ProgressBar::updateDrawableBounds(int w,int h){
    w -= mPaddingRight + mPaddingLeft;
    h -= mPaddingTop + mPaddingBottom;
    int right = w;
    int bottom = h;
    int top = 0;
    int left = 0;

    LOGV("mIndeterminateDrawable=%p,mOnlyIndeterminate=%d",mIndeterminateDrawable,mOnlyIndeterminate);
    if (mIndeterminateDrawable) {
        // Aspect ratio logic does not apply to AnimationDrawables
        if (mOnlyIndeterminate && !(dynamic_cast<AnimationDrawable*>(mIndeterminateDrawable))) {
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
            }
            LOGD_IF(intrinsicWidth*intrinsicHeight==0,"intrinsicsize=%dx%d",intrinsicWidth,intrinsicHeight);
        }
        if (isLayoutRtl() && mMirrorForRtl) {
            int tempLeft = left;
            left = w - right;
            right = w - tempLeft;
        }
        LOGV("%p setBounds(%d,%d-%d,%d) wh=%dx%d",this,left, top, right, bottom,w,h);
        mIndeterminateDrawable->setBounds(left, top, right-left, bottom-top);
    }

    if (mProgressDrawable != nullptr) {
        mProgressDrawable->setBounds(0, 0, right, bottom);
    }
}

void ProgressBar::onSizeChanged(int w,int h,int ow,int oh){
    LOGV("%p:%d sizechanged->(%d,%d)",this,mID,w,h);
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
        Drawable* target = getTintTarget(R::id::progress, true);
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
        Drawable* target = getTintTarget(R::id::background, false);
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
        Drawable* target = getTintTarget(R::id::secondaryProgress, false);
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

void ProgressBar::setProgressTintList(const ColorStateList*tint){
    if (mProgressTintInfo == nullptr) {
        mProgressTintInfo = new ProgressTintInfo();
    }
    if(mProgressTintInfo->mProgressTintList!=tint){
        mProgressTintInfo->mProgressTintList = tint;
        mProgressTintInfo->mHasProgressTint = (tint!=nullptr);
    }

    if (mProgressDrawable ) {
        applyPrimaryProgressTint();
    }
}

const ColorStateList* ProgressBar::getProgressTintList()const{
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

void ProgressBar::setProgressBackgroundTintList(const ColorStateList* tint) {
    if (mProgressTintInfo == nullptr) {
        mProgressTintInfo = new ProgressTintInfo();
    }
    if(mProgressTintInfo->mProgressBackgroundTintList!=tint ){
        mProgressTintInfo->mProgressBackgroundTintList = tint;
        mProgressTintInfo->mHasProgressBackgroundTint = (tint!=nullptr);
    }

    if (mProgressDrawable != nullptr) {
        applyProgressBackgroundTint();
    }
}

const ColorStateList*ProgressBar::getProgressBackgroundTintList()const{
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

void ProgressBar::setSecondaryProgressTintList(const ColorStateList* tint) {
    if (mProgressTintInfo == nullptr) {
        mProgressTintInfo = new ProgressTintInfo();
    }
    if( mProgressTintInfo->mSecondaryProgressTintList!=tint ){
        mProgressTintInfo->mSecondaryProgressTintList = tint;
        mProgressTintInfo->mHasSecondaryProgressTint = (tint!=nullptr);
    }

    if (mProgressDrawable != nullptr) {
        applySecondaryProgressTint();
    }
}

const ColorStateList* ProgressBar::getSecondaryProgressTintList()const{
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

void ProgressBar::applyIndeterminateTint(){

}

void ProgressBar::setProgressDrawableTiled(Drawable* d) {
    if (d != nullptr) {
        d = tileify(d, false);
    }
    setProgressDrawable(d);
}

}//end namespace
