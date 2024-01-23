#include <drawables/animatedimagedrawable.h>
#include <systemclock.h>
#include <cdlog.h>
#include <gui/gui_features.h>
#include <image-decoders/imagedecoder.h>
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
    mRepeatCount = REPEAT_UNDEFINED;
    mIntrinsicWidth = mIntrinsicHeight = 0;
    mAnimatedImageState = state;
    mCurrentFrame= 0;
    mImageHandler = nullptr;
}

AnimatedImageDrawable::AnimatedImageDrawable(cdroid::Context*ctx,const std::string&res)
   :AnimatedImageDrawable(){
    uint8_t*buffer;
    uint32_t pitch;
    LOGD("decoder=%p res=%s",mAnimatedImageState->mDecoder,res.c_str());
    ImageDecoder*decoder = ImageDecoder::create(ctx,res);
    mAnimatedImageState->mDecoder = decoder;
    decoder->load();
#if ENABLE(DMABLIT)
    GFXCreateSurface(0,&mImageHandler,decoder->getWidth(),decoder->getHeight(),0,0);
    GFXLockSurface(mImageHandler,(void**)&buffer,&pitch);
    mAnimatedImageState->mImage = Cairo::ImageSurface::create(buffer,Cairo::Surface::Format::ARGB32,decoder->getWidth(),decoder->getHeight(),pitch);
#else
    mAnimatedImageState->mImage = Cairo::ImageSurface::create(Cairo::Surface::Format::ARGB32,decoder->getWidth(),decoder->getHeight());
#endif
    LOGI("image %dx%dx%d hwsurface.buffer=%p",decoder->getWidth(),decoder->getHeight(),pitch,buffer);
    mAnimatedImageState->mFrameCount = decoder->getFrameCount();
}

AnimatedImageDrawable::~AnimatedImageDrawable(){
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
    return mRepeatCount;
}

int AnimatedImageDrawable::getIntrinsicWidth()const{
    const ImageDecoder*decoder = mAnimatedImageState->mDecoder;
    return decoder?decoder->getWidth():mIntrinsicWidth;
}

int AnimatedImageDrawable::getIntrinsicHeight()const{
    const ImageDecoder*decoder = mAnimatedImageState->mDecoder;
    return decoder?decoder->getHeight():mIntrinsicHeight;
}

void AnimatedImageDrawable::setAlpha(int alpha){
    
}

int AnimatedImageDrawable::getAlpha()const{
    return 255;
}

void AnimatedImageDrawable::draw(Canvas& canvas){
    if (mStarting && (mCurrentFrame==0) ) {
        postOnAnimationStart();
    }
    canvas.save();
    Cairo::RefPtr<Cairo::ImageSurface>image = mAnimatedImageState->mImage;
    ImageDecoder*mDecoder = mAnimatedImageState->mDecoder;
    mDecoder->readImage(image,mCurrentFrame);
    const long nextDelay = mDecoder->getFrameDuration(mCurrentFrame);
    // a value <= 0 indicates that the drawable is stopped or that renderThread
    // will manage the animation
    LOGV("%p draw Frame %d/%d repeat=%d/%d nextDelay=%d",this,mCurrentFrame,
          mAnimatedImageState->mFrameCount,mRepeated,mRepeatCount,nextDelay);
    if(mStarting && ((mRepeated<mRepeatCount) || (mRepeatCount==REPEAT_INFINITE))){
        if (nextDelay > 0) {
            if (mRunnable == nullptr) {
                mRunnable = [this](){
                    invalidateSelf();
                    mCurrentFrame=(mCurrentFrame+1)%mAnimatedImageState->mFrameCount;
                    if(mCurrentFrame==mAnimatedImageState->mFrameCount-1){
                        mRepeated++;
                        mStarting = (mRepeated>=mRepeatCount);
                    }
                };
            }
            unscheduleSelf(mRunnable);
            scheduleSelf(mRunnable, nextDelay + SystemClock::uptimeMillis());
        }
        if ( mCurrentFrame==mAnimatedImageState->mFrameCount-1){// == FINISHED) {
            // This means the animation was drawn in software mode and ended.
            postOnAnimationEnd();
        }
    }
    image->mark_dirty();
    void*handler = canvas.getHandler();
    if((mImageHandler==nullptr)||(handler==nullptr)){
        canvas.set_source(image,mBounds.left,mBounds.top);
        canvas.set_operator(Cairo::Context::Operator::SOURCE);
        canvas.rectangle(mBounds.left,mBounds.top,mBounds.width,mBounds.height);
        canvas.fill();
    }else{
#if ENABLE(DMABLIT)
        Rect rd = {0,0,mBounds.width,mBounds.height};
        Cairo::Matrix mtx = canvas.get_matrix();
        double angle = std::atan2(mtx.xx,-mtx.xy)*180.0/M_PI;
        LOGD("canvas.angle=%d",(int(angle)-90)%360);
        GFXBlit(handler,0,0,mImageHandler,nullptr);
#endif
    }
    canvas.restore();
}

bool AnimatedImageDrawable::isRunning(){
    return true;    
}

void AnimatedImageDrawable::start(){
    if (mAnimatedImageState->mDecoder == nullptr) {
        throw "called start on empty AnimatedImageDrawable";
    }

    if ((mStarting==false)&&(mAnimatedImageState->mFrameCount>1)){
        mStarting = true;
        invalidateSelf();
    }
}

void AnimatedImageDrawable::stop(){
    if (mAnimatedImageState->mDecoder == nullptr) {
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

Drawable*AnimatedImageDrawable::inflate(Context*ctx,const AttributeSet&atts){
    const std::string res = atts.getString("src");
    AnimatedImageDrawable*d = new AnimatedImageDrawable(ctx,res);
    const bool autoStart = atts.getBoolean("autoStart");
    const int repeatCount =atts.getInt("repeatCount",REPEAT_UNDEFINED);
    if(autoStart)d->start();
    if(repeatCount!=REPEAT_UNDEFINED)
	d->setRepeatCount(repeatCount);
    return d;
}

////////////////////////////////////////////////////////////////////////////////////////////////
AnimatedImageDrawable::AnimatedImageState::AnimatedImageState(){
    mAutoMirrored= false;
    mFrameCount  = 0;
}

AnimatedImageDrawable::AnimatedImageState::AnimatedImageState(const AnimatedImageState& state){
    mAutoMirrored = state.mAutoMirrored;
    mFrameCount = state.mFrameCount;
    mDecoder = state.mDecoder;
    mImage = state.mImage;
}

AnimatedImageDrawable::AnimatedImageState::~AnimatedImageState(){
    LOGD("AnimatedImageState~AnimatedImageState");
    delete mDecoder;
}

AnimatedImageDrawable* AnimatedImageDrawable::AnimatedImageState::newDrawable(){
    return new AnimatedImageDrawable(std::dynamic_pointer_cast<AnimatedImageState>(shared_from_this()));
}

int AnimatedImageDrawable::AnimatedImageState::getChangingConfigurations()const{
    return 0;
}

}
