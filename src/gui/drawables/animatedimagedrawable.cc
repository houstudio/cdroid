#include <drawables/animatedimagedrawable.h>
#include <systemclock.h>
#include <cdlog.h>
#include <gui/gui_features.h>
#include <view/view.h>
#include <image-decoders/imagedecoder.h>
#include <image-decoders/framesequence.h>
#include <porting/cdgraph.h>
namespace cdroid{
#define ENABLE_DMABLIT 0

AnimatedImageDrawable::AnimatedImageDrawable()
  :AnimatedImageDrawable(std::make_shared<AnimatedImageState>()){
}

AnimatedImageDrawable::AnimatedImageDrawable(std::shared_ptr<AnimatedImageState> state)
   :Drawable(){
    mStarting = false;
    mRepeated = 0;
    mRepeatCount = state?state->mRepeatCount:REPEAT_UNDEFINED;
    mIntrinsicWidth = mIntrinsicHeight = 0;
    mAnimatedImageState = state;
    mCurrentFrame= -1;
    mNextFrame= 0;
    mFrameScheduled = false;
    mImageHandler = nullptr;
    mFrameSequenceState = nullptr;
    auto frmSequence = mAnimatedImageState->mFrameSequence;
    if(frmSequence){
        mFrameSequenceState = frmSequence->createState();
        mImage = Cairo::ImageSurface::create(Cairo::Surface::Format::ARGB32,frmSequence->getWidth(),frmSequence->getHeight());
    }
}

AnimatedImageDrawable::AnimatedImageDrawable(cdroid::Context*ctx,const std::string&res)
   :AnimatedImageDrawable(){
    uint8_t*buffer;
    uint32_t pitch;
    auto istm = ctx->getInputStream(res);
    auto frmSequence = FrameSequence::create(istm.get());
    mAnimatedImageState->mFrameSequence = frmSequence;
    mRepeatCount = frmSequence->getDefaultLoopCount();
    if(mRepeatCount<=0)
        mRepeatCount = REPEAT_UNDEFINED;
    mFrameSequenceState = frmSequence->createState();
#if ENABLE(DMABLIT)
    GFXCreateSurface(0,&mImageHandler,frmSequence->getWidth(),frmSequence->getHeight(),0,0);
    GFXLockSurface(mImageHandler,(void**)&buffer,&pitch);
    mImage = Cairo::ImageSurface::create(buffer,Cairo::Surface::Format::ARGB32,frmSequence->getWidth(),frmSequence->getHeight(),pitch);
#else
    mImage = Cairo::ImageSurface::create(Cairo::Surface::Format::ARGB32,frmSequence->getWidth(),frmSequence->getHeight());
#endif
    LOGD("%s image %dx%dx%d",res.c_str(),frmSequence->getWidth(),frmSequence->getHeight(),frmSequence->getFrameCount());
    mAnimatedImageState->mFrameCount = frmSequence->getFrameCount();
}

AnimatedImageDrawable::~AnimatedImageDrawable(){
    mStarting = false;
    if(mRunnable)
        unscheduleSelf(mRunnable);
    mRunnable = nullptr;
    delete mFrameSequenceState;
    if(mImageHandler){
        GFXDestroySurface(mImageHandler);
        mImageHandler = nullptr;
    }
}

std::shared_ptr<Drawable::ConstantState>AnimatedImageDrawable::getConstantState(){
    return std::dynamic_pointer_cast<ConstantState>(mAnimatedImageState);
}

void AnimatedImageDrawable::setRepeatCount(int repeatCount){
    if (repeatCount < REPEAT_INFINITE) {
        LOGE("invalid value passed to setRepeatCount %d",repeatCount);
    }
    if (mRepeatCount != repeatCount) {
        mRepeatCount = repeatCount;
    }
}

int AnimatedImageDrawable::getRepeatCount()const{
    auto frameSequence = mAnimatedImageState->mFrameSequence;
    return mRepeatCount;
}

int AnimatedImageDrawable::getIntrinsicWidth()const{
    auto frameSequence = mAnimatedImageState->mFrameSequence;
    return frameSequence ? frameSequence->getWidth():mIntrinsicWidth;
}

int AnimatedImageDrawable::getIntrinsicHeight()const{
    auto frameSequence = mAnimatedImageState->mFrameSequence;
    return frameSequence ? frameSequence->getHeight():mIntrinsicHeight;
}

void AnimatedImageDrawable::setAlpha(int alpha){
    mAnimatedImageState->mAlpha = alpha&0xFF;
    invalidateSelf();
}

int AnimatedImageDrawable::getAlpha()const{
    return mAnimatedImageState->mAlpha;
}

int AnimatedImageDrawable::getOpacity(){
    return PixelFormat::TRANSLUCENT;
}

void AnimatedImageDrawable::setAutoMirrored(bool mirrored) {
    if (mAnimatedImageState->mAutoMirrored != mirrored) {
        mAnimatedImageState->mAutoMirrored = mirrored;
        if (getLayoutDirection() == View::LAYOUT_DIRECTION_RTL) {
            //nSetMirrored(mState.mNativePtr, mirrored);
            invalidateSelf();
        }
    }
}

bool AnimatedImageDrawable::isAutoMirrored() const{
    return mAnimatedImageState->mAutoMirrored;
}

bool AnimatedImageDrawable::onLayoutDirectionChanged(int layoutDirection) {
    if (!mAnimatedImageState->mAutoMirrored) {
        return false;
    }

    const bool mirror = layoutDirection == View::LAYOUT_DIRECTION_RTL;
    mAnimatedImageState->mAutoMirrored= mirror;
    return true;
}

void AnimatedImageDrawable::draw(Canvas& canvas){
    if(mImage == nullptr)return;
    if (mStarting && (mCurrentFrame == 0) ) {
        postOnAnimationStart();
    }
    canvas.save();
    auto frmSequence = mAnimatedImageState->mFrameSequence;
    if( (mCurrentFrame != mNextFrame) && mAnimatedImageState->mFrameCount){
        const long startTime  = SystemClock::uptimeMillis();
        mFrameDelay = mFrameSequenceState->drawFrame(mNextFrame,(uint32_t*)mImage->get_data(),mImage->get_stride()>>2,mCurrentFrame);
        const long decodeTime = SystemClock::uptimeMillis() - startTime;
        mFrameDelay = (decodeTime >= mFrameDelay)?(mFrameDelay/2):(mFrameDelay - decodeTime);
        mCurrentFrame = mNextFrame;
        mImage->mark_dirty();
    }
    // a value <= 0 indicates that the drawable is stopped or that renderThread
    // will manage the animation
    LOGV("%p draw Frame %d/%d started=%d repeat=%d/%d nextDelay=%d",this,mCurrentFrame,
          mAnimatedImageState->mFrameCount,mStarting,mRepeated,mRepeatCount,mFrameDelay);
    if( mStarting && ( (mRepeated < mRepeatCount) || (mRepeatCount < 0))){
        if (mFrameDelay > 0) {
            if (mRunnable == nullptr) {
                mRunnable = [this](){
                    if(mStarting && mAnimatedImageState->mFrameCount){
                        invalidateSelf();
                        mNextFrame = (mNextFrame + 1)%mAnimatedImageState->mFrameCount;
                    }
                    if( mNextFrame == mAnimatedImageState->mFrameCount - 1){
                        mRepeated ++;
                        mStarting = (mRepeated < mRepeatCount)||(mRepeatCount < 0);
                    }
                    mCurrentFrame = (mNextFrame - 1) % mAnimatedImageState->mFrameCount;
                    mFrameScheduled = false;
                };
            }
            if(!mFrameScheduled){
                unscheduleSelf(mRunnable);
                scheduleSelf(mRunnable, SystemClock::uptimeMillis() + mFrameDelay);
                mFrameScheduled = true;
            }
        }
        if ( mCurrentFrame == mAnimatedImageState->mFrameCount - 1){// == FINISHED) {
            // This means the animation was drawn in software mode and ended.
            postOnAnimationEnd();
        }
    }
    void *handler = canvas.getHandler();
    if( (mImageHandler == nullptr) || (handler == nullptr) ){
        const bool isOpaque = mAnimatedImageState->mFrameSequence->isOpaque();
        canvas.set_source(mImage,mBounds.left,mBounds.top);
        canvas.set_operator(isOpaque?Cairo::Context::Operator::SOURCE:Cairo::Context::Operator::OVER);
        canvas.rectangle(mBounds.left,mBounds.top,mBounds.width,mBounds.height);
        canvas.fill();
    }else{
#if ENABLE(DMABLIT)
        Rect rd = {0,0,mBounds.width,mBounds.height};
        Cairo::Matrix mtx = canvas.get_matrix();
        double angle = std::atan2(mtx.xx,-mtx.xy)*180.0/M_PI;
        LOGD("canvas.angle = %d",(int(angle) - 90) % 360);
        GFXBlit(handler,0,0,mImageHandler,nullptr);
#endif
    }
    canvas.restore();
}

bool AnimatedImageDrawable::isRunning(){
    return mStarting;
}

void AnimatedImageDrawable::start(){
    if (mAnimatedImageState->mFrameSequence == nullptr) {
        throw "called start on empty AnimatedImageDrawable";
    }

    if ((mStarting==false)&&(mAnimatedImageState->mFrameCount>1)){
        mStarting = true;
        mRepeated = 0;
        invalidateSelf();
    }
}

void AnimatedImageDrawable::restart(int fromFrame){
    mCurrentFrame=-1;
    mNextFrame = fromFrame;
    if((mStarting==false)&&mAnimatedImageState->mFrameCount){
        invalidateSelf();
        postOnAnimationStart();
    }
}

void AnimatedImageDrawable::stop(){
    if (mAnimatedImageState->mFrameSequence == nullptr) {
        throw "called stop on empty AnimatedImageDrawable";
    }
    if (mStarting){
	mStarting = false;
        postOnAnimationEnd();
    }
}

void AnimatedImageDrawable::registerAnimationCallback(Animatable2::AnimationCallback callback){
    mAnimationCallbacks.push_back(callback);
}

static bool operator==(const Animatable2::AnimationCallback&a,const Animatable2::AnimationCallback&b){
    return (a.onAnimationStart==b.onAnimationStart) && (a.onAnimationEnd==b.onAnimationEnd);
}

bool AnimatedImageDrawable::unregisterAnimationCallback(Animatable2::AnimationCallback callback){
    auto it = std::find(mAnimationCallbacks.begin(),mAnimationCallbacks.end(),callback);
    const bool rc = (it!=mAnimationCallbacks.end());
    if(rc)
        mAnimationCallbacks.erase(it);
    return rc;
}

void AnimatedImageDrawable::postOnAnimationStart(){
    if (mAnimationCallbacks.size() == 0) {
        return;
    }
    Runnable r([this](){
        for (Animatable2::AnimationCallback callback : mAnimationCallbacks) {
            callback.onAnimationStart(*this);
        }
    });
    //todo post callback
}

void AnimatedImageDrawable::postOnAnimationEnd(){
    if (mAnimationCallbacks.size()==0) {
        return;
    }
    Runnable r([this]{
        for (Animatable2::AnimationCallback callback : mAnimationCallbacks) {
            callback.onAnimationEnd(*this);
        }
    });
}

void AnimatedImageDrawable::clearAnimationCallbacks(){
    mAnimationCallbacks.clear();
}

void AnimatedImageDrawable::onBoundsChange(const Rect& bounds) {
    /*if (mAnimatedImageState.mNativePtr != 0) {
        nSetBounds(mState.mNativePtr, bounds);
    }*/
}

Drawable*AnimatedImageDrawable::inflate(Context*ctx,const AttributeSet&atts){
    const std::string res = atts.getString("drawable");
    AnimatedImageDrawable*d = new AnimatedImageDrawable(ctx,res);
    const bool autoStart = atts.getBoolean("autoStart");
    const int repeatCount =atts.getInt("repeatCount",REPEAT_UNDEFINED);
    if(autoStart)d->start();
    if(repeatCount!=REPEAT_UNDEFINED){
	d->mAnimatedImageState->mRepeatCount = repeatCount;
	d->setRepeatCount(repeatCount);
    }
    return d;
}

////////////////////////////////////////////////////////////////////////////////////////////////
AnimatedImageDrawable::AnimatedImageState::AnimatedImageState(){
    mAutoMirrored= false;
    mFrameCount  = 0;
    mFrameSequence = nullptr;
}

AnimatedImageDrawable::AnimatedImageState::AnimatedImageState(const AnimatedImageState& state){
    mAutoMirrored = state.mAutoMirrored;
    mFrameCount = state.mFrameCount;
    mFrameSequence = state.mFrameSequence;
}

AnimatedImageDrawable::AnimatedImageState::~AnimatedImageState(){
    delete mFrameSequence;
}

AnimatedImageDrawable* AnimatedImageDrawable::AnimatedImageState::newDrawable(){
    return new AnimatedImageDrawable(std::dynamic_pointer_cast<AnimatedImageState>(shared_from_this()));
}

int AnimatedImageDrawable::AnimatedImageState::getChangingConfigurations()const{
    return 0;
}

}
