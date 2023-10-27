#include <drawables/animatedimagedrawable.h>
#include <systemclock.h>
#include <cdlog.h>
#include <gui/gui_features.h>
#include <image-decoders/imagedecoder.h>
namespace cdroid{

AnimatedImageDrawable::AnimatedImageDrawable():Drawable(){
    mAnimatedImageState = std::make_shared<AnimatedImageState>();
    mHandler = nullptr;
    mStarting = false;
    mIntrinsicWidth = 0;
    mIntrinsicHeight= 0;
    mCurrentFrame= 0;
    mRepeatCount = REPEAT_UNDEFINED;
}

AnimatedImageDrawable::AnimatedImageDrawable(std::shared_ptr<AnimatedImageState> state){
    mAnimatedImageState =state;
}

AnimatedImageDrawable::AnimatedImageDrawable(cdroid::Context*ctx,const std::string&res)
   :AnimatedImageDrawable(){
    LOGD("decoder=%p res=%s",mAnimatedImageState->mDecoder,res.c_str());
    ImageDecoder*decoder = ImageDecoder::create(ctx,res);
    mAnimatedImageState->mDecoder = decoder;
    mAnimatedImageState->mImage=Cairo::ImageSurface::create(Cairo::Surface::Format::ARGB32,decoder->getWidth(),decoder->getHeight());
}

AnimatedImageDrawable::~AnimatedImageDrawable(){
}

std::shared_ptr<Drawable::ConstantState>AnimatedImageDrawable::getConstantState(){
    return std::dynamic_pointer_cast<ConstantState>(mAnimatedImageState);
}

Handler* AnimatedImageDrawable::getHandler() {
    if (mHandler == nullptr) {
        mHandler = new Handler();//Looper.getMainLooper());
    }
    return mHandler;
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
    return mIntrinsicWidth;
}

int AnimatedImageDrawable::getIntrinsicHeight()const{
    return mIntrinsicHeight;
}

void AnimatedImageDrawable::setAlpha(int alpha){
    
}

int AnimatedImageDrawable::getAlpha()const{
    return 255;
}

constexpr int FINISHED=-1;
void AnimatedImageDrawable::draw(Canvas& canvas){
    if (mStarting) {
        mStarting = false;
        postOnAnimationStart();
    }
    canvas.save();
    Cairo::RefPtr<Cairo::ImageSurface>image = mAnimatedImageState->mImage;
    ImageDecoder*mDecoder = mAnimatedImageState->mDecoder;
    const long nextDelay = mDecoder->getFrameDuration(mCurrentFrame);
    mDecoder->readImage(image,mCurrentFrame);
    // a value <= 0 indicates that the drawable is stopped or that renderThread
    // will manage the animation
    LOGV("draw Frame %d/%d nextDelay=%d",mCurrentFrame,mAnimatedImageState->mFrameCount,nextDelay);
    if (nextDelay > 0) {
        if (mRunnable == nullptr) {
            mRunnable = std::bind(&AnimatedImageDrawable::invalidateSelf,this);
        }
        scheduleSelf(mRunnable, nextDelay + SystemClock::uptimeMillis());
    } else if (nextDelay<=0){// == FINISHED) {
        // This means the animation was drawn in software mode and ended.
        postOnAnimationEnd();
    }
    image->mark_dirty();
    canvas.set_source(image,mBounds.left,mBounds.top);
    canvas.rectangle(mBounds.left,mBounds.top,mBounds.width,mBounds.height);
    canvas.fill();
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
    auto it=std::find(mAnimationCallbacks.begin(),mAnimationCallbacks.end(),callback);
    bool rc=(it!=mAnimationCallbacks.end());
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
    getHandler()->post(r);
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
    getHandler()->post(r);
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
}

AnimatedImageDrawable::AnimatedImageState::~AnimatedImageState(){
    LOGD("AnimatedImageState~AnimatedImageState");
    delete mDecoder;
}

Drawable* AnimatedImageDrawable::AnimatedImageState::newDrawable(){
    return new AnimatedImageDrawable(shared_from_this());
}

int AnimatedImageDrawable::AnimatedImageState::getChangingConfigurations()const{
    return 0;
}

}
